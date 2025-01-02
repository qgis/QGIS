/***************************************************************************
                         qgslayoutsize.cpp
                         -----------------
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

#include "qgslayoutsize.h"
#include "qgis.h"
#include "qgsunittypes.h"
#include <QStringList>

QgsLayoutSize::QgsLayoutSize( const double width, const double height, const Qgis::LayoutUnit units )
  : mWidth( width )
  , mHeight( height )
  , mUnits( units )
{
#ifdef QGISDEBUG
  Q_ASSERT_X( !std::isnan( width ) && !std::isnan( height ), "QgsLayoutSize", "Layout size with NaN dimensions created" );
#endif
}

QgsLayoutSize::QgsLayoutSize( const QSizeF size, const Qgis::LayoutUnit units )
  : mWidth( size.width() )
  , mHeight( size.height() )
  , mUnits( units )
{
}

QgsLayoutSize::QgsLayoutSize( const Qgis::LayoutUnit units )
  : mUnits( units )
{

}

bool QgsLayoutSize::isEmpty() const
{
  return qgsDoubleNear( mWidth, 0 ) && qgsDoubleNear( mHeight, 0 );
}

QSizeF QgsLayoutSize::toQSizeF() const
{
  return QSizeF( mWidth, mHeight );
}

QString QgsLayoutSize::encodeSize() const
{
  return QStringLiteral( "%1,%2,%3" ).arg( mWidth ).arg( mHeight ).arg( QgsUnitTypes::encodeUnit( mUnits ) );
}

QgsLayoutSize QgsLayoutSize::decodeSize( const QString &string )
{
  QStringList parts = string.split( ',' );
  if ( parts.count() != 3 )
  {
    return QgsLayoutSize();
  }

  const double width = parts[0].toDouble();
  const double height = parts[1].toDouble();
  if ( std::isnan( width ) || std::isnan( height ) )
    return QgsLayoutSize();

  return QgsLayoutSize( width, height, QgsUnitTypes::decodeLayoutUnit( parts[2] ) );
}

bool QgsLayoutSize::operator==( const QgsLayoutSize &other ) const
{
  return other.units() == mUnits && qgsDoubleNear( other.width(), mWidth ) && qgsDoubleNear( other.height(), mHeight );
}

bool QgsLayoutSize::operator!=( const QgsLayoutSize &other ) const
{
  return ( ! operator==( other ) );
}

QgsLayoutSize QgsLayoutSize::operator*( const double v ) const
{
  return QgsLayoutSize( mWidth * v, mHeight * v, mUnits );
}

QgsLayoutSize QgsLayoutSize::operator*=( const double v )
{
  *this = *this * v;
  return *this;
}

QgsLayoutSize QgsLayoutSize::operator/( const double v ) const
{
  return QgsLayoutSize( mWidth / v, mHeight / v, mUnits );
}

QgsLayoutSize QgsLayoutSize::operator/=( const double v )
{
  *this = *this / v;
  return *this;
}
