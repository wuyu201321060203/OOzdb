#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdarg.h>
#include <stdio.h>
#include <string>

#include <muduo/base/Logging.h>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include <Config.h>

class Exception
{
public:

    explicit Exception()
    {
    }

    explicit Exception(CONST_STDSTR& funcName , CONST_STDSTR& fileName,
              long const& lineNum , CONST_STDSTR& reason):
        _funcName(funcName),
        _fileName(fileName),
        _lineNum(lineNum),
        _reason(reason)
    {
    }

    virtual ~Exception()
    {
    }

    void setFuncName(char const* funcName)
    {
        _funcName = funcName;
    }

    CONST_STDSTR getFuncName() const
    {
        return _funcName;
    }

    void setFileName(char const* fileName)
    {
        _fileName  = fileName;
    }

    CONST_STDSTR getFileName() const
    {
        return _fileName;
    }

    void setLineNum(long const& lineNum)
    {
        _lineNum = lineNum;
    }

    long getLineNum()
    {
        return _lineNum;
    }

    void setReason(char const* reason)
    {
        _reason = reason;
    }

    CONST_STDSTR getReason() const
    {
        return _reason;
    }

private:

    STDSTR _funcName;
    STDSTR _fileName;
    long   _lineNum;
    STDSTR _reason;
};

template<typename T>
void ExceptionThrow(T e , char const* funcName,
                    char const* fileName , long const& lineNum,
    char const* reason , ...)
{
    BOOST_STATIC_ASSERT(boost::is_base_of<Exception , T>::value);
    va_list ap;
    va_start(ap , reason);
    e.setFuncName(funcName);
    e.setFileName(fileName);
    e.setLineNum(lineNum);
    char message[EXCEPTION_MESSAGE_LENGTH + 1];
    int ret = vsnprintf(message , EXCEPTION_MESSAGE_LENGTH, reason, ap);
    if(ret < 0)
        LOG_ERROR << "vsnprintf return error";
    else
        e.setReason(message);
    va_end(ap);
    throw e;
}

#define THROW(e, cause, ...) \
        ExceptionThrow((e()), __func__, __FILE__, __LINE__, cause, ##__VA_ARGS__, NULL)
#endif