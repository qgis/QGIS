/***************************************************************************
                         qgslayoutpoint.cpp
                         ------------------
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

#include "qgslayoutpoint.h"
#include "qgis.h"

#include <QStringList>

QgsLayoutPoint::QgsLayoutPoint( const double x, const double y, const QgsUnitTypes::LayoutUnit units )
  : mX( x )
  , mY( y )
  , mUnits( units )
{

}

QgsLayoutPoint::QgsLayoutPoint( const QPointF point, const QgsUnitTypes::LayoutUnit units )
  : mX( point.x() )
  , mY( point.y() )
  , mUnits( units )
{

}

QgsLayoutPoint::QgsLayoutPoint( const QgsUnitTypes::LayoutUnit units )
  : mUnits( units )
{

}

bool QgsLayoutPoint::isNull() const
{
  return qgsDoubleNear( mX, 0 ) && qgsDoubleNear( mY, 0 );
}

QPointF QgsLayoutPoint::toQPointF() const
{
  return QPointF( mX, mY );
}

QString QgsLayoutPoint::encodePoint() const
{
  return QStringLiteral( "%1,%2,%3" ).arg( mX ).arg( mY ).arg( QgsUnitTypes::encodeUnit( mUnits ) );
}

QgsLayoutPoint QgsLayoutPoint::decodePoint( const QString &string )
{
  QStringList parts = string.split( ',' );
  if ( parts.count() != 3 )
  {
    return QgsLayoutPoint();
  }
  return QgsLayoutPoint( parts[0].toDouble(), parts[1].toDouble(), QgsUnitTypes::decodeLayoutUnit( parts[2] ) );
}

bool QgsLayoutPoint::operator==( const QgsLayoutPoint &other ) const
{
  return other.units() == mUnits && qgsDoubleNear( other.x(), mX ) && qgsDoubleNear( other.y(), mY );
}

bool QgsLayoutPoint::operator!=( const QgsLayoutPoint &other ) const
{
  return ( ! operator==( other ) );
}

QgsLayoutPoint QgsLayoutPoint::operator*( const double v ) const
{
  return QgsLayoutPoint( mX * v, mY * v, mUnits );
}

QgsLayoutPoint QgsLayoutPoint::operator*=( const double v )
{
  *this = *this * v;
  return *this;
}

QgsLayoutPoint QgsLayoutPoint::operator/( const double v ) const
{
  return QgsLayoutPoint( mX / v, mY / v, mUnits );
}

QgsLayoutPoint QgsLayoutPoint::operator/=( const double v )
{
  *this = *this / v;
  return *this;
}
