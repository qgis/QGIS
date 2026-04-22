/***************************************************************************
  qgssunpositioncalculator.h
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

#ifndef QGSSUNPOSITIONCALCULATOR_H
#define QGSSUNPOSITIONCALCULATOR_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsPointXY;
class QgsCoordinateReferenceSystem;
class QgsCoordinateTransformContext;

/**
 * \ingroup core
 * \brief Contains the results of a solar position calculation.
 * \since QGIS 4.2
 */
struct CORE_EXPORT QgsSunPositionResult
{
    //! Azimuth angle in degrees clockwise from North.
    double azimuth = 0.0;

    //! Apparent topocentric elevation angle in degrees (corrected for atmospheric refraction).
    double apparentElevation = 0.0;

    //! The datetime of the solar midnight preceding the calculation time (in UTC).
    QDateTime solarMidnightBefore;

    //! The datetime of solar transit (solar noon) when the sun reaches its highest elevation (in UTC).
    QDateTime solarTransit;

    //! The datetime of the solar midnight following the calculation time (in UTC).
    QDateTime solarMidnightAfter;

    //! The datetime of sunrise, defined as the moment the upper edge of the sun's disk becomes visible above the horizon (in UTC).
    QDateTime sunrise;

    //! The datetime of sunset, defined as the moment the upper edge of the sun's disk disappears below the horizon (in UTC).
    QDateTime sunset;

    //! The datetime of civil dawn, when the geometric center of the sun is 6 degrees below the horizon in the morning (in UTC).
    QDateTime civilDawn;

    //! The datetime of civil dusk, when the geometric center of the sun is 6 degrees below the horizon in the evening (in UTC).
    QDateTime civilDusk;

    //! The datetime of nautical dawn, when the geometric center of the sun is 12 degrees below the horizon in the morning (in UTC).
    QDateTime nauticalDawn;

    //! The datetime of nautical dusk, when the geometric center of the sun is 12 degrees below the horizon in the evening (in UTC).
    QDateTime nauticalDusk;

    //! The datetime of astronomical dawn, when the geometric center of the sun is 18 degrees below the horizon in the morning (in UTC).
    QDateTime astronomicalDawn;

    //! The datetime of astronomical dusk, when the geometric center of the sun is 18 degrees below the horizon in the evening (in UTC).
    QDateTime astronomicalDusk;
};

/**
 * \ingroup core
 * \brief Calculates the sun's position and related sun events for a given datetime.
 *
 * This class is a high-level wrapper around the freespa algorithm for calculating solar position and events.
 *
 * \warning This class only supports sun position calculation from the Earth. Other celestial bodies are not supported.
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsSunPositionCalculator
{
  public:
    /**
     * Calculates the solar position and events for a given point and time.
     *
     * \param point the target point in the specified \a crs.
     * \param crs Coordinate Reference System associated with the \a point.
     * \param context coordinate transform context.
     * \param dateTime the date and time for the calculation.
     * \param elevationMeters optional elevation of the observer in meters above sea level.
     * \param pressure atmospheric pressure used for refraction correction (in millibars, hPa). The default is one standard atmosphere, or 1013.25 hPa.
     * \param temperature the local temperature used for refraction correction, in degrees Celsius.
     *
     * \returns A QgsSunPositionResult containing the calculated solar angles and event times.
     *
     * \warning This method only supports sun position calculation from the Earth. Other celestial bodies are not supported.
     *
     * \throws QgsCsException if the point could not be transformed to WGS84
     * \throws QgsInvalidArgumentException when the specified arguments are invalid
     */
    static QgsSunPositionResult calculate(
      const QgsPointXY &point,
      const QgsCoordinateReferenceSystem &crs,
      const QgsCoordinateTransformContext &context,
      const QDateTime &dateTime,
      double elevationMeters = 0.0,
      double pressure = 1013.25,
      double temperature = 15.0
    ) SIP_THROW( QgsCsException, QgsInvalidArgumentException );

  private:
    static QString extractSpaErrorMessage( int errorCode );
};

#endif
