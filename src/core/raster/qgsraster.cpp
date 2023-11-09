/***************************************************************************
            qgsraster.cpp - Raster namespace
     --------------------------------------
    Date                 : Apr, 2013
    Copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>

#include "qgsraster.h"

bool QgsRaster::isRepresentableValue( double value, Qgis::DataType dataType )
{
  switch ( dataType )
  {
    case Qgis::DataType::Byte:
      return value >= std::numeric_limits<quint8>::min() && value <= std::numeric_limits<quint8>::max();
    case Qgis::DataType::Int8:
      return value >= std::numeric_limits<qint8>::min() && value <= std::numeric_limits<qint8>::max();
    case Qgis::DataType::UInt16:
      return value >= std::numeric_limits<quint16>::min() && value <= std::numeric_limits<quint16>::max();
    case Qgis::DataType::Int16:
      return value >= std::numeric_limits<qint16>::min() && value <= std::numeric_limits<qint16>::max();
    case Qgis::DataType::UInt32:
      return value >= std::numeric_limits<quint32>::min() && value <= std::numeric_limits<quint32>::max();
    case Qgis::DataType::Int32:
      return value >= std::numeric_limits<qint32>::min() && value <= std::numeric_limits<qint32>::max();
    case Qgis::DataType::Float32:
      return std::isnan( value ) || std::isinf( value ) ||
             ( value >= -std::numeric_limits<float>::max() && value <= std::numeric_limits<float>::max() );
    case Qgis::DataType::Float64:
      return true;
    case Qgis::DataType::CInt16:
    case Qgis::DataType::CInt32:
    case Qgis::DataType::CFloat32:
    case Qgis::DataType::CFloat64:
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
    case Qgis::DataType::UnknownDataType:
      break;
  }
  return true;
}

double QgsRaster::representableValue( double value, Qgis::DataType dataType )
{
  switch ( dataType )
  {
    case Qgis::DataType::Byte:
      return static_cast<quint8>( value );
    case Qgis::DataType::Int8:
      return static_cast<qint8>( value );
    case Qgis::DataType::UInt16:
      return static_cast<quint16>( value );
    case Qgis::DataType::Int16:
      return static_cast<qint16>( value );
    case Qgis::DataType::UInt32:
      return static_cast<quint32>( value );
    case Qgis::DataType::Int32:
      return static_cast<qint32>( value );
    case Qgis::DataType::Float32:
      return static_cast<float>( value );
    case Qgis::DataType::Float64:
      return value;
    case Qgis::DataType::CInt16:
    case Qgis::DataType::CInt32:
    case Qgis::DataType::CFloat32:
    case Qgis::DataType::CFloat64:
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
    case Qgis::DataType::UnknownDataType:
      break;
  }
  return value;
}
