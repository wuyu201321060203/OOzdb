#ifndef RESULTSET_H
#define RESULTSET_H

#include <string>
#include <time.h>//TODO
#include <cassert>

#include <boost/shared_ptr.hpp>

#include <muduo/base/Logging.h>

#include "Exception.h"
#include "Str.h"

class ResultSet
{
public:

    ResultSet(CONST_STDSTR name);
    virtual ~ResultSet();
    virtual int next();
    virtual bool isnull(int columnIndex);
    virtual CONST_STDSTR getString(int columnIndex);
    CONST_STDSTR getStringByName(CONST_STDSTR name);
    virtual int getInt(int columnIndex);
    int getIntByName(CONST_STDSTR name);
    virtual long long  getLLong(int columnIndex);
    long long  getLLongByName(CONST_STDSTR name);
    virtual double getDouble(int columnIndex);
    double getDoubleByName(CONST_STDSTR name);
    virtual void const* getBlob(int columnIndex , int* size);
    void const* getBlobByName(CONST_STDSTR name , int* size);
    virtual time_t getTimestamp(int columnIndex);//TODO
    time_t getTimestampByName(CONST_STDSTR name);//TODO
    virtual struct tm* getDateTime(int columnIndex, struct tm* tm);//TODO
    struct tm* getDateTimeByName(CONST_STDSTR name, struct tm* tm);//TODO
    virtual void clear();

private:

    CONST_STDSTR _resultSetName;

private:

    virtual int getColumnCount();
    virtual CONST_STDSTR getColumnName(int columnIndex);
    virtual long getColumnSize(int columnIndex);

    static inline int checkAndSetColumnIndex(int columnIndex, int columnCount);
};

typedef boost::shared_ptr<ResultSet> ResultSetPtr;

#endif