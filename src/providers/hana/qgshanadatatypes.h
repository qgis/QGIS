/***************************************************************************
   qgshanadatatypes.h
   --------------------------------------
   Date      : 05-08-2024
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

enum class QgsHanaDataType
{
  /// Unknown data type.
  Unknown,
  /// 64-bit integer value.
  BigInt,
  /// Binary data of fixed length.
  Binary,
  /// Single bit binary data.
  Bit,
  /// Boolean value.
  Boolean,
  /// Character string of fixed string length.
  Char,
  /// Year, month, and day fields.
  Date,
  /// Year, month, and day fields.
  DateTime,
  /// Signed, exact, numeric value.
  Decimal,
  /// Double-precision floating point number.
  Double,
  /// Floating point number with driver-specific precision.
  Float,
  /// 32-bit integer value.
  Integer,
  /// Variable length binary data.
  LongVarBinary,
  /// Variable length character data.
  LongVarChar,
  /// Signed, exact, numeric value.
  Numeric,
  /// Single-precision floating point number.
  Real,
  /// 16-bit integer value.
  SmallInt,
  /// Hour, minute, and second fields.
  Time,
  /// Year, month, day, hour, minute, and second fields.
  Timestamp,
  /// 8-bit integer value.
  TinyInt,
  /// Year, month, and day fields.
  TypeDate,
  /// Hour, minute, and second fields.
  TypeTime,
  /// Year, month, day, hour, minute, and second fields.
  TypeTimestamp,
  /// Variable length binary data.
  VarBinary,
  /// Variable-length character string.
  VarChar,
  /// Unicode character string of fixed string length.
  WChar,
  /// Unicode variable-length character data.
  WLongVarChar,
  /// Unicode variable-length character string.
  WVarChar,
  /// ST_GEOMETRY/ST_POINT value.
  Geometry,
  /// REAL_VECTOR value.
  RealVector
};

namespace QgsHanaDataTypeUtils
{

  QgsHanaDataType fromInt( int v ) noexcept;

} // namespace QgsHanaDataTypeUtils

#endif // QGSHANADATATYPES_H
