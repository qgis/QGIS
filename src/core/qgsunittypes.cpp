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
      return Standard;

    case QGis::Degrees:
      return Geographic;

    case QGis::UnknownUnit:
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

    case QGis::Feet:
      return "feet";

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
    case QGis::Feet:
      return QCoreApplication::translate( "QGis::UnitType", "feet" );

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
  if ( normalized == toString( QGis::Feet ) )
    return QGis::Feet;
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

  // Unify degree units
  // remove for QGIS 3.0, as extra degree types will be removed
  if ( fromUnit == QGis::DecimalDegrees || fromUnit == QGis::DegreesMinutesSeconds || fromUnit == QGis::DegreesDecimalMinutes )
    fromUnit = QGis::Degrees;
  if ( toUnit == QGis::DecimalDegrees || toUnit == QGis::DegreesMinutesSeconds || toUnit == QGis::DegreesDecimalMinutes )
    toUnit = QGis::Degrees;

  // Calculate the conversion factor between the specified units
  if ( fromUnit != toUnit )
  {
    switch ( fromUnit )
    {
      case QGis::Meters:
      {
        if ( toUnit == QGis::Feet ) return 1.0 / FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return 1.0 / DEGREE_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return 1.0 / NMILE_TO_METER;
        break;
      }
      case QGis::Feet:
      {
        if ( toUnit == QGis::Meters ) return FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return FEET_TO_METER / DEGREE_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return FEET_TO_METER / NMILE_TO_METER;
        break;
      }
      case QGis::Degrees:
      {
        if ( toUnit == QGis::Meters ) return DEGREE_TO_METER;
        if ( toUnit == QGis::Feet ) return DEGREE_TO_METER / FEET_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return DEGREE_TO_METER / NMILE_TO_METER;
        break;
      }
      case QGis::NauticalMiles:
      {
        if ( toUnit == QGis::Meters ) return NMILE_TO_METER;
        if ( toUnit == QGis::Feet ) return NMILE_TO_METER / FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return NMILE_TO_METER / DEGREE_TO_METER;
        break;
      }
      case QGis::UnknownUnit:
        break;
    }
  }
  return 1.0;
}

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
