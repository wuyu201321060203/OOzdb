#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <cassert>

#include "Config.h"
#include "StringBuffer.h"

StringBuffer::StringBuffer(char const* s):_used(0),
    _length(STRLEN)
{
    _buffer = ALLOC(_length);
    append("%s" , s);
}

StringBuffer::StringBuffer(int length , char const* s)
{
    assert(length > 0);
    _used = 0;
    _length = length;
    _buffer = ALLOC(_length);
    append("%s" , s);
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
    return static_cast<char const*>(_buffer);
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
        int n = vsnprintf((char*)(S->buffer + S->used),
            S->length - S->used, s, ap_copy);
        va_end(ap_copy);
        if ((S->used + n) < S->length) {
            S->used += n;
            break;
        }
        S->length += STRLEN + n;
        RESIZE(S->buffer, S->length);
    }
}

int prepare(char prefix)
{
    int n, i;
    for(n = i = 0; S->buffer[i]; i++) if (S->buffer[i] == '?') n++;
    if(n > 99)
        THROW(SQLException, "Max 99 parameters are allowed in a prepared statement.
                             Found %d parameters in statement", n);
    else if(n)
    {
        int j, xl;
        char x[3] = {prefix};
        int required = (n * 2) + S->used;
        if (required >= S->length) {
            S->length = required;
            RESIZE(S->buffer, S->length);
        }
        for (i = 0, j = 1; (j <= n); i++) {
            if (S->buffer[i] == '?') {
                if(j<10){xl=2;x[1]=j+'0';}else{xl=3;x[1]=(j/10)+'0';x[2]=(j%10)+'0';}
                memmove(S->buffer + i + xl, S->buffer + i + 1, (S->used - (i + 1)));
                memmove(S->buffer + i, x, xl);
                S->used += xl - 1;
                j++;
            }
        }
        S->buffer[S->used] = 0;
    }
    return n;
}
