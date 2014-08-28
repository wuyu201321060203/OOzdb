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
    virtual int ping();
    virtual int beginTransaction();
    virtual int commit();
    virtual int rollback();
    virtual long long getLastRowId();
    virtual long long rowsChanged();
    virtual void execute(char const* sql , ...);
    virtual ResultSetPtr executeQuery(char const* sql , ...);
    virtual PreparedStatementPtr getPreparedStatement(char const* sql , ...);
    virtual CONST_STDSTR getLastError();
    virtual void onStop();

public:

    void setAvailable(bool isAvailable);
    bool isAvailable();
    time_t getLastAccessedTime();
    bool isInTransaction();
    void setQueryTimeout(int ms);
    int getQueryTimeout();
    void setMaxRows(int max);
    int getMaxRows();
    URL_T getURL();

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