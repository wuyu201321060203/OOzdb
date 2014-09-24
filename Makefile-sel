MUDUO_DIRECTORY = $(HOME)/build/debug-install
MUDUO_INCLUDE = $(MUDUO_DIRECTORY)/include
OOZDB_INCLUDE = $(HOME)/gitWork/OOzdb
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/lib
SRC = ./
CXXFLAGS = -g -O0 -Wall -Wextra \
	       -Wno-unused-parameter \
           -Wold-style-cast -Woverloaded-virtual \
           -Wpointer-arith -Wshadow -Wwrite-strings \
           -march=native -rdynamic -std=c++11 -DTEST\
           -I$(MUDUO_INCLUDE)\
		   -I$(OOZDB_INCLUDE)
LDFLAGS = -L$(MUDUO_LIBRARY) -lmuduo_net_cpp11 -lmuduo_base_cpp11 -lpthread -lrt\
-lmysqlclient

BIN_PROGRAM = selectTest

AR = ar
ARFLAGS = -r

all: $(BIN_PROGRAM)

clean:
	test -z "$(BIN_PROGRAM)" || rm -f $(BIN_PROGRAM)
	rm -f *.o core

$(BIN_PROGRAM): Connection.o PreparedStatement.o ResultSet.o ConnectionPool.o\
	  MysqlConnection.o MysqlPreparedStatement.o MysqlResultSet.o \
	  StringBuffer.o StrOperation.o MemoryOperation.o TimeOperation.o URL.o selectTest.o
	g++ $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

OOzdb.a: OOzdb-all.o
	$(AR) $(ARFLAGS) $@ $^

OOzdb-all.o: $(SRC)/OOzdb-all.cc
	g++ $(CXXFLAGS) -c $^

Connection.o: $(SRC)/Db/Connection.cc
	g++ $(CXXFLAGS) -c $^

PreparedStatement.o: $(SRC)/Db/PreparedStatement.cc
	g++ $(CXXFLAGS) -c $^

ResultSet.o: $(SRC)/Db/ResultSet.cc
	g++ $(CXXFLAGS) -c $^

ConnectionPool.o: $(SRC)/Db/ConnectionPool.cc
	g++ $(CXXFLAGS) -c $^

MysqlConnection.o: $(SRC)/mysql/MysqlConnection.cc
	g++ $(CXXFLAGS) -c $^

MysqlPreparedStatement.o: $(SRC)/mysql/MysqlPreparedStatement.cc
	g++ $(CXXFLAGS) -c $^

MysqlResultSet.o: $(SRC)/mysql/MysqlResultSet.cc
	g++ $(CXXFLAGS) -c $^

StringBuffer.o: $(SRC)/Mem/StringBuffer.cc
	g++ $(CXXFLAGS) -c $^

StrOperation.o: $(SRC)/util/StrOperation.cc
	g++ $(CXXFLAGS) -c $^

MemoryOperation.o: $(SRC)/util/MemoryOperation.cc
	g++ $(CXXFLAGS) -c $^

TimeOperation.o: $(SRC)/util/TimeOperation.cc
	g++ $(CXXFLAGS) -c $^

URL.o: $(SRC)/Net/URL.cc
	g++ $(CXXFLAGS) -c $^

selectTest.o: $(SRC)/test/selectTest/selectTest.cc
	g++ $(CXXFLAGS) -c $^

.PHONY: all clean