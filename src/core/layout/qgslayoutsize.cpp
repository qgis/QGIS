/***************************************************************************
                         qgslayoutsize.cpp
                         -----------------
    begin                : June 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutsize.h"
#include "qgis.h"
#include <QStringList>

QgsLayoutSize::QgsLayoutSize( const double width, const double height, const QgsUnitTypes::LayoutUnit units )
  : mWidth( width )
  , mHeight( height )
  , mUnits( units )
{
}

QgsLayoutSize::QgsLayoutSize( const QSizeF size, const QgsUnitTypes::LayoutUnit units )
  : mWidth( size.width() )
  , mHeight( size.height() )
  , mUnits( units )
{
}

QgsLayoutSize::QgsLayoutSize( const QgsUnitTypes::LayoutUnit units )
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
  return QgsLayoutSize( parts[0].toDouble(), parts[1].toDouble(), QgsUnitTypes::decodeLayoutUnit( parts[2] ) );

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
