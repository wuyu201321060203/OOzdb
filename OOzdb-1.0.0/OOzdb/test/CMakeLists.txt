ADD_EXECUTABLE(OOzdb_unittest poolTest/pool_unittest.cc poolTest/gtest_main.cc)
TARGET_LINK_LIBRARIES(OOzdb_unittest OOzdb muduo_base_cpp11 muduo_net_cpp11
    mysqlclient pthread rt /usr/lib/libgtest.a)
INCLUDE_DIRECTORIES(.. $ENV{HOME}/gtest-1.7.0/include)
INCLUDE_DIRECTORIES($ENV{HOME}/build/debug-install/include)