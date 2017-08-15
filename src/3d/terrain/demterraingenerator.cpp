#include "demterraingenerator.h"

#include "qgs3dmapsettings.h"
#include "demterraintilegeometry.h"
#include "maptexturegenerator.h"
#include "terrain.h"

#include <Qt3DRender/QGeometryRenderer>

#include "qgsrasterlayer.h"

#include "chunknode.h"


#if 0
static QByteArray _temporaryHeightMap( int res )
{
  QByteArray heightMap;
  int count = res * res;
  heightMap.resize( count * sizeof( float ) );
  float *bits = ( float * ) heightMap.data();
  for ( int i = 0; i < count; ++i )
    bits[i] = 0;
  return heightMap;
}
#endif

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


// ---------------


DemTerrainGenerator::DemTerrainGenerator()
  : mResolution( 16 )
{
}

void DemTerrainGenerator::setLayer( QgsRasterLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
  updateGenerator();
}

QgsRasterLayer *DemTerrainGenerator::layer() const
{
  return qobject_cast<QgsRasterLayer *>( mLayer.layer.data() );
}

TerrainGenerator *DemTerrainGenerator::clone() const
{
  DemTerrainGenerator *cloned = new DemTerrainGenerator;
  cloned->mLayer = mLayer;
  cloned->mResolution = mResolution;
  cloned->updateGenerator();
  return cloned;
}

TerrainGenerator::Type DemTerrainGenerator::type() const
{
  return TerrainGenerator::Dem;
}

QgsRectangle DemTerrainGenerator::extent() const
{
  return terrainTilingScheme.tileToExtent( 0, 0, 0 );
}

float DemTerrainGenerator::heightAt( double x, double y, const Qgs3DMapSettings &map ) const
{
  Q_UNUSED( map );
  return mHeightMapGenerator->heightAt( x, y );
}

void DemTerrainGenerator::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( "layer", mLayer.layerId );
  elem.setAttribute( "resolution", mResolution );
}

void DemTerrainGenerator::readXml( const QDomElement &elem )
{
  mLayer = QgsMapLayerRef( elem.attribute( "layer" ) );
  mResolution = elem.attribute( "resolution" ).toInt();
}

void DemTerrainGenerator::resolveReferences( const QgsProject &project )
{
  mLayer = QgsMapLayerRef( project.mapLayer( mLayer.layerId ) );
  updateGenerator();
}

ChunkLoader *DemTerrainGenerator::createChunkLoader( ChunkNode *node ) const
{
  return new DemTerrainChunkLoader( mTerrain, node );
}

void DemTerrainGenerator::updateGenerator()
{
  QgsRasterLayer *dem = layer();
  if ( dem )
  {
    terrainTilingScheme = TilingScheme( dem->extent(), dem->crs() );
    mHeightMapGenerator.reset( new DemHeightMapGenerator( dem, terrainTilingScheme, mResolution ) );
  }
  else
  {
    terrainTilingScheme = TilingScheme();
    mHeightMapGenerator.reset();
  }
}


// ------------


DemTerrainChunkLoader::DemTerrainChunkLoader( Terrain *terrain, ChunkNode *node )
  : TerrainChunkLoader( terrain, node )
  , resolution( 0 )
{

  const Qgs3DMapSettings &map = mTerrain->map3D();
  DemTerrainGenerator *generator = static_cast<DemTerrainGenerator *>( map.terrainGenerator() );

  // get heightmap asynchronously
  connect( generator->heightMapGenerator(), &DemHeightMapGenerator::heightMapReady, this, &DemTerrainChunkLoader::onHeightMapReady );
  heightMapJobId = generator->heightMapGenerator()->render( node->x, node->y, node->z );
  resolution = generator->heightMapGenerator()->resolution();
}

DemTerrainChunkLoader::~DemTerrainChunkLoader()
{
  qDebug() << "DEM chunk loader done.";
}

Qt3DCore::QEntity *DemTerrainChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  TerrainChunkEntity *entity = new TerrainChunkEntity;

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

  const Qgs3DMapSettings &map = mTerrain->map3D();
  QgsRectangle extent = map.terrainGenerator()->terrainTilingScheme.tileToExtent( node->x, node->y, node->z ); //node->extent;
  double x0 = extent.xMinimum() - map.originX;
  double y0 = extent.yMinimum() - map.originY;
  double side = extent.width();
  double half = side / 2;

  transform->setScale3D( QVector3D( side, map.terrainVerticalScale(), side ) );
  transform->setTranslation( QVector3D( x0 + half, 0, - ( y0 + half ) ) );

  node->setExactBbox( AABB( x0, zMin * map.terrainVerticalScale(), -y0, x0 + side, zMax * map.terrainVerticalScale(), -( y0 + side ) ) );

  entity->setEnabled( false );
  entity->setParent( parent );
  return entity;
}

void DemTerrainChunkLoader::onHeightMapReady( int jobId, const QByteArray &heightMap )
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

DemHeightMapGenerator::DemHeightMapGenerator( QgsRasterLayer *dtm, const TilingScheme &tilingScheme, int resolution )
  : dtm( dtm )
  , clonedProvider( ( QgsRasterDataProvider * )dtm->dataProvider()->clone() )
  , tilingScheme( tilingScheme )
  , res( resolution )
  , lastJobId( 0 )
{
}

DemHeightMapGenerator::~DemHeightMapGenerator()
{
  delete clonedProvider;
}

#include <QElapsedTimer>

static QByteArray _readDtmData( QgsRasterDataProvider *provider, const QgsRectangle &extent, int res )
{
  QElapsedTimer t;
  t.start();

  // TODO: use feedback object? (but GDAL currently does not support cancellation anyway)
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

int DemHeightMapGenerator::render( int x, int y, int z )
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
  connect( fw, &QFutureWatcher<QByteArray>::finished, this, &DemHeightMapGenerator::onFutureFinished );

  jobs.insert( fw, jd );
  qDebug() << "[TT] added job: " << jd.jobId << " " << x << "|" << y << "|" << z << "  .... in queue: " << jobs.count();

  return jd.jobId;
}

QByteArray DemHeightMapGenerator::renderSynchronously( int x, int y, int z )
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

float DemHeightMapGenerator::heightAt( double x, double y )
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

void DemHeightMapGenerator::onFutureFinished()
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
