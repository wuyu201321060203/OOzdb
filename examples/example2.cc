#include <stdio.h>

#include <Config.h>
#include <Net/URL.h>
#include <Db/ConnectionPool.h>
#include <Db/PreparedStatement.h>
#include <Db/ResultSet.h>
#include <mysql/MysqlConnection.h>

using namespace OOzdb;

int main(void)
{
    ConnectionPool pool("mysql://root:123@localhost:3306/DM");
    pool.setReaper(5);
    pool.start<MysqlConnection>();
    ConnectionPtr con = pool.getConnection<MysqlConnection>();
    ResultSetPtr ret;
    int i = 0;
    while(i < 1000)
    {
        ret = con->executeQuery("select id from USER_INFo where name = 'ddcnmb'");
        ++i;
    }
    return 0;
}
