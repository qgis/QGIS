#include <odbc/Connection.h>
#include <odbc/DatabaseMetaDataBase.h>
#include <odbc/Exception.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
DatabaseMetaDataBase::DatabaseMetaDataBase(Connection* parent)
    : parent_(parent, true)
{
}
//------------------------------------------------------------------------------
unsigned short DatabaseMetaDataBase::getMaxConnections()
{
    return getUSmallIntTypeInfo(SQL_MAX_DRIVER_CONNECTIONS);
}
//------------------------------------------------------------------------------
unsigned long DatabaseMetaDataBase::getMaxStatementLength()
{
    return getUIntTypeInfo(SQL_MAX_STATEMENT_LEN);
}
//------------------------------------------------------------------------------
bool DatabaseMetaDataBase::isReadOnly()
{
    return (getStringTypeInfoA(SQL_DATA_SOURCE_READ_ONLY) == "Y");
}
//------------------------------------------------------------------------------
bool DatabaseMetaDataBase::supportsAlterTableWithAddColumn()
{
    SQLUINTEGER val = getUIntTypeInfo(SQL_ALTER_TABLE);
    return IS_FLAG_SET(val, SQL_AT_ADD_COLUMN_COLLATION) ||
           IS_FLAG_SET(val, SQL_AT_ADD_COLUMN_DEFAULT) ||
           IS_FLAG_SET(val, SQL_AT_ADD_COLUMN_SINGLE);
}
//------------------------------------------------------------------------------
bool DatabaseMetaDataBase::supportsAlterTableWithDropColumn()
{
    SQLUINTEGER val = getUIntTypeInfo(SQL_ALTER_TABLE);
    return IS_FLAG_SET(val, SQL_AT_DROP_COLUMN_CASCADE) ||
           IS_FLAG_SET(val, SQL_AT_DROP_COLUMN_DEFAULT) ||
           IS_FLAG_SET(val, SQL_AT_DROP_COLUMN_RESTRICT);
}
//------------------------------------------------------------------------------
TransactionIsolationLevel DatabaseMetaDataBase::getDefaultTransactionIsolation()
{
    SQLUINTEGER txn = getUIntTypeInfo(SQL_DEFAULT_TXN_ISOLATION);
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
bool DatabaseMetaDataBase::supportsTransactionIsolation(
    TransactionIsolationLevel level)
{
    SQLUINTEGER txn = getUIntTypeInfo(SQL_TXN_ISOLATION_OPTION);
    switch (level)
    {
    case TransactionIsolationLevel::READ_COMMITTED:
        return IS_FLAG_SET(txn, SQL_TXN_READ_COMMITTED);
    case TransactionIsolationLevel::READ_UNCOMMITTED:
        return IS_FLAG_SET(txn, SQL_TXN_READ_UNCOMMITTED);
    case TransactionIsolationLevel::REPEATABLE_READ:
        return IS_FLAG_SET(txn, SQL_TXN_REPEATABLE_READ);
    case TransactionIsolationLevel::SERIALIZABLE:
        return IS_FLAG_SET(txn, SQL_TXN_SERIALIZABLE);
    default:
        return false;
    }
}
//------------------------------------------------------------------------------
StatementRef DatabaseMetaDataBase::createStatement()
{
    return parent_->createStatement();
}
//------------------------------------------------------------------------------
string DatabaseMetaDataBase::getStringTypeInfoA(unsigned short typeInfo)
{
    vector<char> buffer;
    buffer.resize(256);
    while (true)
    {
        SQLPOINTER ptr = &buffer[0];
        SQLSMALLINT bufLen = (SQLSMALLINT)buffer.size();
        SQLSMALLINT dataLen;
        EXEC_DBC(SQLGetInfoA, parent_->hdbc_, typeInfo, ptr, bufLen, &dataLen);
        if (dataLen < bufLen)
            break;
        buffer.resize(dataLen + 1);
    }
    return string(&buffer[0]);
}
//------------------------------------------------------------------------------
unsigned long DatabaseMetaDataBase::getUIntTypeInfo(unsigned short typeInfo)
{
    SQLUINTEGER ret;
    SQLSMALLINT len;
    EXEC_DBC(SQLGetInfo, parent_->hdbc_, typeInfo, &ret, sizeof(ret), &len);
    return ret;
}
//------------------------------------------------------------------------------
unsigned short DatabaseMetaDataBase::getUSmallIntTypeInfo(
    unsigned short typeInfo)
{
    SQLUSMALLINT ret;
    SQLSMALLINT len;
    EXEC_DBC(SQLGetInfo, parent_->hdbc_, typeInfo, &ret, sizeof(ret), &len);
    return ret;
}
//------------------------------------------------------------------------------
NS_ODBC_END
