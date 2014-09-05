#ifndef SQLEXCEPTION_H
#define SQLEXCEPTION_H

#include "Exception.h"

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

#endif