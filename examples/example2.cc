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
    ConnectionPool pool("mysql://root:123@localhost:3306/test");
    pool.setReaper(5);
    pool.start<MysqlConnection>();
    ConnectionPtr con = pool.getConnection<MysqlConnection>();
    PreparedStatementPtr p = con->getPreparedStatement("select name from angel where id = 1");
    ResultSetPtr r = p->executeQuery();
    while(r->next())
    {
        std::string const tmp = r->getString(1);
        char const* tmp1 = tmp.c_str();
        printf("%s\n" , tmp1);
    }
    return 0;
}
