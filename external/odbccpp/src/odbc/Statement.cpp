#include <odbc/Exception.h>
#include <odbc/ResultSet.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
Statement::Statement(Connection* parent) : StatementBase(parent)
{
}
//------------------------------------------------------------------------------
void Statement::setHandle(void* hstmt)
{
    hstmt_ = hstmt;
}
//------------------------------------------------------------------------------
void Statement::execute(const char* sql)
{
    EXEC_STMT(SQLExecDirectA, hstmt_, (SQLCHAR*)sql, SQL_NTS);
}
//------------------------------------------------------------------------------
void Statement::execute(const char16_t* sql)
{
    EXEC_STMT(SQLExecDirectW, hstmt_, (SQLWCHAR*)sql, SQL_NTS);
}
//------------------------------------------------------------------------------
ResultSetRef Statement::executeQuery(const char* sql)
{
    ResultSetRef ret(new ResultSet(this));
    EXEC_STMT(SQLFreeStmt, hstmt_, SQL_CLOSE);
    EXEC_STMT(SQLExecDirectA, hstmt_, (SQLCHAR*)sql, SQL_NTS);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef Statement::executeQuery(const char16_t* sql)
{
    ResultSetRef ret(new ResultSet(this));
    EXEC_STMT(SQLFreeStmt, hstmt_, SQL_CLOSE);
    EXEC_STMT(SQLExecDirectW, hstmt_, (SQLWCHAR*)sql, SQL_NTS);
    return ret;
}
//------------------------------------------------------------------------------
NS_ODBC_END
