#ifndef ODBC_INTERNAL_TYPEINFO_H_INCLUDED
#define ODBC_INTERNAL_TYPEINFO_H_INCLUDED
//------------------------------------------------------------------------------
#include <odbc/Config.h>
#include <odbc/Types.h>
#include <odbc/internal/Odbc.h>
#include <cassert>
#include <cstdint>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * Structure that contains the ODBC value type and parameter type for a C++
 * type.
 *
 * @tparam T  The C++ type.
 */
template<typename T>
struct TypeToOdbc
{
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<bool>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_BIT;
    constexpr static std::int16_t PARAMTYPE = SQL_BIT;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<std::uint8_t>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_UTINYINT;
    constexpr static std::int16_t PARAMTYPE = SQL_TINYINT;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<std::int8_t>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_STINYINT;
    constexpr static std::int16_t PARAMTYPE = SQL_TINYINT;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<std::uint16_t>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_USHORT;
    constexpr static std::int16_t PARAMTYPE = SQL_SMALLINT;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<std::int16_t>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_SSHORT;
    constexpr static std::int16_t PARAMTYPE = SQL_SMALLINT;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<std::uint32_t>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_ULONG;
    constexpr static std::int16_t PARAMTYPE = SQL_INTEGER;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<std::int32_t>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_SLONG;
    constexpr static std::int16_t PARAMTYPE = SQL_INTEGER;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<std::uint64_t>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_UBIGINT;
    constexpr static std::int16_t PARAMTYPE = SQL_BIGINT;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<std::int64_t>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_SBIGINT;
    constexpr static std::int16_t PARAMTYPE = SQL_BIGINT;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<float>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_FLOAT;
    constexpr static std::int16_t PARAMTYPE = SQL_REAL;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<double>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_DOUBLE;
    constexpr static std::int16_t PARAMTYPE = SQL_DOUBLE;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<date>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_TYPE_DATE;
    constexpr static std::int16_t PARAMTYPE = SQL_TYPE_DATE;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<time>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_TYPE_TIME;
    constexpr static std::int16_t PARAMTYPE = SQL_TYPE_TIME;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<timestamp>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_TYPE_TIMESTAMP;
    constexpr static std::int16_t PARAMTYPE = SQL_TYPE_TIMESTAMP;
};
//------------------------------------------------------------------------------
template<>
struct TypeToOdbc<decimal>
{
    constexpr static std::int16_t VALUETYPE = SQL_C_NUMERIC;
    constexpr static std::int16_t PARAMTYPE = SQL_DECIMAL;
};
//------------------------------------------------------------------------------
struct TypeInfo
{
    /**
     * Returns the parameter type to use for a given value type.
     *
     * @param valueType  The ODBC value type.
     * @return           The parameter type to use for a given value type.
     */
    static std::int16_t getParamTypeForValueType(std::int16_t valueType)
    {
        switch (valueType)
        {
        case SQL_C_CHAR:
            return SQL_LONGVARCHAR;
        case SQL_C_WCHAR:
            return SQL_WLONGVARCHAR;
        case SQL_C_SSHORT:
        case SQL_C_USHORT:
            return SQL_SMALLINT;
        case SQL_C_SLONG:
        case SQL_C_ULONG:
            return SQL_INTEGER;
        case SQL_C_FLOAT:
            return SQL_REAL;
        case SQL_C_DOUBLE:
            return SQL_DOUBLE;
        case SQL_C_BIT:
            return SQL_BIT;
        case SQL_C_STINYINT:
        case SQL_C_UTINYINT:
            return SQL_TINYINT;
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
            return SQL_BIGINT;
        case SQL_C_BINARY:
            return SQL_LONGVARBINARY;
        case SQL_C_TYPE_DATE:
            return SQL_TYPE_DATE;
        case SQL_C_TYPE_TIME:
            return SQL_TYPE_TIME;
        case SQL_C_TYPE_TIMESTAMP:
            return SQL_TYPE_TIMESTAMP;
        case SQL_C_NUMERIC:
            return SQL_DECIMAL;
        }
        assert(false);
        return SQL_UNKNOWN_TYPE;
    }

    /**
     * Returns a human-readable name for a value type.
     *
     * @param valueType  The ODBC value type.
     * @return           A human-readable name for the value type.
     */
    static const char* getValueTypeName(std::int16_t valueType)
    {
        switch (valueType)
        {
        case SQL_C_CHAR:
            return "CLOB";
        case SQL_C_WCHAR:
            return "NCLOB";
        case SQL_C_SSHORT:
        case SQL_C_USHORT:
            return "SHORT";
        case SQL_C_SLONG:
        case SQL_C_ULONG:
            return "INTEGER";
        case SQL_C_FLOAT:
            return "REAL";
        case SQL_C_DOUBLE:
            return "DOUBLE";
        case SQL_C_BIT:
            return "BOOLEAN";
        case SQL_C_STINYINT:
        case SQL_C_UTINYINT:
            return "TINYINT";
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
            return "BIGINT";
        case SQL_C_BINARY:
            return "BLOB";
        case SQL_C_TYPE_DATE:
            return "DATE";
        case SQL_C_TYPE_TIME:
            return "TIME";
        case SQL_C_TYPE_TIMESTAMP:
            return "TIMESTAMP";
        case SQL_C_NUMERIC:
            return "DECIMAL";
        }
        assert(false);
        return "<unknown>";
    }

    /**
     * Returns a value's size in bytes of a given value type.
     *
     * This method will return 0 if values of the given type are not of fixed
     * size.
     *
     * @param valueType  The ODBC value type.
     * @return           A value's size in bytes of a given value type or 0 if a
     *                   value of this type does not have a fixed size.
     */
    static std::size_t getSizeOfValueFromValueType(std::int16_t valueType)
    {
        switch (valueType)
        {
        case SQL_C_CHAR:
        case SQL_C_WCHAR:
        case SQL_C_BINARY:
            return 0;
        case SQL_C_BIT:
        case SQL_C_STINYINT:
        case SQL_C_UTINYINT:
            return 1;
        case SQL_C_SSHORT:
        case SQL_C_USHORT:
            return 2;
        case SQL_C_SLONG:
        case SQL_C_ULONG:
        case SQL_C_FLOAT:
            return 4;
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
        case SQL_C_DOUBLE:
            return 8;
        case SQL_C_TYPE_DATE:
            return sizeof(SQL_DATE_STRUCT);
        case SQL_C_TYPE_TIME:
            return sizeof(SQL_TIME_STRUCT);
        case SQL_C_TYPE_TIMESTAMP:
            return sizeof(SQL_TIMESTAMP_STRUCT);
        case SQL_C_NUMERIC:
            return sizeof(SQL_NUMERIC_STRUCT);
        }
        assert(false);
        return 0;
    }
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
