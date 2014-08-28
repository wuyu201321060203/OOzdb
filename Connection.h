#ifndef CONNECTION_H
#define CONNECTION_H

#include <cstdarg>
#include <cassert>
#include <vector>

#include <boost/enable_shared_from_this.hpp>

#include "ConnectionPool.h"
#include "ResultSet.h"
#include "PreparedStatement.h"
#include "URL.h"

class Connection
{
public:

    Connection(ConnectionPoolPtr pool);
    virtual ~Connection();
    void setAvailable(bool isAvailable);
    bool isAvailable();
    time_t getLastAccessedTime();
    bool isInTransaction();
    virtual void setQueryTimeout(int ms);
    int getQueryTimeout();
    virtual void setMaxRows(int max);
    int getMaxRows();
    URL_T getURL();
    virtual int ping();
    virtual int beginTransaction();
    virtual void clear();
    virtual void close();
    virtual int commit();
    virtual int rollback();
    virtual long long getLastRowId();
    virtual long long rowsChanged();
    virtual void execute(CONST_STDSTR sql , ...);
    virtual ResultSetPtr executeQuery(CONST_STDSTR sql , ...);
    virtual PreparedStatementPtr getPreparedStatement(CONST_STDSTR sql , ...);
    virtual CONST_STDSTR getLastError();
    virtual void onStop();

private:

    ConnectionPoolPtr _pool;
    bool _isAvailable;
    bool _isInTransaction;
    int _timeout;
    URL_T _url;
    time_t _lastAccessedTime;
    int _maxRows;
    PreparedStatementVec _prepared;
    ResultSetPtr _resultSet;

private:

    void freePrepared();
};

typedef std::vector<ConnectionPtr> ConnectionVec;

#endif