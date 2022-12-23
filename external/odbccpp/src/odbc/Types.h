#ifndef ODBC_TYPES_H_INCLUDED
#define ODBC_TYPES_H_INCLUDED
//------------------------------------------------------------------------------
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>
#include <odbc/Config.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 *  Specifies the type of an ODBC DSN.
 */
enum class DSNType
{
    /**
     * Indicates that both user and system DSNs will be returned.
     */
    ALL,
    /**
     * Indicates that only system DSNs will be returned.
     */
    SYSTEM,
    /**
     * Indicates that only user DSNs will be returned.
     */
    USER
};
//------------------------------------------------------------------------------
/**
 *  Specifies the type of the index.
 */
enum class IndexType
{
    /**
     * All indexes are returned.
     */
    ALL,
    /**
     * Only unique indexes are returned.
     */
    UNIQUE
};
//------------------------------------------------------------------------------
/**
 *  Specifies the accuracy of the statistics about a single table and
 *  its indexes.
 */
enum class StatisticsAccuracy
{
    /**
     * Requests that the driver unconditionally retrieves the statistics.
     */
    ENSURE,
    /**
     * Requests that the driver retrieves the CARDINALITY and PAGES only if they
     * are readily available from the server.
     */
    QUICK
};
//------------------------------------------------------------------------------
/**
 *  Specifies the constants that identify whether the column allows NULL values.
 */
enum class ColumnNullableValue
{
    /**
     * Column does not allow NULL values.
     */
    NO_NULLS,
    /**
     * Column allows NULL values.
     */
    NULLABLE
};
//------------------------------------------------------------------------------
/**
 *  Specifies the type of unique row identifier.
 */
enum class RowIdentifierType
{
    /**
     * Returns the optimal column or set of columns that, by retrieving values
     * from the column or columns, allows any row in the specified table to be
     * uniquely identified. A column can be either a pseudo - column
     * specifically designed for this purpose (as in Oracle ROWID or Ingres TID)
     * or the column or columns of any unique index for the table.
     */
    BEST_ROWID,
    /**
     * Returns the column or columns in the specified table, if any, that are
     * automatically updated by the data source when any value in the row is
     * updated by any transaction (as in SQLBase ROWID or Sybase TIMESTAMP).
     */
    ROWVER
};
//------------------------------------------------------------------------------
/**
 *  Specifies the required scope of the row identifier (rowid).
 */
enum class RowIdentifierScope
{
    /**
     * The rowid is guaranteed to be valid only while positioned on that row.
     * A later reselect using rowid may not return a row if the row was updated
     * or deleted by another transaction.
     */
     CURRENT_ROW,
     /**
      * The rowid is guaranteed to be valid for the duration of the session
      * (across transaction boundaries).
      */
     SESSION,
     /**
      * The rowid is guaranteed to be valid for the duration of the current
      * transaction.
      */
     TRANSACTION
};
//------------------------------------------------------------------------------
/**
 *  Specifies the constants that identify ODBC SQL data types.
 */
class SQLDataTypes
{
    SQLDataTypes() = delete;

public:
    /// 64-bit integer value.
    static constexpr int BigInt = -5;
    /// Binary data of fixed length.
    static constexpr int Binary = -2;
    /// Single bit binary data.
    static constexpr int Bit = -7;
    /// Boolean value.
    static constexpr int Boolean = 16;
    /// Character string of fixed string length.
    static constexpr int Char = 1;
    /// Year, month, and day fields.
    static constexpr int Date = 9;
    /// Year, month, and day fields.
    static constexpr int DateTime = 9;
    /// Signed, exact, numeric value.
    static constexpr int Decimal = 3;
    /// Double-precision floating point number.
    static constexpr int Double = 8;
    /// Floating point number with driver-specific precision.
    static constexpr int Float = 6;
    /// Fixed length GUID.
    static constexpr int Guid = -11;
    /// 32-bit integer value.
    static constexpr int Integer = 4;
    /// Interval data type.
    static constexpr int Interval = 10;
    /// Variable length binary data.
    static constexpr int LongVarBinary = -4;
    /// Variable length character data.
    static constexpr int LongVarChar = -1;
    /// Signed, exact, numeric value.
    static constexpr int Numeric = 2;
    /// Single-precision floating point number.
    static constexpr int Real = 7;
    /// 16-bit integer value.
    static constexpr int SmallInt = 5;
    /// Hour, minute, and second fields.
    static constexpr int Time = 10;
    /// Year, month, day, hour, minute, and second fields.
    static constexpr int Timestamp = 11;
    /// 8-bit integer value.
    static constexpr int TinyInt = -6;
    /// Year, month, and day fields.
    static constexpr int TypeDate = 91;
    /// Hour, minute, and second fields.
    static constexpr int TypeTime = 92;
    /// Year, month, day, hour, minute, and second fields.
    static constexpr int TypeTimestamp = 93;
    /// Unknown data type.
    static constexpr int Unknown = 0;
    /// Variable length binary data.
    static constexpr int VarBinary = -3;
    /// Variable-length character string.
    static constexpr int VarChar = 12;
    /// Unicode character string of fixed string length.
    static constexpr int WChar = -8;
    /// Unicode variable-length character data.
    static constexpr int WLongVarChar = -10;
    /// Unicode variable-length character string.
    static constexpr int WVarChar = -9;
};
//------------------------------------------------------------------------------
/**
 *  Specifies isolation levels for transactions.
 */
