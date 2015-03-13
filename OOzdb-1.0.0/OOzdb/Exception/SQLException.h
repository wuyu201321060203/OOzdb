#ifndef SQLEXCEPTION_H
#define SQLEXCEPTION_H

#include "Exception.h"

namespace OOzdb
{

class SQLException : public Exception
{
public:

    explicit SQLException():Exception()
    {
    }

    ~SQLException()//TODO
    {
    }
};

}

#endif