#ifndef URL_INCLUDED
#define URL_INCLUDED

#include <boost/shared_ptr.hpp>

class URL
{
public:

    struct param_t
    {
        char* _name;
        char* _value;
        param_t* _next;
    };

    URL(char const* url);
    URL(char const* url , ...);
    ~URL();
    void clear();
    char const* getProtocol() const;
    char const* getUser() const;
    char const* getPassword() const;
    char const* getHost() const;
    int getPort() const;
    char const* getPath() const;
    char const* getQueryString() const;
    char const* getParameterNames() const;
    char const* getParameter(char const* name) const;
    char const* toString() const;
    char* unescape(char* url);
    char* escape(char const* url);
    char* normalize(char* path);

private:

    int _port;
    char* _ref;
	char* _path;
	char* _host;
	char* _user;
    char* _qptr;
	char* _query;
	char* _portStr;
	char* _protocol;
	char* _password;
	char* _toString;
    param_t* _params;
    char** _paramNames;
	uchar_t* _data;
	uchar_t* _buffer;
	uchar_t* _marker;
    uchar_t* _ctx;
    uchar_t* _limit;
    uchar_t* _token;


private:

    void parseURL();
    void freeParams();
};

typedef boost::shared_ptr<URL> URLPtr;

#endif
