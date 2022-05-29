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
#include "qgscoordinatereferencesystemutils.h"
#include "qgscoordinatetransform.h"
#include "qgsproject.h"
#include "qgis.h"
#include "qgsexception.h"
#include "qgscoordinateformatter.h"
#include "qgsrectangle.h"
#include "qgsprojectdisplaysettings.h"
#include "qgscoordinatenumericformat.h"

#include <QLocale>
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
    // only MU or D is used now, but older projects may have DM/DMS
    const bool formatGeographic = format == QLatin1String( "D" ) || format == QLatin1String( "DM" ) || format == QLatin1String( "DMS" );

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
      switch ( project->displaySettings()->geographicCoordinateFormat()->angleFormat() )
      {
        case QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutesSeconds:
        case QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutes:
          dp = 2;
          break;
        case QgsGeographicCoordinateNumericFormat::AngleFormat::DecimalDegrees:
          dp = 4;
          break;
      }
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
  const Qgis::CoordinateOrder axisOrder = qgsEnumKeyToValue( project->readEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/CoordinateOrder" ) ), Qgis::CoordinateOrder::Default );

  // only MU or D is used now, but older projects may have DM/DMS
  const bool formatGeographic = format == QLatin1String( "D" ) || format == QLatin1String( "DM" ) || format == QLatin1String( "DMS" );

  QgsPointXY geo = point;
  if ( formatGeographic )
  {
    // degrees
    QgsCoordinateReferenceSystem geographicCrs = destCrs;
    if ( destCrs.isValid() && !destCrs.isGeographic() )
    {
      // default to EPSG:4326 if the project CRS isn't already geographic
      geographicCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
      // need to transform to geographic coordinates
      const QgsCoordinateTransform ct( destCrs, geographicCrs, project );
      try
      {
        geo = ct.transform( point );
      }
      catch ( QgsCsException & )
      {
        return QString();
      }
    }

    const Qgis::CoordinateOrder order = axisOrder == Qgis::CoordinateOrder::Default ? QgsCoordinateReferenceSystemUtils::defaultCoordinateOrderForCrs( geographicCrs ) : axisOrder;

    std::unique_ptr< QgsGeographicCoordinateNumericFormat > format( project->displaySettings()->geographicCoordinateFormat()->clone() );
    format->setNumberDecimalPlaces( precision );

    QgsNumericFormatContext context;
    context.setInterpretation( QgsNumericFormatContext::Interpretation::Longitude );
    const QString formattedX = format->formatDouble( geo.x(), context );
    context.setInterpretation( QgsNumericFormatContext::Interpretation::Latitude );
    const QString formattedY = format->formatDouble( geo.y(), context );

    switch ( order )
    {
      case Qgis::CoordinateOrder::Default:
      case Qgis::CoordinateOrder::XY:
        return QStringLiteral( "%1%2%3" ).arg( formattedX, QgsCoordinateFormatter::separator(), formattedY );

      case Qgis::CoordinateOrder::YX:
        return QStringLiteral( "%1%2%3" ).arg( formattedY, QgsCoordinateFormatter::separator(), formattedX );
    }
    BUILTIN_UNREACHABLE
  }
  else
  {
    // coordinates in map units
    const Qgis::CoordinateOrder order = axisOrder == Qgis::CoordinateOrder::Default ? QgsCoordinateReferenceSystemUtils::defaultCoordinateOrderForCrs( destCrs ) : axisOrder;
    return QgsCoordinateFormatter::asPair( point.x(), point.y(), precision, order );
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

  const QLocale locale;
  QRegularExpression degreeWithSuffix( QStringLiteral( "^\\s*([-]?\\d{1,3}(?:[\\.\\%1]\\d+)?)\\s*([NSEWnsew])\\s*$" )
                                       .arg( locale.decimalPoint() ) );
  QRegularExpressionMatch match = degreeWithSuffix.match( string );
  if ( match.hasMatch() )
  {
    const QString suffix = match.captured( 2 );
    value = std::abs( match.captured( 1 ).toDouble( ok ) );
    if ( *ok == false )
    {
      value = std::abs( locale.toDouble( match.captured( 1 ), ok ) );
    }
    if ( *ok )
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

  const QLocale locale;
  const QRegularExpression dms( QStringLiteral( "^\\s*(?:([-+nsew])\\s*)?(\\d{1,3})(?:[^0-9.]+([0-5]?\\d))?[^0-9.]+([0-5]?\\d(?:[\\.\\%1]\\d+)?)[^0-9.,]*?([-+nsew])?\\s*$" )
                                .arg( locale.decimalPoint() ), QRegularExpression::CaseInsensitiveOption );
  const QRegularExpressionMatch match = dms.match( string.trimmed() );
  if ( match.hasMatch() )
  {
    const QString dms1 = match.captured( 2 );
    const QString dms2 = match.captured( 3 );
    const QString dms3 = match.captured( 4 );

    double v = dms3.toDouble( ok );
    if ( *ok == false )
    {
      v = locale.toDouble( dms3, ok );
      if ( *ok == false )
        return value;
    }
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
