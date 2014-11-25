#include <cstdarg>
#include <cassert>
#include <mysql/errmsg.h>

#include <Exception/SQLException.h>
#include <Db/ConnectionPool.h>
#include <util/StrOperation.h>
#include <Config.h>

#include "MysqlConnection.h"
#include "MysqlPreparedStatement.h"
#include "MysqlResultSet.h"

//#include <iostream>

using namespace OOzdb;

MysqlConnection::MysqlConnection(ConnectionPool* pool , char** error):
    Connection(pool),
    _lastError(MYSQL_OK),
    _sb(STRLEN)
{
    assert(error);
    if( UNLIKELY( !(_db = doConnect(_url, error) ) ) )
        THROW(SQLException , "Can't connect to MySQL");
}

MysqlConnection::~MysqlConnection()
{
    mysql_close(_db);
    _sb.clear();
}

int MysqlConnection::ping()
{
    return (mysql_ping(_db) == 0);
}

void MysqlConnection::beginTransaction()
{
    _lastError = mysql_query(_db, "START TRANSACTION;");
    if (_lastError != MYSQL_OK)
        THROW(SQLException , "Can't begin transaction");
    ++_isInTransaction;
}

void MysqlConnection::commit()
{
    if(_isInTransaction)
        _isInTransaction = 0;
    _lastError = mysql_query(_db, "COMMIT;");
    if(_lastError != MYSQL_OK)
        THROW(SQLException , "Can't commit");
}

void MysqlConnection::rollback()
{
    if(_isInTransaction)
    {
        _isInTransaction = 0;
        clear();
    }
    _lastError = mysql_query(_db, "ROLLBACK;");
    if(_lastError != MYSQL_OK)
        THROW(SQLException , "Can't rollback");
}

long long MysqlConnection::getLastRowId()
{
    return SC<long long>(mysql_insert_id(_db));
}

long long MysqlConnection::rowsChanged()
{
    return SC<long long>(mysql_affected_rows(_db));
}

void MysqlConnection::execute(char const* sql , ...)
{
    assert(sql);
    va_list ap;
    va_start(ap , sql);
    if(_resultSet)
    {
        _resultSet->clear();
    }
    _sb.vset(sql, ap);
    _lastError = mysql_real_query(_db , _sb.toString(),
                                  _sb.getLength());
    va_end(ap);
    if(_lastError != MYSQL_OK)
        THROW(SQLException , "mysql execute failed");
}

ResultSetPtr MysqlConnection::executeQuery(char const* sql , ...)
{
    assert(sql);
    va_list ap;
    va_start(ap , sql);
    if(_resultSet)
    {
        _resultSet->clear();
    }
    MYSQL_STMT* stmt = NULL;
    _sb.vset(sql , ap);
    if(prepare(_sb.toString() , _sb.getLength() , &stmt))
    {
#if MYSQL_VERSION_ID >= 50002
        ULONG cursor = CURSOR_TYPE_READ_ONLY;
        mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, &cursor);
#endif
        if((_lastError = mysql_stmt_execute(stmt)))
        {
            _sb.set("%s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            //std::cout<<rt;
        }
        else
        {
            ResultSetPtr temp(new MysqlResultSet("MysqlResultSet" ,
                                                 stmt , _maxRows , false));
            _resultSet.swap(temp);
        }
    }
    va_end(ap);
    if(!_resultSet)
        THROW(SQLException , "Mysql executeQuery failed");
    return _resultSet;
}

PreparedStatementPtr MysqlConnection::getPreparedStatement(char const* sql , ...)
{
    PreparedStatementPtr ret;
    assert(sql);
    va_list ap;
    va_start(ap , sql);
    MYSQL_STMT *stmt = NULL;
    _sb.vset(sql , ap);
    if(prepare(_sb.toString() , _sb.getLength() , &stmt))
    {
        int parameterCount = SC<int>(mysql_stmt_param_count(stmt));
        PreparedStatementPtr item(new MysqlPreparedStatement(stmt , _maxRows,
                                                               parameterCount) );
        _prepared.push_back(item);
        ret.swap(item);
    }
    va_end(ap);
    return ret;
}

CONST_STDSTR MysqlConnection::getLastError()
{
    if( UNLIKELY( mysql_errno(_db) ) )
        return SC<CONST_STDSTR>(mysql_error(_db));
    return SC<CONST_STDSTR>( _sb.toString() ); // Either the statement itself or a statement error
}

void MysqlConnection::close()
{
    if(_pool)
        _pool->returnConnection(shared_from_this());
}

void MysqlConnection::onStop()
{
    if (mysql_get_client_version() >= 50003)
        mysql_library_end();
    else
        mysql_server_end();
}

MYSQL* MysqlConnection::doConnect(URLPtr url , char **error)
{
#define ERROR(e) do {*error = strDup(e); goto error;} while (0)
    int port;
    my_bool yes = 1;
    my_bool no = 0;
    int connectTimeout = SQL_DEFAULT_TCP_TIMEOUT;
    ULONG clientFlags = CLIENT_MULTI_STATEMENTS;
    char const *user , *password , *host , *database , *charset , *timeout;
    char const* unix_socket = url->getParameter("unix-socket");
    MYSQL* db = mysql_init(NULL);
    if(UNLIKELY(!db))
    {
        *error = strDup("unable to allocate mysql handler");
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
        mysql_options(db, MYSQL_SECURE_AUTH, SC<char const* >(&yes));
    else
        mysql_options(db, MYSQL_SECURE_AUTH, SC<char const*>(&no));
    if((timeout = url->getParameter("connect-timeout")))
    {
        try
        {
            connectTimeout = strParseInt(timeout);
        }
        catch(...)
        {
            ERROR("invalid connect timeout value");
        }
    }
    mysql_options(db, MYSQL_OPT_CONNECT_TIMEOUT, RC<char const*>(&connectTimeout));
    if((charset = url->getParameter("charset")))
        mysql_options(db, MYSQL_SET_CHARSET_NAME, charset);
#if MYSQL_VERSION_ID >= 50013
    mysql_options(db, MYSQL_OPT_RECONNECT, SC<char const*>(&yes));
#endif
    /* Connect */
    if(mysql_real_connect(db, host, user, password, database, port, unix_socket,clientFlags))
        return db;
    *error = strDup(mysql_error(db));

error:
    mysql_close(db);
    return NULL;
}

int MysqlConnection::prepare(char const* sql , int len , MYSQL_STMT** stmt)
{
    if( UNLIKELY( !( *stmt = mysql_stmt_init(_db) ) ) )
    {
        LOG_DEBUG << "mysql_stmt_init -- Out of memory\n";
        _lastError = CR_OUT_OF_MEMORY;
        return false;
    }
    if( UNLIKELY( _lastError = mysql_stmt_prepare(*stmt, sql, len) ) )
    {
        _sb.set("%s", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        //std::cout<<rt;
        *stmt = NULL;
        THROW(SQLException , "prepare stmt error");
    }
    return true;
}