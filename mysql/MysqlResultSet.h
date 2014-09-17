#ifndef MYSQLRESULTSET_INCLUDED
#define MYSQLRESULTSET_INCLUDED

#include <mysql/mysql.h>
#include <vector>

#include <Db/ResultSet.h>
#include <Config.h>

class MysqlResultSet : public ResultSet
{
public:

    struct column_t
    {
        my_bool is_null;
        MYSQL_FIELD *field;
        ULONG real_length;
        char* buffer;
    };

    typedef std::vector<column_t> ColumnVec;

    MysqlResultSet(CONST_STDSTR name , void *stmt, int maxRows, int keep);
    ~MysqlResultSet();
    virtual int next();
    virtual int isnull(int columnIndex);
    virtual CONST_STDSTR getString(int columnIndex);
    virtual int getInt(int columnIndex);
    virtual long long  getLLong(int columnIndex);
    virtual double getDouble(int columnIndex);
    virtual void const* getBlob(int columnIndex, int *size);
    virtual time_t getTimestamp(int columnIndex);
    virtual struct tm getDateTime(int columnIndex);
    virtual void clear();
    virtual CONST_STDSTR getColumnName(int columnIndex);
    virtual long getColumnSize(int columnIndex);

private:

    int _stop;
    int _keep;
    int _lastError;
    int _needRebind;
    int _currentRow;
    MYSQL_RES* _meta;
    MYSQL_BIND* _bind;
    MYSQL_STMT* _stmt;
    ColumnVec _columns;

private:

    inline void ensureCapacity(int i);
};

#endif
