#include <vector>
#include <odbc/Exception.h>
#include <odbc/ResultSetMetaDataUnicode.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
ResultSetMetaDataUnicode::ResultSetMetaDataUnicode(StatementBase* stmt)
    : ResultSetMetaDataBase(stmt)
{
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getCatalogName(unsigned short columnIndex)
{
  return getStringColAttribute(columnIndex, SQL_DESC_CATALOG_NAME);
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getSchemaName(unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_SCHEMA_NAME);
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getTableName(unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_TABLE_NAME);
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getBaseTableName(
    unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_BASE_TABLE_NAME);
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getBaseColumnName(
    unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_BASE_COLUMN_NAME);
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getColumnLabel(unsigned short columnIndex)
{
  return getStringColAttribute(columnIndex, SQL_DESC_LABEL);
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getColumnName(unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_NAME);
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getColumnTypeName(
    unsigned short columnIndex)
{
    return getStringColAttribute(columnIndex, SQL_DESC_TYPE_NAME);
}
//------------------------------------------------------------------------------
u16string ResultSetMetaDataUnicode::getStringColAttribute(
    unsigned short columnIndex, unsigned short field)
{
    vector<char16_t> buffer;
    buffer.resize(256);
    while (true)
    {
        SQLPOINTER ptr = &buffer[0];
        SQLSMALLINT bufLen = (SQLSMALLINT)(buffer.size() * sizeof(char16_t));
        SQLSMALLINT dataLen;
        EXEC_STMT(SQLColAttributeW, stmt_->hstmt_, columnIndex, field, ptr,
            bufLen, &dataLen, NULL);
        if (dataLen < bufLen)
            break;
        buffer.resize(dataLen / sizeof(char16_t) + 1);
    }
    return u16string(&buffer[0]);
}
//------------------------------------------------------------------------------
NS_ODBC_END
