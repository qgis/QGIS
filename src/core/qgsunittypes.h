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
 * \since QGIS 2.14
 */

class CORE_EXPORT QgsUnitTypes
{
    Q_GADGET

  public:
    //! Systems of unit measurement
    enum SystemOfMeasurement
    {
      UnknownSystem = 0, //!< Unknown system of measurement
      MetricSystem, //!< International System of Units (SI)
      ImperialSystem, //!< British Imperial
      USCSSystem //!< United States customary system
    };
    Q_ENUM( SystemOfMeasurement )

    /**
     * Unit types.
     * \since QGIS 3.10
     */
    enum UnitType
    {
      TypeDistance = 0, //!< Distance unit
      TypeArea, //!< Area unit
      TypeVolume, //!< Volume unit
      TypeUnknown, //!< Unknown unit type
      TypeTemporal, //!< Temporal unit
    };

    //! Units of distance
    enum DistanceUnit
    {
      DistanceMeters = 0, //!< Meters
      DistanceKilometers, //!< Kilometers
      DistanceFeet, //!< Imperial feet
      DistanceNauticalMiles, //!< Nautical miles
      DistanceYards, //!< Imperial yards
      DistanceMiles, //!< Terrestrial miles
      DistanceDegrees, //!< Degrees, for planar geographic CRS distance measurements
      DistanceCentimeters, //!< Centimeters
      DistanceMillimeters, //!< Millimeters
      DistanceUnknownUnit, //!< Unknown distance unit
    };
    Q_ENUM( DistanceUnit )

    /**
     * Types of distance units
     */
    enum DistanceUnitType
    {
      Standard = 0, //!< Unit is a standard measurement unit
      Geographic,   //!< Unit is a geographic (e.g., degree based) unit
      UnknownType,  //!< Unknown unit type
    };

    //! Units of area
    enum AreaUnit
    {
      AreaSquareMeters = 0, //!< Square meters
      AreaSquareKilometers, //!< Square kilometers
      AreaSquareFeet, //!< Square feet
      AreaSquareYards, //!< Square yards
      AreaSquareMiles, //!< Square miles
      AreaHectares, //!< Hectares
      AreaAcres, //!< Acres
      AreaSquareNauticalMiles, //!< Square nautical miles
      AreaSquareDegrees, //!< Square degrees, for planar geographic CRS area measurements
      AreaSquareCentimeters, //!< Square centimeters
      AreaSquareMillimeters, //!< Square millimeters
      AreaUnknownUnit, //!< Unknown areal unit
    };
    Q_ENUM( AreaUnit )

    /**
     * Units of volume.
     * \since QGIS 3.10
     */
    enum VolumeUnit
    {
      VolumeCubicMeters = 0, //!< Cubic meters
      VolumeCubicFeet, //!< Cubic feet
      VolumeCubicYards, //!< Cubic yards
      VolumeBarrel, //!< Barrels
      VolumeCubicDecimeter, //!< Cubic decimeters
      VolumeLiters, //!< Litres
      VolumeGallonUS, //!< US Gallons
      VolumeCubicInch, //!< Cubic inches
      VolumeCubicCentimeter, //!< Cubic Centimeters
      VolumeCubicDegrees, //!< Cubic degrees, for planar geographic CRS volume measurements
      VolumeUnknownUnit, //!< Unknown volume unit
    };
    Q_ENUM( VolumeUnit )

    //! Units of angles
    enum AngleUnit
    {
      AngleDegrees = 0, //!< Degrees
      AngleRadians, //!< Square kilometers
      AngleGon, //!< Gon/gradian
      AngleMinutesOfArc, //!< Minutes of arc
      AngleSecondsOfArc, //!< Seconds of arc
      AngleTurn, //!< Turn/revolutions
      AngleMilliradiansSI, //!< Angular milliradians (SI definition, 1/1000 of radian)
      AngleMilNATO, //!< Angular mil (NATO definition, 6400 mil = 2PI radians)
      AngleUnknownUnit, //!< Unknown angle unit
    };
    Q_ENUM( AngleUnit )

