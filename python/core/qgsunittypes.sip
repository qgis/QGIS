/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsunittypes.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsUnitTypes
{
%Docstring
 Helper functions for various unit types.

.. versionadded:: 2.14
%End

%TypeHeaderCode
#include "qgsunittypes.h"
%End
  public:
    enum DistanceUnit
    {
      DistanceMeters,
      DistanceKilometers,
      DistanceFeet,
      DistanceNauticalMiles,
      DistanceYards,
      DistanceMiles,
      DistanceDegrees,
      DistanceCentimeters,
      DistanceMillimeters,
      DistanceUnknownUnit,
    };

    enum DistanceUnitType
    {
      Standard,
      Geographic,
      UnknownType,
    };

    enum AreaUnit
    {
      AreaSquareMeters,
      AreaSquareKilometers,
      AreaSquareFeet,
      AreaSquareYards,
      AreaSquareMiles,
      AreaHectares,
      AreaAcres,
      AreaSquareNauticalMiles,
      AreaSquareDegrees,
      AreaSquareCentimeters,
      AreaSquareMillimeters,
      AreaUnknownUnit,
    };

    enum AngleUnit
    {
      AngleDegrees,
      AngleRadians,
      AngleGon,
      AngleMinutesOfArc,
      AngleSecondsOfArc,
      AngleTurn,
      AngleUnknownUnit,
    };

    enum RenderUnit
    {
      RenderMillimeters,
      RenderMapUnits,
      RenderPixels,
      RenderPercentage,
      RenderPoints,
      RenderInches,
      RenderUnknownUnit,
      RenderMetersInMapUnits,
    };

    enum LayoutUnit
    {
      LayoutMillimeters,
      LayoutCentimeters,
      LayoutMeters,
      LayoutInches,
      LayoutFeet,
      LayoutPoints,
      LayoutPicas,
      LayoutPixels
    };

    enum LayoutUnitType
    {
      LayoutPaperUnits,
      LayoutScreenUnits
    };

    struct DistanceValue
    {

      double value;

      QgsUnitTypes::DistanceUnit unit;
    };

    struct AreaValue
    {

      double value;

      QgsUnitTypes::AreaUnit unit;
    };

    typedef QList<QgsUnitTypes::RenderUnit> RenderUnitList;


    static DistanceUnitType unitType( QgsUnitTypes::DistanceUnit unit );
%Docstring
Returns the type for a distance unit.
%End

    static QString encodeUnit( QgsUnitTypes::DistanceUnit unit );
%Docstring
Encodes a distance unit to a string.

:param unit: unit to encode

:return: encoded string

.. seealso:: :py:func:`decodeDistanceUnit`
%End

    static QgsUnitTypes::DistanceUnit decodeDistanceUnit( const QString &string, bool *ok /Out/ = 0 );
%Docstring
Decodes a distance unit from a string.

:param string: string to decode
:param ok: optional boolean, will be set to true if string was converted successfully

:return: decoded units

.. seealso:: :py:func:`encodeUnit`
%End

    static QString toString( QgsUnitTypes::DistanceUnit unit );
%Docstring
Returns a translated string representing a distance unit.

:param unit: unit to convert to string

.. seealso:: :py:func:`stringToDistanceUnit`
%End

    static QString toAbbreviatedString( QgsUnitTypes::DistanceUnit unit );
%Docstring
Returns a translated abbreviation representing a distance unit.

:param unit: unit to convert to string

.. seealso:: :py:func:`stringToDistanceUnit`

.. versionadded:: 3.0
%End

    static QgsUnitTypes::DistanceUnit stringToDistanceUnit( const QString &string, bool *ok /Out/ = 0 );
%Docstring
Converts a translated string to a distance unit.

:param string: string representing a distance unit
:param ok: optional boolean, will be set to true if string was converted successfully

.. seealso:: :py:func:`toString`
%End

    static double fromUnitToUnitFactor( QgsUnitTypes::DistanceUnit fromUnit, QgsUnitTypes::DistanceUnit toUnit );
%Docstring
Returns the conversion factor between the specified distance units.

:param fromUnit: distance unit to convert from
:param toUnit: distance unit to convert to

:return: multiplication factor to convert between units
%End


    static DistanceUnitType unitType( AreaUnit unit );
%Docstring
Returns the type for an areal unit.
%End

    static QString encodeUnit( AreaUnit unit );
%Docstring
Encodes an areal unit to a string.

:param unit: unit to encode

:return: encoded string

.. seealso:: :py:func:`decodeAreaUnit`
%End

    static AreaUnit decodeAreaUnit( const QString &string, bool *ok /Out/ = 0 );
%Docstring
Decodes an areal unit from a string.

:param string: string to decode
:param ok: optional boolean, will be set to true if string was converted successfully

:return: decoded units

.. seealso:: :py:func:`encodeUnit`
%End

    static QString toString( AreaUnit unit );
%Docstring
Returns a translated string representing an areal unit.

:param unit: unit to convert to string

.. seealso:: :py:func:`stringToAreaUnit`
%End

    static QString toAbbreviatedString( AreaUnit unit );
%Docstring
Returns a translated abbreviation representing an areal unit.

:param unit: unit to convert to string

.. seealso:: :py:func:`stringToAreaUnit`

.. versionadded:: 3.0
%End

    static AreaUnit stringToAreaUnit( const QString &string, bool *ok /Out/ = 0 );
%Docstring
Converts a translated string to an areal unit.

:param string: string representing an areal unit
:param ok: optional boolean, will be set to true if string was converted successfully

.. seealso:: :py:func:`toString`
%End

    static double fromUnitToUnitFactor( AreaUnit fromUnit, AreaUnit toUnit );
%Docstring
Returns the conversion factor between the specified areal units.

:param fromUnit: area unit to convert from
:param toUnit: area unit to convert to

:return: multiplication factor to convert between units
%End

    static AreaUnit distanceToAreaUnit( QgsUnitTypes::DistanceUnit distanceUnit );
%Docstring
Converts a distance unit to its corresponding area unit, e.g., meters to square meters

:param distanceUnit: distance unit to convert

:return: matching areal unit
%End


    static QString encodeUnit( AngleUnit unit );
%Docstring
Encodes an angular unit to a string.

:param unit: unit to encode

:return: encoded string

.. seealso:: :py:func:`decodeAngleUnit`
%End

    static AngleUnit decodeAngleUnit( const QString &string, bool *ok /Out/ = 0 );
%Docstring
Decodes an angular unit from a string.

:param string: string to decode
:param ok: optional boolean, will be set to true if string was converted successfully

:return: decoded units

.. seealso:: :py:func:`encodeUnit`
%End

    static QString toString( AngleUnit unit );
%Docstring
Returns a translated string representing an angular unit.

:param unit: unit to convert to string
%End

    static double fromUnitToUnitFactor( AngleUnit fromUnit, AngleUnit toUnit );
%Docstring
Returns the conversion factor between the specified angular units.

:param fromUnit: angle unit to convert from
:param toUnit: angle unit to convert to

:return: multiplication factor to convert between units
%End

    static QString formatAngle( double angle, int decimals, AngleUnit unit );
%Docstring
Returns an angle formatted as a friendly string.

:param angle: angle to format
:param decimals: number of decimal places to show
:param unit: unit of angle

:return: formatted angle string
%End


    static QgsUnitTypes::DistanceValue scaledDistance( double distance, QgsUnitTypes::DistanceUnit unit, int decimals, bool keepBaseUnit = false );
%Docstring
Will convert a ``distance`` with a given ``unit`` to a distance value which is nice to display.
It will convert between different units (e.g. from meters to kilometers or millimeters)
if appropriate, unless forced otherwise with ``keepBaseUnit``.
The value will also be rounded to ``decimals`` (be prepared that the returned value is still a double so it will require
further formatting when converting to a string).

.. versionadded:: 3.0
%End

    static QgsUnitTypes::AreaValue scaledArea( double area, QgsUnitTypes::AreaUnit unit, int decimals, bool keepBaseUnit = false );
%Docstring
Will convert an ``area`` with a given ``unit`` to an area value which is nice to display.
It will convert between different units (e.g. from square meters to square kilometers)
if appropriate, unless forced otherwise with ``keepBaseUnit``.
The value will also be rounded to ``decimals`` (be prepared that the returned value is still a double so it will require
further formatting when converting to a string).

.. versionadded:: 3.0
%End

    static QString formatDistance( double distance, int decimals, QgsUnitTypes::DistanceUnit unit, bool keepBaseUnit = false );
%Docstring
Returns an distance formatted as a friendly string.

:param distance: distance to format
:param decimals: number of decimal places to show
:param unit: unit of distance
:param keepBaseUnit: set to false to allow conversion of large distances to more suitable units, e.g., meters to
kilometers

:return: formatted distance string

.. versionadded:: 3.0

.. seealso:: :py:func:`formatArea`
%End

    static QString formatArea( double area, int decimals, QgsUnitTypes::AreaUnit unit, bool keepBaseUnit = false );
%Docstring
Returns an area formatted as a friendly string.

:param area: area to format
:param decimals: number of decimal places to show
:param unit: unit of area
:param keepBaseUnit: set to false to allow conversion of large areas to more suitable units, e.g., square meters to
square kilometers

:return: formatted area string

.. versionadded:: 3.0

.. seealso:: :py:func:`formatDistance`
%End


    static QString encodeUnit( RenderUnit unit );
%Docstring
Encodes a render unit to a string.

:param unit: unit to encode

:return: encoded string

.. seealso:: :py:func:`decodeRenderUnit`
%End

    static RenderUnit decodeRenderUnit( const QString &string, bool *ok /Out/ = 0 );
%Docstring
Decodes a render unit from a string.

:param string: string to decode
:param ok: optional boolean, will be set to true if string was converted successfully

:return: decoded units

.. seealso:: :py:func:`encodeUnit`
%End

    static QString toString( RenderUnit unit );
%Docstring
Returns a translated string representing a render ``unit``.

.. versionadded:: 3.0
%End



    static QString encodeUnit( LayoutUnit unit );
%Docstring
Encodes a layout unit to a string.

:param unit: unit to encode

:return: encoded string

.. seealso:: :py:func:`decodeLayoutUnit`

.. versionadded:: 3.0
%End

    static LayoutUnit decodeLayoutUnit( const QString &string, bool *ok /Out/ = 0 );
%Docstring
Decodes a layout unit from a string.

:param string: string to decode
:param ok: optional boolean, will be set to true if string was converted successfully

:return: decoded units

.. seealso:: :py:func:`encodeUnit`

.. versionadded:: 3.0
%End

    static LayoutUnitType unitType( const LayoutUnit units );
%Docstring
Returns the type for a unit of measurement.

.. versionadded:: 3.0
%End

    static QString toAbbreviatedString( LayoutUnit unit );
%Docstring
Returns a translated abbreviation representing a layout ``unit`` (e.g. "mm").

.. versionadded:: 3.0
%End

    static QString toString( LayoutUnit unit );
%Docstring
Returns a translated string representing a layout ``unit``.

.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsunittypes.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
