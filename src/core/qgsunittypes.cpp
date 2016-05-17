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
#include <QCoreApplication>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

QgsUnitTypes::DistanceUnitType QgsUnitTypes::unitType( QGis::UnitType unit )
{
  switch ( unit )
  {
    case QGis::Meters:
    case QGis::Feet:
    case QGis::NauticalMiles:
    case QGis::Yards:
    case QGis::Miles:
    case QGis::Kilometers:
      return Standard;

    case QGis::Degrees:
      return Geographic;

    case QGis::UnknownUnit:
      return UnknownType;
  }
  return UnknownType;
}

QgsUnitTypes::DistanceUnitType QgsUnitTypes::unitType( QgsUnitTypes::AreaUnit unit )
{
  switch ( unit )
  {
    case SquareMeters:
    case SquareKilometers:
    case SquareFeet:
    case SquareYards:
    case SquareMiles:
    case Hectares:
    case Acres:
    case SquareNauticalMiles:
      return Standard;

    case SquareDegrees:
      return Geographic;

    case UnknownAreaUnit:
      return UnknownType;
  }

  return UnknownType;
}

QString QgsUnitTypes::encodeUnit( QGis::UnitType unit )
{
  switch ( unit )
  {
    case QGis::Meters:
      return "meters";

    case QGis::Kilometers:
      return "km";

    case QGis::Feet:
      return "feet";

    case QGis::Yards:
      return "yd";

    case QGis::Miles:
      return "mi";

    case QGis::Degrees:
      return "degrees";

    case QGis::UnknownUnit:
      return "<unknown>";

    case QGis::NauticalMiles:
      return "nautical miles";
  }
  return QString();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

QGis::UnitType QgsUnitTypes::decodeDistanceUnit( const QString& string, bool* ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( QGis::Meters ) )
    return QGis::Meters;
  if ( normalized == encodeUnit( QGis::Feet ) )
    return QGis::Feet;
  if ( normalized == encodeUnit( QGis::Degrees ) )
    return QGis::Degrees;
  if ( normalized == encodeUnit( QGis::NauticalMiles ) )
    return QGis::NauticalMiles;
  if ( normalized == encodeUnit( QGis::Kilometers ) )
    return QGis::Kilometers;
  if ( normalized == encodeUnit( QGis::Yards ) )
    return QGis::Yards;
  if ( normalized == encodeUnit( QGis::Miles ) )
    return QGis::Miles;
  if ( normalized == encodeUnit( QGis::UnknownUnit ) )
    return QGis::UnknownUnit;

  if ( ok )
    *ok = false;

  return QGis::UnknownUnit;
}

