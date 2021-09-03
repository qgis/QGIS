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
#include "qgsrectangle.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"

bool QgsAbstractGeometrySimplifier::isGeneralizableByDeviceBoundingBox( const QgsRectangle &envelope, float mapToPixelTol )
{
  return ( envelope.xMaximum() - envelope.xMinimum() ) < mapToPixelTol && ( envelope.yMaximum() - envelope.yMinimum() ) < mapToPixelTol;
}

bool QgsAbstractGeometrySimplifier::isGeneralizableByDeviceBoundingBox( const QVector<QPointF> &points, float mapToPixelTol )
{
  QgsRectangle r;
  r.setMinimal();

  for ( int i = 0, numPoints = points.size(); i < numPoints; ++i )
  {
    r.combineExtentWith( points[i].x(), points[i].y() );
  }
  return isGeneralizableByDeviceBoundingBox( r, mapToPixelTol );
}

/***************************************************************************/

QgsTopologyPreservingSimplifier::QgsTopologyPreservingSimplifier( double tolerance ) : mTolerance( tolerance )
{
}

QgsGeometry QgsTopologyPreservingSimplifier::simplify( const QgsGeometry &geometry ) const
{
  return geometry.simplify( mTolerance );
}

QgsAbstractGeometry *QgsTopologyPreservingSimplifier::simplify( const QgsAbstractGeometry *geometry ) const
{
  if ( !geometry )
  {
    return nullptr;
  }

  const QgsGeos geos( geometry );
  std::unique_ptr< QgsAbstractGeometry > simplifiedGeom( geos.simplify( mTolerance ) );
  return simplifiedGeom.release();
}

