#ifndef ODBC_DATABASEMETADATAUNICODE_H_INCLUDED
#define ODBC_DATABASEMETADATAUNICODE_H_INCLUDED
//------------------------------------------------------------------------------
#include <string>
#include <odbc/Config.h>
#include <odbc/DatabaseMetaDataBase.h>
#include <odbc/Forwards.h>
#include <odbc/Types.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * Provides information about the database.
 *
 * A DatabaseMetaDataUnicode object can be created using
 * Connection::getDatabaseMetaDataUnicode().
 *
 * Most functions provide only a rudementary description of the data that is
 * returned. Refer to the ODBC documentation and your ODBC driver documentation
 * for further details.
 */
class ODBC_EXPORT DatabaseMetaDataUnicode : public DatabaseMetaDataBase
{
    friend class Connection;

private:
    DatabaseMetaDataUnicode(Connection* parent);

public:
    /**
     * Retrieves a list of columns in specified tables.
     *
     * The list of columns is returned as a ResultSet object, in which each
     * returned row has the following columns:
     *     1. Catalog name
     *     2. Schema name
     *     3. Table name
     *     4. Column name
     *     5. Data type
     *     6. Type name
     *     7. Column size
     *     8. Buffer length
     *     9. Decimal digits
     *     10. Numeric radix
     *     11. Nullability
     *     12. Remarks
     *     13. Default value (ODBC 3.0)
     *     14. SQL data type (ODBC 3.0)
     *     15. Subtype code for date, time and interval data types (ODBC 3.0)
     *     16. Maximum length of bytes in a character or binary column type
     *         (ODBC 3.0)
     *     17. The ordinal position of the column in the table (1-based)
     *         (ODBC 3.0)
     *     18. Nullability as "NO" or "YES" string (ODBC 3.0)
     *
     * This function uses the ODBC function SQLColumns. Refer to its
     * documentation for further details on the data in the ResultSet object.
     *
     * @param catalogName  A string indicating the catalog name.
     * @param schemaName   A string search pattern for schema names.
     * @param tableName    A string search pattern for table names.
     * @param columnName   A string search pattern for column names.
     * @return             Returns a ResultSet object containing the requested
     *                     table description.
     */
    ResultSetRef getColumns(
        const char16_t* catalogName,
        const char16_t* schemaName,
        const char16_t* tableName,
        const char16_t* columnName);

    /**
     * Retrieves a list of columns and associated privileges for the specified
     * table.
     *
     * The list of columns is returned as a ResultSet object, in which each
     * returned row has the following columns:
     *     1. Catalog name
     *     2. Schema name
     *     3. Table name
     *     4. Column name
     *     5. Grantor
     *     6. Grantee
     *     7. Privilege
     *     8. Grantable
     *
     * This function uses the ODBC function SQLColumnPrivileges. Refer to its
     * documentation for further details on the data in the ResultSet object.
     *
     * @param catalogName  A string indicating the catalog name.
     * @param schemaName   A string indicating the schema name.
     * @param tableName    A string indicating the table name.
     * @param columnName   A string search pattern for column names.
     * @return             Returns a ResultSet object containing the requested
     *                     information about privileges.
     */
    ResultSetRef getColumnPrivileges(
        const char16_t* catalogName,
        const char16_t* schemaName,
        const char16_t* tableName,
        const char16_t* columnName);

    /**
     * Retrieves a list of primary keys in the specified table.
     *
     * The list of primary keys is returned as a ResultSet object, in which
     * each returned row has the following columns:
     *     1. Table catalog name
     *     2. Table schema name
     *     3. Table name
     *     4. Primary key column name
     *     5. Primary column sequence number in key (1-based)
     *     6. Primary key name
     *
     * This functions uses the ODBC function SQLPrimaryKeys. Refer to its
     * documentation for further details on the data in the ResultSet object.
     *
     * @param catalogName  A string indicating the catalog name.
     * @param schemaName   A string indicating the schema name.
     * @param tableName    A string indicating the table name.
     * @return             Returns a ResultSet object containing the requested
     *                     table description.
     */
    ResultSetRef getPrimaryKeys(
        const char16_t* catalogName,
        const char16_t* schemaName,
        const char16_t* tableName);

    /**
     * Retrieves information about the unique row identifier of a table.
     *
     * The list of columns is returned as a ResultSet object, in which each
     * returned row has the following columns:
     *     1. Actual scope of the rowid
     *     2. Column name
     *     3. SQL data type
     *     4. Data source-dependent data type name
     *     5. The size of the column on the data source
     *     6. The length in bytes of data transferred
     *     7. The decimal digits of the column on the data source
     *     8. Indicates whether the column is a pseudo-column
     *
     * This function uses the ODBC function SQLSpecialColumns. Refer to its
     * documentation for further details on the data in the ResultSet object.
     *
     * @param identifierType  Type of unique row identifier to return.
     * @param catalogName     A string indicating the catalog name.
     * @param schemaName      A string indicating the schema name.
     * @param tableName       A string indicating the table name.
     * @param scope           Minimum required scope of the rowid.
     * @param nullable        Determines whether to return special columns that
     *                        can have a NULL value.
     * @return                Returns a ResultSet object containing the
     *                        unique row identifier information.
     */
    ResultSetRef getSpecialColumns(
        RowIdentifierType identifierType,
        const char16_t* catalogName,
        const char16_t* schemaName,
        const char16_t* tableName,
        RowIdentifierScope scope,
        ColumnNullableValue nullable);