enum class TransactionIsolationLevel
{
    /**
     * Indicates that any phenomena such as dirty reads, non-repeatable reads
     * and phantoms can occur.
     */
    READ_UNCOMMITTED,
    /**
     * Prevents from dirty reads, but non-repeatable reads and phantoms can
     * occur.
     */
    READ_COMMITTED,
    /**
     * Prevents from dirty and non-repeatable reads, but phantoms can occur.
     */
    REPEATABLE_READ,
    /**
     * Prevents from any phenomena such as dirty reads, non-repeatable reads and
     * phantoms.
     */
    SERIALIZABLE,
    /**
     * Indicates that transactions are not supported by a database.
     */
    NONE
};
//------------------------------------------------------------------------------
/**
 * Represents a decimal number.
 *
 * A decimal number has a precision and a scale. The precision is the total
 * number of digits used to represent a number. The scale is the number of
 * digits after the decimal point.
 *
 * The maximum supported precision is 38. The scale must not be greater than the
 * precision.
 */
class ODBC_EXPORT decimal
{
public:
    /**
     * Constructs a decimal with value 0, precision 1 and scale 1.
     */
    decimal();

    /**
     * Constructs a decimal from a given integer value applying the given scale.
     *
     * Applying the scale means that the value is divided by 10^scale, e.g. 12
     * is converted to 1.2 if the given scale is 1.
     *
     * @param value      The value.
     * @param precision  The precision.
     * @param scale      The scale.
     */
    decimal(std::int64_t value, std::uint8_t precision, std::uint8_t scale = 0);

    /**
     * Constructs a decimal from a given integer value applying the given scale.
     *
     * Applying the scale means that the value is divided by 10^scale, e.g. 12
     * is converted to 1.2 if the given scale is 1.
     *
     * @param value      The value.
     * @param precision  The precision.
     * @param scale      The scale.
     */
    decimal(std::uint64_t value, std::uint8_t precision,
        std::uint8_t scale = 0);

    /**
     * Constructs a decimal from a given value applying the given scale.
     *
     * Applying the scale means that the value is divided by 10^scale, e.g. 12
     * is converted to 1.2 if the given scale is 1.
     *
     * @param value      The value.
     * @param precision  The precision.
     * @param scale      The scale.
     */
    decimal(const std::string& value, std::uint8_t precision,
        std::uint8_t scale = 0);

    /**
     * Constructs a decimal from a given value applying the given scale.
     *
     * Applying the scale means that the value is divided by 10^scale, e.g. 12
     * is converted to 1.2 if the given scale is 1.
     *
     * @param value      The value.
     * @param precision  The precision.
     * @param scale      The scale.
     */
    decimal(const char* value, std::uint8_t precision, std::uint8_t scale = 0);

    /**
     * Returns the precision of this decimal number.
     *
     * @return  Returns the precision of this decimal number.
     */
    std::uint8_t precision() const { return precision_; }

    /**
     * Returns the scale of this decimal number.
     *
     * @return  Returns the scale of this decimal number.
     */
    std::uint8_t scale() const { return scale_; }

    /**
     * Returns the signum of this decimal number.
     *
     * The signum is 1 if the number is positive, -1 if the number is negative,
     * 0 if the number is 0.
     *
     * @return  Returns the signum of this decimal number.
     */
    std::int8_t signum() const;

    /**
     * Returns the unscaled value of this decimal.
     *
     * In the unscaled value, the decimal point is ommited.
     *
     * @return  Returns the unscaled value of this decimal number.
     */
    const char* unscaledValue() const { return value_.c_str(); };

