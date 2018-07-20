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
#include "qgsterrainentity_p.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsterraintileentity_p.h"

#include <Qt3DRender/QGeometryRenderer>

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


QgsDemTerrainTileLoader::QgsDemTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node )
  : QgsTerrainTileLoader( terrain, node )
  , mResolution( 0 )
{

  const Qgs3DMapSettings &map = terrain->map3D();
  QgsDemTerrainGenerator *generator = static_cast<QgsDemTerrainGenerator *>( map.terrainGenerator() );

  // get heightmap asynchronously
  connect( generator->heightMapGenerator(), &QgsDemHeightMapGenerator::heightMapReady, this, &QgsDemTerrainTileLoader::onHeightMapReady );
  mHeightMapJobId = generator->heightMapGenerator()->render( node->tileX(), node->tileY(), node->tileZ() );
  mResolution = generator->heightMapGenerator()->resolution();
  mSkirtHeight = generator->skirtHeight();
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

  QgsTerrainTileEntity *entity = new QgsTerrainTileEntity;

  // create geometry renderer

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new DemTerrainTileGeometry( mResolution, mSkirtHeight, mHeightMap, mesh ) );
  entity->addComponent( mesh ); // takes ownership if the component has no parent

  // create material

  createTextureComponent( entity );

  // create transform

  Qt3DCore::QTransform *transform = nullptr;
  transform = new Qt3DCore::QTransform();
  entity->addComponent( transform );

  const Qgs3DMapSettings &map = terrain()->map3D();
  QgsRectangle extent = map.terrainGenerator()->tilingScheme().tileToExtent( mNode->tileX(), mNode->tileY(), mNode->tileZ() ); //node->extent;
  double x0 = extent.xMinimum() - map.origin().x();
  double y0 = extent.yMinimum() - map.origin().y();
  double side = extent.width();
  double half = side / 2;

  transform->setScale3D( QVector3D( side, map.terrainVerticalScale(), side ) );
  transform->setTranslation( QVector3D( x0 + half, 0, - ( y0 + half ) ) );

  mNode->setExactBbox( QgsAABB( x0, zMin * map.terrainVerticalScale(), -y0, x0 + side, zMax * map.terrainVerticalScale(), -( y0 + side ) ) );

  entity->setEnabled( false );
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

QgsDemHeightMapGenerator::QgsDemHeightMapGenerator( QgsRasterLayer *dtm, const QgsTilingScheme &tilingScheme, int resolution )
  : mDtm( dtm )
  , mClonedProvider( ( QgsRasterDataProvider * )dtm->dataProvider()->clone() )
  , mTilingScheme( tilingScheme )
  , mResolution( resolution )
  , mLastJobId( 0 )
{
}

QgsDemHeightMapGenerator::~QgsDemHeightMapGenerator()
{
  delete mClonedProvider;
}

#include <QElapsedTimer>

static QByteArray _readDtmData( QgsRasterDataProvider *provider, const QgsRectangle &extent, int res, const QgsCoordinateReferenceSystem &destCrs )
{
  QElapsedTimer t;
  t.start();

  // TODO: use feedback object? (but GDAL currently does not support cancelation anyway)
  QgsRasterInterface *input = provider;
  std::unique_ptr<QgsRasterProjector> projector;
  if ( provider->crs() != destCrs )
  {
    projector.reset( new QgsRasterProjector );
    projector->setCrs( provider->crs(), destCrs );
    projector->setInput( provider );
    input = projector.get();
  }
  std::unique_ptr< QgsRasterBlock > block( input->block( 1, extent, res, res ) );

  QByteArray data;
  if ( block )
  {
    block->convert( Qgis::Float32 ); // currently we expect just floats
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
  return data;
}

int QgsDemHeightMapGenerator::render( int x, int y, int z )
{
  Q_ASSERT( mJobs.isEmpty() );  // should be always just one active job...

  // extend the rect by half-pixel on each side? to get the values in "corners"
  QgsRectangle extent = mTilingScheme.tileToExtent( x, y, z );
  float mapUnitsPerPixel = extent.width() / mResolution;
  extent.grow( mapUnitsPerPixel / 2 );
  // but make sure not to go beyond the full extent (returns invalid values)
  QgsRectangle fullExtent = mTilingScheme.tileToExtent( 0, 0, 0 );
  extent = extent.intersect( fullExtent );

  JobData jd;
  jd.jobId = ++mLastJobId;
  jd.extent = extent;
  jd.timer.start();
  // make a clone of the data provider so it is safe to use in worker thread
  jd.future = QtConcurrent::run( _readDtmData, mClonedProvider, extent, mResolution, mTilingScheme.crs() );

  QFutureWatcher<QByteArray> *fw = new QFutureWatcher<QByteArray>( nullptr );
  fw->setFuture( jd.future );
  connect( fw, &QFutureWatcher<QByteArray>::finished, this, &QgsDemHeightMapGenerator::onFutureFinished );

  mJobs.insert( fw, jd );

  return jd.jobId;
}

QByteArray QgsDemHeightMapGenerator::renderSynchronously( int x, int y, int z )
{
  // extend the rect by half-pixel on each side? to get the values in "corners"
  QgsRectangle extent = mTilingScheme.tileToExtent( x, y, z );
  float mapUnitsPerPixel = extent.width() / mResolution;
  extent.grow( mapUnitsPerPixel / 2 );
  // but make sure not to go beyond the full extent (returns invalid values)
  QgsRectangle fullExtent = mTilingScheme.tileToExtent( 0, 0, 0 );
  extent = extent.intersect( fullExtent );

  std::unique_ptr< QgsRasterBlock > block( mDtm->dataProvider()->block( 1, extent, mResolution, mResolution ) );

  QByteArray data;
  if ( block )
  {
    block->convert( Qgis::Float32 ); // currently we expect just floats
    data = block->data();
    data.detach();  // this should make a deep copy
  }

  return data;
}

float QgsDemHeightMapGenerator::heightAt( double x, double y )
{
  // TODO: this is quite a primitive implementation: better to use heightmaps currently in use
  int res = 1024;
  QgsRectangle rect = mDtm->extent();
  if ( mDtmCoarseData.isEmpty() )
  {
    std::unique_ptr< QgsRasterBlock > block( mDtm->dataProvider()->block( 1, rect, res, res ) );
    block->convert( Qgis::Float32 );
    mDtmCoarseData = block->data();
    mDtmCoarseData.detach();  // make a deep copy
  }

  int cellX = ( int )( ( x - rect.xMinimum() ) / rect.width() * res + .5f );
  int cellY = ( int )( ( rect.yMaximum() - y ) / rect.height() * res + .5f );
  cellX = qBound( 0, cellX, res - 1 );
  cellY = qBound( 0, cellY, res - 1 );

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

  QByteArray data = jobData.future.result();
  emit heightMapReady( jobData.jobId, data );
}

/// @endcond
