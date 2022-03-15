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

#include <QElapsedTimer>

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

QgsVectorTileLayerRenderer::QgsVectorTileLayerRenderer( QgsVectorTileLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mSourceType( layer->sourceType() )
  , mSourcePath( layer->sourcePath() )
  , mRenderer( layer->renderer()->clone() )
  , mDrawTileBoundaries( layer->isTileBorderRenderingEnabled() )
  , mFeedback( new QgsFeedback )
  , mLayerOpacity( layer->opacity() )
  , mTileMatrixSet( layer->tileMatrixSet() )
{

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( layer->source() );
  mAuthCfg = dsUri.authConfigId();
  mHeaders [QStringLiteral( "referer" ) ] = dsUri.param( QStringLiteral( "referer" ) );

  if ( QgsLabelingEngine *engine = context.labelingEngine() )
  {
    if ( layer->labeling() )
    {
      mLabelProvider = layer->labeling()->provider( layer );
      if ( mLabelProvider )
      {
        engine->addProvider( mLabelProvider );
      }
    }
  }

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );
}

bool QgsVectorTileLayerRenderer::render()
{
  QgsRenderContext &ctx = *renderContext();

  if ( ctx.renderingStopped() )
    return false;

  const QgsScopedQPainterState painterState( ctx.painter() );

  if ( !mClippingRegions.empty() )
  {
    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *renderContext(), QgsMapLayerType::VectorTileLayer, needsPainterClipPath );
    if ( needsPainterClipPath )
      renderContext()->painter()->setClipPath( path, Qt::IntersectClip );
  }

  QElapsedTimer tTotal;
  tTotal.start();

  QgsDebugMsgLevel( QStringLiteral( "Vector tiles rendering extent: " ) + ctx.extent().toString( -1 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Vector tiles map scale 1 : %1" ).arg( ctx.rendererScale() ), 2 );

  mTileZoom = mTileMatrixSet.scaleToZoomLevel( ctx.rendererScale() );
  QgsDebugMsgLevel( QStringLiteral( "Vector tiles zoom level: %1" ).arg( mTileZoom ), 2 );

  mTileMatrix = mTileMatrixSet.tileMatrix( mTileZoom );

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

  const bool isAsync = ( mSourceType == QLatin1String( "xyz" ) );

  if ( mSourceType == QLatin1String( "xyz" ) && mSourcePath.contains( QLatin1String( "{usage}" ) ) )
  {
    switch ( renderContext()->rendererUsage() )
    {
      case Qgis::RendererUsage::View:
        mSourcePath.replace( QLatin1String( "{usage}" ), QLatin1String( "view" ) );
        break;
      case Qgis::RendererUsage::Export:
        mSourcePath.replace( QLatin1String( "{usage}" ), QLatin1String( "export" ) );
        break;
      case Qgis::RendererUsage::Unknown:
        mSourcePath.replace( QLatin1String( "{usage}" ), QString() );
        break;
    }
  }

  std::unique_ptr<QgsVectorTileLoader> asyncLoader;
  QList<QgsVectorTileRawData> rawTiles;
  if ( !isAsync )
  {
    QElapsedTimer tFetch;
    tFetch.start();
    rawTiles = QgsVectorTileLoader::blockingFetchTileRawData( mSourceType, mSourcePath, mTileMatrix, viewCenter, mTileRange, mAuthCfg, mHeaders );
    QgsDebugMsgLevel( QStringLiteral( "Tile fetching time: %1" ).arg( tFetch.elapsed() / 1000. ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "Fetched tiles: %1" ).arg( rawTiles.count() ), 2 );
  }
  else
  {
    asyncLoader.reset( new QgsVectorTileLoader( mSourcePath, mTileMatrix, mTileRange, viewCenter, mAuthCfg, mHeaders, mFeedback.get() ) );
    QObject::connect( asyncLoader.get(), &QgsVectorTileLoader::tileRequestFinished, asyncLoader.get(), [this]( const QgsVectorTileRawData & rawTile )
    {
      QgsDebugMsgLevel( QStringLiteral( "Got tile asynchronously: " ) + rawTile.id.toString(), 2 );
      if ( !rawTile.data.isEmpty() )
        decodeAndDrawTile( rawTile );
    } );
  }

  if ( ctx.renderingStopped() )
    return false;

  // add @zoom_level variable which can be used in styling
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Tiles" ) ); // will be deleted by popper
  scope->setVariable( QStringLiteral( "zoom_level" ), mTileZoom, true );
  scope->setVariable( QStringLiteral( "vector_tile_zoom" ), mTileMatrixSet.scaleToZoom( ctx.rendererScale() ), true );
  const QgsExpressionContextScopePopper popper( ctx.expressionContext(), scope );

  mRenderer->startRender( *renderContext(), mTileZoom, mTileRange );

  QMap<QString, QSet<QString> > requiredFields = mRenderer->usedAttributes( ctx );

  if ( mLabelProvider )
  {
    const QMap<QString, QSet<QString> > requiredFieldsLabeling = mLabelProvider->usedAttributes( ctx, mTileZoom );
    for ( auto it = requiredFieldsLabeling.begin(); it != requiredFieldsLabeling.end(); ++it )
    {
      requiredFields[it.key()].unite( it.value() );
    }
  }

  for ( auto it = requiredFields.constBegin(); it != requiredFields.constEnd(); ++it )
    mPerLayerFields[it.key()] = QgsVectorTileUtils::makeQgisFields( it.value() );

  mRequiredLayers = mRenderer->requiredLayers( ctx, mTileZoom );

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
      mRequiredLayers.unite( mLabelProvider->requiredLayers( ctx, mTileZoom ) );
    }
  }

  if ( !isAsync )
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

  mRenderer->stopRender( ctx );

  QgsDebugMsgLevel( QStringLiteral( "Total time for decoding: %1" ).arg( mTotalDecodeTime / 1000. ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Drawing time: %1" ).arg( mTotalDrawTime / 1000. ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Total time: %1" ).arg( tTotal.elapsed() / 1000. ), 2 );

  return !ctx.renderingStopped();
}

