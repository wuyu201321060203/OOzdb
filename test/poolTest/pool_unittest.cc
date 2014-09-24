#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>

#include <muduo/base/Thread.h>

#include <Config.h>
#include <Net/URL.h>
#include <Db/ResultSet.h>
#include <Db/PreparedStatement.h>
#include <Db/Connection.h>
#include <Db/ConnectionPool.h>
#include <Exception/AssertException.h>
#include <Exception/SQLException.h>
#include <Exception/ParameterException.h>
#include <mysql/MysqlConnection.h>

#include "gtest/gtest.h"


/**
 * OOzdb connection pool unity tests.
 */
#define BSIZE 2048

#define SCHEMA_MYSQL      "CREATE TABLE zild_t(id INTEGER AUTO_INCREMENT PRIMARY KEY, name VARCHAR(255), percent REAL, image BLOB);"
#define SCHEMA_POSTGRESQL "CREATE TABLE zild_t(id SERIAL PRIMARY KEY, name VARCHAR(255), percent REAL, image BYTEA);"
#define SCHEMA_SQLITE     "CREATE TABLE zild_t(id INTEGER PRIMARY KEY, name VARCHAR(255), percent REAL, image BLOB);"
#define SCHEMA_ORACLE     "CREATE TABLE zild_t(id NUMBER , name VARCHAR(255), percent REAL, image CLOB);"

#if HAVE_STRUCT_TM_TM_GMTOFF
#define TM_GMTOFF tm_gmtoff
#else
#define TM_GMTOFF tm_wday
#endif

char const* testURL = "mysql://root:123@localhost:3306/test";

/*
 * Test1: ConnectionPool's constructure
 */

TEST(ConnectionPoolTest , ConstructTest)
{
    ConnectionPoolPtr pool;
    EXPECT_NO_THROW(
    {
        pool.reset(new ConnectionPool(testURL));
    }
    );
    URLPtr url = pool->getURL();
    ASSERT_TRUE(pool != NULL);
    //EXPECT_STREQ(testURL , url->toString());//TODO
    EXPECT_EQ(SQL_DEFAULT_MAX_CONNECTIONS , pool->getMaxConnections());
    EXPECT_EQ(SQL_DEFAULT_INIT_CONNECTIONS , pool->getInitialConnections());
    EXPECT_EQ(SQL_DEFAULT_CONNECTION_TIMEOUT , pool->getConnectionTimeout());
    EXPECT_TRUE(pool->isFilled() == false);
    EXPECT_TRUE(pool->needDoSweep() == false);
    EXPECT_EQ(DEFAULT_SWEEP_INTERVAL , pool->getSweepInterval());
}

/*
 * Test2: test Bad URL
 */

TEST(ConnectionPoolTest , BadURLTest)
{
    char const* url = NULL;
    EXPECT_THROW(
        {ConnectionPool pool(url);}
     , ParameterException
    );
}

/*
 * Test3: test ConnectionPool start/stop
 */

TEST(ConnectionPoolTest , StartStopTest)
{
    ConnectionPoolPtr pool( new ConnectionPool(testURL) );
    ASSERT_TRUE(pool != NULL);
    pool->start<MysqlConnection>();
    ASSERT_LT( 0 , pool->getSize() );
    EXPECT_EQ( pool->getInitialConnections() , pool->getSize() );
    if(pool->isFilled() && pool->needDoSweep())
        EXPECT_TRUE( ( pool->getReaper() ) != NULL);
    pool->stop();
    EXPECT_EQ( 0 , pool->getSize() );
    EXPECT_THROW(
    {
        char const invalidURL[] = "not://a/database";
        pool.reset( new ConnectionPool(invalidURL) );
        pool->start<MysqlConnection>();
    } , SQLException
    );
}

/*
 * Test4: Connection execute & transaction
 */

