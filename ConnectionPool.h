#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/bind.hpp>

#include <muduo/base/Thread.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Condition.h>
#include <muduo/base/Logging.h>

#include "URL.h"
#include "Connection.h"

using muduo::Thread;
using muduo::MutexLock;
using muduo::Condition;
using muduo::MutexLockGuard;

//typedef boost::function<void (char const*)> AbortHandlerFunc;//TODO
typedef boost::function<void (void)> ThreadFunc;
typedef boost::function<void (void)> StopHandler;
typedef boost::shared_ptr<Thread> ThreadPtr;

#define VERSION "0.1"
#define OOzdb_URL "https://github.com/wuyu201321060203/OOzdb"

#define ABOUT "OOzdb/" VERSION " Copyright (C) Wu Yu" OOzdb_URL

class ConnectionPool : boost::noncopyable
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
 //   void setAbortHandler(AbortHandlerFunc handler);
    void setStopHandler(StopHandler handler);
    void setReaper(int sweepInterval);
    int getSize() const;
    int getActiveConnections();
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
  //  AbortHandlerFunc _abortHandler;
    StopHandler  _stopHandler;
    ThreadPtr _reaper;

private:

    ConnectionVec _connectionsVec;

private:

    void drainPool();
    template<typename ConcreteConnection> int fillPool();
    int doGetActiveConnections();
    int doReapConnections();
    void doSweep();
};

template<typename ConcreteConnection>
void ConnectionPool::start()
{
    BOOST_STATIC_ASSERT(boost::is_base_of<Connection , ConcreteConnection>::value);
    {
        MutexLockGuard lock(_mutex);
        _stopped = false;
        _filled = fillPool<ConcreteConnection>();
        if(_filled && _doSweep)
        {
            LOG_DEBUG << "Starting Database reaper thread\n";
            _reaper->start();//Any question?
        }
    }
    if(_filled)
        setStopHandler(boost::bind(&ConcreteConnection::onStop));
    else
        LOG_FATAL << "Failed to start connection pool";
}

template<typename ConcreteConnection>
int ConnectionPool::fillPool()
{
    BOOST_STATIC_ASSERT(boost::is_base_of<Connection , ConcreteConnection>::value);
    for(int i = 0 ; i != _initialConnections ; ++i)
    {
        ConnectionPtr conn( new ConcreteConnection(this , &_error));
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

typedef boost::shared_ptr<ConnectionPool> ConnectionPoolPtr;

#endif