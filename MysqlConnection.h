#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include <mysql.h>
#include <errmsg.h>

#include "StringBuffer.h"
#include "Connection.h"

class MysqlConnection : public Connection
{
public:

    MysqlConnection(ConnectionPoolPtr pool , URL_T url , char** error);
    ~MysqlConnection();
    virtual int ping();
    virtual int beginTransaction();
    virtual int commit();
    virtual int rollback();
    virtual long long getLastRowId();
    virtual long long rowsChanged();
    virtual void execute(char const* sql , ...);
    virtual ResultSetPtr executeQuery(char const* sql , ...);
    virtual PreparedStatement getPrepareStatement(char const* sql , ...);
    virtual CONST_STDSTR getLastError();
    virtual void onstop();

private:

    MYSQL* db;
    int lastError;
    StringBuffer_t sb;

private:

    static MYSQL* doConnect(URL_T url , char **error);
    int prepare(char const*sql , int len, MYSQL_STMT **stmt);
};

#endif