TEST(ConnectionPoolTest , ExecuteTransactionTest)
{
    char *data[]=
    {
        "Fry", "Leela", "Bender", "Farnsworth",
        "Zoidberg", "Amy", "Hermes", "Nibbler", "Cubert",
        "Zapp", "Joey Mousepad", 0
    };

    ConnectionPoolPtr pool(new ConnectionPool(testURL));
    ASSERT_TRUE(pool != NULL);
    pool->start<MysqlConnection>();
    ConnectionPtr conn = pool->getConnection();
    ASSERT_TRUE(conn != NULL);
    EXPECT_THROW(
    {
        conn->execute("drop table zild_t");
    } , SQLException);
    EXPECT_NO_THROW(
    {
        conn->execute("%s" , SCHEMA_MYSQL);
        conn->beginTransaction();
        for(int i = 0 ; data[i] ; ++i)
            conn->execute("insert into zild_t(name , percent)values('%s' , %d.%d);",
            data[i] , i + 1 , i);

    });
    EXPECT_EQ( 1 , conn->rowsChanged() );
    EXPECT_EQ( 11 , conn->getLastRowId() );
    EXPECT_NO_THROW(
    {
        conn->commit();
    }
    );
    conn->close();
}

/*
 * Test5: Prepared Statement
 */

TEST(ConnectionPoolTest , PreparedStatement)
{
    char blob[8192];
    char *images[]=
    {
        "Ceci n'est pas une pipe", "Mona Lisa",
        "Bryllup i Hardanger", "The Scream",
        "Vampyre", "Balcony", "Cycle", "Day & Night",
        "Hand with Reflecting Sphere",
        "Drawing Hands", "Ascending and Descending", 0
    };
    ConnectionPoolPtr pool( new ConnectionPool(testURL) );
    ASSERT_TRUE(pool != NULL);
    pool->start<MysqlConnection>();
    ConnectionPtr conn = pool->getConnection();
    ASSERT_TRUE(conn != NULL);
    PreparedStatementPtr st = conn->getPreparedStatement("update zild_t set image=?;");
    ASSERT_TRUE(st != NULL);
    st->setString(1 , "xx");
    st->execute();
    EXPECT_EQ(11 , st->rowsChanged());
    PreparedStatementPtr st1 = conn->getPreparedStatement("update zild_t set image=? where id=?;");
    ASSERT_TRUE(st1 != NULL);
    EXPECT_EQ(2 , st1->getParameterCount());
    int i;
    for(i = 0; images[i]; i++)
    {
        st1->setBlob(1, images[i], static_cast<int>( strlen( images[i]) + 1 ) );
        st1->setInt(2, i + 1);
        st1->execute();
    }
    EXPECT_EQ(1 , st1->rowsChanged());
    EXPECT_NO_THROW({
        st1->setBlob(1, NULL, 0);
        st1->setInt(2, 5);
        st1->execute();
        st1->setString(1, "");
        st1->setInt(2, 1);
        st1->execute();
        memset(blob, 'x', 8192);
        blob[8191] = 0;
        *blob='S'; blob[8190] = 'E';
        st1->setBlob(1, blob, 8192);
        st1->setInt(2, i + 1);
        st1->execute();
        conn->close();
    }
    );
}

/*
 * Test6: test Result sets
 */

