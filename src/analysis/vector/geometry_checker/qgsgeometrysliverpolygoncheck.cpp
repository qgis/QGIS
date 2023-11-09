/***************************************************************************
    qgsgeometryareacheck.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrysliverpolygoncheck.h"
#include "qgsfeaturepool.h"

bool QgsGeometrySliverPolygonCheck::checkThreshold( double layerToMapUnits, const QgsAbstractGeometry *geom, double &value ) const
{
  const double maxArea = mMaxArea / ( layerToMapUnits * layerToMapUnits );
  const QgsRectangle bb = geom->boundingBox();
  const double maxDim = std::max( bb.width(), bb.height() );
  const double area = geom->area();
  value = ( maxDim * maxDim ) / area;
  if ( maxArea > 0. && area > maxArea )
  {
    return false;
  }
  return value > mThresholdMapUnits; // the sliver threshold is actually a map unit independent number, just abusing QgsGeometryAreaCheck::mThresholdMapUnits to store it
}
