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

#include "qgs3dmapsettings.h"
#include "qgschunknode_p.h"
#include "qgsdemterraingenerator.h"
#include "qgsdemterraintilegeometry_p.h"
#include "qgseventtracing.h"
#include "qgsonlineterraingenerator.h"
#include "qgsterrainentity_p.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsterraintileentity_p.h"
#include "qgsterraingenerator.h"

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QTransform>
#include <QMutexLocker>

///@cond PRIVATE

static void _heightMapMinMax( const QByteArray &heightMap, float &zMin, float &zMax )
{
  const float *zBits = ( const float * ) heightMap.constData();
  int zCount = heightMap.count() / sizeof( float );
  bool first = true;

  zMin = zMax = std::numeric_limits<float>::quiet_NaN();
  for ( int i = 0; i < zCount; ++i )
  {
    float z = zBits[i];
    if ( std::isnan( z ) )
      continue;
    if ( first )
    {
      zMin = zMax = z;
      first = false;
    }
    zMin = std::min( zMin, z );
    zMax = std::max( zMax, z );
  }
}


QgsDemTerrainTileLoader::QgsDemTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, QgsTerrainGenerator *terrainGenerator )
  : QgsTerrainTileLoader( terrain, node )
  , mResolution( 0 )
{

  QgsDemHeightMapGenerator *heightMapGenerator = nullptr;
  if ( terrainGenerator->type() == QgsTerrainGenerator::Dem )
  {
    QgsDemTerrainGenerator *generator = static_cast<QgsDemTerrainGenerator *>( terrainGenerator );
    heightMapGenerator = generator->heightMapGenerator();
    mSkirtHeight = generator->skirtHeight();
  }
  else if ( terrainGenerator->type() == QgsTerrainGenerator::Online )
  {
    QgsOnlineTerrainGenerator *generator = static_cast<QgsOnlineTerrainGenerator *>( terrainGenerator );
    heightMapGenerator = generator->heightMapGenerator();
    mSkirtHeight = generator->skirtHeight();
  }
  else
    Q_ASSERT( false );

  // get heightmap asynchronously
  connect( heightMapGenerator, &QgsDemHeightMapGenerator::heightMapReady, this, &QgsDemTerrainTileLoader::onHeightMapReady );
  mHeightMapJobId = heightMapGenerator->render( node->tileId() );
  mResolution = heightMapGenerator->resolution();
}

Qt3DCore::QEntity *QgsDemTerrainTileLoader::createEntity( Qt3DCore::QEntity *parent )
{
  float zMin, zMax;
  _heightMapMinMax( mHeightMap, zMin, zMax );

  if ( std::isnan( zMin ) || std::isnan( zMax ) )
  {
    // no data available for this tile
    return nullptr;
  }

  const Qgs3DMapSettings &map = terrain()->map3D();
  QgsChunkNodeId nodeId = mNode->tileId();
  QgsRectangle extent = map.terrainGenerator()->tilingScheme().tileToExtent( nodeId );
  double x0 = extent.xMinimum() - map.origin().x();
  double y0 = extent.yMinimum() - map.origin().y();
  double side = extent.width();
  double half = side / 2;


  QgsTerrainTileEntity *entity = new QgsTerrainTileEntity( nodeId );

  // create geometry renderer

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new DemTerrainTileGeometry( mResolution, side, map.terrainVerticalScale(), mSkirtHeight, mHeightMap, mesh ) );
  entity->addComponent( mesh ); // takes ownership if the component has no parent

  // create material

  createTextureComponent( entity, map.isTerrainShadingEnabled(), map.terrainShadingMaterial(), !map.layers().empty() );

  // create transform

  Qt3DCore::QTransform *transform = nullptr;
  transform = new Qt3DCore::QTransform();
  entity->addComponent( transform );

  transform->setScale( side );
  transform->setTranslation( QVector3D( x0 + half, 0, - ( y0 + half ) ) );

  mNode->setExactBbox( QgsAABB( x0, zMin * map.terrainVerticalScale(), -y0, x0 + side, zMax * map.terrainVerticalScale(), -( y0 + side ) ) );
  mNode->updateParentBoundingBoxesRecursively();

  entity->setParent( parent );
  return entity;
}

