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
    virtual ~MysqlPreparedStatement();
    virtual void setString(int parameterIndex , CONST_STDSTR str);
    virtual void setInt(int parameterIndex , int x);
    virtual void setLLong(int parameterIndex, long long x);
    virtual void setDouble(int parameterIndex , double x);
    virtual void setTimestamp(int parameterIndex, time_t x);
    virtual void setBlob(int parameterIndex, const void *x, int size);
    virtual void execute();
    virtual ResultSetPtr executeQuery();
    virtual long long rowsChanged();

private:

    int _maxRows;
    int _lastError;
    param_t* _params;
    MYSQL_STMT *_stmt;
    MYSQL_BIND *_bind;
};

#endif
