SET(OOZDB_SRCS
    Db/Connection.cc
    Db/ConnectionPool.cc
    Db/PreparedStatement.cc
    Db/ResultSet.cc
    Mem/StringBuffer.cc
    mysql/MysqlConnection.cc
    mysql/MysqlPreparedStatement.cc
    mysql/MysqlResultSet.cc
    Net/URL.cc
    util/MemoryOperation.cc
    util/StrOperation.cc
    util/TimeOperation.cc
  )

IF(NOT CMAKE_BUILD_NO_EXAMPLES)
    ADD_SUBDIRECTORY(test)
ENDIF()

ADD_LIBRARY(OOzdb SHARED ${OOZDB_SRCS})
ADD_LIBRARY(OOzdb-static STATIC ${OOZDB_SRCS})
SET_TARGET_PROPERTIES(OOzdb-static PROPERTIES OUTPUT_NAME "OOzdb")
SET_TARGET_PROPERTIES( OOzdb PROPERTIES VERSION 1.0 SOVERSION 1  )
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES($ENV{HOME}/build/debug-install/include)

INSTALL(TARGETS OOzdb OOzdb-static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

SET(HEADERS
    Config.h
  )

INSTALL(FILES ${HEADERS} DESTINATION include/OOzdb)
ADD_TEST(mytest ${PROJECT_BINARY_DIR}/bin/OOzdb_unittest)
ENABLE_TESTING()
