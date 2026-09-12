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
#include "qgsfutureutils.h"
#include "qgsgeotransform.h"
#include "qgsgltf3dutils.h"
#include "qgsgltfutils.h"
#include "qgsmaterial3dhandler.h"
#include "qgsquantizedmeshtiles.h"
#include "qgsray3d.h"
#include "qgsraycastcontext.h"
#include "qgsraycastingutils.h"
#include "qgsthreadingutils.h"
#include "qgstiledsceneboundingvolume.h"
#include "qgstiledscenetile.h"

#include <QFuture>
#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QGeometryRenderer>
#include <QtConcurrentRun>

#include "moc_qgstiledscenechunkloader_p.cpp"

using namespace Qt::StringLiterals;

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

QgsTiledSceneChunkLoader::QgsTiledSceneChunkLoader(
  const Qgs3DRenderContext &context, const QgsTiledSceneIndex &index, QgsCoordinateReferenceSystem tileCrs, QgsCoordinateReferenceSystem layerCrs, double zValueScale, double zValueOffset
)
  : mData {
      .mRenderContext = context,
      .mIndex = index,
      .mZValueScale = zValueScale,
      .mZValueOffset = zValueOffset,
      .mLayerCrs = layerCrs,
      .mBoundsTransform = QgsCoordinateTransform( tileCrs, context.crs(), context.transformContext() ),
    }
{}

QFuture<QgsChunkLoaderResult> QgsTiledSceneChunkLoader::loadChunk( QgsChunkNode *node )
{
  const QgsChunkNodeId tileId = node->tileId();
  const QgsVector3D chunkOrigin = node->box3D().center();
  const bool isGlobe = mData.mRenderContext.crs().type() == Qgis::CrsType::Geocentric;
  return QtConcurrent::run( [data = mData, tileId, chunkOrigin, isGlobe]() mutable -> QgsChunkLoaderResult {
    const QgsTiledSceneTile tile = data.mIndex.getTile( tileId.uniqueId );

    // we do not load tiles that are too big when not in globe scene mode...
    // the problem is that their 3D bounding boxes with ECEF coordinates are huge
    // and we are unable to turn them into planar bounding boxes
    if ( !isGlobe && hasLargeBounds( tile, data.mBoundsTransform ) )
      return QgsChunkLoaderResult::sEmpty;

    QString uri = tile.resources().value( u"content"_s ).toString();
    if ( uri.isEmpty() )
    {
      // nothing to show for this tile
      // TODO: can we skip loading it at all?
      return QgsChunkLoaderResult::sEmpty;
    }

    uri = tile.baseUrl().resolved( uri ).toString();
    QByteArray content = data.mIndex.retrieveContent( uri );
    if ( content.isEmpty() )
    {
      // the request probably failed
      // TODO: how can we report it?
      return QgsChunkLoaderResult::sEmpty;
    }

    QgsGltf3DUtils::EntityTransform entityTransform;
    entityTransform.tileTransform = ( tile.transform() ? *tile.transform() : QgsMatrix4x4() );
    entityTransform.chunkOriginTargetCrs = chunkOrigin;
    entityTransform.ecefToTargetCrs = &data.mBoundsTransform;
    entityTransform.zValueScale = data.mZValueScale;
    entityTransform.zValueOffset = data.mZValueOffset;
    entityTransform.gltfUpAxis = static_cast<Qgis::Axis>( tile.metadata().value( u"gltfUpAxis"_s, static_cast<int>( Qgis::Axis::Y ) ).toInt() );

    const QString &format = tile.metadata().value( u"contentFormat"_s ).value<QString>();
    QStringList errors;
    Qt3DCore::QEntity *entity = nullptr;
    if ( format == "quantizedmesh"_L1 )
    {
      try
      {
        QgsQuantizedMeshTile qmTile( content );
        qmTile.removeDegenerateTriangles();
        tinygltf::Model model = qmTile.toGltf( true, 100 );
        entity = QgsGltf3DUtils::parsedGltfToEntity( model, entityTransform, uri, data.mRenderContext, &errors );
      }
      catch ( QgsQuantizedMeshParsingException &ex )
      {
        errors.append( u"Failed to parse tile from '%1'"_s.arg( uri ) );
      }
    }
    else if ( format == "cesiumtiles"_L1 )
    {
      const QVector<QgsCesiumUtils::TileContents> tileContents = QgsCesiumUtils::extractTileContent( content, uri );
      if ( tileContents.isEmpty() )
        return QgsChunkLoaderResult::sEmpty;

      QVector<Qt3DCore::QEntity *> childEntities;

      for ( const QgsCesiumUtils::TileContents &innerContent : tileContents )
      {
        if ( innerContent.gltf.isEmpty() )
          continue;

        QgsGltf3DUtils::EntityTransform innerTransform = entityTransform;
        innerTransform.tileTransform.translate( innerContent.rtcCenter );

        // Check for instancing (i3dm or EXT_mesh_gpu_instancing)
        tinygltf::Model model;
        QString gltfErrors, gltfWarnings;
        if ( !QgsGltfUtils::loadGltfModel( innerContent.gltf, model, &gltfErrors, &gltfWarnings ) )
        {
          errors.append( u"GLTF load error: "_s + gltfErrors );
          continue;
        }

        const QgsMatrix4x4 rawTileTransform = ( tile.transform() ? *tile.transform() : QgsMatrix4x4() );
        const auto instancedPrimitives = QgsCesiumUtils::resolveInstancing( model, innerContent.instancing, innerTransform.gltfUpAxis, rawTileTransform, innerContent.rtcCenter );

        if ( instancedPrimitives.isEmpty() )
        {
          // the common case (b3dm or glTF tile without EXT_mesh_gpu_instancing)
          Qt3DCore::QEntity *e = QgsGltf3DUtils::parsedGltfToEntity( model, innerTransform, uri, data.mRenderContext, &errors );
          if ( e )
            childEntities << e;
        }
        else
        {
          // the instanced case (i3dm or glTF tile with EXT_mesh_gpu_instancing)
          QgsMaterialContext materialContext = QgsMaterialContext::fromRenderContext( data.mRenderContext );
          childEntities << QgsGltf3DUtils::createInstancedEntities( model, instancedPrimitives, innerTransform, uri, materialContext, &errors );

          if ( !innerContent.instancing.has_value() )
          {
            // for glTF tiles with EXT_mesh_gpu_instancing, the model can have a mixture of instanced
            // and non-instanced nodes. Handle the non-instanced nodes here (if any).
            Qt3DCore::QEntity *nonInstancedEntity = QgsGltf3DUtils::parsedGltfToEntity( model, innerTransform, uri, data.mRenderContext, &errors );
            if ( nonInstancedEntity )
              childEntities << nonInstancedEntity;
          }
        }

        if ( childEntities.size() == 1 )
        {
          entity = childEntities[0];
        }
        else
        {
          entity = new Qt3DCore::QEntity;
          for ( Qt3DCore::QEntity *e : childEntities )
            e->setParent( entity );
        }
      }
    }
    else if ( format == "draco"_L1 )
    {
      QgsGltfUtils::I3SNodeContext i3sContext;
      i3sContext.initFromTile( tile, data.mLayerCrs, data.mBoundsTransform.sourceCrs(), data.mRenderContext.transformContext() );

      QString dracoLoadError;
      tinygltf::Model model;
      if ( !QgsGltfUtils::loadDracoModel( content, i3sContext, model, &dracoLoadError ) )
      {
        errors.append( dracoLoadError );
        return QgsChunkLoaderResult::sEmpty;
      }

      entity = QgsGltf3DUtils::parsedGltfToEntity( model, entityTransform, QString(), data.mRenderContext, &errors );
    }
    else
      return QgsChunkLoaderResult::sEmpty; // unsupported tile content type

    // TODO: report errors somewhere?
    if ( !errors.isEmpty() )
    {
      QgsDebugError( "gltf load errors: " + errors.join( '\n' ) );
    }

    if ( entity )
    {
      QgsGeoTransform *transform = new QgsGeoTransform;
      transform->setGeoTranslation( chunkOrigin );
      entity->addComponent( transform );

      entity->moveToThread( QgsApplication::instance()->thread() );
    }

    return QgsChunkLoaderResult { [entity]( Qt3DCore::QEntity *parent ) {
      QGIS_CHECK_MAIN_THREAD_ACCESS
      if ( entity )
        entity->setParent( parent );
      return entity;
    } };
  } );
}

