#ifndef URL_INCLUDED
#define URL_INCLUDED

#include <boost/shared_ptr.hpp>

namespace OOzdb
{

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
    void URLCreate(char const* url , ...);
    ~URL();
    void clear();
    char const* getProtocol() const;
    char const* getUser() const;
    char const* getPassword() const;
    char const* getHost() const;
    int getPort() const;
    char const* getPath() const;
    char const* getQueryString() const;
    char** getParameterNames();
    char const* getParameter(char const* name);
    char const* toString();
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
	char* _data;
	char* _buffer;
	char* _marker;
    char* _ctx;
    char* _limit;
    char* _token;
    bool  _isCleared;


private:

    bool parseURL();
    void freeParams();
};

typedef boost::shared_ptr<URL> URLPtr;

}

#endif
