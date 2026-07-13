#ifndef ODBC_CONFIG_H_INCLUDED
#define ODBC_CONFIG_H_INCLUDED
//------------------------------------------------------------------------------
#ifndef NS_ODBC
#    define NS_ODBC odbc
#    define NS_ODBC_START namespace odbc {
#    define NS_ODBC_END }
#endif
//------------------------------------------------------------------------------
#ifndef ODBC_EXPORT
#    ifdef ODBC_STATIC
#        define ODBC_EXPORT
#    else
#        if defined(__clang__) || defined(__GNUC__)
#            define ODBC_EXPORT __attribute__ ((visibility("default")))
#        elif defined(_MSC_VER)
#            ifdef ODBC_EXPORTS
#                define ODBC_EXPORT __declspec(dllexport)
#            else
#                define ODBC_EXPORT __declspec(dllimport)
#            endif
#        endif
#    endif
#endif
//------------------------------------------------------------------------------
#if defined(__clang__) || defined(__GNUC__)
#    define ODBC_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#    define ODBC_UNREACHABLE __assume(false)
#else
#    define ODBC_UNREACHABLE ((void)0)
#endif
//------------------------------------------------------------------------------
#ifdef _MSC_VER
#   pragma warning(disable: 4251 4275)
#endif
//------------------------------------------------------------------------------
#endif