    /**
     * Temporal units.
     * \since QGIS 3.14
     */
    enum TemporalUnit
    {
      TemporalMilliseconds, //!< Milliseconds
      TemporalSeconds, //!< Seconds
      TemporalMinutes, //!< Minutes
      TemporalHours, //!< Hours
      TemporalDays, //!< Days
      TemporalWeeks, //!< Weeks
      TemporalMonths,  //!< Months
      TemporalYears, //!< Years
      TemporalDecades, //!< Decades
      TemporalCenturies, //!< Centuries
      TemporalIrregularStep, //!< Special "irregular step" time unit, used for temporal data which uses irregular, non-real-world unit steps (since QGIS 3.20)
      TemporalUnknownUnit //!< Unknown time unit
    };
    Q_ENUM( TemporalUnit )

    //! Rendering size units
    enum RenderUnit
    {
      RenderMillimeters = 0, //!< Millimeters
      RenderMapUnits, //!< Map units
      RenderPixels, //!< Pixels
      RenderPercentage, //!< Percentage of another measurement (e.g., canvas size, feature size)
      RenderPoints, //!< Points (e.g., for font sizes)
      RenderInches, //!< Inches
      RenderUnknownUnit, //!< Mixed or unknown units
      RenderMetersInMapUnits, //!< Meters value as Map units
    };
    Q_ENUM( RenderUnit )

    //! Layout measurement units
    enum LayoutUnit
    {
      LayoutMillimeters = 0, //!< Millimeters
      LayoutCentimeters, //!< Centimeters
      LayoutMeters, //!< Meters
      LayoutInches, //!< Inches
      LayoutFeet, //!< Feet
      LayoutPoints, //!< Typographic points
      LayoutPicas, //!< Typographic picas
      LayoutPixels //!< Pixels
    };
    Q_ENUM( LayoutUnit )

    //! Types of layout units
    enum LayoutUnitType
    {
      LayoutPaperUnits = 0, //!< Unit is a paper based measurement unit
      LayoutScreenUnits //!< Unit is a screen based measurement unit
    };

    /**
     * A combination of distance value and unit.
     *
     * \since QGIS 3.0
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
      QgsUnitTypes::DistanceUnit unit;
    };

    /**
     * A combination of area value and unit.
     *
     * \since QGIS 3.0
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
      QgsUnitTypes::AreaUnit unit;
    };

    //! List of render units
    typedef QList<QgsUnitTypes::RenderUnit> RenderUnitList;

    /**
     * Encodes a unit \a type to a string.
     * \returns encoded string
     * \see decodeUnitType()
     * \since QGIS 3.10
     */
    Q_INVOKABLE static QString encodeUnitType( QgsUnitTypes::UnitType type );

    /**
     * Decodes a unit type from a \a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded unit type
     * \see encodeUnitType()
     * \since QGIS 3.10
     */
    Q_INVOKABLE static QgsUnitTypes::UnitType decodeUnitType( const QString &string, bool *ok SIP_OUT = nullptr );


    // DISTANCE UNITS

    /**
     * Returns the type for a distance unit.
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceUnitType unitType( QgsUnitTypes::DistanceUnit unit );

    /**
     * Encodes a distance unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeDistanceUnit()
     */
    Q_INVOKABLE static QString encodeUnit( QgsUnitTypes::DistanceUnit unit );

    /**
     * Decodes a distance unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceUnit decodeDistanceUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing a distance unit.
     * \param unit unit to convert to string
     * \see stringToDistanceUnit()
     */
    Q_INVOKABLE static QString toString( QgsUnitTypes::DistanceUnit unit );

    /**
     * Returns a translated abbreviation representing a distance unit.
     * \param unit unit to convert to string
     * \see stringToDistanceUnit()
     *
     * \since QGIS 3.0
     */
    Q_INVOKABLE static QString toAbbreviatedString( QgsUnitTypes::DistanceUnit unit );

