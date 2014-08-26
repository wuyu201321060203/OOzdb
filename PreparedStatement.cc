#include "PreparedStatement.h"

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
    return ResultSetPtr(NULL);
}

long long PreparedStatement::rowsChanged()
{
}

int PreparedStatement::getParameterCount()
{
    return _parameterCount;
}