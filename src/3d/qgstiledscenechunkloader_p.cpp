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
#include "qgscesiumutils.h"
#include "qgsgltf3dutils.h"
#include "qgstiledsceneboundingvolume.h"
#include "qgstiledscenetile.h"

#include <QtCore/QBuffer>

///@cond PRIVATE

size_t qHash( const QgsChunkNodeId &n )
{
  return n.d ^ n.x ^ n.y ^ n.z;
}

static bool hasLargeBounds( const QgsTiledSceneTile &t )
{
  if ( t.geometricError() > 1e6 )
    return true;

  switch ( t.boundingVolume()->type() )
  {
    case Qgis::TiledSceneBoundingVolumeType::Region:
    {
      // TODO: is region always lat/lon in degrees?
      QgsBox3D region = static_cast<const QgsTiledSceneBoundingVolumeRegion *>( t.boundingVolume() )->region();
      return region.width() > 15 || region.height() > 15;
    }

    case Qgis::TiledSceneBoundingVolumeType::OrientedBox:
    {
      QgsOrientedBox3D box = static_cast<const QgsTiledSceneBoundingVolumeBox *>( t.boundingVolume() )->box();
      QgsVector3D size = box.size();
      return size.x() > 1e5 || size.y() > 1e5 || size.z() > 1e5;
    }

    case Qgis::TiledSceneBoundingVolumeType::Sphere:
      return static_cast<const QgsTiledSceneBoundingVolumeSphere *>( t.boundingVolume() )->sphere().diameter() > 1e5;
  }

  return false;
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

static QgsChunkNodeId nodeIdFromUuid( const QString &uuid )
{
  QgsChunkNodeId nodeId;
  QUuid nodeUuid = QUuid::fromString( uuid );
  Q_ASSERT( !nodeUuid.isNull() );
  Q_ASSERT( sizeof( QUuid ) == 16 && sizeof( QgsChunkNodeId ) == 16 );
  memcpy( reinterpret_cast<char *>( &nodeId ), reinterpret_cast<const char *>( &nodeUuid ), 16 );
  return nodeId;
}

static QString uuidFromNodeId( const QgsChunkNodeId &nodeId )
{
  QUuid nodeUuid;
  Q_ASSERT( sizeof( QUuid ) == 16 && sizeof( QgsChunkNodeId ) == 16 );
  memcpy( reinterpret_cast<char *>( &nodeUuid ), reinterpret_cast<const char *>( &nodeId ), 16 );
  return nodeUuid.toString();
}

///

QgsTiledSceneChunkLoader::QgsTiledSceneChunkLoader( QgsChunkNode *node, const QgsTiledSceneChunkLoaderFactory *factory, const QgsTiledSceneTile &t )
  : QgsChunkLoader( node ), mFactory( factory ), mTile( t )
{
  qDebug() << "chunk loader " << node->tileId().text();

  // start async (actually just do deferred finish()!)
  // TODO: loading in background thread
  QMetaObject::invokeMethod( this, "finished", Qt::QueuedConnection );
}

Qt3DCore::QEntity *QgsTiledSceneChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  qDebug() << "create entity!" << mNode->tileId().text();

  // we do not load tiles that are too big - at least for the time being
  // the problem is that their 3D bounding boxes with ECEF coordinates are huge
  // and we are unable to turn them into planar bounding boxes
  if ( hasLargeBounds( mTile ) )
  {
    return new Qt3DCore::QEntity( parent );
  }

  QString uri = mTile.resources().value( QStringLiteral( "content" ) ).toString();
  if ( uri.isEmpty() )
  {
    // nothing to show for this tile
    // TODO: can we skip loading it at all?
    return new Qt3DCore::QEntity( parent );
  }

  uri = resolveUri( uri, mFactory->mRelativePathBase );

  qDebug() << "loading: " << uri;
  QByteArray content = mFactory->mIndex.retrieveContent( uri );

  if ( content.isEmpty() )
  {
    // the request probably failed
    // TODO: how can we report it?
    qDebug() << "retrieveContent returned no content";
    return new Qt3DCore::QEntity( parent );
  }

  QByteArray gltfData;
  if ( content.startsWith( QByteArray( "b3dm" ) ) )
  {
    QBuffer buffer( &content );
    buffer.open( QIODevice::ReadOnly );
    gltfData = QgsCesiumUtils::extractGltfFromB3dm( buffer );
  }
  else if ( content.startsWith( QByteArray( "glTF" ) ) )
  {
    gltfData = content;
  }
  else
  {
    // unsupported tile content type
    // TODO: we could extract "b3dm" data from a composite tile ("cmpt")
    qDebug() << "unsupported content";
    return new Qt3DCore::QEntity( parent );
  }

  QgsGltf3DUtils::EntityTransform entityTransform;
  entityTransform.tileTransform = mTile.transform() ? *mTile.transform() : QgsMatrix4x4();
  entityTransform.sceneOriginTargetCrs = mFactory->mMap.origin();
  entityTransform.ecefToTargetCrs = mFactory->mBoundsTransform.get();

  QStringList errors;
  Qt3DCore::QEntity *gltfEntity = QgsGltf3DUtils::gltfToEntity( gltfData, entityTransform, uri, &errors );

  // TODO: report errors somewhere?
  if ( !errors.isEmpty() )
    qDebug() << "gltf load errors: " << errors;

  gltfEntity->setParent( parent );
  return gltfEntity;
}

