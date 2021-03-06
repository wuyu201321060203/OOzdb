#ifndef CONNECTION_H
#define CONNECTION_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include <util/TimeOperation.h>
#include <Net/URL.h>

#include "ResultSet.h"
#include "PreparedStatement.h"

namespace OOzdb
{

class ConnectionPool;

class Connection
{
public:

    Connection(ConnectionPool* pool);
    virtual ~Connection();
    virtual int ping();
    virtual void beginTransaction();
    virtual void commit();
    virtual void rollback();
    virtual long long getLastRowId();
    virtual long long rowsChanged();
    virtual void execute(char const* sql ,  ...) __attribute__((format (printf, 2, 3)));//第2个参数
    //是格式字符串，从第3个参数开始是可变参数列表，从这按照printf检查要求进行检查,第一个参数是this指针
    virtual ResultSetPtr executeQuery(char const* sql,
                                      ...)__attribute__((format (printf, 2, 3)));
    virtual PreparedStatementPtr getPreparedStatement(char const* sql,
                                        ...)__attribute__((format (printf, 2, 3)));

    virtual CONST_STDSTR getLastError();
    virtual void close();

    /*
     *The following functions are inherited by subclasses directly
     */

public:

    void setAvailable(bool available);
    bool isAvailable();
    time_t getLastAccessedTime();
    bool isInTransaction();
    void setQueryTimeout(int ms);
    int getQueryTimeout();
    void setMaxRows(int rows);
    int getMaxRows();
    URLPtr getURL();
    void clear();

protected:

    ConnectionPool* _pool;
    bool _isAvailable;
    int _isInTransaction;
    int _timeout;
    URLPtr _url;
    time_t _lastAccessedTime;
    int _maxRows;
    //PreparedStatementVec _prepared;
    ResultSetPtr _resultSet;

private:

    void freePrepared();
};

typedef boost::shared_ptr<Connection> ConnectionPtr;
typedef std::vector<ConnectionPtr> ConnectionVec;

}

#endif