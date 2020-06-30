/***************************************************************************
  qgsmapclippingutils.cpp
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapclippingutils.h"
#include "qgsgeometry.h"
#include "qgsrendercontext.h"
#include "qgsmapclippingregion.h"
#include "qgslogger.h"
#include <algorithm>

QList<QgsMapClippingRegion> QgsMapClippingUtils::collectClippingRegionsForLayer( const QgsRenderContext &context, const QgsMapLayer *layer )
{
  QList< QgsMapClippingRegion > res;
  const QList< QgsMapClippingRegion > regions = context.clippingRegions();
  res.reserve( regions.size() );

  std::copy_if( regions.begin(), regions.end(), std::back_inserter( res ), [layer]( const QgsMapClippingRegion & region )
  {
    return region.appliesToLayer( layer );
  } );

  return res;
}

QgsGeometry QgsMapClippingUtils::calculateFeatureRequestGeometry( const QList< QgsMapClippingRegion > &regions, const QgsRenderContext &context, bool &shouldFilter )
{
  QgsGeometry result;
  bool first = true;
  shouldFilter = false;
  for ( const QgsMapClippingRegion &region : regions )
  {
    if ( region.geometry().type() != QgsWkbTypes::PolygonGeometry )
      continue;

    shouldFilter = true;
    if ( first )
    {
      result = region.geometry();
      first = false;
    }
    else
    {
      result = result.intersection( region.geometry() );
    }
  }

  // filter out polygon parts from result only
  result.convertGeometryCollectionToSubclass( QgsWkbTypes::PolygonGeometry );

  // lastly transform back to layer CRS
  try
  {
    result.transform( context.coordinateTransform(), QgsCoordinateTransform::ReverseTransform );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Could not transform clipping region to layer CRS" ) );
    shouldFilter = false;
    return QgsGeometry();
  }

  return result;
}
