#ifndef MYSQLRESULTSET_INCLUDED
#define MYSQLRESULTSET_INCLUDED

#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <errmsg.h>
#include <stdarg.h>

template<typename T>
void THROW(T ExceptionType , char const* cause , ...)
{
    assert(cause);
    char message[EXCEPTION_MESSAGE_LENGTH + 1];
    va_list ap;
    va_start(ap , cause);
    vsnprintf(message , EXCEPTION_MESSAGE_LENGTH , cause , ap);
    va_end(ap);
    throw(ExceptionType(message));
}

class MysqlResultSet : public ResultSet
{
public:

    class column_t
    {
        my_bool is_null;
        MYSQL_FIELD *field;
        unsigned long real_length;
        char *buffer;
    };

    MysqlResultSet(CONST_STDSTR name , void *stmt, int maxRows, int keep);
    virtual ~MysqlResultSet();
    virtual int getColumnCount();
    CONST_STDSTR getColumnName(int columnIndex);
    long getColumnSize(int columnIndex);
    int getColumnCount();
    virtual int next();
    int isnull(int columnIndex);
    virtual CONST_STDSTR getString(int columnIndex);
    void const* getBlob(int columnIndex, int *size);
    virtual int getInt(int columnIndex);
    virtual long long  getLLong(int columnIndex);
    virtual double getDouble(int columnIndex);
    virtual time_t getTimestamp(int columnIndex);//TODO
    virtual struct tm* getDateTime(int columnIndex, struct tm* tm);//TODO
    virtual void clear();

private:

    int _stop;
    int _keep;
    int _maxRows;
    int _lastError;
    int _needRebind;
    int _currentRow;
    int _columnCount;
    MYSQL_RES* _meta;
    MYSQL_BIND* _bind;
    MYSQL_STMT* _stmt;
    column_t* _columns;//??

private:

    inline void ensureCapacity(int i);
};

#endif
