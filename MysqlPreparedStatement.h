#ifndef MYSQLPREPAREDSTATEMENT_H
#define MYSQLPREPAREDSTATEMENT_H

#include <stdio.h>
#include <string.h>
#include <mysql.h>

#include "ResultSet.h"
#include "MysqlResultSet.h"
#include "PreparedStatementDelegate.h"
#include "MysqlPreparedStatement.h"

#define MYSQL_OK 0

class MysqlPreparedStatement : public PreparedStatement
{
public:
    typedef struct param_t
    {
        union
        {
            int integer;
            long long llong;
            double real;
            MYSQL_TIME timestamp;
        } type;
        long length;
    }param_t;

    MysqlPreparedStatement(void* stmt , int maxRows , int parameterCount);
    ~MysqlPreparedStatement();
    void setString(int parameterIndex , CONST_STDSTR str);
    void setInt(int parameterIndex , int x);
    void setLLong(int parameterIndex, long long x);
    void setDouble(int parameterIndex , double x);
    void setTimestamp(int parameterIndex, time_t x);
    void setBlob(int parameterIndex, const void *x, int size);
    void MysqlPreparedStatement_execute();
    ResultSet_T MysqlPreparedStatement_executeQuery();
    long long rowsChanged();

private:

    int _maxRows;
    int _lastError;
    param_t* _params;
    MYSQL_STMT *_stmt;
    MYSQL_BIND *_bind;
    int _parameterCount;
};

#endif
