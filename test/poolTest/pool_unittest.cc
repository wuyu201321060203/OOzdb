#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#include <muduo/base/Thead.h>

#include <Config.h>
#include <Net/URL.h>
#include <Db/ResultSet.h>
#include <Db/PreparedStatement.h>
#include <Db/Connection.h>
#include <Db/ConnectionPool.h>
#include <Exception/AssertException.h>
#include <Exception/SQLException.h>

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

char const* testUrl = "mysql://root:123@localhost:3306/test";

/*
 * Test1:test ConnectionPool's constructure and destructure
 */

TEST(ConnectionPoolTest , Construct/Destruct Test)
{
    try
    {
        ConnectionPoolPtr pool(new ConnectionPool(testUrl));
    }
    catch(...)
    {
        printf("wrong\n");//TODO
    }
    URLPtr url = pool->getURL();
    ASSERT_TRUE(pool != NULL);
    EXPECT_STREQ(testurl , url->toString());
    EXPECT_EQ(SQL_DEFAULT_MAX_CONNECTIONS , pool->getMaxConnections());
    EXPECT_EQ(SQL_DEFAULT_INIT_CONNECTIONS , pool->getInitialConnections());
    EXPECT_EQ(SQL_DEFAULT_CONNECTION_TIMEOUT , pool->getConnectionTimeout());
    EXPECT_TRUE(_filled == false);
    EXPECT_TRUE(_doSweep == false);
    EXPECT_EQ(DEFAULT_SWEEP_INTERVAL , pool->getSweepInterval());
    EXPECT_TRUE(_stopped == true);
}

/*
 * Test2: test NULL URL
 */

TEST(ConnectionPoolTest , NULLURLTest)
{
    char const* url = NULL;
    try
    {
        ConnectionPool pool(url);
        printf("Wrong here")//TODO
    }
    catch(ParameterException const& e)
    {
        EXPECT_EQ("url is invalid" , e.getReason());
    }
    ASSERT_TRUE( (pool.getURL()) == NULL);
}

/*
 * Test3: test ConnectionPool start/stop
 */

TEST(ConnectionPoolTest , Start/Stop Test)
{
    ConnectionPoolPtr pool( new ConnectionPool(testurl) );//TODO 测试共享变量
    ASSERT_TRUE(pool != NULL);
    pool->start<MysqlConnection>();
    ASSERT_GT( 0 , pool->getSize() );
    EXPECT_EQ( pool->getInitialConnections() , pool->getSize() );
    if(pool->isFilled() && pool->needDoSweep())
        EXPECT_TRUE( ( pool->getReaper() ) != NULL)
    pool->stop();
    EXPECT_EQ( 0 , pool->getSize() );
    try
    {
        char const invalidURL[] = "not://a/database";
        pool.reset( new ConnectionPool(url) );
        ASSERT_TRUE(pool != NULL);
        pool->start<MysqlConnection>();
        printf("wrong\n");//TODO
    }
    catch(SQLException const& errorItem)//TODO
    {
        printf("normal\n");
    }
}

/*
 * Test4: Connection execute & transaction
 */

TEST(ConnectionPoolTest , Execute/Transaction)
{
    char *data[]=
    {
        "Fry", "Leela", "Bender", "Farnsworth",
        "Zoidberg", "Amy", "Hermes", "Nibbler", "Cubert",
        "Zapp", "Joey Mousepad", 0
    };

    ConnectionPoolPtr pool(new ConnectionPool(testURL));
    ASSERT_TRUE(pool != NULL);
    ConnectionPtr conn = pool->getConnection();
    ASSERT_TRUE(conn != NULL);
    try
    {
        conn->execute("drop table zild_t;");
        printf("wrong\n");
    }
    catch(...)//TODO
    {
        normal;
    }
    try
    {
        conn->execute("%s" , SCHEMA_MYSQL);
        conn->beginTransaction();
        for(int i = 0 ; data[i] ; ++i)
            conn->execute("insert into zild_t(name , percent)values('%s' , %d.%d);",
            data[i] , i + 1 , i);

    }
    catch(...)
    {
        wrong;
    }
    EXPECT_EQ( 1 , conn->getRowsChanged() );
    EXPECT_EQ( 12 , conn->getLastRowId() );
    try conn->commit();
    catch(...) wrong;
    conn->close();
}

/*
 * Test5: Prepared Statement
 */

