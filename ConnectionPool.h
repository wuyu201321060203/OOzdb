#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <algorithm>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

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
typedef boost::function<void (void)> StopHandlerFunc;
typedef boost::shared_ptr<Thread> ThreadPtr;

#define VERSION 0.1
#define OOzdb_URL "https://github.com/wuyu201321060203/OOzdb"

#define ABOUT "OOzdb/" #VERSION " Copyright (C) Wu Yu" OOzdb_URL

class ConnectionPool : boost::noncopyable , public boost::enable_shared_from_this<ConnectionPool>
{
public:

    ConnectionPool(char const* url);
    ~ConnectionPool();
    URLPtr getURL();
    void setInitialConnections(int connections);
    int getInitialConnections() const;
    void setMaxConnections(int maxConnections);
    int getMaxConnections() const;
    void setConnectionTimeout(int connectionTimeout);
    int getConnectionTimeout() const;
    void setAbortHandler(AbortHandlerFunc handler);
    void setStopHandler(StopHandlerFunc handler);
    void setReaper(int sweepInterval);
    int getSize() const;
    int getActiveConnections() const;
    template<typename ConcreteConnection> void start();
    void stop();
    ConnectionPtr getConnection();
    void returnConnection(ConnectionPtr conn);
    int reapConnections();
    CONST_STDSTR getVersion() const;

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
    AbortHandlerFunc _abortHandler;
    StopHandlerFunc  _stopHandler;
    ThreadPtr _reaper;

private:

    ConnectionVec _connectionsVec;

private:

    void drainPool();
    template<typename ConcreteConnection> int fillPool();
    int getActive();
    int onReapConnections();
    virtual void doSweep();
};

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

#endif