    /**
     * Converts a translated string to a distance unit.
     * \param string string representing a distance unit
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns the distance unit
     * \see toString()
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceUnit stringToDistanceUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the conversion factor between the specified distance units.
     * \param fromUnit distance unit to convert from
     * \param toUnit distance unit to convert to
     * \returns multiplication factor to convert between units
     */
    Q_INVOKABLE static double fromUnitToUnitFactor( QgsUnitTypes::DistanceUnit fromUnit, QgsUnitTypes::DistanceUnit toUnit );

    // AREAL UNITS

    /**
     * Returns the type for an areal unit.
    */
    static QgsUnitTypes::DistanceUnitType unitType( QgsUnitTypes::AreaUnit unit );

    /**
     * Encodes an areal unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeAreaUnit()
    */
    static QString encodeUnit( QgsUnitTypes::AreaUnit unit );

    /**
     * Decodes an areal unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
    */
    Q_INVOKABLE static QgsUnitTypes::AreaUnit decodeAreaUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing an areal unit.
     * \param unit unit to convert to string
     * \see stringToAreaUnit()
     */
    static QString toString( QgsUnitTypes::AreaUnit unit );

    /**
     * Returns a translated abbreviation representing an areal unit.
     * \param unit unit to convert to string
     * \see stringToAreaUnit()
     *
     * \since QGIS 3.0
     */
    static QString toAbbreviatedString( QgsUnitTypes::AreaUnit unit );

    /**
     * Converts a translated string to an areal unit.
     * \param string string representing an areal unit
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns the area unit
     * \see toString()
     */
    Q_INVOKABLE static QgsUnitTypes::AreaUnit stringToAreaUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the conversion factor between the specified areal units.
     * \param fromUnit area unit to convert from
     * \param toUnit area unit to convert to
     * \returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( QgsUnitTypes::AreaUnit fromUnit, QgsUnitTypes::AreaUnit toUnit );

    /**
     * Converts a distance unit to its corresponding area unit, e.g., meters to square meters
     * \param distanceUnit distance unit to convert
     * \returns matching areal unit
     */
    Q_INVOKABLE static QgsUnitTypes::AreaUnit distanceToAreaUnit( QgsUnitTypes::DistanceUnit distanceUnit );

    /**
     * Converts an area unit to its corresponding distance unit, e.g., square meters to meters
     * \param areaUnit area unit to convert
     * \returns matching distance unit
     * \since QGIS 3.10
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceUnit areaToDistanceUnit( QgsUnitTypes::AreaUnit areaUnit );

    // TEMPORAL UNITS

    /**
     * Encodes a temporal \a unit to a string.
     * \returns encoded string
     * \see decodeTemporalUnit()
     * \since QGIS 3.14
    */
    static QString encodeUnit( QgsUnitTypes::TemporalUnit unit );

    /**
     * Decodes a temporal unit from a \a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     * \since QGIS 3.14
    */
    Q_INVOKABLE static QgsUnitTypes::TemporalUnit decodeTemporalUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing a temporal \a unit.
     * \see stringToTemporalUnit()
     * \since QGIS 3.14
     */
    static QString toString( QgsUnitTypes::TemporalUnit unit );

    /**
     * Returns a translated abbreviation representing a temporal \a unit.
     * \see stringToTemporalUnit()
     *
     * \since QGIS 3.14
     */
    static QString toAbbreviatedString( QgsUnitTypes::TemporalUnit unit );

    /**
     * Converts a translated \a string to a temporal unit.
     * \param string string representing a volume unit
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns the temporal unit
     * \see toString()
     * \since QGIS 3.14
     */
    Q_INVOKABLE static QgsUnitTypes::TemporalUnit stringToTemporalUnit( const QString &string, bool *ok SIP_OUT = nullptr );

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
    static double fromUnitToUnitFactor( QgsUnitTypes::TemporalUnit fromUnit, QgsUnitTypes::TemporalUnit toUnit );

