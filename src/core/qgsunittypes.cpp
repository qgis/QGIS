/***************************************************************************
                         qgsunittypes.cpp
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

#include "qgsunittypes.h"
#include "qgis.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

QgsUnitTypes::DistanceUnitType QgsUnitTypes::unitType( DistanceUnit unit )
{
  switch ( unit )
  {
    case DistanceMeters:
    case DistanceFeet:
    case DistanceNauticalMiles:
    case DistanceYards:
    case DistanceMiles:
    case DistanceKilometers:
    case DistanceCentimeters:
    case DistanceMillimeters:
      return Standard;

    case DistanceDegrees:
      return Geographic;

    case DistanceUnknownUnit:
      return UnknownType;
  }
  return UnknownType;
}

QgsUnitTypes::DistanceUnitType QgsUnitTypes::unitType( QgsUnitTypes::AreaUnit unit )
{
  switch ( unit )
  {
    case AreaSquareMeters:
    case AreaSquareKilometers:
    case AreaSquareFeet:
    case AreaSquareYards:
    case AreaSquareMiles:
    case AreaHectares:
    case AreaAcres:
    case AreaSquareNauticalMiles:
    case AreaSquareCentimeters:
    case AreaSquareMillimeters:
      return Standard;

    case AreaSquareDegrees:
      return Geographic;

    case AreaUnknownUnit:
      return UnknownType;
  }

  return UnknownType;
}

QString QgsUnitTypes::encodeUnit( DistanceUnit unit )
{
  switch ( unit )
  {
    case DistanceMeters:
      return QStringLiteral( "meters" );

    case DistanceKilometers:
      return QStringLiteral( "km" );

    case DistanceFeet:
      return QStringLiteral( "feet" );

    case DistanceYards:
      return QStringLiteral( "yd" );

    case DistanceMiles:
      return QStringLiteral( "mi" );

    case DistanceDegrees:
      return QStringLiteral( "degrees" );

    case DistanceUnknownUnit:
      return QStringLiteral( "<unknown>" );

    case DistanceNauticalMiles:
      return QStringLiteral( "nautical miles" );

    case DistanceCentimeters:
      return QStringLiteral( "cm" );

    case DistanceMillimeters:
      return QStringLiteral( "mm" );
  }
  return QString();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

QgsUnitTypes::DistanceUnit QgsUnitTypes::decodeDistanceUnit( const QString &string, bool *ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( DistanceMeters ) )
    return DistanceMeters;
  if ( normalized == encodeUnit( DistanceFeet ) )
    return DistanceFeet;
  if ( normalized == encodeUnit( DistanceDegrees ) )
    return DistanceDegrees;
  if ( normalized == encodeUnit( DistanceNauticalMiles ) )
    return DistanceNauticalMiles;
  if ( normalized == encodeUnit( DistanceKilometers ) )
    return DistanceKilometers;
  if ( normalized == encodeUnit( DistanceYards ) )
    return DistanceYards;
  if ( normalized == encodeUnit( DistanceMiles ) )
    return DistanceMiles;
  if ( normalized == encodeUnit( DistanceCentimeters ) )
    return DistanceCentimeters;
  if ( normalized == encodeUnit( DistanceMillimeters ) )
    return DistanceMillimeters;
  if ( normalized == encodeUnit( DistanceUnknownUnit ) )
    return DistanceUnknownUnit;

  if ( ok )
    *ok = false;

  return DistanceUnknownUnit;
}

QString QgsUnitTypes::toString( DistanceUnit unit )
{
  switch ( unit )
  {
    case DistanceMeters:
      return QObject::tr( "meters", "distance" );

    case DistanceKilometers:
      return QObject::tr( "kilometers", "distance" );

    case DistanceFeet:
      return QObject::tr( "feet", "distance" );

    case DistanceYards:
      return QObject::tr( "yards", "distance" );

    case DistanceMiles:
      return QObject::tr( "miles", "distance" );

    case DistanceDegrees:
      return QObject::tr( "degrees", "distance" );

    case DistanceCentimeters:
      return QObject::tr( "centimeters", "distance" );

    case DistanceMillimeters:
      return QObject::tr( "millimeters", "distance" );

    case DistanceUnknownUnit:
      return QObject::tr( "<unknown>", "distance" );

    case DistanceNauticalMiles:
      return QObject::tr( "nautical miles", "distance" );
  }
  return QString();
}

QString QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::DistanceUnit unit )
{
  switch ( unit )
  {
    case DistanceMeters:
      return QObject::tr( "m", "distance" );

    case DistanceKilometers:
      return QObject::tr( "km", "distance" );

    case DistanceFeet:
      return QObject::tr( "ft", "distance" );

    case DistanceYards:
      return QObject::tr( "yd", "distance" );

    case DistanceMiles:
      return QObject::tr( "mi", "distance" );

    case DistanceDegrees:
      return QObject::tr( "deg", "distance" );

    case DistanceCentimeters:
      return QObject::tr( "cm", "distance" );

    case DistanceMillimeters:
      return QObject::tr( "mm", "distance" );

    case DistanceUnknownUnit:
      return QString();

    case DistanceNauticalMiles:
      return QObject::tr( "NM", "distance" );
  }
  return QString();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

QgsUnitTypes::DistanceUnit QgsUnitTypes::stringToDistanceUnit( const QString &string, bool *ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == toString( DistanceMeters ) )
    return DistanceMeters;
  if ( normalized == toString( DistanceKilometers ) )
    return DistanceKilometers;
  if ( normalized == toString( DistanceFeet ) )
    return DistanceFeet;
  if ( normalized == toString( DistanceYards ) )
    return DistanceYards;
  if ( normalized == toString( DistanceMiles ) )
    return DistanceMiles;
  if ( normalized == toString( DistanceDegrees ) )
    return DistanceDegrees;
  if ( normalized == toString( DistanceCentimeters ) )
    return DistanceCentimeters;
  if ( normalized == toString( DistanceMillimeters ) )
    return DistanceMillimeters;
  if ( normalized == toString( DistanceNauticalMiles ) )
    return DistanceNauticalMiles;
  if ( normalized == toString( DistanceUnknownUnit ) )
    return DistanceUnknownUnit;

  if ( ok )
    *ok = false;

  return DistanceUnknownUnit;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

double QgsUnitTypes::fromUnitToUnitFactor( DistanceUnit fromUnit, DistanceUnit toUnit )
{
#define DEGREE_TO_METER 111319.49079327358
#define FEET_TO_METER 0.3048
#define NMILE_TO_METER 1852.0
#define KILOMETERS_TO_METER 1000.0
#define CENTIMETERS_TO_METER 0.01
#define MILLIMETERS_TO_METER 0.001
#define YARDS_TO_METER 0.9144
#define YARDS_TO_FEET 3.0
#define MILES_TO_METER 1609.344

  // Calculate the conversion factor between the specified units
  switch ( fromUnit )
  {
    case DistanceMeters:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return 1.0;
        case DistanceKilometers:
          return 1.0 / KILOMETERS_TO_METER;
        case DistanceMillimeters:
          return 1.0 / MILLIMETERS_TO_METER;
        case DistanceCentimeters:
          return 1.0 / CENTIMETERS_TO_METER;
        case DistanceFeet:
          return 1.0 / FEET_TO_METER;
        case DistanceYards:
          return 1.0 / YARDS_TO_METER;
        case DistanceMiles:
          return 1.0 / MILES_TO_METER;
        case DistanceDegrees:
          return 1.0 / DEGREE_TO_METER;
        case DistanceNauticalMiles:
          return 1.0 / NMILE_TO_METER;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceKilometers:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return KILOMETERS_TO_METER;
        case DistanceKilometers:
          return 1.0;
        case DistanceCentimeters:
          return KILOMETERS_TO_METER / CENTIMETERS_TO_METER;
        case DistanceMillimeters:
          return KILOMETERS_TO_METER / MILLIMETERS_TO_METER;
        case DistanceFeet:
          return KILOMETERS_TO_METER / FEET_TO_METER;
        case DistanceYards:
          return KILOMETERS_TO_METER / YARDS_TO_METER;
        case DistanceMiles:
          return KILOMETERS_TO_METER / MILES_TO_METER;
        case DistanceDegrees:
          return KILOMETERS_TO_METER / DEGREE_TO_METER;
        case DistanceNauticalMiles:
          return KILOMETERS_TO_METER / NMILE_TO_METER;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceFeet:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return FEET_TO_METER;
        case DistanceKilometers:
          return FEET_TO_METER / KILOMETERS_TO_METER;
        case DistanceCentimeters:
          return FEET_TO_METER / CENTIMETERS_TO_METER;
        case DistanceMillimeters:
          return FEET_TO_METER / MILLIMETERS_TO_METER;
        case DistanceFeet:
          return 1.0;
        case DistanceYards:
          return 1.0 / YARDS_TO_FEET;
        case DistanceMiles:
          return FEET_TO_METER / MILES_TO_METER;
        case DistanceDegrees:
          return FEET_TO_METER / DEGREE_TO_METER;
        case DistanceNauticalMiles:
          return FEET_TO_METER / NMILE_TO_METER;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceYards:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return YARDS_TO_METER;
        case DistanceKilometers:
          return YARDS_TO_METER / KILOMETERS_TO_METER;
        case DistanceCentimeters:
          return YARDS_TO_METER / CENTIMETERS_TO_METER;
        case DistanceMillimeters:
          return YARDS_TO_METER / MILLIMETERS_TO_METER;
        case DistanceFeet:
          return YARDS_TO_FEET;
        case DistanceYards:
          return 1.0;
        case DistanceMiles:
          return YARDS_TO_METER / MILES_TO_METER;
        case DistanceDegrees:
          return YARDS_TO_METER / DEGREE_TO_METER;
        case DistanceNauticalMiles:
          return YARDS_TO_METER / NMILE_TO_METER;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceMiles:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return MILES_TO_METER;
        case DistanceKilometers:
          return MILES_TO_METER / KILOMETERS_TO_METER;
        case DistanceCentimeters:
          return MILES_TO_METER / CENTIMETERS_TO_METER;
        case DistanceMillimeters:
          return MILES_TO_METER / MILLIMETERS_TO_METER;
        case DistanceFeet:
          return MILES_TO_METER / FEET_TO_METER;
        case DistanceYards:
          return MILES_TO_METER / YARDS_TO_METER;
        case DistanceMiles:
          return 1.0;
        case DistanceDegrees:
          return MILES_TO_METER / DEGREE_TO_METER;
        case DistanceNauticalMiles:
          return MILES_TO_METER / NMILE_TO_METER;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceDegrees:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return DEGREE_TO_METER;
        case DistanceKilometers:
          return DEGREE_TO_METER / KILOMETERS_TO_METER;
        case DistanceCentimeters:
          return DEGREE_TO_METER / CENTIMETERS_TO_METER;
        case DistanceMillimeters:
          return DEGREE_TO_METER / MILLIMETERS_TO_METER;
        case DistanceFeet:
          return DEGREE_TO_METER / FEET_TO_METER;
        case DistanceYards:
          return DEGREE_TO_METER / YARDS_TO_METER;
        case DistanceMiles:
          return DEGREE_TO_METER / MILES_TO_METER;
        case DistanceDegrees:
          return 1.0;
        case DistanceNauticalMiles:
          return DEGREE_TO_METER / NMILE_TO_METER;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceNauticalMiles:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return NMILE_TO_METER;
        case DistanceKilometers:
          return NMILE_TO_METER / KILOMETERS_TO_METER;
        case DistanceCentimeters:
          return NMILE_TO_METER / CENTIMETERS_TO_METER;
        case DistanceMillimeters:
          return NMILE_TO_METER / MILLIMETERS_TO_METER;
        case DistanceFeet:
          return NMILE_TO_METER / FEET_TO_METER;
        case DistanceYards:
          return NMILE_TO_METER / YARDS_TO_METER;
        case DistanceMiles:
          return NMILE_TO_METER / MILES_TO_METER;
        case DistanceDegrees:
          return NMILE_TO_METER / DEGREE_TO_METER;
        case DistanceNauticalMiles:
          return 1.0;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceCentimeters:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return CENTIMETERS_TO_METER;
        case DistanceKilometers:
          return CENTIMETERS_TO_METER / KILOMETERS_TO_METER;;
        case DistanceCentimeters:
          return 1.0;
        case DistanceMillimeters:
          return CENTIMETERS_TO_METER / MILLIMETERS_TO_METER;
        case DistanceFeet:
          return CENTIMETERS_TO_METER / FEET_TO_METER;
        case DistanceYards:
          return CENTIMETERS_TO_METER / YARDS_TO_METER;
        case DistanceMiles:
          return CENTIMETERS_TO_METER / MILES_TO_METER;
        case DistanceDegrees:
          return CENTIMETERS_TO_METER / DEGREE_TO_METER;
        case DistanceNauticalMiles:
          return CENTIMETERS_TO_METER / NMILE_TO_METER;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceMillimeters:
    {
      switch ( toUnit )
      {
        case DistanceMeters:
          return MILLIMETERS_TO_METER;
        case DistanceKilometers:
          return MILLIMETERS_TO_METER / KILOMETERS_TO_METER;
        case DistanceCentimeters:
          return MILLIMETERS_TO_METER / CENTIMETERS_TO_METER;
        case DistanceMillimeters:
          return 1.0;
        case DistanceFeet:
          return MILLIMETERS_TO_METER / FEET_TO_METER;
        case DistanceYards:
          return MILLIMETERS_TO_METER / YARDS_TO_METER;
        case DistanceMiles:
          return MILLIMETERS_TO_METER / MILES_TO_METER;
        case DistanceDegrees:
          return MILLIMETERS_TO_METER / DEGREE_TO_METER;
        case DistanceNauticalMiles:
          return MILLIMETERS_TO_METER / NMILE_TO_METER;
        case DistanceUnknownUnit:
          break;
      }

      break;
    }
    case DistanceUnknownUnit:
      break;
  }
  return 1.0;
}

QString QgsUnitTypes::encodeUnit( QgsUnitTypes::AreaUnit unit )
{
  switch ( unit )
  {
    case AreaSquareMeters:
      return QStringLiteral( "m2" );
    case AreaSquareKilometers:
      return QStringLiteral( "km2" );
    case AreaSquareFeet:
      return QStringLiteral( "ft2" );
    case AreaSquareYards:
      return QStringLiteral( "y2" );
    case AreaSquareMiles:
      return QStringLiteral( "mi2" );
    case AreaHectares:
      return QStringLiteral( "ha" );
    case AreaAcres:
      return QStringLiteral( "ac" );
    case AreaSquareNauticalMiles:
      return QStringLiteral( "nm2" );
    case AreaSquareDegrees:
      return QStringLiteral( "deg2" );
    case AreaSquareCentimeters:
      return QStringLiteral( "cm2" );
    case AreaSquareMillimeters:
      return QStringLiteral( "mm2" );
    case AreaUnknownUnit:
      return QStringLiteral( "<unknown>" );
  }
  return QString();
}

QgsUnitTypes::AreaUnit QgsUnitTypes::decodeAreaUnit( const QString &string, bool *ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( AreaSquareMeters ) )
    return AreaSquareMeters;
  if ( normalized == encodeUnit( AreaSquareKilometers ) )
    return AreaSquareKilometers;
  if ( normalized == encodeUnit( AreaSquareFeet ) )
    return AreaSquareFeet;
  if ( normalized == encodeUnit( AreaSquareYards ) )
    return AreaSquareYards;
  if ( normalized == encodeUnit( AreaSquareMiles ) )
    return AreaSquareMiles;
  if ( normalized == encodeUnit( AreaHectares ) )
    return AreaHectares;
  if ( normalized == encodeUnit( AreaAcres ) )
    return AreaAcres;
  if ( normalized == encodeUnit( AreaSquareNauticalMiles ) )
    return AreaSquareNauticalMiles;
  if ( normalized == encodeUnit( AreaSquareDegrees ) )
    return AreaSquareDegrees;
  if ( normalized == encodeUnit( AreaSquareCentimeters ) )
    return AreaSquareCentimeters;
  if ( normalized == encodeUnit( AreaSquareMillimeters ) )
    return AreaSquareMillimeters;
  if ( normalized == encodeUnit( AreaUnknownUnit ) )
    return AreaUnknownUnit;

  if ( ok )
    *ok = false;

  return AreaUnknownUnit;
}

QString QgsUnitTypes::toString( QgsUnitTypes::AreaUnit unit )
{
  switch ( unit )
  {
    case AreaSquareMeters:
      return QObject::tr( "square meters", "area" );
    case AreaSquareKilometers:
      return QObject::tr( "square kilometers", "area" );
    case AreaSquareFeet:
      return QObject::tr( "square feet", "area" );
    case AreaSquareYards:
      return QObject::tr( "square yards", "area" );
    case AreaSquareMiles:
      return QObject::tr( "square miles", "area" );
    case AreaHectares:
      return QObject::tr( "hectares", "area" );
    case AreaAcres:
      return QObject::tr( "acres", "area" );
    case AreaSquareNauticalMiles:
      return QObject::tr( "square nautical miles", "area" );
    case AreaSquareDegrees:
      return QObject::tr( "square degrees", "area" );
    case AreaSquareMillimeters:
      return QObject::tr( "square millimeters", "area" );
    case AreaSquareCentimeters:
      return QObject::tr( "square centimeters", "area" );
    case AreaUnknownUnit:
      return QObject::tr( "<unknown>", "area" );
  }
  return QString();
}

QString QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::AreaUnit unit )
{
  switch ( unit )
  {
    case AreaSquareMeters:
      return QObject::trUtf8( "m²", "area" );
    case AreaSquareKilometers:
      return QObject::trUtf8( "km²", "area" );
    case AreaSquareFeet:
      return QObject::trUtf8( "ft²", "area" );
    case AreaSquareYards:
      return QObject::trUtf8( "yd²", "area" );
    case AreaSquareMiles:
      return QObject::trUtf8( "mi²", "area" );
    case AreaHectares:
      return QObject::trUtf8( "ha", "area" );
    case AreaAcres:
      return QObject::trUtf8( "ac²", "area" );
    case AreaSquareNauticalMiles:
      return QObject::trUtf8( "NM²", "area" );
    case AreaSquareDegrees:
      return QObject::trUtf8( "deg²", "area" );
    case AreaSquareCentimeters:
      return QObject::trUtf8( "cm²", "area" );
    case AreaSquareMillimeters:
      return QObject::trUtf8( "mm²", "area" );
    case AreaUnknownUnit:
      return QString();
  }
  return QString();
}

QgsUnitTypes::AreaUnit QgsUnitTypes::stringToAreaUnit( const QString &string, bool *ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == toString( AreaSquareMeters ) )
    return AreaSquareMeters;
  if ( normalized == toString( AreaSquareKilometers ) )
    return AreaSquareKilometers;
  if ( normalized == toString( AreaSquareFeet ) )
    return AreaSquareFeet;
  if ( normalized == toString( AreaSquareYards ) )
    return AreaSquareYards;
  if ( normalized == toString( AreaSquareMiles ) )
    return AreaSquareMiles;
  if ( normalized == toString( AreaHectares ) )
    return AreaHectares;
  if ( normalized == toString( AreaAcres ) )
    return AreaAcres;
  if ( normalized == toString( AreaSquareNauticalMiles ) )
    return AreaSquareNauticalMiles;
  if ( normalized == toString( AreaSquareDegrees ) )
    return AreaSquareDegrees;
  if ( normalized == toString( AreaSquareMillimeters ) )
    return AreaSquareMillimeters;
  if ( normalized == toString( AreaSquareCentimeters ) )
    return AreaSquareCentimeters;
  if ( normalized == toString( AreaUnknownUnit ) )
    return AreaUnknownUnit;
  if ( ok )
    *ok = false;

  return AreaUnknownUnit;
}

double QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaUnit fromUnit, QgsUnitTypes::AreaUnit toUnit )
{
#define KM2_TO_M2 1000000.0
#define CM2_TO_M2 0.0001
#define MM2_TO_M2 0.000001
#define FT2_TO_M2 0.09290304
#define YD2_TO_M2 0.83612736
#define MI2_TO_M2 2589988.110336
#define HA_TO_M2 10000.0
#define AC_TO_FT2 43560.0
#define DEG2_TO_M2 12392029030.5
#define NM2_TO_M2 3429904.0

  // Calculate the conversion factor between the specified units
  switch ( fromUnit )
  {
    case AreaSquareMeters:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return 1.0;
        case AreaSquareKilometers:
          return 1.0 / KM2_TO_M2;
        case AreaSquareFeet:
          return 1.0 / FT2_TO_M2;
        case AreaSquareYards:
          return 1.0 / YD2_TO_M2;
        case AreaSquareMiles:
          return 1.0 / MI2_TO_M2;
        case AreaHectares:
          return 1.0 / HA_TO_M2;
        case AreaAcres:
          return 1.0 / AC_TO_FT2 / FT2_TO_M2;
        case AreaSquareNauticalMiles:
          return 1.0 / NM2_TO_M2;
        case AreaSquareDegrees:
          return 1.0 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return 1.0 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return 1.0 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }
    case AreaSquareKilometers:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return KM2_TO_M2;
        case AreaSquareKilometers:
          return 1.0;
        case AreaSquareFeet:
          return KM2_TO_M2 / FT2_TO_M2;
        case AreaSquareYards:
          return KM2_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return KM2_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return KM2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return KM2_TO_M2 / AC_TO_FT2 / FT2_TO_M2;
        case AreaSquareNauticalMiles:
          return KM2_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return KM2_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return KM2_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return KM2_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }
    case AreaSquareFeet:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return FT2_TO_M2;
        case AreaSquareKilometers:
          return FT2_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return 1.0;
        case AreaSquareYards:
          return FT2_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return FT2_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return FT2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return 1.0 / AC_TO_FT2;
        case AreaSquareNauticalMiles:
          return FT2_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return FT2_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return FT2_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return FT2_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }

    case AreaSquareYards:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return YD2_TO_M2;
        case AreaSquareKilometers:
          return YD2_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return YD2_TO_M2 / FT2_TO_M2;
        case AreaSquareYards:
          return 1.0;
        case AreaSquareMiles:
          return YD2_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return YD2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return YD2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case AreaSquareNauticalMiles:
          return YD2_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return YD2_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return YD2_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return YD2_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }
      break;
    }

    case AreaSquareMiles:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return MI2_TO_M2;
        case AreaSquareKilometers:
          return MI2_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return MI2_TO_M2 / FT2_TO_M2;
        case AreaSquareYards:
          return MI2_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return 1.0;
        case AreaHectares:
          return MI2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return MI2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case AreaSquareNauticalMiles:
          return MI2_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return MI2_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return MI2_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return MI2_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }

    case AreaHectares:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return HA_TO_M2;
        case AreaSquareKilometers:
          return HA_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return HA_TO_M2 / FT2_TO_M2;
        case AreaSquareYards:
          return HA_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return HA_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return 1.0;
        case AreaAcres:
          return HA_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case AreaSquareNauticalMiles:
          return HA_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return HA_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return HA_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return HA_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }

    case AreaAcres:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return AC_TO_FT2 * FT2_TO_M2;
        case AreaSquareKilometers:
          return AC_TO_FT2 * FT2_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return AC_TO_FT2;
        case AreaSquareYards:
          return AC_TO_FT2 * FT2_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return AC_TO_FT2 * FT2_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return AC_TO_FT2 * FT2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return 1.0;
        case AreaSquareNauticalMiles:
          return AC_TO_FT2 * FT2_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return AC_TO_FT2 * FT2_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return AC_TO_FT2 * FT2_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return AC_TO_FT2 * FT2_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }

    case AreaSquareNauticalMiles:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return NM2_TO_M2;
        case AreaSquareKilometers:
          return NM2_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return NM2_TO_M2 / FT2_TO_M2;
        case AreaSquareYards:
          return NM2_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return NM2_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return NM2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return NM2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case AreaSquareNauticalMiles:
          return 1.0;
        case AreaSquareDegrees:
          return NM2_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return NM2_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return NM2_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }

    case AreaSquareDegrees:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return DEG2_TO_M2;
        case AreaSquareKilometers:
          return DEG2_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return DEG2_TO_M2 / FT2_TO_M2;
        case AreaSquareYards:
          return DEG2_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return DEG2_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return DEG2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return DEG2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case AreaSquareNauticalMiles:
          return DEG2_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return 1.0;
        case AreaSquareCentimeters:
          return DEG2_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return DEG2_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }

    case AreaSquareMillimeters:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return MM2_TO_M2;
        case AreaSquareKilometers:
          return MM2_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return MM2_TO_M2 / FT2_TO_M2;
        case AreaSquareYards:
          return MM2_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return MM2_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return MM2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return MM2_TO_M2 / AC_TO_FT2 / FT2_TO_M2;
        case AreaSquareNauticalMiles:
          return MM2_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return MM2_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return MM2_TO_M2 / CM2_TO_M2;
        case AreaSquareMillimeters:
          return 1.0;
        case AreaUnknownUnit:
          break;
      }

      break;
    }
    case AreaSquareCentimeters:
    {
      switch ( toUnit )
      {
        case AreaSquareMeters:
          return CM2_TO_M2;
        case AreaSquareKilometers:
          return CM2_TO_M2 / KM2_TO_M2;
        case AreaSquareFeet:
          return CM2_TO_M2 / FT2_TO_M2;
        case AreaSquareYards:
          return CM2_TO_M2 / YD2_TO_M2;
        case AreaSquareMiles:
          return CM2_TO_M2 / MI2_TO_M2;
        case AreaHectares:
          return CM2_TO_M2 / HA_TO_M2;
        case AreaAcres:
          return CM2_TO_M2 / AC_TO_FT2 / FT2_TO_M2;
        case AreaSquareNauticalMiles:
          return CM2_TO_M2 / NM2_TO_M2;
        case AreaSquareDegrees:
          return CM2_TO_M2 / DEG2_TO_M2;
        case AreaSquareCentimeters:
          return 1.0;
        case AreaSquareMillimeters:
          return CM2_TO_M2 / MM2_TO_M2;
        case AreaUnknownUnit:
          break;
      }

      break;
    }
    case AreaUnknownUnit:
      break;
  }
  return 1.0;
}

QgsUnitTypes::AreaUnit QgsUnitTypes::distanceToAreaUnit( DistanceUnit distanceUnit )
{
  switch ( distanceUnit )
  {
    case DistanceMeters:
      return AreaSquareMeters;

    case DistanceKilometers:
      return AreaSquareKilometers;

    case DistanceCentimeters:
      return AreaSquareCentimeters;

    case DistanceMillimeters:
      return AreaSquareCentimeters;

    case DistanceFeet:
      return AreaSquareFeet;

    case DistanceYards:
      return AreaSquareYards;

    case DistanceMiles:
      return AreaSquareMiles;

    case DistanceDegrees:
      return AreaSquareDegrees;

    case DistanceUnknownUnit:
      return AreaUnknownUnit;

    case DistanceNauticalMiles:
      return AreaSquareNauticalMiles;
  }

  return AreaUnknownUnit;
}

QString QgsUnitTypes::encodeUnit( QgsUnitTypes::AngleUnit unit )
{
  switch ( unit )
  {
    case AngleDegrees:
      return QStringLiteral( "degrees" );
    case AngleRadians:
      return QStringLiteral( "radians" );
    case AngleGon:
      return QStringLiteral( "gon" );
    case AngleMinutesOfArc:
      return QStringLiteral( "moa" );
    case AngleSecondsOfArc:
      return QStringLiteral( "soa" );
    case AngleTurn:
      return QStringLiteral( "tr" );
    case AngleUnknownUnit:
      return QStringLiteral( "<unknown>" );
  }
  return QString();
}

QgsUnitTypes::AngleUnit QgsUnitTypes::decodeAngleUnit( const QString &string, bool *ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( AngleDegrees ) )
    return AngleDegrees;
  if ( normalized == encodeUnit( AngleRadians ) )
    return AngleRadians;
  if ( normalized == encodeUnit( AngleGon ) )
    return AngleGon;
  if ( normalized == encodeUnit( AngleMinutesOfArc ) )
    return AngleMinutesOfArc;
  if ( normalized == encodeUnit( AngleSecondsOfArc ) )
    return AngleSecondsOfArc;
  if ( normalized == encodeUnit( AngleTurn ) )
    return AngleTurn;
  if ( normalized == encodeUnit( AngleUnknownUnit ) )
    return AngleUnknownUnit;
  if ( ok )
    *ok = false;

  return AngleUnknownUnit;
}

QString QgsUnitTypes::toString( QgsUnitTypes::AngleUnit unit )
{
  switch ( unit )
  {
    case AngleDegrees:
      return QObject::tr( "degrees", "angle" );
    case AngleRadians:
      return QObject::tr( "radians", "angle" );
    case AngleGon:
      return QObject::tr( "gon", "angle" );
    case AngleMinutesOfArc:
      return QObject::tr( "minutes of arc", "angle" );
    case AngleSecondsOfArc:
      return QObject::tr( "seconds of arc", "angle" );
    case AngleTurn:
      return QObject::tr( "turns", "angle" );
    case AngleUnknownUnit:
      return QObject::tr( "<unknown>", "angle" );
  }
  return QString();
}

double QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleUnit fromUnit, QgsUnitTypes::AngleUnit toUnit )
{
  // Calculate the conversion factor between the specified units
  switch ( fromUnit )
  {
    case AngleDegrees:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 1.0;
        case AngleRadians:
          return M_PI / 180.0;
        case AngleGon:
          return 400.0 / 360.0;
        case AngleMinutesOfArc:
          return 60;
        case AngleSecondsOfArc:
          return 3600;
        case AngleTurn:
          return 1.0 / 360.0;
        case AngleUnknownUnit:
          break;
      }
      break;
    }
    case AngleRadians:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 180.0 / M_PI;
        case AngleRadians:
          return 1.0;
        case AngleGon:
          return 200.0 / M_PI;
        case AngleMinutesOfArc:
          return 60 * 180.0 / M_PI;
        case AngleSecondsOfArc:
          return 3600 * 180.0 / M_PI;
        case AngleTurn:
          return 0.5 / M_PI;
        case AngleUnknownUnit:
          break;
      }
      break;
    }
    case AngleGon:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 360.0 / 400.0;
        case AngleRadians:
          return M_PI / 200.0;
        case AngleGon:
          return 1.0;
        case AngleMinutesOfArc:
          return 60 * 360.0 / 400.0;
        case AngleSecondsOfArc:
          return 3600 * 360.0 / 400.0;
        case AngleTurn:
          return 1.0 / 400.0;
        case AngleUnknownUnit:
          break;
      }
      break;
    }
    case AngleMinutesOfArc:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 1 / 60.0;
        case AngleRadians:
          return M_PI / 180.0 / 60.0;
        case AngleGon:
          return 400.0 / 360.0 / 60.0;
        case AngleMinutesOfArc:
          return 1.0;
        case AngleSecondsOfArc:
          return 60.0;
        case AngleTurn:
          return 1.0 / 360.0 / 60.0;
        case AngleUnknownUnit:
          break;
      }
      break;
    }
    case AngleSecondsOfArc:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 1 / 3600.0;
        case AngleRadians:
          return M_PI / 180.0 / 3600.0;
        case AngleGon:
          return 400.0 / 360.0 / 3600.0;
        case AngleMinutesOfArc:
          return 1.0 / 60.0;
        case AngleSecondsOfArc:
          return 1.0;
        case AngleTurn:
          return 1.0 / 360.0 / 3600.0;
        case AngleUnknownUnit:
          break;
      }
      break;
    }
    case AngleTurn:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 360.0;
        case AngleRadians:
          return 2 * M_PI;
        case AngleGon:
          return 400.0;
        case AngleMinutesOfArc:
          return 360.0 * 60.0;
        case AngleSecondsOfArc:
          return 360.0 * 3600.0;
        case AngleTurn:
          return 1.0;
        case AngleUnknownUnit:
          break;
      }
      break;
    }
    case AngleUnknownUnit:
      break;
  }
  return 1.0;
}

QString QgsUnitTypes::formatAngle( double angle, int decimals, QgsUnitTypes::AngleUnit unit )
{
  QString unitLabel;

  switch ( unit )
  {
    case AngleDegrees:
      unitLabel = QObject::trUtf8( "°", "angle" );
      break;
    case AngleRadians:
      unitLabel = QObject::trUtf8( " rad", "angle" );
      break;
    case AngleGon:
      unitLabel = QObject::trUtf8( " gon", "angle" );
      break;
    case AngleMinutesOfArc:
      unitLabel = QObject::trUtf8( "′", "angle minutes" );
      break;
    case AngleSecondsOfArc:
      unitLabel = QObject::trUtf8( "″", "angle seconds" );
      break;
    case AngleTurn:
      unitLabel = QObject::trUtf8( " tr", "angle turn" );
      break;
    case AngleUnknownUnit:
      break;
  }

  return QStringLiteral( "%L1%2" ).arg( angle, 0, 'f', decimals ).arg( unitLabel );
}


QgsUnitTypes::DistanceValue QgsUnitTypes::scaledDistance( double distance, QgsUnitTypes::DistanceUnit unit, int decimals, bool keepBaseUnit )
{
  DistanceValue result;

  switch ( unit )
  {
    case DistanceMeters:
      if ( keepBaseUnit )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = QgsUnitTypes::DistanceMeters;
      }
      else if ( std::fabs( distance ) > 1000.0 )
      {
        result.value = qgsRound( distance / 1000, decimals );
        result.unit = QgsUnitTypes::DistanceKilometers;
      }
      else if ( std::fabs( distance ) < 0.01 )
      {
        result.value = qgsRound( distance * 1000, decimals );
        result.unit = QgsUnitTypes::DistanceMillimeters;
      }
      else if ( std::fabs( distance ) < 0.1 )
      {

        result.value = qgsRound( distance * 100, decimals );
        result.unit = QgsUnitTypes::DistanceCentimeters;
      }
      else
      {
        result.value = qgsRound( distance, decimals );
        result.unit = QgsUnitTypes::DistanceMeters;
      }
      break;

    case DistanceKilometers:
      if ( keepBaseUnit || std::fabs( distance ) >= 1.0 )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = QgsUnitTypes::DistanceKilometers;
      }
      else
      {
        result.value = qgsRound( distance * 1000, decimals );
        result.unit = QgsUnitTypes::DistanceMeters;
      }
      break;

    case DistanceFeet:
      if ( std::fabs( distance ) <= 5280.0 || keepBaseUnit )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = QgsUnitTypes::DistanceFeet;
      }
      else
      {
        result.value = qgsRound( distance / 5280.0, decimals );
        result.unit = QgsUnitTypes::DistanceMiles;
      }
      break;

    case DistanceYards:
      if ( std::fabs( distance ) <= 1760.0 || keepBaseUnit )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = QgsUnitTypes::DistanceYards;
      }
      else
      {
        result.value = qgsRound( distance / 1760.0, decimals );
        result.unit = QgsUnitTypes::DistanceMiles;
      }
      break;

    case DistanceMiles:
      if ( std::fabs( distance ) >= 1.0 || keepBaseUnit )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = QgsUnitTypes::DistanceMiles;
      }
      else
      {
        result.value = qgsRound( distance * 5280.0, decimals );
        result.unit = QgsUnitTypes::DistanceFeet;
      }
      break;

    case DistanceNauticalMiles:
      result.value = qgsRound( distance, decimals );
      result.unit = QgsUnitTypes::DistanceNauticalMiles;
      break;

    case DistanceDegrees:
      result.value = qgsRound( distance, decimals );
      result.unit = QgsUnitTypes::DistanceDegrees;
      break;

    case DistanceUnknownUnit:
      result.value = qgsRound( distance, decimals );
      result.unit = QgsUnitTypes::DistanceUnknownUnit;
      break;

    default:
      result.value = qgsRound( distance, decimals );
      result.unit = unit;
      break;
  }

  return result;
}

QgsUnitTypes::AreaValue QgsUnitTypes::scaledArea( double area, QgsUnitTypes::AreaUnit unit, int decimals, bool keepBaseUnit )
{
  AreaValue result;
  result.value = -1.0;
  result.unit = AreaUnknownUnit;

  // If we are not forced to keep the base units, switch to meter calculation
  if ( unit == AreaSquareMillimeters )
  {
    if ( keepBaseUnit )
    {
      result.value = qgsRound( area, decimals );
      result.unit = QgsUnitTypes::AreaSquareMillimeters;
    }
    else
    {
      area /= 1000000.0;
      unit = QgsUnitTypes::AreaSquareMeters;
    }
  }
  else if ( unit == AreaSquareCentimeters )
  {
    if ( keepBaseUnit )
    {
      result.value = qgsRound( area, decimals );
      result.unit = QgsUnitTypes::AreaSquareCentimeters;
    }
    else
    {
      area /= 10000.0;
      unit = QgsUnitTypes::AreaSquareMeters;
    }
  }

  switch ( unit )
  {
    case AreaSquareCentimeters:
      // handled in the if above
      break;
    case AreaSquareMillimeters:
      // handled in the if above
      break;
    case AreaSquareMeters:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaSquareMeters;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareKilometers, QgsUnitTypes::AreaSquareMeters ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareMeters, QgsUnitTypes::AreaSquareKilometers ), decimals );
        result.unit = QgsUnitTypes::AreaSquareKilometers;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaHectares, QgsUnitTypes::AreaSquareMeters ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareMeters, QgsUnitTypes::AreaHectares ), decimals );
        result.unit = QgsUnitTypes::AreaHectares;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaSquareMeters;
      }
      break;
    }

    case AreaSquareKilometers:
    {
      result.value = qgsRound( area, decimals );
      result.unit = QgsUnitTypes::AreaSquareKilometers;
      break;
    }

    case AreaSquareFeet:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaSquareFeet;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareMiles, QgsUnitTypes::AreaSquareFeet ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareFeet, QgsUnitTypes::AreaSquareMiles ), decimals );
        result.unit = QgsUnitTypes::AreaSquareMiles;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaSquareFeet;
      }
      break;
    }

    case AreaSquareYards:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaSquareYards;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareMiles, QgsUnitTypes::AreaSquareYards ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareYards, QgsUnitTypes::AreaSquareMiles ), decimals );
        result.unit = QgsUnitTypes::AreaSquareMiles;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaSquareYards;
      }
      break;
    }

    case AreaSquareMiles:
    {
      result.value = qgsRound( area, decimals );
      result.unit = QgsUnitTypes::AreaSquareMiles;
      break;
    }

    case AreaHectares:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaHectares;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareKilometers, QgsUnitTypes::AreaHectares ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaHectares, QgsUnitTypes::AreaSquareKilometers ), decimals );
        result.unit = QgsUnitTypes::AreaSquareKilometers;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaHectares;
      }
      break;
    }

    case AreaAcres:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaAcres;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaSquareMiles, QgsUnitTypes::AreaAcres ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaAcres, QgsUnitTypes::AreaSquareMiles ), decimals );
        result.unit = QgsUnitTypes::AreaSquareMiles;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = QgsUnitTypes::AreaAcres;
      }
      break;
    }

    case AreaSquareNauticalMiles:
    {
      result.value = qgsRound( area, decimals );
      result.unit = QgsUnitTypes::AreaSquareNauticalMiles;
      break;
    }

    case AreaSquareDegrees:
    {
      result.value = qgsRound( area, decimals );
      result.unit = QgsUnitTypes::AreaSquareDegrees;
      break;
    }

    case AreaUnknownUnit:
    {
      result.value = qgsRound( area, decimals );
      result.unit = QgsUnitTypes::AreaUnknownUnit;
      break;
    }
  }
  return result;
}


QString QgsUnitTypes::formatDistance( double distance, int decimals, QgsUnitTypes::DistanceUnit unit, bool keepBaseUnit )
{
  DistanceValue dist = scaledDistance( distance, unit, decimals, keepBaseUnit );

  QString unitText;

  if ( dist.unit != DistanceUnknownUnit )
    unitText = QChar( ' ' ) + QgsUnitTypes::toAbbreviatedString( dist.unit );

  return QStringLiteral( "%L1%2" ).arg( dist.value, 0, 'f', decimals ).arg( unitText );
}

QString QgsUnitTypes::formatArea( double area, int decimals, QgsUnitTypes::AreaUnit unit, bool keepBaseUnit )
{
  AreaValue areaValue = scaledArea( area, unit, decimals, keepBaseUnit );

  QString unitText;

  if ( areaValue.unit != AreaUnknownUnit )
    unitText = QChar( ' ' ) + QgsUnitTypes::toAbbreviatedString( areaValue.unit );

  return QStringLiteral( "%L1%2" ).arg( areaValue.value, 0, 'f', decimals ).arg( unitText );
}

QString QgsUnitTypes::encodeUnit( RenderUnit unit )
{
  switch ( unit )
  {
    case RenderMillimeters:
      return QStringLiteral( "MM" );
    case RenderMetersInMapUnits:
      return QStringLiteral( "RenderMetersInMapUnits" );
    case RenderMapUnits:
      return QStringLiteral( "MapUnit" );
    case RenderPixels:
      return QStringLiteral( "Pixel" );
    case RenderPercentage:
      return QStringLiteral( "Percentage" );
    case RenderPoints:
      return QStringLiteral( "Point" );
    case RenderInches:
      return QStringLiteral( "Inch" );
    case RenderUnknownUnit:
      return QString();
  }
  return QString();
}

QgsUnitTypes::RenderUnit QgsUnitTypes::decodeRenderUnit( const QString &string, bool *ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( RenderMillimeters ).toLower() )
    return RenderMillimeters;
  if ( normalized == encodeUnit( RenderMetersInMapUnits ).toLower() )
    return RenderMetersInMapUnits;
  if ( normalized == QLatin1String( "meters" ) )
    return RenderMetersInMapUnits;
  if ( normalized == encodeUnit( RenderMapUnits ).toLower() )
    return RenderMapUnits;
  if ( normalized == QLatin1String( "mapunits" ) )
    return RenderMapUnits;
  if ( normalized == encodeUnit( RenderPixels ).toLower() )
    return RenderPixels;
  if ( normalized == encodeUnit( RenderPercentage ).toLower() )
    return RenderPercentage;
  if ( normalized == QLatin1String( "percent" ) )
    return RenderPercentage;
  if ( normalized == encodeUnit( RenderPoints ).toLower() )
    return RenderPoints;
  if ( normalized == QLatin1String( "points" ) )
    return RenderPoints;
  if ( normalized == encodeUnit( RenderInches ).toLower() )
    return RenderInches;

  if ( ok )
    *ok = false;

  // millimeters are default
  return RenderMillimeters;
}

QString QgsUnitTypes::toString( QgsUnitTypes::RenderUnit unit )
{
  switch ( unit )
  {
    case RenderMillimeters:
      return QObject::tr( "millimeters", "render" );

    case RenderMetersInMapUnits:
      return QObject::tr( "meters (at map scale)", "render" );

    case RenderMapUnits:
      return QObject::tr( "map units", "render" );

    case RenderPixels:
      return QObject::tr( "pixels", "render" );

    case RenderPercentage:
      return QObject::tr( "percent", "render" );

    case RenderPoints:
      return QObject::tr( "points", "render" );

    case RenderInches:
      return QObject::tr( "inches", "render" );

    case RenderUnknownUnit:
      return QObject::tr( "<unknown>", "render" );

  }
  return QString();
}



QString QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutUnit unit )
{
  switch ( unit )
  {
    case LayoutCentimeters:
      return QStringLiteral( "cm" );
    case LayoutMeters:
      return QStringLiteral( "m" );
    case LayoutInches:
      return QStringLiteral( "in" );
    case LayoutFeet:
      return QStringLiteral( "ft" );
    case LayoutPoints:
      return QStringLiteral( "pt" );
    case LayoutPicas:
      return QStringLiteral( "pi" );
    case LayoutPixels:
      return QStringLiteral( "px" );
    case  LayoutMillimeters:
      return QStringLiteral( "mm" );
  }
  return QString();
}

QgsUnitTypes::LayoutUnit QgsUnitTypes::decodeLayoutUnit( const QString &string, bool *ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( LayoutMillimeters ).toLower() )
    return LayoutMillimeters;
  if ( normalized == encodeUnit( LayoutCentimeters ).toLower() )
    return LayoutCentimeters;
  if ( normalized == encodeUnit( LayoutMeters ).toLower() )
    return LayoutMeters;
  if ( normalized == encodeUnit( LayoutInches ).toLower() )
    return LayoutInches;
  if ( normalized == encodeUnit( LayoutFeet ).toLower() )
    return LayoutFeet;
  if ( normalized == encodeUnit( LayoutPoints ).toLower() )
    return LayoutPoints;
  if ( normalized == encodeUnit( LayoutPicas ).toLower() )
    return LayoutPicas;
  if ( normalized == encodeUnit( LayoutPixels ).toLower() )
    return LayoutPixels;

  if ( ok )
    *ok = false;

  // millimeters are default
  return LayoutMillimeters;
}

QgsUnitTypes::LayoutUnitType QgsUnitTypes::unitType( const QgsUnitTypes::LayoutUnit units )
{
  switch ( units )
  {
    case LayoutPixels:
      return LayoutScreenUnits;
    case  LayoutMillimeters:
    case LayoutCentimeters:
    case LayoutMeters:
    case LayoutInches:
    case LayoutFeet:
    case LayoutPoints:
    case LayoutPicas:
      return LayoutPaperUnits;
  }

  // avoid warnings
  return LayoutPaperUnits;
}

QString QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::LayoutUnit unit )
{
  switch ( unit )
  {
    case LayoutPixels:
      return QObject::tr( "px" );
    case  LayoutMillimeters:
      return QObject::tr( "mm" );
    case LayoutCentimeters:
      return QObject::tr( "cm" );
    case LayoutMeters:
      return QObject::tr( "m" );
    case LayoutInches:
      return QObject::tr( "in" );
    case LayoutFeet:
      return QObject::tr( "ft" );
    case LayoutPoints:
      return QObject::tr( "pt" );
    case LayoutPicas:
      return QObject::tr( "pica" );
  }
  return QString(); // no warnings
}

QString QgsUnitTypes::toString( QgsUnitTypes::LayoutUnit unit )
{
  switch ( unit )
  {
    case LayoutPixels:
      return QObject::tr( "pixels" );
    case  LayoutMillimeters:
      return QObject::tr( "millimeters" );
    case LayoutCentimeters:
      return QObject::tr( "centimeters" );
    case LayoutMeters:
      return QObject::tr( "meters" );
    case LayoutInches:
      return QObject::tr( "inches" );
    case LayoutFeet:
      return QObject::tr( "feet" );
    case LayoutPoints:
      return QObject::tr( "points" );
    case LayoutPicas:
      return QObject::tr( "picas" );
  }
  return QString(); // no warnings
}
