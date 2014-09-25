#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <Config.h>
#include <Exception/SQLException.h>
#include <util/MemoryOperation.h>
#include <util/StrOperation.h>

#include "URL.h"

#define UNKNOWN_PORT -1
#define YYCTYPE       uchar_t
#define YYCURSOR      _buffer
#define YYLIMIT       _limit
#define YYMARKER      _marker
#define YYCTXMARKER   _ctx
#define YYFILL(n)     ((void)0)
#define YYTOKEN       _token
#define SET_PROTOCOL(PORT) *(YYCURSOR-3)=0; _protocol=_token; _port=PORT; goto authority

static const uchar_t urlunsafe[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static inline int x2b(uchar_t* x)
{
    register int b;
    b = ((x[0] >= 'A') ? ((x[0] & 0xdf) - 'A')+10 : (x[0] - '0'));
    b *= 16;
    b += (x[1] >= 'A' ? ((x[1] & 0xdf) - 'A')+10 : (x[1] - '0'));
    return b;
}

static inline uchar_t* b2x(uchar_t b, uchar_t* x)
{
    static const char b2x_table[] = "0123456789ABCDEF";
    *x++ = '%';
    *x++ = b2x_table[b >> 4];
    *x = b2x_table[b & 0xf];
    return x;
}

URL::URL(char const* url) : _port(UNKNOWN_PORT),
                            _ref(NULL),
                            _path(NULL),
                            _host(NULL),
                            _user(NULL),
                            _qptr(NULL),
                            _query(NULL),
                            _portStr(NULL),
                            _protocol(NULL),
                            _password(NULL),
                            _toString(NULL),
                            _params(NULL),
                            _paramNames(NULL),
                            _data(NULL),
                            _buffer(NULL),
                            _marker(NULL),
                            _ctx(NULL),
                            _limit(NULL),
                            _token(NULL),
                            _isCleared(false)
{
    if(STR_DEF(url))
    {
        size_t len = strlen(url) + 1;
        _data = SC<char*>(ALLOC(len));
        memcpy(_data , url , len);
        YYCURSOR = _data;
        YYLIMIT = _data + strlen(RC<char const*>(_data));
        if(!parseURL())
            THROW(SQLException , "URL create fail");
    }
    else
        THROW(SQLException , "URL create fail");
}

void URL::URLCreate(char const* url , ...)
{
    if(STR_DEF(url))
    {
        _port = UNKNOWN_PORT;
        _ref = NULL;
        _path = NULL;
        _host = NULL;
        _user = NULL;
        _qptr = NULL;
        _query = NULL;
        _portStr = NULL;
        _protocol = NULL;
        _password = NULL;
        _toString = NULL;
        _params = NULL;
        _paramNames = NULL;
        _buffer = NULL;
        _marker = NULL;
        _ctx = NULL;
        _limit = NULL;
        _token = NULL;
        _isCleared = false;
        va_list ap;
        va_start(ap , url);
        _data = strVcat(url , ap);
        YYCURSOR = _data;
        YYLIMIT = _data + strlen(RC<char const*>(_data));
        if(!parseURL())
            THROW(SQLException , "URL create fail");
    }
    else
        THROW(SQLException , "URL create fail");
}

URL::~URL()
{
    if(!_isCleared)
        clear();
}

void URL::clear()
{
    freeParams();
    FREE(_paramNames);
    FREE(_toString);
	FREE(_query);
	FREE(_data);
	FREE(_host);
    _isCleared = true;
}

char const* URL::getProtocol() const
{
	return _protocol;
}

char const* URL::getUser() const
{
	return _user;
}

char const* URL::getPassword() const
{
	return _password;
}

char const* URL::getHost() const
{
	return _host;
}

int URL::getPort() const
{
	return _port;
}

char const* URL::getPath() const
{
	return _path;
}

char const* URL::getQueryString() const
{
	return _query;
}

char** URL::getParameterNames()
{
    if(_params && (_paramNames == NULL))
    {
        param_t* p;
        int i = 0 , len = 0;
        for(p = _params ; p ; p = p->_next) len++;
        _paramNames = SC<char**>(ALLOC((len + 1) * sizeof *(_paramNames)));
        for(p = _params ; p ; p = p->_next)
            _paramNames[i++] = p->_name;
        _paramNames[i] = NULL;
    }
    return _paramNames;
}

char const* URL::getParameter(char const* name)
{
    assert(name);
    for(param_t* p = _params ; p ; p = p->_next)
    {
        if(strIsByteEqual(p->_name, name))
            return p->_value;
    }
    return NULL;
}

char const* URL::toString()
{
    if(!_toString)
    {
        uchar_t port[11] = {0};
        if(_port >= 0)
            snprintf(RC<char*>(port), 10, ":%d", _port);
        _toString = strCat("%s://%s%s%s%s%s%s%s%s%s",
            _protocol,
            _user ? _user : "",
            _password ? ":" : "",
            _password ? _password : "",
            _user ? "@" : "",
            _host ? _host : "",
            port,
            _path ? _path : "",
            _query ? "?" : "",
            _query ? _query : "");
    }
    return _toString;
}

char* URL::unescape(char* url)
{
    if(STR_DEF(url))
    {
        register int x, y;
        for(x = 0, y = 0; url[y]; x++, y++)
        {
            if((url[x] = url[y]) == '+')
                url[x] = ' ';
            else if (url[x] == '%')
            {
                if (! (url[x + 1] && url[x + 2]))
                    break;
                url[x] = x2b(RC<uchar_t*>(url + y + 1));
                y += 2;
            }
        }
        url[x] = 0;
    }
    return url;
}

char* URL::escape(char const* url)
{
    char* escaped = 0;
    if(url)
    {
        char* p;
        int i, n;
        for(n = i = 0 ; url[i] ; i++)
            if( urlunsafe[SC<int>( SC<uchar_t>(url[i]) )] )
                n += 2;
        p = escaped = SC<char*>(ALLOC(i + n + 1));
        for( ; *url ; url++ , p++)
        {
            if( urlunsafe[ SC<int>( SC<uchar_t>(*p = *url) )] )
                p = RC<char*>(b2x(*url, RC<uchar_t*>(p)));
        }
        *p = 0;
    }
    return escaped;
}

char* URL::normalize(char* path)
{
    if(path)
    {
        char c;
        int i , j;
        for(i = j = 0 ; (c = path[i]) ; ++i)
        {
            if(c == '/')
            {
                while (path[i+1] == '/') ++i;
            }
            else if(c == '.' && j && path[j-1] == '/')
            {
                if(path[i+1] == '.' && (path[i+2] == '/' || path[i+2] == 0))
                {
                    if (j>1)
                        for (j -= 2; path[j] != '/' && j > 0; --j);
                    i += 2;
                }
                else if(path[i+1] == '/' || path[i+1] == 0)
                {
                    ++i;
                    continue;
                }
            }
            if(!(path[j] = path[i])) break; ++j;
        }
        if(!j) { path[0] = '/'; j = 1; }
        path[j] = 0;
        if(path[0] == '/' && path[1] == '/')
        {
            for(i = j = 0 ; (c = path[i]) ; ++i)
            {
                if(c == '/')
                {
                    while (path[i+1] == '/') ++i;
                }
                if(!(path[j] = path[i])) break;
                ++j;
            }
            path[j] = 0;
        }
    }
    return path;
}

bool URL::parseURL()
{
    param_t* param = NULL;

proto:
    if (YYCURSOR >= YYLIMIT)
        return false;
    YYTOKEN = YYCURSOR;

    {
        YYCTYPE yych;
        static const unsigned char yybm[] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128,   0,   0,   0,   0,   0,   0,
            0, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128,   0,   0,   0,   0,   0,
            0, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
        };

        if ((YYLIMIT - YYCURSOR) < 13) YYFILL(13);
        yych = *YYCURSOR;
        if (yych <= '@') {
            if (yych <= '\r') {
                if (yych <= 0x08) goto yy9;
                if (yych <= '\n') goto yy2;
                if (yych <= '\f') goto yy9;
            } else {
                if (yych <= ' ') {
                    if (yych <= 0x1F) goto yy9;
                } else {
                    if (yych <= '/') goto yy9;
                    if (yych <= '9') goto yy8;
                    goto yy9;
                }
            }
        } else {
            if (yych <= 'm') {
                if (yych <= 'Z') goto yy8;
                if (yych <= '`') goto yy9;
                if (yych <= 'l') goto yy8;
                goto yy4;
            } else {
                if (yych <= 'o') {
                    if (yych <= 'n') goto yy8;
                    goto yy7;
                } else {
                    if (yych <= 'p') goto yy6;
                    if (yych <= 'z') goto yy8;
                    goto yy9;
                }
            }
        }
yy2:
        ++YYCURSOR;
        {
            goto proto;
        }
yy4:
        yych = *(YYMARKER = ++YYCURSOR);
        if (yych <= 'Z') {
            if (yych <= '/') goto yy5;
            if (yych <= ':') goto yy13;
            if (yych >= 'A') goto yy13;
        } else {
            if (yych <= 'x') {
                if (yych >= 'a') goto yy13;
            } else {
                if (yych <= 'y') goto yy39;
                if (yych <= 'z') goto yy13;
            }
        }
yy5:
        {
            goto proto;
        }
yy6:
        yych = *(YYMARKER = ++YYCURSOR);
        if (yych <= 'Z') {
            if (yych <= '/') goto yy5;
            if (yych <= ':') goto yy13;
            if (yych <= '@') goto yy5;
            goto yy13;
        } else {
            if (yych <= 'n') {
                if (yych <= '`') goto yy5;
                goto yy13;
            } else {
                if (yych <= 'o') goto yy26;
                if (yych <= 'z') goto yy13;
                goto yy5;
            }
        }
yy7:
        yych = *(YYMARKER = ++YYCURSOR);
        if (yych <= 'Z') {
            if (yych <= '/') goto yy5;
            if (yych <= ':') goto yy13;
            if (yych <= '@') goto yy5;
            goto yy13;
        } else {
            if (yych <= 'q') {
                if (yych <= '`') goto yy5;
                goto yy13;
            } else {
                if (yych <= 'r') goto yy17;
                if (yych <= 'z') goto yy13;
                goto yy5;
            }
        }
yy8:
        yych = *(YYMARKER = ++YYCURSOR);
        if (yych <= '@') {
            if (yych <= '/') goto yy5;
            if (yych <= ':') goto yy13;
            goto yy5;
        } else {
            if (yych <= 'Z') goto yy13;
            if (yych <= '`') goto yy5;
            if (yych <= 'z') goto yy13;
            goto yy5;
        }
yy9:
        yych = *++YYCURSOR;
        goto yy5;
yy10:
        yych = *++YYCURSOR;
        if (yych == '/') goto yy14;
yy11:
        YYCURSOR = YYMARKER;
        goto yy5;
yy12:
        ++YYCURSOR;
        if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
        yych = *YYCURSOR;
yy13:
        if (yybm[0+yych] & 128) {
            goto yy12;
        }
        if (yych == ':') goto yy10;
        goto yy11;
yy14:
        yych = *++YYCURSOR;
        if (yych != '/') goto yy11;
        ++YYCURSOR;
        {
            SET_PROTOCOL(UNKNOWN_PORT);
        }
yy17:
        yych = *++YYCURSOR;
        if (yych != 'a') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'c') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'l') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'e') goto yy13;
        yych = *++YYCURSOR;
        if (yych != ':') goto yy13;
        yych = *++YYCURSOR;
        if (yych != '/') goto yy11;
        yych = *++YYCURSOR;
        if (yych != '/') goto yy11;
        ++YYCURSOR;
        {
            SET_PROTOCOL(ORACLE_DEFAULT_PORT);
        }
