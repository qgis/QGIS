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
    case Qgis::Byte:
      return value >= std::numeric_limits<quint8>::min() && value <= std::numeric_limits<quint8>::max();
    case Qgis::UInt16:
      return value >= std::numeric_limits<quint16>::min() && value <= std::numeric_limits<quint16>::max();
    case Qgis::Int16:
      return value >= std::numeric_limits<qint16>::min() && value <= std::numeric_limits<qint16>::max();
    case Qgis::UInt32:
      return value >= std::numeric_limits<quint32>::min() && value <= std::numeric_limits<quint32>::max();
    case Qgis::Int32:
      return value >= std::numeric_limits<qint32>::min() && value <= std::numeric_limits<qint32>::max();
    case Qgis::Float32:
      return std::isnan( value ) || std::isinf( value ) ||
             ( value >= -std::numeric_limits<float>::max() && value <= std::numeric_limits<float>::max() );
    default:
      return true;
      break;
  }
}

double QgsRaster::representableValue( double value, Qgis::DataType dataType )
{
  switch ( dataType )
  {
    case Qgis::Byte:
      return static_cast<quint8>( value );
    case Qgis::UInt16:
      return static_cast<quint16>( value );
    case Qgis::Int16:
      return static_cast<qint16>( value );
    case Qgis::UInt32:
      return static_cast<quint32>( value );
    case Qgis::Int32:
      return static_cast<qint32>( value );
    case Qgis::Float32:
      return static_cast<float>( value );
    default:
      break;
  }
  return value;
}
