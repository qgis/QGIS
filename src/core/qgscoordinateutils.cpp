/***************************************************************************
                             qgscoordinateutils.cpp
                             ----------------------
    begin                : February 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscoordinateutils.h"
#include "qgsproject.h"
#include "qgis.h"

///@cond NOT_STABLE_API

int QgsCoordinateUtils::calculateCoordinatePrecision( double mapUnitsPerPixel, const QgsCoordinateReferenceSystem& mapCrs )
{
  // Get the display precision from the project settings
  bool automatic = QgsProject::instance()->readBoolEntry( "PositionPrecision", "/Automatic" );
  int dp = 0;

  if ( automatic )
  {
    QString format = QgsProject::instance()->readEntry( "PositionPrecision", "/DegreeFormat", "MU" );
    bool formatGeographic = ( format == "DM" || format == "DMS" || format == "D" );

    // we can only calculate an automatic precision if one of these is true:
    // - both map CRS and format are geographic
    // - both map CRS and format are not geographic
    // - map CRS is geographic but format is not geographic (i.e. map units)
    if ( mapCrs.geographicFlag() || !formatGeographic )
    {
      // Work out a suitable number of decimal places for the coordinates with the aim of always
      // having enough decimal places to show the difference in position between adjacent pixels.
      // Also avoid taking the log of 0.
      if ( !qgsDoubleNear( mapUnitsPerPixel, 0.0 ) )
        dp = static_cast<int>( ceil( -1.0 * log10( mapUnitsPerPixel ) ) );
    }
    else
    {
      dp = format == "D" ? 4 : 2; //guess sensible fallback
    }
  }
  else
    dp = QgsProject::instance()->readNumEntry( "PositionPrecision", "/DecimalPlaces" );

  // Keep dp sensible
  if ( dp < 0 )
    dp = 0;

  return dp;
}

QString QgsCoordinateUtils::formatCoordinateForProject( const QgsPoint& point, const QgsCoordinateReferenceSystem& destCrs, int precision )
{
  QString format = QgsProject::instance()->readEntry( "PositionPrecision", "/DegreeFormat", "MU" );

  QgsPoint geo = point;
  if ( format == "DM" || format == "DMS" || format == "D" )
  {
    // degrees
    if ( destCrs.isValid() && !destCrs.geographicFlag() )
    {
      // need to transform to geographic coordinates
      QgsCoordinateTransform ct( destCrs, QgsCoordinateReferenceSystem( GEOSRID ) );
      try
      {
        geo = ct.transform( point );
      }
      catch ( QgsCsException& )
      {
        return QString();
      }
    }

    if ( format == "DM" )
      return geo.toDegreesMinutes( precision, true, true );
    else if ( format == "DMS" )
      return geo.toDegreesMinutesSeconds( precision, true, true );
    else
      return geo.toString( precision );
  }
  else
  {
    // coordinates in map units
    return point.toString( precision );
  }
}

///@endcond
