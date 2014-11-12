#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdarg.h>
#include <cassert>

#include <Exception/SQLException.h>
#include <util/MemoryOperation.h>
#include <util/StrOperation.h>

#include "StringBuffer.h"

using namespace OOzdb;

StringBuffer::StringBuffer(char const* s):_used(0),
    _length(STRLEN)
{
    _buffer = SC<uchar_t*>(ALLOC(_length));
    bzero(_buffer , _length);
    append("%s" , s);
}

StringBuffer::StringBuffer(int length , char const* s)
{
    assert(length > 0);
    _used = 0;
    _length = length;
    _buffer = SC<uchar_t*>(ALLOC(_length));
    bzero(_buffer , _length);
    append("%s" , s);
}

StringBuffer::StringBuffer(int length)
{
    assert(length > 0);
    _used = 0;
    _length = length;
    _buffer = SC<uchar_t*>(ALLOC(_length));
    bzero(_buffer , _length);
}

StringBuffer::~StringBuffer()
{
    FREE(_buffer);
}

void StringBuffer::append(char const* s , ...)
{
    if(STR_DEF(s))
    {
        va_list ap;
        va_start(ap , s);
        doAppend(s , ap);
        va_end(ap);
    }
}

void StringBuffer::vappend(char const* s , va_list ap)
{
    if(STR_DEF(s))
    {
        va_list ap_copy;
        va_copy(ap_copy , ap);
        doAppend(s , ap_copy);
        va_end(ap_copy);
    }
}

void StringBuffer::set(char const* s , ...)
{
    clear();
    if(STR_DEF(s))
    {
        va_list ap;
        va_start(ap , s);
        doAppend(s , ap);
        va_end(ap);
    }
}

void StringBuffer::vset(char const* s , va_list ap)
{
    clear();
    if(STR_DEF(s))
    {
        va_list ap_copy;
        va_copy(ap_copy , ap);
        doAppend(s , ap_copy);
        va_end(ap_copy);
    }
}

int StringBuffer::getLength()
{
    return _used;
}

void StringBuffer::clear()
{
    _used = 0;
    *_buffer = 0;
}

char const* StringBuffer::toString()
{
    return RC<char const*>(_buffer);//may be it's a problem
}

int StringBuffer::prepare4postgres()
{
    return prepare('$');
}

int StringBuffer::prepare4oracle()
{
    return prepare(':');
}

void StringBuffer::trim()
{
    while(_used && ( (_buffer[_used - 1] == ';') || isspace(_buffer[_used - 1]) ))
        _buffer[--_used] = 0;
    if(isspace(*_buffer))
    {
        int i = 0;
        for( ; isspace(_buffer[i]) ; ++i );
        memmove(_buffer , _buffer + i , _used - i);
        _used -= i;
        _buffer[_used] = 0;
    }
}

void StringBuffer::doAppend(char const* s , va_list ap)
{
    va_list ap_copy;
    while (true)
    {
        va_copy(ap_copy, ap);
        int n = vsnprintf(RC<char*>(_buffer + _used),//ugly
            _length - _used, s, ap_copy);
        va_end(ap_copy);
        if ((_used + n) < _length) {
            _used += n;
            break;
        }
        _length += STRLEN + n;
        _buffer = SC<uchar_t*>( RESIZE(_buffer, _length) );
    }
}

int StringBuffer::prepare(char prefix)
{
    int n, i;
    for(n = i = 0; _buffer[i]; i++) if (_buffer[i] == '?') n++;
    if(n > 99)
        THROW(SQLException , "Max 99 parameters are allowed in a prepared statement.\
                             Found %d parameters in statement", n);
    else if(n)
    {
        int j, xl;
        char x[3] = {prefix};
        int required = (n * 2) + _used;
        if (required >= _length) {
            _length = required;
            _buffer = SC<uchar_t*>(RESIZE(_buffer , _length));
        }
        for (i = 0, j = 1; (j <= n); i++) {
            if (_buffer[i] == '?') {
                if(j<10){xl=2;x[1]=SC<char>(j + '0');}
                else{xl=3;x[1]=SC<char>((j/10) + 48);x[2]=SC<char>((j%10)+48);}
                memmove(_buffer + i + xl , _buffer + i + 1 , (_used - (i + 1)));
                memmove(_buffer + i, x, xl);
                _used += xl - 1;
                j++;
            }
        }
        _buffer[_used] = 0;
    }
    return n;
}
