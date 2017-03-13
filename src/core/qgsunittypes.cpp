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
      return QObject::trUtf8( "ha²", "area" );
    case AreaAcres:
      return QObject::trUtf8( "ac²", "area" );
    case AreaSquareNauticalMiles:
      return QObject::trUtf8( "NM²", "area" );
    case AreaSquareDegrees:
      return QObject::trUtf8( "deg²", "area" );
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
  if ( normalized == toString( AreaUnknownUnit ) )
    return AreaUnknownUnit;
  if ( ok )
    *ok = false;

  return AreaUnknownUnit;
}

double QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AreaUnit fromUnit, QgsUnitTypes::AreaUnit toUnit )
{
#define KM2_TO_M2 1000000.0
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
          return KM2_TO_M2 / FT2_TO_M2 ;
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

QString QgsUnitTypes::encodeUnit( RenderUnit unit )
{
  switch ( unit )
  {
    case RenderMillimeters:
      return QStringLiteral( "MM" );
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
