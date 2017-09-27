#include "qgsterrainentity_p.h"

#include "qgsaabb.h"
#include "qgs3dmapsettings.h"
#include "qgschunknode_p.h"
#include "qgsterraingenerator.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintileentity_p.h"

#include "qgscoordinatetransform.h"

#include <Qt3DRender/QObjectPicker>


///@cond PRIVATE

//! Factory for map update jobs
class TerrainMapUpdateJobFactory : public QgsChunkQueueJobFactory
{
  public:
    TerrainMapUpdateJobFactory( QgsTerrainTextureGenerator *textureGenerator )
      : mTextureGenerator( textureGenerator )
    {
    }

    virtual QgsChunkQueueJob *createJob( QgsChunkNode *chunk )
    {
      return new TerrainMapUpdateJob( mTextureGenerator, chunk );
    }

  private:
    QgsTerrainTextureGenerator *mTextureGenerator;
};


// -----------


QgsTerrainEntity::QgsTerrainEntity( int maxLevel, const Qgs3DMapSettings &map, Qt3DCore::QNode *parent )
  : QgsChunkedEntity( map.terrainGenerator()->rootChunkBbox( map ),
                      map.terrainGenerator()->rootChunkError( map ),
                      map.maxTerrainScreenError(), maxLevel, map.terrainGenerator(), parent )
  , map( map )
  , mTerrainPicker( nullptr )
{
  map.terrainGenerator()->setTerrain( this );

  connect( &map, &Qgs3DMapSettings::showTerrainBoundingBoxesChanged, this, &QgsTerrainEntity::onShowBoundingBoxesChanged );
  connect( &map, &Qgs3DMapSettings::showTerrainTilesInfoChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( &map, &Qgs3DMapSettings::layersChanged, this, &QgsTerrainEntity::onLayersChanged );
  connect( &map, &Qgs3DMapSettings::backgroundColorChanged, this, &QgsTerrainEntity::invalidateMapImages );

  connectToLayersRepaintRequest();

  mTerrainToMapTransform = new QgsCoordinateTransform( map.terrainGenerator()->crs(), map.crs );

  mTextureGenerator = new QgsTerrainTextureGenerator( map );

  mUpdateJobFactory.reset( new TerrainMapUpdateJobFactory( mTextureGenerator ) );

  mTerrainPicker = new Qt3DRender::QObjectPicker;
  // add camera control's terrain picker as a component to be able to capture height where mouse was
  // pressed in order to correcly pan camera when draggin mouse
  addComponent( mTerrainPicker );
}

QgsTerrainEntity::~QgsTerrainEntity()
{
  // cancel / wait for jobs
  if ( mActiveJob )
    cancelActiveJob();

  delete mTextureGenerator;
  delete mTerrainToMapTransform;
}

void QgsTerrainEntity::onShowBoundingBoxesChanged()
{
  setShowBoundingBoxes( map.showTerrainBoundingBoxes() );
}


void QgsTerrainEntity::invalidateMapImages()
{
  qDebug() << "TERRAIN - INVALIDATE MAP IMAGES";

  // handle active nodes

  updateNodes( mActiveNodes, mUpdateJobFactory.get() );
  qDebug() << " updating " << mActiveNodes.count() << " active nodes";

  // handle inactive nodes afterwards

  QList<QgsChunkNode *> inactiveNodes;
  Q_FOREACH ( QgsChunkNode *node, mRootNode->descendants() )
  {
    if ( !node->entity() )
      continue;
    if ( mActiveNodes.contains( node ) )
      continue;
    inactiveNodes << node;
  }

  updateNodes( inactiveNodes, mUpdateJobFactory.get() );
  qDebug() << " updating " << inactiveNodes.count() << " inactive nodes";

  setNeedsUpdate( true );
}

void QgsTerrainEntity::onLayersChanged()
{
  connectToLayersRepaintRequest();
  invalidateMapImages();
}

void QgsTerrainEntity::connectToLayersRepaintRequest()
{
  Q_FOREACH ( QgsMapLayer *layer, mLayers )
  {
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsTerrainEntity::invalidateMapImages );
  }

  mLayers = map.layers();

  Q_FOREACH ( QgsMapLayer *layer, mLayers )
  {
    connect( layer, &QgsMapLayer::repaintRequested, this, &QgsTerrainEntity::invalidateMapImages );
  }
}


// -----------


TerrainMapUpdateJob::TerrainMapUpdateJob( QgsTerrainTextureGenerator *textureGenerator, QgsChunkNode *node )
  : QgsChunkQueueJob( node )
  , mTextureGenerator( textureGenerator )
{
  QgsTerrainTileEntity *entity = qobject_cast<QgsTerrainTileEntity *>( node->entity() );
  connect( textureGenerator, &QgsTerrainTextureGenerator::tileReady, this, &TerrainMapUpdateJob::onTileReady );
  mJobId = textureGenerator->render( entity->textureImage()->imageExtent(), entity->textureImage()->imageDebugText() );
}

void TerrainMapUpdateJob::cancel()
{
  if ( mJobId != -1 )
    mTextureGenerator->cancelJob( mJobId );
}


void TerrainMapUpdateJob::onTileReady( int jobId, const QImage &image )
{
  if ( mJobId == jobId )
  {
    QgsTerrainTileEntity *entity = qobject_cast<QgsTerrainTileEntity *>( mNode->entity() );
    entity->textureImage()->setImage( image );
    mJobId = -1;
    emit finished();
  }
}

/// @endcond