    /**
      * Retrieves the statistics about the specified table and its indexes.
      *
      * The list of columns is returned as a ResultSet object, in which each
      * returned row has the following columns:
      *     1. Catalog name of the table
      *     2. Schema name of the table
      *     3. Table name of the table
      *     4. Indicates whether the index does not allow duplicate values
      *     5. The identifier that is used to qualify the index name doing a
      *        DROP INDEX
      *     6. Index name
      *     7. Type of information being returned
      *     8. Column sequence number in index (starting with 1)
      *     9. Column name
      *     10. Sort sequence for the column: "A" for ascending; "D" for
      *         descending
      *     11. Cardinality of table or index
      *     12. Number of pages used to store the index or table
      *     13. If the index is a filtered index
      *
      * This function uses the ODBC function SQLStatistics. Refer to its
      * documentation for further details on the data in the ResultSet object.
      *
      * @param catalogName  A string indicating the catalog name.
      * @param schemaName   A string indicating the schema name.
      * @param tableName    A string indicating the table name.
      * @param indexType    Type of index.
      * @param accuracy     Indicates the type of the returned statistics.
      * @return             Returns a ResultSet object containing the statistics
      *                     about the specified table and its indexes.
      */
    ResultSetRef getStatistics(
        const char16_t* catalogName,
        const char16_t* schemaName,
        const char16_t* tableName,
        IndexType indexType,
        StatisticsAccuracy accuracy);

    /**
     * Retrieves a description of the tables that are available in the connected
     * database.
     *
     * The list of tables is returned as a ResultSet object, in which each
     * returned row has the following columns:
     *     1. Catalog name
     *     2. Schema name
     *     3. Table name
     *     4. Table type
     *     5. Remarks
     *
     * This function uses the ODBC function SQLTables. Refer to its
     * documentation for further details on the data in the ResultSet object.
     *
     * @param catalogName  A string indicating the catalog name.
     * @param schemaName   A string search pattern for schema names.
     * @param tableName    A string search pattern for table names.
     * @param tableType    A list of table types to be searched. The list must
     *                     be empty or must contain a list of a comma-separated
     *                     values. These values are "TABLE", "VIEW",
     *                     "SYSTEM TABLE", "GLOBAL TEMPORARY",
     *                     "LOCAL TEMPORARY", "ALIAS", "SYNONYM", or a data
     *                     source-specific type name.
     * @return             Returns a ResultSet object containing the requested
     *                     table description.
     */
    ResultSetRef getTables(
        const char16_t* catalogName,
        const char16_t* schemaName,
        const char16_t* tableName,
        const char16_t* tableType);

    /**
     * Retrieves information about all data types.
     *
     * The information is returned as a ResultSet object, in which each returned
     * row has the following columns:
     *     1. Type name
     *     2. Data type
     *     3. Maximum column size for the data type.
     *     4. Characters used to prefix a literal of that data type.
     *     5. Characters used to suffix a literal of that data type.
     *     6. A list of keywords, separated by commas, corresponding to each
     *        parameter that the application may specify in parentheses when
     *        using the name that is returned in field 1.
     *     7. Nullability of the type.
     *     8. Case-sensitiveness.
     *     9. Searchability.
     *     10. Unsignedness.
     *     11. Flag indicating if the type has a predefined fixed precision and
     *         scale.
     *     12. Auto-incrementing flag.
     *     13. Localized type name.
     *     14. Minimum scale.
     *     15. Maximum scale.
     *     16. SQL data type code.
     *     17. Date/time subcode.
     *     18. The radix used by a numeric type.
     *     19. Interval leading precision.
     *
     * This function uses the ODBC function SQLGetTypeInfo. Refer to its
     * documentation for further details on the data in the ResultSet object.
     *
     * @return      Returns a ResultSet object containing the requested data
     *              type information.
     */
    ResultSetRef getTypeInfo();

    /**
     * Retrieves information about a specific data type.
     *
     * See the documentation of getTypeInfo() for further details.
     *
     * @param type  The data type to retrieve the type information of.
     * @return      Returns a ResultSet object containing the requested data
     *              type information.
     */
    ResultSetRef getTypeInfo(int type);

    /**
     * Retrieves the name of the data source.
     *
     * @return  Returns the name of the data source.
     */
    std::u16string getDataSourceName();

    /**
     * Retrieves the current database in use.
     *
     * @return  Returns the current database in use.
     */
    std::u16string getDatabaseName();

    /**
     * Retrieves the name of the DBMS system.
     *
     * @return  Returns the name of the DBMS system.
     */
    std::u16string getDBMSName();

    /**
     * Retrieves the version of the DBMS system.
     *
     * @return  Returns the version of the DBMS system.
     */
    std::u16string getDBMSVersion();

    /**
     * Retrieves the name of the ODBC driver.
     *
     * @return  Returns the name of the ODBC driver.
     */
    std::u16string getDriverName();

    /**
     * Retrieves the version of the ODBC driver.
     *
     * @return  Returns the version of the ODBC driver.
     */
    std::u16string getDriverVersion();

    /**
     * Retrieves the server name.
     *
     * @return  Returns the server name.
     */
    std::u16string getServerName();

    /**
     * Retrieves the name used in the database.
     *
     * @return  Returns the name used in the database.
     */
    std::u16string getUserName();

private:
    std::u16string getStringTypeInfoW(unsigned short typeInfo);
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
