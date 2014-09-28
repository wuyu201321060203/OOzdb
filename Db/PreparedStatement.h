#ifndef PREPAREDSTATEMENT_H
#define PREPAREDSTATEMENT_H

#include <time.h>
#include <vector>

#include "ResultSet.h"

namespace OOzdb
{

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
    virtual void setBlob(int parameterIndex , void const* x , int size);
    virtual void setTimestamp(int parameterIndex , time_t x);
    virtual void execute();
    virtual ResultSetPtr executeQuery();
    virtual long long rowsChanged();
    virtual void clear();
    int getParameterCount();

protected:

    int _parameterCount;
    ResultSetPtr _resultSet;

protected:

    int checkAndSetParameterIndex(int parameterIndex);
};

typedef boost::shared_ptr<PreparedStatement> PreparedStatementPtr;
typedef std::vector<PreparedStatementPtr> PreparedStatementVec;

}

#endif