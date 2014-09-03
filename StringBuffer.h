#ifndef STRINGBUFFER_INCLUDED
#define STRINGBUFFER_INCLUDED

#include <boost/noncopyable.hpp>

class StringBuffer : boost::noncopyable
{
public:

    StringBuffer(int length , char const* str);
    ~StringBuffer();
    void append(char const* s , ...)__attribute__((format (printf, 1, 2)));
    void vappend(char const* s , va_list ap);
    void set(char const* s , ...)__attribute__((format (printf, 1, 2)));
    void vset(char const* s , va_list ap);
    int length();
    void clear();
    char const* toString();
    void trim();

};

#endif
