#include "ResultSet.h"

#define OK 0//TODO

ResultSet::ResultSet(CONST_STDSTR name):_resultSetName(name)
{
    assert(_resultSetName);
}

ResultSet::~ResultSet()
{
}

int ResultSet::next()
{
    return OK;
}

bool ResultSet::isnull(int columnIndex)
{
    return false;
}

CONST_STDSTR ResultSet::getString(int columnIndex)
{
    return "null"
}

CONST_STDSTR ResultSet::getStringByName(CONST_STDSTR name)
{
    return getString(getIndex(name));
}

int ResultSet::getInt(int columnIndex)
{
    return -1;
}

int ResultSet::getIntByName(CONST_STDSTR name)
{
    return getInt(getIndex(name));
}

long long ResultSet::getLLong(int columnIndex)
{
    return -1;
}

long long ResultSet::getLLongByName(CONST_STDSTR name)
{
    return getLLong(getIndex(name));
}

double ResultSet::getDouble(int columnIndex)
{
    return -1.0;
}

double ResultSet::getDoubleByName(CONST_STDSTR name)
{
    return getDouble(getIndex(name));
}

void const* ResultSet::getBlob(int columnIndex , int* size)
{
    return NULL;
}

void const* ResultSet::getBlobByName(CONST_STDSTR name , int* size)
{
    return getBlob(getIndex(name) , size);
}

time_t ResultSet::getTimestamp(int columnIndex)
{
    return 0;
}

time_t ResultSet::getTimestampByName(CONST_STDSTR name)
{
    return getTimestamp(getIndex(name));
}

struct tm* ResultSet::getDateTime(int columnIndex, struct tm* tm)
{
    return NULL;
}

struct tm* ResultSet::getDateTimeByName(CONST_STDSTR name , struct tm* tm)
{
    return getDateTime(getIndex(name) , tm);
}

void clear()
{
}

int ResultSet::getColumnCount()
{
    return _columnCount;
}

CONST_STDSTR ResultSet::getColumnName(int columnIndex)
{
    return "null";
}

long ResultSet::getColumnSize(int columnIndex)
{
    return -1;
}

int ResultSet::checkAndSetColumnIndex(int columnIndex , int columnCount)
{
    int i = columnIndex - 1;
    if (columnCount <= 0 || i < 0 || i >= columnCount)
        throw( SQLException("Column index is out of range") );
    return i;
}

int ResultSet::getIndex(CONST_STDSTR name)
{
    int i;
    int columns = getColumnCount();
    for (i = 1; i <= columns; i++)
        if ( name == getColumnName(i) )
            return i;
    throw(SQLException(name ? name : "null"));
    return -1;
}