yy26:
        yych = *++YYCURSOR;
        if (yych != 's') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 't') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'g') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'r') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'e') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 's') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'q') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'l') goto yy13;
        yych = *++YYCURSOR;
        if (yych != ':') goto yy13;
        yych = *++YYCURSOR;
        if (yych != '/') goto yy11;
        yych = *++YYCURSOR;
        if (yych != '/') goto yy11;
        ++YYCURSOR;
        {
            SET_PROTOCOL(POSTGRESQL_DEFAULT_PORT);
        }
yy39:
        yych = *++YYCURSOR;
        if (yych != 's') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'q') goto yy13;
        yych = *++YYCURSOR;
        if (yych != 'l') goto yy13;
        yych = *++YYCURSOR;
        if (yych != ':') goto yy13;
        yych = *++YYCURSOR;
        if (yych != '/') goto yy11;
        yych = *++YYCURSOR;
        if (yych != '/') goto yy11;
        ++YYCURSOR;
        {
            SET_PROTOCOL(MYSQL_DEFAULT_PORT);
        }
    }

authority:
    if (YYCURSOR >= YYLIMIT)
        return true;
    YYTOKEN = YYCURSOR;

    {
        YYCTYPE yych;
        unsigned int yyaccept = 0;
        static const unsigned char yybm[] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            16, 112, 112,  16, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 240, 112, 112,
            248, 248, 248, 248, 248, 248, 248, 248,
            248, 248, 112,  16, 112, 112, 112,  16,
            64, 240, 240, 240, 240, 240, 240, 240,
            240, 240, 240, 240, 240, 240, 240, 240,
            240, 240, 240, 240, 240, 240, 240, 240,
            240, 240, 240, 112, 112, 112, 112, 112,
            112, 240, 240, 240, 240, 240, 240, 240,
            240, 240, 240, 240, 240, 240, 240, 240,
            240, 240, 240, 240, 240, 240, 240, 240,
            240, 240, 240, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
            112, 112, 112, 112, 112, 112, 112, 112,
        };
        if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
        yych = *YYCURSOR;
        if (yych <= '.') {
            if (yych <= '\r') {
                if (yych <= 0x08) goto yy59;
                if (yych <= '\n') goto yy49;
                if (yych <= '\f') goto yy59;
            } else {
                if (yych <= ' ') {
                    if (yych <= 0x1F) goto yy59;
                    goto yy51;
                } else {
                    if (yych == '-') goto yy54;
                    goto yy52;
                }
            }
        } else {
            if (yych <= '?') {
                if (yych <= '/') goto yy56;
                if (yych <= '9') goto yy54;
                if (yych <= ':') goto yy58;
                goto yy52;
            } else {
                if (yych <= 'Z') {
                    if (yych <= '@') goto yy59;
                    goto yy54;
                } else {
                    if (yych <= '`') goto yy52;
                    if (yych <= 'z') goto yy54;
                    goto yy52;
                }
            }
        }
