#include <stdio.h>

#include <Config.h>
#include <Net/URL.h>
#include <Db/ConnectionPool.h>
#include <Db/PreparedStatement.h>
#include <Db/ResultSet.h>
#include <mysql/MysqlConnection.h>

int main(void)
{
    ConnectionPool pool("mysql://root:123@localhost:3306/test");
    //pool.setStopHandler(boost::bind(&onStop));
    pool.start<MysqlConnection>();
    ConnectionPtr conn = pool.getConnection();
    PreparedStatementPtr pre = conn->getPreparedStatement("select name from zild_t where id = ?");
    pre->setInt(1, 2);
    //ResultSetPtr names = pre->executeQuery();
    ResultSetPtr names = pre->executeQuery();
    while(names->next())
       printf("%s\n", (names->getString(1)).c_str());
//    pre = conn->getPreparedStatement("select name from zild_t where id = ?");
    pre->setInt(1 , 1);
    names = pre->executeQuery();
    while(names->next())
       printf("%s\n", (names->getString(1)).c_str());
    /*
    ResultSetPtr names = conn->executeQuery("select name from zild_t where id = 1");
    while(names->next())
    {
        std::string const tmp = names->getString(1);
        char const* tmp1 = tmp.c_str();
        printf("%s\n", tmp1);
    }*/
    return 0;
}