QgsChunkNode *QgsTiledSceneChunkLoader::nodeForTile( const QgsTiledSceneTile &t, const QgsChunkNodeId &nodeId, QgsChunkNode *parent ) const
{
  QgsChunkNode *node = nullptr;
  if ( mData.mRenderContext.crs().type() != Qgis::CrsType::Geocentric && hasLargeBounds( t, mData.mBoundsTransform ) )
  {
    // use the full extent of the scene
    QgsVector3D v0( mData.mRenderContext.extent().xMinimum(), mData.mRenderContext.extent().yMinimum(), -100 );
    QgsVector3D v1( mData.mRenderContext.extent().xMaximum(), mData.mRenderContext.extent().yMaximum(), +100 );
    QgsBox3D box3D( v0, v1 );
    float err = std::min( 1e6, t.geometricError() );
    node = new QgsChunkNode( nodeId, box3D, err, parent );
  }
  else
  {
    QgsBox3D box = t.boundingVolume().bounds( mData.mBoundsTransform );
    box.setZMinimum( box.zMinimum() * mData.mZValueScale + mData.mZValueOffset );
    box.setZMaximum( box.zMaximum() * mData.mZValueScale + mData.mZValueOffset );
    node = new QgsChunkNode( nodeId, box, t.geometricError(), parent );
  }

  node->setRefinementProcess( t.refinementProcess() );
  return node;
}


