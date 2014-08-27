#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <algorithm>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <muduo/base/Thread.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Condition.h>

#include "URL.h"
#include "ResultSet.h"
#include "PreparedStatement.h"
#include "Connection.h"

using namespace muduo;

typedef boost::function<void (char const*)> AbortHandlerFunc;
typedef boost::function<void (void)> ThreadFunc;
typedef boost::shared_ptr<Thread> ThreadPtr;

#define VERSION 0.1
#define OOzdb_URL "https://github.com/wuyu201321060203/OOzdb"

#define ABOUT "OOzdb/" #VERSION " Copyright (C) Wu Yu" OOzdb_URL

class ConnectionPool
{
public:

    ConnectionPool(URLPtr url);
    ~ConnectionPool();
    URLPtr getURL();
    void setInitialConnections(int connections);
    int getInitialConnections();
    void setMaxConnections(int maxConnections);
    int getMaxConnections();
    void setConnectionTimeout(int connectionTimeout);
    int getConnectionTimeout();
    void setAbortHandler(AbortHandlerFunc handler);
    void setReaper(int sweepInterval);
    int getSize();
    int getActiveConnections();
    template<typename ConcreteConnection> void start();
    void stop();
    ConnectionPtr getConnection();
    void returnConnection(ConnectionPtr conn);
    int reapConnections();
    CONST_STDSTR getVersion();

private:

    URLPtr _url;
    int _maxConnections;
    int _initialConnections;
    int _connectionTimeout;
    bool _filled;
    bool _doSweep;
    char* _error;
    Condition _alarm;
    MutexLock _mutex;
    int _sweepInterval;
    volatile int _stopped;
    AbortHandlerFunc _handler;
    ThreadPtr _reaper;

private:

    ConnectionVec _connectionsVec;

private:

    void drainPool();
    template<typename ConcreteConnection> int fillPool();
    int getActive();
    int onReapConnections();
    virtual void doSweep();//TODO
    virtual void onStop();
};

ConnectionPool::ConnectionPool(URLPtr url):
    _url(url),
    _maxConnections(SQL_DEFAULT_MAX_CONNECTIONS),
    _initialConnections(SQL_DEFAULT_INIT_CONNECTIONS),
    _connectionTimeout(SQL_DEFAULT_CONNECTION_TIMEOUT),
    _reaper( new Thread( boost::bind(&doSweep , this) ) )
{
    assert(_url);
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
        MutexLockGuard lock(&_mutex);
        _initialConnections = connections;
    }
}

int ConnectionPool::getInitialConnections()
{
    return _initialConnections;
}

void ConnectionPool::setMaxConnections(int maxConnections)
{
    assert(_initialConnections <= maxConnections);
    {
        MutexLockGuard lock(&_mutex);
        _maxConnections = maxConnections;
    }
}

int ConnectionPool::getMaxConnections()
{
    return _maxConnections;
}

void ConnectionPool::setConnectionTimeout(int connectionTimeout)
{
    assert(connectionTimeout > 0);
    _connectionTimeout = connectionTimeout;
}

int ConnectionPool::getConnectionTimeout()
{
    return _connectionTimeout;
}

void ConnectionPool::setAbortHandler(AbortHandlerFunc handler)
{
    _handler = handler;
}

void ConnectionPool::setReaper(int sweepInterval)
{
    assert(sweepInterval > 0);
    {
        MutexLockGuard lock(&_mutex);
        _doSweep = true;
        _sweepInterval = sweepInterval;
    }
}

int ConnectionPool::getSize()
{
    return _connectionsVec.size();
}

int ConnectionPool::getActiveConnections()
{
    int n = 0;
    {
        MutexLockGuard lock(&_mutex);
        n = getActive();
    }
    return n;
}

template<typename ConcreteConnection>
void ConnectionPool::start()
{
    {
        MutexLockGuard lock(&_mutex);
        _stopped = false;
        _filled = fillPool<ConcreteConnection>();
        if(_filled && _doSweep)
        {
            LOG_DEBUG << "Starting Database reaper thread\n";
            _reaper->start();
        }
    }
    if(!_filled)
        throw SQLException("Failed to start connection pool");
}

void ConnectionPool::stop()
{
    int stopSweep = false;
    {
        MutexLockGuard(&_mutex);
        _stopped = true;
        if(_filled)
        {
            drainPool();
            _filled = false;
            stopSweep = _doSweep && _reaper;
            onStop();
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
    ConnectionPtr conn(NULL);
    {
        MutexLockGuard(&_mutex);
        int i = 0;
        int size = _connectionsVec.size();
        for( ; i != size ; ++i )
        {
            ConnectionPtr temp = _connectionsVec[i];
            if(temp->isAvailable() && temp->ping())
            {
                temp->setAvailable(false);
                conn = temp;
                goto done;
            }
        }
        if(size < _maxConnections)
        {
            ConnectionPtr temp(new Connection(enable_shared_from_this()));
            if(temp)
            {
                temp->setAvailable(false);
                _connectionsVec.push_back(temp);
                conn = temp;
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
        MutexLockGuard(&_mutex);
        conn->setAvailable(true);
    }
}

int ConnectionPool::reapConnections()
{
    int n = 0;
    {
        MutexLockGuard(&_mutex);
        n = onReapConnections();
    }
    return n;
}

CONST_STDSTR ConnectionPool::getVersion()
{
    return ABOUT;
}

void ConnectionPool::drainPool()
{
    std::for_each( _connectionsVec.begin() , _connectionsVec.end(),
                  [](ConnectionPtr& conn){ conn->clear(); } );

    _connectionsVec.clear();
}

template<typename ConcreteConnection>
int ConnectionPool::fillPool()
{
    for(int i = 0 ; i != _initialConnections ; ++i)
    {
        ConnectionPtr conn( new ConcreteConnection( enable_shared_from_this() ) );
        if(!conn)
        {
            if(i > 0)
            {
                LOG_DEBUG << "failed to fill the pool with initial connections";
                return true;
            }
            return false;
        }
        _connectionsVec.push_back(conn);
    }
    return true;
}

int ConnectionPool::getActive()
{
    int n = 0;
    std::for_each(_connectionsVec.begin() , _connectionsVec.end(),
                  [&n](ConnectionPtr& conn){if(conn->isAvailable()) ++n;});
    return n;
}

int ConnectionPool::onReapConnections()
{
    int n = 0;
    int totalSize = _connectionsVec.size();
    int reapUpperLimit = size - getActive() - _initialConnections;
    time_t timeout = Time::now() - _connectionTimeout;
    ConnectionPtr conn;
    for(int i = 0 ; (i != _connectionsVec.size()) && n != reapUpperLimit ; ++i)
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

void ConnectionPool::doSweep()//TODO
{
    {
        MutexLockGuard(&_mutex);
        while(!_stopped)
        {
            _alarm.waitForSeconds(_sweepInterval);
            if(_stopped) break;
            reapConnections();
        }
    }
    LOG_DEBUG << "Reaper thread stopped\n";
    return NULL;
}

#endif