#include <algorithm>
#include <error.h>
#include <errno.h>

#include <Config.h>
#include <util/TimeOperation.h>
#include <Exception/Exception.h>
#include <Exception/ParameterException.h>

#include "ConnectionPool.h"
#include "ResultSet.h"
#include "PreparedStatement.h"

ConnectionPool::ConnectionPool(char const* url):
    _maxConnections(SQL_DEFAULT_MAX_CONNECTIONS),
    _initialConnections(SQL_DEFAULT_INIT_CONNECTIONS),
    _connectionTimeout(SQL_DEFAULT_CONNECTION_TIMEOUT),
    _filled(false),
    _doSweep(false),
    _alarm(_mutex),
    _sweepInterval(DEFAULT_SWEEP_INTERVAL),
    _stopped(true)
{
    if(UNLIKELY(!url))
        THROW(ParameterException , "url is invalid");
    _url.reset(new URL(url)),
    _connectionsVec.reserve(SQL_DEFAULT_MAX_CONNECTIONS);
}

ConnectionPool::~ConnectionPool()
{
    if(!_stopped) stop();
}

URLPtr ConnectionPool::getURL()
{
    return _url;
}

void ConnectionPool::setInitialConnections(int connections)
{
    assert(connections >= 0);
    {
        MutexLockGuard lock(_mutex);
        _initialConnections = connections;
    }
}

int ConnectionPool::getInitialConnections() const
{
    return _initialConnections;
}

void ConnectionPool::setMaxConnections(int maxConnections)
{
    assert(_initialConnections <= maxConnections);
    {
        MutexLockGuard lock(_mutex);
        _maxConnections = maxConnections;
    }
}

int ConnectionPool::getMaxConnections() const
{
    return _maxConnections;
}

void ConnectionPool::setConnectionTimeout(int connectionTimeout)
{
    assert(connectionTimeout > 0);
    _connectionTimeout = connectionTimeout;
}

int ConnectionPool::getConnectionTimeout() const
{
    return _connectionTimeout;
}

/*
void ConnectionPool::setAbortHandler(AbortHandlerFunc handler)//TODO
{
    _abortHandler = handler;
}*/

void ConnectionPool::setStopHandler(StopHandler handler)
{
    _stopHandler = handler;
}

void ConnectionPool::setReaper(int sweepInterval)
{
    assert(sweepInterval > 0);
    {
        MutexLockGuard lock(_mutex);
        _doSweep = true;
        _sweepInterval = sweepInterval;
    }
}

int ConnectionPool::getSize() const
{
    return _connectionsVec.size();
}

int ConnectionPool::getActiveConnections()
{
    int n = 0;
    {
        MutexLockGuard lock(_mutex);
        n = doGetActiveConnections();
    }
    return n;
}

void ConnectionPool::stop()
{
    int stopSweep = false;
    {
        MutexLockGuard lock(_mutex);
        _stopped = true;
        if(_filled)
        {
            drainPool();
            _filled = false;
            stopSweep = _doSweep && _reaper;
            if(_stopHandler) _stopHandler();
        }
    }
    if(stopSweep)
    {
        LOG_DEBUG << "Stopping Database reaper thread...\n";
        _alarm.notify();
        _reaper->join();
    }
}

ConnectionPtr ConnectionPool::getConnection()
{
    ConnectionPtr conn;
    {
        MutexLockGuard lock(_mutex);
        int size = _connectionsVec.size();
        for( int i = 0; i != size ; ++i )
        {
            ConnectionPtr temp = _connectionsVec.at(i);
            if(temp->isAvailable() && temp->ping())
            {
                temp->setAvailable(false);
                conn.swap(temp);
                goto done;
            }
        }
        if(size <= _maxConnections)
        {
            ConnectionPtr temp(new Connection(this));
            if(temp)
            {
                temp->setAvailable(false);
                _connectionsVec.push_back(temp);
                conn.swap(temp);
            }
            else
            {
                LOG_DEBUG << "Failed to create connection\n";
            }
        }
    }
done:
    return conn;
}

void ConnectionPool::returnConnection(ConnectionPtr conn)
{
    assert(conn);
    if(conn->isInTransaction())
    {
        try
        {
            conn->rollback();
        }
        catch(...)
        {
            LOG_ERROR << "Connection can't rollback\n";
        }
    }
    conn->clear();
    {
        MutexLockGuard lock(_mutex);
        conn->setAvailable(true);
    }
}

int ConnectionPool::reapConnections()
{
    int n = 0;
    {
        MutexLockGuard lock(_mutex);
        n = doReapConnections();
    }
    return n;
}

CONST_STDSTR ConnectionPool::getVersion() const
{
    return ABOUT;
}

int ConnectionPool::getSweepInterval() const
{
    return _sweepInterval;
}

#ifdef TEST
bool ConnectionPool::isFilled() const
{
    return _filled;
}

bool ConnectionPool::needDoSweep() const
{
    return _doSweep;
}

ThreadPtr ConnectionPool::getReaper()
{
    return _reaper;
}
#endif

void ConnectionPool::drainPool()
{
    _connectionsVec.clear();
}

int ConnectionPool::doGetActiveConnections()
{
    int n = 0;
    std::for_each(_connectionsVec.begin() , _connectionsVec.end(),
                  [&n](ConnectionPtr& conn){if(conn->isAvailable()) ++n;});
    return n;
}

int ConnectionPool::doReapConnections()
{
    int n = 0;
    int totalSize = _connectionsVec.size();
    int reapUpperLimit = totalSize - getActiveConnections() - _initialConnections;
    time_t timeout = Time_now() - _connectionTimeout;
    ConnectionPtr conn;
    for(UINT i = 0 ; (i != _connectionsVec.size()) && n != reapUpperLimit + 1 ; ++i)
    {
        conn = _connectionsVec[i];
        if(conn->isAvailable())
        {
            if( (conn->getLastAccessedTime() < timeout) || (!conn->ping()) )
            {
                conn->clear();
                _connectionsVec.erase(_connectionsVec.begin() + i);
                ++n;
                --i;
            }
        }
    }
    return n;
}

void ConnectionPool::doSweep()
{
    {
        MutexLockGuard lock(_mutex);
        while(!_stopped)
        {
            _alarm.waitForSeconds(_sweepInterval);
            if(_stopped) break;
            reapConnections();
        }
    }
    LOG_DEBUG << "Reaper thread stopped\n";
}