QString QgsUnitTypes::toString( QGis::UnitType unit )
{
  switch ( unit )
  {
    case QGis::Meters:
      return QCoreApplication::translate( "QGis::UnitType", "meters" );

    case QGis::Kilometers:
      return QCoreApplication::translate( "QGis::UnitType", "kilometers" );

    case QGis::Feet:
      return QCoreApplication::translate( "QGis::UnitType", "feet" );

    case QGis::Yards:
      return QCoreApplication::translate( "QGis::UnitType", "yards" );

    case QGis::Miles:
      return QCoreApplication::translate( "QGis::UnitType", "miles" );

    case QGis::Degrees:
      return QCoreApplication::translate( "QGis::UnitType", "degrees" );

    case QGis::UnknownUnit:
      return QCoreApplication::translate( "QGis::UnitType", "<unknown>" );

    case QGis::NauticalMiles:
      return QCoreApplication::translate( "QGis::UnitType", "nautical miles" );
  }
  return QString();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

QGis::UnitType QgsUnitTypes::stringToDistanceUnit( const QString& string, bool* ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == toString( QGis::Meters ) )
    return QGis::Meters;
  if ( normalized == toString( QGis::Kilometers ) )
    return QGis::Kilometers;
  if ( normalized == toString( QGis::Feet ) )
    return QGis::Feet;
  if ( normalized == toString( QGis::Yards ) )
    return QGis::Yards;
  if ( normalized == toString( QGis::Miles ) )
    return QGis::Miles;
  if ( normalized == toString( QGis::Degrees ) )
    return QGis::Degrees;
  if ( normalized == toString( QGis::NauticalMiles ) )
    return QGis::NauticalMiles;
  if ( normalized == toString( QGis::UnknownUnit ) )
    return QGis::UnknownUnit;

  if ( ok )
    *ok = false;

  return QGis::UnknownUnit;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

double QgsUnitTypes::fromUnitToUnitFactor( QGis::UnitType fromUnit, QGis::UnitType toUnit )
{
#define DEGREE_TO_METER 111319.49079327358
#define FEET_TO_METER 0.3048
#define NMILE_TO_METER 1852.0
#define KILOMETERS_TO_METER 1000.0
#define YARDS_TO_METER 0.9144
#define YARDS_TO_FEET 3.0
#define MILES_TO_METER 1609.344
  // Unify degree units
  // remove for QGIS 3.0, as extra degree types will be removed
  if ( fromUnit == QGis::DecimalDegrees || fromUnit == QGis::DegreesMinutesSeconds || fromUnit == QGis::DegreesDecimalMinutes )
    fromUnit = QGis::Degrees;
  if ( toUnit == QGis::DecimalDegrees || toUnit == QGis::DegreesMinutesSeconds || toUnit == QGis::DegreesDecimalMinutes )
    toUnit = QGis::Degrees;

  // Calculate the conversion factor between the specified units
  switch ( fromUnit )
  {
    case QGis::Meters:
    {
      switch ( toUnit )
      {
        case QGis::Meters:
          return 1.0;
        case QGis::Kilometers:
          return 1.0 / KILOMETERS_TO_METER;
        case QGis::Feet:
          return 1.0 / FEET_TO_METER;
        case QGis::Yards:
          return 1.0 / YARDS_TO_METER;
        case QGis::Miles:
          return 1.0 / MILES_TO_METER;
        case QGis::Degrees:
          return 1.0 / DEGREE_TO_METER;
        case QGis::NauticalMiles:
          return 1.0 / NMILE_TO_METER;
        case QGis::UnknownUnit:
          break;
      }

      break;
    }
    case QGis::Kilometers:
    {
      switch ( toUnit )
      {
        case QGis::Meters:
          return KILOMETERS_TO_METER;
        case QGis::Kilometers:
          return 1.0;
        case QGis::Feet:
          return KILOMETERS_TO_METER / FEET_TO_METER;
        case QGis::Yards:
          return KILOMETERS_TO_METER / YARDS_TO_METER;
        case QGis::Miles:
          return KILOMETERS_TO_METER / MILES_TO_METER;
        case QGis::Degrees:
          return KILOMETERS_TO_METER / DEGREE_TO_METER;
        case QGis::NauticalMiles:
          return KILOMETERS_TO_METER / NMILE_TO_METER;
        case QGis::UnknownUnit:
          break;
      }

      break;
    }
    case QGis::Feet:
    {
      switch ( toUnit )
      {
        case QGis::Meters:
          return FEET_TO_METER;
        case QGis::Kilometers:
          return FEET_TO_METER / KILOMETERS_TO_METER;
        case QGis::Feet:
          return 1.0;
        case QGis::Yards:
          return 1.0 / YARDS_TO_FEET;
        case QGis::Miles:
          return FEET_TO_METER / MILES_TO_METER;
        case QGis::Degrees:
          return FEET_TO_METER / DEGREE_TO_METER;
        case QGis::NauticalMiles:
          return FEET_TO_METER / NMILE_TO_METER;
        case QGis::UnknownUnit:
          break;
      }

      break;
    }
    case QGis::Yards:
    {
      switch ( toUnit )
      {
        case QGis::Meters:
          return YARDS_TO_METER;
        case QGis::Kilometers:
          return YARDS_TO_METER / KILOMETERS_TO_METER;
        case QGis::Feet:
          return YARDS_TO_FEET;
        case QGis::Yards:
          return 1.0;
        case QGis::Miles:
          return YARDS_TO_METER / MILES_TO_METER;
        case QGis::Degrees:
          return YARDS_TO_METER / DEGREE_TO_METER;
        case QGis::NauticalMiles:
          return YARDS_TO_METER / NMILE_TO_METER;
        case QGis::UnknownUnit:
          break;
      }

      break;
    }
    case QGis::Miles:
    {
      switch ( toUnit )
      {
        case QGis::Meters:
          return MILES_TO_METER;
        case QGis::Kilometers:
          return MILES_TO_METER / KILOMETERS_TO_METER;
        case QGis::Feet:
          return MILES_TO_METER / FEET_TO_METER;
        case QGis::Yards:
          return MILES_TO_METER / YARDS_TO_METER;
        case QGis::Miles:
          return 1.0;
        case QGis::Degrees:
          return MILES_TO_METER / DEGREE_TO_METER;
        case QGis::NauticalMiles:
          return MILES_TO_METER / NMILE_TO_METER;
        case QGis::UnknownUnit:
          break;
      }

      break;
    }
    case QGis::Degrees:
    {
      switch ( toUnit )
      {
        case QGis::Meters:
          return DEGREE_TO_METER;
        case QGis::Kilometers:
          return DEGREE_TO_METER / KILOMETERS_TO_METER;
        case QGis::Feet:
          return DEGREE_TO_METER / FEET_TO_METER;
        case QGis::Yards:
          return DEGREE_TO_METER / YARDS_TO_METER;
        case QGis::Miles:
          return DEGREE_TO_METER / MILES_TO_METER;
        case QGis::Degrees:
          return 1.0;
        case QGis::NauticalMiles:
          return DEGREE_TO_METER / NMILE_TO_METER;
        case QGis::UnknownUnit:
          break;
      }

      break;
    }
    case QGis::NauticalMiles:
    {
      switch ( toUnit )
      {
        case QGis::Meters:
          return NMILE_TO_METER;
        case QGis::Kilometers:
          return NMILE_TO_METER / KILOMETERS_TO_METER;
        case QGis::Feet:
          return NMILE_TO_METER / FEET_TO_METER;
        case QGis::Yards:
          return NMILE_TO_METER / YARDS_TO_METER;
        case QGis::Miles:
          return NMILE_TO_METER / MILES_TO_METER;
        case QGis::Degrees:
          return NMILE_TO_METER / DEGREE_TO_METER;
        case QGis::NauticalMiles:
          return 1.0;
        case QGis::UnknownUnit:
          break;
      }

      break;
    }
    case QGis::UnknownUnit:
      break;
  }
  return 1.0;
}

QString QgsUnitTypes::encodeUnit( QgsUnitTypes::AreaUnit unit )
{
  switch ( unit )
  {
    case SquareMeters:
      return "m2";
    case SquareKilometers:
      return "km2";
    case SquareFeet:
      return "ft2";
    case SquareYards:
      return "y2";
    case SquareMiles:
      return "mi2";
    case Hectares:
      return "ha";
    case Acres:
      return "ac";
    case SquareNauticalMiles:
      return "nm2";
    case SquareDegrees:
      return "deg2";
    case UnknownAreaUnit:
      return "<unknown>";
  }
  return QString();
}

QgsUnitTypes::AreaUnit QgsUnitTypes::decodeAreaUnit( const QString& string, bool* ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( SquareMeters ) )
    return SquareMeters;
  if ( normalized == encodeUnit( SquareKilometers ) )
    return SquareKilometers;
  if ( normalized == encodeUnit( SquareFeet ) )
    return SquareFeet;
  if ( normalized == encodeUnit( SquareYards ) )
    return SquareYards;
  if ( normalized == encodeUnit( SquareMiles ) )
    return SquareMiles;
  if ( normalized == encodeUnit( Hectares ) )
    return Hectares;
  if ( normalized == encodeUnit( Acres ) )
    return Acres;
  if ( normalized == encodeUnit( SquareNauticalMiles ) )
    return SquareNauticalMiles;
  if ( normalized == encodeUnit( SquareDegrees ) )
    return SquareDegrees;
  if ( normalized == encodeUnit( UnknownAreaUnit ) )
    return UnknownAreaUnit;

  if ( ok )
    *ok = false;

  return UnknownAreaUnit;
}

