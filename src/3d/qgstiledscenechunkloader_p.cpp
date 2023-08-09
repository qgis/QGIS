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
#include "qgsapplication.h"
#include "qgscesiumutils.h"
#include "qgsgltf3dutils.h"
#include "qgstiledsceneboundingvolume.h"
#include "qgstiledscenetile.h"

#include <QtConcurrentRun>


///@cond PRIVATE

size_t qHash( const QgsChunkNodeId &n )
{
  return n.uniqueId;
}

static bool hasLargeBounds( const QgsTiledSceneTile &t )
{
  if ( t.geometricError() > 1e6 )
    return true;

  const QgsVector3D size = t.boundingVolume().box().size();
  return size.x() > 1e5 || size.y() > 1e5 || size.z() > 1e5;
}


// TODO: move elsewhere
static QString resolveUri( QString uri, const QString &baseUri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( baseUri );

  const QString tileSetUri = dsUri.param( QStringLiteral( "url" ) );
  if ( !tileSetUri.isEmpty() )
  {
    QUrl base( tileSetUri );
    uri = base.resolved( uri ).toString();
  }
  else
  {
    // local files
    if ( uri.startsWith( "./" ) )
    {
      uri.replace( "./", baseUri );
    }
    else if ( QFileInfo( uri ).isRelative() )
    {
      uri = baseUri + uri;
    }
  }
  return uri;
}

///

