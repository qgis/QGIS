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
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsproject.h"
#include "qgis.h"
#include "qgsexception.h"
#include "qgscoordinateformatter.h"
///@cond NOT_STABLE_API

int QgsCoordinateUtils::calculateCoordinatePrecision( double mapUnitsPerPixel, const QgsCoordinateReferenceSystem &mapCrs, QgsProject *project )
{
  if ( !project )
    project = QgsProject::instance();
  // Get the display precision from the project settings
  bool automatic = project->readBoolEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ) );
  int dp = 0;

  if ( automatic )
  {
    QString format = project->readEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DegreeFormat" ), QStringLiteral( "MU" ) );
    bool formatGeographic = ( format == QLatin1String( "DM" ) || format == QLatin1String( "DMS" ) || format == QLatin1String( "D" ) );

    // we can only calculate an automatic precision if one of these is true:
    // - both map CRS and format are geographic
    // - both map CRS and format are not geographic
    // - map CRS is geographic but format is not geographic (i.e. map units)
    if ( mapCrs.isGeographic() || !formatGeographic )
    {
      // Work out a suitable number of decimal places for the coordinates with the aim of always
      // having enough decimal places to show the difference in position between adjacent pixels.
      // Also avoid taking the log of 0.
      if ( !qgsDoubleNear( mapUnitsPerPixel, 0.0 ) )
        dp = static_cast<int>( std::ceil( -1.0 * std::log10( mapUnitsPerPixel ) ) );
    }
    else
    {
      if ( format == QLatin1String( "D" ) )
        dp = 4;
      else
        dp = 2;
    }
  }
  else
    dp = project->readNumEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ) );

  // Keep dp sensible
  if ( dp < 0 )
    dp = 0;

  return dp;
}

QString QgsCoordinateUtils::formatCoordinateForProject( QgsProject *project, const QgsPointXY &point, const QgsCoordinateReferenceSystem &destCrs, int precision )
{
  if ( !project )
    return QString();

  QString format = project->readEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DegreeFormat" ), QStringLiteral( "MU" ) );

  QgsPointXY geo = point;
  if ( format == QLatin1String( "DM" ) || format == QLatin1String( "DMS" ) || format == QLatin1String( "D" ) )
  {
    // degrees
    if ( destCrs.isValid() && !destCrs.isGeographic() )
    {
      // need to transform to geographic coordinates
      QgsCoordinateTransform ct( destCrs, QgsCoordinateReferenceSystem( GEOSRID ), project );
      try
      {
        geo = ct.transform( point );
      }
      catch ( QgsCsException & )
      {
        return QString();
      }
    }

    if ( format == QLatin1String( "DM" ) )
      return QgsCoordinateFormatter::format( geo, QgsCoordinateFormatter::FormatDegreesMinutes, precision, QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds | QgsCoordinateFormatter::FlagDegreesUseStringSuffix );
    else if ( format == QLatin1String( "DMS" ) )
      return QgsCoordinateFormatter::format( geo, QgsCoordinateFormatter::FormatDegreesMinutesSeconds, precision, QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds | QgsCoordinateFormatter::FlagDegreesUseStringSuffix );
    else
      return QgsCoordinateFormatter::asPair( geo.x(), geo.y(), precision );
  }
  else
  {
    // coordinates in map units
    return QgsCoordinateFormatter::asPair( point.x(), point.y(), precision );
  }
}

///@endcond