QString QgsUnitTypes::toString( QgsUnitTypes::AreaUnit unit )
{
  switch ( unit )
  {
    case SquareMeters:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "square meters" );
    case SquareKilometers:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "square kilometers" );
    case SquareFeet:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "square feet" );
    case SquareYards:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "square yards" );
    case SquareMiles:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "square miles" );
    case Hectares:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "hectares" );
    case Acres:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "acres" );
    case SquareNauticalMiles:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "square nautical miles" );
    case SquareDegrees:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "square degrees" );
    case UnknownAreaUnit:
      return QCoreApplication::translate( "QgsUnitTypes::AreaUnit", "<unknown>" );
  }
  return QString();
}

QgsUnitTypes::AreaUnit QgsUnitTypes::stringToAreaUnit( const QString& string, bool* ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == toString( SquareMeters ) )
    return SquareMeters;
  if ( normalized == toString( SquareKilometers ) )
    return SquareKilometers;
  if ( normalized == toString( SquareFeet ) )
    return SquareFeet;
  if ( normalized == toString( SquareYards ) )
    return SquareYards;
  if ( normalized == toString( SquareMiles ) )
    return SquareMiles;
  if ( normalized == toString( Hectares ) )
    return Hectares;
  if ( normalized == toString( Acres ) )
    return Acres;
  if ( normalized == toString( SquareNauticalMiles ) )
    return SquareNauticalMiles;
  if ( normalized == toString( SquareDegrees ) )
    return SquareDegrees;
  if ( normalized == toString( UnknownAreaUnit ) )
    return UnknownAreaUnit;
  if ( ok )
    *ok = false;

  return UnknownAreaUnit;
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
    case SquareMeters:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return 1.0;
        case SquareKilometers:
          return 1.0 / KM2_TO_M2;
        case SquareFeet:
          return 1.0 / FT2_TO_M2;
        case SquareYards:
          return 1.0 / YD2_TO_M2;
        case SquareMiles:
          return 1.0 / MI2_TO_M2;
        case Hectares:
          return 1.0 / HA_TO_M2;
        case Acres:
          return 1.0 / AC_TO_FT2 / FT2_TO_M2;
        case SquareNauticalMiles:
          return 1.0 / NM2_TO_M2;
        case SquareDegrees:
          return 1.0 / DEG2_TO_M2;
        case UnknownAreaUnit:
          break;
      }

      break;
    }
    case SquareKilometers:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return KM2_TO_M2;
        case SquareKilometers:
          return 1.0;
        case SquareFeet:
          return KM2_TO_M2 / FT2_TO_M2 ;
        case SquareYards:
          return KM2_TO_M2 / YD2_TO_M2;
        case SquareMiles:
          return KM2_TO_M2 / MI2_TO_M2;
        case Hectares:
          return KM2_TO_M2 / HA_TO_M2;
        case Acres:
          return KM2_TO_M2 / AC_TO_FT2 / FT2_TO_M2;
        case SquareNauticalMiles:
          return KM2_TO_M2 / NM2_TO_M2;
        case SquareDegrees:
          return KM2_TO_M2 / DEG2_TO_M2;
        case UnknownAreaUnit:
          break;
      }

      break;
    }
    case SquareFeet:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return FT2_TO_M2;
        case SquareKilometers:
          return FT2_TO_M2 / KM2_TO_M2;
        case SquareFeet:
          return 1.0;
        case SquareYards:
          return FT2_TO_M2 / YD2_TO_M2;
        case SquareMiles:
          return FT2_TO_M2 / MI2_TO_M2;
        case Hectares:
          return FT2_TO_M2 / HA_TO_M2;
        case Acres:
          return 1.0 / AC_TO_FT2;
        case SquareNauticalMiles:
          return FT2_TO_M2 / NM2_TO_M2;
        case SquareDegrees:
          return FT2_TO_M2 / DEG2_TO_M2;
        case UnknownAreaUnit:
          break;
      }

      break;
    }

    case SquareYards:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return YD2_TO_M2;
        case SquareKilometers:
          return YD2_TO_M2 / KM2_TO_M2;
        case SquareFeet:
          return YD2_TO_M2 / FT2_TO_M2;
        case SquareYards:
          return 1.0;
        case SquareMiles:
          return YD2_TO_M2 / MI2_TO_M2;
        case Hectares:
          return YD2_TO_M2 / HA_TO_M2;
        case Acres:
          return YD2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case SquareNauticalMiles:
          return YD2_TO_M2 / NM2_TO_M2;
        case SquareDegrees:
          return YD2_TO_M2 / DEG2_TO_M2;
        case UnknownAreaUnit:
          break;
      }
      break;
    }

    case SquareMiles:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return MI2_TO_M2;
        case SquareKilometers:
          return MI2_TO_M2 / KM2_TO_M2;
        case SquareFeet:
          return MI2_TO_M2 / FT2_TO_M2;
        case SquareYards:
          return MI2_TO_M2 / YD2_TO_M2;
        case SquareMiles:
          return 1.0;
        case Hectares:
          return MI2_TO_M2 / HA_TO_M2;
        case Acres:
          return MI2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case SquareNauticalMiles:
          return MI2_TO_M2 / NM2_TO_M2;
        case SquareDegrees:
          return MI2_TO_M2 / DEG2_TO_M2;
        case UnknownAreaUnit:
          break;
      }

      break;
    }

    case Hectares:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return HA_TO_M2;
        case SquareKilometers:
          return HA_TO_M2 / KM2_TO_M2;
        case SquareFeet:
          return HA_TO_M2 / FT2_TO_M2;
        case SquareYards:
          return HA_TO_M2 / YD2_TO_M2;
        case SquareMiles:
          return HA_TO_M2 / MI2_TO_M2;
        case Hectares:
          return 1.0;
        case Acres:
          return HA_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case SquareNauticalMiles:
          return HA_TO_M2 / NM2_TO_M2;
        case SquareDegrees:
          return HA_TO_M2 / DEG2_TO_M2;
        case UnknownAreaUnit:
          break;
      }

      break;
    }

    case Acres:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return AC_TO_FT2 * FT2_TO_M2;
        case SquareKilometers:
          return AC_TO_FT2 * FT2_TO_M2 / KM2_TO_M2;
        case SquareFeet:
          return AC_TO_FT2;
        case SquareYards:
          return AC_TO_FT2 * FT2_TO_M2 / YD2_TO_M2;
        case SquareMiles:
          return AC_TO_FT2 * FT2_TO_M2 / MI2_TO_M2;
        case Hectares:
          return AC_TO_FT2 * FT2_TO_M2 / HA_TO_M2;
        case Acres:
          return 1.0;
        case SquareNauticalMiles:
          return AC_TO_FT2 * FT2_TO_M2 / NM2_TO_M2;
        case SquareDegrees:
          return AC_TO_FT2 * FT2_TO_M2 / DEG2_TO_M2;
        case UnknownAreaUnit:
          break;
      }

      break;
    }

    case SquareNauticalMiles:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return NM2_TO_M2;
        case SquareKilometers:
          return NM2_TO_M2 / KM2_TO_M2;
        case SquareFeet:
          return NM2_TO_M2 / FT2_TO_M2;
        case SquareYards:
          return NM2_TO_M2 / YD2_TO_M2;
        case SquareMiles:
          return NM2_TO_M2 / MI2_TO_M2;
        case Hectares:
          return NM2_TO_M2 / HA_TO_M2;
        case Acres:
          return NM2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case SquareNauticalMiles:
          return 1.0;
        case SquareDegrees:
          return NM2_TO_M2 / DEG2_TO_M2;
        case UnknownAreaUnit:
          break;
      }

      break;
    }

    case SquareDegrees:
    {
      switch ( toUnit )
      {
        case SquareMeters:
          return DEG2_TO_M2;
        case SquareKilometers:
          return DEG2_TO_M2 / KM2_TO_M2;
        case SquareFeet:
          return DEG2_TO_M2 / FT2_TO_M2;
        case SquareYards:
          return DEG2_TO_M2 / YD2_TO_M2;
        case SquareMiles:
          return DEG2_TO_M2 / MI2_TO_M2;
        case Hectares:
          return DEG2_TO_M2 / HA_TO_M2;
        case Acres:
          return DEG2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case SquareNauticalMiles:
          return DEG2_TO_M2 / NM2_TO_M2;
        case SquareDegrees:
          return 1.0;
        case UnknownAreaUnit:
          break;
      }

      break;
    }

    case UnknownAreaUnit:
      break;
  }
  return 1.0;
}

