/***************************************************************************
  qgssunpositioncalculator.cpp
  -----------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssunpositioncalculator.h"

#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsexception.h"
#include "qgspointxy.h"

#include <QString>

using namespace Qt::StringLiterals;

#define FS_TIME_T int64_t
extern "C"
{
#include <freespa.h>
}

QgsSunPositionResult QgsSunPositionCalculator::calculate(
  const QgsPointXY &point, const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext, const QDateTime &dateTime, double elevationMeters, double pressure, double temperature
)
{
  QgsSunPositionResult result;
  const QgsCoordinateReferenceSystem wgs84( u"EPSG:4326"_s );
  QgsPointXY latLonPoint = point;
  if ( crs != wgs84 )
  {
    // may throw QgsCsException, which we want raised to the caller
    QgsCoordinateTransform ct( crs, wgs84, transformContext );
    latLonPoint = ct.transform( point );
  }

  // freespa uses radians for angles
  const double longitudeRad = latLonPoint.x() * M_PI / 180.0;
  const double latitudeRad = latLonPoint.y() * M_PI / 180.0;

  const QDateTime utcTime = dateTime.toUTC();
  struct tm ut_tm;
  memset( &ut_tm, 0, sizeof( struct tm ) );
  ut_tm.tm_year = utcTime.date().year() - 1900;
  ut_tm.tm_mon = utcTime.date().month() - 1;
  ut_tm.tm_mday = utcTime.date().day();
  ut_tm.tm_hour = utcTime.time().hour();
  ut_tm.tm_min = utcTime.time().minute();
  ut_tm.tm_sec = utcTime.time().second();

  // calculate real solar position
  const sol_pos realPos = SPA( &ut_tm, nullptr, 0.0, longitudeRad, latitudeRad, elevationMeters );
  if ( realPos.E != 0 )
  {
    throw QgsInvalidArgumentException( u"Invalid freespa calculation: "_s + extractSpaErrorMessage( realPos.E ) );
  }

  // calculate apparent solar position (applies atmospheric refraction)
  const sol_pos apparentPos = ApSolposBennet( realPos, nullptr, elevationMeters, pressure, temperature );
  if ( apparentPos.E != 0 )
  {
    throw QgsInvalidArgumentException( u"Invalid freespa calculation: "_s + extractSpaErrorMessage( apparentPos.E ) );
  }

  // convert back to degrees
  result.azimuth = apparentPos.a * 180.0 / M_PI;
  const double zenithDegrees = apparentPos.z * 180.0 / M_PI;
  result.apparentElevation = 90.0 - zenithDegrees;

  // calculate solar events
  const solar_day dayEvents = SolarDay( &ut_tm, nullptr, 0.0, longitudeRad, latitudeRad, elevationMeters, nullptr, pressure, temperature, ApSolposBennet );

  auto extractEvent = []( const solar_day &sd, int index ) -> QDateTime {
    if ( sd.status[index] == _FREESPA_EV_ERR )
    {
      throw QgsInvalidArgumentException( u"Error calculating solar event at index %1"_s.arg( index ) );
    }
    else if ( sd.status[index] == _FREESPA_EV_OK )
    {
      return QDateTime( QDate( sd.ev[index].tm_year + 1900, sd.ev[index].tm_mon + 1, sd.ev[index].tm_mday ), QTime( sd.ev[index].tm_hour, sd.ev[index].tm_min, sd.ev[index].tm_sec ), Qt::UTC );
    }
    // event doesn't occur
    return QDateTime();
  };

  result.solarMidnightBefore = extractEvent( dayEvents, 0 );
  result.solarTransit = extractEvent( dayEvents, 1 );
  result.solarMidnightAfter = extractEvent( dayEvents, 2 );

  result.sunrise = extractEvent( dayEvents, 3 );
  result.sunset = extractEvent( dayEvents, 4 );

  result.civilDawn = extractEvent( dayEvents, 5 );
  result.civilDusk = extractEvent( dayEvents, 6 );

  result.nauticalDawn = extractEvent( dayEvents, 7 );
  result.nauticalDusk = extractEvent( dayEvents, 8 );

  result.astronomicalDawn = extractEvent( dayEvents, 9 );
  result.astronomicalDusk = extractEvent( dayEvents, 10 );

  return result;
}

QString QgsSunPositionCalculator::extractSpaErrorMessage( int errorCode )
{
  QStringList errors;

  if ( errorCode & _FREESPA_DEU_OOR )
    errors << u"ΔUT1 out of range"_s;
  if ( errorCode & _FREESPA_LON_OOR )
    errors << u"Longitude out of range"_s;
  if ( errorCode & _FREESPA_LAT_OOR )
    errors << u"Latitude out of range"_s;
  if ( errorCode & _FREESPA_ELE_OOR )
    errors << u"Elevation out of range"_s;
  if ( errorCode & _FREESPA_PRE_OOR )
    errors << u"Pressure out of range"_s;
  if ( errorCode & _FREESPA_TEM_OOR )
    errors << u"Temperature out of range"_s;
  if ( errorCode & _FREESPA_DIP_OOR )
    errors << u"Geometric dip out of range"_s;
  if ( errorCode & _FREESPA_GMTIMEF )
    errors << u"Time conversion error"_s;

  if ( errors.isEmpty() )
  {
    return u"Unknown calculation error code: %1"_s.arg( errorCode );
  }

  return errors.join( ", "_L1 );
}