///

QgsTiledSceneChunkLoaderFactory::QgsTiledSceneChunkLoaderFactory( const Qgs3DMapSettings &map, QString relativePathBase, const QgsTiledSceneIndex &index )
  : mMap( map ), mRelativePathBase( relativePathBase ), mIndex( index )
{
  mBoundsTransform.reset( new QgsCoordinateTransform( QgsCoordinateReferenceSystem( "EPSG:4978" ), mMap.crs(), mMap.transformContext() ) );
  mRegionTransform.reset( new QgsCoordinateTransform( QgsCoordinateReferenceSystem( "EPSG:4979" ), mMap.crs(), mMap.transformContext() ) );
}


QgsChunkLoader *QgsTiledSceneChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  QString id = uuidFromNodeId( node->tileId() );
  QgsTiledSceneTile t = mIndex.getTile( id );

  return new QgsTiledSceneChunkLoader( node, this, t );
}


// converts box from map coordinates to world coords (also flips [X,Y] to [X,-Z])
static QgsAABB aabbConvert( const QgsBox3D &b0, const QgsVector3D &sceneOriginTargetCrs )
{
  QgsBox3D b = b0 - sceneOriginTargetCrs;
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
    //qDebug() << "child" << nodeId.text() << aabb.toString() << err;
    return new QgsChunkNode( nodeId, aabb, err );
  }
  else
  {
    bool isRegion = t.boundingVolume()->type() == Qgis::TiledSceneBoundingVolumeType::Region;
    QgsBox3D box = t.boundingVolume()->bounds( isRegion ? *mRegionTransform.get() : *mBoundsTransform.get() );
    QgsAABB aabb = aabbConvert( box, mMap.origin() );
    //qDebug() << "child" << nodeId.text() << aabb.toString() << t.geometricError();
    return new QgsChunkNode( nodeId, aabb, t.geometricError() );
  }
}


QgsChunkNode *QgsTiledSceneChunkLoaderFactory::createRootNode() const
{
  QgsTiledSceneTile t = mIndex.rootTile();
  QgsChunkNodeId nodeId = nodeIdFromUuid( t.id() );
  return nodeForTile( t, nodeId );
}


QVector<QgsChunkNode *> QgsTiledSceneChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  qDebug() << "create children" << node->tileId().text();
  QVector<QgsChunkNode *> children;
  QString indexTileId = uuidFromNodeId( node->tileId() );

  switch ( mIndex.childAvailability( indexTileId ) )
  {
    case Qgis::TileChildrenAvailability::NoChildren:
      return children;
    case Qgis::TileChildrenAvailability::Available:
      break;
    case Qgis::TileChildrenAvailability::NeedFetching:
      qDebug() << "going to fetch hierarchy!";
      if ( !mIndex.fetchHierarchy( indexTileId ) )
        return children;
      break;
  }

  const QStringList childIds = mIndex.childTileIds( indexTileId );
  for ( const QString &childId : childIds )
  {
    QgsChunkNodeId chId = nodeIdFromUuid( childId );
    QgsTiledSceneTile t = mIndex.getTile( childId );

    // first check if this node should be even considered
    if ( hasLargeBounds( t ) )
    {
      // if the tile is huge, let's try to see if our scene is actually inside
      // (if not, let' skip this child altogether!)
      // TODO: make OBB of our scene in ECEF rather than just using center of the scene?
      if ( t.boundingVolume()->type() == Qgis::TiledSceneBoundingVolumeType::OrientedBox )
      {
        QgsOrientedBox3D obb = static_cast<const QgsTiledSceneBoundingVolumeBox *>( t.boundingVolume() )->box();

        QgsPointXY c = mMap.extent().center();
        QgsVector3D cEcef = mBoundsTransform->transform( QgsVector3D( c.x(), c.y(), 0 ), Qgis::TransformDirection::Reverse );
        QgsVector3D ecef2 = cEcef - obb.center();

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
          qDebug() << "skipping child because our scene is not in it" << chId.text();
          continue;
        }
      }
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
