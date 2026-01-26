/***************************************************************************
  qgstiledscenechunkloader_p.cpp
  --------------------------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledscenechunkloader_p.h"

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgscesiumutils.h"
#include "qgscoordinatetransform.h"
#include "qgsgeotransform.h"
#include "qgsgltf3dutils.h"
#include "qgsgltfutils.h"
#include "qgsquantizedmeshtiles.h"
#include "qgsray3d.h"
#include "qgsraycastcontext.h"
#include "qgsraycastingutils.h"
#include "qgstiledsceneboundingvolume.h"
#include "qgstiledscenetile.h"

#include <QtConcurrentRun>

#include "moc_qgstiledscenechunkloader_p.cpp"

///@cond PRIVATE

size_t qHash( const QgsChunkNodeId &n )
{
  return n.uniqueId;
}

static bool hasLargeBounds( const QgsTiledSceneTile &t, const QgsCoordinateTransform &boundsTransform )
{
  if ( t.geometricError() > 1e6 )
    return true;

  if ( t.boundingVolume().box().isNull() )
    return true;

  Q_ASSERT( boundsTransform.destinationCrs().mapUnits() == Qgis::DistanceUnit::Meters );
  QgsBox3D bounds = t.boundingVolume().bounds( boundsTransform );
  return bounds.width() > 1e5 || bounds.height() > 1e5 || bounds.depth() > 1e5;
}

///

QgsTiledSceneChunkLoader::QgsTiledSceneChunkLoader( QgsChunkNode *node, const QgsTiledSceneIndex &index, const QgsTiledSceneChunkLoaderFactory &factory, double zValueScale, double zValueOffset )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mZValueScale( zValueScale )
  , mZValueOffset( zValueOffset )
  , mIndex( index )
{
}

void QgsTiledSceneChunkLoader::start()
{
  QgsChunkNode *node = chunk();

  mFutureWatcher = new QFutureWatcher<void>( this );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QgsCoordinateTransform &boundsTransform = mFactory.mBoundsTransform;

  const QgsChunkNodeId tileId = node->tileId();
  const QgsVector3D chunkOrigin = node->box3D().center();
  const bool isGlobe = mFactory.mRenderContext.crs().type() == Qgis::CrsType::Geocentric;
  const QFuture<void> future = QtConcurrent::run( [this, tileId, boundsTransform, chunkOrigin, isGlobe] {
    const QgsTiledSceneTile tile = mIndex.getTile( tileId.uniqueId );

    // we do not load tiles that are too big when not in globe scene mode...
    // the problem is that their 3D bounding boxes with ECEF coordinates are huge
    // and we are unable to turn them into planar bounding boxes
    if ( !isGlobe && hasLargeBounds( tile, boundsTransform ) )
      return;

    QString uri = tile.resources().value( u"content"_s ).toString();
    if ( uri.isEmpty() )
    {
      // nothing to show for this tile
      // TODO: can we skip loading it at all?
      return;
    }

    uri = tile.baseUrl().resolved( uri ).toString();
    QByteArray content = mFactory.mIndex.retrieveContent( uri );
    if ( content.isEmpty() )
    {
      // the request probably failed
      // TODO: how can we report it?
      return;
    }

    QgsGltf3DUtils::EntityTransform entityTransform;
    entityTransform.tileTransform = ( tile.transform() ? *tile.transform() : QgsMatrix4x4() );
    entityTransform.chunkOriginTargetCrs = chunkOrigin;
    entityTransform.ecefToTargetCrs = &mFactory.mBoundsTransform;
    entityTransform.zValueScale = mZValueScale;
    entityTransform.zValueOffset = mZValueOffset;
    entityTransform.gltfUpAxis = static_cast<Qgis::Axis>( tile.metadata().value( u"gltfUpAxis"_s, static_cast<int>( Qgis::Axis::Y ) ).toInt() );

    const QString &format = tile.metadata().value( u"contentFormat"_s ).value<QString>();
    QStringList errors;
    if ( format == "quantizedmesh"_L1 )
    {
      try
      {
        QgsQuantizedMeshTile qmTile( content );
        qmTile.removeDegenerateTriangles();
        tinygltf::Model model = qmTile.toGltf( true, 100 );
        mEntity = QgsGltf3DUtils::parsedGltfToEntity( model, entityTransform, uri, &errors );
      }
      catch ( QgsQuantizedMeshParsingException &ex )
      {
        errors.append( u"Failed to parse tile from '%1'"_s.arg( uri ) );
      }
    }
    else if ( format == "cesiumtiles"_L1 )
    {
      const QgsCesiumUtils::TileContents tileContent = QgsCesiumUtils::extractGltfFromTileContent( content );
      if ( tileContent.gltf.isEmpty() )
        return;
      entityTransform.tileTransform.translate( tileContent.rtcCenter );
      mEntity = QgsGltf3DUtils::gltfToEntity( tileContent.gltf, entityTransform, uri, &errors );
    }
    else if ( format == "draco"_L1 )
    {
      QgsGltfUtils::I3SNodeContext i3sContext;
      i3sContext.initFromTile( tile, mFactory.mLayerCrs, mFactory.mBoundsTransform.sourceCrs(), mFactory.mRenderContext.transformContext() );

      QString dracoLoadError;
      tinygltf::Model model;
      if ( !QgsGltfUtils::loadDracoModel( content, i3sContext, model, &dracoLoadError ) )
      {
        errors.append( dracoLoadError );
        return;
      }

      mEntity = QgsGltf3DUtils::parsedGltfToEntity( model, entityTransform, QString(), &errors );
    }
    else
      return; // unsupported tile content type

    // TODO: report errors somewhere?
    if ( !errors.isEmpty() )
    {
      QgsDebugError( "gltf load errors: " + errors.join( '\n' ) );
    }

    if ( mEntity )
    {
      QgsGeoTransform *transform = new QgsGeoTransform;
      transform->setGeoTranslation( chunkOrigin );
      mEntity->addComponent( transform );

      mEntity->moveToThread( QgsApplication::instance()->thread() );
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher->setFuture( future );
}

QgsTiledSceneChunkLoader::~QgsTiledSceneChunkLoader()
{
  if ( !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mFutureWatcher->waitForFinished();
  }
}

Qt3DCore::QEntity *QgsTiledSceneChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mEntity )
    mEntity->setParent( parent );
  return mEntity;
}

///

QgsTiledSceneChunkLoaderFactory::QgsTiledSceneChunkLoaderFactory(
  const Qgs3DRenderContext &context,
  const QgsTiledSceneIndex &index,
  QgsCoordinateReferenceSystem tileCrs,
  QgsCoordinateReferenceSystem layerCrs,
  double zValueScale,
  double zValueOffset
)
  : mRenderContext( context )
  , mIndex( index )
  , mZValueScale( zValueScale )
  , mZValueOffset( zValueOffset )
  , mLayerCrs( layerCrs )
{
  mBoundsTransform = QgsCoordinateTransform( tileCrs, context.crs(), context.transformContext() );
}

QgsChunkLoader *QgsTiledSceneChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsTiledSceneChunkLoader( node, mIndex, *this, mZValueScale, mZValueOffset );
}

QgsChunkNode *QgsTiledSceneChunkLoaderFactory::nodeForTile( const QgsTiledSceneTile &t, const QgsChunkNodeId &nodeId, QgsChunkNode *parent ) const
{
  QgsChunkNode *node = nullptr;
  if ( mRenderContext.crs().type() != Qgis::CrsType::Geocentric && hasLargeBounds( t, mBoundsTransform ) )
  {
    // use the full extent of the scene
    QgsVector3D v0( mRenderContext.extent().xMinimum(), mRenderContext.extent().yMinimum(), -100 );
    QgsVector3D v1( mRenderContext.extent().xMaximum(), mRenderContext.extent().yMaximum(), +100 );
    QgsBox3D box3D( v0, v1 );
    float err = std::min( 1e6, t.geometricError() );
    node = new QgsChunkNode( nodeId, box3D, err, parent );
  }
  else
  {
    QgsBox3D box = t.boundingVolume().bounds( mBoundsTransform );
    box.setZMinimum( box.zMinimum() * mZValueScale + mZValueOffset );
    box.setZMaximum( box.zMaximum() * mZValueScale + mZValueOffset );
    node = new QgsChunkNode( nodeId, box, t.geometricError(), parent );
  }

  node->setRefinementProcess( t.refinementProcess() );
  return node;
}


QgsChunkNode *QgsTiledSceneChunkLoaderFactory::createRootNode() const
{
  const QgsTiledSceneTile t = mIndex.rootTile();
  return nodeForTile( t, QgsChunkNodeId( t.id() ), nullptr );
}


QVector<QgsChunkNode *> QgsTiledSceneChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;
  const long long indexTileId = node->tileId().uniqueId;

  // fetching of hierarchy is handled by canCreateChildren() + prepareChildren()
  Q_ASSERT( mIndex.childAvailability( indexTileId ) != Qgis::TileChildrenAvailability::NeedFetching );

  const QVector<long long> childIds = mIndex.childTileIds( indexTileId );
  for ( long long childId : childIds )
  {
    const QgsChunkNodeId chId( childId );
    QgsTiledSceneTile t = mIndex.getTile( childId );

    // first check if this node should be even considered
    // XXX: This check doesn't work for Quantized Mesh layers and possibly some
    // Cesium 3D tiles as well. For now this hack is in place to make sure both
    // work in practice.
    if ( t.metadata()["contentFormat"] == u"cesiumtiles"_s
         && mRenderContext.crs().type() != Qgis::CrsType::Geocentric
         && hasLargeBounds( t, mBoundsTransform ) )
    {
      // if the tile is huge, let's try to see if our scene is actually inside
      // (if not, let' skip this child altogether!)
      // TODO: make OBB of our scene in ECEF rather than just using center of the scene?
      const QgsOrientedBox3D obb = t.boundingVolume().box();
      const QgsPointXY c = mRenderContext.extent().center();
      const QgsVector3D cEcef = mBoundsTransform.transform( QgsVector3D( c.x(), c.y(), 0 ), Qgis::TransformDirection::Reverse );
      const QgsVector3D ecef2 = cEcef - obb.center();
      const double *half = obb.halfAxes();
      // this is an approximate check anyway, no need for double precision matrix/vector
      QMatrix4x4 rot(
        half[0], half[3], half[6], 0,
        half[1], half[4], half[7], 0,
        half[2], half[5], half[8], 0,
        0, 0, 0, 1
      );
      QVector3D aaa = rot.inverted().map( ecef2.toVector3D() );
      if ( aaa.x() > 1 || aaa.y() > 1 || aaa.z() > 1 || aaa.x() < -1 || aaa.y() < -1 || aaa.z() < -1 )
      {
        continue;
      }
    }

    // fetching of hierarchy is handled by canCreateChildren() + prepareChildren()
    Q_ASSERT( mIndex.childAvailability( childId ) != Qgis::TileChildrenAvailability::NeedFetching );

    QgsChunkNode *nChild = nodeForTile( t, chId, node );
    children.append( nChild );
  }
  return children;
}

bool QgsTiledSceneChunkLoaderFactory::canCreateChildren( QgsChunkNode *node )
{
  long long nodeId = node->tileId().uniqueId;
  if ( mFutureHierarchyFetches.contains( nodeId ) || mPendingHierarchyFetches.contains( nodeId ) )
    return false;

  if ( mIndex.childAvailability( nodeId ) == Qgis::TileChildrenAvailability::NeedFetching )
  {
    mFutureHierarchyFetches.insert( nodeId );
    return false;
  }

  // we need to make sure that if a child tile's content references another tileset JSON,
  // we fetch its hierarchy before a chunk node is created for such child tile - otherwise we
  // end up trying to load tileset JSON file instead of the actual content

  const QVector<long long> childIds = mIndex.childTileIds( nodeId );
  for ( long long childId : childIds )
  {
    if ( mFutureHierarchyFetches.contains( childId ) || mPendingHierarchyFetches.contains( childId ) )
      return false;

    if ( mIndex.childAvailability( childId ) == Qgis::TileChildrenAvailability::NeedFetching )
    {
      mFutureHierarchyFetches.insert( childId );
      return false;
    }
  }
  return true;
}

void QgsTiledSceneChunkLoaderFactory::fetchHierarchyForNode( long long nodeId, QgsChunkNode *origNode )
{
  Q_ASSERT( !mPendingHierarchyFetches.contains( nodeId ) );
  mFutureHierarchyFetches.remove( nodeId );
  mPendingHierarchyFetches.insert( nodeId );

  QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>( this );
  connect( futureWatcher, &QFutureWatcher<void>::finished, this, [this, origNode, nodeId, futureWatcher] {
    mPendingHierarchyFetches.remove( nodeId );
    emit childrenPrepared( origNode );
    futureWatcher->deleteLater();
  } );
  futureWatcher->setFuture( QtConcurrent::run( [this, nodeId] {
    mIndex.fetchHierarchy( nodeId );
  } ) );
}

void QgsTiledSceneChunkLoaderFactory::prepareChildren( QgsChunkNode *node )
{
  long long nodeId = node->tileId().uniqueId;
  if ( mFutureHierarchyFetches.contains( nodeId ) )
  {
    fetchHierarchyForNode( nodeId, node );
    return;
  }

  // we need to make sure that if a child tile's content references another tileset JSON,
  // we fetch its hierarchy before a chunk node is created for such child tile - otherwise we
  // end up trying to load tileset JSON file instead of the actual content

  const QVector<long long> childIds = mIndex.childTileIds( nodeId );
  for ( long long childId : childIds )
  {
    if ( mFutureHierarchyFetches.contains( childId ) )
    {
      fetchHierarchyForNode( childId, node );
    }
  }
}


///

QgsTiledSceneLayerChunkedEntity::QgsTiledSceneLayerChunkedEntity(
  Qgs3DMapSettings *map,
  const QgsTiledSceneIndex &index,
  QgsCoordinateReferenceSystem tileCrs,
  QgsCoordinateReferenceSystem layerCrs,
  double maximumScreenError,
  bool showBoundingBoxes,
  double zValueScale,
  double zValueOffset
)
  : QgsChunkedEntity( map, maximumScreenError, new QgsTiledSceneChunkLoaderFactory( Qgs3DRenderContext::fromMapSettings( map ), index, tileCrs, layerCrs, zValueScale, zValueOffset ), true )
  , mIndex( index )
{
  setShowBoundingBoxes( showBoundingBoxes );
}

QgsTiledSceneLayerChunkedEntity::~QgsTiledSceneLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

int QgsTiledSceneLayerChunkedEntity::pendingJobsCount() const
{
  return QgsChunkedEntity::pendingJobsCount() + static_cast<QgsTiledSceneChunkLoaderFactory *>( mChunkLoaderFactory )->mPendingHierarchyFetches.count();
}

QList<QgsRayCastHit> QgsTiledSceneLayerChunkedEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  Q_UNUSED( context );
  QgsDebugMsgLevel( u"Ray cast on tiled scene layer"_s, 2 );
#ifdef QGISDEBUG
  int nodeUsed = 0;
  int nodesAll = 0;
  int hits = 0;
#endif

  QList<QgsRayCastHit> result;
  float minDist = -1;
  QVector3D intersectionPoint;
  QgsChunkNode *minNode = nullptr;
  int minTriangleIndex = -1;

  const QList<QgsChunkNode *> active = activeNodes();
  for ( QgsChunkNode *node : active )
  {
#ifdef QGISDEBUG
    nodesAll++;
#endif

    QgsAABB nodeBbox = Qgs3DUtils::mapToWorldExtent( node->box3D(), mMapSettings->origin() );

    if ( node->entity() && ( minDist < 0 || nodeBbox.distanceFromPoint( ray.origin() ) < minDist ) && QgsRayCastingUtils::rayBoxIntersection( ray, nodeBbox ) )
    {
#ifdef QGISDEBUG
      nodeUsed++;
#endif
      const QList<Qt3DRender::QGeometryRenderer *> rendLst = node->entity()->findChildren<Qt3DRender::QGeometryRenderer *>();
      for ( Qt3DRender::QGeometryRenderer *rend : rendLst )
      {
        QVector3D nodeIntPoint;
        int triangleIndex = -1;
        QgsGeoTransform *nodeGeoTransform = node->entity()->findChild<QgsGeoTransform *>();
        Q_ASSERT( nodeGeoTransform );
        bool success = QgsRayCastingUtils::rayMeshIntersection( rend, ray, context.maximumDistance(), nodeGeoTransform->matrix(), nodeIntPoint, triangleIndex );
        if ( success )
        {
#ifdef QGISDEBUG
          hits++;
#endif
          float dist = ( ray.origin() - nodeIntPoint ).length();
          if ( minDist < 0 || dist < minDist )
          {
            minDist = dist;
            minNode = node;
            minTriangleIndex = triangleIndex;
            intersectionPoint = nodeIntPoint;
          }
        }
      }
    }
  }

  if ( minDist >= 0 )
  {
    QVariantMap vm;
    QgsTiledSceneTile tile = mIndex.getTile( minNode->tileId().uniqueId );
    // at this point this is mostly for debugging - we may want to change/rename what's returned here
    vm[u"node_id"_s] = tile.id();
    vm[u"node_error"_s] = tile.geometricError();
    vm[u"node_content"_s] = tile.resources().value( u"content"_s );
    vm[u"triangle_index"_s] = minTriangleIndex;

    QgsRayCastHit hit;
    hit.setDistance( minDist );
    hit.setMapCoordinates( mMapSettings->worldToMapCoordinates( intersectionPoint ) );
    hit.setProperties( vm );
    result.append( hit );
  }

  QgsDebugMsgLevel( u"Active Nodes: %1, checked nodes: %2, hits found: %3"_s.arg( nodesAll ).arg( nodeUsed ).arg( hits ), 2 );
  return result;
}

/// @endcond
