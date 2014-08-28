#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <errmsg.h>

#include "URL.h"
#include "ResultSet.h"
#include "StringBuffer.h"
#include "PreparedStatement.h"
#include "MysqlResultSet.h"
#include "MysqlPreparedStatement.h"
#include "ConnectionDelegate.h"
#include "MysqlConnection.h"

#define MYSQL_OK 0

class MysqlConnection : public Connection
{
public:

    MysqlConnection(URL_T url , char** error);
    virtual ~MysqlConnection();
    virtual void setQueryTimeout(int ms);
    virtual void setMaxRows(int max);
    virtual int ping();
    virtual int beginTransaction();
    virtual void clear();
    virtual void close();
    virtual int commit();
    virtual int rollback();
    virtual long long getLastRowId();
    virtual long long rowsChanged();
    virtual void execute(char const* sql , va_list ap);
    virtual ResultSetPtr executeQuery(char const* sql , va_list ap);
    virtual PreparedStatement getPrepareStatement(char const* sql , va_list ap);
    virtual CONST_STDSTR getLastError();
    /* Class method: MySQL client library finalization */
    virtual void onstop();

private:

    URL_T _url;
    MYSQL* db;
    int lastError;
    StringBuffer_t sb;

private:

    static MYSQL* doConnect(URL_T url , char **error);
    static int prepare(T C, const char *sql, int len, MYSQL_STMT **stmt);
};

#endif
