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
#include "moc_qgsreferencedgeometry.cpp"

QgsReferencedGeometryBase::QgsReferencedGeometryBase( const QgsCoordinateReferenceSystem &crs )
  : mCrs( crs )
{}

QgsReferencedRectangle::QgsReferencedRectangle( const QgsRectangle &rect, const QgsCoordinateReferenceSystem &crs )
  : QgsRectangle( rect )
  , QgsReferencedGeometryBase( crs )
{}

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
