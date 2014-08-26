#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <algorithm>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <muduo/base/Thread.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Condition.h>

#include "URL.h"
#include "ResultSet.h"
#include "PreparedStatement.h"
#include "Connection.h"

using namespace muduo;

typedef boost::function<void (char const*)> AbortHandlerFunc;

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
    void start();
    void stop();
    ConnectionPtr getConnection();
    void returnConnection();
    int reapConnections();
    CONST_STDSTR getVersion();

private:

    URLPtr _url;
    bool _filled;
    bool _doSweep;
    char* _error;
    Condition _alarm;
    MutexLock _mutex;
    Thread _reaper;
    int _sweepInterval;
    int _maxConnections;
    volatile int _stopped;
    int _connectionTimeout;
    int _initialConnections;

private:

    ConnectionVec _connectionsVec;

private:

    void drainPool();
    int fillPool();
    int getActive();
    int reapConnections();
    void* doSweep(void* args);//TODO
};

ConnectionPool::ConnectionPool(URLPtr url)
{

}

ConnectionPool::~ConnectionPool()
{

}

URLPtr ConnectionPool::getURL()
{

}

void ConnectionPool::setInitialConnections(int connections)
{

}

int ConnectionPool::getInitialConnections()
{

}

void ConnectionPool::setMaxConnections(int maxConnections)
{

}

int ConnectionPool::getMaxConnections()
{

}

void ConnectionPool::setConnectionTimeout(int connectionTimeout)
{

}

int ConnectionPool::getConnectionTimeout()
{

}

void ConnectionPool::setAbortHandler(AbortHandlerFunc handler)
{

}

void ConnectionPool::setReaper(int sweepInterval)
{

}

int ConnectionPool::getSize()
{

}

int ConnectionPool::getActiveConnections()
{

}

void ConnectionPool::start()
{

}

void ConnectionPool::stop()
{

}

ConnectionPtr ConnectionPool::getConnection()
{

}

void ConnectionPool::returnConnection()
{

}

int ConnectionPool::reapConnections()
{

}

CONST_STDSTR ConnectionPool::getVersion()
{

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

}

int ConnectionPool::reapConnections()
{

}

void* ConnectionPool::doSweep(void* args)//TODO
{

}

#endif