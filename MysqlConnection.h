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
    ~MysqlConnection();
    void setQueryTimeout(int ms);
    void setMaxRows(int max);
    int ping();
    int beginTransaction();
    int commit();
    int rollback();
    long long lastRowId();
    long long rowsChanged();
    int execute(char const* sql , va_list ap);
    ResultSet executeQuery(char const* sql , va_list ap);
    PreparedStatement_T prepareStatement(char const* sql , va_list ap);
    char const* getLastError();
    /* Class method: MySQL client library finalization */
    void onstop(void);

private:

    URL_T _url;
    MYSQL* db;
    int maxRows;
    int timeout;
    int lastError;
    StringBuffer_t sb;

private:

    static MYSQL* doConnect(URL_T url , char **error);
    static int prepare(T C, const char *sql, int len, MYSQL_STMT **stmt);
};

#endif
