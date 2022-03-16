#include <odbc/Exception.h>
#include <odbc/ParameterMetaData.h>
#include <odbc/PreparedStatement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
ParameterMetaData::ParameterMetaData(PreparedStatement* ps)
    : ps_(ps, true)
{
}
//------------------------------------------------------------------------------
unsigned short ParameterMetaData::getParameterCount()
{
    SQLSMALLINT ret;
    EXEC_STMT(SQLNumParams, ps_->hstmt_, &ret);
    return ret;
}
//------------------------------------------------------------------------------
short ParameterMetaData::getParameterType(unsigned short paramIndex)
{
    SQLSMALLINT dataType;
    SQLULEN parameterSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;
    EXEC_STMT(SQLDescribeParam, ps_->hstmt_, paramIndex, &dataType,
        &parameterSize, &decimalDigits, &nullable);
    return dataType;
}
//------------------------------------------------------------------------------
size_t ParameterMetaData::getParameterSize(unsigned short paramIndex)
{
    SQLSMALLINT dataType;
    SQLULEN parameterSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;
    EXEC_STMT(SQLDescribeParam, ps_->hstmt_, paramIndex, &dataType,
        &parameterSize, &decimalDigits, &nullable);
    return (size_t)parameterSize;
}
//------------------------------------------------------------------------------
unsigned short ParameterMetaData::getPrecision(unsigned short paramIndex)
{
    SQLSMALLINT dataType;
    SQLULEN parameterSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;
    EXEC_STMT(SQLDescribeParam, ps_->hstmt_, paramIndex, &dataType,
        &parameterSize, &decimalDigits, &nullable);
    return (unsigned short)parameterSize;
}
//------------------------------------------------------------------------------
unsigned short ParameterMetaData::getScale(unsigned short paramIndex)
{
    SQLSMALLINT dataType;
    SQLULEN parameterSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;
    EXEC_STMT(SQLDescribeParam, ps_->hstmt_, paramIndex, &dataType,
        &parameterSize, &decimalDigits, &nullable);
    return (decimalDigits >= 0) ? decimalDigits : 0;
}
//------------------------------------------------------------------------------
bool ParameterMetaData::isNullable(unsigned short paramIndex)
{
    SQLSMALLINT dataType;
    SQLULEN parameterSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;
    EXEC_STMT(SQLDescribeParam, ps_->hstmt_, paramIndex, &dataType,
        &parameterSize, &decimalDigits, &nullable);
    return nullable == SQL_NULLABLE;
}
//------------------------------------------------------------------------------
NS_ODBC_END
