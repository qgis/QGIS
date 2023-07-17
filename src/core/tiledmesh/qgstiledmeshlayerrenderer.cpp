/***************************************************************************
                         qgstiledmeshlayerrenderer.cpp
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
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


#include "qgstiledmeshlayerrenderer.h"
#include "qgscurve.h"
#include "qgstiledmeshboundingvolume.h"
#include "qgstiledmeshlayer.h"
#include "qgsfeedback.h"
#include "qgsmapclippingutils.h"
#include "qgsrendercontext.h"
#include "qgscurvepolygon.h"
#include "qgslinesymbol.h"

QgsTiledMeshLayerRenderer::QgsTiledMeshLayerRenderer( QgsTiledMeshLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mFeedback( new QgsFeedback )
{
  // We must not keep pointer to mLayer (it's dangerous) - we must copy anything we need for rendering
  // or use some locking to prevent read/write from multiple threads
  if ( !layer->dataProvider() ) // || !layer->renderer() )
    return;

  mMeshCrs = layer->dataProvider()->meshCrs();

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );
  if ( const QgsAbstractTiledMeshNodeBoundingVolume *layerBoundingVolume = layer->dataProvider()->boundingVolume() )
    mLayerBoundingVolume.reset( layerBoundingVolume->clone() );

  mReadyToCompose = false;
}

QgsTiledMeshLayerRenderer::~QgsTiledMeshLayerRenderer() = default;

bool QgsTiledMeshLayerRenderer::render()
{
  // Set up the render configuration options
  QPainter *painter = renderContext()->painter();

  QgsScopedQPainterState painterState( painter );
  renderContext()->setPainterFlagsUsingContext( painter );

  if ( !mClippingRegions.empty() )
  {
    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *renderContext(), Qgis::LayerType::VectorTile, needsPainterClipPath );
    if ( needsPainterClipPath )
      renderContext()->painter()->setClipPath( path, Qt::IntersectClip );
  }

  // if the previous layer render was relatively quick (e.g. less than 3 seconds), the we show any previously
  // cached version of the layer during rendering instead of the usual progressive updates
  if ( mRenderTimeHint > 0 && mRenderTimeHint <= MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
  {
    mBlockRenderUpdates = true;
    mElapsedTimer.start();
  }

  if ( mLayerBoundingVolume )
  {
    const QgsCoordinateTransform transform = QgsCoordinateTransform( mMeshCrs, renderContext()->coordinateTransform().destinationCrs(), renderContext()->transformContext() );
    try
    {
      std::unique_ptr< QgsAbstractGeometry > rootBoundsGeometry( mLayerBoundingVolume->as2DGeometry( transform ) );
      if ( QgsCurvePolygon *polygon = qgsgeometry_cast< QgsCurvePolygon * >( rootBoundsGeometry.get() ) )
      {
        QPolygonF rootBoundsPoly = polygon->exteriorRing()->asQPolygonF();

        // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
        rootBoundsPoly.erase( std::remove_if( rootBoundsPoly.begin(), rootBoundsPoly.end(),
                                              []( const QPointF point )
        {
          return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
        } ), rootBoundsPoly.end() );

        QPointF *ptr = rootBoundsPoly.data();
        for ( int i = 0; i < rootBoundsPoly.size(); ++i, ++ptr )
        {
          renderContext()->mapToPixel().transformInPlace( ptr->rx(), ptr->ry() );
        }

        QgsLineSymbol symbol;
        symbol.startRender( *renderContext() );
        symbol.renderPolyline( rootBoundsPoly, nullptr, *renderContext() );
        symbol.stopRender( *renderContext() );
      }
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Error transforming root bounding volume" ) );
    }
  }

  mReadyToCompose = true;
  return false;
}

void QgsTiledMeshLayerRenderer::setLayerRenderingTimeHint( int time )
{
  mRenderTimeHint = time;
}