bool QgsVectorTileLayerRenderer::forceRasterRender() const
{
  return renderContext()->testFlag( Qgis::RenderContextFlag::UseAdvancedEffects ) && ( !qgsDoubleNear( mLayerOpacity, 1.0 ) );
}

void QgsVectorTileLayerRenderer::decodeAndDrawTile( const QgsVectorTileRawData &rawTile )
{
  QgsRenderContext &ctx = *renderContext();

  QgsDebugMsgLevel( QStringLiteral( "Drawing tile " ) + rawTile.id.toString(), 2 );

  QElapsedTimer tLoad;
  tLoad.start();

  // currently only MVT encoding supported
  QgsVectorTileMVTDecoder decoder( mTileMatrixSet );
  if ( !decoder.decode( rawTile.id, rawTile.data ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Failed to parse raw tile data! " ) + rawTile.id.toString(), 2 );
    return;
  }

  if ( ctx.renderingStopped() )
    return;

  const QgsCoordinateTransform ct = ctx.coordinateTransform();

  QgsVectorTileRendererData tile( rawTile.id );
  tile.setFields( mPerLayerFields );
  tile.setFeatures( decoder.layerFeatures( mPerLayerFields, ct, &mRequiredLayers ) );

  try
  {
    tile.setTilePolygon( QgsVectorTileUtils::tilePolygon( rawTile.id, ct, mTileMatrix, ctx.mapToPixel() ) );
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
#if 0
    ctx.painter()->setBrush( QBrush( QColor( 255, 0, 0 ) ) );
    ctx.painter()->drawText( tile.tilePolygon().boundingRect().center(), tile.id().toString() );
#endif
  }
}
