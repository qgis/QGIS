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
#include "qgstiledmeshlayer.h"
#include "qgsfeedback.h"
#include "qgsmapclippingutils.h"
#include "qgsrendercontext.h"

QgsTiledMeshLayerRenderer::QgsTiledMeshLayerRenderer( QgsTiledMeshLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mFeedback( new QgsFeedback )
{
  // We must not keep pointer to mLayer (it's dangerous) - we must copy anything we need for rendering
  // or use some locking to prevent read/write from multiple threads
  if ( !layer || !layer->dataProvider() ) // || !layer->renderer() )
    return;

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );

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

  mReadyToCompose = true;
  return false;
}

void QgsTiledMeshLayerRenderer::setLayerRenderingTimeHint( int time )
{
  mRenderTimeHint = time;
}
