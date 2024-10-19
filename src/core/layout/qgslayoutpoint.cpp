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
#include "qgsunittypes.h"

#include <QStringList>

QgsLayoutPoint::QgsLayoutPoint( const double x, const double y, const Qgis::LayoutUnit units )
  : mX( x )
  , mY( y )
  , mUnits( units )
{
#ifdef QGISDEBUG
  Q_ASSERT_X( !std::isnan( mX ) && !std::isnan( mY ), "QgsLayoutPoint", "Layout point with NaN coordinates created" );
#endif
}

QgsLayoutPoint::QgsLayoutPoint( const QPointF point, const Qgis::LayoutUnit units )
  : mX( point.x() )
  , mY( point.y() )
  , mUnits( units )
{

}

QgsLayoutPoint::QgsLayoutPoint( const Qgis::LayoutUnit units )
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
  const double x = parts[0].toDouble();
  const double y = parts[1].toDouble();

  // don't restore corrupted coordinates from xml. This can happen when eg a broken item size causes a nan position,
  // which breaks the layout size calculation and results in nan or massive x/y values. Restoring these leads to a broken
  // layout which cannot be interacted with.
  if ( std::isnan( x ) || std::isnan( y ) || x > 9.99998e+06 || y > 9.99998e+06 )
    return QgsLayoutPoint();

  return QgsLayoutPoint( x, y, QgsUnitTypes::decodeLayoutUnit( parts[2] ) );
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
