MUDUO_DIRECTORY = $(HOME)/build/debug-install
MUDUO_INCLUDE = $(MUDUO_DIRECTORY)/include
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/lib
SRC = ./
CXXFLAGS = -g -O0 -Wall -Wextra -Werror \
	       -Wconversion -Wno-unused-parameter \
           -Wold-style-cast -Woverloaded-virtual \
           -Wpointer-arith -Wshadow -Wwrite-strings \
           -march=native -rdynamic -std=c++11\
           -I$(MUDUO_INCLUDE)
LDFLAGS = -L$(MUDUO_LIBRARY) -lmuduo_net -lmuduo_base -lpthread -lrt


BIN_PROGRAM = test

all: $(BIN_PROGRAM)

clean:
	test -z "$(BIN_PROGRAM)" || rm -f $(BIN_PROGRAM)
	rm -f *.o core

$(BIN_PROGRAM): Connection.o PreparedStatement.o ResultSet.o ConnectionPool.o\
	  MysqlConnection.o MysqlPreparedStatement.o MysqlResultSet.o \
	  StringBuffer.o StrOperation.o MemoryOperation.o TimeOperation.o URL.o test.o
	g++ $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

Connection.o: $(SRC)/Connection.cc
	g++ $(CXXFLAGS) -c $^

PreparedStatement.o: $(SRC)/PreparedStatement.cc
	g++ $(CXXFLAGS) -c $^

ResultSet.o: $(SRC)/ResultSet.cc
	g++ $(CXXFLAGS) -c $^

ConnectionPool.o: $(SRC)/ConnectionPool.cc
	g++ $(CXXFLAGS) -c $^

MysqlConnection.o: $(SRC)/MysqlConnection.cc
	g++ $(CXXFLAGS) -c $^

MysqlPreparedStatement.o: $(SRC)/MysqlPreparedStatement.cc
	g++ $(CXXFLAGS) -c $^

MysqlResultSet.o: $(SRC)/MysqlResultSet.cc
	g++ $(CXXFLAGS) -c $^

StringBuffer.o: $(SRC)/StringBuffer.cc
	g++ $(CXXFLAGS) -c $^

StrOperation.o: $(SRC)/StrOperation.cc
	g++ $(CXXFLAGS) -c $^

MemoryOperation.o: $(SRC)/MemoryOperation.cc
	g++ $(CXXFLAGS) -c $^

TimeOperation.o: $(SRC)/TimeOperation.cc
	g++ $(CXXFLAGS) -c $^

URL.o: $(SRC)/URL.cc
	g++ $(CXXFLAGS) -c $^

test.o: $(SRC)/test.cc
	g++ $(CXXFLAGS) -c $^

.PHONY: all clean