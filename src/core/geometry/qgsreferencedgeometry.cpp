/***************************************************************************
                             qgsreferencedgeometry.cpp
                             ------------------------
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

#include "qgsreferencedgeometry.h"

#include "qgslogger.h"

#include <QString>

#include "moc_qgsreferencedgeometry.cpp"

using namespace Qt::StringLiterals;

QgsReferencedGeometryBase::QgsReferencedGeometryBase( const QgsCoordinateReferenceSystem &crs )
  : mCrs( crs )
{}

QgsReferencedRectangle::QgsReferencedRectangle( const QgsRectangle &rect, const QgsCoordinateReferenceSystem &crs )
  : QgsRectangle( rect )
  , QgsReferencedGeometryBase( crs )
{}

QString QgsReferencedRectangle::toString( int precision ) const // cppcheck-suppress duplInheritedMember
{
  QString rep;

  if ( precision < 0 )
  {
    precision = 0;
    if ( ( width() < 10 || height() < 10 ) && ( width() > 0 && height() > 0 ) )
    {
      precision = static_cast<int>( std::ceil( -1.0 * std::log10( std::min( width(), height() ) ) ) ) + 1;
      // sanity check
      if ( precision > 20 )
        precision = 20;
    }
  }

  if ( isNull() )
    rep = u"Null"_s;
  else
    rep = u"%1,%2,%3,%4 [%5]"_s
          .arg( xMinimum(), 0, 'f', precision )
          .arg( xMaximum(), 0, 'f', precision )
          .arg( yMinimum(), 0, 'f', precision )
          .arg( yMaximum(), 0, 'f', precision )
          .arg( crs().authid() );

  QgsDebugMsgLevel( u"Extents : %1"_s.arg( rep ), 4 );

  return rep;
}

bool QgsReferencedRectangle::operator==( const QgsReferencedRectangle &other ) const
{
  return QgsRectangle::operator==( other ) && crs() == other.crs();
}

bool QgsReferencedRectangle::operator!=( const QgsReferencedRectangle &other ) const
{
  return !( *this == other );
}

QgsReferencedPointXY::QgsReferencedPointXY( const QgsPointXY &point, const QgsCoordinateReferenceSystem &crs )
  : QgsPointXY( point )
  , QgsReferencedGeometryBase( crs )
{}

bool QgsReferencedPointXY::operator==( const QgsReferencedPointXY &other )
{
  return QgsPointXY::operator==( other ) && crs() == other.crs();
}

bool QgsReferencedPointXY::operator!=( const QgsReferencedPointXY &other )
{
  return !( *this == other );
}

QgsReferencedGeometry::QgsReferencedGeometry( const QgsGeometry &geom, const QgsCoordinateReferenceSystem &crs )
  : QgsGeometry( geom )
  , QgsReferencedGeometryBase( crs )
{}

bool QgsReferencedGeometry::operator==( const QgsReferencedGeometry &other ) const
{
  return ( ( this->isNull() && other.isNull() ) || this->equals( other ) ) && crs() == other.crs();
}

bool QgsReferencedGeometry::operator!=( const QgsReferencedGeometry &other ) const
{
  return !( *this == other );
}

QgsReferencedGeometry QgsReferencedGeometry::fromReferencedPointXY( const QgsReferencedPointXY &point )
{
  return QgsReferencedGeometry( QgsGeometry::fromPointXY( point ), point.crs() );
}

QgsReferencedGeometry QgsReferencedGeometry::fromReferencedRect( const QgsReferencedRectangle &rectangle )
{
  return QgsReferencedGeometry( QgsGeometry::fromRect( rectangle ), rectangle.crs() );
}
