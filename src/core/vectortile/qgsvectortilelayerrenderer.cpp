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

#include "qgsvectortilemvtdecoder.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortileloader.h"
#include "qgsvectortileutils.h"


QgsVectorTileLayerRenderer::QgsVectorTileLayerRenderer( QgsVectorTileLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mSourceType( layer->sourceType() )
  , mSourcePath( layer->sourcePath() )
  , mSourceMinZoom( layer->sourceMinZoom() )
  , mSourceMaxZoom( layer->sourceMaxZoom() )
  , mRenderer( layer->renderer()->clone() )
  , mDrawTileBoundaries( layer->isTileBorderRenderingEnabled() )
  , mFeedback( new QgsFeedback )
{
}

bool QgsVectorTileLayerRenderer::render()
{
  QgsRenderContext &ctx = *renderContext();

  if ( ctx.renderingStopped() )
    return false;

  QElapsedTimer tTotal;
  tTotal.start();

  qDebug() << "MVT rend" << ctx.extent().toString( -1 );

  mTileZoom = QgsVectorTileUtils::scaleToZoomLevel( ctx.rendererScale(), mSourceMinZoom, mSourceMaxZoom );
  qDebug() << "MVT zoom level" << mTileZoom;

  mTileMatrix = QgsTileMatrix::fromWebMercator( mTileZoom );

  mTileRange = mTileMatrix.tileRangeFromExtent( ctx.extent() );
  qDebug() << "MVT tile range" << mTileRange.startColumn() << mTileRange.endColumn() << " | " << mTileRange.startRow() << mTileRange.endRow();

  // view center is used to sort the order of tiles for fetching and rendering
  QPointF viewCenter = mTileMatrix.mapToTileCoordinates( ctx.extent().center() );

  if ( !mTileRange.isValid() )
  {
    qDebug() << "outside of range";
    return true;   // nothing to do
  }

  bool isAsync = ( mSourceType == "xyz" );

  std::unique_ptr<QgsVectorTileLoader> asyncLoader;
  QList<QgsVectorTileRawData> rawTiles;
  if ( !isAsync )
  {
    QElapsedTimer tFetch;
    tFetch.start();
    rawTiles = QgsVectorTileLoader::blockingFetchTileRawData( mSourceType, mSourcePath, mTileZoom, viewCenter, mTileRange );
    qDebug() << "FETCH TIME" << tFetch.elapsed() / 1000.;
    qDebug() << "fetched tiles:" << rawTiles.count();
  }
  else
  {
    asyncLoader.reset( new QgsVectorTileLoader( mSourcePath, mTileZoom, mTileRange, viewCenter, mFeedback.get() ) );
    QObject::connect( asyncLoader.get(), &QgsVectorTileLoader::tileRequestFinished, [this]( const QgsVectorTileRawData & rawTile )
    {
      qDebug() << "got async tile" << rawTile.id.column() << rawTile.id.row() << rawTile.id.zoomLevel();
      if ( !rawTile.data.isEmpty() )
        decodeAndDrawTile( rawTile );
    } );
  }

  if ( ctx.renderingStopped() )
    return false;

  mRenderer->startRender( *renderContext(), mTileZoom, mTileRange );

  QMap<QString, QSet<QString> > requiredFields = mRenderer->usedAttributes( *renderContext() );

  QMap<QString, QgsFields> perLayerFields;
  for ( QString layerName : requiredFields.keys() )
    mPerLayerFields[layerName] = QgsVectorTileUtils::makeQgisFields( requiredFields[layerName] );

  if ( !isAsync )
  {
    for ( QgsVectorTileRawData &rawTile : rawTiles )
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
  }

  mRenderer->stopRender( ctx );

  ctx.painter()->setClipping( false );

  qDebug() << "DECODE TIME" << mTotalDecodeTime / 1000.;
  qDebug() << "DRAW TIME" << mTotalDrawTime / 1000.;
  qDebug() << "TOTAL TIME" << tTotal.elapsed() / 1000.;

  return !ctx.renderingStopped();
}

void QgsVectorTileLayerRenderer::decodeAndDrawTile( const QgsVectorTileRawData &rawTile )
{
  QgsRenderContext &ctx = *renderContext();

  qDebug() << "decoding tile " << rawTile.id.zoomLevel() << rawTile.id.column() << rawTile.id.row();

  QElapsedTimer tLoad;
  tLoad.start();

  // currently only MVT encoding supported
  QgsVectorTileMVTDecoder decoder;
  if ( !decoder.decode( rawTile.id, rawTile.data ) )
  {
    qDebug() << "Failed to parse raw tile data!";
    return;
  }

  if ( ctx.renderingStopped() )
    return;

  QgsVectorTileRendererData tile( rawTile.id );
  tile.setFeatures( decoder.layerFeatures( mPerLayerFields ) );
  tile.setTilePolygon( QgsVectorTileUtils::tilePolygon( rawTile.id, mTileMatrix, ctx.mapToPixel() ) );

  mTotalDecodeTime += tLoad.elapsed();

  // calculate tile polygon in screen coordinates

  if ( ctx.renderingStopped() )
    return;

  // set up clipping so that rendering does not go behind tile's extent

  ctx.painter()->setClipRegion( QRegion( tile.tilePolygon() ) );

  qDebug() << "drawing tile" << tile.id().zoomLevel() << tile.id().column() << tile.id().row();

  QElapsedTimer tDraw;
  tDraw.start();

  mRenderer->renderTile( tile, ctx );
  mTotalDrawTime += tDraw.elapsed();

  if ( mDrawTileBoundaries )
  {
    ctx.painter()->setClipping( false );

    QPen pen( Qt::red );
    pen.setWidth( 3 );
    ctx.painter()->setPen( pen );
    ctx.painter()->drawPolygon( tile.tilePolygon() );
  }
}
