#include <vector>
#include <odbc/Exception.h>
#include <odbc/ResultSetMetaDataBase.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
ResultSetMetaDataBase::ResultSetMetaDataBase(StatementBase* stmt)
    : stmt_(stmt, true)
{
}
//------------------------------------------------------------------------------
unsigned short ResultSetMetaDataBase::getColumnCount()
{
    SQLSMALLINT ret;
    EXEC_STMT(SQLNumResultCols, stmt_->hstmt_, &ret);
    return (unsigned short)ret;
}
//------------------------------------------------------------------------------
short ResultSetMetaDataBase::getColumnType(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_TYPE, NULL, 0, NULL, &ret);
    return (short)ret;
}
//------------------------------------------------------------------------------
size_t ResultSetMetaDataBase::getColumnLength(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_LENGTH, NULL, 0, NULL, &ret);
    return (size_t)ret;
}
//------------------------------------------------------------------------------
size_t ResultSetMetaDataBase::getColumnOctetLength(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_OCTET_LENGTH, NULL, 0, NULL, &ret);
    return (size_t)ret;
}
//------------------------------------------------------------------------------
size_t ResultSetMetaDataBase::getColumnDisplaySize(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &ret);
    return (size_t)ret;
}
//------------------------------------------------------------------------------
unsigned short ResultSetMetaDataBase::getPrecision(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_PRECISION, NULL, 0, NULL, &ret);
    return (unsigned short)ret;
}
//------------------------------------------------------------------------------
unsigned short ResultSetMetaDataBase::getScale(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_SCALE, NULL, 0, NULL, &ret);
    return (unsigned short)ret;
}
//------------------------------------------------------------------------------
bool ResultSetMetaDataBase::isAutoIncrement(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_AUTO_UNIQUE_VALUE, NULL, 0, NULL, &ret);
    return ret == SQL_TRUE;
}
//------------------------------------------------------------------------------
bool ResultSetMetaDataBase::isCaseSensitive(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_CASE_SENSITIVE, NULL, 0, NULL, &ret);
    return ret == SQL_TRUE;
}
//------------------------------------------------------------------------------
bool ResultSetMetaDataBase::isNamed(unsigned short columnIndex)
{
  SQLLEN ret;
  EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
    SQL_DESC_UNNAMED, NULL, 0, NULL, &ret);
  return ret == SQL_NAMED;
}
//------------------------------------------------------------------------------
bool ResultSetMetaDataBase::isNullable(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_NULLABLE, NULL, 0, NULL, &ret);
    return ret == SQL_NULLABLE;
}
//------------------------------------------------------------------------------
bool ResultSetMetaDataBase::isReadOnly(unsigned short columnIndex)
{
  SQLLEN ret;
  EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
    SQL_DESC_UPDATABLE, NULL, 0, NULL, &ret);
  return ret == SQL_ATTR_READONLY;
}
//------------------------------------------------------------------------------
bool ResultSetMetaDataBase::isSearchable(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_SEARCHABLE, NULL, 0, NULL, &ret);
    return ret == SQL_PRED_SEARCHABLE;
}
//------------------------------------------------------------------------------
bool ResultSetMetaDataBase::isSigned(unsigned short columnIndex)
{
    SQLLEN ret;
    EXEC_STMT(SQLColAttribute, stmt_->hstmt_, columnIndex,
        SQL_DESC_UNSIGNED, NULL, 0, NULL, &ret);
    return ret == SQL_FALSE;
}
//------------------------------------------------------------------------------
NS_ODBC_END