TEST(ConnectionPoolTest , ResultSet)
{
    int imagesize = 0;
    ConnectionPoolPtr pool(new ConnectionPool(testURL));
    ASSERT_TRUE(pool != NULL);
    pool->start<MysqlConnection>();
    ConnectionPtr conn = pool->getConnection();
    ASSERT_TRUE(conn != NULL);
    ResultSetPtr rset;
    EXPECT_NO_THROW({
        rset = conn->executeQuery("select id, name, percent, \
                            image from zild_t where id < %d order by id;", 100);
    });
    EXPECT_EQ(4 , rset->getColumnCount());
    int i = 1;
    EXPECT_EQ("id" , rset->getColumnName(i++));
    EXPECT_EQ("name" , rset->getColumnName(i++));
    EXPECT_EQ("percent" , rset->getColumnName(i++));
    EXPECT_EQ("image" , rset->getColumnName(i++));
    while(rset->next())
    {
        int id = rset->getIntByName("id");
        char const* name = (rset->getString(2)).c_str();
        double percent = rset->getDoubleByName("percent");
        char const* blob = static_cast<char const*>( rset->getBlob(4, &imagesize) );
        printf("\t%-5d%-16s%-10.2f%-16.38s\n", id, name ? name : "null", percent, imagesize ? blob : "");
    }
    rset = conn->executeQuery("select image from zild_t where id=11;");
    EXPECT_EQ(1 , rset->getColumnCount());
    // Assert that types are interchangeable and that all data is returned
    while(rset->next())
    {
        char const* image = (rset->getStringByName("image")).c_str();
        void const* blob = rset->getBlobByName("image", &imagesize);
        ASSERT_TRUE((image && blob) == true);
        EXPECT_EQ(25 , strlen(image) + 1);
        EXPECT_EQ(25 , imagesize);
    }
    rset = conn->executeQuery("select id, image from zild_t where id in(1,5,2);");
    while(rset->next())
    {
        int id = rset->getIntByName("id");
        if (id == 1 || id == 5)
            EXPECT_TRUE(rset->isnull(2) == true);
        else
            EXPECT_TRUE(rset->isnull(2) == false);
    }
    conn->setMaxRows(3);
    rset = conn->executeQuery("select id from zild_t;");
    ASSERT_TRUE(rset != NULL);
    i = 0;
    while (rset->next()) i++;
    EXPECT_TRUE(i == 3);
    PreparedStatementPtr pre = conn->getPreparedStatement("select name from zild_t where id=?");
    ASSERT_TRUE(pre != NULL);
    pre->setInt(1, 2);
    ResultSetPtr names = pre->executeQuery();
    ASSERT_TRUE(names != NULL);
    ASSERT_TRUE(names->next());
    EXPECT_EQ("Leela", names->getString(1));
    pre->setInt(1, 1);
    names = pre->executeQuery();
    ASSERT_TRUE(names != NULL);
    ASSERT_TRUE(names->next());//TODO
    EXPECT_EQ("Fry", names->getString(1));
    pre = conn->getPreparedStatement("select name from zild_t;");
    ASSERT_TRUE(pre);
    names = pre->executeQuery();
    ASSERT_TRUE(names);
    for(i = 0 ; rset->next() ; i++);
    EXPECT_EQ(11 , i);
    /* Need to close and release statements before
       we can drop the table, sqlite need this */
    conn->clear();
    conn->execute("drop table zild_t;");
}

/*
 * Test7: reaper start/stop
 */

TEST(ConnectionPoolTest , ReaperTest)
{
    int i;
    ConnectionVec connections(20);
    //url = URL_new(testURL);
    ConnectionPoolPtr pool(new ConnectionPool(testURL));
    ASSERT_TRUE(pool != NULL);
    pool->setInitialConnections(4);
    pool->setMaxConnections(20);
    pool->setConnectionTimeout(4);
    pool->setReaper(4);
    pool->start<MysqlConnection>();
    ASSERT_EQ(4 , pool->getSize());
    for(i = 0 ; i<20 ; i++)
        connections.push_back(pool->getConnection());
    EXPECT_EQ(20 , pool->getSize());
    EXPECT_EQ(0 , pool->getActiveConnections());
    while(!connections.empty())
    {
        (connections.front())->close();
        connections.erase(connections.begin());
    }
    EXPECT_EQ(20 , pool->getActiveConnections());
    EXPECT_EQ(20 , pool->getSize());
    ConnectionPtr con = pool->getConnection();
    sleep(10);
    EXPECT_EQ(5 , pool->getSize()); // 4 initial connections + the one active we got above
    EXPECT_EQ(1 , pool->getActiveConnections());
    con->close();
    pool->stop();
}

/*
 * Test8: Exception handling
 */
void testHelper()
    {
        char *data[]=
        {
            "Fry", "Leela", "Bender", "Farnsworth",
            "Zoidberg", "Amy", "Hermes", "Nibbler", "Cubert",
            "Zapp", "Joey Mousepad", 0
        };
        ConnectionPoolPtr pool(new ConnectionPool(testURL));
        pool->start<MysqlConnection>();
        ConnectionPtr con = pool->getConnection();
            con->beginTransaction();
        for(int i = 0 ; data[i] ; i++)
            con->execute("insert into zild_t (name, percent) values('%s', %d.%d);" , data[i] , i+1 , i);
        con->commit();
    }

