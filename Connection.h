#ifndef CONNECTION_H
#define CONNECTION_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include <muduo/base/Timestamp.h>

#include "Config.h"
#include "ConnectionPool.h"
#include "ResultSet.h"
#include "PreparedStatement.h"
#include "URL.h"

class Connection
{
public:

    /*
     * The following functions are virtual functions and must be implemented by
     * subclass
     */

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

    /*
     *The following functions are inherited by subclasses directly
     */

public:

    void setAvailable(bool isAvailable);
    bool isAvailable();
    time_t getLastAccessedTime();
    bool isInTransaction();
    void setQueryTimeout(int ms);
    int getQueryTimeout();
    void setMaxRows(int rows);
    int getMaxRows();
    URL_T getURL();
    void clear();
    void close();

protected:

    ConnectionPoolPtr _pool;
    bool _isAvailable;
    int _isInTransaction;
    int _timeout;
    URL_T _url;
    time_t _lastAccessedTime;
    int _maxRows;
    PreparedStatementVec _prepared;
    ResultSetPtr _resultSet;

private:

    void freePrepared();
};

typedef boost::shared_ptr<Connection> ConnectionPtr;
typedef std::vector<ConnectionPtr> ConnectionVec;

#endif