#include <odbc/Connection.h>
#include <odbc/Exception.h>
#include <odbc/StatementBase.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
StatementBase::StatementBase(Connection* parent)
: parent_(parent, true)
, hstmt_(SQL_NULL_HANDLE)
{
}
//------------------------------------------------------------------------------
StatementBase::~StatementBase()
{
    if (hstmt_)
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
}
//------------------------------------------------------------------------------
unsigned long StatementBase::getMaxRows()
{
    SQLULEN ret;
    EXEC_STMT(SQLGetStmtAttr, hstmt_, SQL_ATTR_MAX_ROWS,
        (SQLPOINTER)&ret, 0, NULL);
    return (unsigned long)ret;
}
//------------------------------------------------------------------------------
void StatementBase::setMaxRows(unsigned long maxRows)
{
    EXEC_STMT(SQLSetStmtAttr, hstmt_, SQL_ATTR_MAX_ROWS,
        (SQLPOINTER)(ptrdiff_t)maxRows, SQL_IS_UINTEGER);
}
//------------------------------------------------------------------------------
unsigned long StatementBase::getQueryTimeout()
{
    SQLULEN ret;
    EXEC_STMT(SQLGetStmtAttr, hstmt_, SQL_ATTR_QUERY_TIMEOUT,
        (SQLPOINTER)&ret, 0, NULL);
    return (unsigned long)ret;
}
//------------------------------------------------------------------------------
void StatementBase::setQueryTimeout(unsigned long seconds)
{
    EXEC_STMT(SQLSetStmtAttr, hstmt_, SQL_ATTR_QUERY_TIMEOUT,
        (SQLPOINTER)(ptrdiff_t)seconds, SQL_IS_UINTEGER);
}
//------------------------------------------------------------------------------
NS_ODBC_END
