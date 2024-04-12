/***************************************************************************
   qgshanadatatypes.h
   --------------------------------------
   Date      : 10-04-2024
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANADATATYPES_H
#define QGSHANADATATYPES_H

#include "odbc/Types.h"

using namespace NS_ODBC;

class QgsHanaDataTypes
{
    QgsHanaDataTypes() = delete;

  public:
    /// Unknown data type.
    static constexpr int Unknown = SQLDataTypes::Unknown;
    /// 64-bit integer value.
    static constexpr int BigInt = SQLDataTypes::BigInt;
    /// Binary data of fixed length.
    static constexpr int Binary = SQLDataTypes::Binary;
    /// Single bit binary data.
    static constexpr int Bit = SQLDataTypes::Bit;
    /// Boolean value.
    static constexpr int Boolean = SQLDataTypes::Boolean;
    /// Character string of fixed string length.
    static constexpr int Char = SQLDataTypes::Char;
    /// Year, month, and day fields.
    static constexpr int Date = SQLDataTypes::Date;
    /// Year, month, and day fields.
    static constexpr int DateTime = SQLDataTypes::DateTime;
    /// Signed, exact, numeric value.
    static constexpr int Decimal = SQLDataTypes::Decimal;
    /// Double-precision floating point number.
    static constexpr int Double = SQLDataTypes::Double;
    /// Floating point number with driver-specific precision.
    static constexpr int Float = SQLDataTypes::Float;
    /// 32-bit integer value.
    static constexpr int Integer = SQLDataTypes::Integer;
    /// Variable length binary data.
    static constexpr int LongVarBinary = SQLDataTypes::LongVarBinary;
    /// Variable length character data.
    static constexpr int LongVarChar = SQLDataTypes::LongVarChar;
    /// Signed, exact, numeric value.
    static constexpr int Numeric = SQLDataTypes::Numeric;
    /// Single-precision floating point number.
    static constexpr int Real = SQLDataTypes::Real;
    /// 16-bit integer value.
    static constexpr int SmallInt = SQLDataTypes::SmallInt;
    /// Hour, minute, and second fields.
    static constexpr int Time = SQLDataTypes::Time;
    /// Year, month, day, hour, minute, and second fields.
    static constexpr int Timestamp = SQLDataTypes::Timestamp;
    /// 8-bit integer value.
    static constexpr int TinyInt = SQLDataTypes::TinyInt;
    /// Year, month, and day fields.
    static constexpr int TypeDate = SQLDataTypes::TypeDate;
    /// Hour, minute, and second fields.
    static constexpr int TypeTime = SQLDataTypes::TypeTime;
    /// Year, month, day, hour, minute, and second fields.
    static constexpr int TypeTimestamp = SQLDataTypes::TypeTimestamp;
    /// Variable length binary data.
    static constexpr int VarBinary = SQLDataTypes::VarBinary;
    /// Variable-length character string.
    static constexpr int VarChar = SQLDataTypes::VarChar;
    /// Unicode character string of fixed string length.
    static constexpr int WChar = SQLDataTypes::WChar;
    /// Unicode variable-length character data.
    static constexpr int WLongVarChar = SQLDataTypes::WLongVarChar;
    /// Unicode variable-length character string.
    static constexpr int WVarChar = SQLDataTypes::WVarChar;
    /// ST_GEOMETRY/ST_POINT value.
    static constexpr int Geometry = 29812;
    /// REAL_VECTOR value.
    static constexpr int RealVector = 29814;
};

#endif // QGSHANADATATYPES_H
