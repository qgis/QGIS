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
#include "qgsrectangle.h"
#include <QRegularExpression>

///@cond NOT_STABLE_API

int QgsCoordinateUtils::calculateCoordinatePrecision( double mapUnitsPerPixel, const QgsCoordinateReferenceSystem &mapCrs, QgsProject *project )
{
  if ( !project )
    project = QgsProject::instance();
  // Get the display precision from the project settings
  const bool automatic = project->readBoolEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ) );
  int dp = 0;

  if ( automatic )
  {
    const QString format = project->readEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DegreeFormat" ), QStringLiteral( "MU" ) );
    const bool formatGeographic = ( format == QLatin1String( "DM" ) || format == QLatin1String( "DMS" ) || format == QLatin1String( "D" ) );

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

int QgsCoordinateUtils::calculateCoordinatePrecisionForCrs( const QgsCoordinateReferenceSystem &crs, QgsProject *project )
{
  QgsProject *prj = project;
  if ( !prj )
  {
    prj = QgsProject::instance();
  }

  const bool automatic = prj->readBoolEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ) );
  if ( !automatic )
  {
    return prj->readNumEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 6 );
  }

  const QgsUnitTypes::DistanceUnit unit = crs.mapUnits();
  if ( unit == QgsUnitTypes::DistanceDegrees )
  {
    return 8;
  }
  else
  {
    return 3;
  }
}

QString QgsCoordinateUtils::formatCoordinateForProject( QgsProject *project, const QgsPointXY &point, const QgsCoordinateReferenceSystem &destCrs, int precision )
{
  if ( !project )
    return QString();

  const QString format = project->readEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DegreeFormat" ), QStringLiteral( "MU" ) );

  QgsPointXY geo = point;
  if ( format == QLatin1String( "DM" ) || format == QLatin1String( "DMS" ) || format == QLatin1String( "D" ) )
  {
    // degrees
    if ( destCrs.isValid() && !destCrs.isGeographic() )
    {
      // need to transform to geographic coordinates
      const QgsCoordinateTransform ct( destCrs, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), project );
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

QString QgsCoordinateUtils::formatExtentForProject( QgsProject *project, const QgsRectangle &extent, const QgsCoordinateReferenceSystem &destCrs, int precision )
{
  const QgsPointXY p1( extent.xMinimum(), extent.yMinimum() );
  const QgsPointXY p2( extent.xMaximum(), extent.yMaximum() );
  return QStringLiteral( "%1 : %2" ).arg( QgsCoordinateUtils::formatCoordinateForProject( project, p1, destCrs, precision ),
                                          QgsCoordinateUtils::formatCoordinateForProject( project, p2, destCrs, precision ) );
}

double QgsCoordinateUtils::degreeToDecimal( const QString &string, bool *ok, bool *isEasting )
{
  const QString negative( QStringLiteral( "swSW" ) );
  const QString easting( QStringLiteral( "eEwW" ) );
  double value = 0.0;
  bool okValue = false;

  if ( ok )
  {
    *ok = false;
  }
  else
  {
    ok = &okValue;
  }

  QRegularExpression degreeWithSuffix( QStringLiteral( "^\\s*([0-9\\-\\.]*)\\s*([NSEWnsew])\\s*$" ) );
  QRegularExpressionMatch match = degreeWithSuffix.match( string );
  if ( match.hasMatch() )
  {
    const QString suffix = match.captured( 2 );
    value = std::abs( match.captured( 1 ).toDouble( ok ) );
    if ( ok )
    {
      value *= ( negative.contains( suffix ) ? -1 : 1 );
      if ( isEasting )
      {
        *isEasting = easting.contains( suffix );
      }
    }
  }
  return value;
}

double QgsCoordinateUtils::dmsToDecimal( const QString &string, bool *ok, bool *isEasting )
{
  const QString negative( QStringLiteral( "swSW-" ) );
  const QString easting( QStringLiteral( "eEwW" ) );
  double value = 0.0;
  bool okValue = false;

  if ( ok )
  {
    *ok = false;
  }
  else
  {
    ok = &okValue;
  }

  const QRegularExpression dms( "^\\s*(?:([-+nsew])\\s*)?(\\d{1,3})(?:[^0-9.]+([0-5]?\\d))?[^0-9.]+([0-5]?\\d(?:\\.\\d+)?)[^0-9.,]*?([-+nsew])?\\s*$", QRegularExpression::CaseInsensitiveOption );
  const QRegularExpressionMatch match = dms.match( string.trimmed() );
  if ( match.hasMatch() )
  {
    const QString dms1 = match.captured( 2 );
    const QString dms2 = match.captured( 3 );
    const QString dms3 = match.captured( 4 );

    double v = dms3.toDouble( ok );
    if ( *ok == false )
      return value;
    // Allow for Degrees/minutes format as well as DMS
    if ( !dms2.isEmpty() )
    {
      v = dms2.toInt( ok ) + v / 60.0;
      if ( *ok == false )
        return value;
    }
    v = dms1.toInt( ok ) + v / 60.0;
    if ( *ok == false )
      return value;

    const QString sign1 = match.captured( 1 );
    const QString sign2 = match.captured( 5 );

    if ( sign1.isEmpty() )
    {
      value = !sign2.isEmpty() && negative.contains( sign2 ) ? -v : v;
      if ( isEasting )
      {
        *isEasting = easting.contains( sign2 );
      }
    }
    else if ( sign2.isEmpty() )
    {
      value = !sign1.isEmpty() && negative.contains( sign1 ) ? -v : v;
      if ( isEasting )
      {
        *isEasting = easting.contains( sign2 );
      }
    }
    else
    {
      *ok = false;
    }
  }
  return value;
}

///@endcond