Test(ConnectionPoolTest , PreparedStatement)
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
    ConnectionPoolPtr pool( new ConnectionPool(testUrl) );
    Assert_TRUE(pool != NULL);
    ConnectionPtr conn = pool->getConnection();
    Assert_TRUE(conn != NULL);
    PreparedStatementPtr st = conn->getPreparedStatement("update zild_t set image=?;");
    Assert_TRUE(st != NULL);
    st->setString(1 , "");
    st->execute();
    EXPECT_TRUE(12 , st->getRowsChanged());
    PreparedStatementPtr st1 = conn->getPreparedStatement("update zild_t set image=? where id=?;");
    ASSERT_TRUE(st1 != NULL);
    EXPECT_TRUE(2 , st1->getParameterCount());
    for(i = 0; images[i]; i++)
    {
        st1->setBlob(1, images[i], static_cast<int>( strlen( images[i]) + 1 ) );
        st1->setInt(2, i + 1);
        st1->execute();
    }
    EXPECT_EQ(1 , st1->getRowsChanged());
    try
    {
        st1->setBlob(1, NULL, 0);
        st1->setInt(2, 5);
        st1->execute();
        st1->setString(1, NULL);
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
    catch(...)
    {
        wrong;
    }
}

/*
 * Test6: test Result sets
 */

TEST(ConnectionPoolTest , ResultSet)
{
    int imagesize = 0;
    ConnectionPoolPtr pool(new ConnectionPool(testUrl));
    ASSERT_TRUE(pool != NULL);
    ConnectionPtr conn = pool->getConnection();
    ASSERT_TRUE(conn != NULL);
    ResultSetPtr rset = conn->executeQuery("select id, name, percent, \
                            image from zild_t where id < %d order by id;", 100);
    ASSERT_TRUE(rset != NULL);
    EXPECT_EQ(4 , rset->getColumnCount());
    int i = 1;
    //ResultSet_T names;
    //PreparedStatement_T pre;
    EXPECT_STREQ("id" , rset->getColumnName(i++));
    EXPECT_STREQ("name" , rset->getColumnName(i++));
    EXPECT_STREQ("percent" , rset->getColumnName(i++));
    EXPECT_STREQ("image" , rset->getColumnName(i++));
    while(rset->next())
    {
        int id = rset->getIntByName("id");
        char const* name = rset->getString(2);
        double percent = rset->getDoubleByName("percent");
        char const* blob = static_cast<char*>( rset->getBlob(4, &imagesize) );
        printf("\t%-5d%-16s%-10.2f%-16.38s\n", id, name ? name : "null", percent, imagesize ? blob : "");
    }
    rset = conn->executeQuery("select image from zild_t where id=12;");
    EXPECT_TRUE(1 , rset->getColumnCount());
    // Assert that types are interchangeable and that all data is returned
    while(rset->next())
    {
        char const* image = rset->getStringByName("image");
        void const* blob = rset->getBlobByName("image", &imagesize);
        ASSERT_TRUE(image && blob == true);
        EXPECT_EQ(8192 , strlen(image) + 1);
        EXPECT_EQ(8192 , imagesize);
    }
    rset = conn->executeQuery("select id, image from zild_t where id in(1,5,2);");
    while(rset->next())
    {
        int id = rset->getIntByName("id");
        if (id == 1 || id == 5)
            EXPECT_TRUE(rset->isnull(2) == true);
        else
            EXPECT_TRUE(rset->isnull(rset, 2) == false);
    }
    conn->setMaxRows(3);
    rset = conn->executeQuery("select id from zild_t;");
    ASSERT_TRUE(rset != NULL);
    i = 0;
    while (rset->next()) i++;
    EXPECT_TRUE(i == 3);
    conn->setMaxRows(0);
    pre = conn->getPrepareStatement("select name from zild_t where id=?");
    ASSERT_TRUE(pre != NULL);
    pre->setInt(1, 2);
    names = Pre->executeQuery();
    ASSERT_TRUE(names != NULL);
    ASSERT_TRUE(names->next());
    EXPECT_TRUE("Leela", names->getString(1));
    Pre->setInt(1, 1);
    names = Pre->executeQuery();
    ASSERT_TRUE(names != NULL);
    ASSERT_TRUE(names->next());
    EXPECT_EQ("Fry", names->getString(1));
    pre = Conn->getPrepareStatement("select name from zild_t;");
    ASSERT_TRUE(pre);
    names = Pre->executeQuery();
    ASSERT_TRUE(names);
    for(i = 0 ; rset->next() ; i++);
    EXPECT_EQ(12 , i);
    /* Need to close and release statements before
       we can drop the table, sqlite need this */
    conn->clear();
    Conn->execute("drop table zild_t;");
}

/*
 * Test7: reaper start/stop
 */

