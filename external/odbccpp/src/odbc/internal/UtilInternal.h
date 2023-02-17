#ifndef ODBC_INTERNAL_UTILINTERNAL_H_INCLUDED
#define ODBC_INTERNAL_UTILINTERNAL_H_INCLUDED
//------------------------------------------------------------------------------
#include <odbc/Config.h>
#include <odbc/Types.h>
#include <cinttypes>
//------------------------------------------------------------------------------
typedef struct tagSQL_NUMERIC_STRUCT SQL_NUMERIC_STRUCT;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
class UtilInternal
{
public:
    static void numericToString(const SQL_NUMERIC_STRUCT& num, char* str);
    static void decimalToNumeric(const decimal& dec, SQL_NUMERIC_STRUCT& num);
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