    /**
     * Returns a string representation of this number.
     *
     * The string representation has the usual human-readable format with sign
     * and decimal point.
     *
     * @return  Returns the human-readable string representation of this value.
     */
    std::string toString() const;

    /**
     * Checks whether this number is equal to another number.
     *
     * Only the scaled values are considered by the comparison. Hence, numbers
     * that have different scale and precisions can still be considered equal.
     *
     * @param other  Another number.
     * @return       Returns true if this number is equal to the other number,
     *               false otherwise.
     */
    bool operator==(const decimal& other) const;

    /**
     * Checks whether this number is not equal to another number.
     *
     * Only the scaled values are considered by the comparison. Hence, numbers
     * that have different scale and precisions can still be considered equal.
     *
     * @param other  Another number.
     * @return       Returns true if this number is not equal to the other
     *               number, false otherwise.
     */
    bool operator!=(const decimal& other) const;

    /**
     * Checks whether this number is less than another number.
     *
     * Only the scaled values are considered by the comparison. Hence, numbers
     * that have different scale and precisions can still be considered equal.
     *
     * @param other  Another number.
     * @return       Returns true if this number is less than the other number,
     *               false otherwise.
     */
    bool operator< (const decimal& other) const;

    /**
     * Checks whether this number is greater than another number.
     *
     * Only the scaled values are considered by the comparison. Hence, numbers
     * that have different scale and precisions can still be considered equal.
     *
     * @param other  Another number.
     * @return       Returns true if this number is greater than the other
     *               number, false otherwise.
     */
    bool operator> (const decimal& other) const;

    /**
     * Checks whether this number is not greater than another number.
     *
     * Only the scaled values are considered by the comparison. Hence, numbers
     * that have different scale and precisions can still be considered equal.
     *
     * @param other  Another number.
     * @return       Returns true if this number is not greater than the other
     *               number, false otherwise.
     */
    bool operator<=(const decimal& other) const;

    /**
     * Checks whether this number is not les than another number.
     *
     * Only the scaled values are considered by the comparison. Hence, numbers
     * that have different scale and precisions can still be considered equal.
     *
     * @param other  Another number.
     * @return       Returns true if this number is not less than the other
     *               number, false otherwise.
     */
    bool operator>=(const decimal& other) const;

private:
    int cmp(const decimal& other) const;

private:
    std::string value_;
    std::uint8_t precision_;
    std::uint8_t scale_;
};
//------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const decimal& d);
//------------------------------------------------------------------------------
/**
 * Represents a date consisting of year, month and day in month.
 */
class ODBC_EXPORT date
{
    friend class timestamp;

public:
    /**
     * Constructs a date with value 0000-01-01.
     */
    date();

    /**
     * Constructs a date with the given year, month and day in month.
     *
     * @param year   The year.
     * @param month  The month.
     * @param day    The day in month.
     */
    date(int year, int month, int day);

public:
    /**
     * Returns the year of this date object.
     *
     * @return Returns the year of this date object.
     */
    int year() const { return year_; }

    /**
     * Returns the month of this date object.
     *
     * @return Returns the month of this date object.
     */
    int month() const { return month_; }

    /**
     * Returns the day of month of this date object.
     *
     * @return Returns the day of month of this date object.
     */
    int day() const { return day_; }

    /**
     * Returns this date as a string in the format yyyy-mm-dd.
     *
     * @return  Returns this date as a string in the format yyyy-mm-dd.
     */
    std::string toString() const;

    /**
     * Checks whether this date is equal to another date.
     *
     * @param other  Another date.
     * @return       Returns true if this date is equal to the other date, false
     *               otherwise.
     */
    bool operator==(const date& other) const;

    /**
     * Checks whether this date is not equal to another date.
     *
     * @param other  Another date.
     * @return       Returns true if this date is not equal to the other date,
     *               false otherwise.
     */
    bool operator!=(const date& other) const;

    /**
     * Checks whether this date is before another date.
     *
     * @param other  Another date.
     * @return       Returns true if this date is before the other date, false
     *               otherwise.
     */
    bool operator< (const date& other) const;

    /**
     * Checks whether this date is after another date.
     *
     * @param other  Another date.
     * @return       Returns true if this date is after the other date, false
     *               otherwise.
     */
    bool operator> (const date& other) const;

    /**
     * Checks whether this date is not after another date.
     *
     * @param other  Another date.
     * @return       Returns true if this date is not after the other date,
     *               false otherwise.
     */
    bool operator<=(const date& other) const;