void QgsDemTerrainTileLoader::onHeightMapReady( int jobId, const QByteArray &heightMap )
{
  if ( mHeightMapJobId == jobId )
  {
    this->mHeightMap = heightMap;
    mHeightMapJobId = -1;

    // continue loading - texture
    loadTexture();
  }
}


// ---------------------

#include <qgsrasterlayer.h>
#include <qgsrasterprojector.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include "qgsterraindownloader.h"

QgsDemHeightMapGenerator::QgsDemHeightMapGenerator( QgsRasterLayer *dtm, const QgsTilingScheme &tilingScheme, int resolution, const QgsCoordinateTransformContext &transformContext )
  : mDtm( dtm )
  , mClonedProvider( dtm ? qgis::down_cast<QgsRasterDataProvider *>( dtm->dataProvider()->clone() ) : nullptr )
  , mTilingScheme( tilingScheme )
  , mResolution( resolution )
  , mLastJobId( 0 )
  , mDownloader( dtm ? nullptr : new QgsTerrainDownloader( transformContext ) )
  , mTransformContext( transformContext )
{
}

QgsDemHeightMapGenerator::~QgsDemHeightMapGenerator()
{
  delete mClonedProvider;
}


static QByteArray _readDtmData( QgsRasterDataProvider *provider, const QgsRectangle &extent, int res, const QgsCoordinateReferenceSystem &destCrs )
{
  provider->moveToThread( QThread::currentThread() );

  QgsEventTracing::ScopedEvent e( QStringLiteral( "3D" ), QStringLiteral( "DEM" ) );

  // TODO: use feedback object? (but GDAL currently does not support cancellation anyway)
  QgsRasterInterface *input = provider;
  std::unique_ptr<QgsRasterProjector> projector;
  if ( provider->crs() != destCrs )
  {
    projector.reset( new QgsRasterProjector );
    projector->setCrs( provider->crs(), destCrs, provider->transformContext() );
    projector->setInput( provider );
    input = projector.get();
  }
  std::unique_ptr< QgsRasterBlock > block( input->block( 1, extent, res, res ) );

  QByteArray data;
  if ( block )
  {
    block->convert( Qgis::DataType::Float32 ); // currently we expect just floats
    data = block->data();
    data.detach();  // this should make a deep copy

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

  provider->moveToThread( nullptr );

  return data;
}

static QByteArray _readOnlineDtm( QgsTerrainDownloader *downloader, const QgsRectangle &extent, int res, const QgsCoordinateReferenceSystem &destCrs, const QgsCoordinateTransformContext &context )
{
  return downloader->getHeightMap( extent, res, destCrs, context );
}

int QgsDemHeightMapGenerator::render( const QgsChunkNodeId &nodeId )
{
  QgsEventTracing::addEvent( QgsEventTracing::AsyncBegin, QStringLiteral( "3D" ), QStringLiteral( "DEM" ), nodeId.text() );

  // extend the rect by half-pixel on each side? to get the values in "corners"
  QgsRectangle extent = mTilingScheme.tileToExtent( nodeId );
  float mapUnitsPerPixel = extent.width() / mResolution;
  extent.grow( mapUnitsPerPixel / 2 );
  // but make sure not to go beyond the full extent (returns invalid values)
  QgsRectangle fullExtent = mTilingScheme.tileToExtent( 0, 0, 0 );
  extent = extent.intersect( fullExtent );

  JobData jd;
  jd.jobId = ++mLastJobId;
  jd.tileId = nodeId;
  jd.extent = extent;
  jd.timer.start();
  QFutureWatcher<QByteArray> *fw = new QFutureWatcher<QByteArray>( nullptr );
  connect( fw, &QFutureWatcher<QByteArray>::finished, this, &QgsDemHeightMapGenerator::onFutureFinished );
  connect( fw, &QFutureWatcher<QByteArray>::finished, fw, &QObject::deleteLater );
  // make a clone of the data provider so it is safe to use in worker thread
  if ( mDtm )
  {
    mClonedProvider->moveToThread( nullptr );
    jd.future = QtConcurrent::run( _readDtmData, mClonedProvider, extent, mResolution, mTilingScheme.crs() );
  }
  else
  {
    jd.future = QtConcurrent::run( _readOnlineDtm, mDownloader.get(), extent, mResolution, mTilingScheme.crs(), mTransformContext );
  }

  fw->setFuture( jd.future );

  mJobs.insert( fw, jd );

  return jd.jobId;
}

void QgsDemHeightMapGenerator::waitForFinished()
{
  for ( auto it = mJobs.keyBegin(); it != mJobs.keyEnd(); it++ )
  {
    QFutureWatcher<QByteArray> *fw = *it;
    disconnect( fw, &QFutureWatcher<QByteArray>::finished, this, &QgsDemHeightMapGenerator::onFutureFinished );
    disconnect( fw, &QFutureWatcher<QByteArray>::finished, fw, &QObject::deleteLater );
  }
  QVector<QFutureWatcher<QByteArray>*> toBeDeleted;
  for ( auto it = mJobs.keyBegin(); it != mJobs.keyEnd(); it++ )
  {
    QFutureWatcher<QByteArray> *fw = *it;
    fw->waitForFinished();
    JobData jobData = mJobs.value( fw );
    toBeDeleted.push_back( fw );

    QByteArray data = jobData.future.result();
    emit heightMapReady( jobData.jobId, data );
  }

  for ( QFutureWatcher<QByteArray> *fw : toBeDeleted )
  {
    mJobs.remove( fw );
    fw->deleteLater();
  }
}

void QgsDemHeightMapGenerator::lazyLoadDtmCoarseData( int res, const QgsRectangle &rect )
{
  QMutexLocker locker( &mLazyLoadDtmCoarseDataMutex );
  if ( mDtmCoarseData.isEmpty() )
  {
    std::unique_ptr< QgsRasterBlock > block( mDtm->dataProvider()->block( 1, rect, res, res ) );
    block->convert( Qgis::DataType::Float32 );
    mDtmCoarseData = block->data();
    mDtmCoarseData.detach();  // make a deep copy
  }
}

float QgsDemHeightMapGenerator::heightAt( double x, double y )
{
  if ( !mDtm )
    return 0;  // TODO: calculate heights for online DTM

  // TODO: this is quite a primitive implementation: better to use heightmaps currently in use
  int res = 1024;
  QgsRectangle rect = mDtm->extent();
  lazyLoadDtmCoarseData( res, rect );

  int cellX = ( int )( ( x - rect.xMinimum() ) / rect.width() * res + .5f );
  int cellY = ( int )( ( rect.yMaximum() - y ) / rect.height() * res + .5f );
  cellX = std::clamp( cellX, 0, res - 1 );
  cellY = std::clamp( cellY, 0, res - 1 );

  const float *data = ( const float * ) mDtmCoarseData.constData();
  return data[cellX + cellY * res];
}

void QgsDemHeightMapGenerator::onFutureFinished()
{
  QFutureWatcher<QByteArray> *fw = static_cast<QFutureWatcher<QByteArray>*>( sender() );
  Q_ASSERT( fw );
  Q_ASSERT( mJobs.contains( fw ) );
  JobData jobData = mJobs.value( fw );

  mJobs.remove( fw );
  fw->deleteLater();

  QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "DEM" ), jobData.tileId.text() );

  QByteArray data = jobData.future.result();
  emit heightMapReady( jobData.jobId, data );
}

/// @endcond
