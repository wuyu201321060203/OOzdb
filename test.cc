#include <stdio.h>

#include "Config.h"
#include "URL.h"
#include "ConnectionPool.h"
#include "MysqlConnection.h"
#include "PreparedStatement.h"
#include "ResultSet.h"

#include <boost/bind.hpp>

void onStop()
{
    if (mysql_get_client_version() >= 50003)
        mysql_library_end();
    else
        mysql_server_end();
}

int main(void)
{
    ConnectionPool pool("mysql://root:123@localhost:3306/test");
    pool.setStopHandler(boost::bind(&onStop));
    pool.start<MysqlConnection>();
    ConnectionPtr con = pool.getConnection();
    con->execute("create table if not exists bleach(name varchar(255), created_at timestamp)");
    PreparedStatementPtr p =
        con->getPreparedStatement("insert into bleach values (?, ?)");
    const char *bleach[] =
    {
        "Ichigo Kurosaki", "Rukia Kuchiki", "Orihime Inoue",  "Yasutora \"Chad\" Sado",
        "Kisuke Urahara", "Ury\u016b Ishida", "Renji Abarai", 0
    };
    for(int i = 0 ; bleach[i] ; i++)
    {
        p->setString(1, bleach[i]);
        p->setTimestamp(2, time(NULL) + i);
        p->execute();
    }
    ResultSetPtr r = con->executeQuery(" select name , created_at from bleach where created_at = '2014-08-12 16:27:40' ");
    while (r->next())
        printf("%-22s\t %s\n", (r->getString(1)).c_str() , (r->getString(2)).c_str());
    con->execute("drop table bleach;");
    return 0;
}
