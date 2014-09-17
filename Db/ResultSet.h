#ifndef RESULTSET_H
#define RESULTSET_H

#include <boost/shared_ptr.hpp>

#include <util/TimeOperation.h>
#include <Config.h>

class ResultSet
{
public:

    ResultSet(CONST_STDSTR name);
    virtual ~ResultSet();
    virtual int next();
    virtual int isnull(int columnIndex);
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
    virtual time_t getTimestamp(int columnIndex);
    time_t getTimestampByName(CONST_STDSTR name);
    virtual struct tm getDateTime(int columnIndex);
    struct tm getDateTimeByName(CONST_STDSTR name);
    virtual void clear();
    void setClearFlag();
    void unsetClearFlag();
    bool isCleared();
    int getColumnCount();
    virtual CONST_STDSTR getColumnName(int columnIndex);
    virtual long getColumnSize(int columnIndex);

protected:

    CONST_STDSTR _resultSetName;
    int _maxRows;
    int _columnCount;

protected:

    int checkAndSetColumnIndex(int columnIndex);
    int getIndex(CONST_STDSTR name);

private:

    bool _isCleared;
};

typedef boost::shared_ptr<ResultSet> ResultSetPtr;

#endif