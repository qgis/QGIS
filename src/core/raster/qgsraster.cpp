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

#include "qgsraster.h"

QString QgsRaster::contrastEnhancementLimitsAsString( ContrastEnhancementLimits theLimits )
{
  switch ( theLimits )
  {
    case QgsRaster::ContrastEnhancementMinMax:
      return "MinMax";
      break;
    case QgsRaster::ContrastEnhancementStdDev:
      return "StdDev";
      break;
    case QgsRaster::ContrastEnhancementCumulativeCut:
      return "CumulativeCut";
      break;
    default:
      break;
  }
  return "None";
}

QgsRaster::ContrastEnhancementLimits QgsRaster::contrastEnhancementLimitsFromString( QString theLimits )
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