    // VOLUME UNITS

    /**
     * Returns the type for an volume unit.
     * \since QGIS 3.10
    */
    static QgsUnitTypes::DistanceUnitType unitType( QgsUnitTypes::VolumeUnit unit );

    /**
     * Encodes a volume \a unit to a string.
     * \returns encoded string
     * \see decodeVolumeUnit()
     * \since QGIS 3.10
    */
    static QString encodeUnit( QgsUnitTypes::VolumeUnit unit );

    /**
     * Decodes a volume unit from a \a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     * \since QGIS 3.10
    */
    Q_INVOKABLE static QgsUnitTypes::VolumeUnit decodeVolumeUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing a volume \a unit.
     * \see stringToVolumeUnit()
     * \since QGIS 3.10
     */
    static QString toString( QgsUnitTypes::VolumeUnit unit );

    /**
     * Returns a translated abbreviation representing a volume \a unit.
     * \see stringToVolumeUnit()
     *
     * \since QGIS 3.10
     */
    static QString toAbbreviatedString( QgsUnitTypes::VolumeUnit unit );

    /**
     * Converts a translated \a string to a volume unit.
     * \param string string representing a volume unit
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns the volume unit
     * \see toString()
     * \since QGIS 3.10
     */
    Q_INVOKABLE static QgsUnitTypes::VolumeUnit stringToVolumeUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the conversion factor between the specified volume units.
     * \param fromUnit volume unit to convert from
     * \param toUnit volume unit to convert to
     * \returns multiplication factor to convert between units
     * \since QGIS 3.10
     */
    static double fromUnitToUnitFactor( QgsUnitTypes::VolumeUnit fromUnit, QgsUnitTypes::VolumeUnit toUnit );

    /**
     * Converts a distance unit to its corresponding volume unit, e.g., meters to cubic meters
     * \param distanceUnit distance unit to convert
     * \returns matching volume unit
     * \since QGIS 3.10
     */
    Q_INVOKABLE static QgsUnitTypes::VolumeUnit distanceToVolumeUnit( QgsUnitTypes::DistanceUnit distanceUnit );

    /**
     * Converts a volume unit to its corresponding distance unit, e.g., cubic meters to meters
     * \param volumeUnit volume unit to convert
     * \returns matching distance unit
     * \since QGIS 3.10
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceUnit volumeToDistanceUnit( QgsUnitTypes::VolumeUnit volumeUnit );

    // ANGULAR UNITS

    /**
     * Encodes an angular unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeAngleUnit()
    */
    static QString encodeUnit( QgsUnitTypes::AngleUnit unit );

    /**
     * Decodes an angular unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
    */
    Q_INVOKABLE static QgsUnitTypes::AngleUnit decodeAngleUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns a translated string representing an angular unit.
     * \param unit unit to convert to string
     */
    static QString toString( QgsUnitTypes::AngleUnit unit );

    /**
     * Returns the conversion factor between the specified angular units.
     * \param fromUnit angle unit to convert from
     * \param toUnit angle unit to convert to
     * \returns multiplication factor to convert between units
     */
    static double fromUnitToUnitFactor( QgsUnitTypes::AngleUnit fromUnit, QgsUnitTypes::AngleUnit toUnit );

    /**
     * Returns an angle formatted as a friendly string.
     * \param angle angle to format
     * \param decimals number of decimal places to show. A value of -1 indicates that an appropriate number of decimal places should automatically be selected.
     * \param unit unit of angle
     * \returns formatted angle string
     */
    Q_INVOKABLE static QString formatAngle( double angle, int decimals, QgsUnitTypes::AngleUnit unit );

