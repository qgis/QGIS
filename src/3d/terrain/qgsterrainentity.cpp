/***************************************************************************
  qgsterrainentity.cpp
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

#include "qgsterrainentity.h"

#include <memory>

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsaabb.h"
#include "qgsabstractterrainsettings.h"
#include "qgschunknode.h"
#include "qgscoordinatetransform.h"
#include "qgsdemterraintilegeometry_p.h"
#include "qgseventtracing.h"
#include "qgsraycastingutils.h"
#include "qgsterraingenerator.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintileentity_p.h"

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>

#include "moc_qgsterrainentity.cpp"

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


QgsTerrainEntity::QgsTerrainEntity( Qgs3DMapSettings *map, Qt3DCore::QNode *parent )
  : QgsChunkedEntity( map, map->terrainSettings()->maximumScreenError(), map->terrainGenerator(), false, std::numeric_limits<int>::max(), parent )
{
  map->terrainGenerator()->setTerrain( this );
  mIsValid = map->terrainGenerator()->isValid();

  connect( map, &Qgs3DMapSettings::showTerrainBoundingBoxesChanged, this, &QgsTerrainEntity::onShowBoundingBoxesChanged );
  connect( map, &Qgs3DMapSettings::showTerrainTilesInfoChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( map, &Qgs3DMapSettings::showLabelsChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( map, &Qgs3DMapSettings::layersChanged, this, &QgsTerrainEntity::onLayersChanged );
  connect( map, &Qgs3DMapSettings::backgroundColorChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( map, &Qgs3DMapSettings::terrainMapThemeChanged, this, &QgsTerrainEntity::invalidateMapImages );
  connect( map, &Qgs3DMapSettings::terrainSettingsChanged, this, &QgsTerrainEntity::onTerrainElevationOffsetChanged );

  connectToLayersRepaintRequest();

  mTextureGenerator = new QgsTerrainTextureGenerator( *map );

  mUpdateJobFactory = std::make_unique<TerrainMapUpdateJobFactory>( mTextureGenerator );

  mTerrainTransform = new Qt3DCore::QTransform;
  mTerrainTransform->setScale( 1.0f );
  mTerrainTransform->setTranslation( QVector3D( 0.0f, 0.0f, map->terrainSettings()->elevationOffset() ) );
  addComponent( mTerrainTransform );
}

QgsTerrainEntity::~QgsTerrainEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();

  delete mTextureGenerator;
}

QList<QgsRayCastHit> QgsTerrainEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  Q_UNUSED( context )
  QList<QgsRayCastHit> result;

  float minDist = -1;
  QgsVector3D intersectionPointMapCoords;
  switch ( mMapSettings->terrainGenerator()->type() )
  {
    case QgsTerrainGenerator::Flat:
    {
      if ( ray.direction().z() == 0 )
        break; // the ray is parallel to the flat terrain

      const float dist = static_cast<float>( mMapSettings->terrainSettings()->elevationOffset() - ray.origin().z() - mMapSettings->origin().z() ) / ray.direction().z();
      const QVector3D terrainPlanePoint = ray.origin() + ray.direction() * dist;
      const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( terrainPlanePoint, mMapSettings->origin() );
      if ( mMapSettings->extent().contains( mapCoords.x(), mapCoords.y() ) )
      {
        minDist = dist;
        intersectionPointMapCoords = mapCoords;
      }
      break;
    }
    case QgsTerrainGenerator::Dem:
    {
      const QList<QgsChunkNode *> activeNodes = this->activeNodes();
      QVector3D nearestIntersectionPoint;
      for ( QgsChunkNode *node : activeNodes )
      {
        QgsAABB nodeBbox = Qgs3DUtils::mapToWorldExtent( node->box3D(), mMapSettings->origin() );

        if ( node->entity() && ( minDist < 0 || nodeBbox.distanceFromPoint( ray.origin() ) < minDist ) && QgsRayCastingUtils::rayBoxIntersection( ray, nodeBbox ) )
        {
          Qt3DRender::QGeometryRenderer *rend = node->entity()->findChild<Qt3DRender::QGeometryRenderer *>();
          auto *geom = rend->geometry();
          Qt3DCore::QTransform *tr = node->entity()->findChild<Qt3DCore::QTransform *>();
          QVector3D nodeIntPoint;
          DemTerrainTileGeometry *demGeom = static_cast<DemTerrainTileGeometry *>( geom );
          if ( demGeom->rayIntersection( ray, context, tr->matrix(), nodeIntPoint ) )
          {
            const float dist = ( ray.origin() - nodeIntPoint ).length();
            if ( minDist < 0 || dist < minDist )
            {
              minDist = dist;
              nearestIntersectionPoint = nodeIntPoint;
            }
          }
        }
      }
      if ( minDist >= 0 )
        intersectionPointMapCoords = Qgs3DUtils::worldToMapCoordinates( nearestIntersectionPoint, mMapSettings->origin() );
      break;
    }
    case QgsTerrainGenerator::Mesh:
    case QgsTerrainGenerator::Online:
    case QgsTerrainGenerator::QuantizedMesh:
      // not supported
      break;
  }
  if ( minDist >= 0 )
  {
    QgsRayCastHit hit;
    hit.setDistance( minDist );
    hit.setMapCoordinates( intersectionPointMapCoords );
    result.append( hit );
  }
  return result;
}

void QgsTerrainEntity::onShowBoundingBoxesChanged()
{
  setShowBoundingBoxes( mMapSettings->showTerrainBoundingBoxes() );
}


void QgsTerrainEntity::invalidateMapImages()
{
  QgsEventTracing::addEvent( QgsEventTracing::Instant, u"3D"_s, u"Invalidate textures"_s );

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

  mLayers = mMapSettings->layers();

  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    connect( layer, &QgsMapLayer::repaintRequested, this, &QgsTerrainEntity::invalidateMapImages );
  }
}

void QgsTerrainEntity::onTerrainElevationOffsetChanged()
{
  float newOffset = qobject_cast<Qgs3DMapSettings *>( sender() )->terrainSettings()->elevationOffset();
  mTerrainTransform->setTranslation( QVector3D( 0.0f, 0.0f, newOffset ) );
}

float QgsTerrainEntity::terrainElevationOffset() const
{
  return mMapSettings->terrainSettings()->elevationOffset();
}


// -----------


TerrainMapUpdateJob::TerrainMapUpdateJob( QgsTerrainTextureGenerator *textureGenerator, QgsChunkNode *node )
  : QgsChunkQueueJob( node )
  , mTextureGenerator( textureGenerator )
{
}

void TerrainMapUpdateJob::start()
{
  QgsChunkNode *node = chunk();

  QgsTerrainTileEntity *entity = qobject_cast<QgsTerrainTileEntity *>( node->entity() );
  connect( mTextureGenerator, &QgsTerrainTextureGenerator::tileReady, this, &TerrainMapUpdateJob::onTileReady );
  mJobId = mTextureGenerator->render( entity->textureImage()->imageExtent(), node->tileId(), entity->textureImage()->imageDebugText() );
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
