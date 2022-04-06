#ifndef ODBC_RESULTSETMETADATA_H_INCLUDED
#define ODBC_RESULTSETMETADATA_H_INCLUDED
//------------------------------------------------------------------------------
#include <odbc/Config.h>
#include <odbc/Forwards.h>
#include <odbc/ResultSetMetaDataBase.h>
#include <odbc/Types.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * Metadata on a result set.
 */
class ODBC_EXPORT ResultSetMetaData : public ResultSetMetaDataBase
{
    friend class PreparedStatement;
    friend class ResultSet;

private:
    ResultSetMetaData(StatementBase* parent);

public:
    /**
     * Returns a column's catalog name.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column's catalog name.
     */
    std::string getCatalogName(unsigned short columnIndex);

    /**
     * Returns a column's schema name.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column's schema name.
     */
    std::string getSchemaName(unsigned short columnIndex);

    /**
     * Returns a column's table name.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column's table name.
     */
    std::string getTableName(unsigned short columnIndex);

    /**
     * Returns the name of the base table that contains the column.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the base table name.
     */
    std::string getBaseTableName(unsigned short columnIndex);

    /**
     * Returns the base column name for the result set column.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the base column name.
     */
    std::string getBaseColumnName(unsigned short columnIndex);

    /**
     * Returns a column's label.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column's label.
     */
    std::string getColumnLabel(unsigned short columnIndex);

    /**
     * Returns a column's name.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column's name.
     */
    std::string getColumnName(unsigned short columnIndex);

    /**
     * Returns a column's type name.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column's type name.
     */
    std::string getColumnTypeName(unsigned short columnIndex);

private:
    std::string getStringColAttribute(unsigned short columnIndex,
        unsigned short field);
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
