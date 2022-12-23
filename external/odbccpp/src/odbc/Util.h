#ifndef ODBC_UTIL_H_INCLUDED
#define ODBC_UTIL_H_INCLUDED
//------------------------------------------------------------------------------
#include <string>
#include <odbc/Config.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * Hosts various utility functions.
 */
class ODBC_EXPORT Util
{
public:
    /**
     * Quotes an identifier.
     *
     * The function adds a "-character at the start and end of the string. If
     * the string contains a "-character, it is escaped by duplicating it, e.g.
     * a"b becomes "a""b".
     *
     * @param s  The identifier to quote.
     * @return   Returns the quoted identifier.
     */
    static std::string quote(const std::string& s);

    /**
     * Quotes an identifier.
     *
     * The function adds a "-character at the start and end of the string. If
     * the string contains a "-character, it is escaped by duplicating it, e.g.
     * a"b becomes "a""b".
     *
     * @param s  The identifier to quote.
     * @return   Returns the quoted identifier.
     */
    static std::string quote(const char* s);

    /**
     * Quotes an schema/table name-pair.
     *
     * This function quotes the schema and table and separates then by a dot.
     *
     * @param schema  The schema.
     * @param table   The table.
     * @return        Returns the quoted schema/table name-pair.
     */
    static std::string quote(
        const std::string& schema,
        const std::string& table);

    /**
     * Quotes an schema/table name-pair.
     *
     * This function quotes the schema and table and separates then by a dot.
     *
     * @param schema  The schema.
     * @param table   The table.
     * @return        Returns the quoted schema/table name-pair.
     */
    static std::string quote(
        const char* schema,
        const char* table);
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