    /**
     * Checks whether this date is not before another date.
     *
     * @param other  Another date.
     * @return       Returns true if this date is not before the other date,
     *               false otherwise.
     */
    bool operator>=(const date& other) const;

private:
    static int daysInMonth(int year, int month);
    static int daysInFebruary(int year);

private:
    std::int16_t year_;
    std::uint8_t month_;
    std::uint8_t day_;
};
//------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const date& d);
//------------------------------------------------------------------------------
/**
 * Represents a time consisting of hour, minute and second.
 */
class ODBC_EXPORT time
{
    friend class timestamp;

public:
    /**
     * Constructs a time with value 00:00:00.
     */
    time();

    /**
     * Constructs a time with the given hour, minute and second.
     *
     * @param hour    The hour (0-23).
     * @param minute  The minute (0-59).
     * @param second  The second (0-59).
     */
    time(int hour, int minute, int second);

public:
    /**
     * Returns the hour.
     *
     * @return  Returns the hour.
     */
    int hour() const { return hour_; }

    /**
     * Returns the minute.
     *
     * @return  Returns the minute.
     */
    int minute() const { return minute_; }

    /**
     * Returns the second.
     *
     * @return  Returns the second.
     */
    int second() const { return second_; }

    /**
     * Returns the time in the format hh:mm::ss.
     *
     * @return  Returns the time in the format hh:mm::ss.
     */
    std::string toString() const;

    /**
     * Checks whether this time is equal to another time.
     *
     * @param other  Another time.
     * @return       Returns true if this time is equal to the other time, false
     *               otherwise.
     */
    bool operator==(const time& other) const;

    /**
     * Checks whether this time is not equal to another time.
     *
     * @param other  Another time.
     * @return       Returns true if this time is not equal to the other time,
     *               false otherwise.
     */
    bool operator!=(const time& other) const;

    /**
     * Checks whether this time is before another time.
     *
     * @param other  Another time.
     * @return       Returns true if this time is before the other time, false
     *               otherwise.
     */
    bool operator< (const time& other) const;

    /**
     * Checks whether this time is after another time.
     *
     * @param other  Another time.
     * @return       Returns true if this time is after the other time, false
     *               otherwise.
     */
    bool operator> (const time& other) const;

    /**
     * Checks whether this time is not after another time.
     *
     * @param other  Another time.
     * @return       Returns true if this time is not after the other time,
     *               false otherwise.
     */
    bool operator<=(const time& other) const;

    /**
     * Checks whether this time is not before another time.
     *
     * @param other  Another time.
     * @return       Returns true if this time is not before the other time,
     *               false otherwise.
     */
    bool operator>=(const time& other) const;

private:
    std::uint8_t hour_;
    std::uint8_t minute_;
    std::uint8_t second_;
};
//------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const time& t);
//------------------------------------------------------------------------------
/**
 * Represents a timestamp consisting of year, month, day in month, hour, minute,
 * second and milliseconds.
 */
class ODBC_EXPORT timestamp : public date, public time
{
public:
    /**
     * Creates a timestamp with value 0000-01-01 00:00:00.000.
     */
    timestamp();

    /**
     * Creates a timestamp with the given values.
     *
     * @param year          The year.
     * @param month         The month.
     * @param day           The day.
     * @param hour          The hour.
     * @param minute        The minute.
     * @param second        The second.
     * @param milliseconds  The milliseconds.
     */
    timestamp(int year, int month, int day, int hour, int minute, int second,
        int milliseconds);

public:
    /**
     * Returns the milliseconds.
     *
     * @return  Returns the milliseconds.
     */
    int milliseconds() const { return milliseconds_; }

    /**
     * Returns the timestamp as string in the format yyyy-MM-dd HH:mm:ss.SSS.
     *
     * @return  Returns the timestamp as string in the format
     *          yyyy-MM-dd HH:mm:ss.SSS.
     */
    std::string toString() const;

    /**
     * Checks whether this timestamp is equal to another timestamp.
     *
     * @param other  Another timestamp.
     * @return       Returns true if this timestamp is equal to the other
     *               timestamp, false otherwise.
     */
    bool operator==(const timestamp& other) const;

    /**
     * Checks whether this timestamp is not equal to another timestamp.
     *
     * @param other  Another timestamp.
     * @return       Returns true if this timestamp is not equal to the other
     *               timestamp, false otherwise.
     */
    bool operator!=(const timestamp& other) const;

