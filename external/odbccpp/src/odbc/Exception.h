#ifndef ODBC_EXCEPTION_H_INCLUDED
#define ODBC_EXCEPTION_H_INCLUDED
//------------------------------------------------------------------------------
#include <stdexcept>
#include <string>
#include <odbc/Config.h>
#include <odbc/Forwards.h>
#include <odbc/Types.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * ODBC-related exception.
 */
class ODBC_EXPORT Exception : public std::exception
{
private:
    friend class Batch;
    friend class Connection;
    friend class DatabaseMetaData;
    friend class DatabaseMetaDataBase;
    friend class DatabaseMetaDataUnicode;
    friend class date;
    friend class decimal;
    friend class Environment;
    friend class ParameterMetaData;
    friend class PreparedStatement;
    friend class ResultSet;
    friend class ResultSetMetaData;
    friend class ResultSetMetaDataBase;
    friend class ResultSetMetaDataUnicode;
    friend class Statement;
    friend class StatementBase;
    friend class time;
    friend class timestamp;
    friend class ValueBuffer;
    friend class Util;

    static void checkForError(short rc, short handleType, void* handle);
    static Exception create(short handleType, void* handle);

    Exception(const char* s);
    Exception(const std::string& s);

public:
    /**
     * Describes the reason of the exception.
     *
     * @return  Returns a description of the error that occurred.
     */
    virtual const char* what() const noexcept;

private:
    std::string msg_;
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
