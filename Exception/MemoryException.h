#ifndef MEMORYEXCEPTION_H
#define MEMORYEXCEPTION_H

#include "Exception.h"

class MemoryException : public Exception
{
public:

    explicit MemoryException():Exception()
    {
    }

    ~MemoryException()//TODO
    {
    }
};

#endif