QgsUnitTypes::AreaUnit QgsUnitTypes::distanceToAreaUnit( QGis::UnitType distanceUnit )
{
  switch ( distanceUnit )
  {
    case QGis::Meters:
      return SquareMeters;

    case QGis::Kilometers:
      return SquareKilometers;

    case QGis::Feet:
      return SquareFeet;

    case QGis::Yards:
      return SquareYards;

    case QGis::Miles:
      return SquareMiles;

    case QGis::Degrees:
      return SquareDegrees;

    case QGis::UnknownUnit:
      return UnknownAreaUnit;

    case QGis::NauticalMiles:
      return SquareNauticalMiles;
  }

  return UnknownAreaUnit;
}

QString QgsUnitTypes::encodeUnit( QgsUnitTypes::AngleUnit unit )
{
  switch ( unit )
  {
    case AngleDegrees:
      return "degrees";
    case Radians:
      return "radians";
    case Gon:
      return "gon";
    case MinutesOfArc:
      return "moa";
    case SecondsOfArc:
      return "soa";
    case Turn:
      return "tr";
    case UnknownAngleUnit:
      return "<unknown>";
  }
  return QString();
}

QgsUnitTypes::AngleUnit QgsUnitTypes::decodeAngleUnit( const QString& string, bool* ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( AngleDegrees ) )
    return AngleDegrees;
  if ( normalized == encodeUnit( Radians ) )
    return Radians;
  if ( normalized == encodeUnit( Gon ) )
    return Gon;
  if ( normalized == encodeUnit( MinutesOfArc ) )
    return MinutesOfArc;
  if ( normalized == encodeUnit( SecondsOfArc ) )
    return SecondsOfArc;
  if ( normalized == encodeUnit( Turn ) )
    return Turn;
  if ( normalized == encodeUnit( UnknownAngleUnit ) )
    return UnknownAngleUnit;
  if ( ok )
    *ok = false;

  return UnknownAngleUnit;
}

