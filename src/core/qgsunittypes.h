/***************************************************************************
                         qgsunittypes.h
                         --------------
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

#ifndef QGSUNITTYPES_H
#define QGSUNITTYPES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QObject>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \ingroup core
 * \class QgsUnitTypes
 * \brief Helper functions for various unit types.
 */

class CORE_EXPORT QgsUnitTypes
{
    Q_GADGET

  public:

    /**
     * A combination of distance value and unit.
     *
     */
    struct DistanceValue
    {

      /**
       * The value part of the distance. For 3.7 meters, this will be 3.7.
       */
      double value;

      /**
       * The value part of the distance. For 3.7 meters, this will be QgsUnitTypes::DistanceMeters.
       */
      Qgis::DistanceUnit unit;
    };

    /**
     * A combination of area value and unit.
     *
     */
    struct AreaValue
    {

      /**
       * The value part of the distance. For 3.7 square meters, this will be 3.7.
       */
      double value;

      /**
       * The value part of the distance. For 3.7 square meters, this will be QgsUnitTypes::AreaSquareMeters.
       */
      Qgis::AreaUnit unit;
    };

    //! List of render units
    typedef QList<Qgis::RenderUnit> RenderUnitList;

    /**
     * Encodes a unit \a type to a string.
     * \returns encoded string
     * \see decodeUnitType()
     * \since QGIS 3.10
     */
    Q_INVOKABLE static QString encodeUnitType( Qgis::UnitType type );

    /**
     * Decodes a unit type from a \a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded unit type
     * \see encodeUnitType()
     * \since QGIS 3.10
     */
    Q_INVOKABLE static Qgis::UnitType decodeUnitType( const QString &string, bool *ok SIP_OUT = nullptr );


    // DISTANCE UNITS

    /**
     * Returns the type for a distance unit.
     */
    Q_INVOKABLE static Qgis::DistanceUnitType unitType( Qgis::DistanceUnit unit );

    /**
     * Encodes a distance unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeDistanceUnit()
     */
    Q_INVOKABLE static QString encodeUnit( Qgis::DistanceUnit unit );

    /**
     * Decodes a distance unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     */
    Q_INVOKABLE static Qgis::DistanceUnit decodeDistanceUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing a distance unit.
     * \param unit unit to convert to string
     * \see stringToDistanceUnit()
     */
    Q_INVOKABLE static QString toString( Qgis::DistanceUnit unit );

    /**
     * Returns a translated abbreviation representing a distance unit.
     * \param unit unit to convert to string
     * \see stringToDistanceUnit()
     *
     */
    Q_INVOKABLE static QString toAbbreviatedString( Qgis::DistanceUnit unit );

    /**
     * Converts a translated string to a distance unit.
     * \param string string representing a distance unit
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns the distance unit
     * \see toString()
     */
    Q_INVOKABLE static Qgis::DistanceUnit stringToDistanceUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the conversion factor between the specified distance units.
     * \param fromUnit distance unit to convert from
     * \param toUnit distance unit to convert to
     * \returns multiplication factor to convert between units
     */
    Q_INVOKABLE static double fromUnitToUnitFactor( Qgis::DistanceUnit fromUnit, Qgis::DistanceUnit toUnit );

    // AREAL UNITS

    /**
     * Returns the type for an areal unit.
    */
    static Qgis::DistanceUnitType unitType( Qgis::AreaUnit unit );

    /**
     * Encodes an areal unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeAreaUnit()
    */
    static QString encodeUnit( Qgis::AreaUnit unit );

    /**
     * Decodes an areal unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
    */
    Q_INVOKABLE static Qgis::AreaUnit decodeAreaUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing an areal unit.
     * \param unit unit to convert to string
     * \see stringToAreaUnit()
     */
    static QString toString( Qgis::AreaUnit unit );

    /**
     * Returns a translated abbreviation representing an areal unit.
     * \param unit unit to convert to string
     * \see stringToAreaUnit()
     *
     */
    static QString toAbbreviatedString( Qgis::AreaUnit unit );

    /**
     * Converts a translated string to an areal unit.
     * \param string string representing an areal unit
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns the area unit
     * \see toString()
     */
    Q_INVOKABLE static Qgis::AreaUnit stringToAreaUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the conversion factor between the specified areal units.
     * \param fromUnit area unit to convert from
     * \param toUnit area unit to convert to
     * \returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( Qgis::AreaUnit fromUnit, Qgis::AreaUnit toUnit );