yy49:
        ++YYCURSOR;
yy50:
        {
            goto authority;
        }
yy51:
        yyaccept = 0;
        yych = *(YYMARKER = ++YYCURSOR);
        if (yych <= 0x1F) goto yy50;
        goto yy66;
yy52:
        yyaccept = 1;
        yych = *(YYMARKER = ++YYCURSOR);
        if (yych >= ' ') goto yy66;
yy53:
        {
            return true;
        }
yy54:
        yyaccept = 2;
        yych = *(YYMARKER = ++YYCURSOR);
        if (yybm[0+yych] & 128) {
            goto yy77;
        }
        if (yych <= 0x1F) goto yy55;
        if (yych == '.') goto yy76;
        goto yy66;
yy55:
        {
            _host = strNDup(YYTOKEN, SC<int>(YYCURSOR - YYTOKEN));
            goto authority;
        }
yy56:
        yyaccept = 3;
        yych = *(YYMARKER = ++YYCURSOR);
        if (yybm[0+yych] & 32) {
            goto yy68;
        }
        if (yych <= 0x1F) goto yy57;
        if (yych <= '>') goto yy65;
        if (yych <= '?') goto yy71;
        goto yy70;
yy57:
            *YYCURSOR = 0;
            _path = unescape(YYTOKEN);
            return true;
