#ifndef ODBC_STATEMENT_H_INCLUDED
#define ODBC_STATEMENT_H_INCLUDED
//------------------------------------------------------------------------------
#include <odbc/Config.h>
#include <odbc/Forwards.h>
#include <odbc/StatementBase.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * SQL Statement wrapper.
 *
 * Represents a SQL statement that can be executed.
 */
class ODBC_EXPORT Statement : public StatementBase
{
    friend class Connection;
    friend class DatabaseMetaData;

private:
    Statement(Connection* parent);

private:
    void setHandle(void* hstmt);

public:
    /**
     * Executes the given SQL statement.
     *
     * @param sql  The SQL statement.
     */
    void execute(const char* sql);

    /**
     * Executes the given SQL statement.
     *
     * @param sql  The SQL statement.
     */
    void execute(const char16_t* sql);

    /**
     * Executes the given SQL statement and returns a ResultSet object.
     *
     * @param sql  The SQL statement.
     * @return     Returns a ResultSet object that contains the data produced
     *             by the given SQL statement.
     */
    ResultSetRef executeQuery(const char* sql);

    /**
     * Executes the given SQL statement and returns a ResultSet object.
     *
     * @param sql  The SQL statement.
     * @return     Returns a ResultSet object that contains the data produced
     *             by the given SQL statement.
     */
    ResultSetRef executeQuery(const char16_t* sql);
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