TEST(COnnectionPoolTest , ExceptionHandling)
{
    int i;
    ConnectionPoolPtr pool(new ConnectionPool(testURL));
    ASSERT_TRUE(pool != NULL);
    pool->start<MysqlConnection>();
    ConnectionPtr con = pool->getConnection();
    ASSERT_TRUE(con != NULL);
    ResultSetPtr r;
    PreparedStatementPtr p;
    /*
     * The following should work without throwing exceptions
     */
    EXPECT_NO_THROW({
        con->execute("%s", SCHEMA_MYSQL);
    });


    EXPECT_NO_THROW
    ({
        testHelper();
    });
    con->close();
    con = pool->getConnection();

    /* //TODO
    EXPECT_NO_THROW({
        char const* bg[] =
        {
            "Starbuck", "Sharon Valerii",
            "Number Six", "Gaius Baltar", "William Adama",
            "Lee \"Apollo\" Adama", "Laura Roslin", 0
        };
        p = con->getPrepareStatement("insert into zild_t (name) values(?);");
        for(int i = 0, int j = 42 ; bg[i] ; i++, j++)
        {
            p->setString(1, bg[i]);
            p->execute();
        }
    });
    */

    EXPECT_NO_THROW({
        printf("\t\tBattlestar Galactica: \n");
        r = con->executeQuery("select name from zild_t where id > 12;");
        while(r->next())
            printf("\t\t%s\n", (r->getString(1)).c_str());
    });

    con->close();
    /*
     * The following should fail and throw exceptions. The exception error
     * message can be obtained with Exception_frame.message, or from
     * Connection_getLastError(con). Exception_frame.message contains both
     * SQL errors or api errors such as prepared statement parameter index
     * out of range, while Connection_getLastError(con) only has SQL errors
     */
    EXPECT_THROW({
        con = pool->getConnection();
        ASSERT_TRUE(con != NULL);
        con->execute("%s", SCHEMA_MYSQL);
        /* Creating the table again should fail and we
           should not come here */
        printf("\tResult: Test failed -- exception not thrown\n");
    } , SQLException);
    con->close();
    EXPECT_THROW({
        con = pool->getConnection();
        ASSERT_TRUE(con != NULL);
        con->executeQuery("blablabala;");
    } , SQLException);
    con->close();
    EXPECT_THROW({
        printf("\tTesting: Prepared statement query with errors.. ");
        con = pool->getConnection();
        ASSERT_TRUE(con != NULL);
        p = con->getPreparedStatement("blalalalal");
        r = p->executeQuery();
        while(r->next());
    } , SQLException);
    con->close();
    EXPECT_THROW(
    {
        con = pool->getConnection();
        r = con->executeQuery("select id, name from zild_t;");
        while(r->next())
        {
            int id = r->getInt(1);
            char const* name = (r->getString(2)).c_str();
            /* So far so good, now, try access an invalid
               column, which should throw an SQLException */
            int bogus = r->getInt(3);
        }
    } , SQLException);
    con->close();
    EXPECT_THROW(
    {
        con = pool->getConnection();
        r = con->executeQuery("select name from zild_t;");
        while(r->next())
        {
            char const* name = (r->getStringByName("nonexistingcolumnname")).c_str();
        }
    } , SQLException);
    con->close();
    EXPECT_THROW(
    {
        con = pool->getConnection();
        p = con->getPreparedStatement("update zild_t set name = ? where id = ?;");
        p->setInt(3, 123);
    } , SQLException);
    con->close();
    con = pool->getConnection();
    ASSERT_TRUE(con != NULL);
    con->execute("drop table zild_t;");
    con->close();
}

/*
 * Test9: Ensure Capacity
 */

TEST(ConnectionPoolTest , EnsureCapacityTest)
{
    /* Check that MySQL ensureCapacity works for columns that exceed the preallocated buffer and that no truncation is done */
    int myimagesize;
    ConnectionPoolPtr pool(new ConnectionPool(testURL));
    ASSERT_TRUE(pool != NULL);
    pool->start<MysqlConnection>();
    ConnectionPtr con = pool->getConnection();
    ASSERT_TRUE(con != NULL);
    con->execute("CREATE TABLE zild_t(id INTEGER AUTO_INCREMENT PRIMARY KEY, image BLOB, string TEXT);");
    PreparedStatementPtr p = con->getPreparedStatement("insert into zild_t (image, string) values(?, ?);");
    char t[4096];
    memset(t, 'x', 4096);
    t[4095] = 0;
    for(int i = 0 ; i < 4 ; i++)
    {
        p->setBlob(1, t, (i+1)*512); // store successive larger string-blobs to trigger realloc on ResultSet_getBlobByName
        p->setString(2, t);
        p->execute();
    }
    ResultSetPtr r = con->executeQuery("select image, string from zild_t;");
    for(int i = 0 ; r->next() ; i++)
    {
        r->getBlobByName("image", &myimagesize);
        char const* image = (r->getStringByName("image")).c_str(); // Blob as image should be terminated
        char const* string = (r->getStringByName("string")).c_str();
        EXPECT_EQ((i + 1) * 512 , myimagesize);
        EXPECT_EQ((i + 1) * 512 , strlen(image));
        EXPECT_EQ(4095 , strlen(string));
    }
    p = con->getPreparedStatement("select image, string from zild_t;");
    r = p->executeQuery();
    for(int i = 0 ; r->next() ; i++)
    {
        r->getBlobByName("image", &myimagesize);
        char const* image = (r->getStringByName("image")).c_str();
        char const* string = (r->getStringByName("string").c_str());
        EXPECT_EQ((i + 1) * 512 , myimagesize);
        EXPECT_EQ((i + 1) * 512 , strlen(image));
        EXPECT_EQ(4095 , strlen(string));
    }
    con->execute("drop table zild_t;");
    con->close();
}

