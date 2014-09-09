#include "PreparedStatement.h"
#include "SQLException.h"

PreparedStatement::PreparedStatement(int parameterCount):
    _parameterCount(parameterCount)
{
}

PreparedStatement::~PreparedStatement()
{
}

void PreparedStatement::setString(int parameterIndex , CONST_STDSTR str)
{
}

void PreparedStatement::setInt(int parameterIndex , int x)
{
}

void PreparedStatement::setLLong(int parameterIndex , long long x)
{
}

void PreparedStatement::setDouble(int parameterIndex , double x)
{
}

void PreparedStatement::setBlob(int parameterIndex , void const* x)
{
}

void PreparedStatement::setTimestamp(int parameterIndex , time_t x)
{
}

void PreparedStatement::execute()
{
    _resultSet->clear();
}

ResultSetPtr PreparedStatement::executeQuery()
{
    _resultSet->clear();
    return _resultSet;
}

long long PreparedStatement::rowsChanged()
{
    return 0;
}

void PreparedStatement::clear()
{
}

int PreparedStatement::getParameterCount()
{
    return _parameterCount;
}

int PreparedStatement::checkAndSetParameterIndex(int parameterIndex)
{
    int i = parameterIndex - 1;
    if (_parameterCount <= 0 || i < 0 || i >= _parameterCount)
        THROW(SQLException , "Parameter index is out of range");//TODO
    return i;
}