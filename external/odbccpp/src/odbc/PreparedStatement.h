#ifndef ODBC_PREPAREDSTATEMENT_H_INCLUDED
#define ODBC_PREPAREDSTATEMENT_H_INCLUDED
//------------------------------------------------------------------------------
#include <memory>
#include <vector>
#include <odbc/Config.h>
#include <odbc/Forwards.h>
#include <odbc/StatementBase.h>
#include <odbc/Types.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
class Batch;
class ParameterData;
//------------------------------------------------------------------------------
/**
 * Represents a precompiled SQL statement.
 */
class ODBC_EXPORT PreparedStatement : public StatementBase
{
    friend class Connection;
    friend class ParameterMetaData;

private:
    PreparedStatement(Connection* parent);

public:
    ~PreparedStatement();

private:
    void setHandleAndQuery(void* hstmt, const char* query);

    void setHandleAndQuery(void* hstmt, const char16_t* query);

public:
    /**
     * Retrieves the number, types and properties of the parameters.
     *
     * @return  Returns a ParameterMetaData object that contains information
     *          about the number, types and properties for each parameter.
     */
    ParameterMetaDataRef getParameterMetaData();

    /**
     * Sets the specified parameter to the given boolean value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setBoolean(unsigned short paramIndex, const Boolean& value);

    /**
     * Sets the specified parameter to the given signed byte value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setByte(unsigned short paramIndex, const Byte& value);
    /**
     * Sets the specified parameter to the given unsigned byte value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setUByte(unsigned short paramIndex, const UByte& value);
    /**
     * Sets the specified parameter to the given signed short integer value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setShort(unsigned short paramIndex, const Short& value);
    /**
     * Sets the specified parameter to the given unsigned short integer value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setUShort(unsigned short paramIndex, const UShort& value);
    /**
     * Sets the specified parameter to the given signed integer value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setInt(unsigned short paramIndex, const Int& value);
    /**
     * Sets the specified parameter to the given unsigned integer value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setUInt(unsigned short paramIndex, const UInt& value);
    /**
     * Sets the specified parameter to the given signed long integer value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setLong(unsigned short paramIndex, const Long& value);
    /**
     * Sets the specified parameter to the given unsigned long integer value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setULong(unsigned short paramIndex, const ULong& value);

    /**
     * Sets the specified parameter to the given decimal value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setDecimal(unsigned short paramIndex, const Decimal& value);

    /**
     * Sets the specified parameter to the given float value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setFloat(unsigned short paramIndex, const Float& value);
    /**
     * Sets the specified parameter to the given double value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setDouble(unsigned short paramIndex, const Double& value);

    /**
     * Sets the specified parameter to the given string value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setString(unsigned short paramIndex, const String& value);
    /**
     * Sets the specified parameter to the given null-terminated character
     * sequence (C-String).
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param s           The null-terminated character sequence.
     */
    void setCString(unsigned short paramIndex, const char* s);
    /**
     * Sets the specified parameter to the given character sequence of the
     * desired length.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param s           The sequence of characters.
     * @param len         The length of the sequence.
     */
    void setCString(unsigned short paramIndex, const char* s, std::size_t len);

    /**
     * Sets the specified parameter to the given string with 16-bit characters.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setNString(unsigned short paramIndex, const NString& value);
    /**
     * Sets the specified parameter to the given null-terminated 16-bit
     * character sequence (C-String).
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param s           The null-terminated character sequence.
     */
    void setNCString(unsigned short paramIndex, const char16_t* s);
    /**
     * Sets the specified parameter to the given 16-bit character sequence of
     * the desired length.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param s           The sequence of characters.
     * @param len         The length of the sequence.
     */
    void setNCString(unsigned short paramIndex, const char16_t* s,
        std::size_t len);

    /**
     * Sets the specified parameter to the given Binary object.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setBinary(unsigned short paramIndex, const Binary& value);
    /**
     * Sets the specified parameter to the given byte sequence of the desired
     * length.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param data        The sequence of bytes.
     * @param size        The length of the sequence.
     */
    void setBytes(unsigned short paramIndex, const void* data,
        std::size_t size);

    /**
     * Sets the specified parameter to the given Date value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setDate(unsigned short paramIndex, const Date& value);
    /**
     * Sets the specified parameter to the given Time value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setTime(unsigned short paramIndex, const Time& value);
    /**
     * Sets the specified parameter to the given Timestamp value.
     *
     * @param paramIndex  The parameter index starting from 1.
     * @param value       The parameter value.
     */
    void setTimestamp(unsigned short paramIndex, const Timestamp& value);

    /**
     * Clears the parameter values.
     */
    void clearParameters();

    /**
     * Retrieves a ResultSetMetaData object that contains information about a
     * ResultSet object returned by executeQuery method.
     *
     * @return  Returns a ResultSetMetaData object.
     */
    ResultSetMetaDataRef getMetaData();

    /**
     * Retrieves a ResultSetMetaDataUnicode object that contains information
     * about a ResultSet object returned by executeQuery method.
     *
     * @return  Returns a ResultSetMetaDataUnicode object.
     */
    ResultSetMetaDataUnicodeRef getMetaDataUnicode();

    /**
     * Executes the SQL query of this PreparedStatement object and returns a
     * ResultSet object.
     *
     * @return  Returns a ResultSet object that contains the data produced
     *          by the given SQL statement.
     */
    ResultSetRef executeQuery();

    /**
     * Executes the SQL query of this PreparedStatement object and returns
     * the number of rows affected by an UPDATE, INSERT, or DELETE statement.
     *
     * @return  Returns the number of rows affected by an UPDATE, INSERT, or
     *          DELETE statement.
     */
    std::size_t executeUpdate();

    /**
     * Add the current set of parameters to the batch of commands.
     */
    void addBatch();
    /**
     * Executes the batch of commands.
     */
    void executeBatch();
    /**
     * Clears the batch of commands.
     */
    void clearBatch();

    /**
     * Retrieves the number of bytes required by the batch of commands.
     *
     * @return  Returns the number of bytes required by the batch of commands.
     */
    std::size_t getBatchDataSize() const;

private:
    void bindParameters();

    template<typename T>
    void setFixedSizeData(unsigned short paramIndex, const Nullable<T>& value);

    void verifyValidParamIndex(unsigned short paramIndex) const;
    void verifyAllParametersValid() const;

private:
    std::vector<ParameterData> parameterData_;
    std::unique_ptr<Batch> batch_;
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
