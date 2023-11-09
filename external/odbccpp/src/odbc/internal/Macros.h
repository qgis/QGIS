#ifndef ODBC_INTERNAL_MACROS_H_INCLUDED
#define ODBC_INTERNAL_MACROS_H_INCLUDED
//------------------------------------------------------------------------------
#include <sstream>
//------------------------------------------------------------------------------
#define EXEC_DBC(function, handle, ...)                                        \
    do {                                                                       \
        SQLRETURN rc = function(handle, ##__VA_ARGS__);                        \
        ::NS_ODBC::Exception::checkForError(rc, SQL_HANDLE_DBC, handle);       \
    } while (false)
//------------------------------------------------------------------------------
#define EXEC_ENV(function, handle, ...)                                        \
    do {                                                                       \
        SQLRETURN rc = function(handle, ##__VA_ARGS__);                        \
        ::NS_ODBC::Exception::checkForError(rc, SQL_HANDLE_ENV, handle);       \
    } while (false)

//------------------------------------------------------------------------------
#define EXEC_STMT(function, handle, ...)                                       \
    do {                                                                       \
        SQLRETURN rc = function(handle, ##__VA_ARGS__);                        \
        ::NS_ODBC::Exception::checkForError(rc, SQL_HANDLE_STMT, handle);      \
    } while (false)
//------------------------------------------------------------------------------
#define ODBC_FAIL(msg)                                                         \
    do {                                                                       \
        ::std::ostringstream out;                                              \
        out << msg;                                                            \
        throw ::NS_ODBC::Exception(out.str());                                 \
    } while (false)
//------------------------------------------------------------------------------
#define ODBC_CHECK(condition, msg)                                             \
    do {                                                                       \
        if (!(condition)) {                                                    \
            ::std::ostringstream out;                                          \
            out << msg;                                                        \
            throw ::NS_ODBC::Exception(out.str());                             \
        }                                                                      \
    } while (false)
//------------------------------------------------------------------------------
#define IS_FLAG_SET(value, flag) (((value & flag) == flag))
//------------------------------------------------------------------------------
#endif
