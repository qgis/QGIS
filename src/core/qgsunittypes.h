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
#include "qgis.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * \class QgsUnitTypes
 * \brief Helper functions for various unit types.
 * \note Added in version 2.14
 */

class CORE_EXPORT QgsUnitTypes
{
    Q_GADGET

  public:

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
      DistanceUnknownUnit, //!< Unknown distance unit
    };

    /** Types of distance units
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
      AreaUnknownUnit, //!< Unknown areal unit
    };

    //! Units of angles
    enum AngleUnit
    {
      AngleDegrees = 0, //!< Degrees
      AngleRadians, //!< Square kilometers
      AngleGon, //!< Gon/gradian
      AngleMinutesOfArc, //!< Minutes of arc
      AngleSecondsOfArc, //!< Seconds of arc
      AngleTurn, //!< Turn/revolutions
      AngleUnknownUnit, //!< Unknown angle unit
    };

    //! Rendering size units
    enum RenderUnit
    {
      RenderMillimeters = 0, //!< Millimeters
      RenderMapUnits, //!< Map units
      RenderPixels, //!< Pixels
      RenderPercentage, //!< Percentage of another measurement (e.g., canvas size, feature size)
      RenderPoints, //! points (e.g., for font sizes)
      RenderUnknownUnit, //!< Mixed or unknown units
    };

    //! List of render units
    typedef QList<RenderUnit> RenderUnitList;

    // DISTANCE UNITS

    /** Returns the type for a distance unit.
     */
    Q_INVOKABLE static DistanceUnitType unitType( DistanceUnit unit );

    /** Encodes a distance unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeDistanceUnit()
     */
    Q_INVOKABLE static QString encodeUnit( QgsUnitTypes::DistanceUnit unit );

    /** Decodes a distance unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceUnit decodeDistanceUnit( const QString& string, bool *ok = 0 );

    /** Returns a translated string representing a distance unit.
     * @param unit unit to convert to string
     * @see stringToDistanceUnit()
     */
    Q_INVOKABLE static QString toString( QgsUnitTypes::DistanceUnit unit );

    /** Converts a translated string to a distance unit.
     * @param string string representing a distance unit
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @see toString()
     */
    Q_INVOKABLE static QgsUnitTypes::DistanceUnit stringToDistanceUnit( const QString& string, bool *ok = 0 );

    /** Returns the conversion factor between the specified distance units.
     * @param fromUnit distance unit to convert from
     * @param toUnit distance unit to convert to
     * @returns multiplication factor to convert between units
     */
    Q_INVOKABLE static double fromUnitToUnitFactor( QgsUnitTypes::DistanceUnit fromUnit, QgsUnitTypes::DistanceUnit toUnit );

    // AREAL UNITS

    /** Returns the type for an areal unit.
    */
    Q_INVOKABLE static DistanceUnitType unitType( AreaUnit unit );

    /** Encodes an areal unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeAreaUnit()
    */
    Q_INVOKABLE static QString encodeUnit( AreaUnit unit );

    /** Decodes an areal unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
    */
    Q_INVOKABLE static AreaUnit decodeAreaUnit( const QString& string, bool *ok = 0 );

    /** Returns a translated string representing an areal unit.
     * @param unit unit to convert to string
     * @see stringToAreaUnit()
     */
    Q_INVOKABLE static QString toString( AreaUnit unit );

    /** Converts a translated string to an areal unit.
     * @param string string representing an areal unit
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @see toString()
     */
    Q_INVOKABLE static AreaUnit stringToAreaUnit( const QString& string, bool *ok = 0 );

    /** Returns the conversion factor between the specified areal units.
     * @param fromUnit area unit to convert from
     * @param toUnit area unit to convert to
     * @returns multiplication factor to convert between units
     */
    Q_INVOKABLE static double fromUnitToUnitFactor( AreaUnit fromUnit, AreaUnit toUnit );

    /** Converts a distance unit to its corresponding area unit, e.g., meters to square meters
     * @param distanceUnit distance unit to convert
     * @return matching areal unit
     */
    Q_INVOKABLE static AreaUnit distanceToAreaUnit( QgsUnitTypes::DistanceUnit distanceUnit );

    // ANGULAR UNITS

    /** Encodes an angular unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeAngleUnit()
    */
    Q_INVOKABLE static QString encodeUnit( AngleUnit unit );

    /** Decodes an angular unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
    */
    Q_INVOKABLE static AngleUnit decodeAngleUnit( const QString& string, bool *ok = 0 );

    /** Returns a translated string representing an angular unit.
     * @param unit unit to convert to string
     */
    Q_INVOKABLE static QString toString( AngleUnit unit );

    /** Returns the conversion factor between the specified angular units.
     * @param fromUnit angle unit to convert from
     * @param toUnit angle unit to convert to
     * @returns multiplication factor to convert between units
     */
    Q_INVOKABLE static double fromUnitToUnitFactor( AngleUnit fromUnit, AngleUnit toUnit );

    /** Returns an angle formatted as a friendly string.
     * @param angle angle to format
     * @param decimals number of decimal places to show
     * @param unit unit of angle
     * @returns formatted angle string
     */
    Q_INVOKABLE static QString formatAngle( double angle, int decimals, AngleUnit unit );

    // RENDER UNITS

    /** Encodes a render unit to a string.
     * @param unit unit to encode
     * @returns encoded string
     * @see decodeRenderUnit()
     */
    Q_INVOKABLE static QString encodeUnit( RenderUnit unit );

    /** Decodes a render unit from a string.
     * @param string string to decode
     * @param ok optional boolean, will be set to true if string was converted successfully
     * @returns decoded units
     * @see encodeUnit()
     */
    Q_INVOKABLE static RenderUnit decodeRenderUnit( const QString& string, bool *ok = 0 );

};

#endif // QGSUNITTYPES_H