QgsTiledSceneChunkLoader::QgsTiledSceneChunkLoader( QgsChunkNode *node, const QgsTiledSceneChunkLoaderFactory &factory, const QgsTiledSceneTile &t )
  : QgsChunkLoader( node ), mFactory( factory ), mTile( t )
{
  mFutureWatcher = new QFutureWatcher<void>( this );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QFuture<void> future = QtConcurrent::run( [this]
  {
    // we do not load tiles that are too big - at least for the time being
    // the problem is that their 3D bounding boxes with ECEF coordinates are huge
    // and we are unable to turn them into planar bounding boxes
    if ( hasLargeBounds( mTile ) )
      return;

    QString uri = mTile.resources().value( QStringLiteral( "content" ) ).toString();
    if ( uri.isEmpty() )
    {
      // nothing to show for this tile
      // TODO: can we skip loading it at all?
      return;
    }

    uri = resolveUri( uri, mFactory.mRelativePathBase );
    QByteArray content = mFactory.mIndex.retrieveContent( uri );
    if ( content.isEmpty() )
    {
      // the request probably failed
      // TODO: how can we report it?
      return;
    }

    QByteArray gltfData = QgsCesiumUtils::extractGltfFromTileContent( content );
    if ( gltfData.isEmpty() )
    {
      // unsupported tile content type
      return;
    }

    QgsGltf3DUtils::EntityTransform entityTransform;
    entityTransform.tileTransform = mTile.transform() ? *mTile.transform() : QgsMatrix4x4();
    entityTransform.sceneOriginTargetCrs = mFactory.mMap.origin();
    entityTransform.ecefToTargetCrs = &mFactory.mBoundsTransform;

    QStringList errors;
    mEntity = QgsGltf3DUtils::gltfToEntity( gltfData, entityTransform, uri, &errors );

    if ( mEntity )
      mEntity->moveToThread( QgsApplication::instance()->thread() );

    // TODO: report errors somewhere?
    if ( !errors.isEmpty() )
    {
      QgsDebugError( "gltf load errors: " + errors.join( '\n' ) );
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher->setFuture( future );
}

Qt3DCore::QEntity *QgsTiledSceneChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( !mEntity )
    return new Qt3DCore::QEntity( parent );

  mEntity->setParent( parent );
  return mEntity;
}

///

QgsTiledSceneChunkLoaderFactory::QgsTiledSceneChunkLoaderFactory( const Qgs3DMapSettings &map, QString relativePathBase, const QgsTiledSceneIndex &index )
  : mMap( map ), mRelativePathBase( relativePathBase ), mIndex( index )
{
  mBoundsTransform = QgsCoordinateTransform( QgsCoordinateReferenceSystem( "EPSG:4978" ), mMap.crs(), mMap.transformContext() );
}

QgsChunkLoader *QgsTiledSceneChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  const QgsTiledSceneTile t = mIndex.getTile( node->tileId().uniqueId );

  return new QgsTiledSceneChunkLoader( node, *this, t );
}

// converts box from map coordinates to world coords (also flips [X,Y] to [X,-Z])
static QgsAABB aabbConvert( const QgsBox3D &b0, const QgsVector3D &sceneOriginTargetCrs )
{
  const QgsBox3D b = b0 - sceneOriginTargetCrs;
  return QgsAABB( b.xMinimum(), b.zMinimum(), -b.yMaximum(), b.xMaximum(), b.zMaximum(), -b.yMinimum() );
}

QgsChunkNode *QgsTiledSceneChunkLoaderFactory::nodeForTile( const QgsTiledSceneTile &t, const QgsChunkNodeId &nodeId ) const
{
  if ( hasLargeBounds( t ) )
  {
    // use the full extent of the scene
    QgsVector3D v0 = mMap.mapToWorldCoordinates( QgsVector3D( mMap.extent().xMinimum(), mMap.extent().yMinimum(), -100 ) );
    QgsVector3D v1 = mMap.mapToWorldCoordinates( QgsVector3D( mMap.extent().xMaximum(), mMap.extent().yMaximum(), +100 ) );
    QgsAABB aabb( v0.x(), v0.y(), v0.z(), v1.x(), v1.y(), v1.z() );
    float err = std::min( 1e6, t.geometricError() );
    return new QgsChunkNode( nodeId, aabb, err );
  }
  else
  {
    const QgsBox3D box = t.boundingVolume().bounds( mBoundsTransform );
    const QgsAABB aabb = aabbConvert( box, mMap.origin() );
    return new QgsChunkNode( nodeId, aabb, t.geometricError() );
  }
}


QgsChunkNode *QgsTiledSceneChunkLoaderFactory::createRootNode() const
{
  const QgsTiledSceneTile t = mIndex.rootTile();
  return nodeForTile( t, QgsChunkNodeId( t.id() ) );
}


QVector<QgsChunkNode *> QgsTiledSceneChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;
  const long long indexTileId = node->tileId().uniqueId;

  switch ( mIndex.childAvailability( indexTileId ) )
  {
    case Qgis::TileChildrenAvailability::NoChildren:
      return children;
    case Qgis::TileChildrenAvailability::Available:
      break;
    case Qgis::TileChildrenAvailability::NeedFetching:
      if ( !mIndex.fetchHierarchy( indexTileId ) )
        return children;
      break;
  }

  const QVector< long long > childIds = mIndex.childTileIds( indexTileId );
  for ( long long childId : childIds )
  {
    const QgsChunkNodeId chId( childId );
    QgsTiledSceneTile t = mIndex.getTile( childId );

    // first check if this node should be even considered
    if ( hasLargeBounds( t ) )
    {
      // if the tile is huge, let's try to see if our scene is actually inside
      // (if not, let' skip this child altogether!)
      // TODO: make OBB of our scene in ECEF rather than just using center of the scene?
      const QgsOrientedBox3D obb = t.boundingVolume().box();

      const QgsPointXY c = mMap.extent().center();
      const QgsVector3D cEcef = mBoundsTransform.transform( QgsVector3D( c.x(), c.y(), 0 ), Qgis::TransformDirection::Reverse );
      const QgsVector3D ecef2 = cEcef - obb.center();

      const double *half = obb.halfAxes();

      // this is an approximate check anyway, no need for double precision matrix/vector
      QMatrix4x4 rot(
        half[0], half[3], half[6], 0,
        half[1], half[4], half[7], 0,
        half[2], half[5], half[8], 0,
        0, 0, 0, 1 );
      QVector3D aaa = rot.inverted().map( ecef2.toVector3D() );

      if ( aaa.x() > 1 || aaa.y() > 1 || aaa.z() > 1 ||
           aaa.x() < -1 || aaa.y() < -1 || aaa.z() < -1 )
      {
        continue;
      }
    }

    if ( mIndex.childAvailability( childId ) == Qgis::TileChildrenAvailability::NeedFetching )
    {
      // we need to make sure that if a child tile's content references another tileset JSON,
      // we fetch its hierarchy before a chunk node is created for such child tile - otherwise we
      // end up trying to load tileset JSON file instead of the actual content
      mIndex.fetchHierarchy( childId );
    }

    QgsChunkNode *nChild = nodeForTile( t, chId );
    children.append( nChild );
  }
  return children;
}

///

QgsTiledSceneLayerChunkedEntity::QgsTiledSceneLayerChunkedEntity( const Qgs3DMapSettings &map, QString relativePathBase, const QgsTiledSceneIndex &index, double maximumScreenError, bool showBoundingBoxes )
  : QgsChunkedEntity( maximumScreenError, new QgsTiledSceneChunkLoaderFactory( map, relativePathBase, index ), true )
{
  if ( index.rootTile().refinementProcess() == Qgis::TileRefinementProcess::Additive )
    setUsingAdditiveStrategy( true );
  setShowBoundingBoxes( showBoundingBoxes );
}

QgsTiledSceneLayerChunkedEntity::~QgsTiledSceneLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

/// @endcond