    /**
     * Will convert a \a distance with a given \a unit to a distance value which is nice to display.
     * It will convert between different units (e.g. from meters to kilometers or millimeters)
     * if appropriate, unless forced otherwise with \a keepBaseUnit.
     * The value will also be rounded to \a decimals (be prepared that the returned value is still a double so it will require
     * further formatting when converting to a string).
     *
     * \since QGIS 3.0
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceValue scaledDistance( double distance, QgsUnitTypes::DistanceUnit unit, int decimals, bool keepBaseUnit = false );

    /**
     * Will convert an \a area with a given \a unit to an area value which is nice to display.
     * It will convert between different units (e.g. from square meters to square kilometers)
     * if appropriate, unless forced otherwise with \a keepBaseUnit.
     * The value will also be rounded to \a decimals (be prepared that the returned value is still a double so it will require
     * further formatting when converting to a string).
     *
     * \since QGIS 3.0
     */
    Q_INVOKABLE static QgsUnitTypes::AreaValue scaledArea( double area, QgsUnitTypes::AreaUnit unit, int decimals, bool keepBaseUnit = false );

    /**
     * Returns an distance formatted as a friendly string.
     * \param distance distance to format
     * \param decimals number of decimal places to show
     * \param unit unit of distance
     * \param keepBaseUnit set to FALSE to allow conversion of large distances to more suitable units, e.g., meters to
     * kilometers
     * \returns formatted distance string
     * \see formatArea()
     * \since QGIS 3.0
     */
    Q_INVOKABLE static QString formatDistance( double distance, int decimals, QgsUnitTypes::DistanceUnit unit, bool keepBaseUnit = false );

    /**
     * Returns an area formatted as a friendly string.
     * \param area area to format
     * \param decimals number of decimal places to show
     * \param unit unit of area
     * \param keepBaseUnit set to FALSE to allow conversion of large areas to more suitable units, e.g., square meters to
     * square kilometers
     * \returns formatted area string
     * \see formatDistance()
     * \since QGIS 3.0
     */
    Q_INVOKABLE static QString formatArea( double area, int decimals, QgsUnitTypes::AreaUnit unit, bool keepBaseUnit = false );

    // RENDER UNITS

    /**
     * Encodes a render unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeRenderUnit()
     */
    static QString encodeUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Decodes a render unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     */
    Q_INVOKABLE static QgsUnitTypes::RenderUnit decodeRenderUnit( const QString &string, bool *ok SIP_OUT = nullptr );


    /**
     * Returns a translated string representing a render \a unit.
     * \since QGIS 3.0
     */
    static QString toString( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns a translated abbreviation representing a render unit.
     * \param unit unit to convert to string
     *
     * \since QGIS 3.8
     */
    static QString toAbbreviatedString( QgsUnitTypes::RenderUnit unit );


    // LAYOUT UNITS

    /**
     * Encodes a layout unit to a string.
     * \param unit unit to encode
     * \returns encoded string
     * \see decodeLayoutUnit()
     * \since QGIS 3.0
     */
    static QString encodeUnit( QgsUnitTypes::LayoutUnit unit );

    /**
     * Decodes a layout unit from a string.
     * \param string string to decode
     * \param ok optional boolean, will be set to TRUE if string was converted successfully
     * \returns decoded units
     * \see encodeUnit()
     * \since QGIS 3.0
     */
    Q_INVOKABLE static QgsUnitTypes::LayoutUnit decodeLayoutUnit( const QString &string, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the type for a unit of measurement.
     *
     * \since QGIS 3.0
    */
    Q_INVOKABLE static QgsUnitTypes::LayoutUnitType unitType( QgsUnitTypes::LayoutUnit units );

    /**
     * Returns a translated abbreviation representing a layout \a unit (e.g. "mm").
     *
     * \since QGIS 3.0
     */
    static QString toAbbreviatedString( QgsUnitTypes::LayoutUnit unit );

    /**
     * Returns a translated string representing a layout \a unit.
     *
     * \since QGIS 3.0
     */
    static QString toString( QgsUnitTypes::LayoutUnit unit );

};

#endif // QGSUNITTYPES_H
