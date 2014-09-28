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
    ConnectionPool pool("mysql://root:mysql@192.168.10.121:3306/test");
    pool.setReaper(5);
    pool.start<MysqlConnection>();
    ConnectionPtr con = pool.getConnection<MysqlConnection>();
    con->execute("create table if not exists thinkingYu(id INTEGER AUTO_INCREMENT PRIMARY KEY , name varchar(255))");
    PreparedStatementPtr p = con->getPreparedStatement("insert into thinkingYu values (?, ?)");
    char const* items[] =
    {
        "aaa", "bbb", "ccc",  "ddd", 0
    };
    int i = 0;
    for( ; items[i] ; i++)
    {
        p->setInt(1, i + 1);
        p->setString(2, items[i]);
        p->execute();
    }
    con->execute("insert into thinkingYu values (5 , 'eee')");
    ResultSetPtr r = con->executeQuery("select name from thinkingYu where id = 3");
    PreparedStatementPtr p1 = con->getPreparedStatement("select name from thinkingYu where id = ?");
    p1->setInt(1 , i + 1);
    ResultSetPtr r1 = p1->executeQuery();
    while(r->next())
    {
        std::string const tmp = r->getString(1);
        char const* tmp1 = tmp.c_str();
        printf("%s\n" , tmp1);
    }
    while(r1->next())
    {
        std::string const tmp = r1->getString(1);
        char const* tmp1 = tmp.c_str();
        printf("%s\n" , tmp1);
    }
    con->execute("drop table thinkingYu;");
    return 0;
}