    /**
     * Converts a distance unit to its corresponding area unit, e.g., meters to square meters
     * \param distanceUnit distance unit to convert
     * \returns matching areal unit
     */
    Q_INVOKABLE static Qgis::AreaUnit distanceToAreaUnit( Qgis::DistanceUnit distanceUnit );

    /**
     * Converts an area unit to its corresponding distance unit, e.g., square meters to meters
     * \param areaUnit area unit to convert
     * \returns matching distance unit
     * \since QGIS 3.10
     */
    Q_INVOKABLE static Qgis::DistanceUnit areaToDistanceUnit( Qgis::AreaUnit areaUnit );

    // TEMPORAL UNITS

    /**
     * Encodes a temporal \a unit to a string.
     * \returns encoded string
     * \see decodeTemporalUnit()
     * \since QGIS 3.14
    */
    static QString encodeUnit( Qgis::TemporalUnit unit );

    /**
     * Decodes a temporal unit from a \a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     * \since QGIS 3.14
    */
    Q_INVOKABLE static Qgis::TemporalUnit decodeTemporalUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing a temporal \a unit.
     * \see stringToTemporalUnit()
     * \since QGIS 3.14
     */
    static QString toString( Qgis::TemporalUnit unit );

    /**
     * Returns a translated abbreviation representing a temporal \a unit.
     * \see stringToTemporalUnit()
     *
     * \since QGIS 3.14
     */
    static QString toAbbreviatedString( Qgis::TemporalUnit unit );

    /**
     * Converts a translated \a string to a temporal unit.
     * \param string string representing a volume unit
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns the temporal unit
     * \see toString()
     * \since QGIS 3.14
     */
    Q_INVOKABLE static Qgis::TemporalUnit stringToTemporalUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the conversion factor between the specified temporal units.
     *
     * \note Conversion to or from month units assumes a 30 day month length.
     * \note Conversion to or from year based units assumes a 365.25 day year length.
     *
     * \param fromUnit temporal unit to convert from
     * \param toUnit temporal unit to convert to
     * \returns multiplication factor to convert between units
     * \since QGIS 3.14
     */
    static double fromUnitToUnitFactor( Qgis::TemporalUnit fromUnit, Qgis::TemporalUnit toUnit );

    // VOLUME UNITS

    /**
     * Returns the type for an volume unit.
     * \since QGIS 3.10
    */
    static Qgis::DistanceUnitType unitType( Qgis::VolumeUnit unit );

    /**
     * Encodes a volume \a unit to a string.
     * \returns encoded string
     * \see decodeVolumeUnit()
     * \since QGIS 3.10
    */
    static QString encodeUnit( Qgis::VolumeUnit unit );

    /**
     * Decodes a volume unit from a \a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     * \since QGIS 3.10
    */
    Q_INVOKABLE static Qgis::VolumeUnit decodeVolumeUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing a volume \a unit.
     * \see stringToVolumeUnit()
     * \since QGIS 3.10
     */
    static QString toString( Qgis::VolumeUnit unit );

    /**
     * Returns a translated abbreviation representing a volume \a unit.
     * \see stringToVolumeUnit()
     *
     * \since QGIS 3.10
     */
    static QString toAbbreviatedString( Qgis::VolumeUnit unit );

    /**
     * Converts a translated \a string to a volume unit.
     * \param string string representing a volume unit
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns the volume unit
     * \see toString()
     * \since QGIS 3.10
     */
    Q_INVOKABLE static Qgis::VolumeUnit stringToVolumeUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the conversion factor between the specified volume units.
     * \param fromUnit volume unit to convert from
     * \param toUnit volume unit to convert to
     * \returns multiplication factor to convert between units
     * \since QGIS 3.10
     */
    static double fromUnitToUnitFactor( Qgis::VolumeUnit fromUnit, Qgis::VolumeUnit toUnit );

    /**
     * Converts a distance unit to its corresponding volume unit, e.g., meters to cubic meters
     * \param distanceUnit distance unit to convert
     * \returns matching volume unit
     * \since QGIS 3.10
     */
    Q_INVOKABLE static Qgis::VolumeUnit distanceToVolumeUnit( Qgis::DistanceUnit distanceUnit );

    /**
     * Converts a volume unit to its corresponding distance unit, e.g., cubic meters to meters
     * \param volumeUnit volume unit to convert
     * \returns matching distance unit
     * \since QGIS 3.10
     */
    Q_INVOKABLE static Qgis::DistanceUnit volumeToDistanceUnit( Qgis::VolumeUnit volumeUnit );

    // ANGULAR UNITS

