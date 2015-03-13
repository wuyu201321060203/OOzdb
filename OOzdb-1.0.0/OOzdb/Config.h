#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

/**
 * Global defines, macros and types
 *
 * @file
 */

#include <errno.h>
#include <string>

/* --------------------------------------------- SQL standard value macros */


/**
 * Standard millisecond timeout value for a database call.
 */
#define SQL_DEFAULT_TIMEOUT 3000

/**
 * The default maximum number of database connections
 */
#define SQL_DEFAULT_MAX_CONNECTIONS 20

/**
 * The initial number of database connections
 */
#define SQL_DEFAULT_INIT_CONNECTIONS 5


/**
 * The standard sweep interval in seconds for a ConnectionPool reaper thread
 */
#define SQL_DEFAULT_SWEEP_INTERVAL 60


/**
 * Default Connection timeout in seconds, used by reaper to remove
 * inactive connections
 */
#define SQL_DEFAULT_CONNECTION_TIMEOUT 30


/**
 * Default TCP/IP Connection timeout in seconds, used when connecting to
 * a database server over a TCP/IP connection
 */
#define SQL_DEFAULT_TCP_TIMEOUT 3


/**
 * MySQL default server port number
 */
#define MYSQL_DEFAULT_PORT 3306


/**
 * PostgreSQL default server port number
 */
#define POSTGRESQL_DEFAULT_PORT 5432


/**
 * Oracle default server port number
 */
#define ORACLE_DEFAULT_PORT 1521


/* ------------------------------------------ General Purpose value macros */


/**
 * Standard String length
 */
#define STRLEN 256


/**
 * Boolean truth value
 */
#define true 1


/**
 * Boolean false value
 */
#define false 0


/**
 * Microseconds per second
*/
#define USEC_PER_SEC 1000000


/**
 * Microseconds per millisecond
 */
#define USEC_PER_MSEC 1000


/* ------------------------------------- General Purpose functional macros */


#define IS      strIsEqual


/* ---------------------------------------------------------- Build macros */


/* Mask out GCC __attribute__ extension for non-gcc compilers. */
#ifndef __GNUC__
#define __attribute__(x)
#endif


/* ------------------------------------------------------ Type definitions */


/**
 * The internal 8-bit char type
 */
typedef unsigned char uchar_t;


/**
 * The internal 32 bits integer type
 */
typedef  unsigned int uint32_t;


/* -------------------------------------------------------------- Globals  */



/*
 * The successful mysql operation return code
 */
#define MYSQL_OK 0

/*
 * Default Exception message length
 */
#define EXCEPTION_MESSAGE_LENGTH 512

/*
 * If this string is returned , it means that there must be something wrong
 * happend
 */
#define BADSTR "null"

/*
 * The general successful operation return code
 */
#define SUCCESSFUL 0

/*
 * Invaild row id
 */
#define INVALID_ROWID -1

/*
 * Invalid num of changed rows
 */
#define INVALID_ROWSCHANGED -1

/*
 * The macro of std::string and its const type
 */
typedef std::string STDSTR;
#define CONST_STDSTR STDSTR const

/*
 * The macro of cast operation of C++
 */
#define SC static_cast
#define RC reinterpret_cast

/*
 * The function macro which is used to return information about error occured
 * after last operation
 */

#define System_getLastError (strerror(errno))

#define UINT unsigned int
#define ULONG unsigned long

/*
 * Default sweep interval(seconds)
 */
#define DEFAULT_SWEEP_INTERVAL 60

/*
 * The macro of optimization about branch case
 */
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/*
 * Default max rows of a query's result , 0 means there is no limit
 */
#define DEFAULT_MAX_ROWS 0

#endif
