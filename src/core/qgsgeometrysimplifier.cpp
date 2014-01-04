/***************************************************************************
    qgsgeometrysimplifier.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include "qgsgeometrysimplifier.h"

QgsAbstractGeometrySimplifier::~QgsAbstractGeometrySimplifier()
{
}

//! Returns whether the device-envelope can be replaced by its BBOX when is applied the specified tolerance
bool QgsAbstractGeometrySimplifier::canbeGeneralizedByDeviceBoundingBox( const QgsRectangle& envelope, float mapToPixelTol )
{
  return ( envelope.xMaximum() - envelope.xMinimum() ) < mapToPixelTol && ( envelope.yMaximum() - envelope.yMinimum() ) < mapToPixelTol;
}

//! Returns whether the device-geometry can be replaced by its BBOX when is applied the specified tolerance
bool QgsAbstractGeometrySimplifier::canbeGeneralizedByDeviceBoundingBox( const QVector<QPointF>& points, float mapToPixelTol )
{
  double xmin =  std::numeric_limits<double>::max(), x, y;
  double ymin =  std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  for ( int i = 0, numPoints = points.size(); i < numPoints; ++i )
  {
    x = points[i].x();
    y = points[i].y();

    if ( xmin > x ) xmin = x;
    if ( ymin > y ) ymin = y;
    if ( xmax < x ) xmax = x;
    if ( ymax < y ) ymax = y;
  }
  return canbeGeneralizedByDeviceBoundingBox( QgsRectangle( xmin, ymin, xmax, ymax ), mapToPixelTol );
}

/***************************************************************************/
/**
 * Implementation of GeometrySimplifier using the Douglas-Peucker algorithm
 */
QgsTopologyPreservingSimplifier::QgsTopologyPreservingSimplifier( double tolerance ) : mTolerance( tolerance )
{
}
QgsTopologyPreservingSimplifier::~QgsTopologyPreservingSimplifier()
{
}

//! Returns a simplified version the specified geometry
QgsGeometry* QgsTopologyPreservingSimplifier::simplify( QgsGeometry* geometry ) const
{
  return geometry->simplify( mTolerance );
}

//! Simplifies the specified geometry
bool QgsTopologyPreservingSimplifier::simplifyGeometry( QgsGeometry* geometry ) const
{
  QgsGeometry* g = geometry->simplify( mTolerance );

  if ( g )
  {
    size_t wkbSize = g->wkbSize();
    unsigned char* wkb = ( unsigned char* )malloc( wkbSize );
    memcpy( wkb, g->asWkb(), wkbSize );
    geometry->fromWkb( wkb, wkbSize );
    delete g;

    return true;
  }
  return false;
}
