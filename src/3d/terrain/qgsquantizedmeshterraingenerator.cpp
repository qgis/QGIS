/***************************************************************************
  qgsterraingenerator.h
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by David Koňařík
  Email                : dvdkon at konarici dot cz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquantizedmeshterraingenerator.h"
#include "qgschunkloader_p.h"
#include "qgschunknode_p.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmesh3dentity_p.h"
#include "qgsmetalroughmaterial.h"
#include "qgsproject.h"
#include "qgsquantizedmeshdataprovider.h"
#include "qgsquantizedmeshtiles.h"
#include "qgsrectangle.h"
#include "qgsterraintileentity_p.h"
#include "qgsterraintileloader_p.h"
#include "qgstiledsceneindex.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenetile.h"
#include "qgstiles.h"
#include "qgstriangularmesh.h"
#include "qgsgltf3dutils.h"
#include "qgsterrainentity_p.h"
#include "qgs3dmapsettings.h"
#include "qgsvector3d.h"
#include "qgsapplication.h"
#include <qcomponent.h>
#include <qdiffusespecularmaterial.h>
#include <qentity.h>
#include <qglobal.h>
#include <qnamespace.h>
#include <qphongmaterial.h>
#include <qtconcurrentrun.h>
#include <qtexturematerial.h>

///@cond PRIVATE

class QgsQuantizedMeshTerrainChunkLoader : public QgsTerrainTileLoader
{
    Q_OBJECT
  public:
    QgsQuantizedMeshTerrainChunkLoader(
      QgsTerrainEntity *terrain, QgsChunkNode *node, long long tileId, QgsTiledSceneIndex index, const QgsCoordinateTransform &tileCrsToMapCrs );
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  protected:
    virtual void onTextureLoaded() override;

  private:
    QgsTerrainTileEntity *mEntity = nullptr;
    bool mMeshLoaded = false;
    bool mTextureLoaded = false;
    std::mutex mFinishedMutex;
};

QgsQuantizedMeshTerrainChunkLoader::QgsQuantizedMeshTerrainChunkLoader( QgsTerrainEntity *terrain_, QgsChunkNode *node, long long tileId, QgsTiledSceneIndex index, const QgsCoordinateTransform &tileCrsToMapCrs )
  : QgsTerrainTileLoader( terrain_, node )
{
  loadTexture(); // Start loading texture

  // Access terrain only on the original thread.
  Qgs3DMapSettings *map = terrain()->mapSettings();
  double vertScale = map->terrainVerticalScale();
  QgsVector3D mapOrigin = map->origin();
  bool shadingEnabled = map->isTerrainShadingEnabled();

  QThreadPool::globalInstance()->start( [ this, node, tileId, index, tileCrsToMapCrs, vertScale, mapOrigin, shadingEnabled ]()
  {
    if ( tileId == QgsQuantizedMeshIndex::ROOT_TILE_ID )
    {
      // Nothing to load for imaginary root tile
      emit finished();
      return;
    }

    // We need to copy index, since capture makes it const. It's just a wrapped smart pointer anyway.
    QgsTiledSceneIndex index2 = index;
    QgsTiledSceneTile tile = index2.getTile( tileId );

    QString uri = tile.resources().value( QStringLiteral( "content" ) ).toString();
    Q_ASSERT( !uri.isEmpty() );

    uri = tile.baseUrl().resolved( uri ).toString();
    QByteArray content = index2.retrieveContent( uri );

    QgsGltf3DUtils::EntityTransform entityTransform;
    entityTransform.tileTransform = ( tile.transform() ? *tile.transform() : QgsMatrix4x4() );
    entityTransform.sceneOriginTargetCrs = mapOrigin;
    entityTransform.ecefToTargetCrs = &tileCrsToMapCrs;
    entityTransform.gltfUpAxis = static_cast< Qgis::Axis >( tile.metadata().value( QStringLiteral( "gltfUpAxis" ), static_cast< int >( Qgis::Axis::Y ) ).toInt() );

    try
    {
      QgsAABB bbox = node->bbox();
      QgsQuantizedMeshTile qmTile( content );
      qmTile.removeDegenerateTriangles();

      // We now know the exact height range of the tile, set it to the node.
      node->setExactBbox(
        QgsAABB(
          // Note that in the 3D view, Y is up!
          bbox.xMin, qmTile.mHeader.MinimumHeight * vertScale, bbox.zMin,
          bbox.xMax, qmTile.mHeader.MaximumHeight * vertScale, bbox.zMax ) );

      if ( shadingEnabled && qmTile.mNormalCoords.size() == 0 )
      {
        qmTile.generateNormals();
      }

      tinygltf::Model model = qmTile.toGltf( true, 100, true );

      QStringList errors;
      Qt3DCore::QEntity *gltfEntity = QgsGltf3DUtils::parsedGltfToEntity( model, entityTransform, uri, &errors );
      if ( !errors.isEmpty() )
      {
        QgsDebugError( "gltf load errors: " + errors.join( '\n' ) );
        emit finished();
        return;
      }

      QgsTerrainTileEntity *terrainEntity = new QgsTerrainTileEntity( node->tileId() );
      // We count on only having one mesh.
      Q_ASSERT( gltfEntity->children().size() == 1 );
      gltfEntity->children()[0]->setParent( terrainEntity );
      terrainEntity->moveToThread( QgsApplication::instance()->thread() );
      mEntity = terrainEntity;
    }
    catch ( QgsQuantizedMeshParsingException &ex )
    {
      QgsDebugError( QStringLiteral( "Failed to parse tile from '%1'" ).arg( uri ) );
      emit finished();
      return;
    }

    {
      std::lock_guard lock( mFinishedMutex );
      if ( mTextureLoaded )
        emit finished();
      mMeshLoaded = true;
    }
  } );
}

Qt3DCore::QEntity *QgsQuantizedMeshTerrainChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mEntity )
  {
    mEntity->setParent( parent );
    Qt3DRender::QTexture2D *texture = createTexture( mEntity );

    // Copied from part of QgsTerrainTileLoader::createTextureComponent, since we can't use that directly on the GLTF entity.
    Qt3DRender::QMaterial *material = nullptr;
    Qgs3DMapSettings *map = terrain()->mapSettings();
    if ( map->isTerrainShadingEnabled() )
    {
      const QgsPhongMaterialSettings &shadingMaterial = map->terrainShadingMaterial();
      Qt3DExtras::QDiffuseSpecularMaterial *diffuseMapMaterial = new Qt3DExtras::QDiffuseSpecularMaterial;
      diffuseMapMaterial->setDiffuse( QVariant::fromValue( texture ) );
      diffuseMapMaterial->setAmbient( shadingMaterial.ambient() );
      diffuseMapMaterial->setSpecular( shadingMaterial.specular() );
      diffuseMapMaterial->setShininess( shadingMaterial.shininess() );
      material = diffuseMapMaterial;
    }
    else
    {
      Qt3DExtras::QTextureMaterial *textureMaterial = new Qt3DExtras::QTextureMaterial;
      textureMaterial->setTexture( texture );
      material = textureMaterial;
    }
    // Get the child that actually has the mesh and add the texture
    Qt3DCore::QEntity *gltfEntity = mEntity->findChild<Qt3DCore::QEntity *>();
    // Remove default material
    auto oldMaterial = gltfEntity->componentsOfType<QgsMetalRoughMaterial>();
    Q_ASSERT( oldMaterial.size() > 0 );
    gltfEntity->removeComponent( oldMaterial[0] );
    gltfEntity->addComponent( material );
  }
  return mEntity;
}

void QgsQuantizedMeshTerrainChunkLoader::onTextureLoaded()
{
  std::lock_guard lock( mFinishedMutex );
  if ( mMeshLoaded )
    emit finished();
  mTextureLoaded = true;
}

///@endcond

void QgsQuantizedMeshTerrainGenerator::setTerrain( QgsTerrainEntity *t )
{
  mTerrain = t;
  mTileCrsToMapCrs =
    QgsCoordinateTransform(
      mMetadata->mCrs,
      mTerrain->mapSettings()->crs(),
      mTerrain->mapSettings()->transformContext() );
}

QgsTerrainGenerator *QgsQuantizedMeshTerrainGenerator::clone() const
{
  QgsQuantizedMeshTerrainGenerator *clone = new QgsQuantizedMeshTerrainGenerator();
  if ( mIsValid )
    clone->setLayer( layer() );
  else
    clone->mLayerRef = mLayerRef; // Copy just the reference
  return clone;
}

QgsTerrainGenerator::Type QgsQuantizedMeshTerrainGenerator::type() const
{
  return QgsTerrainGenerator::QuantizedMesh;
}

QgsRectangle QgsQuantizedMeshTerrainGenerator::rootChunkExtent() const
{
  return mMetadata->mBoundingVolume.bounds().toRectangle();
}

float QgsQuantizedMeshTerrainGenerator::rootChunkError( const Qgs3DMapSettings &map ) const
{
  Q_UNUSED( map );
  return mMetadata->geometricErrorAtZoom( -1 );
}

void QgsQuantizedMeshTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  hMin = mMetadata->mBoundingVolume.bounds().zMinimum();
  hMax = mMetadata->mBoundingVolume.bounds().xMaximum();
}
float QgsQuantizedMeshTerrainGenerator::heightAt( double x, double y, const Qgs3DRenderContext &context ) const
{
  // TODO: This is the interesting part! We can read the height from the best
  // currently loaded tile, or fetch the most precise tile for the coordinates
  // given, but both have downsides.
  Q_UNUSED( x );
  Q_UNUSED( y );
  Q_UNUSED( context );
  return 0;
}

void QgsQuantizedMeshTerrainGenerator::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( QStringLiteral( "layer" ), mLayerRef.layerId );
}

void QgsQuantizedMeshTerrainGenerator::readXml( const QDomElement &elem )
{
  QgsMapLayerRef layerRef = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );
  // We can't call setLayer yet, the reference is not resolved
  mLayerRef = layerRef;
}

void QgsQuantizedMeshTerrainGenerator::resolveReferences( const QgsProject &project )
{
  mLayerRef.resolve( &project );
  setLayer( layer() );
}

QgsChunkLoader *QgsQuantizedMeshTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  long long tileId = QgsQuantizedMeshIndex::encodeTileId( nodeIdToTile( node->tileId() ) );
  return new QgsQuantizedMeshTerrainChunkLoader( mTerrain, node, tileId, mIndex, mTileCrsToMapCrs );
}

QgsChunkNode *QgsQuantizedMeshTerrainGenerator::createRootNode() const
{
  return new QgsChunkNode(
  {0, 0, 0},
  mRootBbox, // Given to us by setupQuadtree()
  mMetadata->geometricErrorAtZoom( -1 ) );
}

QVector<QgsChunkNode *> QgsQuantizedMeshTerrainGenerator::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;

  for ( auto offset : std::vector<std::pair<int, int>> {{0, 0}, {0, 1}, {1, 0}, {1, 1}} )
  {
    QgsChunkNodeId childId(
      node->tileId().d + 1,
      node->tileId().x * 2 + offset.first,
      node->tileId().y * 2 + offset.second
    );
    QgsTileXYZ tile = nodeIdToTile( childId );
    if ( !mMetadata->containsTile( tile ) )
      continue;

    QgsTileMatrix zoomedTileMatrix = QgsTileMatrix::fromTileMatrix( tile.zoomLevel(), mMetadata->mTileMatrix );
    QgsRectangle extent2d = mTileCrsToMapCrs.transform( zoomedTileMatrix.tileExtent( tile ) );
    Q_ASSERT( mTerrain );
    QgsVector3D corner1 = mTerrain->mapSettings()->mapToWorldCoordinates(
    {extent2d.xMinimum(), extent2d.yMinimum(), mMetadata->dummyZRange.lower()} );
    QgsVector3D corner2 = mTerrain->mapSettings()->mapToWorldCoordinates(
    {extent2d.xMaximum(), extent2d.yMaximum(), mMetadata->dummyZRange.upper()} );
    children.push_back(
      new QgsChunkNode(
        childId,
        QgsAABB(
          corner1.x(), corner1.y(), corner1.z(),
          corner2.x(), corner2.y(), corner2.z() ),
        mMetadata->geometricErrorAtZoom( tile.zoomLevel() ),
        node ) );
  }

  return children;
}

bool QgsQuantizedMeshTerrainGenerator::setLayer( QgsTiledSceneLayer *layer )
{
  if ( !layer )
  {
    mIsValid = false;
    return false;
  }

  mLayerRef = layer;
  const QgsQuantizedMeshDataProvider *provider = qobject_cast<const QgsQuantizedMeshDataProvider *>( layer->dataProvider() );
  if ( !provider )
  {
    QgsDebugError( "QgsQuantizedMeshTerrainGenerator provided with non-QM layer" );
    return false;
  }
  mMetadata = provider->quantizedMeshMetadata();
  mIndex = provider->index();

  mTerrainTilingScheme = QgsTilingScheme( mMetadata->mTileMatrix.extent(), mMetadata->mCrs );

  mIsValid = true;
  return true;
}

QgsTiledSceneLayer *QgsQuantizedMeshTerrainGenerator::layer() const
{
  return qobject_cast<QgsTiledSceneLayer *>( mLayerRef.get() );
}

QgsQuantizedMeshTerrainGenerator::QgsQuantizedMeshTerrainGenerator( QgsMapLayerRef layerRef, const QgsQuantizedMeshMetadata &metadata )
  : mLayerRef( layerRef )
  , mMetadata( metadata )
{
}

QgsTileXYZ QgsQuantizedMeshTerrainGenerator::nodeIdToTile( QgsChunkNodeId nodeId ) const
{
  // nodeId zoom=0 is tile zoom=-1 to get unique root tile
  if ( nodeId.d == 0 )
    return { 0, 0, -1 };
  return
  {
    nodeId.x,
    mMetadata->mTileScheme == QStringLiteral( "tms" )
    ? ( 1 << ( nodeId.d - 1 ) ) - nodeId.y - 1
    : nodeId.y,
    nodeId.d - 1 };
}

#include "qgsquantizedmeshterraingenerator.moc"