    /**
     * Checks whether this timestamp is before another timestamp.
     *
     * @param other  Another timestamp.
     * @return       Returns true if this timestamp is before the other
     *               timestamp, false otherwise.
     */
    bool operator< (const timestamp& other) const;

    /**
     * Checks whether this timestamp is after another timestamp.
     *
     * @param other  Another timestamp.
     * @return       Returns true if this timestamp is after the other
     *               timestamp, false otherwise.
     */
    bool operator> (const timestamp& other) const;

    /**
     * Checks whether this timestamp is not after another timestamp.
     *
     * @param other  Another timestamp.
     * @return       Returns true if this timestamp is not after the other
     *               timestamp, false otherwise.
     */
    bool operator<=(const timestamp& other) const;

    /**
     * Checks whether this timestamp is not before another timestamp.
     *
     * @param other  Another timestamp.
     * @return       Returns true if this timestamp is not before the other
     *               timestamp, false otherwise.
     */
    bool operator>=(const timestamp& other) const;

private:
    std::uint16_t milliseconds_;
};
//------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const timestamp& ts);
//------------------------------------------------------------------------------
/**
 * Wrapper class for types that don't have a dedicated NULL value.
 *
 * @tparam T  The type to wrap.
 */
template<typename T>
class Nullable
{
public:
    /**
     * Constructs a NULL value.
     */
    Nullable() : val_(), isNull_(true) { }

    /**
     * Constructs a non-NULL value.
     *
     * @param val  The value to copy.
     */
    Nullable(const T& val) : val_(val), isNull_(false) { }

    /**
     * Constructs a non-NULL value.
     *
     * @param val  The value to move.
     */
    Nullable(T&& val) : val_(std::move(val)), isNull_(false) { }

    /**
     * Copy constructor.
     *
     * @param other  The instance to copy.
     */
    Nullable(const Nullable<T>& other);

    /**
     * Move constructor.
     *
     * @param other  The instance to move.
     */
    Nullable(Nullable<T>&& other);

    /**
     * Copy assignment operator.
     *
     * @param other  The instance to copy.
     * @return       Returns a reference to this instance.
     */
    Nullable<T>& operator=(const Nullable<T>& other);

    /**
     * Move assignment operator.
     *
     * @param other  The instance to move.
     * @return       Returns a reference to this instance.
     */
    Nullable<T>& operator=(Nullable<T>&& other);

public:
    /**
     * Checks whether this value is NULL.
     *
     * @return  Returns true if this value is NULL, false otherwise.
     */
    bool isNull() const { return isNull_; }

    /**
     * Returns a constant reference to the wrapped value.
     *
     * @return  Returns a constant reference to the wrapped value.
     */
    const T& operator*() const { return val_; }

    /**
     * Returns a reference to the wrapped value.
     *
     * @return  Returns a reference to the wrapped value.
     */
    T& operator*() { return val_; }

    /**
     * Returns a constant pointer to the wrapped value.
     *
     * @return  Returns a constant pointer to the wrapped value.
     */
    const T* operator->() const { return &val_; }

    /**
     * Returns a pointer to the wrapped value.
     *
     * @return  Returns a pointer to the wrapped value.
     */
    T* operator->() { return &val_; }

    /**
     * Checks whether this value is equal to another value.
     *
     * @return  Returns true if this value is equal to the other value, false
     *          otherwise.
     */
    bool operator==(const Nullable<T>& other) const;

    /**
     * Checks whether this value is not equal to another value.
     *
     * @return  Returns true if this value is not equal to the other value,
     *          false otherwise.
     */
    bool operator!=(const Nullable<T>& other) const;

    /**
     * Checks whether this value is less than another value.
     *
     * A NULL value is never considered less than a non-NULL value.
     *
     * @return  Returns true if this value is less than the other value, false
     *          otherwise.
     */
    bool operator< (const Nullable<T>& other) const;

    /**
     * Checks whether this value is greater than another value.
     *
     * A NULL value is always considered greater than a non-NULL value.
     *
     * @return  Returns true if this value is greater than the other value,
     *          false otherwise.
     */
    bool operator> (const Nullable<T>& other) const;

    /**
     * Checks whether this value is not greater than another value.
     *
     * A NULL value is always considered greater than a non-NULL value.
     *
     * @return  Returns true if this value is not greater than the other value,
     *          false otherwise.
     */
    bool operator<=(const Nullable<T>& other) const;

