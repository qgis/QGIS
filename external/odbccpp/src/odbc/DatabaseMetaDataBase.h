#ifndef ODBC_DATABASEMETADATABASE_H_INCLUDED
#define ODBC_DATABASEMETADATABASE_H_INCLUDED
//------------------------------------------------------------------------------
#include <string>
#include <odbc/Config.h>
#include <odbc/Forwards.h>
#include <odbc/Types.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
 /**
  * Base class for DatabaseMetaData and DatabaseMetaDataUnicode.
  */
class ODBC_EXPORT DatabaseMetaDataBase : public RefCounted
{
protected:
    DatabaseMetaDataBase(Connection* parent);

public:
    /**
     * Retrieves the maximum number of active connections the driver can
     * support.
     *
     * @return  Returns the maximum number of active connections the driver can
     *          support. If there is no limit or if the limit is not known,
     *          the function returns 0.
     */
    unsigned short getMaxConnections();

    /**
     * Retrieves the maximum length of a statement string.
     *
     * @return  Returns the maximum length of a statement string. If there is no
     *          limit or if the limit is not known, the function returns 0.
     */
    unsigned long getMaxStatementLength();

    /**
     * Checks whether the database is in read-only mode.
     *
     * @return  Returns true if the database is in read-only mode, false
     *          otherwise.
     */
    bool isReadOnly();

    /**
     * Checks whether the database supports adding columns to existing tables.
     *
     * @returns  Returns true if the database supports adding columns to
     *           existing tables, false otherwise.
     */
    bool supportsAlterTableWithAddColumn();

    /**
     * Checks whether the database supports dropping columns from existing
     * tables.
     *
     * @returns  Returns true if the database supports dropping columns to
     *           existing tables, false otherwise.
     */
    bool supportsAlterTableWithDropColumn();

    /**
     * Retrieves the default transaction isolation level of the database.
     *
     * @return   Returns the default transaction isolation level.
     */
    TransactionIsolationLevel getDefaultTransactionIsolation();

    /**
     * Checks whether the database supports the given transaction isolation
     * level or not.
     *
     * @return   Returns true if the given transaction isolation level is
     *           supported, otherwise false.
     */
    bool supportsTransactionIsolation(TransactionIsolationLevel level);

protected:
    StatementRef createStatement();
    std::string getStringTypeInfoA(unsigned short typeInfo);

private:
    unsigned long getUIntTypeInfo(unsigned short typeInfo);
    unsigned short getUSmallIntTypeInfo(unsigned short typeInfo);

protected:
    ConnectionRef parent_;
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
