#ifndef MYSQLRESULTSET_INCLUDED
#define MYSQLRESULTSET_INCLUDED

#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <errmsg.h>
#include <stdarg.h>

#define MYSQL_OK 0
#define EXCEPTION_MESSAGE_LENGTH 512

#define FREE(p) (if(p) free(p);)

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

    MysqlResultSet(CONST_STDSTR name , void *stmt, int maxRows, int keep):
        ResultSet(name)
    {
        assert(stmt);
        _stmt = stmt;
        _keep = keep;
        _maxRows = maxRows;
        _columnCount = mysql_stmt_field_count(_stmt);
        if ( (_columnCount <= 0) || ! (_meta = mysql_stmt_result_metadata(_stmt) ) )
        {
            LOG_DEBUG << "Warning: column error - " << (mysql_stmt_error(stmt));
            _stop = true;
        }
        else
        {
            _bind = calloc(_columnCount, sizeof (MYSQL_BIND));
            if(!_bind)
                THROW(MemoryException , "Get memory error");
            _columns = calloc(_columnCount, sizeof (struct column_t));
            for (int i = 0; i < _columnCount; i++) {
                _columns[i].buffer = ALLOC(STRLEN + 1);
                _bind[i].buffer_type = MYSQL_TYPE_STRING;
                _bind[i].buffer = _columns[i].buffer;
                _bind[i].buffer_length = STRLEN;
                _bind[i].is_null = &_columns[i].is_null;
                _bind[i].length = &_columns[i].real_length;
                _columns[i].field = mysql_fetch_field_direct(_meta, i);
            }
            if ((_lastError = mysql_stmt_bind_result(_stmt, _bind)))
            {
                LOG_DEBUG << "Warning: bind error - " << (mysql_stmt_error(stme));
                _stop = true;
            }
        }
    }

    virtual ~MysqlResultSet()
    {
        for (int i = 0; i < (*R)->columnCount; i++)
            FREE(_columns[i].buffer);
        mysql_stmt_free_result(_stmt);
        if (_keep == false)
            mysql_stmt_close(_stmt);
        if (_meta)
            mysql_free_result(_meta);
        FREE(_columns);
        FREE(_bind);
    }

    virtual int getColumnCount()
    {
        return _columnCount;
    }

    CONST_STDSTR getColumnName(int columnIndex)
    {
        --columnIndex;
        if (_columnCount <= 0 || columnIndex < 0 || columnIndex > _columnCount)
            return "";
        return CONST_STDSTR(_columns[columnIndex].field->name);
    }

    long getColumnSize(int columnIndex)
    {
        int i = checkAndSetColumnIndex(columnIndex);
        if (_columns[i].is_null)
                return 0;
        return _columns[i].real_length;
    }

    int getColumnCount()
    {
        return _columnCount;
    }

    virtual int next()
    {
        if (_stop)
            return false;
        if (_maxRows && (_currentRow++ >= _maxRows))
        {
            _stop = true;
#if MYSQL_VERSION_ID >= 50002
            mysql_stmt_reset(R->stmt);
#else
            while (mysql_stmt_fetch(R->stmt) == 0);
#endif
            return false;
        }
        if (_needRebind)
        {
            if ((_lastError = mysql_stmt_bind_result(_stmt, _bind)))
                THROW(SQLException, "mysql_stmt_bind_result -- %s",
                    mysql_stmt_error(_stmt));
            _needRebind = false;
        }
        _lastError = mysql_stmt_fetch(_stmt);
        if (_lastError == 1)
            THROW(SQLException, "mysql_stmt_fetch -- %s", mysql_stmt_error(_stmt));
        return ((_lastError == MYSQL_OK) || (_lastError == MYSQL_DATA_TRUNCATED));
    }

    int isnull(int columnIndex)
    {
        int i = checkAndSetColumnIndex(columnIndex, _columnCount);
        return _columns[i].is_null;
    }

    virtual CONST_STDSTR getString(int columnIndex)
    {
        int i = checkAndSetColumnIndex(columnIndex , _columnCount);
        if (_columns[i].is_null)
            return NULL;
        ensureCapacity(i);
        _columns[i].buffer[_columns[i].real_length] = 0;
        return _columns[i].buffer;
    }

    void const* getBlob(int columnIndex, int *size)
    {
        int i = checkAndSetColumnIndex(columnIndex , _columnCount);
        if (_columns[i].is_null)
            return NULL;
        ensureCapacity(i);
        *size = (int)_columns[i].real_length;
        return _columns[i].buffer;
    }

    virtual int getInt(int columnIndex);
    virtual long long  getLLong(int columnIndex);
    virtual double getDouble(int columnIndex);
    virtual void const* getBlob(int columnIndex , int* size);
    void const* getBlobByName(CONST_STDSTR name , int* size);
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

    inline void ensureCapacity(int i)
    {
        if(_columns[i].real_length > _bind[i].buffer_length)
        {
            RESIZE(_columns[i].buffer , _columns[i].real_length + 1);
            _bind[i].buffer = _columns[i].buffer;
            _bind[i].buffer_length = _columns[i].real_length;
            if ((_lastError = mysql_stmt_fetch_column(_stmt, &_bind[i], i, 0)))
                THROW(SQLException, "mysql_stmt_fetch_column -- %s", mysql_stmt_error(R->stmt));
            _needRebind = true;
        }
    }
};

#endif
