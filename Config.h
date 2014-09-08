#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED


/**
 * Global defines, macros and types
 *
 * @file
 */


#include <assert.h>
#include <errno.h>

#include "MemoryOperation.h"
#include "StrOperation.h"
#include <string>
#include <cstring>


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



#define MYSQL_OK 0
#define EXCEPTION_MESSAGE_LENGTH 512
#define BADSTR "null"
#define SUCCESSFUL 0
#define INVALID_ROWID -1
#define INVALID_ROWSCHANGED -1

typedef std::string STDSTR;
#define CONST_STDSTR STDSTR const

const char *System_getLastError(void) {
        return strerror(errno);
}

#endif
