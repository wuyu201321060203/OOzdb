#include "MysqlConnection.h"

MysqlConnection::MysqlConnection(URL_T url , char** error)
{
    MYSQL *db;
    assert(url);
    assert(error);
    if(!(db = doConnect(url, error)))
        return NULL;
    _db = db;
    _url = url;
    _sb = StringBuffer_create(STRLEN);
    _timeout = SQL_DEFAULT_TIMEOUT;
}

MysqlConnection::~MysqlConnection()
{
    mysql_close(_db);
    StringBuffer_free(&_sb);
}

void MysqlConnection::setQueryTimeout(int ms)
{
    _timeout = ms;
}

void MysqlConnection::setMaxRows(int max)
{
    _maxRows = max;
}

int MysqlConnection::ping()
{
    return (mysql_ping(_db) == 0);
}

int MysqlConnection::beginTransaction()
{
    _lastError = mysql_query(_db, "START TRANSACTION;");
    return (_lastError == MYSQL_OK);
}

int MysqlConnection::commit()
{
    _lastError = mysql_query(_db, "COMMIT;");
    return (_lastError == MYSQL_OK);
}

int MysqlConnection::rollback()
{
    _lastError = mysql_query(db, "ROLLBACK;");
    return (_lastError == MYSQL_OK);
}

long long MysqlConnection::lastRowId()
{
    return (long long)mysql_insert_id(_db);
}

long long MysqlConnection::rowsChanged()
{
    return (long long)mysql_affected_rows(_db);
}

int MysqlConnection::execute(char const* sql , va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    StringBuffer_vset(_sb, sql, ap_copy);
    va_end(ap_copy);
    _lastError = mysql_real_query(_db, StringBuffer_toString(_sb), StringBuffer_length(_sb));
    return (_lastError == MYSQL_OK);
}

ResultSet MysqlConnection::executeQuery(char const* sql , va_list ap)
{
    va_list ap_copy;
    MYSQL_STMT *stmt = NULL;
    va_copy(ap_copy, ap);
    StringBuffer_vset(_sb, sql, ap_copy);
    va_end(ap_copy);
    if(prepare(StringBuffer_toString(_sb), StringBuffer_length(_sb), &stmt))
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
            return ResultSet_new(MysqlResultSet_new(_stmt , _maxRows, false));
    }
    return NULL;
}

PreparedStatement MysqlConnection::prepareStatement(char const* sql , va_list ap)
{
    va_list ap_copy;
    MYSQL_STMT *stmt = NULL;
    va_copy(ap_copy, ap);
    StringBuffer_vset(_sb, sql, ap_copy);
    va_end(ap_copy);
    if (prepare(StringBuffer_toString(_sb), StringBuffer_length(_sb), &stmt))
    {
        int parameterCount = (int)mysql_stmt_param_count(_stmt);
        return PreparedStatement_new(MysqlPreparedStatement_new(stmt,
            _maxRows, parameterCount), (Pop_T)&mysqlpops, parameterCount);
    }
    return NULL;
}

char const* MysqlConnection::getLastError()
{
    if (mysql_errno(_db))
        return mysql_error(_db);
    return StringBuffer_toString(_sb); // Either the statement itself or a statement error
}

/* Class method: MySQL client library finalization */
void MysqlConnection::onstop(void)
{
    if (mysql_get_client_version() >= 50003)
        mysql_library_end();
    else
        mysql_server_end();
}

MYSQL* MysqlConnection::doConnect(URL_T url , char **error)
{
#define ERROR(e) do {*error = Str_dup(e); goto error;} while (0)
    int port;
    my_bool yes = 1;
    my_bool no = 0;
    int connectTimeout = SQL_DEFAULT_TCP_TIMEOUT;
    unsigned long clientFlags = CLIENT_MULTI_STATEMENTS;
    const char *user , *password, *host, *database, *charset, *timeout;
    const char *unix_socket = URL_getParameter(url, "unix-socket");
    MYSQL* db = mysql_init(NULL);
    if(!db)
    {
        *error = Str_dup("unable to allocate mysql handler");//TODO
        return NULL;
    }
    if( !( user = URL_getUser(url) ) )
        if(! (user = URL_getParameter(url, "user") ) )
            ERROR("no username specified in URL");
    if( !(password = URL_getPassword(url)))
        if (! (password = URL_getParameter(url, "password")))
            ERROR("no password specified in URL");
    if (unix_socket) {
        host = "localhost"; // Make sure host is localhost if unix socket is to be used
    } else if (! (host = URL_getHost(url)))
        ERROR("no host specified in URL");
    if ((port = URL_getPort(url)) <= 0)
        ERROR("no port specified in URL");
    if (! (database = URL_getPath(url)))
        ERROR("no database specified in URL");
    else
        database++;
    /* Options */
    if (IS(URL_getParameter(url, "compress"), "true"))
        clientFlags |= CLIENT_COMPRESS;
    if (IS(URL_getParameter(url, "use-ssl"), "true"))
        mysql_ssl_set(db, 0,0,0,0,0);
    if (IS(URL_getParameter(url, "secure-auth"), "true"))
        mysql_options(db, MYSQL_SECURE_AUTH, (const char*)&yes);
    else
        mysql_options(db, MYSQL_SECURE_AUTH, (const char*)&no);
    if ((timeout = URL_getParameter(url, "connect-timeout"))) {
        TRY connectTimeout = Str_parseInt(timeout); ELSE ERROR("invalid connect timeout value"); END_TRY;
    }
    mysql_options(db, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&connectTimeout);
    if ((charset = URL_getParameter(url, "charset")))
        mysql_options(db, MYSQL_SET_CHARSET_NAME, charset);
#if MYSQL_VERSION_ID >= 50013
    mysql_options(db, MYSQL_OPT_RECONNECT, (const char*)&yes);
#endif
    /* Connect */
    if (mysql_real_connect(db, host, user, password, database, port, unix_socket, clientFlags))
        return db;
    *error = Str_dup(mysql_error(db));
error:
    mysql_close(db);
    return NULL;
}

int MysqlConnection::prepare(T C, const char *sql, int len, MYSQL_STMT **stmt)
{
    if (! (*stmt = mysql_stmt_init(C->db))) {
        DEBUG("mysql_stmt_init -- Out of memory\n");
        C->lastError = CR_OUT_OF_MEMORY;
        return false;
    }
    if ((C->lastError = mysql_stmt_prepare(*stmt, sql, len))) {
        StringBuffer_set(C->sb, "%s", mysql_stmt_error(*stmt));
        mysql_stmt_close(*stmt);
        *stmt = NULL;
        return false;
    }
    return true;
}