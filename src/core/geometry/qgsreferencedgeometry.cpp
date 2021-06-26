/***************************************************************************
                             qgsreferencedgeometry.cpp
                             ------------------------
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

#include "qgsreferencedgeometry.h"

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

QgsReferencedGeometry QgsReferencedGeometry::fromReferencedPointXY( const QgsReferencedPointXY &point )
{
  return QgsReferencedGeometry( QgsGeometry::fromPointXY( point ), point.crs() );
}

QgsReferencedGeometry QgsReferencedGeometry::fromReferencedRect( const QgsReferencedRectangle &rectangle )
{
  return QgsReferencedGeometry( QgsGeometry::fromRect( rectangle ), rectangle.crs() );
}

