MUDUO_DIRECTORY = $(HOME)/build/debug-install
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/lib
MUDUO_INCLUDE = $(MUDUO_DIRECTORY)/include
OOZDB_INCLUDE = $(HOME)/gitWork/OOzdb
# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = $(HOME)/gtest-1.7.0

# Where to find user code.
USER_DIR = .

CXX = g++
LDFLAGS = -L$(MUDUO_LIBRARY) -lmuduo_net_cpp11 -lmuduo_base_cpp11 -lpthread -lrt\
		  -lmysqlclient

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -ggdb3 -Wall -Wextra -pthread -DTEST -I$(MUDUO_INCLUDE) -I$(OOZDB_INCLUDE)

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = pool_unittest

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.

all : $(TESTS)

clean :
	rm -f $(TESTS)

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

pool_unittest.o : $(USER_DIR)/pool_unittest.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^

pool_unittest : gtest_main.a pool_unittest.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ Connection.o PreparedStatement.o ResultSet.o ConnectionPool.o MysqlConnection.o MysqlPreparedStatement.o MysqlResultSet.o StringBuffer.o StrOperation.o MemoryOperation.o TimeOperation.o URL.o -o $@ $(LDFLAGS)