yy58:
        yyaccept = 1;
        yych = *(YYMARKER = ++YYCURSOR);
        if (yybm[0+yych] & 8) {
            goto yy60;
        }
        if (yych <= 0x1F) goto yy53;
        goto yy66;
yy59:
        yych = *++YYCURSOR;
        goto yy53;
yy60:
        yyaccept = 4;
        YYMARKER = ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
        if (yybm[0+yych] & 8) {
            goto yy60;
        }
        if (yych <= 0x1F) goto yy62;
        if (yych == '@') goto yy63;
        goto yy65;
yy62:
        {
            _port = strParseInt(YYTOKEN + 1); // read past ':'
            goto authority;
        }
yy63:
        ++YYCURSOR;
yy64:
        {
            *(YYCURSOR - 1) = 0;
            _user = YYTOKEN;
            char *p = strchr(_user, ':');
            if (p) {
                *(p++) = 0;
                _password = unescape(p);
            }
            unescape(_user);
            goto authority;
        }
yy65:
        ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
yy66:
        if (yybm[0+yych] & 16) {
            goto yy65;
        }
        if (yych >= '@') goto yy63;
yy67:
        YYCURSOR = YYMARKER;
        if (yyaccept <= 2) {
            if (yyaccept <= 1) {
                if (yyaccept <= 0) {
                    goto yy50;
                } else {
                    goto yy53;
                }
            } else {
                goto yy55;
            }
        } else {
            if (yyaccept <= 4) {
                if (yyaccept <= 3) {
                    goto yy57;
                } else {
                    goto yy62;
                }
            } else {
                goto yy72;
            }
        }
