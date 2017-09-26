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
  zMin = zBits[0];
  zMax = zBits[0];
  for ( int i = 0; i < zCount; ++i )
  {
    float z = zBits[i];
    zMin = qMin( zMin, z );
    zMax = qMax( zMax, z );
  }
}


QgsDemTerrainTileLoader::QgsDemTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node )
  : QgsTerrainTileLoader( terrain, node )
  , resolution( 0 )
{

  const Qgs3DMapSettings &map = terrain->map3D();
  QgsDemTerrainGenerator *generator = static_cast<QgsDemTerrainGenerator *>( map.terrainGenerator() );

  // get heightmap asynchronously
  connect( generator->heightMapGenerator(), &QgsDemHeightMapGenerator::heightMapReady, this, &QgsDemTerrainTileLoader::onHeightMapReady );
  heightMapJobId = generator->heightMapGenerator()->render( node->x, node->y, node->z );
  resolution = generator->heightMapGenerator()->resolution();
}

QgsDemTerrainTileLoader::~QgsDemTerrainTileLoader()
{
  qDebug() << "DEM chunk loader done.";
}

Qt3DCore::QEntity *QgsDemTerrainTileLoader::createEntity( Qt3DCore::QEntity *parent )
{
  QgsTerrainTileEntity *entity = new QgsTerrainTileEntity;

  // create geometry renderer

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new DemTerrainTileGeometry( resolution, heightMap, mesh ) );
  entity->addComponent( mesh ); // takes ownership if the component has no parent

  // create material

  createTextureComponent( entity );

  // create transform

  Qt3DCore::QTransform *transform;
  transform = new Qt3DCore::QTransform();
  entity->addComponent( transform );

  float zMin, zMax;
  _heightMapMinMax( heightMap, zMin, zMax );

  const Qgs3DMapSettings &map = terrain()->map3D();
  QgsRectangle extent = map.terrainGenerator()->terrainTilingScheme.tileToExtent( node->x, node->y, node->z ); //node->extent;
  double x0 = extent.xMinimum() - map.originX;
  double y0 = extent.yMinimum() - map.originY;
  double side = extent.width();
  double half = side / 2;

  transform->setScale3D( QVector3D( side, map.terrainVerticalScale(), side ) );
  transform->setTranslation( QVector3D( x0 + half, 0, - ( y0 + half ) ) );

  node->setExactBbox( QgsAABB( x0, zMin * map.terrainVerticalScale(), -y0, x0 + side, zMax * map.terrainVerticalScale(), -( y0 + side ) ) );

  entity->setEnabled( false );
  entity->setParent( parent );
  return entity;
}

void QgsDemTerrainTileLoader::onHeightMapReady( int jobId, const QByteArray &heightMap )
{
  if ( heightMapJobId == jobId )
  {
    this->heightMap = heightMap;
    heightMapJobId = -1;

    // continue loading - texture
    loadTexture();
  }
}


// ---------------------

#include <qgsrasterlayer.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>

QgsDemHeightMapGenerator::QgsDemHeightMapGenerator( QgsRasterLayer *dtm, const QgsTilingScheme &tilingScheme, int resolution )
  : dtm( dtm )
  , clonedProvider( ( QgsRasterDataProvider * )dtm->dataProvider()->clone() )
  , tilingScheme( tilingScheme )
  , res( resolution )
  , lastJobId( 0 )
{
}

QgsDemHeightMapGenerator::~QgsDemHeightMapGenerator()
{
  delete clonedProvider;
}

#include <QElapsedTimer>

static QByteArray _readDtmData( QgsRasterDataProvider *provider, const QgsRectangle &extent, int res )
{
  QElapsedTimer t;
  t.start();

  // TODO: use feedback object? (but GDAL currently does not support cancelation anyway)
  QgsRasterBlock *block = provider->block( 1, extent, res, res );

  QByteArray data;
  if ( block )
  {
    block->convert( Qgis::Float32 ); // currently we expect just floats
    data = block->data();
    data.detach();  // this should make a deep copy
    delete block;
  }
  qDebug() << "[TT] read block time " << t.elapsed() << "ms";
  return data;
}

