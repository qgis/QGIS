#include <cstring>
#include <odbc/Connection.h>
#include <odbc/DatabaseMetaData.h>
#include <odbc/Exception.h>
#include <odbc/ResultSet.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
DatabaseMetaData::DatabaseMetaData(Connection* parent)
    : DatabaseMetaDataBase(parent)
{
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaData::getColumns(const char* catalogName,
    const char* schemaName, const char* tableName, const char* columnName)
{
    size_t catalogLen = catalogName ? strlen(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen(schemaName) : 0;
    size_t tableLen = tableName ? strlen(tableName) : 0;
    size_t columnLen = columnName ? strlen(columnName) : 0;

    size_t maxLen = (1 << 8*sizeof(SQLSMALLINT)) - 1;
    if (catalogLen > maxLen)
        throw Exception("The catalog name is too long");
    if (schemaLen > maxLen)
        throw Exception("The schema name is too long");
    if (tableLen > maxLen)
        throw Exception("The table name is too long");
    if (columnLen > maxLen)
        throw Exception("The column name is too long");

    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLColumnsA, stmt->hstmt_,
        (SQLCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLCHAR*)tableName, (SQLSMALLINT)tableLen,
        (SQLCHAR*)columnName, (SQLSMALLINT)columnLen);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaData::getColumnPrivileges(const char* catalogName,
    const char* schemaName, const char* tableName, const char* columnName)
{
    size_t catalogLen = catalogName ? strlen(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen(schemaName) : 0;
    size_t tableLen = tableName ? strlen(tableName) : 0;
    size_t columnLen = columnName ? strlen(columnName) : 0;

    size_t maxLen = (1 << 8 * sizeof(SQLSMALLINT)) - 1;
    if (catalogLen > maxLen)
        throw Exception("The catalog name is too long");
    if (schemaLen > maxLen)
        throw Exception("The schema name is too long");
    if (tableLen > maxLen)
        throw Exception("The table name is too long");
    if (columnLen > maxLen)
        throw Exception("The column name is too long");

    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLColumnPrivilegesA, stmt->hstmt_,
        (SQLCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLCHAR*)tableName, (SQLSMALLINT)tableLen,
        (SQLCHAR*)columnName, (SQLSMALLINT)columnLen);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaData::getPrimaryKeys(const char* catalogName,
    const char* schemaName, const char* tableName)
{
    size_t catalogLen = catalogName ? strlen(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen(schemaName) : 0;
    size_t tableLen = tableName ? strlen(tableName) : 0;

    size_t maxLen = (1 << 8*sizeof(SQLSMALLINT)) - 1;
    if (catalogLen > maxLen)
        throw Exception("The catalog name is too long");
    if (schemaLen > maxLen)
        throw Exception("The schema name is too long");
    if (tableLen > maxLen)
        throw Exception("The table name is too long");

    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLPrimaryKeysA, stmt->hstmt_,
        (SQLCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLCHAR*)tableName, (SQLSMALLINT)tableLen);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaData::getSpecialColumns(
    RowIdentifierType identifierType,
    const char* catalogName, const char* schemaName, const char* tableName,
    RowIdentifierScope scope, ColumnNullableValue nullable)
{
    SQLUSMALLINT fColType;
    switch (identifierType)
    {
    case RowIdentifierType::BEST_ROWID:
        fColType = SQL_BEST_ROWID;
        break;
    case RowIdentifierType::ROWVER:
        fColType = SQL_ROWVER;
        break;
    default:
        throw Exception("Unknown rowid type");
    }

    size_t catalogLen = catalogName ? strlen(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen(schemaName) : 0;
    size_t tableLen = tableName ? strlen(tableName) : 0;

    size_t maxLen = (1 << 8 * sizeof(SQLSMALLINT)) - 1;
    if (catalogLen > maxLen)
        throw Exception("The catalog name is too long");
    if (schemaLen > maxLen)
        throw Exception("The schema name is too long");
    if (tableLen > maxLen)
        throw Exception("The table name is too long");

    SQLUSMALLINT fScope;
    switch (scope)
    {
    case RowIdentifierScope::CURRENT_ROW:
        fScope = SQL_SCOPE_CURROW;
        break;
    case RowIdentifierScope::TRANSACTION:
        fScope = SQL_SCOPE_TRANSACTION;
        break;
    case RowIdentifierScope::SESSION:
        fScope = SQL_SCOPE_SESSION;
        break;
    default:
        throw Exception("Unknown rowid scope");
    }

    SQLUSMALLINT fNullable;
    switch (nullable)
    {
    case ColumnNullableValue::NO_NULLS:
        fNullable = SQL_NO_NULLS;
        break;
    case ColumnNullableValue::NULLABLE:
        fNullable = SQL_NULLABLE;
        break;
    default:
        throw Exception("Unknown nullable value");
    }

    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLSpecialColumnsA, stmt->hstmt_, fColType,
        (SQLCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLCHAR*)tableName, (SQLSMALLINT)tableLen,
        fScope, fNullable);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaData::getStatistics(
    const char* catalogName, const char* schemaName, const char* tableName,
    IndexType indexType, StatisticsAccuracy accuracy)
{
    size_t catalogLen = catalogName ? strlen(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen(schemaName) : 0;
    size_t tableLen = tableName ? strlen(tableName) : 0;

    size_t maxLen = (1 << 8 * sizeof(SQLSMALLINT)) - 1;
    if (catalogLen > maxLen)
        throw Exception("The catalog name is too long");
    if (schemaLen > maxLen)
        throw Exception("The schema name is too long");
    if (tableLen > maxLen)
        throw Exception("The table name is too long");

    SQLUSMALLINT fUnique;
    switch (indexType)
    {
    case IndexType::ALL:
        fUnique = SQL_INDEX_ALL;
        break;
    case IndexType::UNIQUE:
        fUnique = SQL_INDEX_UNIQUE;
        break;
    default:
        throw Exception("Unknown index type");
    }

    SQLUSMALLINT fAccuracy;
    switch (accuracy)
    {
    case StatisticsAccuracy::ENSURE:
        fAccuracy = SQL_ENSURE;
        break;
    case StatisticsAccuracy::QUICK:
        fAccuracy = SQL_QUICK;
        break;
    default:
        throw Exception("Unknown statistics accuracy");
    }

    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLStatisticsA, stmt->hstmt_,
        (SQLCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLCHAR*)tableName, (SQLSMALLINT)tableLen,
        fUnique, fAccuracy);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaData::getTables(const char* catalogName,
    const char* schemaName, const char* tableName, const char* tableType)
{
    size_t catalogLen = catalogName ? strlen(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen(schemaName) : 0;
    size_t tableLen = tableName ? strlen(tableName) : 0;
    size_t tableTypeLen = tableType ? strlen(tableType) : 0;

    size_t maxLen = (1 << 8 * sizeof(SQLSMALLINT)) - 1;
    if (catalogLen > maxLen)
        throw Exception("The catalog name is too long");
    if (schemaLen > maxLen)
        throw Exception("The schema name is too long");
    if (tableLen > maxLen)
        throw Exception("The table name is too long");
    if (tableTypeLen > maxLen)
        throw Exception("The table type is too long");

    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLTablesA, stmt->hstmt_,
        (SQLCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLCHAR*)tableName, (SQLSMALLINT)tableLen,
        (SQLCHAR*)tableType, (SQLSMALLINT)tableTypeLen);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaData::getTypeInfo()
{
    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLGetTypeInfoA, stmt->hstmt_, SQL_ALL_TYPES);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaData::getTypeInfo(int type)
{
    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLGetTypeInfoA, stmt->hstmt_, type);
    return ret;
}
//------------------------------------------------------------------------------
string DatabaseMetaData::getDataSourceName()
{
    return getStringTypeInfoA(SQL_DATA_SOURCE_NAME);
}
//------------------------------------------------------------------------------
string DatabaseMetaData::getDatabaseName()
{
    return getStringTypeInfoA(SQL_DATABASE_NAME);
}
//------------------------------------------------------------------------------
string DatabaseMetaData::getDBMSName()
{
    return getStringTypeInfoA(SQL_DBMS_NAME);
}
//------------------------------------------------------------------------------
string DatabaseMetaData::getDBMSVersion()
{
    return getStringTypeInfoA(SQL_DBMS_VER);
}
//------------------------------------------------------------------------------
string DatabaseMetaData::getDriverName()
{
    return getStringTypeInfoA(SQL_DRIVER_NAME);
}
//------------------------------------------------------------------------------
string DatabaseMetaData::getDriverVersion()
{
    return getStringTypeInfoA(SQL_DRIVER_VER);
}
//------------------------------------------------------------------------------
string DatabaseMetaData::getServerName()
{
    return getStringTypeInfoA(SQL_SERVER_NAME);
}
//------------------------------------------------------------------------------
string DatabaseMetaData::getUserName()
{
    return getStringTypeInfoA(SQL_USER_NAME);
}
//------------------------------------------------------------------------------
NS_ODBC_END
