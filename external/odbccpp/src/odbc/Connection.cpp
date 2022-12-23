#include <limits>
#include <odbc/Connection.h>
#include <odbc/DatabaseMetaData.h>
#include <odbc/DatabaseMetaDataUnicode.h>
#include <odbc/Environment.h>
#include <odbc/Exception.h>
#include <odbc/PreparedStatement.h>
#include <odbc/ResultSet.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
Connection::Connection(Environment* parent)
: parent_(parent, true)
, hdbc_(SQL_NULL_HANDLE)
, connected_(false)
{
}
//------------------------------------------------------------------------------
Connection::~Connection()
{
    if (connected_)
        SQLDisconnect(hdbc_);
    if (hdbc_)
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc_);
}
//------------------------------------------------------------------------------
void Connection::setHandle(void* hdbc)
{
    hdbc_ = hdbc;
}
//------------------------------------------------------------------------------
void Connection::connect(const char* dsn, const char* user,
    const char* password)
{
    EXEC_DBC(SQLConnectA, hdbc_,
            (SQLCHAR*)dsn, SQL_NTS,
            (SQLCHAR*)user, SQL_NTS,
            (SQLCHAR*)password, SQL_NTS);
    connected_ = true;
}
//------------------------------------------------------------------------------
void Connection::connect(const char* connString)
{
    SQLCHAR outString[1024];
    SQLSMALLINT outLength;
    EXEC_DBC(SQLDriverConnectA, hdbc_, NULL,
            (SQLCHAR*)connString, SQL_NTS,
            outString, sizeof(outString),
            &outLength, SQL_DRIVER_NOPROMPT);
    connected_ = true;
}
//------------------------------------------------------------------------------
void Connection::disconnect()
{
    SQLRETURN rc = SQLDisconnect(hdbc_);
    connected_ = false;
    Exception::checkForError(rc, SQL_HANDLE_DBC, hdbc_);
}
//------------------------------------------------------------------------------
bool Connection::connected() const
{
    return connected_;
}
//------------------------------------------------------------------------------
bool Connection::isValid()
{
    SQLULEN ret = 0;
    EXEC_DBC(SQLGetConnectAttr, hdbc_, SQL_ATTR_CONNECTION_DEAD, &ret, 0, NULL);
    return ret == SQL_CD_FALSE;
}
//------------------------------------------------------------------------------
unsigned long Connection::getConnectionTimeout()
{
    SQLULEN ret = 0;
    EXEC_DBC(SQLGetConnectAttr, hdbc_, SQL_ATTR_CONNECTION_TIMEOUT, &ret, 0,
        NULL);
    return (unsigned long)ret;
}
//------------------------------------------------------------------------------
void Connection::setConnectionTimeout(unsigned long seconds)
{
    EXEC_DBC(SQLSetConnectAttr, hdbc_, SQL_ATTR_CONNECTION_TIMEOUT,
        (SQLPOINTER)(ptrdiff_t)seconds, SQL_IS_UINTEGER);
}
//------------------------------------------------------------------------------
unsigned long Connection::getLoginTimeout()
{
    SQLULEN ret = 0;
    EXEC_DBC(SQLGetConnectAttr, hdbc_, SQL_ATTR_LOGIN_TIMEOUT, &ret, 0, NULL);
    return (unsigned long)ret;
}
//------------------------------------------------------------------------------
void Connection::setLoginTimeout(unsigned long seconds)
{
    EXEC_DBC(SQLSetConnectAttr, hdbc_, SQL_ATTR_LOGIN_TIMEOUT,
        (SQLPOINTER)(ptrdiff_t)seconds, SQL_IS_UINTEGER);
}
//------------------------------------------------------------------------------
void Connection::setAttribute(int attr, int value)
{
    EXEC_DBC(SQLSetConnectAttr, hdbc_, attr, (SQLPOINTER)(ptrdiff_t)value,
        SQL_IS_INTEGER);
}
//------------------------------------------------------------------------------
void Connection::setAttribute(int attr, unsigned int value)
{
    EXEC_DBC(SQLSetConnectAttr, hdbc_, attr, (SQLPOINTER)(ptrdiff_t)value,
        SQL_IS_UINTEGER);
}
//------------------------------------------------------------------------------
void Connection::setAttribute(int attr, const char* value)
{
    EXEC_DBC(SQLSetConnectAttr, hdbc_, attr, (SQLPOINTER)value, SQL_NTS);
}
//------------------------------------------------------------------------------
void Connection::setAttribute(int attr, const char* value, std::size_t size)
{
    if (size > (size_t)numeric_limits<SQLINTEGER>::max())
        throw Exception("The attribute value is too long");
    EXEC_DBC(SQLSetConnectAttr, hdbc_, attr, (SQLPOINTER)value,
        (SQLINTEGER)size);
}
//------------------------------------------------------------------------------
void Connection::setAttribute(int attr, const void* value, std::size_t size)
{
    if (size > (size_t)numeric_limits<SQLINTEGER>::max())
        throw Exception("The attribute value is too long");
    EXEC_DBC(SQLSetConnectAttr, hdbc_, attr, (SQLPOINTER)value,
        (SQLINTEGER)size);
}
//------------------------------------------------------------------------------
bool Connection::getAutoCommit() const
{
    SQLULEN ret = 0;
    EXEC_DBC(SQLGetConnectAttr, hdbc_, SQL_ATTR_AUTOCOMMIT, &ret, 0, NULL);
    return ret == SQL_AUTOCOMMIT_ON;
}
//------------------------------------------------------------------------------
void Connection::setAutoCommit(bool autoCommit)
{
    EXEC_DBC(SQLSetConnectAttr, hdbc_, SQL_ATTR_AUTOCOMMIT,
        autoCommit ?
          (SQLPOINTER)SQL_AUTOCOMMIT_ON: (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
        SQL_IS_INTEGER);
}
//------------------------------------------------------------------------------
void Connection::commit()
{
    SQLRETURN rc = SQLEndTran(SQL_HANDLE_DBC, hdbc_, SQL_COMMIT);
    Exception::checkForError(rc, SQL_HANDLE_DBC, hdbc_);
}
//------------------------------------------------------------------------------
void Connection::rollback()
{
    SQLRETURN rc = SQLEndTran(SQL_HANDLE_DBC, hdbc_, SQL_ROLLBACK);
    Exception::checkForError(rc, SQL_HANDLE_DBC, hdbc_);
}
//------------------------------------------------------------------------------
bool Connection::isReadOnly()
{
    SQLULEN ret = 0;
    EXEC_DBC(SQLGetConnectAttr, hdbc_, SQL_ATTR_ACCESS_MODE, &ret, 0, NULL);
    return ret == SQL_MODE_READ_ONLY;
}
//------------------------------------------------------------------------------
void Connection::setReadOnly(bool readOnly)
{
    unsigned int mode = readOnly ? SQL_MODE_READ_ONLY : SQL_MODE_READ_WRITE;
    setAttribute(SQL_ATTR_ACCESS_MODE, mode);
}
//------------------------------------------------------------------------------
TransactionIsolationLevel Connection::getTransactionIsolation()
{
    SQLULEN txn = 0;
    EXEC_DBC(SQLGetConnectAttr, hdbc_, SQL_ATTR_TXN_ISOLATION, &txn, 0, NULL);

    switch (txn)
    {
    case SQL_TXN_READ_COMMITTED:
        return TransactionIsolationLevel::READ_COMMITTED;
    case SQL_TXN_READ_UNCOMMITTED:
        return TransactionIsolationLevel::READ_UNCOMMITTED;
    case SQL_TXN_REPEATABLE_READ:
        return TransactionIsolationLevel::REPEATABLE_READ;
    case SQL_TXN_SERIALIZABLE:
        return TransactionIsolationLevel::SERIALIZABLE;
    case 0:
        return TransactionIsolationLevel::NONE;
    default:
        throw Exception("Unknown transaction isolation level.");
    }
}
//------------------------------------------------------------------------------
void Connection::setTransactionIsolation(TransactionIsolationLevel level)
{
    unsigned int txn = 0;
    switch (level)
    {
    case TransactionIsolationLevel::READ_COMMITTED:
        txn = SQL_TXN_READ_COMMITTED;
        break;
    case TransactionIsolationLevel::READ_UNCOMMITTED:
        txn = SQL_TXN_READ_UNCOMMITTED;
        break;
    case TransactionIsolationLevel::REPEATABLE_READ:
        txn = SQL_TXN_REPEATABLE_READ;
        break;
    case TransactionIsolationLevel::SERIALIZABLE:
        txn = SQL_TXN_SERIALIZABLE;
        break;
    case TransactionIsolationLevel::NONE:
        throw Exception("NONE transaction isolation level cannot be set.");
    }

    setAttribute(SQL_ATTR_TXN_ISOLATION, txn);
}
//------------------------------------------------------------------------------
StatementRef Connection::createStatement()
{
    SQLHANDLE hstmt;
    StatementRef ret(new Statement(this));
    SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt);
    Exception::checkForError(rc, SQL_HANDLE_DBC, hdbc_);
    ret->setHandle(hstmt);
    return ret;
}
//------------------------------------------------------------------------------
PreparedStatementRef Connection::prepareStatement(const char* sql)
{
    SQLHANDLE hstmt;
    PreparedStatementRef ret(new PreparedStatement(this));
    SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt);
    Exception::checkForError(rc, SQL_HANDLE_DBC, hdbc_);
    ret->setHandleAndQuery(hstmt, sql);
    return ret;
}
//------------------------------------------------------------------------------
PreparedStatementRef Connection::prepareStatement(const char16_t* sql)
{
    SQLHANDLE hstmt;
    PreparedStatementRef ret(new PreparedStatement(this));
    SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt);
    Exception::checkForError(rc, SQL_HANDLE_DBC, hdbc_);
    ret->setHandleAndQuery(hstmt, sql);
    return ret;
}
//------------------------------------------------------------------------------
DatabaseMetaDataRef Connection::getDatabaseMetaData()
{
    DatabaseMetaDataRef ret(new DatabaseMetaData(this));
    return ret;
}
//------------------------------------------------------------------------------
DatabaseMetaDataUnicodeRef Connection::getDatabaseMetaDataUnicode()
{
    DatabaseMetaDataUnicodeRef ret(new DatabaseMetaDataUnicode(this));
    return ret;
}
//------------------------------------------------------------------------------
NS_ODBC_END
