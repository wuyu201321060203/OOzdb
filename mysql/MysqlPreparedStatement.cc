#include <Exception/SQLException.h>
#include <util/MemoryOperation.h>

#include "MysqlPreparedStatement.h"

static my_bool yes = true;

MysqlPreparedStatement::MysqlPreparedStatement(void* stmt , int maxRows,
                                               int parameterCount):
    PreparedStatement(parameterCount)
{
    assert(stmt);
    _stmt = SC<MYSQL_STMT*>(stmt);
    _maxRows = maxRows;
    if(LIKELY(_parameterCount > 0))
    {
        _params = SC<param_t*>(CALLOC(_parameterCount, sizeof(param_t)));
        _bind = SC<MYSQL_BIND*>(CALLOC(_parameterCount, sizeof(MYSQL_BIND)));
    }
    _lastError = MYSQL_OK;
}

MysqlPreparedStatement::~MysqlPreparedStatement()
{
    clear();
}

void MysqlPreparedStatement::setString(int parameterIndex , CONST_STDSTR str)
{
    int i = checkAndSetParameterIndex(parameterIndex);
    _bind[i].buffer_type = MYSQL_TYPE_STRING;
    _bind[i].buffer = const_cast<char*>( str.c_str() );
    if( UNLIKELY( str.empty() ) )
    {
        _params[i].length = 0;
        _bind[i].is_null = &yes;
    }
    else
    {
        _params[i].length = str.length();
        _bind[i].is_null = 0;
    }
    _bind[i].length = &_params[i].length;
}

void MysqlPreparedStatement::setInt(int parameterIndex , int x)
{
    int i = checkAndSetParameterIndex(parameterIndex);
    _params[i].type.integer = x;
    _bind[i].buffer_type = MYSQL_TYPE_LONG;
    _bind[i].buffer = &_params[i].type.integer;
    _bind[i].is_null = 0;
}

void MysqlPreparedStatement::setLLong(int parameterIndex, long long x)
{
    int i = checkAndSetParameterIndex(parameterIndex);
    _params[i].type.llong = x;
    _bind[i].buffer_type = MYSQL_TYPE_LONGLONG;
    _bind[i].buffer = &_params[i].type.llong;
    _bind[i].is_null = 0;
}

void MysqlPreparedStatement::setDouble(int parameterIndex , double x)
{
    int i = checkAndSetParameterIndex(parameterIndex);
    _params[i].type.real = x;
    _bind[i].buffer_type = MYSQL_TYPE_DOUBLE;
    _bind[i].buffer = &_params[i].type.real;
    _bind[i].is_null = 0;
}

void MysqlPreparedStatement::setBlob(int parameterIndex , void const* x , int size)
{
    int i = checkAndSetParameterIndex(parameterIndex);
    _bind[i].buffer_type = MYSQL_TYPE_BLOB;
    _bind[i].buffer = const_cast<void*>(x);
    if(UNLIKELY(!x))
    {
        _params[i].length = 0;
        _bind[i].is_null = &yes;
    }
    else
    {
        _params[i].length = size;
        _bind[i].is_null = 0;
    }
    _bind[i].length = &_params[i].length;
}

void MysqlPreparedStatement::setTimestamp(int parameterIndex, time_t x)
{
    int i = checkAndSetParameterIndex(parameterIndex);
    struct tm ts = {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , -1 , 0 , 0};
    gmtime_r(&x, &ts);
    _params[i].type.timestamp.year = ts.tm_year + 1900;
    _params[i].type.timestamp.month = ts.tm_mon + 1;
    _params[i].type.timestamp.day = ts.tm_mday;
    _params[i].type.timestamp.hour = ts.tm_hour;
    _params[i].type.timestamp.minute = ts.tm_min;
    _params[i].type.timestamp.second = ts.tm_sec;
    _bind[i].buffer_type = MYSQL_TYPE_TIMESTAMP;
    _bind[i].buffer = &_params[i].type.timestamp;
    _bind[i].is_null = 0;
}

void MysqlPreparedStatement::execute()
{
    if(_resultSet)
        _resultSet->clear();
    if(LIKELY(_parameterCount > 0))
        if( ( _lastError = mysql_stmt_bind_param(_stmt , _bind) ) )
            THROW(SQLException , "%s", mysql_stmt_error(_stmt));
#if MYSQL_VERSION_ID >= 50002
    ULONG cursor = CURSOR_TYPE_NO_CURSOR;
    mysql_stmt_attr_set(_stmt, STMT_ATTR_CURSOR_TYPE, &cursor);
#endif
    if( UNLIKELY(_lastError = mysql_stmt_execute(_stmt) ) )
        THROW(SQLException , "%s", mysql_stmt_error(_stmt));
    if( LIKELY(_lastError == MYSQL_OK) )
    {
        _lastError = mysql_stmt_reset(_stmt);
    }
}

ResultSetPtr MysqlPreparedStatement::executeQuery()
{
    ResultSetPtr ret;
    if(_resultSet)
        _resultSet->clear();
    if( LIKELY(_parameterCount > 0) )
        if((_lastError = mysql_stmt_bind_param(_stmt , _bind)))
            THROW(SQLException , "%s", mysql_stmt_error(_stmt));
#if MYSQL_VERSION_ID >= 50002
    ULONG cursor = CURSOR_TYPE_READ_ONLY;
    mysql_stmt_attr_set(_stmt, STMT_ATTR_CURSOR_TYPE, &cursor);
#endif
    if( UNLIKELY(_lastError = mysql_stmt_execute(_stmt)) )
        THROW(SQLException , "%s", mysql_stmt_error(_stmt));
    else
    {
        ResultSetPtr temp( new MysqlResultSet("MysqlResultSet" , _stmt, _maxRows,
            true));
        ret.swap(temp);
    }
    return ret;
}

long long MysqlPreparedStatement::rowsChanged()
{
    return SC<long long>(mysql_stmt_affected_rows(_stmt));
}

void MysqlPreparedStatement::clear()
{
    FREE(_bind);
    mysql_stmt_free_result(_stmt);
#if MYSQL_VERSION_ID >= 50503
    while(mysql_stmt_next_result(_stmt) == 0);
#endif
    mysql_stmt_close(_stmt);
    FREE(_params);
}