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

QString QgsRaster::contrastEnhancementLimitsAsString( ContrastEnhancementLimits theLimits )
{
  switch ( theLimits )
  {
    case QgsRaster::ContrastEnhancementMinMax:
      return "MinMax";
    case QgsRaster::ContrastEnhancementStdDev:
      return "StdDev";
    case QgsRaster::ContrastEnhancementCumulativeCut:
      return "CumulativeCut";
    default:
      break;
  }
  return "None";
}

QgsRaster::ContrastEnhancementLimits QgsRaster::contrastEnhancementLimitsFromString( const QString& theLimits )
{
  if ( theLimits == "MinMax" )
  {
    return ContrastEnhancementMinMax;
  }
  else if ( theLimits == "StdDev" )
  {
    return ContrastEnhancementStdDev;
  }
  else if ( theLimits == "CumulativeCut" )
  {
    return ContrastEnhancementCumulativeCut;
  }
  return ContrastEnhancementNone;
}

bool QgsRaster::isRepresentableValue( double value, QGis::DataType dataType )
{
  switch ( dataType )
  {
    case QGis::Byte:
      return value >= std::numeric_limits<quint8>::min() && value <= std::numeric_limits<quint8>::max();
    case QGis::UInt16:
      return value >= std::numeric_limits<quint16>::min() && value <= std::numeric_limits<quint16>::max();
    case QGis::Int16:
      return value >= std::numeric_limits<qint16>::min() && value <= std::numeric_limits<qint16>::max();
    case QGis::UInt32:
      return value >= std::numeric_limits<quint32>::min() && value <= std::numeric_limits<quint32>::max();
    case QGis::Int32:
      return value >= std::numeric_limits<qint32>::min() && value <= std::numeric_limits<qint32>::max();
    case QGis::Float32:
      return qIsNaN( value ) || qIsInf( value ) ||
             ( value >= -std::numeric_limits<float>::max() && value <= std::numeric_limits<float>::max() );
    default:
      return true;
      break;
  }
}

double QgsRaster::representableValue( double value, QGis::DataType dataType )
{
  switch ( dataType )
  {
    case QGis::Byte:
      return static_cast<quint8>( value );
    case QGis::UInt16:
      return static_cast<quint16>( value );
    case QGis::Int16:
      return static_cast<qint16>( value );
    case QGis::UInt32:
      return static_cast<quint32>( value );
    case QGis::Int32:
      return static_cast<qint32>( value );
    case QGis::Float32:
      return static_cast<float>( value );
    default:
      break;
  }
  return value;
}