    /**
     * Encodes an angular unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeAngleUnit()
    */
    static QString encodeUnit( Qgis::AngleUnit unit );

    /**
     * Decodes an angular unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
    */
    Q_INVOKABLE static Qgis::AngleUnit decodeAngleUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing an angular unit.
     * \param unit unit to convert to string
     */
    static QString toString( Qgis::AngleUnit unit );

    /**
     * Returns the conversion factor between the specified angular units.
     * \param fromUnit angle unit to convert from
     * \param toUnit angle unit to convert to
     * \returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( Qgis::AngleUnit fromUnit, Qgis::AngleUnit toUnit );

    /**
     * Returns an angle formatted as a friendly string.
     * \param angle angle to format
     * \param decimals number of decimal places to show. A value of -1 indicates that an appropriate number of decimal places should automatically be selected.
     * \param unit unit of angle
     * \returns formatted angle string
     */
    Q_INVOKABLE static QString formatAngle( double angle, int decimals, Qgis::AngleUnit unit );

    /**
     * Will convert a \a distance with a given \a unit to a distance value which is nice to display.
     * It will convert between different units (e.g. from meters to kilometers or millimeters)
     * if appropriate, unless forced otherwise with \a keepBaseUnit.
     * The value will also be rounded to \a decimals (be prepared that the returned value is still a double so it will require
     * further formatting when converting to a string).
     *
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceValue scaledDistance( double distance, Qgis::DistanceUnit unit, int decimals, bool keepBaseUnit = false );

    /**
     * Will convert an \a area with a given \a unit to an area value which is nice to display.
     * It will convert between different units (e.g. from square meters to square kilometers)
     * if appropriate, unless forced otherwise with \a keepBaseUnit.
     * The value will also be rounded to \a decimals (be prepared that the returned value is still a double so it will require
     * further formatting when converting to a string).
     *
     */
    Q_INVOKABLE static QgsUnitTypes::AreaValue scaledArea( double area, Qgis::AreaUnit unit, int decimals, bool keepBaseUnit = false );

    /**
     * Returns an distance formatted as a friendly string.
     * \param distance distance to format
     * \param decimals number of decimal places to show
     * \param unit unit of distance
     * \param keepBaseUnit set to FALSE to allow conversion of large distances to more suitable units, e.g., meters to
     * kilometers
     * \returns formatted distance string
     * \see formatArea()
     */
    Q_INVOKABLE static QString formatDistance( double distance, int decimals, Qgis::DistanceUnit unit, bool keepBaseUnit = false );

    /**
     * Returns an area formatted as a friendly string.
     * \param area area to format
     * \param decimals number of decimal places to show
     * \param unit unit of area
     * \param keepBaseUnit set to FALSE to allow conversion of large areas to more suitable units, e.g., square meters to
     * square kilometers
     * \returns formatted area string
     * \see formatDistance()
     */
    Q_INVOKABLE static QString formatArea( double area, int decimals, Qgis::AreaUnit unit, bool keepBaseUnit = false );

    // RENDER UNITS

    /**
     * Encodes a render unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeRenderUnit()
     */
    static QString encodeUnit( Qgis::RenderUnit unit );

    /**
     * Decodes a render unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     */
    Q_INVOKABLE static Qgis::RenderUnit decodeRenderUnit( const QString &string, bool *ok SIP_OUT = nullptr );


    /**
     * Returns a translated string representing a render \a unit.
     */
    static QString toString( Qgis::RenderUnit unit );

    /**
     * Returns a translated abbreviation representing a render unit.
     * \param unit unit to convert to string
     *
     * \since QGIS 3.8
     */
    static QString toAbbreviatedString( Qgis::RenderUnit unit );


    // LAYOUT UNITS

    /**
     * Encodes a layout unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeLayoutUnit()
     */
    static QString encodeUnit( Qgis::LayoutUnit unit );

    /**
     * Decodes a layout unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     */
    Q_INVOKABLE static Qgis::LayoutUnit decodeLayoutUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the type for a unit of measurement.
     *
    */
    Q_INVOKABLE static Qgis::LayoutUnitType unitType( Qgis::LayoutUnit units );

    /**
     * Returns a translated abbreviation representing a layout \a unit (e.g. "mm").
     *
     */
    static QString toAbbreviatedString( Qgis::LayoutUnit unit );

    /**
     * Returns a translated string representing a layout \a unit.
     *
     */
    static QString toString( Qgis::LayoutUnit unit );

};

#endif // QGSUNITTYPES_H
