#ifndef ODBC_STATEMENTBASE_H_INCLUDED
#define ODBC_STATEMENTBASE_H_INCLUDED
//------------------------------------------------------------------------------
#include <odbc/Config.h>
#include <odbc/Forwards.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * Base class for Statement and PreparedStatement.
 */
class ODBC_EXPORT StatementBase : public RefCounted
{
    friend class DatabaseMetaData;
    friend class DatabaseMetaDataBase;
    friend class DatabaseMetaDataUnicode;
    friend class ParameterMetaData;
    friend class PreparedStatement;
    friend class ResultSet;
    friend class ResultSetMetaData;
    friend class ResultSetMetaDataBase;
    friend class ResultSetMetaDataUnicode;
    friend class Statement;

private:
    StatementBase(Connection* parent);
    ~StatementBase();

private:
    ConnectionRef parent_;
    void* hstmt_;

public:

    /**
     * Gets the maximum number of rows that any ResultSet object created by the
     * current instance can contain.
     *
     * If the returned value equals 0, the driver returns all rows.
     *
     * @return  Returns the maximum number of rows in a ResultSet object.
     */
    unsigned long getMaxRows();

    /**
     * Sets the maximum number of rows that any ResultSet object created by the
     * current instance can contain.
     *
     * If the value is set to 0, the driver returns all rows.
     *
     * @param maxRows  The the maximum number of rows in a ResultSet object.
     */
    void setMaxRows(unsigned long maxRows);

    /**
     * Retrieves the number of seconds that the Statement object has at its
     * disposal to finish query execution.
     *
     * If the returned value equals 0, there is no limit.
     *
     * @return  Returns the query timeout.
     */
    unsigned long getQueryTimeout();

    /**
     * Sets the numer of seconds that the Statement object has at its
     * disposal to finish query execution.
     *
     * If the value is set to 0, there is no limit.
     *
     * @param seconds  The query timeout in seconds.
     */
    void setQueryTimeout(unsigned long seconds);
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
