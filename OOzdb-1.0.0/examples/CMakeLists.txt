ADD_EXECUTABLE(example1 example1.cc)
INCLUDE_DIRECTORIES(../OOzdb)
INCLUDE_DIRECTORIES($ENV{HOME}/build/debug-install/include)
TARGET_LINK_LIBRARIES(example1 OOzdb muduo_base_cpp11 muduo_net_cpp11 mysqlclient rt pthread)