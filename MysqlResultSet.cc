#include <cassert>
#include <mysql.h>
#include <stdlib.h>

#include "Config.h"

#include "MysqlResultSet.h"
#include "Str.h"
#include "Memory.h"
#include "Time.h

#define MYSQL_OK 0
#define EXCEPTION_MESSAGE_LENGTH 512
#define BADSTR "null"

#define FREE(p) (if(p) free(p);)

MysqlResultSet::MysqlResultSet(CONST_STDSTR name , void* stmt , int maxRows,
                               int keep):
    ResultSet(name)
{
    assert(stmt);
    _stmt = stmt;
    _keep = keep;
    _maxRows = maxRows;
    _columnCount = mysql_stmt_field_count(_stmt);
    if( (_columnCount <= 0) || !(_meta = mysql_stmt_result_metadata(_stmt) ) )
    {
        LOG_DEBUG << "Warning: column error - " << (mysql_stmt_error(stmt));
        _stop = true;
    }
    else
    {
        _bind = CALLOC(_columnCount, sizeof MYSQL_BIND);
        _columns = CALLOC(_columnCount, sizeof(struct column_t));
        for(int i = 0; i < _columnCount; i++)
        {
            _columns[i].buffer = ALLOC(STRLEN + 1);
            _bind[i].buffer_type = MYSQL_TYPE_STRING;
            _bind[i].buffer = _columns[i].buffer;
            _bind[i].buffer_length = STRLEN;
            _bind[i].is_null = &_columns[i].is_null;
            _bind[i].length = &_columns[i].real_length;
            _columns[i].field = mysql_fetch_field_direct(_meta, i);
        }
        if((_lastError = mysql_stmt_bind_result(_stmt, _bind)))
        {
            LOG_DEBUG << "Warning: bind error - " << (mysql_stmt_error(stmt));
            _stop = true;
        }
    }
}

MysqlResultSet::~MysqlResultSet()
{
    clear();
}

CONST_STDSTR MysqlResultSet::getColumnName(int columnIndex)
{
    --columnIndex;
    if( _columnCount <= 0 || columnIndex < 0 || columnIndex > _columnCount)
        return BADSTR;
    return CONST_STDSTR(_columns[columnIndex].field->name);
}

long MysqlResultSet::getColumnSize(int columnIndex)
{
    int i = checkAndSetColumnIndex(columnIndex);
    if (_columns[i].is_null)
            return 0;
    return _columns[i].real_length;
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
            THROW(SQLException, "mysql_stmt_bind_result -- %s",
                mysql_stmt_error(_stmt));
        _needRebind = false;
    }
    _lastError = mysql_stmt_fetch(_stmt);
    if (_lastError == 1)
        THROW(SQLException, "mysql_stmt_fetch -- %s", mysql_stmt_error(_stmt));
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
    return CONST_STDSTR(_columns[i].buffer);
}

void const* MysqlResultSet::getBlob(int columnIndex, int* size)
{
    int i = checkAndSetColumnIndex(columnIndex);
    if (_columns[i].is_null)
        return NULL;
    ensureCapacity(i);
    *size = (int)_columns[i].real_length;
    return _columns[i].buffer;
}

int MysqlResultSet::getInt(int columnIndex)
{
    CONST_STDSTR s = getString(columnIndex);
    return s ? strParseInt(s.c_str()) : 0;
}

long long  MysqlResultSet::getLLong(int columnIndex);
{
    CONST_STDSTR s = getString(columnIndex);
    return s ? strParseLLong(s.c_str()) : 0;
}

double MysqlResultSet::getDouble(int columnIndex);
{
    CONST_STDSTR s = getString(columnIndex);
    return s ? strParseDouble(s.c_str()) : 0.0;
}

time_t MysqlResultSet::getTimestamp(int columnIndex)
{
    time_t t = 0;
    CONST_STDSTR s = getString(columnIndex);
    if(!s.empty()) t = Time_toTimestamp(s.c_str());
    return t;
}

struct tm* MysqlResultSet::getDateTime(int columnIndex, struct tm* tm)
{
    CONST_STDSTR s = getString(columnIndex);
    struct tm t = {.tm_year = 0};
    if(!s.empty()) t = Time_toDateTime(s, &t);
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
    FREE(_columns);
    FREE(_bind);
}

void MysqlResultSet::ensureCapacity(int i)
{
    if(_columns[i].real_length > _bind[i].buffer_length)
    {
        RESIZE(_columns[i].buffer , _columns[i].real_length + 1);
        _bind[i].buffer = _columns[i].buffer;
        _bind[i].buffer_length = _columns[i].real_length;
        if ((_lastError = mysql_stmt_fetch_column(_stmt, &_bind[i], i, 0)))
            THROW(SQLException, "mysql_stmt_fetch_column -- %s", mysql_stmt_error(_stmt));//TODO
        _needRebind = true;
    }
}

int MysqlResultSet::checkAndSetColumnIndex(int columnIndex)
{
    int i = columnIndex - 1;
    if (_columnCount <= 0 || i < 0 || i >= _columnCount)
        THROW(SQLException, "Column index is out of range");
    return i;
}