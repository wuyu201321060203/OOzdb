#include <Config.h>

#include "Connection.h"
#include "ConnectionPool.h"

using namespace OOzdb;

Connection::Connection(ConnectionPool* pool):
    _pool(pool),
    _isAvailable(true),
    _isInTransaction(0),
    _timeout(SQL_DEFAULT_TIMEOUT),
    _url(_pool->getURL()),
    _lastAccessedTime(Time_now()),
    _maxRows(DEFAULT_MAX_ROWS)
{
    assert(_pool);
}

Connection::~Connection()
{
}

int Connection::ping()
{
    return SUCCESSFUL;
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

void Connection::execute(char const* sql , ...)
{
    assert(sql);
    if(_resultSet)
        _resultSet->clear();
}

ResultSetPtr Connection::executeQuery(char const* sql , ...)
{
    assert(sql);
    if(_resultSet)
        _resultSet->clear();
    return _resultSet;
}

PreparedStatementPtr Connection::getPreparedStatement(char const* sql , ...)
{
    assert(sql);
    return PreparedStatementPtr();//non-beautiful
}

CONST_STDSTR Connection::getLastError()
{
    return "?";
}

void Connection::setAvailable(bool available)
{
    _isAvailable = available;
    _lastAccessedTime = Time_now();
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
    return (_isInTransaction > 0);
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

void Connection::setMaxRows(int rows)
{
    _maxRows = rows;
}

int Connection::getMaxRows()
{
    return _maxRows;
}

URLPtr Connection::getURL()
{
    return _url;
}

void Connection::clear()
{
    if(_resultSet)
        _resultSet->clear();
    if(_maxRows)
        setMaxRows(DEFAULT_MAX_ROWS);
    if(_timeout != SQL_DEFAULT_TIMEOUT)
        setQueryTimeout(SQL_DEFAULT_TIMEOUT);
    freePrepared();
}

void Connection::close()
{
}

void Connection::freePrepared()
{
    //_prepared.clear();//TODO
}