yy68:
        yyaccept = 3;
        YYMARKER = ++YYCURSOR;
        if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
        yych = *YYCURSOR;
        if (yybm[0+yych] & 32) {
            goto yy68;
        }
        if (yych <= 0x1F) goto yy57;
        if (yych <= '>') goto yy65;
        if (yych <= '?') goto yy71;
yy70:
        yych = *++YYCURSOR;
        if (yych <= '#') {
            if (yych <= ' ') goto yy64;
            if (yych <= '"') goto yy74;
            goto yy64;
        } else {
            if (yych == ';') goto yy64;
            goto yy74;
        }
yy71:
        yyaccept = 5;
        yych = *(YYMARKER = ++YYCURSOR);
        if (yych >= ' ') goto yy66;
yy72:
        {
            *(YYCURSOR-1) = 0;
            _path = unescape(YYTOKEN);
            goto query;
        }
yy73:
        ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
yy74:
        if (yybm[0+yych] & 64) {
            goto yy73;
        }
        if (yych <= '>') goto yy57;
        yych = *++YYCURSOR;
        goto yy72;
yy76:
        ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
        if (yych <= '9') {
            if (yych <= ',') {
                if (yych <= 0x1F) goto yy67;
                goto yy65;
            } else {
                if (yych <= '-') goto yy79;
                if (yych <= '/') goto yy65;
                goto yy79;
            }
        } else {
            if (yych <= 'Z') {
                if (yych <= '?') goto yy65;
                if (yych <= '@') goto yy63;
                goto yy79;
            } else {
                if (yych <= '`') goto yy65;
                if (yych <= 'z') goto yy79;
                goto yy65;
            }
        }
yy77:
        yyaccept = 2;
        YYMARKER = ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
        if (yybm[0+yych] & 128) {
            goto yy77;
        }
        if (yych <= '.') {
            if (yych <= 0x1F) goto yy55;
            if (yych <= '-') goto yy65;
            goto yy76;
        } else {
            if (yych == '@') goto yy63;
            goto yy65;
        }
