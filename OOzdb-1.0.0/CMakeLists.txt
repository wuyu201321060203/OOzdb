cmake_minimum_required(VERSION 2.6)

PROJECT(OOzdb CXX)

ADD_DEFINITIONS(
    -g
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wold-style-cast
    -Woverloaded-virtual
    -Wpointer-arith
    -Wshadow
    -march=native
    -rdynamic
    -std=c++11)

ADD_DEFINITIONS(-DOOZDBTEST)

SET(CMAKE_CXX_COMPILER "g++")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

FIND_PATH(MUDUO_DEBUG_PATH libmuduo_net_cpp11.a $ENV{HOME}/build/debug-install/lib)
FIND_PATH(MUDUO_RELEASE_PATH libmuduo_net_cpp11.a $ENV{HOME}/build/release-install/lib)

IF(NOT MUDUO_DEBUG_PATH AND NOT MUDUO_RELEASE_PATH)
    MESSAGE(STATUS "can't find muduo path")
ELSE()
    MESSAGE(STATUS "muduo path found")
ENDIF()

FIND_LIBRARY(MUDUO_NET_CPP11 libmuduo_net_cpp11.a /usr/lib /usr/local/lib
    ${MUDUO_DEBUG_PATH} ${MUDUO_RELEASE_PATH})
FIND_LIBRARY(MUDUO_BASE_CPP11 libmuduo_base_cpp11.a /usr/lib /usr/local/lib
    ${MUDUO_DEBUG_PATH} ${MUDUO_RELEASE_PATH})
FIND_LIBRARY(MYSQLCLIENT mysqlclient /usr/lib /usr/local/lib)
FIND_LIBRARY(GTEST gtest $ENV{HOME}/gtest-1.7.0/build/)

IF(NOT MUDUO_NET_CPP11)
    MESSAGE(FATAL_ERROR "lib muduo_net_cpp11 not found")
ELSE()
    MESSAGE(STATUS "lib muduo_net_cpp11 found ${MUDUO_NET_CPP11}")
ENDIF()

IF(NOT MUDUO_BASE_CPP11)
    MESSAGE(FATAL_ERROR "lib muduo_base_cpp11 not found")
ELSE()
    MESSAGE(STATUS "lib muduo_base_cpp11 found ${MUDUO_BASE_CPP11}")
ENDIF()

IF(NOT MYSQLCLIENT)
    MESSAGE(FATAL_ERROR "lib mysqlclient not found")
ELSE()
    MESSAGE(STATUS "lib mysqlclient found ${MYSQLCLIENT}")
ENDIF()

IF(NOT GTEST)
    MESSAGE(FATAL_ERROR "lib gtest not found")
ELSE()
    MESSAGE(STATUS "lib gtest found ${GTEST}")
ENDIF()

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

ADD_SUBDIRECTORY(OOzdb)

IF(NOT CMAKE_BUILD_NO_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF()

INSTALL(FILES COPYRIGHT README DESTINATION share/doc/OOzdb)