#include <cassert>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>

#include "Config.h"

#include "MysqlResultSet.h"
#include "StrOperation.h"
#include "MemoryOperation.h"
#include "SQLException.h"

MysqlResultSet::MysqlResultSet(CONST_STDSTR name , void* stmt , int maxRows,
                               int keep):
    ResultSet(name)
{
    assert(stmt);
    _stmt = static_cast<MYSQL_STMT*>(stmt);
    _keep = keep;
    _maxRows = maxRows;
    _columnCount = mysql_stmt_field_count(_stmt);
    if( (_columnCount <= 0) || !(_meta = mysql_stmt_result_metadata(_stmt) ) )
    {
        LOG_DEBUG << "Warning: column error - " << (mysql_stmt_error(_stmt));
        _stop = true;
    }
    else
    {
        _bind = static_cast<MYSQL_BIND*>( CALLOC(_columnCount , sizeof(MYSQL_BIND)) );
        ColumnVec temp(_columnCount);
        _columns.swap(temp);//C++11
        for(int i = 0; i < _columnCount; i++)
        {
            _columns[i].buffer = static_cast<char*>(ALLOC(STRLEN + 1));
            _bind[i].buffer_type = MYSQL_TYPE_STRING;
            _bind[i].buffer = _columns[i].buffer;
            _bind[i].buffer_length = STRLEN;
            _bind[i].is_null = &_columns[i].is_null;
            _bind[i].length = &_columns[i].real_length;
            _columns[i].field = mysql_fetch_field_direct(_meta, i);
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
    return _columns[i].is_null;
}

CONST_STDSTR MysqlResultSet::getString(int columnIndex)
{
    int i = checkAndSetColumnIndex(columnIndex);
    if (_columns[i].is_null)
        return BADSTR;
    ensureCapacity(i);
    _columns[i].buffer[_columns[i].real_length] = 0;
    return STDSTR(_columns[i].buffer);
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
    if (_columns[i].is_null)
    {
        *size = 0;
        return NULL;
    }
    ensureCapacity(i);
    *size = static_cast<int>(_columns[i].real_length);
    return _columns[i].buffer;
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
    for (int i = 0; i < _columnCount; i++)
        FREE(_columns[i].buffer);
    mysql_stmt_free_result(_stmt);
    if (_keep == false)
        mysql_stmt_close(_stmt);
    if (_meta)
        mysql_free_result(_meta);
    FREE(_bind);
}

CONST_STDSTR MysqlResultSet::getColumnName(int columnIndex)
{
    --columnIndex;
    if( _columnCount <= 0 || columnIndex < 0 || columnIndex > _columnCount)
        return BADSTR;
    return STDSTR(_columns[columnIndex].field->name);
}

long MysqlResultSet::getColumnSize(int columnIndex)
{
    int i = checkAndSetColumnIndex(columnIndex);
    if (_columns[i].is_null)
            return 0;
    return _columns[i].real_length;
}

void MysqlResultSet::ensureCapacity(int i)
{
    if(_columns[i].real_length > _bind[i].buffer_length)
    {
        //ugly realization
        void* temp = NULL;
        temp = CALLOC(1 , _columns[i].real_length + 1);
        memcpy(temp , _columns[i].buffer , _bind[i].buffer_length);
        free(_columns[i].buffer);
        _columns[i].buffer = static_cast<char*>(temp);
        //TODO
        _bind[i].buffer = _columns[i].buffer;
        _bind[i].buffer_length = _columns[i].real_length;
        if ((_lastError = mysql_stmt_fetch_column(_stmt, &_bind[i], i, 0)))
            THROW(SQLException , "mysql_stmt_fetch_column -- %s",
                                                        mysql_stmt_error(_stmt));
        _needRebind = true;
    }
}