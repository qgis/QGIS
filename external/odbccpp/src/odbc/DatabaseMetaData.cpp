#include <cstring>
#include <odbc/Connection.h>
#include <odbc/DatabaseMetaData.h>
#include <odbc/Exception.h>
#include <odbc/ResultSet.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
namespace odbc {
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
} // namespace odbc
