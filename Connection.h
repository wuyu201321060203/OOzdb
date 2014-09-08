#ifndef CONNECTION_H
#define CONNECTION_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include "Config.h"
#include "ConnectionPool.h"
#include "TimeOperation.h"
#include "ResultSet.h"
#include "PreparedStatement.h"
#include "URL.h"

class Connection
{
public:

    /*
     * The following functions are virtual functions and must be implemented by
     * subclass
     */

    Connection(ConnectionPool* pool);
    virtual ~Connection();
    virtual int ping();
    virtual int beginTransaction();
    virtual int commit();
    virtual int rollback();
    virtual long long getLastRowId();
    virtual long long rowsChanged();
    virtual void execute(char const* sql,
                         ...)__attribute__((format (printf, 1, 2)));//第一个参数
    //是格式字符串，从第二个参数开始是可变参数列表，从这按照printf检查要求进行检查
    virtual ResultSetPtr executeQuery(char const* sql,
                                      ...)__attribute__((format (printf, 1, 2)));
    virtual PreparedStatementPtr getPreparedStatement(char const* sql,
                                        ...)__attribute__((format (printf, 1, 2)));

    virtual CONST_STDSTR getLastError();
    virtual void onStop();
    virtual void close();

    /*
     *The following functions are inherited by subclasses directly
     */

public:

    void setAvailable(bool isAvailable);
    bool isAvailable();
    time_t getLastAccessedTime();
    bool isInTransaction();
    void setQueryTimeout(int ms);
    int getQueryTimeout();
    void setMaxRows(int rows);
    int getMaxRows();
    URLPtr getURL();
    void clear();

protected:

    ConnectionPool* _pool;
    bool _isAvailable;
    int _isInTransaction;
    int _timeout;
    URLPtr _url;
    time_t _lastAccessedTime;
    int _maxRows;
    PreparedStatementVec _prepared;
    ResultSetPtr _resultSet;

private:

    void freePrepared();
};

typedef boost::shared_ptr<Connection> ConnectionPtr;
typedef std::vector<ConnectionPtr> ConnectionVec;

#endif