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

#include <Net/URL.h>

#include "Connection.h"

using muduo::Thread;
using muduo::MutexLock;
using muduo::Condition;
using muduo::MutexLockGuard;

//typedef boost::function<void (char const*)> AbortHandlerFunc;//TODO
typedef boost::function<void (void)> ThreadFunc;
typedef boost::function<void (void)> StopHandler;
typedef boost::shared_ptr<Thread> ThreadPtr;

#define VERSION "1.0"
#define OOzdb_URL "https://github.com/wuyu201321060203/OOzdb"

#define ABOUT "OOzdb/" VERSION " Copyright (C) Wu Yu" OOzdb_URL

namespace OOzdb
{

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
    template<typename ConcreteConnection> ConnectionPtr getConnection();
    void returnConnection(ConnectionPtr conn);
    int reapConnections();
    CONST_STDSTR getVersion() const;
    int getSweepInterval() const;
#ifdef TEST
    bool isFilled() const;
    bool needDoSweep() const;
    ThreadPtr getReaper();
#endif

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
void ConnectionPool::start()//Best Practice:each ConnectionPool only start once
{
    BOOST_STATIC_ASSERT(boost::is_base_of<Connection , ConcreteConnection>::value);
    {
        MutexLockGuard lock(_mutex);
        _stopped = false;
        _filled = fillPool<ConcreteConnection>();
        if(_filled && _doSweep)
        {
            LOG_DEBUG << "Starting Database reaper thread\n";
            _reaper.reset(new Thread(boost::bind(&ConnectionPool::doSweep , this)));
            _reaper->start();
        }
    }
    if(LIKELY(_filled))
        setStopHandler(boost::bind(&ConcreteConnection::onStop));
    else
        LOG_FATAL << "Failed to start connection pool";
}

template<typename ConcreteConnection>
ConnectionPtr ConnectionPool::getConnection()
{
    ConnectionPtr conn;
    {
        MutexLockGuard lock(_mutex);
        int size = _connectionsVec.size();
        for( int i = 0; i != size ; ++i )
        {
            ConnectionPtr temp(_connectionsVec.at(i));
            if(temp->isAvailable() && temp->ping())
            {
                temp->setAvailable(false);
                conn.swap(temp);
                goto done;
            }
        }
        if(size <= _maxConnections)
        {
            ConnectionPtr temp(new ConcreteConnection(this , &_error));
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

template<typename ConcreteConnection>
int ConnectionPool::fillPool()
{
    BOOST_STATIC_ASSERT(boost::is_base_of<Connection , ConcreteConnection>::value);
    for(int i = 0 ; i != _initialConnections ; ++i)
    {
        ConnectionPtr conn( new ConcreteConnection(this , &_error));
        if(UNLIKELY(!conn))
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

}

#endif