QString QgsUnitTypes::toString( QgsUnitTypes::AngleUnit unit )
{
  switch ( unit )
  {
    case AngleDegrees:
      return QCoreApplication::translate( "QgsUnitTypes::AngleUnit", "degrees" );
    case Radians:
      return QCoreApplication::translate( "QgsUnitTypes::AngleUnit", "radians" );
    case Gon:
      return QCoreApplication::translate( "QgsUnitTypes::AngleUnit", "gon" );
    case MinutesOfArc:
      return QCoreApplication::translate( "QgsUnitTypes::AngleUnit", "minutes of arc" );
    case SecondsOfArc:
      return QCoreApplication::translate( "QgsUnitTypes::AngleUnit", "seconds of arc" );
    case Turn:
      return QCoreApplication::translate( "QgsUnitTypes::AngleUnit", "turns" );
    case UnknownAngleUnit:
      return QCoreApplication::translate( "QgsUnitTypes::AngleUnit", "<unknown>" );
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
        case Radians:
          return M_PI / 180.0;
        case Gon:
          return 400.0 / 360.0;
        case MinutesOfArc:
          return 60;
        case SecondsOfArc:
          return 3600;
        case Turn:
          return 1.0 / 360.0;
        case UnknownAngleUnit:
          break;
      }
      break;
    }
    case Radians:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 180.0 / M_PI;
        case Radians:
          return 1.0;
        case Gon:
          return 200.0 / M_PI;
        case MinutesOfArc:
          return 60 * 180.0 / M_PI;
        case SecondsOfArc:
          return 3600 * 180.0 / M_PI;
        case Turn:
          return 0.5 / M_PI;
        case UnknownAngleUnit:
          break;
      }
      break;
    }
    case Gon:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 360.0 / 400.0;
        case Radians:
          return M_PI / 200.0;
        case Gon:
          return 1.0;
        case MinutesOfArc:
          return 60 * 360.0 / 400.0;
        case SecondsOfArc:
          return 3600 * 360.0 / 400.0;
        case Turn:
          return 1.0 / 400.0;
        case UnknownAngleUnit:
          break;
      }
      break;
    }
    case MinutesOfArc:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 1 / 60.0;
        case Radians:
          return M_PI / 180.0 / 60.0;
        case Gon:
          return 400.0 / 360.0 / 60.0;
        case MinutesOfArc:
          return 1.0;
        case SecondsOfArc:
          return 60.0;
        case Turn:
          return 1.0 / 360.0 / 60.0;
        case UnknownAngleUnit:
          break;
      }
      break;
    }
    case SecondsOfArc:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 1 / 3600.0;
        case Radians:
          return M_PI / 180.0 / 3600.0;
        case Gon:
          return 400.0 / 360.0 / 3600.0;
        case MinutesOfArc:
          return 1.0 / 60.0;
        case SecondsOfArc:
          return 1.0;
        case Turn:
          return 1.0 / 360.0 / 3600.0;
        case UnknownAngleUnit:
          break;
      }
      break;
    }
    case Turn:
    {
      switch ( toUnit )
      {
        case AngleDegrees:
          return 360.0;
        case Radians:
          return 2 * M_PI;
        case Gon:
          return 400.0;
        case MinutesOfArc:
          return 360.0 * 60.0;
        case SecondsOfArc:
          return 360.0 * 3600.0;
        case Turn:
          return 1.0;
        case UnknownAngleUnit:
          break;
      }
      break;
    }
    case UnknownAngleUnit:
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
      unitLabel = QObject::trUtf8( "°" );
      break;
    case Radians:
      unitLabel = QObject::trUtf8( " rad" );
      break;
    case Gon:
      unitLabel = QObject::trUtf8( " gon" );
      break;
    case MinutesOfArc:
      unitLabel = QObject::trUtf8( "′" );
      break;
    case SecondsOfArc:
      unitLabel = QObject::trUtf8( "″" );
      break;
    case Turn:
      unitLabel = QObject::trUtf8( " tr" );
      break;
    case UnknownAngleUnit:
      break;
  }

  return QString( "%L1%2" ).arg( angle, 0, 'f', decimals ).arg( unitLabel );
}

// enable for QGIS 3.0
#if 0

QString QgsUnitTypes::encodeUnit( QgsSymbolV2::OutputUnit unit )
{
  switch ( unit )
  {
    case QgsSymbolV2::MM:
      return "MM";
    case QgsSymbolV2::MapUnit:
      return "MapUnit";
    case QgsSymbolV2::Pixel:
      return "Pixel";
    case QgsSymbolV2::Percentage:
      return "Percentage";
    default:
      return "MM";
  }
}

QgsSymbolV2::OutputUnit QgsUnitTypes::decodeSymbolUnit( const QString& string, bool* ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( QgsSymbolV2::MM ).toLower() )
    return QgsSymbolV2::MM;
  if ( normalized == encodeUnit( QgsSymbolV2::MapUnit ).toLower() )
    return QgsSymbolV2::MapUnit;
  if ( normalized == encodeUnit( QgsSymbolV2::Pixel ).toLower() )
    return QgsSymbolV2::Pixel;
  if ( normalized == encodeUnit( QgsSymbolV2::Percentage ).toLower() )
    return QgsSymbolV2::Percentage;

  if ( ok )
    *ok = false;

  // millimeters are default
  return QgsSymbolV2::MM;
}

#endif
