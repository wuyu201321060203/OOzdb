#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

using std::exception;
using std::string;

class Exception
{
public:

    Exception(string const& reason):_reason(reason)
    {
    }

    std::string const getReason()
    {
        return _reason;
    }

private:

    string _reason;
};

#endif