#ifndef ODBC_RESULTSET_H_INCLUDED
#define ODBC_RESULTSET_H_INCLUDED
//------------------------------------------------------------------------------
#include <odbc/Config.h>
#include <odbc/Forwards.h>
#include <odbc/Types.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * Represents the result set of a SQL query.
 */
class ODBC_EXPORT ResultSet : public RefCounted
{
    friend class DatabaseMetaData;
    friend class DatabaseMetaDataBase;
    friend class DatabaseMetaDataUnicode;
    friend class PreparedStatement;
    friend class Statement;

public:
    /**
     * Data length constant that signals that the data is NULL.
     */
    constexpr static std::size_t NULL_DATA = (std::size_t)-1;

    /**
     * Data length constant that signals that the database could not determine
     * the length of the data.
     */
    constexpr static std::size_t UNKNOWN_LENGTH = (std::size_t)-2;

private:
    ResultSet(StatementBase* parent);
    ~ResultSet();

public:
    /**
     * Advances the cursor to the next row of the ResultSet object and fetches
     * data for all bound columns.
     *
     * @return  Returns true if the current row is valid, false otherwise.
     */
    bool next();

    /**
     * Closes the cursor associated with the current Statement object and
     * discards all pending results.
     */
    void close();

    /**
     * Retrieves a ResultSetMetaData object that contains information about the
     * number, types and properties of the columns in this ResultSet object.
     *
     * @return  Returns a ResultSetMetaData object.
     */
    ResultSetMetaDataRef getMetaData();

    /**
     * Retrieves a ResultSetMetaDataUnicode object that contains information
     * about the number, types and properties of the columns in this ResultSet
     * object.
     *
     * @return  Returns a ResultSetMetaDataUnicode object.
     */
    ResultSetMetaDataUnicodeRef getMetaDataUnicode();

    /**
     * Retrieves the value of the specified column in the current row as
     * a boolean.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Boolean getBoolean(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a signed byte.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Byte getByte(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * an unsigned byte.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    UByte getUByte(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a signed short integer.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Short getShort(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * an unsigned short integer.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    UShort getUShort(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a signed integer.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Int getInt(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * an unsigned integer.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    UInt getUInt(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a signed long integer.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Long getLong(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * an unsigned long integer.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    ULong getULong(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a decimal object.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Decimal getDecimal(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a floating point type with single precision.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Float getFloat(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a floating point type with double precision.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Double getDouble(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a date object.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Date getDate(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a time object.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Time getTime(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a timestamp object.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Timestamp getTimestamp(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a string object.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    String getString(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * a string with 16-bit characters.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    NString getNString(unsigned short columnIndex);

    /**
     * Retrieves the value of the specified column in the current row as
     * an object with binary data.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the column value.
     */
    Binary getBinary(unsigned short columnIndex);

    /**
     * Retrieves the length of the binary data stored in the specified column of
     * the current row.
     *
     * If the binary data is NULL, the constant NULL_DATA is returned. If the
     * database cannot determine the length, the constant UNKNOWN_LENGTH is
     * returned.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             Returns the length of binary data, NULL_DATA or
     *                     UNKNOWN_LENGTH.
     */
    std::size_t getBinaryLength(unsigned short columnIndex);

    /**
     * Retrieves the binary data stored in the specified column of the current
     * row.
     *
     * The total length of the binary data can be found by calling
     * the getBinaryLength() function.
     *
     * @param columnIndex  The column index starting from 1.
     * @param data         The pointer to a buffer where the read data will be
     *                     stored.
     * @param size  The maximum number of bytes to be read.
     */
    void getBinaryData(unsigned short columnIndex, void* data,
        std::size_t size);

    /**
     * Retrieves the lengh of the string stored in the specified column of
     * the current row.
     *
     * If the string is NULL, the constant NULL_DATA is returned. If the
     * database cannot determine the length, the constant UNKNOWN_LENGTH is
     * returned.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             The length of a string in characters, NULL_DATA or
     *                     UNKNOWN_LENGTH.
     */
    std::size_t getStringLength(unsigned short columnIndex);

    /**
     * Retrieves the string data stored in the specified column of the current
     * row.
     *
     * The total length of the string can be found by calling the
     * getStringLength() function.
     *
     * @param columnIndex  The column index starting from 1.
     * @param[out] data  The pointer to a buffer where the read data will be
     *                   stored.
     * @param size  The maximum number of bytes to be read.
     */
    void getStringData(unsigned short columnIndex, void* data,
        std::size_t size);

    /**
     * Retrieves the length of the string with 16-bit characters stored in the
     * specified column of the current row.
     *
     * If the string is NULL, the constant NULL_DATA is returned. If the
     * database cannot determine the length, the constant UNKNOWN_LENGTH is
     * returned.
     *
     * @param columnIndex  The column index starting from 1.
     * @return             The length of a string in characters, NULL_DATA or
     *                     UNKNOWN_LENGTH.
     */
    std::size_t getNStringLength(unsigned short columnIndex);

    /**
     * Retrieves the 16-bit character string data stored in the specified column
     * of the current row.
     *
     * The total length of the string can be found by calling the
     * getNStringLength() function.
     *
     * @param columnIndex  The column index starting from 1.
     * @param data         The pointer to a buffer where read data will be
                           stored.
     * @param size         The maximum number of 16-bit characters to be read.
     */
    void getNStringData(unsigned short columnIndex, void* data,
        std::size_t size);

private:
    StatementBaseRef parent_;
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
