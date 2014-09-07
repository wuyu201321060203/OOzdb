#include <cstdarg>
#include <cassert>

#include "MysqlConnection.h"

MysqlConnection::MysqlConnection(ConnectionPoolPtr pool , char** error):
    Connection(pool),
    _lastError(MYSQL_OK)
{
    assert(error);
    if(!(_db = doConnect(_url, error)))
        THROW(SQLException("Can't connect to MySQL"));//TODO
    _sb = StringBuffer_create(STRLEN);
}

MysqlConnection::~MysqlConnection()
{
    mysql_close(_db);
    StringBuffer_free(&_sb);
}

int MysqlConnection::ping()
{
    return (mysql_ping(_db) == 0);
}

int MysqlConnection::beginTransaction()
{
    _lastError = mysql_query(_db, "START TRANSACTION;");
    if (_lastError == MYSQL_OK)
    {
        ++_isInTransaction;
        return true;
    }
    THROW(SQLException);//TODO
}

int MysqlConnection::commit()
{
    if(_isInTransaction)
        _isInTransaction = 0;
    _lastError = mysql_query(_db, "COMMIT;");
    if(_lastError == MYSQL_OK)
        return true;
    else
        THROW(SQLException());//TODO
}

int MysqlConnection::rollback()
{
    if(_isInTransaction)
    {
        _isInTransaction = 0;
        Clear();
    }
    _lastError = mysql_query(db, "ROLLBACK;");
    if(_lastError == MYSQL_OK)
        return true;
    else
        THROW();//TODO
}

long long MysqlConnection::getLastRowId()
{
    return (long long)mysql_insert_id(_db);
}

long long MysqlConnection::rowsChanged()
{
    return (long long)mysql_affected_rows(_db);
}

int MysqlConnection::execute(char const* sql , ...)
{
    assert(sql);
    va_list ap;
    va_start(ap , sql);
    _resultSet->clear();
    StringBuffer_vset(_sb, sql, ap);
    va_end(ap);
    _lastError = mysql_real_query(_db , StringBuffer_toString(_sb),
                                  StringBuffer_length(_sb));
    if(_lastError == MYSQL_OK)
        return true;
    else
        THROW();//TODO
}

ResultSetPtr MysqlConnection::executeQuery(char const* sql , ...)
{
    assert(sql);
    _resultSet->clear();
    va_list ap;
    va_start(ap , sql);
    MYSQL_STMT* stmt = NULL;
    StringBuffer_vset(_sb , sql , ap);
    va_end(ap);
    if(prepare(StringBuffer_toString(_sb) , StringBuffer_length(_sb) , &stmt))
    {
#if MYSQL_VERSION_ID >= 50002
        unsigned long cursor = CURSOR_TYPE_READ_ONLY;
        mysql_stmt_attr_set(_stmt, STMT_ATTR_CURSOR_TYPE, &cursor);
#endif
        if ((_lastError = mysql_stmt_execute(_stmt)))
        {
            StringBuffer_set(_sb, "%s", mysql_stmt_error(_stmt));
            mysql_stmt_close(_stmt);
        }
        else
            return ResultSetPtr(new MysqlResultSet("MysqlResultSet" , _stmt , _maxRows,
                                                    false));
    }
    THROW();//TODO
}

PreparedStatementPtr MysqlConnection::getPrepareStatement(char const* sql , ...)
{
    assert(sql);
    va_list ap;
    va_start(ap , sql);
    MYSQL_STMT *stmt = NULL;
    StringBuffer_vset(_sb , sql , ap);
    va_end(ap);
    if (prepare(StringBuffer_toString(_sb) , StringBuffer_length(_sb) , &stmt))
    {
        int parameterCount = (int)mysql_stmt_param_count(_stmt);
        PreparedStatementPtr item(new MysqlPreparedStatement(stmt , _maxRows,
                                                               parameterCount) );
        _prepared.push_back(item);
        return item;
    }
    THROW();//TODO
}

CONST_STDSTR MysqlConnection::getLastError()
{
    if(mysql_errno(_db))
        return CONST_STDSTR(mysql_error(_db));
    return CONST_STDSTR( StringBuffer_toString(_sb) ); // Either the statement itself or a statement error
}

/* Class method: MySQL client library finalization */
void MysqlConnection::onstop()
{
    if (mysql_get_client_version() >= 50003)
        mysql_library_end();
    else
        mysql_server_end();
}

MYSQL* MysqlConnection::doConnect(URLPtr url , char **error)
{
#define ERROR(e) do {*error = Str_dup(e); goto error;} while (0)
    int port;
    my_bool yes = 1;
    my_bool no = 0;
    int connectTimeout = SQL_DEFAULT_TCP_TIMEOUT;
    unsigned long clientFlags = CLIENT_MULTI_STATEMENTS;
    char const *user , *password , *host , *database , *charset , *timeout;
    char const* unix_socket = url->getParameter("unix-socket");
    MYSQL* db = mysql_init(NULL);
    if(!db)
    {
        *error = Str_dup("unable to allocate mysql handler");
        return NULL;
    }
    if(!( user = url->getUser() ) )
        if(! (user = url->getParameter("user") ) )
            ERROR("no username specified in URL");
    if(!(password = url->getPassword()))
        if (! (password = url->getParameter("password")))
            ERROR("no password specified in URL");
    if(unix_socket)
    {
        host = "localhost"; // Make sure host is localhost if unix socket is to be used
    }
    else if(! (host = url->getHost()))
        ERROR("no host specified in URL");
    if((port = url->getPort()) <= 0)
        ERROR("no port specified in URL");
    if(!(database = url->getPath()))
        ERROR("no database specified in URL");
    else
        database++;
    /* Options */
    if(IS(url->getParameter("compress"), "true"))
        clientFlags |= CLIENT_COMPRESS;
    if(IS(url->getParameter("use-ssl"), "true"))
        mysql_ssl_set(db, 0,0,0,0,0);
    if(IS(url->getParameter("secure-auth"), "true"))
        mysql_options(db, MYSQL_SECURE_AUTH, (const char*)&yes);
    else
        mysql_options(db, MYSQL_SECURE_AUTH, (const char*)&no);
    if((timeout = url->getParameter("connect-timeout")))
    {
        try
        {
            connectTimeout = Str_parseInt(timeout);
        }
        catch(...)
        {
            ERROR("invalid connect timeout value");
        }
    }
    mysql_options(db, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&connectTimeout);
    if((charset = url->getParameter("charset")))
        mysql_options(db, MYSQL_SET_CHARSET_NAME, charset);
#if MYSQL_VERSION_ID >= 50013
    mysql_options(db, MYSQL_OPT_RECONNECT, (const char*)&yes);
#endif
    /* Connect */
    if(mysql_real_connect(db, host, user, password, database, port, unix_socket,
                           clientFlags))
        return db;
    *error = Str_dup(mysql_error(db));
error:
    mysql_close(db);
    return NULL;
}

int MysqlConnection::prepare(char const* sql , int len , MYSQL_STMT** stmt)
{
    if(!(*stmt = mysql_stmt_init(_db)))
    {
        LOG_DEBUG << "mysql_stmt_init -- Out of memory\n";
        _lastError = CR_OUT_OF_MEMORY;
        return false;
    }
    if((_lastError = mysql_stmt_prepare(*stmt, sql, len)))
    {
        StringBuffer_set(_sb, "%s", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        *stmt = NULL;
        return false;
    }
    return true;
}