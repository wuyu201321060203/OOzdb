#ifndef PREPAREDSTATEMENT_H
#define PREPAREDSTATEMENT_H

#include <time.h>
#include <vector>

#include "Str.h"
#include "ResultSet.h"

class PreparedStatement
{
public:

    PreparedStatement(int parameterCount);
    virtual ~PreparedStatement();

public:

    virtual void setString(int parameterIndex , CONST_STDSTR str);
    virtual void setInt(int parameterIndex , int x);
    virtual void setLLong(int parameterIndex , long long x);
    virtual void setDouble(int parameterIndex , double x);
    virtual void setBlob(int parameterIndex , void const* x);
    virtual void setTimestamp(int parameterIndex , time_t x);
    virtual void execute();
    virtual ResultSetPtr executeQuery();
    virtual long long rowsChanged();
    int getParameterCount();

private:

    int _parameterCount;
    ResultSetPtr _resultSet;
};

typedef boost::shared_ptr<PreparedStatement> PreparedStatementPtr;
typedef std::vector<PreparedStatementPtr> PreparedStatementVec;

#endif