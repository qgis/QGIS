/***************************************************************************
    qgsvectorfieldwindbarbsettings.h
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDWINDBARBSETTINGS_H
#define QGSVECTORFIELDWINDBARBSETTINGS_H

#include "qgis.h"
#include "qgis_core.h"

#include <QDomElement>

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for vector datasets displayed with wind barbs.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVectorFieldWindBarbSettings
{
  public:
    //! Wind speed units. Wind barbs use knots so we use this enum for preset conversion values
    enum class WindSpeedUnit
    {
      MetersPerSecond = 0, //!< Meters per second
      KilometersPerHour,   //!< Kilometers per hour
      Knots,               //!< Knots (Nautical miles per hour)
      MilesPerHour,        //!< Miles per hour
      FeetPerSecond,       //!< Feet per second
      OtherUnit            //!< Other unit
    };

    /**
     * Returns the multiplier for the magnitude to convert it to knots, according to the units set with setMagnitudeUnits()
     * A custom multiplier can be set with setMagnitudeMultiplier() for the case when units are set to OtherUnit
     */
    double magnitudeMultiplier() const;

    /**
     * Sets a multiplier for the magnitude to convert it to knots
     */
    void setMagnitudeMultiplier( double magnitudeMultiplier );

    /**
     * Returns the shaft length (in millimeters)
     */
    double shaftLength() const;

    /**
     * Sets the shaft length  (in millimeters)
     */
    void setShaftLength( double shaftLength );

    /**
     * Returns the units for the shaft length.
     *
     * \see setShaftLengthUnits()
     */
    Qgis::RenderUnit shaftLengthUnits() const;

    /**
     * Sets the units for the shaft length.
     *
     * \see shaftLengthUnits()
     */
    void setShaftLengthUnits( Qgis::RenderUnit shaftLengthUnit );

    /**
     * Returns the units that the data are in
     */
    WindSpeedUnit magnitudeUnits() const;

    /**
     * Sets the units that the data are in
     */
    void setMagnitudeUnits( WindSpeedUnit units );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    double mShaftLength = 10;
    Qgis::RenderUnit mShaftLengthUnits = Qgis::RenderUnit::Millimeters;
    WindSpeedUnit mMagnitudeUnits = WindSpeedUnit::MetersPerSecond;
    double mMagnitudeMultiplier = 1;
};

#endif // QGSVECTORFIELDWINDBARBSETTINGS_H
