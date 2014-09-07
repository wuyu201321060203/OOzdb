#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include <mysql.h>
#include <errmsg.h>

#include "StringBuffer.h"
#include "Connection.h"

class MysqlConnection : public Connection
{
public:

    MysqlConnection(ConnectionPoolPtr pool , URLPtr url , char** error);
    ~MysqlConnection();
    virtual int ping();
    virtual int beginTransaction();
    virtual int commit();
    virtual int rollback();
    virtual long long getLastRowId();
    virtual long long rowsChanged();
    virtual void execute(char const* sql , ...) __attribute__((format (printf, 1, 2)));
    virtual ResultSetPtr executeQuery(char const* sql,
                                    ...)__attribute__((format (printf, 1, 2)));
    virtual PreparedStatement getPrepareStatement(char const* sql,
                                    ...)__attribute__((format (printf, 1, 2)));
    virtual CONST_STDSTR getLastError();
    virtual void onstop();

private:

    MYSQL* db;
    int lastError;
    StringBuffer sb;

private:

    static MYSQL* doConnect(URLPtr url , char **error);
    int prepare(char const*sql , int len, MYSQL_STMT **stmt);
};

#endif
