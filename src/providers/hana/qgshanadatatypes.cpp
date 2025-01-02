/***************************************************************************
   qgshanadatatypes.cpp
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

#include "qgshanadatatypes.h"
#include "odbc/Types.h"

using namespace NS_ODBC;

namespace QgsHanaDataTypeUtils
{

  QgsHanaDataType fromInt( int v ) noexcept
  {
    switch ( v )
    {
      case SQLDataTypes::BigInt:
        return QgsHanaDataType::BigInt;
      case SQLDataTypes::Binary:
        return QgsHanaDataType::Binary;
      case SQLDataTypes::Bit:
        return QgsHanaDataType::Bit;
      case SQLDataTypes::Boolean:
        return QgsHanaDataType::Boolean;
      case SQLDataTypes::Char:
        return QgsHanaDataType::Char;
      //case SQLDataTypes::Date:
      case SQLDataTypes::DateTime:
        return QgsHanaDataType::DateTime;
      case SQLDataTypes::Decimal:
        return QgsHanaDataType::Decimal;
      case SQLDataTypes::Double:
        return QgsHanaDataType::Double;
      case SQLDataTypes::Float:
        return QgsHanaDataType::Float;
      case SQLDataTypes::Integer:
        return QgsHanaDataType::Integer;
      case SQLDataTypes::LongVarBinary:
        return QgsHanaDataType::LongVarBinary;
      case SQLDataTypes::LongVarChar:
        return QgsHanaDataType::LongVarChar;
      case SQLDataTypes::Numeric:
        return QgsHanaDataType::Numeric;
      case SQLDataTypes::Real:
        return QgsHanaDataType::Real;
      case SQLDataTypes::SmallInt:
        return QgsHanaDataType::SmallInt;
      case SQLDataTypes::Time:
        return QgsHanaDataType::Time;
      case SQLDataTypes::Timestamp:
        return QgsHanaDataType::Timestamp;
      case SQLDataTypes::TinyInt:
        return QgsHanaDataType::TinyInt;
      case SQLDataTypes::TypeDate:
        return QgsHanaDataType::TypeDate;
      case SQLDataTypes::TypeTime:
        return QgsHanaDataType::TypeTime;
      case SQLDataTypes::TypeTimestamp:
        return QgsHanaDataType::TypeTimestamp;
      case SQLDataTypes::VarBinary:
        return QgsHanaDataType::VarBinary;
      case SQLDataTypes::VarChar:
        return QgsHanaDataType::VarChar;
      case SQLDataTypes::WChar:
        return QgsHanaDataType::WChar;
      case SQLDataTypes::WLongVarChar:
        return QgsHanaDataType::WLongVarChar;
      case SQLDataTypes::WVarChar:
        return QgsHanaDataType::WVarChar;
      case 29812:
        return QgsHanaDataType::Geometry;
      case 29814:
        return QgsHanaDataType::RealVector;
      default:
        return QgsHanaDataType::Unknown;
    }
  }

} // namespace QgsHanaDataTypeUtils
