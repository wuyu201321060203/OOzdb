#ifndef ASSERTEXCEPTION_H
#define ASSERTEXCEPTION_H

#include "Exception.h"

class AssertException : public Exception
{
public:

    explicit AssertException():Exception()
    {
    }

    ~AssertException()//TODO
    {
    }
};

#endif