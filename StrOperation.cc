#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "Config.h"
#include "SQLException.h"

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

int strIsEqual(char const* a , char const* b)
{
    if(a && b)
    {
        while (*a && *b)
            if (toupper(*a++) != toupper(*b++)) return false;
        return (*a == *b);
    }
    return false;
}

int strIsByteEqual(char const* a , char const* b)
{
    if(a && b)
    {
        while (*a && *b)
            if (*a++ != *b++) return false;
        return (*a == *b);
    }
    return false;
}

int strStartsWith(char const* a , char const* b)
{
    if(a && b)
    {
        do
            if (*a++ != *b++) return false;
        while (*b);
        return true;
    }
    return false;
}

char* strCopy(char* dest , char const* src , int n)
{
    if(src && dest && (n > 0))
    {
        char *t = dest;
        while (*src && n--)
            *t++ = *src++;
        *t = 0;
    }
    else if(dest)
        *dest = 0;
    return dest;
}

char* strDup(char const* s)
{
    char *t = NULL;
    if(s)
    {
        size_t n = strlen(s) + 1;
        t = static_cast<char*>(ALLOC(n));
        memcpy(t, s, n);
    }
    return t;
}

char* strNDup(char const* s , int n)
{
    char *t = NULL;
    assert(n >= 0);
    if (s)
    {
        int l = strlen(s);
        n = l < n ? l : n; // Use the actual length of s if shorter than n
        t = static_cast<char*>(ALLOC(n + 1));
        memcpy(t, s, n);
        t[n] = 0;
    }
    return t;
}

char* strCat(char const*s , ...)
{
    char* t = 0;
    if(s)
    {
        va_list ap;
        va_start(ap, s);
        t = strVcat(s, ap);
        va_end(ap);
    }
    return t;
}

char* strVcat(char const* s , va_list ap)
{
    char* buf = NULL;
    if(s)
    {
        int n = 0;
        va_list ap_copy;
        int size = 88;
        buf = static_cast<char*>(ALLOC(size));
        while(true)
        {
            va_copy(ap_copy, ap);
            n = vsnprintf(buf, size, s, ap_copy);
            va_end(ap_copy);
            if (n < size)
                break;
            size = n + 1;
            buf = static_cast<char*>(RESIZE(buf, size));
        }
    }
    return buf;
}


int strParseInt(char const* s)
{
    if (STR_UNDEF(s))
        THROW(SQLException , "NumberFormatException: For input string null");
    errno = 0;
    char* e;
    int i = strtol(s, &e, 10);
    if(errno || (e == s))
        THROW(SQLException , "NumberFormatException: For input string %s -- %s",
                                s , System_getLastError);
    return i;
}


long long strParseLLong(char const* s)
{
    if(STR_UNDEF(s))
        THROW(SQLException, "NumberFormatException: For input string null");
    errno = 0;
    char* e;
    long long ll = strtoll(s, &e, 10);
    if(errno || (e == s))
        THROW(SQLException, "NumberFormatException: For input string %s -- %s",
                                s, System_getLastError);
    return ll;
}

double strParseDouble(const char *s)
{
    if(STR_UNDEF(s))
        THROW(SQLException, "NumberFormatException: For input string null");
    errno = 0;
    char* e;
    double d = strtod(s, &e);
    if(errno || (e == s))
        THROW(SQLException, "NumberFormatException: For input string %s -- %s",
                                s , System_getLastError);
    return d;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif
