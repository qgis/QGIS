/***************************************************************************
    qgsvectorfieldwindbarbsettings.cpp
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

#include "qgsvectorfieldwindbarbsettings.h"

#include <QString>

using namespace Qt::StringLiterals;

void QgsVectorFieldWindBarbSettings::readXml( const QDomElement &elem )
{
  mShaftLength = elem.attribute( u"shaft-length"_s, u"10"_s ).toDouble();
  mShaftLengthUnits = static_cast<Qgis::RenderUnit>( elem.attribute( u"shaft-length-units"_s ).toInt() );
  mMagnitudeMultiplier = elem.attribute( u"magnitude-multiplier"_s, u"1"_s ).toDouble();
  mMagnitudeUnits = static_cast<WindSpeedUnit>( elem.attribute( u"magnitude-units"_s, u"0"_s ).toInt() );
}

QDomElement QgsVectorFieldWindBarbSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"vector-windbarb-settings"_s );
  elem.setAttribute( u"shaft-length"_s, mShaftLength );
  elem.setAttribute( u"shaft-length-units"_s, static_cast< int >( mShaftLengthUnits ) );
  elem.setAttribute( u"magnitude-multiplier"_s, mMagnitudeMultiplier );
  elem.setAttribute( u"magnitude-units"_s, static_cast< int >( mMagnitudeUnits ) );
  return elem;
}

double QgsVectorFieldWindBarbSettings::magnitudeMultiplier() const
{
  switch ( mMagnitudeUnits )
  {
    case QgsVectorFieldWindBarbSettings::WindSpeedUnit::Knots:
      return 1.0;
    case QgsVectorFieldWindBarbSettings::WindSpeedUnit::MetersPerSecond:
      return 3600.0 / 1852.0;
    case QgsVectorFieldWindBarbSettings::WindSpeedUnit::KilometersPerHour:
      return 1.0 / 1.852;
    case QgsVectorFieldWindBarbSettings::WindSpeedUnit::MilesPerHour:
      return 1.609344 / 1.852;
    case QgsVectorFieldWindBarbSettings::WindSpeedUnit::FeetPerSecond:
      return 3600.0 / 1.852 / 5280.0 * 1.609344;
    case QgsVectorFieldWindBarbSettings::WindSpeedUnit::OtherUnit:
      return mMagnitudeMultiplier;
  }
  return 1.0; // should not reach
}

void QgsVectorFieldWindBarbSettings::setMagnitudeMultiplier( double magnitudeMultiplier )
{
  mMagnitudeMultiplier = magnitudeMultiplier;
}

double QgsVectorFieldWindBarbSettings::shaftLength() const
{
  return mShaftLength;
}

void QgsVectorFieldWindBarbSettings::setShaftLength( double shaftLength )
{
  mShaftLength = shaftLength;
}

Qgis::RenderUnit QgsVectorFieldWindBarbSettings::shaftLengthUnits() const
{
  return mShaftLengthUnits;
}

void QgsVectorFieldWindBarbSettings::setShaftLengthUnits( Qgis::RenderUnit shaftLengthUnit )
{
  mShaftLengthUnits = shaftLengthUnit;
}

QgsVectorFieldWindBarbSettings::WindSpeedUnit QgsVectorFieldWindBarbSettings::magnitudeUnits() const
{
  return mMagnitudeUnits;
}

void QgsVectorFieldWindBarbSettings::setMagnitudeUnits( WindSpeedUnit units )
{
  mMagnitudeUnits = units;
}
