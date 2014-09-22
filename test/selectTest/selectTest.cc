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
    //pool.setReaper(5);
    pool.start<MysqlConnection>();
    ConnectionPtr con = pool.getConnection();
    con->execute("create table if not exists bleach(name varchar(255), created_at timestamp)");
    PreparedStatementPtr p = con->getPreparedStatement("insert into bleach values (?, ?)");
    PreparedStatementPtr pp = con->getPreparedStatement("select created_at from bleach where name = ?");
    char const* bleach[] =
    {
        "lala", "Rukia Kuchiki", "Orihime Inoue",  "Yasutora \"Chad\" Sado",
        "Kisuke Urahara", "Ury\u016b Ishida", "Renji Abarai", 0
    };
    for(int i = 0 ; bleach[i] ; i++)
    {
        p->setString(1, bleach[i]);
        p->setTimestamp(2, time(NULL) + i);
        p->execute();
    }
    pp->setString(1 , "Orihime Inoue");
    ResultSetPtr rr = pp->executeQuery();
    ResultSetPtr rrr = con->executeQuery("select created_at from bleach where name ='Orihime Inoue'");
    while (rr->next())
    {
        std::string const tmp = rr->getString(1);
        char const* tmp1 = tmp.c_str();
        printf("%s\n" , tmp1);
    }
    while (rrr->next())
    {
        std::string const tmp = rr->getString(1);
        char const* tmp1 = tmp.c_str();
        printf("%s\n" , tmp1);
    }

    /*
    p = con->getPreparedStatement("select name from zild_t where id = ?");
    p->setInt(1 , 2);
    */
    /*
    ResultSetPtr r = con->executeQuery("select name from zild_t where id = 2");
    while (r->next())
    {
        std::string const tmp = r->getString(1);
        char const* tmp1 = tmp.c_str();
        printf("%s\n" , tmp1);
    }
    */
    con->execute("drop table bleach;");
    return 0;
}
