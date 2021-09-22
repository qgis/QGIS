/***************************************************************************
  qgsterrainentity_p.cpp
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

#include "qgsterrainentity_p.h"

#include "qgsaabb.h"
#include "qgs3dmapsettings.h"
#include "qgschunknode_p.h"
#include "qgsdemterraintilegeometry_p.h"
#include "qgseventtracing.h"
#include "qgsraycastingutils_p.h"
#include "qgsterraingenerator.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintileentity_p.h"

#include "qgscoordinatetransform.h"

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>
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

    QgsChunkQueueJob *createJob( QgsChunkNode *chunk ) override
    {
      return new TerrainMapUpdateJob( mTextureGenerator, chunk );
    }

  private:
    QgsTerrainTextureGenerator *mTextureGenerator = nullptr;
};


// -----------


QgsTerrainEntity::QgsTerrainEntity( const Qgs3DMapSettings &map, Qt3DCore::QNode *parent )
  : QgsChunkedEntity( map.maxTerrainScreenError(), map.terrainGenerator(), false, std::numeric_limits<int>::max(), parent )
  , mMap( map )
{
  map.terrainGenerator()->setTerrain( this );
  mIsValid = map.terrainGenerator()->isValid();

  connect( &map, &Qgs3DMapSettings::showTerrainBoundingBoxesChanged, this, &QgsTerrainEntity::onShowBoundingBoxesChanged );
  connect( &map, &Qgs3DMapSettings::showTerrainTilesInfoChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( &map, &Qgs3DMapSettings::showLabelsChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( &map, &Qgs3DMapSettings::layersChanged, this, &QgsTerrainEntity::onLayersChanged );
  connect( &map, &Qgs3DMapSettings::backgroundColorChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( &map, &Qgs3DMapSettings::terrainMapThemeChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( &map, &Qgs3DMapSettings::terrainElevationOffsetChanged, this, &QgsTerrainEntity::onTerrainElevationOffsetChanged );

  connectToLayersRepaintRequest();

  mTerrainToMapTransform = new QgsCoordinateTransform( map.terrainGenerator()->crs(), map.crs(), map.transformContext() );

  mTextureGenerator = new QgsTerrainTextureGenerator( map );

  mUpdateJobFactory.reset( new TerrainMapUpdateJobFactory( mTextureGenerator ) );

  mTerrainPicker = new Qt3DRender::QObjectPicker;
  // add camera control's terrain picker as a component to be able to capture height where mouse was
  // pressed in order to correctly pan camera when dragging mouse
  addComponent( mTerrainPicker );

  mTerrainTransform = new Qt3DCore::QTransform;
  mTerrainTransform->setScale( 1.0f );
  mTerrainTransform->setTranslation( QVector3D( 0.0f, map.terrainElevationOffset(), 0.0f ) );
  addComponent( mTerrainTransform );
}

QgsTerrainEntity::~QgsTerrainEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();

  delete mTextureGenerator;
  delete mTerrainToMapTransform;
}

bool QgsTerrainEntity::rayIntersection( const QgsRayCastingUtils::Ray3D &ray, QVector3D &intersectionPoint )
{
  if ( !rootNode() )
    return false;

  if ( mMap.terrainGenerator()->type() != QgsTerrainGenerator::Dem )
    return false;  // currently only working with DEM terrain

  float minDist = -1;

  QList<QgsChunkNode *> lst = activeNodes();
  for ( QgsChunkNode *n : lst )
  {
    if ( n->entity() && ( minDist < 0 || n->bbox().distanceFromPoint( ray.origin() ) < minDist ) && QgsRayCastingUtils::rayBoxIntersection( ray, n->bbox() ) )
    {
      Qt3DRender::QGeometryRenderer *rend = n->entity()->findChild<Qt3DRender::QGeometryRenderer *>();
      Qt3DRender::QGeometry *geom = rend->geometry();
      DemTerrainTileGeometry *demGeom = static_cast<DemTerrainTileGeometry *>( geom );
      Qt3DCore::QTransform *tr = n->entity()->findChild<Qt3DCore::QTransform *>();
      QVector3D nodeIntPoint;
      if ( demGeom->rayIntersection( ray, tr->matrix(), nodeIntPoint ) )
      {
        float dist = ( ray.origin() - intersectionPoint ).length();
        if ( minDist < 0 || dist < minDist )
        {
          minDist = dist;
          intersectionPoint = nodeIntPoint;
        }
      }
    }
  }

  return minDist >= 0;
}

void QgsTerrainEntity::onShowBoundingBoxesChanged()
{
  setShowBoundingBoxes( mMap.showTerrainBoundingBoxes() );
}


void QgsTerrainEntity::invalidateMapImages()
{
  QgsEventTracing::addEvent( QgsEventTracing::Instant, QStringLiteral( "3D" ), QStringLiteral( "Invalidate textures" ) );

  // handle active nodes

  updateNodes( mActiveNodes, mUpdateJobFactory.get() );

  // handle inactive nodes afterwards

  QList<QgsChunkNode *> inactiveNodes;
  const QList<QgsChunkNode *> descendants = mRootNode->descendants();
  for ( QgsChunkNode *node : descendants )
  {
    if ( !node->entity() )
      continue;
    if ( mActiveNodes.contains( node ) )
      continue;
    inactiveNodes << node;
  }

  updateNodes( inactiveNodes, mUpdateJobFactory.get() );

  setNeedsUpdate( true );
}

void QgsTerrainEntity::onLayersChanged()
{
  connectToLayersRepaintRequest();
  invalidateMapImages();
}

void QgsTerrainEntity::connectToLayersRepaintRequest()
{
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsTerrainEntity::invalidateMapImages );
  }

  mLayers = mMap.layers();

  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    connect( layer, &QgsMapLayer::repaintRequested, this, &QgsTerrainEntity::invalidateMapImages );
  }
}

void QgsTerrainEntity::onTerrainElevationOffsetChanged( float newOffset )
{
  mTerrainTransform->setTranslation( QVector3D( 0.0f, newOffset, 0.0f ) );
}

float QgsTerrainEntity::terrainElevationOffset() const
{
  return mMap.terrainElevationOffset();
}


// -----------


TerrainMapUpdateJob::TerrainMapUpdateJob( QgsTerrainTextureGenerator *textureGenerator, QgsChunkNode *node )
  : QgsChunkQueueJob( node )
  , mTextureGenerator( textureGenerator )
{
  QgsTerrainTileEntity *entity = qobject_cast<QgsTerrainTileEntity *>( node->entity() );
  connect( textureGenerator, &QgsTerrainTextureGenerator::tileReady, this, &TerrainMapUpdateJob::onTileReady );
  mJobId = textureGenerator->render( entity->textureImage()->imageExtent(), node->tileId(), entity->textureImage()->imageDebugText() );
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