/*
 * Test10: Date , Time , DateTime and Timestamp
 */

TEST(ConnectionPoolTest , TimeTest)
{
    ConnectionPoolPtr pool(new ConnectionPool(testURL));
    ASSERT_TRUE( pool != NULL );
    setenv("TZ", "Europe/Oslo" , 1);
    pool->start<MysqlConnection>();
    ConnectionPtr con = pool->getConnection();
    con->execute("create table zild_t(d date, t time, dt datetime, ts timestamp)");
    PreparedStatementPtr p = con->getPreparedStatement("insert into zild_t values (?, ?, ?, ?)");
    p->setString(1, "2013-12-28");
    p->setString(2, "10:12:42");
    p->setString(3, "2013-12-28 10:12:42");
    p->setTimestamp(4, 1387066378);
    p->execute();
    ResultSetPtr r = con->executeQuery("select * from zild_t");
    if(r->next())
    {
        struct tm date = r->getDateTime(1);
        struct tm time = r->getDateTime(2);
        struct tm datetime = r->getDateTime(3);
        time_t timestamp = r->getTimestamp(4);
        struct tm timestampAsTm = r->getDateTime(4);
        // Check Date
        EXPECT_EQ(0 , date.tm_hour);
        EXPECT_EQ(2013 , date.tm_year);
        EXPECT_EQ(11 , date.tm_mon);
        EXPECT_EQ(28 , date.tm_mday);
        EXPECT_EQ(0 , date.TM_GMTOFF);
        // Check Time
        EXPECT_EQ(0 , time.tm_year);
        EXPECT_EQ(10 , time.tm_hour);
        EXPECT_EQ(12 , time.tm_min);
        EXPECT_EQ(42 , time.tm_sec);
        EXPECT_EQ(0 , time.TM_GMTOFF);
        // Check datetime
        EXPECT_EQ(2013 , datetime.tm_year);
        EXPECT_EQ(11 , datetime.tm_mon);
        EXPECT_EQ(28 , datetime.tm_mday);
        EXPECT_EQ(10 , datetime.tm_hour);
        EXPECT_EQ(12 , datetime.tm_min);
        EXPECT_EQ(42 , datetime.tm_sec);
        EXPECT_EQ(0 , datetime.TM_GMTOFF);
        // Check timestamp
        EXPECT_EQ(1387066378 , timestamp);
        // Check timestamp as datetime
        EXPECT_EQ(2013 , timestampAsTm.tm_year);
        EXPECT_EQ(11 , timestampAsTm.tm_mon);
        EXPECT_EQ(15 , timestampAsTm.tm_mday);
        EXPECT_EQ(0 , timestampAsTm.tm_hour);
        EXPECT_EQ(12 , timestampAsTm.tm_min);
        EXPECT_EQ(58 , timestampAsTm.tm_sec);
        EXPECT_EQ(0 , timestampAsTm.TM_GMTOFF);
        // Result
        printf("\tResult: Date: %s, Time: %s, DateTime: %s, Timestamp: %s\n",
            (r->getString(1)).c_str(),
            (r->getString(2)).c_str(),
            (r->getString(3)).c_str(),
            (r->getString(4)).c_str()); // SQLite will show unix time, others will show a time string
    }
    con->execute("drop table zild_t;");
    con->close();
}
