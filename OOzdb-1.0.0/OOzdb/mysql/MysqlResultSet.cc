#include <cassert>
#include <stdlib.h>
#include <stdarg.h>
#include <strings.h>
#include <mysql/errmsg.h>

#include <util/StrOperation.h>
#include <util/MemoryOperation.h>
#include <Exception/SQLException.h>

#include "MysqlResultSet.h"

//#include <iostream>

using namespace OOzdb;

MysqlResultSet::MysqlResultSet(CONST_STDSTR name , void* stmt , int maxRows,
                               int keep):
    ResultSet(name) , _stop(false) , _needRebind(false) , _currentRow(0)
{
    assert(stmt);
    _stmt = SC<MYSQL_STMT*>(stmt);
    _keep = keep;
    _maxRows = maxRows;
    _columnCount = mysql_stmt_field_count(_stmt);
    if( UNLIKELY((_columnCount <= 0) ||
        !(_meta = mysql_stmt_result_metadata(_stmt)) ))
    {
        LOG_DEBUG << "Warning: column error - " << (mysql_stmt_error(_stmt));
        _stop = true;
    }
    else
    {
        _bind = SC<MYSQL_BIND*>( CALLOC(_columnCount , sizeof(MYSQL_BIND)) );
        ColumnVec temp(_columnCount);
        _columns.swap(temp);
        for(int i = 0 ; i != _columnCount ; ++i)
        {
            _columns[i]._buffer = SC<char*>(ALLOC(STRLEN + 1));
            bzero(_columns[i]._buffer , STRLEN + 1);
            _bind[i].buffer_type = MYSQL_TYPE_STRING;
            _bind[i].buffer = _columns[i]._buffer;
            _bind[i].buffer_length = STRLEN;
            _bind[i].is_null = &_columns[i]._is_null;
            _bind[i].length = &_columns[i]._real_length;
            _columns[i]._field = mysql_fetch_field_direct(_meta, i);
        }
        if((_lastError = mysql_stmt_bind_result(_stmt, _bind)))
        {
            LOG_DEBUG << "Warning: bind error - " << (mysql_stmt_error(_stmt));
            _stop = true;
        }
    }
}

MysqlResultSet::~MysqlResultSet()
{
    clear();
}

int MysqlResultSet::next()
{
    if(_stop)
        return false;
    if(_maxRows && (_currentRow++ >= _maxRows))
    {
        _stop = true;
#if MYSQL_VERSION_ID >= 50002
        mysql_stmt_reset(_stmt);
#else
        while(mysql_stmt_fetch(_stmt) == 0);
#endif
        return false;
    }
    if(_needRebind)
    {
        if((_lastError = mysql_stmt_bind_result(_stmt, _bind)))
            THROW(SQLException , "mysql_stmt_bind_result -- %s",
                mysql_stmt_error(_stmt));
        _needRebind = false;
    }
    _lastError = mysql_stmt_fetch(_stmt);
    if (_lastError == 1)
        THROW(SQLException , "mysql_stmt_fetch -- %s", mysql_stmt_error(_stmt));
    return ((_lastError == MYSQL_OK) || (_lastError == MYSQL_DATA_TRUNCATED));
}

int MysqlResultSet::isnull(int columnIndex)
{
    int i = checkAndSetColumnIndex(columnIndex);
    return _columns[i]._is_null;
}

CONST_STDSTR MysqlResultSet::getString(int columnIndex)
{
    int i = checkAndSetColumnIndex(columnIndex);
    if (_columns[i]._is_null)
        return BADSTR;
    ensureCapacity(i);
    _columns[i]._buffer[_columns[i]._real_length] = 0;
    return STDSTR(_columns[i]._buffer);
}

int MysqlResultSet::getInt(int columnIndex)
{
    CONST_STDSTR s = getString(columnIndex);
    return s.empty() ?  0 : strParseInt(s.c_str());
}

long long MysqlResultSet::getLLong(int columnIndex)
{
    CONST_STDSTR s = getString(columnIndex);
    return s.empty() ? 0 : strParseLLong(s.c_str());
}

double MysqlResultSet::getDouble(int columnIndex)
{
    CONST_STDSTR s = getString(columnIndex);
    return s.empty() ? 0.0 : strParseDouble(s.c_str());
}

void const* MysqlResultSet::getBlob(int columnIndex, int* size)
{
    int i = checkAndSetColumnIndex(columnIndex);
    if (_columns[i]._is_null)
    {
        *size = 0;
        return NULL;
    }
    ensureCapacity(i);
    *size = SC<int>(_columns[i]._real_length);
    return _columns[i]._buffer;
}

time_t MysqlResultSet::getTimestamp(int columnIndex)
{
    time_t t = 0;
    CONST_STDSTR s = getString(columnIndex);
    if(!s.empty()) t = Time_toTimestamp(s.c_str());
    return t;
}

struct tm MysqlResultSet::getDateTime(int columnIndex)
{
    CONST_STDSTR s = getString(columnIndex);
    struct tm t = {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0};
    if(!s.empty()) Time_toDateTime(s.c_str(), &t);
    return t;
}

void MysqlResultSet::clear()
{
    if(!isCleared())
    {
        for(int i = 0; i < _columnCount; i++)
            FREE(_columns[i]._buffer);
        mysql_stmt_free_result(_stmt);
        if(_keep == false)
        {
            mysql_stmt_close(_stmt);//dangerous
        }
        if(_meta)
            mysql_free_result(_meta);//dangerous
        FREE(_bind);
        setClearFlag();
    }
}

CONST_STDSTR MysqlResultSet::getColumnName(int columnIndex)
{
    int i = checkAndSetColumnIndex(columnIndex);
    return STDSTR(_columns[i]._field->name);
}

long MysqlResultSet::getColumnSize(int columnIndex)
{
    int i = checkAndSetColumnIndex(columnIndex);
    if (_columns[i]._is_null)
            return 0;
    return _columns[i]._real_length;
}

void MysqlResultSet::ensureCapacity(int i)
{
    if(_columns[i]._real_length > _bind[i].buffer_length)
    {
        _columns[i]._buffer = SC<char*>(RESIZE(SC<void*>(_columns[i]._buffer) ,
                                _columns[i]._real_length + 1));
        bzero(_columns[i]._buffer , _columns[i]._real_length + 1);
        _bind[i].buffer = _columns[i]._buffer;
        _bind[i].buffer_length = _columns[i]._real_length;
        if ((_lastError = mysql_stmt_fetch_column(_stmt, &_bind[i], i, 0)))
            THROW(SQLException , "mysql_stmt_fetch_column -- %s",
                                                        mysql_stmt_error(_stmt));
        _needRebind = true;
    }
}