QgsChunkNode *QgsTiledSceneChunkLoader::createRootNode() const
{
  const QgsTiledSceneTile t = mData.mIndex.rootTile();
  return nodeForTile( t, QgsChunkNodeId( t.id() ), nullptr );
}


QFuture<QVector<QgsChunkNode *>> QgsTiledSceneChunkLoader::createChildren( QgsChunkNode *node )
{
  return prepareChildren( node ).then( this, [this, node]() {
    QVector<QgsChunkNode *> children;
    const long long indexTileId = node->tileId().uniqueId;

    // Already fetched by prepareChildren().
    Q_ASSERT( mData.mIndex.childAvailability( indexTileId ) != Qgis::TileChildrenAvailability::NeedFetching );

    const QVector<long long> childIds = mData.mIndex.childTileIds( indexTileId );
    for ( long long childId : childIds )
    {
      const QgsChunkNodeId chId( childId );
      QgsTiledSceneTile t = mData.mIndex.getTile( childId );

      // first check if this node should be even considered
      // XXX: This check doesn't work for Quantized Mesh layers and possibly some
      // Cesium 3D tiles as well. For now this hack is in place to make sure both
      // work in practice.
      if ( t.metadata()["contentFormat"] == "cesiumtiles"_L1 && mData.mRenderContext.crs().type() != Qgis::CrsType::Geocentric && hasLargeBounds( t, mData.mBoundsTransform ) )
      {
        // if the tile is huge, let's try to see if our scene is actually inside
        // (if not, let' skip this child altogether!)
        // TODO: make OBB of our scene in ECEF rather than just using center of the scene?
        const QgsOrientedBox3D obb = t.boundingVolume().box();
        const QgsPointXY c = mData.mRenderContext.extent().center();
        const QgsVector3D cEcef = mData.mBoundsTransform.transform( QgsVector3D( c.x(), c.y(), 0 ), Qgis::TransformDirection::Reverse );
        const QgsVector3D ecef2 = cEcef - obb.center();
        const double *half = obb.halfAxes();
        // this is an approximate check anyway, no need for double precision matrix/vector
        // clang-format off
      QMatrix4x4 rot(
        half[0], half[3], half[6], 0,
        half[1], half[4], half[7], 0,
        half[2], half[5], half[8], 0,
        0, 0, 0, 1
      );
        // clang-format on
        QVector3D aaa = rot.inverted().map( ecef2.toVector3D() );
        if ( aaa.x() > 1 || aaa.y() > 1 || aaa.z() > 1 || aaa.x() < -1 || aaa.y() < -1 || aaa.z() < -1 )
        {
          continue;
        }
      }

      Q_ASSERT( mData.mIndex.childAvailability( childId ) != Qgis::TileChildrenAvailability::NeedFetching );

      QgsChunkNode *nChild = nodeForTile( t, chId, node );
      children.append( nChild );
    }
    return children;
  } );
}

QFuture<void> QgsTiledSceneChunkLoader::fetchHierarchyForNode( long long nodeId )
{
  if ( auto it = mRunningHierarchyFetches.find( nodeId ); it != mRunningHierarchyFetches.end() )
    return QgsFutureUtils::clone( it.value() );

  QFuture<void> future = QtConcurrent::run( [nodeId, index = mData.mIndex]() mutable { index.fetchHierarchy( nodeId ); } );
  mRunningHierarchyFetches[nodeId] = future;
  return future.then( this, [this, nodeId]() { mRunningHierarchyFetches.remove( nodeId ); } );
}

QFuture<void> QgsTiledSceneChunkLoader::prepareChildren( QgsChunkNode *node )
{
  long long nodeId = node->tileId().uniqueId;

  return fetchHierarchyForNode( nodeId )
    .then(
      this,
      [this, nodeId]() {
        // we need to make sure that if a child tile's content references
        // another tileset JSON, we fetch its hierarchy before a chunk node is
        // created for such child tile - otherwise we end up trying to load
        // tileset JSON file instead of the actual content

        const QVector<long long> childIds = mData.mIndex.childTileIds( nodeId );
        QList<QFuture<void>> fetches;
        for ( long long childId : childIds )
        {
          fetches.append( fetchHierarchyForNode( childId ) );
        }
        return QtFuture::whenAll( fetches.begin(), fetches.end() );
      }
    )
    .unwrap();
}

//////////

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
  : QgsChunkedEntity( map, maximumScreenError, new QgsTiledSceneChunkLoader( Qgs3DRenderContext::fromMapSettings( map ), index, tileCrs, layerCrs, zValueScale, zValueOffset ), true )
  , mIndex( index )
{
  setShowBoundingBoxes( showBoundingBoxes );
}

QgsTiledSceneLayerChunkedEntity::~QgsTiledSceneLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
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

#ifdef QGISDEBUG
  QgsDebugMsgLevel( u"Active Nodes: %1, checked nodes: %2, hits found: %3"_s.arg( nodesAll ).arg( nodeUsed ).arg( hits ), 2 );
#endif
  return result;
}

/// @endcond
