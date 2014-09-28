#ifndef ASSERTEXCEPTION_H
#define ASSERTEXCEPTION_H

#include "Exception.h"

namespace OOzdb
{

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

}

#endif