Test(ConnectionPoolTest , ReaperTest)
{
    int i;
    ConnectionVec connctions(20);
    //url = URL_new(testURL);
    ConnectionPoolPtr pool(new ConnectionPool(url));
    ASSERT_TRUE(pool != NULL);
    pool->setInitialConnections(4);
    pool->setMaxConnections(20);
    pool->setConnectionTimeout(4);
    pool->setReaper(4);
    pool->start<MysqlConnection>();
    ASSERT_TRUE(4 , pool->getSize());
    for(i = 0; i<20; i++)
        connections.push_back(pool->getConnection());
    EXPECT_EQ(20 , pool->getSize());
    EXPECT_EQ(20 , pool->getActive());
    while(!connections.empty())
        ;
    assert(ConnectionPool_active(pool) == 0);
    assert(ConnectionPool_size(pool) == 20);
    printf("success\n");
    printf("Please wait 10 sec for reaper to harvest closed connections..");
    Connection_T con = ConnectionPool_getConnection(pool); // Activate one connection to verify the reaper does not close any active
    fflush(stdout);
    sleep(10);
    assert(5 == ConnectionPool_size(pool)); // 4 initial connections + the one active we got above
    assert(1 == ConnectionPool_active(pool));
    printf("success\n");
    Connection_close(con);
    ConnectionPool_stop(pool);
    ConnectionPool_free(&pool);
    Vector_free(&v);
    assert(pool==NULL);
    URL_free(&url);

}

/*
 * Test8: Exception handling
 */

