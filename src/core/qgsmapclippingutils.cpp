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
#include <QPointer>

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
    if ( region.geometry().type() != Qgis::GeometryType::Polygon )
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

  if ( !shouldFilter )
    return QgsGeometry();

  // filter out polygon parts from result only
  result.convertGeometryCollectionToSubclass( Qgis::GeometryType::Polygon );

  // lastly transform back to layer CRS
  try
  {
    result.transform( context.coordinateTransform(), Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Could not transform clipping region to layer CRS" ) );
    shouldFilter = false;
    return QgsGeometry();
  }

  return result;
}

QgsGeometry QgsMapClippingUtils::calculateFeatureIntersectionGeometry( const QList<QgsMapClippingRegion> &regions, const QgsRenderContext &context, bool &shouldClip )
{
  QgsGeometry result;
  bool first = true;
  shouldClip = false;
  for ( const QgsMapClippingRegion &region : regions )
  {
    if ( region.geometry().type() != Qgis::GeometryType::Polygon )
      continue;

    if ( region.featureClip() != QgsMapClippingRegion::FeatureClippingType::ClipToIntersection )
      continue;

    shouldClip = true;
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

  if ( !shouldClip )
    return QgsGeometry();

  // filter out polygon parts from result only
  result.convertGeometryCollectionToSubclass( Qgis::GeometryType::Polygon );

  // lastly transform back to layer CRS
  try
  {
    result.transform( context.coordinateTransform(), Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Could not transform clipping region to layer CRS" ) );
    shouldClip = false;
    return QgsGeometry();
  }

  return result;
}

QPainterPath QgsMapClippingUtils::calculatePainterClipRegion( const QList<QgsMapClippingRegion> &regions, const QgsRenderContext &context, Qgis::LayerType layerType, bool &shouldClip )
{
  QgsGeometry result;
  bool first = true;
  shouldClip = false;
  for ( const QgsMapClippingRegion &region : regions )
  {
    if ( region.geometry().type() != Qgis::GeometryType::Polygon )
      continue;

    switch ( layerType )
    {
      case Qgis::LayerType::Vector:
        if ( region.featureClip() != QgsMapClippingRegion::FeatureClippingType::ClipPainterOnly )
          continue;
        break;

      case Qgis::LayerType::VectorTile:
        // for now, we ignore the region's featureClip behavior when rendering vector tiles
        // TODO: ideally we should apply this during rendering, just like we do for normal
        // vector layers
        break;

      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::Raster:
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
        // for these layer types, we ignore the region's featureClip behavior.
        break;

    }

    shouldClip = true;
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

  if ( !shouldClip )
    return QPainterPath();

  // transform to painter coordinates
  result.mapToPixel( context.mapToPixel() );

  return result.constGet()->asQPainterPath();
}

QgsGeometry QgsMapClippingUtils::calculateLabelIntersectionGeometry( const QList<QgsMapClippingRegion> &regions, const QgsRenderContext &context, bool &shouldClip )
{
  QgsGeometry result;
  bool first = true;
  shouldClip = false;
  for ( const QgsMapClippingRegion &region : regions )
  {
    if ( region.geometry().type() != Qgis::GeometryType::Polygon )
      continue;

    // for labeling, we clip using either painter clip regions or intersects type regions.
    // unlike feature rendering, we clip features to painter clip regions for labeling, because
    // we want the label to sit within the clip region if possible
    if ( region.featureClip() != QgsMapClippingRegion::FeatureClippingType::ClipPainterOnly &&
         region.featureClip() != QgsMapClippingRegion::FeatureClippingType::ClipToIntersection )
      continue;

    shouldClip = true;
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

  if ( !shouldClip )
    return QgsGeometry();

  // filter out polygon parts from result only
  result.convertGeometryCollectionToSubclass( Qgis::GeometryType::Polygon );

  // lastly transform back to layer CRS
  try
  {
    result.transform( context.coordinateTransform(), Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Could not transform clipping region to layer CRS" ) );
    shouldClip = false;
    return QgsGeometry();
  }

  return result;
}
