#include "ResultSet.h"

ResultSet::ResultSet(CONST_STDSTR name):_resultSetName(name)
{
    assert(_resultSetName);
}

ResultSet::~ResultSet()
{
}

int ResultSet::getColumnCount()
{
}

CONST_STDSTR ResultSet::getColumnName(int columnIndex)
{
}

long ResultSet::getColumnSize(int columnIndex)
{
}

int ResultSet::next()
{
}

bool ResultSet::isnull(int columnIndex)
{
}

CONST_STDSTR ResultSet::getString(int columnIndex)
{
}

CONST_STDSTR ResultSet::getStringByName(CONST_STDSTR name)
{
    return getString(getIndex(name));
}

int ResultSet::getInt(int columnIndex)
{
}

int ResultSet::getIntByName(CONST_STDSTR name)
{
    return getInt(getIndex(name));
}

long long ResultSet::getLLong(int columnIndex)
{
}

long long ResultSet::getLLongByName(CONST_STDSTR name)
{
    return getLLong(getIndex(name));
}

double ResultSet::getDouble(int columnIndex)
{
}

double ResultSet::getDoubleByName(CONST_STDSTR name)
{
    return getDouble(getIndex(name));
}

void const* ResultSet::getBlob(int columnIndex , int* size)
{
}

void const* ResultSet::getBlobByName(CONST_STDSTR name , int* size)
{
    return getBlob(getIndex(name) , size);
}

time_t ResultSet::getTimestamp(int columnIndex)
{
}

time_t ResultSet::getTimestampByName(CONST_STDSTR name)
{
    return getTimestamp(getIndex(name));
}

struct tm* ResultSet::getDateTime(int columnIndex, struct tm* tm)
{
}

struct tm* ResultSet::getDateTimeByName(CONST_STDSTR name , struct tm* tm)
{
    return getDateTime(getIndex(name) , tm);
}

void clear()
{
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
        if (name == getColumnName(i))
            return i;
    throw(SQLException(name ? name : "null"));
    return -1;
}