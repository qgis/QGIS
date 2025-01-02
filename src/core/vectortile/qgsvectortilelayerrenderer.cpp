/***************************************************************************
  qgsvectortilelayerrenderer.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilelayerrenderer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeedback.h"
#include "qgslogger.h"
#include "qgsvectortilemvtdecoder.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortileloader.h"
#include "qgsvectortileutils.h"
#include "qgslabelingengine.h"
#include "qgsvectortilelabeling.h"
#include "qgsmapclippingutils.h"
#include "qgsrendercontext.h"
#include "qgsvectortiledataprovider.h"
#include "qgstextrenderer.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"

#include <QElapsedTimer>
#include <QThread>

QgsVectorTileLayerRenderer::QgsVectorTileLayerRenderer( QgsVectorTileLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mLayerName( layer->name() )
  , mDataProvider( qgis::down_cast< const QgsVectorTileDataProvider* >( layer->dataProvider() )->clone() )
  , mRenderer( layer->renderer()->clone() )
  , mLayerBlendMode( layer->blendMode() )
  , mDrawTileBoundaries( layer->isTileBorderRenderingEnabled() )
  , mLabelsEnabled( layer->labelsEnabled() )
  , mFeedback( new QgsFeedback )
  , mSelectedFeatures( layer->selectedFeatures() )
  , mLayerOpacity( layer->opacity() )
  , mTileMatrixSet( layer->tileMatrixSet() )
  , mEnableProfile( context.flags() & Qgis::RenderContextFlag::RecordProfile )
{
  QElapsedTimer timer;
  timer.start();

  if ( QgsLabelingEngine *engine = context.labelingEngine() )
  {
    if ( layer->labelsEnabled() )
    {
      mLabelProvider = layer->labeling()->provider( layer );
      if ( mLabelProvider )
      {
        engine->addProvider( mLabelProvider );
      }
    }
  }

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );

  mDataProvider->moveToThread( nullptr );

  mPreparationTime = timer.elapsed();
}

QgsVectorTileLayerRenderer::~QgsVectorTileLayerRenderer() = default;

bool QgsVectorTileLayerRenderer::render()
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( mEnableProfile )
  {
    profile = std::make_unique< QgsScopedRuntimeProfile >( mLayerName, QStringLiteral( "rendering" ), layerId() );
    if ( mPreparationTime > 0 )
      QgsApplication::profiler()->record( QObject::tr( "Create renderer" ), mPreparationTime / 1000.0, QStringLiteral( "rendering" ) );
  }

  QgsRenderContext &ctx = *renderContext();

  if ( ctx.renderingStopped() )
    return false;

  std::unique_ptr< QgsScopedRuntimeProfile > preparingProfile;
  if ( mEnableProfile )
  {
    preparingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Preparing render" ), QStringLiteral( "rendering" ) );
  }

  mDataProvider->moveToThread( QThread::currentThread() );

  const QgsScopedQPainterState painterState( ctx.painter() );

  if ( !mClippingRegions.empty() )
  {
    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *renderContext(), Qgis::LayerType::VectorTile, needsPainterClipPath );
    if ( needsPainterClipPath )
      renderContext()->painter()->setClipPath( path, Qt::IntersectClip );
  }

  QElapsedTimer tTotal;
  tTotal.start();

  const double tileRenderScale = mTileMatrixSet.scaleForRenderContext( ctx );
  QgsDebugMsgLevel( QStringLiteral( "Vector tiles rendering extent: " ) + ctx.extent().toString( -1 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Vector tiles map scale 1 : %1" ).arg( tileRenderScale ), 2 );

  mTileZoomToFetch = mTileMatrixSet.scaleToZoomLevel( tileRenderScale );
  QgsDebugMsgLevel( QStringLiteral( "Vector tiles zoom level: %1" ).arg( mTileZoomToFetch ), 2 );
  mTileZoomToRender = mTileMatrixSet.scaleToZoomLevel( tileRenderScale, false );
  QgsDebugMsgLevel( QStringLiteral( "Render zoom level: %1" ).arg( mTileZoomToRender ), 2 );

  mTileMatrix = mTileMatrixSet.tileMatrix( mTileZoomToFetch );

  mTileRange = mTileMatrix.tileRangeFromExtent( ctx.extent() );
  QgsDebugMsgLevel( QStringLiteral( "Vector tiles range X: %1 - %2  Y: %3 - %4" )
                    .arg( mTileRange.startColumn() ).arg( mTileRange.endColumn() )
                    .arg( mTileRange.startRow() ).arg( mTileRange.endRow() ), 2 );

  // view center is used to sort the order of tiles for fetching and rendering
  const QPointF viewCenter = mTileMatrix.mapToTileCoordinates( ctx.extent().center() );

  if ( !mTileRange.isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Vector tiles - outside of range" ), 2 );
    return true;   // nothing to do
  }

  preparingProfile.reset();
  std::unique_ptr< QgsScopedRuntimeProfile > renderingProfile;
  if ( mEnableProfile )
  {
    renderingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering" ), QStringLiteral( "rendering" ) );
  }

  std::unique_ptr<QgsVectorTileLoader> asyncLoader;
  QList<QgsVectorTileRawData> rawTiles;
  if ( !mDataProvider->supportsAsync() )
  {
    QElapsedTimer tFetch;
    tFetch.start();
    rawTiles = QgsVectorTileLoader::blockingFetchTileRawData( mDataProvider.get(), mTileMatrixSet, viewCenter, mTileRange, mTileZoomToFetch, mFeedback.get() );
    QgsDebugMsgLevel( QStringLiteral( "Tile fetching time: %1" ).arg( tFetch.elapsed() / 1000. ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "Fetched tiles: %1" ).arg( rawTiles.count() ), 2 );
  }
  else
  {
    asyncLoader.reset( new QgsVectorTileLoader( mDataProvider.get(), mTileMatrixSet, mTileRange, mTileZoomToFetch, viewCenter, mFeedback.get(), renderContext()->rendererUsage() ) );
    QObject::connect( asyncLoader.get(), &QgsVectorTileLoader::tileRequestFinished, asyncLoader.get(), [this]( const QgsVectorTileRawData & rawTile )
    {
      QgsDebugMsgLevel( QStringLiteral( "Got tile asynchronously: " ) + rawTile.id.toString(), 2 );
      if ( !rawTile.data.isEmpty() )
        decodeAndDrawTile( rawTile );
    } );
  }

  if ( ctx.renderingStopped() )
    return false;

  // add @zoom_level and @vector_tile_zoom variables which can be used in styling
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Tiles" ) ); // will be deleted by popper
  scope->setVariable( QStringLiteral( "zoom_level" ), mTileZoomToRender, true );
  scope->setVariable( QStringLiteral( "vector_tile_zoom" ), mTileMatrixSet.scaleToZoom( tileRenderScale ), true );
  const QgsExpressionContextScopePopper popper( ctx.expressionContext(), scope );

  mRenderer->startRender( *renderContext(), mTileZoomToRender, mTileRange );

  // Draw background style if present
  mRenderer->renderBackground( ctx );

  QMap<QString, QSet<QString> > requiredFields = mRenderer->usedAttributes( ctx );

  if ( mLabelProvider )
  {
    const QMap<QString, QSet<QString> > requiredFieldsLabeling = mLabelProvider->usedAttributes( ctx, mTileZoomToRender );
    for ( auto it = requiredFieldsLabeling.begin(); it != requiredFieldsLabeling.end(); ++it )
    {
      requiredFields[it.key()].unite( it.value() );
    }
  }

  for ( auto it = requiredFields.constBegin(); it != requiredFields.constEnd(); ++it )
    mPerLayerFields[it.key()] = QgsVectorTileUtils::makeQgisFields( it.value() );

  mRequiredLayers = mRenderer->requiredLayers( ctx, mTileZoomToRender );

  if ( mLabelProvider )
  {
    mLabelProvider->setFields( mPerLayerFields );
    QSet<QString> attributeNames;  // we don't need this - already got referenced columns in provider constructor
    if ( !mLabelProvider->prepare( ctx, attributeNames ) )
    {
      ctx.labelingEngine()->removeProvider( mLabelProvider );
      mLabelProvider = nullptr; // provider is deleted by the engine
    }
    else
    {
      mRequiredLayers.unite( mLabelProvider->requiredLayers( ctx, mTileZoomToRender ) );
    }
  }

  if ( !mDataProvider->supportsAsync() )
  {
    for ( const QgsVectorTileRawData &rawTile : std::as_const( rawTiles ) )
    {
      if ( ctx.renderingStopped() )
        break;

      decodeAndDrawTile( rawTile );
    }
  }
  else
  {
    // Block until tiles are fetched and rendered. If the rendering gets canceled at some point,
    // the async loader will catch the signal, abort requests and return from downloadBlocking()
    asyncLoader->downloadBlocking();
    if ( !asyncLoader->error().isEmpty() )
      mErrors.append( asyncLoader->error() );
  }

  if ( ctx.flags() & Qgis::RenderContextFlag::DrawSelection )
    mRenderer->renderSelectedFeatures( mSelectedFeatures, ctx );

  mRenderer->stopRender( ctx );

  QgsDebugMsgLevel( QStringLiteral( "Total time for decoding: %1" ).arg( mTotalDecodeTime / 1000. ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Drawing time: %1" ).arg( mTotalDrawTime / 1000. ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Total time: %1" ).arg( tTotal.elapsed() / 1000. ), 2 );

  return !ctx.renderingStopped();
}

bool QgsVectorTileLayerRenderer::forceRasterRender() const
{
  if ( !renderContext()->testFlag( Qgis::RenderContextFlag::UseAdvancedEffects ) )
    return false;

  if ( !qgsDoubleNear( mLayerOpacity, 1.0 ) )
    return true;

  if ( mLayerBlendMode != QPainter::CompositionMode_SourceOver )
    return true;

  return false;
}

void QgsVectorTileLayerRenderer::decodeAndDrawTile( const QgsVectorTileRawData &rawTile )
{
  QgsRenderContext &ctx = *renderContext();

  QgsDebugMsgLevel( QStringLiteral( "Drawing tile " ) + rawTile.id.toString(), 2 );

  QElapsedTimer tLoad;
  tLoad.start();

  // currently only MVT encoding supported
  QgsVectorTileMVTDecoder decoder( mTileMatrixSet );
  if ( !decoder.decode( rawTile ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Failed to parse raw tile data! " ) + rawTile.id.toString(), 2 );
    return;
  }

  if ( ctx.renderingStopped() )
    return;

  const QgsCoordinateTransform ct = ctx.coordinateTransform();

  QgsVectorTileRendererData tile( rawTile.id );
  tile.setRenderZoomLevel( mTileZoomToRender );

  tile.setFields( mPerLayerFields );
  tile.setFeatures( decoder.layerFeatures( mPerLayerFields, ct, &mRequiredLayers ) );

  try
  {
    tile.setTilePolygon( QgsVectorTileUtils::tilePolygon( rawTile.id, ct, mTileMatrixSet.tileMatrix( rawTile.id.zoomLevel() ), ctx.mapToPixel() ) );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsgLevel( QStringLiteral( "Failed to generate tile polygon " ) + rawTile.id.toString(), 2 );
    return;
  }

  mTotalDecodeTime += tLoad.elapsed();

  // calculate tile polygon in screen coordinates

  if ( ctx.renderingStopped() )
    return;

  {
    // set up clipping so that rendering does not go behind tile's extent
    const QgsScopedQPainterState savePainterState( ctx.painter() );
    // we have to intersect with any existing painter clip regions, or we risk overwriting valid clip
    // regions setup outside of the vector tile renderer (e.g. layout map clip region)
    ctx.painter()->setClipRegion( QRegion( tile.tilePolygon() ), Qt::IntersectClip );

    QElapsedTimer tDraw;
    tDraw.start();

    mRenderer->renderTile( tile, ctx );
    mTotalDrawTime += tDraw.elapsed();
  }

  if ( mLabelProvider )
    mLabelProvider->registerTileFeatures( tile, ctx );

  if ( mDrawTileBoundaries )
  {
    const QgsScopedQPainterState savePainterState( ctx.painter() );
    ctx.painter()->setClipping( false );

    QPen pen( Qt::red );
    pen.setWidth( 3 );
    QBrush brush( QColor( 255, 0, 0, 40 ), Qt::BrushStyle::Dense3Pattern );

    ctx.painter()->setPen( pen );
    ctx.painter()->setBrush( brush );
    ctx.painter()->drawPolygon( tile.tilePolygon() );
#if 1
    QgsTextFormat format;
    format.setColor( QColor( 255, 0, 0 ) );
    format.buffer().setEnabled( true );

    QgsTextRenderer::drawText( QRectF( QPoint( 0, 0 ), ctx.outputSize() ).intersected( tile.tilePolygon().boundingRect() ),
                               0, Qgis::TextHorizontalAlignment::Center, { tile.id().toString() },
                               ctx, format, true, Qgis::TextVerticalAlignment::VerticalCenter );
#endif
  }
}