yy79:
        yyaccept = 2;
        YYMARKER = ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
        if (yych <= '9') {
            if (yych <= '-') {
                if (yych <= 0x1F) goto yy55;
                if (yych <= ',') goto yy65;
                goto yy79;
            } else {
                if (yych <= '.') goto yy76;
                if (yych <= '/') goto yy65;
                goto yy79;
            }
        } else {
            if (yych <= 'Z') {
                if (yych <= '?') goto yy65;
                if (yych <= '@') goto yy63;
                goto yy79;
            } else {
                if (yych <= '`') goto yy65;
                if (yych <= 'z') goto yy79;
                goto yy65;
            }
        }
    }

query:
    if (YYCURSOR >= YYLIMIT)
        return true;
    YYTOKEN =  YYCURSOR;

    {
        YYCTYPE yych;
        static const unsigned char yybm[] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            128, 128, 128,   0, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
            128, 128, 128, 128, 128, 128, 128, 128,
        };
        if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
        yych = *YYCURSOR;
        if (yych <= 0x1F) goto yy85;
        if (yych == '#') goto yy85;
        goto yy84;
yy83:
        {
            *YYCURSOR = 0;
            _query = strNDup(YYTOKEN, SC<int>(YYCURSOR - YYTOKEN));
            YYCURSOR = YYTOKEN; // backtrack to start of query string after terminating it and
            goto params;
        }
yy84:
        yych = *++YYCURSOR;
        goto yy88;
yy85:
        ++YYCURSOR;
        {
            return true;
        }
yy87:
        ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
yy88:
        if (yybm[0+yych] & 128) {
            goto yy87;
        }
        goto yy83;
    }

params:
    if (YYCURSOR >= YYLIMIT)
        return true;
    YYTOKEN =  YYCURSOR;

    {
        YYCTYPE yych;
        static const unsigned char yybm[] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,
            64, 192, 192, 192, 192, 192, 128, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192,  64, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
            192, 192, 192, 192, 192, 192, 192, 192,
        };
        if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
        yych = *YYCURSOR;
        if (yych <= ' ') goto yy95;
        if (yych == '=') goto yy93;
        YYCTXMARKER = YYCURSOR + 1;
        ++YYCURSOR;
        yych = *YYCURSOR;
        goto yy103;
yy92:
        {
            /* No parameters in querystring */
            return true;
        }
yy93:
        ++YYCURSOR;
        yych = *YYCURSOR;
        goto yy98;
yy94:
        {
            *YYTOKEN++ = 0;
            if (*(YYCURSOR - 1) == '&')
                *(YYCURSOR - 1) = 0;
            if (! param) /* format error */
                return true;
            param->_value = unescape(YYTOKEN);
            goto params;
        }
yy95:
        ++YYCURSOR;
        {
            return true;
        }
yy97:
        ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
yy98:
        if (yybm[0+yych] & 64) {
            goto yy97;
        }
        if (yych <= '%') goto yy94;
        yych = *++YYCURSOR;
        goto yy94;
yy100:
        ++YYCURSOR;
        YYCURSOR = YYCTXMARKER;
        {
            NEW(param);
            param->_name = YYTOKEN;
            param->_next = _params;
            _params = param;
            goto params;
        }
yy102:
        YYCTXMARKER = YYCURSOR + 1;
        ++YYCURSOR;
        if (YYLIMIT <= YYCURSOR) YYFILL(1);
        yych = *YYCURSOR;
yy103:
        if (yybm[0+yych] & 128) {
            goto yy102;
        }
        if (yych <= '<') goto yy92;
        goto yy100;
    }

    return false;
}

void URL::freeParams()
{
    while(_params)
    {
        param_t* tmp = _params->_next;
        FREE(_params);
        _params = tmp;
    }
}
