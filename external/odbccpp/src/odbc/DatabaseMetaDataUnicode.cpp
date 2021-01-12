#include <string>
#include <odbc/Connection.h>
#include <odbc/DatabaseMetaDataUnicode.h>
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
namespace {
//------------------------------------------------------------------------------
size_t strlen16(const char16_t* str)
{
    return char_traits<char16_t>::length(str);
}
//------------------------------------------------------------------------------
}
//------------------------------------------------------------------------------
DatabaseMetaDataUnicode::DatabaseMetaDataUnicode(Connection* parent)
    : DatabaseMetaDataBase(parent)
{
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaDataUnicode::getColumns(const char16_t* catalogName,
    const char16_t* schemaName, const char16_t* tableName,
    const char16_t* columnName)
{
    size_t catalogLen = catalogName ? strlen16(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen16(schemaName) : 0;
    size_t tableLen = tableName ? strlen16(tableName) : 0;
    size_t columnLen = columnName ? strlen16(columnName) : 0;

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
    EXEC_STMT(SQLColumnsW, stmt->hstmt_,
        (SQLWCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLWCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLWCHAR*)tableName, (SQLSMALLINT)tableLen,
        (SQLWCHAR*)columnName, (SQLSMALLINT)columnLen);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaDataUnicode::getColumnPrivileges(
    const char16_t* catalogName, const char16_t* schemaName,
    const char16_t* tableName, const char16_t* columnName)
{
    size_t catalogLen = catalogName ? strlen16(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen16(schemaName) : 0;
    size_t tableLen = tableName ? strlen16(tableName) : 0;
    size_t columnLen = columnName ? strlen16(columnName) : 0;

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
    EXEC_STMT(SQLColumnPrivilegesW, stmt->hstmt_,
        (SQLWCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLWCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLWCHAR*)tableName, (SQLSMALLINT)tableLen,
        (SQLWCHAR*)columnName, (SQLSMALLINT)columnLen);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaDataUnicode::getPrimaryKeys(
    const char16_t* catalogName, const char16_t* schemaName,
    const char16_t* tableName)
{
    size_t catalogLen = catalogName ? strlen16(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen16(schemaName) : 0;
    size_t tableLen = tableName ? strlen16(tableName) : 0;

    size_t maxLen = (1 << 8*sizeof(SQLSMALLINT)) - 1;
    if (catalogLen > maxLen)
        throw Exception("The catalog name is too long");
    if (schemaLen > maxLen)
        throw Exception("The schema name is too long");
    if (tableLen > maxLen)
        throw Exception("The table name is too long");

    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLPrimaryKeysW, stmt->hstmt_,
        (SQLWCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLWCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLWCHAR*)tableName, (SQLSMALLINT)tableLen);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaDataUnicode::getTables(const char16_t* catalogName,
    const char16_t* schemaName, const char16_t* tableName,
    const char16_t* tableType)
{
    size_t catalogLen = catalogName ? strlen16(catalogName) : 0;
    size_t schemaLen = schemaName ? strlen16(schemaName) : 0;
    size_t tableLen = tableName ? strlen16(tableName) : 0;
    size_t tableTypeLen = tableType ? strlen16(tableType) : 0;

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
    EXEC_STMT(SQLTablesW, stmt->hstmt_,
        (SQLWCHAR*)catalogName, (SQLSMALLINT)catalogLen,
        (SQLWCHAR*)schemaName, (SQLSMALLINT)schemaLen,
        (SQLWCHAR*)tableName, (SQLSMALLINT)tableLen,
        (SQLWCHAR*)tableType, (SQLSMALLINT)tableTypeLen);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaDataUnicode::getTypeInfo()
{
    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLGetTypeInfoW, stmt->hstmt_, SQL_ALL_TYPES);
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef DatabaseMetaDataUnicode::getTypeInfo(int type)
{
    StatementRef stmt = createStatement();
    ResultSetRef ret(new ResultSet(stmt.get()));
    EXEC_STMT(SQLGetTypeInfoW, stmt->hstmt_, type);
    return ret;
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getDataSourceName()
{
    return getStringTypeInfoW(SQL_DATA_SOURCE_NAME);
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getDatabaseName()
{
    return getStringTypeInfoW(SQL_DATABASE_NAME);
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getDBMSName()
{
    return getStringTypeInfoW(SQL_DBMS_NAME);
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getDBMSVersion()
{
    return getStringTypeInfoW(SQL_DBMS_VER);
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getDriverName()
{
    return getStringTypeInfoW(SQL_DRIVER_NAME);
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getDriverVersion()
{
    return getStringTypeInfoW(SQL_DRIVER_VER);
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getServerName()
{
    return getStringTypeInfoW(SQL_SERVER_NAME);
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getUserName()
{
    return getStringTypeInfoW(SQL_USER_NAME);
}
//------------------------------------------------------------------------------
u16string DatabaseMetaDataUnicode::getStringTypeInfoW(unsigned short typeInfo)
{
    vector<char16_t> buffer;
    buffer.resize(256);
    while (true)
    {
        SQLPOINTER ptr = &buffer[0];
        SQLSMALLINT bufLen = (SQLSMALLINT)(buffer.size() * sizeof(char16_t));
        SQLSMALLINT dataLen;
        EXEC_DBC(SQLGetInfoW, parent_->hdbc_, typeInfo, ptr, bufLen, &dataLen);
        if (dataLen < bufLen)
            break;
        buffer.resize(dataLen / sizeof(char16_t) + 1);
    }
    return u16string(&buffer[0]);
}
//------------------------------------------------------------------------------
} // namespace odbc
