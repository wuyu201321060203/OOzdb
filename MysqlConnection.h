#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include <mysql/mysql.h>
#include <mysql/errmsg.h>

#include <boost/enable_shared_from_this.hpp>

#include "StringBuffer.h"
#include "Connection.h"

class MysqlConnection : public Connection,
                        public boost::enable_shared_from_this<MysqlConnection>
{
public:

    MysqlConnection(ConnectionPool* pool , char** error);
    ~MysqlConnection();
    virtual int ping();
    virtual void beginTransaction();
    virtual void commit();
    virtual void rollback();
    virtual long long getLastRowId();
    virtual long long rowsChanged();
    virtual void execute(char const* sql , ...) __attribute__((format (printf, 2, 3)));
    virtual ResultSetPtr executeQuery(char const* sql,
                                    ...)__attribute__((format (printf, 2, 3)));
    virtual PreparedStatementPtr getPrepareStatement(char const* sql,
                                    ...)__attribute__((format (printf, 2, 3)));
    virtual CONST_STDSTR getLastError();
    virtual void close();//non-beautiful

private:

    MYSQL* _db;
    int _lastError;
    StringBuffer _sb;

private:

    static MYSQL* doConnect(URLPtr url , char **error);
    int prepare(char const*sql , int len, MYSQL_STMT **stmt);
};

#endif
