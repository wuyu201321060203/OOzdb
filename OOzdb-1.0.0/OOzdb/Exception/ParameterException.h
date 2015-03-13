#ifndef PARAMETEREXCEPTION_H
#define PARAMETEREXCEPTION_H

#include "Exception.h"

namespace OOzdb
{

class ParameterException : public Exception
{
public:

    explicit ParameterException():Exception()
    {
    }

    ~ParameterException()//TODO
    {
    }
};

}

#endif