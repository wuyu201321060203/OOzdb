#include "Connection.h"

Connection::Connection(ConnectionPoolPtr pool):
    _pool(pool),
    _isAvailable(true),
    _isInTransaction(false),
    _timeout(SQL_DEFAULT_TIMEOUT),
    _url(_pool->getURL()),
    _lastAccessedTime(Time::now())
{
    assert(_pool);
}

Connection::~Connection()
{
    clear();
}

void Connection::setAvailable(bool isAvailable)
{
    _isAvailable = isAvailable;
    _lastAccessedTime = Time::now();
}

bool Connection::isAvailable()
{
    return _isAvailable;
}

time_t Connection::getLastAccessedTime()
{
    return _lastAccessedTime;
}

bool Connection::isInTransaction()
{
    return _isInTransaction;
}

void Connection::setQueryTimeout(int ms)
{
    assert(ms >= 0);
    _timeout = ms;
}

int Connection::getQueryTimeout()
{
    return _timeout;
}

void Connection::setMaxRows(int max)
{
    _maxRows = max;
}

int Connection::getMaxRows()
{
    return _maxRows;
}

URL_T Connection::getURL()
{
    return _url;
}

int Connection::ping()
{
    return SUCCESSFUL;
}

void Connection::clear()
{
    if(_resultSet)
        _resultSet->clear();
    if(_maxRows)
        setMaxRows(0);
    if(_timeout != SQL_DEFAULT_TIMEOUT)
        setQueryTimeout(SQL_DEFAULT_TIMEOUT);
    freePrepared();
}

void Connection::close()
{
    _pool->returnConnection( enable_shared_from_this() );
}

void Connection::beginTransaction()
{
    ++_isInTransaction;
}

void Connection::commit()
{
    if(_isInTransaction)
        _isInTransaction = 0;

}

void Connection::rollback()
{
    if(_isInTransaction)
    {
        clear();
        _isInTransaction = 0;
    }
}

long long Connection::getLastRowId()
{
    return INVALID_ROWID;
}

long long Connection::rowsChanged()
{
    return INVALID_ROWSCHANGED;
}

void Connection::execute(CONST_STDSTR sql , ...)
{
    assert(sql);
    if(_resultSet)
        _resultSet->clear();
}

ResultSetPtr Connection::executeQuery(CONST_STDSTR sql , ...)
{
    assert(sql);
    if(_resultSet)
        _resultSet->clear();
    return _resultSet;
}

PreparedStatementPtr Connection::getPreparedStatement(CONST_STDSTR sql , ...)
{
    assert(sql);
    return PreparedStatementPtr(NULL);
}

CONST_STDSTR Connection::getLastError()
{
    return "?";
}