Test(COnnectionPoolTest , ExceptionHandling)
{
    int i;
    Connection_T con;
    ResultSet_T result;
    url = URL_new(testURL);
    pool = ConnectionPool_new(url);
    assert(pool);
    ConnectionPool_setAbortHandler(pool, TabortHandler);
    ConnectionPool_start(pool);
    con = ConnectionPool_getConnection(pool);
    assert(con);
    /*
     * The following should work without throwing exceptions
     */
    TRY
    {
        Connection_execute(con, "%s", schema);
    }
    ELSE
    {
        printf("\tResult: Creating table zild_t failed -- %s\n", Exception_frame.message);
        assert(false); // Should not fail
    }
    END_TRY;
    TRY
    {
        Connection_beginTransaction(con);
        for (i = 0; data[i]; i++)
            Connection_execute(con, "insert into zild_t (name, percent) values('%s', %d.%d);", data[i], i+1, i);
        Connection_commit(con);
        printf("\tResult: table zild_t successfully created\n");
    }
    ELSE
    {
        printf("\tResult: Test failed -- %s\n", Exception_frame.message);
        assert(false); // Should not fail
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    assert((con = ConnectionPool_getConnection(pool)));
    TRY
    {
        int i, j;
        const char *bg[]= {"Starbuck", "Sharon Valerii",
            "Number Six", "Gaius Baltar", "William Adama",
            "Lee \"Apollo\" Adama", "Laura Roslin", 0};
        PreparedStatement_T p = Connection_prepareStatement
            (con, "insert into zild_t (name) values(?);");
        /* If we did not get a statement, an SQLException is thrown
           and we will not get here. So we can safely use the
           statement now. Likewise, below, we do not have to
           check return values from the statement since any error
           will throw an SQLException and transfer the control
           to the exception handler
           */
        for (i = 0, j = 42; bg[i]; i++, j++) {
            PreparedStatement_setString(p, 1, bg[i]);
            PreparedStatement_execute(p);
        }
    }
    CATCH(SQLException)
    {
        printf("\tResult: prepare statement failed -- %s\n", Exception_frame.message);
        assert(false);
    }
    END_TRY;
    TRY
    {
        printf("\t\tBattlestar Galactica: \n");
        result = Connection_executeQuery(con, "select name from zild_t where id > 12;");
        while (ResultSet_next(result))
            printf("\t\t%s\n", ResultSet_getString(result, 1));
    }
    CATCH(SQLException)
    {
        printf("\tResult: resultset failed -- %s\n", Exception_frame.message);
        assert(false);
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    /*
     * The following should fail and throw exceptions. The exception error
     * message can be obtained with Exception_frame.message, or from
     * Connection_getLastError(con). Exception_frame.message contains both
     * SQL errors or api errors such as prepared statement parameter index
     * out of range, while Connection_getLastError(con) only has SQL errors
     */
    TRY
    {
        assert((con = ConnectionPool_getConnection(pool)));
        Connection_execute(con, "%s", schema);
        /* Creating the table again should fail and we
           should not come here */
        printf("\tResult: Test failed -- exception not thrown\n");
        exit(1);
    }
    CATCH(SQLException)
    {
        Connection_close(con);
    }
    END_TRY;
    TRY
    {
        assert((con = ConnectionPool_getConnection(pool)));
        printf("\tTesting: Query with errors.. ");
        Connection_executeQuery(con, "blablabala;");
        printf("\tResult: Test failed -- exception not thrown\n");
        exit(1);
    }
    CATCH(SQLException)
    {
        printf("ok\n");
        Connection_close(con);
    }
    END_TRY;
    TRY
    {
        printf("\tTesting: Prepared statement query with errors.. ");
        assert((con = ConnectionPool_getConnection(pool)));
        PreparedStatement_T p = Connection_prepareStatement(con, "blablabala;");
        ResultSet_T r = PreparedStatement_executeQuery(p);
        while(ResultSet_next(r));
        printf("\tResult: Test failed -- exception not thrown\n");
        exit(1);
    }
    CATCH(SQLException)
    {
        printf("ok\n");
        Connection_close(con);
    }
    END_TRY;
    TRY
    {
        assert((con = ConnectionPool_getConnection(pool)));
        printf("\tTesting: Column index out of range.. ");
        result = Connection_executeQuery(con, "select id, name from zild_t;");
        while (ResultSet_next(result)) {
            int id = ResultSet_getInt(result, 1);
            const char *name = ResultSet_getString(result, 2);
            /* So far so good, now, try access an invalid
               column, which should throw an SQLException */
            int bogus = ResultSet_getInt(result, 3);
            printf("\tResult: Test failed -- exception not thrown\n");
            printf("%d, %s, %d", id, name, bogus);
            exit(1);
        }
    }
    CATCH(SQLException)
    {
        printf("ok\n");
        Connection_close(con);
    }
    END_TRY;
    TRY
    {
        assert((con = ConnectionPool_getConnection(pool)));
        printf("\tTesting: Invalid column name.. ");
        result = Connection_executeQuery(con, "select name from zild_t;");
        while (ResultSet_next(result)) {
            const char *name = ResultSet_getStringByName(result, "nonexistingcolumnname");
            printf("%s", name);
            printf("\tResult: Test failed -- exception not thrown\n");
            exit(1);
        }
    }
    CATCH(SQLException)
    {
        printf("ok\n");
        Connection_close(con);
    }
    END_TRY;
    TRY
    {
        assert((con = ConnectionPool_getConnection(pool)));
        PreparedStatement_T p = Connection_prepareStatement(con, "update zild_t set name = ? where id = ?;");
        printf("\tTesting: Parameter index out of range.. ");
        PreparedStatement_setInt(p, 3, 123);
        printf("\tResult: Test failed -- exception not thrown\n");
        exit(1);
    }
    CATCH(SQLException)
    {
        printf("ok\n");
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    assert((con = ConnectionPool_getConnection(pool)));
    Connection_execute(con, "drop table zild_t;");
    Connection_close(con);
    ConnectionPool_stop(pool);
    ConnectionPool_free(&pool);
    assert(pool==NULL);
    URL_free(&url);

}

/*
 * Test9: Ensure Capacity
 */

Test(ConnectionPoolTest , EnsureCapacityTest)
{
    /* Check that MySQL ensureCapacity works for columns that exceed the preallocated buffer and that no truncation is done */
    if ( Str_startsWith(testURL, "mysql")) {
        int myimagesize;
        url = URL_new(testURL);
        pool = ConnectionPool_new(url);
        assert(pool);
        ConnectionPool_start(pool);
        Connection_T con = ConnectionPool_getConnection(pool);
        assert(con);
        Connection_execute(con, "CREATE TABLE zild_t(id INTEGER AUTO_INCREMENT PRIMARY KEY, image BLOB, string TEXT);");
        PreparedStatement_T p = Connection_prepareStatement(con, "insert into zild_t (image, string) values(?, ?);");
        char t[4096];
        memset(t, 'x', 4096);
        t[4095] = 0;
        for (int i = 0; i < 4; i++) {
            PreparedStatement_setBlob(p, 1, t, (i+1)*512); // store successive larger string-blobs to trigger realloc on ResultSet_getBlobByName
            PreparedStatement_setString(p, 2, t);
            PreparedStatement_execute(p);
        }
        ResultSet_T r = Connection_executeQuery(con, "select image, string from zild_t;");
        for (int i = 0; ResultSet_next(r); i++) {
            ResultSet_getBlobByName(r, "image", &myimagesize);
            const char *image = ResultSet_getStringByName(r, "image"); // Blob as image should be terminated
            const char *string = ResultSet_getStringByName(r, "string");
            assert(myimagesize == (i+1)*512);
            assert(strlen(image) == ((i+1)*512));
            assert(strlen(string) == 4095);
        }
        p = Connection_prepareStatement(con, "select image, string from zild_t;");
        r = PreparedStatement_executeQuery(p);
        for (int i = 0; ResultSet_next(r); i++) {
            ResultSet_getBlobByName(r, "image", &myimagesize);
            const char *image = ResultSet_getStringByName(r, "image");
            const char *string = (char*)ResultSet_getStringByName(r, "string");
            assert(myimagesize == (i+1)*512);
            assert(strlen(image) == ((i+1)*512));
            assert(strlen(string) == 4095);
        }
        Connection_execute(con, "drop table zild_t;");
        Connection_close(con);
        ConnectionPool_stop(pool);
        ConnectionPool_free(&pool);
        URL_free(&url);
    }
}

/*
 * Test10: Date , Time , DateTime and Timestamp
 */

Test10(ConnectionPoolTest , TimeTest)
{
    url = URL_new(testURL);
    pool = ConnectionPool_new(url);
    assert(pool);
    setenv("TZ", "Europe/Oslo" , 1);
    ConnectionPool_start(pool);
    Connection_T con = ConnectionPool_getConnection(pool);
    if (Str_startsWith(testURL, "postgres"))
        Connection_execute(con, "create table zild_t(d date, t time, dt timestamp, ts timestamp)");
    else if (Str_startsWith(testURL, "oracle"))
        Connection_execute(con, "create table zild_t(d date, t time, dt date, ts timestamp)");
    else
        Connection_execute(con, "create table zild_t(d date, t time, dt datetime, ts timestamp)");
    PreparedStatement_T p = Connection_prepareStatement(con, "insert into zild_t values (?, ?, ?, ?)");
    PreparedStatement_setString(p, 1, "2013-12-28");
    PreparedStatement_setString(p, 2, "10:12:42");
    PreparedStatement_setString(p, 3, "2013-12-28 10:12:42");
    PreparedStatement_setTimestamp(p, 4, 1387066378);
    PreparedStatement_execute(p);
    ResultSet_T r = Connection_executeQuery(con, "select * from zild_t");
    if (ResultSet_next(r)) {
        struct tm date = ResultSet_getDateTime(r, 1);
        struct tm time = ResultSet_getDateTime(r, 2);
        struct tm datetime = ResultSet_getDateTime(r, 3);
        time_t timestamp = ResultSet_getTimestamp(r, 4);
        struct tm timestampAsTm = ResultSet_getDateTime(r, 4);
        // Check Date
        assert(date.tm_hour == 0);
        assert(date.tm_year == 2013);
        assert(date.tm_mon == 11); // Remember month - 1
        assert(date.tm_mday == 28);
        assert(date.TM_GMTOFF == 0);
        // Check Time
        assert(time.tm_year == 0);
        assert(time.tm_hour == 10);
        assert(time.tm_min == 12);
        assert(time.tm_sec == 42);
        assert(time.TM_GMTOFF == 0);
        // Check datetime
        assert(datetime.tm_year == 2013);
        assert(datetime.tm_mon == 11); // Remember month - 1
        assert(datetime.tm_mday == 28);
        assert(datetime.tm_hour == 10);
        assert(datetime.tm_min == 12);
        assert(datetime.tm_sec == 42);
        assert(datetime.TM_GMTOFF == 0);
        // Check timestamp
        assert(timestamp == 1387066378);
        // Check timestamp as datetime
        assert(timestampAsTm.tm_year == 2013);
        assert(timestampAsTm.tm_mon == 11); // Remember month - 1
        assert(timestampAsTm.tm_mday == 15);
        assert(timestampAsTm.tm_hour == 0);
        assert(timestampAsTm.tm_min == 12);
        assert(timestampAsTm.tm_sec == 58);
        assert(timestampAsTm.TM_GMTOFF == 0);
        // Result
        printf("\tResult: Date: %s, Time: %s, DateTime: %s, Timestamp: %s\n",
            ResultSet_getString(r, 1),
            ResultSet_getString(r, 2),
            ResultSet_getString(r, 3),
            ResultSet_getString(r, 4)); // SQLite will show unix time, others will show a time string
    }
    Connection_execute(con, "drop table zild_t;");
    Connection_close(con);
    ConnectionPool_stop(pool);
    ConnectionPool_free(&pool);
    assert(pool==NULL);
    URL_free(&url);
}
