/***************************************************************************
                         qgslayoutmeasurement.cpp
                         --------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutmeasurement.h"

#include <QStringList>

QgsLayoutMeasurement::QgsLayoutMeasurement( const double length, const QgsUnitTypes::LayoutUnit units )
  : mLength( length )
  , mUnits( units )
{
}

QString QgsLayoutMeasurement::encodeMeasurement() const
{
  return QStringLiteral( "%1,%2" ).arg( mLength ).arg( QgsUnitTypes::encodeUnit( mUnits ) );
}

QgsLayoutMeasurement QgsLayoutMeasurement::decodeMeasurement( const QString &string )
{
  QStringList parts = string.split( ',' );
  if ( parts.count() != 2 )
  {
    return QgsLayoutMeasurement( 0 );
  }
  return QgsLayoutMeasurement( parts[0].toDouble(), QgsUnitTypes::decodeLayoutUnit( parts[1] ) );
}

bool QgsLayoutMeasurement::operator==( const QgsLayoutMeasurement other ) const
{
  return other.units() == mUnits && qgsDoubleNear( other.length(), mLength );
}

bool QgsLayoutMeasurement::operator!=( const QgsLayoutMeasurement other ) const
{
  return ( ! operator==( other ) );
}

QgsLayoutMeasurement QgsLayoutMeasurement::operator+( const double v ) const
{
  return QgsLayoutMeasurement( mLength + v, mUnits );
}

QgsLayoutMeasurement QgsLayoutMeasurement::operator+=( const double v )
{
  *this = *this + v;
  return *this;
}

QgsLayoutMeasurement QgsLayoutMeasurement::operator-( const double v ) const
{
  return QgsLayoutMeasurement( mLength - v, mUnits );
}

QgsLayoutMeasurement QgsLayoutMeasurement::operator-=( const double v )
{
  *this = *this - v;
  return *this;
}

QgsLayoutMeasurement QgsLayoutMeasurement::operator*( const double v ) const
{
  return QgsLayoutMeasurement( mLength * v, mUnits );
}

QgsLayoutMeasurement QgsLayoutMeasurement::operator*=( const double v )
{
  *this = *this * v;
  return *this;
}

QgsLayoutMeasurement QgsLayoutMeasurement::operator/( const double v ) const
{
  return QgsLayoutMeasurement( mLength / v, mUnits );
}

QgsLayoutMeasurement QgsLayoutMeasurement::operator/=( const double v )
{
  *this = *this / v;
  return *this;
}
