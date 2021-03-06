#include <Exception/SQLException.h>

#include "ResultSet.h"

using namespace OOzdb;

ResultSet::ResultSet(CONST_STDSTR name):
    _resultSetName(name),
    _maxRows(DEFAULT_MAX_ROWS),
    _columnCount(0),
    _isCleared(false)
{
    assert(!_resultSetName.empty());
}

ResultSet::~ResultSet()
{
}

int ResultSet::next()
{
    return SUCCESSFUL;
}

int ResultSet::isnull(int columnIndex)
{
    return false;
}

CONST_STDSTR ResultSet::getString(int columnIndex)
{
    return BADSTR;
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
    time_t t = 0;
    return t;
}

time_t ResultSet::getTimestampByName(CONST_STDSTR name)
{
    return getTimestamp(getIndex(name));
}

struct tm ResultSet::getDateTime(int columnIndex)
{
    struct tm t = {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0};//??
    return t;
}

struct tm ResultSet::getDateTimeByName(CONST_STDSTR name)
{
    return getDateTime(getIndex(name));
}

void ResultSet::clear()
{
    _isCleared = true;
}

void ResultSet::setClearFlag()
{
    _isCleared = true;
}

void ResultSet::unsetClearFlag()
{
    _isCleared = false;
}

bool ResultSet::isCleared()
{
    return _isCleared;
}

int ResultSet::getColumnCount()
{
    return _columnCount;
}

CONST_STDSTR ResultSet::getColumnName(int columnIndex)
{
    return BADSTR;
}

long ResultSet::getColumnSize(int columnIndex)
{
    return -1;
}

int ResultSet::checkAndSetColumnIndex(int columnIndex)
{
    int i = columnIndex - 1;
    if (_columnCount <= 0 || i < 0 || i >= _columnCount)
        THROW(SQLException , "Column index is out of range");
    return i;
}

int ResultSet::getIndex(CONST_STDSTR name)
{
    int i;
    int columns = getColumnCount();
    for (i = 1; i <= columns; i++)
        if ( name == getColumnName(i) )
            return i;
    THROW(SQLException , (name.empty() ? "null" : name.c_str()));
    return -1;
}