    /**
     * Checks whether this value is not less than another value.
     *
     * A NULL value is never considered less than a non-NULL value.
     *
     * @return  Returns true if this value is not less than the other value,
     *          false otherwise.
     */
    bool operator>=(const Nullable<T>& other) const;

private:
    T val_;
    bool isNull_;
};
//------------------------------------------------------------------------------
template<typename T, typename... Args>
Nullable<T> makeNullable(Args&&... args)
{
    return Nullable<T>(T(args...));
}
//------------------------------------------------------------------------------
template<typename T>
Nullable<T>::Nullable(const Nullable<T>& other)
    : val_(other.val_)
    , isNull_(other.isNull_)
{
}
//------------------------------------------------------------------------------
template<typename T>
Nullable<T>::Nullable(Nullable<T>&& other)
    : val_(std::move(other.val_))
    , isNull_(other.isNull_)
{
}
//------------------------------------------------------------------------------
template<typename T>
Nullable<T>& Nullable<T>::operator=(const Nullable<T>& other)
{
    val_ = other.val_;
    isNull_ = other.isNull_;
    return *this;
}
//------------------------------------------------------------------------------
template<typename T>
Nullable<T>& Nullable<T>::operator=(Nullable<T>&& other)
{
    val_ = std::move(other.val_);
    isNull_ = other.isNull_;
    return *this;
}
//------------------------------------------------------------------------------
template<typename T>
bool Nullable<T>::operator==(const Nullable<T>& other) const
{
    if (isNull())
        return other.isNull();
    if (other.isNull())
        return false;
    return **this == *other;
}
//------------------------------------------------------------------------------
template<typename T>
bool Nullable<T>::operator!=(const Nullable<T>& other) const
{
    return !(*this == other);
}
//------------------------------------------------------------------------------
template<typename T>
bool Nullable<T>::operator<(const Nullable<T>& other) const
{
    if (isNull())
        return false;
    if (other.isNull())
        return true;
    return **this < *other;
}
//------------------------------------------------------------------------------
template<typename T>
bool Nullable<T>::operator>(const Nullable<T>& other) const
{
    if (isNull())
        return !other.isNull();
    if (other.isNull())
        return false;
    return **this > *other;
}
//------------------------------------------------------------------------------
template<typename T>
bool Nullable<T>::operator<=(const Nullable<T>& other) const
{
    return !(*this > other);
}
//------------------------------------------------------------------------------
template<typename T>
bool Nullable<T>::operator>=(const Nullable<T>& other) const
{
    return !(*this < other);
}
//------------------------------------------------------------------------------
template<typename T>
std::ostream& operator<<(std::ostream& out, const Nullable<T>& val)
{
    if (val.isNull())
        out << "<NULL>";
    else
        out << *val;
    return out;
}
//------------------------------------------------------------------------------
typedef Nullable<bool>              Boolean;
typedef Nullable<std::int8_t>       Byte;
typedef Nullable<std::uint8_t>      UByte;
typedef Nullable<std::int16_t>      Short;
typedef Nullable<std::uint16_t>     UShort;
typedef Nullable<std::int32_t>      Int;
typedef Nullable<std::uint32_t>     UInt;
typedef Nullable<std::int64_t>      Long;
typedef Nullable<std::uint64_t>     ULong;
typedef Nullable<decimal>           Decimal;
typedef Nullable<float>             Float;
typedef Nullable<double>            Double;
typedef Nullable<std::string>       String;
typedef Nullable<std::u16string>    NString;
typedef Nullable<std::vector<char>> Binary;
typedef Nullable<date>              Date;
typedef Nullable<time>              Time;
typedef Nullable<timestamp>         Timestamp;
//------------------------------------------------------------------------------
template<>
inline std::ostream& operator<<(std::ostream& out, const NString& val)
{
    if (val.isNull()) {
        out << "<NULL>";
        return out;
    }
    for (char16_t c : *val)
        out << ((c <= u'~') ? (char)c : '?');
    return out;
}
//------------------------------------------------------------------------------
template<>
inline std::ostream& operator<<(std::ostream& out, const Binary& val)
{
    if (val.isNull()) {
        out << "<NULL>";
        return out;
    }
    const char* hexdigits = "0123456789ABCDEF";
    for (std::size_t i = 0; i < val->size(); ++i)
    {
        if ((i % 16) == 0)
            out << std::endl;
        else if (((i + 8) % 16) == 0)
            out << "  ";
        else
            out << ' ';
        unsigned char uc = (unsigned char)(*val)[i];
        out << hexdigits[uc >> 4];
        out << hexdigits[uc & 0xF];
    }
    return out;
}
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
