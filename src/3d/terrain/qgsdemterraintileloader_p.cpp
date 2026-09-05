/***************************************************************************
  qgsdemterraintileloader_p.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdemterraintileloader_p.h"

#include <limits>
#include <memory>

#include "qgs3dmapsettings.h"
#include "qgschunknode.h"
#include "qgsdemterraingenerator.h"
#include "qgseventtracing.h"
#include "qgsonlineterraingenerator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterprojector.h"
#include "qgsterraindownloader.h"
#include "qgsterraingenerator.h"
#include "qgsterraintexturegenerator_p.h"

#include <QFutureWatcher>
#include <QMutexLocker>
#include <QString>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>
#include <QtConcurrentRun>

#include "moc_qgsdemterraintileloader_p.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QgsDemHeightMapGenerator::QgsDemHeightMapGenerator( QgsRasterLayer *dtm, const QgsTilingScheme &tilingScheme, int resolution, const QgsCoordinateTransformContext &transformContext )
  : mDtmExtent( dtm ? dtm->extent() : QgsRectangle() )
  , mClonedProvider( dtm ? qgis::down_cast<QgsRasterDataProvider *>( dtm->dataProvider()->clone() ) : nullptr )
  , mTilingScheme( tilingScheme )
  , mResolution( resolution )
  , mDownloader( dtm ? nullptr : new QgsTerrainDownloader( transformContext ) )
  , mTransformContext( transformContext )
{}

QgsDemHeightMapGenerator::~QgsDemHeightMapGenerator()
{}


static QByteArray readDtmData( QgsRasterDataProvider *provider, const QgsRectangle &extent, int res, const QgsCoordinateReferenceSystem &destCrs, const QgsRectangle &clippingExtent )
{
  provider->moveToThread( QThread::currentThread() );

  QgsScopedEvent e( u"3D"_s, u"DEM"_s );

  // TODO: use feedback object? (but GDAL currently does not support cancellation anyway)
  QgsRasterInterface *input = provider;
  std::unique_ptr<QgsRasterProjector> projector;
  if ( provider->crs() != destCrs )
  {
    projector = std::make_unique<QgsRasterProjector>();
    projector->setCrs( provider->crs(), destCrs, provider->transformContext() );
    projector->setInput( provider );
    input = projector.get();
  }
  std::unique_ptr<QgsRasterBlock> block( input->block( 1, extent, res, res ) );

  QByteArray data;
  if ( block )
  {
    block->convert( Qgis::DataType::Float32 ); // currently we expect just floats

    // set noData outside our clippingExtent
    const QRect subRect = QgsRasterBlock::subRect( extent, block->width(), block->height(), clippingExtent );
    if ( !block->hasNoDataValue() )
    {
      // QgsRasterBlock::setIsNoDataExcept() misbehaves without a defined no data value
      // see https://github.com/qgis/QGIS/issues/51285
      block->setNoDataValue( std::numeric_limits<float>::lowest() );
    }
    block->setIsNoDataExcept( subRect );

    data = block->data();
    data.detach(); // this should make a deep copy

    if ( block->hasNoData() )
    {
      // turn all no-data values into NaN in the output array
      float *floatData = reinterpret_cast<float *>( data.data() );
      Q_ASSERT( data.count() % sizeof( float ) == 0 );
      int count = data.count() / sizeof( float );
      for ( int i = 0; i < count; ++i )
      {
        if ( block->isNoData( i ) )
          floatData[i] = std::numeric_limits<float>::quiet_NaN();
      }
    }
  }

  delete provider;
  return data;
}

static QByteArray readOnlineDtm( QgsTerrainDownloader *downloader, const QgsRectangle &extent, int res, const QgsCoordinateReferenceSystem &destCrs, const QgsCoordinateTransformContext &context )
{
  return downloader->getHeightMap( extent, res, destCrs, context );
}

QFuture<QByteArray> QgsDemHeightMapGenerator::render( const QgsChunkNodeId &nodeId )
{
  QgsEventTracing::addEvent( QgsEventTracing::AsyncBegin, u"3D"_s, u"DEM"_s, nodeId.text() );

  // extend the rect by half-pixel on each side? to get the values in "corners"
  QgsRectangle extent = mTilingScheme.tileToExtent( nodeId );
  float mapUnitsPerPixel = extent.width() / mResolution;
  extent.grow( mapUnitsPerPixel / 2 );
  // but make sure not to go beyond the root tile's full extent (returns invalid values)
  QgsRectangle rootTileExtent = mTilingScheme.tileToExtent( 0, 0, 0 );
  extent = extent.intersect( rootTileExtent );

  QFutureWatcher<QByteArray> *fw = new QFutureWatcher<QByteArray>( nullptr );
  connect( fw, &QFutureWatcher<QByteArray>::finished, this, [this, fw]() { mJobs.remove( fw ); } );
  connect( fw, &QFutureWatcher<QByteArray>::finished, fw, &QObject::deleteLater );

  QFuture<QByteArray> future;
  if ( mClonedProvider )
  {
    // make a clone of the data provider so it is safe to use in worker thread
    std::unique_ptr<QgsRasterDataProvider> clonedProviderClone( mClonedProvider->clone() );
    clonedProviderClone->moveToThread( nullptr );
    future = QtConcurrent::run( readDtmData, clonedProviderClone.release(), extent, mResolution, mTilingScheme.crs(), mTilingScheme.fullExtent() );
  }
  else
  {
    future = QtConcurrent::run( readOnlineDtm, mDownloader.get(), extent, mResolution, mTilingScheme.crs(), mTransformContext );
  }
  mJobs[fw] = future;
  return future;
}

void QgsDemHeightMapGenerator::waitForFinished()
{
  QVector<QFutureWatcher<QByteArray> *> toBeDeleted;
  for ( auto it = mJobs.keyBegin(); it != mJobs.keyEnd(); it++ )
  {
    toBeDeleted.push_back( *it );
  }

  for ( QFutureWatcher<QByteArray> *fw : toBeDeleted )
  {
    fw->waitForFinished(); // Signals will delete the job and the FutureWatcher
  }
}

void QgsDemHeightMapGenerator::lazyLoadDtmCoarseData( int res, const QgsRectangle &rect )
{
  QMutexLocker locker( &mLazyLoadDtmCoarseDataMutex );
  if ( !mDtmCoarseRasterBlock )
  {
    mDtmCoarseRasterBlock.reset( mClonedProvider->block( 1, rect, res, res ) );
  }
}

float QgsDemHeightMapGenerator::heightAt( double x, double y )
{
  if ( !mClonedProvider )
    return std::numeric_limits<float>::quiet_NaN(); // TODO: calculate heights for online DTM

  // TODO: this is quite a primitive implementation: better to use heightmaps currently in use
  int res = 1024;
  lazyLoadDtmCoarseData( res, mDtmExtent );

  int cellX = ( int ) ( ( x - mDtmExtent.xMinimum() ) / mDtmExtent.width() * res + .5f );
  int cellY = ( int ) ( ( mDtmExtent.yMaximum() - y ) / mDtmExtent.height() * res + .5f );
  cellX = std::clamp( cellX, 0, res - 1 );
  cellY = std::clamp( cellY, 0, res - 1 );

  bool isNoData = false;
  const double val = mDtmCoarseRasterBlock->valueAndNoData( cellY, cellX, isNoData );

  return isNoData ? std::numeric_limits<float>::quiet_NaN() : static_cast<float>( val );
}

/// @endcond
