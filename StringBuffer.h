#ifndef STRINGBUFFER_INCLUDED
#define STRINGBUFFER_INCLUDED

#include <boost/noncopyable.hpp>

typedef unsigned char uchar_t;

class StringBuffer : boost::noncopyable
{
public:

    explicit StringBuffer(char const* s);
    explicit StringBuffer(int length , char const* s);
    explicit StringBuffer(int length);
    ~StringBuffer();
    void append(char const* s , ...)__attribute__((format (printf, 2, 3)));
    void vappend(char const* s , va_list ap);
    void set(char const* s , ...)__attribute__((format (printf, 2, 3)));
    void vset(char const* s , va_list ap);
    int getLength();
    void clear();
    char const* toString();
    int prepare4postgres();
    int prepare4oracle();
    void trim();

private:

    int _used;
    int _length;
    uchar_t* _buffer;

private:

    inline void doAppend(char const* s , va_list ap);
    int prepare(char prefix);
};

#endif