int QgsDemHeightMapGenerator::render( int x, int y, int z )
{
  Q_ASSERT( jobs.isEmpty() );  // should be always just one active job...

  // extend the rect by half-pixel on each side? to get the values in "corners"
  QgsRectangle extent = tilingScheme.tileToExtent( x, y, z );
  float mapUnitsPerPixel = extent.width() / res;
  extent.grow( mapUnitsPerPixel / 2 );
  // but make sure not to go beyond the full extent (returns invalid values)
  QgsRectangle fullExtent = tilingScheme.tileToExtent( 0, 0, 0 );
  extent = extent.intersect( &fullExtent );

  JobData jd;
  jd.jobId = ++lastJobId;
  jd.extent = extent;
  jd.timer.start();
  // make a clone of the data provider so it is safe to use in worker thread
  jd.future = QtConcurrent::run( _readDtmData, clonedProvider, extent, res );

  QFutureWatcher<QByteArray> *fw = new QFutureWatcher<QByteArray>;
  fw->setFuture( jd.future );
  connect( fw, &QFutureWatcher<QByteArray>::finished, this, &QgsDemHeightMapGenerator::onFutureFinished );

  jobs.insert( fw, jd );
  qDebug() << "[TT] added job: " << jd.jobId << " " << x << "|" << y << "|" << z << "  .... in queue: " << jobs.count();

  return jd.jobId;
}

QByteArray QgsDemHeightMapGenerator::renderSynchronously( int x, int y, int z )
{
  // extend the rect by half-pixel on each side? to get the values in "corners"
  QgsRectangle extent = tilingScheme.tileToExtent( x, y, z );
  float mapUnitsPerPixel = extent.width() / res;
  extent.grow( mapUnitsPerPixel / 2 );
  // but make sure not to go beyond the full extent (returns invalid values)
  QgsRectangle fullExtent = tilingScheme.tileToExtent( 0, 0, 0 );
  extent = extent.intersect( &fullExtent );

  QgsRasterBlock *block = dtm->dataProvider()->block( 1, extent, res, res );

  QByteArray data;
  if ( block )
  {
    block->convert( Qgis::Float32 ); // currently we expect just floats
    data = block->data();
    data.detach();  // this should make a deep copy
    delete block;
  }

  return data;
}

float QgsDemHeightMapGenerator::heightAt( double x, double y )
{
  // TODO: this is quite a primitive implementation: better to use heightmaps currently in use
  int res = 1024;
  QgsRectangle rect = dtm->extent();
  if ( dtmCoarseData.isEmpty() )
  {
    QgsRasterBlock *block = dtm->dataProvider()->block( 1, rect, res, res );
    block->convert( Qgis::Float32 );
    dtmCoarseData = block->data();
    dtmCoarseData.detach();  // make a deep copy
    delete block;
  }

  int cellX = ( int )( ( x - rect.xMinimum() ) / rect.width() * res + .5f );
  int cellY = ( int )( ( rect.yMaximum() - y ) / rect.height() * res + .5f );
  cellX = qBound( 0, cellX, res - 1 );
  cellY = qBound( 0, cellY, res - 1 );

  const float *data = ( const float * ) dtmCoarseData.constData();
  return data[cellX + cellY * res];
}

void QgsDemHeightMapGenerator::onFutureFinished()
{
  QFutureWatcher<QByteArray> *fw = static_cast<QFutureWatcher<QByteArray>*>( sender() );
  Q_ASSERT( fw );
  Q_ASSERT( jobs.contains( fw ) );
  JobData jobData = jobs.value( fw );

  jobs.remove( fw );
  fw->deleteLater();
  qDebug() << "[TT] finished job " << jobData.jobId << "total time " << jobData.timer.elapsed() << "ms  ... in queue: " << jobs.count();

  QByteArray data = jobData.future.result();
  emit heightMapReady( jobData.jobId, data );
}

/// @endcond
