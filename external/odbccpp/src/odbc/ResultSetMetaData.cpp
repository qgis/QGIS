#include <vector>
#include <odbc/Exception.h>
#include <odbc/ResultSetMetaData.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
namespace odbc {
//------------------------------------------------------------------------------
ResultSetMetaData::ResultSetMetaData(StatementBase* stmt)
    : ResultSetMetaDataBase(stmt)
{
}
//------------------------------------------------------------------------------
string ResultSetMetaData::getCatalogName(unsigned short columnIndex)
{
  return getStringColAttribute(columnIndex, SQL_DESC_CATALOG_NAME);
}
//------------------------------------------------------------------------------
string ResultSetMetaData::getSchemaName(unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_SCHEMA_NAME);
}
//------------------------------------------------------------------------------
string ResultSetMetaData::getTableName(unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_TABLE_NAME);
}
//------------------------------------------------------------------------------
string ResultSetMetaData::getColumnLabel(unsigned short columnIndex)
{
  return getStringColAttribute(columnIndex, SQL_DESC_LABEL);
}
//------------------------------------------------------------------------------
string ResultSetMetaData::getColumnName(unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_NAME);
}
//------------------------------------------------------------------------------
string ResultSetMetaData::getColumnTypeName(unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_TYPE_NAME);
}
//------------------------------------------------------------------------------
string ResultSetMetaData::getStringColAttribute(unsigned short columnIndex,
    unsigned short field)
{
    vector<char> buffer;
    buffer.resize(256);
    while (true)
    {
        SQLPOINTER ptr = &buffer[0];
        SQLSMALLINT bufLen = (SQLSMALLINT)buffer.size();
        SQLSMALLINT dataLen;
        EXEC_STMT(SQLColAttributeA, stmt_->hstmt_, columnIndex, field, ptr,
            bufLen, &dataLen, NULL);
        if (dataLen < bufLen)
            break;
        buffer.resize(dataLen + 1);
    }
    return string(&buffer[0]);
}
//------------------------------------------------------------------------------
} // namespace odbc
