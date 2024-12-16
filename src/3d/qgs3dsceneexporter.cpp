/***************************************************************************
  qgs3dsceneexporter.cpp
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dsceneexporter.h"
#include "moc_qgs3dsceneexporter.cpp"

#include <QVector>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QComponent>
#include <Qt3DCore/QNode>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QPlaneGeometry>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QTextureImage>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DExtras/QCylinderGeometry>
#include <Qt3DExtras/QConeGeometry>
#include <Qt3DExtras/QSphereGeometry>
#include <Qt3DExtras/QCuboidGeometry>
#include <Qt3DExtras/QTorusGeometry>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QAbstractTextureImage>

#include <QByteArray>
#include <QFile>
#include <QTextStream>

#include "qgsbillboardgeometry.h"
#include "qgsterraintileentity_p.h"
#include "qgsterrainentity.h"
#include "qgschunknode.h"
#include "qgsterraingenerator.h"
#include "qgs3dmapsettings.h"
#include "qgsflatterraingenerator.h"
#include "qgsdemterraingenerator.h"
#include "qgsdemterraintileloader_p.h"
#include "qgsdemterraintilegeometry_p.h"
#include "qgs3dexportobject.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsmeshterraingenerator.h"
#include "qgsmeshterraintileloader_p.h"
#include "qgsvectorlayer.h"
#include "qgsabstract3drenderer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgs3dutils.h"
#include "qgsimagetexture.h"
#include "qgstessellatedpolygongeometry.h"

#include <numeric>

template<typename T>
QVector<T> getAttributeData( Qt3DQAttribute *attribute, const QByteArray &data )
{
  const uint bytesOffset = attribute->byteOffset();
  const uint bytesStride = attribute->byteStride();
  const uint vertexSize = attribute->vertexSize();
  const uint dataSize = static_cast<uint>( data.size() );
  QVector<T> result;

  if ( bytesStride == 0 )
  {
    QgsDebugError( "bytesStride==0, the attribute probably was not set properly" );
    return result;
  }

  const char *pData = data.constData();
  for ( unsigned int i = bytesOffset; i < dataSize; i += bytesStride )
  {
    for ( unsigned int j = 0; j < vertexSize * sizeof( T ); j += sizeof( T ) )
    {
      T v;
      memcpy( &v, pData + i + j, sizeof( T ) );
      result.push_back( v );
    }
  }
  return result;
}

template<typename T>
QVector<uint> _getIndexDataImplementation( const QByteArray &data )
{
  QVector<uint> result;
  const char *pData = data.constData();
  for ( int i = 0; i < data.size(); i += sizeof( T ) )
  {
    T v;
    memcpy( &v, pData + i, sizeof( T ) );
    result.push_back( ( uint ) v );
  }
  return result;
}

QVector<uint> getIndexData( Qt3DQAttribute *indexAttribute, const QByteArray &data )
{
  switch ( indexAttribute->vertexBaseType() )
  {
    case Qt3DQAttribute::VertexBaseType::Int:
      return _getIndexDataImplementation<int>( data );
    case Qt3DQAttribute::VertexBaseType::UnsignedInt:
      return _getIndexDataImplementation<uint>( data );
    case Qt3DQAttribute::VertexBaseType::Short:
      return _getIndexDataImplementation<short>( data );
    case Qt3DQAttribute::VertexBaseType::UnsignedShort:
      return _getIndexDataImplementation<ushort>( data );
    case Qt3DQAttribute::VertexBaseType::Byte:
      return _getIndexDataImplementation<char>( data );
    case Qt3DQAttribute::VertexBaseType::UnsignedByte:
      return _getIndexDataImplementation<uchar>( data );
    default:
      QgsDebugError( "Probably trying to get index data using an attribute that has vertex data" );
      break;
  }
  return QVector<uint>();
}

QByteArray getData( Qt3DQBuffer *buffer )
{
  QByteArray bytes = buffer->data();
  if ( bytes.isNull() )
  {
    QgsDebugError( "QBuffer is null" );
  }
  return bytes;
}

Qt3DQAttribute *findAttribute( Qt3DQGeometry *geometry, const QString &name, Qt3DQAttribute::AttributeType type )
{
  QVector<Qt3DQAttribute *> attributes = geometry->attributes();
  for ( Qt3DQAttribute *attribute : attributes )
  {
    if ( attribute->attributeType() != type )
      continue;
    if ( name.isEmpty() || attribute->name() == name )
      return attribute;
  }
  return nullptr;
}

template<typename Component>
Component *findTypedComponent( Qt3DCore::QEntity *entity )
{
  if ( !entity )
    return nullptr;
  QVector<Qt3DCore::QComponent *> components = entity->components();
  for ( Qt3DCore::QComponent *component : components )
  {
    Component *typedComponent = qobject_cast<Component *>( component );
    if ( typedComponent )
      return typedComponent;
  }
  return nullptr;
}

bool Qgs3DSceneExporter::parseVectorLayerEntity( Qt3DCore::QEntity *entity, QgsVectorLayer *layer )
{
  QgsAbstract3DRenderer *abstractRenderer = layer->renderer3D();
  const QString rendererType = abstractRenderer->type();

  if ( rendererType == "rulebased" )
  {
    int prevSize = mObjects.size();
    // Potential bug: meshes loaded using Qt3DRender::QSceneLoader will probably have wrong scale and translation
    const QList<Qt3DRender::QGeometryRenderer *> renderers = entity->findChildren<Qt3DRender::QGeometryRenderer *>();
    for ( Qt3DRender::QGeometryRenderer *renderer : renderers )
    {
      Qt3DCore::QEntity *parentEntity = qobject_cast<Qt3DCore::QEntity *>( renderer->parent() );
      if ( !parentEntity )
        continue;
      Qgs3DExportObject *object = processGeometryRenderer( renderer, layer->name() + QStringLiteral( "_" ) );
      if ( !object )
        continue;
      if ( mExportTextures )
        processEntityMaterial( parentEntity, object );
      mObjects.push_back( object );
    }
    return mObjects.size() > prevSize;
  }

  else if ( rendererType == "vector" )
  {
    QgsVectorLayer3DRenderer *vectorLayerRenderer = dynamic_cast<QgsVectorLayer3DRenderer *>( abstractRenderer );
    if ( vectorLayerRenderer )
    {
      const QgsAbstract3DSymbol *symbol = vectorLayerRenderer->symbol();
      return symbol->exportGeometries( this, entity, layer->name() + QStringLiteral( "_" ) );
    }
    else
      return false;
  }

  else
  {
    // TODO: handle pointcloud/mesh/etc. layers
    QgsDebugMsgLevel( QStringLiteral( "Type '%1' of layer '%2' is not exportable." ).arg( layer->name(), rendererType ), 2 );
    return false;
  }

  return false;
}

void Qgs3DSceneExporter::processEntityMaterial( Qt3DCore::QEntity *entity, Qgs3DExportObject *object )
{
  Qt3DExtras::QPhongMaterial *phongMaterial = findTypedComponent<Qt3DExtras::QPhongMaterial>( entity );
  if ( phongMaterial )
  {
    QgsPhongMaterialSettings material = Qgs3DUtils::phongMaterialFromQt3DComponent( phongMaterial );
    object->setupMaterial( &material );
  }

  Qt3DExtras::QDiffuseSpecularMaterial *diffuseMapMaterial = findTypedComponent<Qt3DExtras::QDiffuseSpecularMaterial>( entity );
  if ( diffuseMapMaterial )
  {
    const Qt3DRender::QTexture2D *diffuseTexture = diffuseMapMaterial->diffuse().value<Qt3DRender::QTexture2D *>();
    if ( diffuseTexture )
    {
      const QVector<Qt3DRender::QAbstractTextureImage *> textureImages = diffuseTexture->textureImages();
      for ( const Qt3DRender::QAbstractTextureImage *tex : textureImages )
      {
        const QgsImageTexture *imageTexture = dynamic_cast<const QgsImageTexture *>( tex );
        if ( imageTexture )
        {
          const QImage image = imageTexture->getImage();
          object->setTextureImage( image );
          break;
        }
      }
    }
  }
}

void Qgs3DSceneExporter::parseTerrain( QgsTerrainEntity *terrain, const QString &layerName )
{
  Qgs3DMapSettings *settings = terrain->mapSettings();
  if ( !settings->terrainRenderingEnabled() )
    return;

  QgsChunkNode *node = terrain->rootNode();

  QgsTerrainGenerator *generator = settings->terrainGenerator();
  if ( !generator )
    return;
  QgsTerrainTileEntity *terrainTile = nullptr;
  QgsTerrainTextureGenerator *textureGenerator = terrain->textureGenerator();
  textureGenerator->waitForFinished();
  const QSize oldResolution = textureGenerator->textureSize();
  textureGenerator->setTextureSize( QSize( mTerrainTextureResolution, mTerrainTextureResolution ) );
  switch ( generator->type() )
  {
    case QgsTerrainGenerator::Dem:
      terrainTile = getDemTerrainEntity( terrain, node );
      parseDemTile( terrainTile, layerName + QStringLiteral( "_" ) );
      break;
    case QgsTerrainGenerator::Flat:
      terrainTile = getFlatTerrainEntity( terrain, node );
      parseFlatTile( terrainTile, layerName + QStringLiteral( "_" ) );
      break;
    case QgsTerrainGenerator::Mesh:
      terrainTile = getMeshTerrainEntity( terrain, node );
      parseMeshTile( terrainTile, layerName + QStringLiteral( "_" ) );
      break;
    // TODO: implement other terrain types
    case QgsTerrainGenerator::Online:
    case QgsTerrainGenerator::QuantizedMesh:
      break;
  }
  textureGenerator->setTextureSize( oldResolution );
}

QgsTerrainTileEntity *Qgs3DSceneExporter::getFlatTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node )
{
  QgsFlatTerrainGenerator *generator = dynamic_cast<QgsFlatTerrainGenerator *>( terrain->mapSettings()->terrainGenerator() );
  FlatTerrainChunkLoader *flatTerrainLoader = qobject_cast<FlatTerrainChunkLoader *>( generator->createChunkLoader( node ) );
  if ( mExportTextures )
    terrain->textureGenerator()->waitForFinished();
  // the entity we created will be deallocated once the scene exporter is deallocated
  Qt3DCore::QEntity *entity = flatTerrainLoader->createEntity( this );
  QgsTerrainTileEntity *tileEntity = qobject_cast<QgsTerrainTileEntity *>( entity );
  return tileEntity;
}

QgsTerrainTileEntity *Qgs3DSceneExporter::getDemTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node )
{
  // Just create a new tile (we don't need to export exact level of details as in the scene)
  // create the entity synchronously and then it will be deleted once our scene exporter instance is deallocated
  QgsDemTerrainGenerator *generator = dynamic_cast<QgsDemTerrainGenerator *>( terrain->mapSettings()->terrainGenerator()->clone() );
  generator->setResolution( mTerrainResolution );
  QgsDemTerrainTileLoader *loader = qobject_cast<QgsDemTerrainTileLoader *>( generator->createChunkLoader( node ) );
  generator->heightMapGenerator()->waitForFinished();
  if ( mExportTextures )
    terrain->textureGenerator()->waitForFinished();
  QgsTerrainTileEntity *tileEntity = qobject_cast<QgsTerrainTileEntity *>( loader->createEntity( this ) );
  delete generator;
  return tileEntity;
}

QgsTerrainTileEntity *Qgs3DSceneExporter::getMeshTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node )
{
  QgsMeshTerrainGenerator *generator = dynamic_cast<QgsMeshTerrainGenerator *>( terrain->mapSettings()->terrainGenerator() );
  QgsMeshTerrainTileLoader *loader = qobject_cast<QgsMeshTerrainTileLoader *>( generator->createChunkLoader( node ) );
  // TODO: export textures
  QgsTerrainTileEntity *tileEntity = qobject_cast<QgsTerrainTileEntity *>( loader->createEntity( this ) );
  return tileEntity;
}

void Qgs3DSceneExporter::parseFlatTile( QgsTerrainTileEntity *tileEntity, const QString &layerName )
{
  Qt3DRender::QGeometryRenderer *mesh = findTypedComponent<Qt3DRender::QGeometryRenderer>( tileEntity );
  Qt3DCore::QTransform *transform = findTypedComponent<Qt3DCore::QTransform>( tileEntity );

  Qt3DQGeometry *geometry = mesh->geometry();
  Qt3DExtras::QPlaneGeometry *tileGeometry = qobject_cast<Qt3DExtras::QPlaneGeometry *>( geometry );
  if ( !tileGeometry )
  {
    QgsDebugError( "Qt3DExtras::QPlaneGeometry* is expected but something else was given" );
    return;
  }

  // Generate vertice data
  Qt3DQAttribute *positionAttribute = tileGeometry->positionAttribute();
  if ( !positionAttribute )
  {
    QgsDebugError( QString( "Cannot export '%1' - geometry has no position attribute!" ).arg( layerName ) );
    return;
  }
  const QByteArray verticesBytes = getData( positionAttribute->buffer() );
  if ( verticesBytes.isNull() )
  {
    QgsDebugError( QString( "Geometry for '%1' has position attribute with empty data!" ).arg( layerName ) );
    return;
  }
  const QVector<float> positionBuffer = getAttributeData<float>( positionAttribute, verticesBytes );

  // Generate index data
  Qt3DQAttribute *indexAttribute = tileGeometry->indexAttribute();
  if ( !indexAttribute )
  {
    QgsDebugError( QString( "Cannot export '%1' - geometry has no index attribute!" ).arg( layerName ) );
    return;
  }
  const QByteArray indexBytes = getData( indexAttribute->buffer() );
  if ( indexBytes.isNull() )
  {
    QgsDebugError( QString( "Geometry for '%1' has index attribute with empty data!" ).arg( layerName ) );
    return;
  }
  const QVector<uint> indexesBuffer = getIndexData( indexAttribute, indexBytes );

  QString objectNamePrefix = layerName;
  if ( objectNamePrefix != QString() )
    objectNamePrefix += QString();

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "Flat_tile" ) ) );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );
  object->setupPositionCoordinates( positionBuffer, transform->matrix() );
  object->setupFaces( indexesBuffer );

  if ( mExportNormals )
  {
    // Everts
    QVector<float> normalsBuffer;
    for ( int i = 0; i < positionBuffer.size(); i += 3 )
      normalsBuffer << 0.0f << 1.0f << 0.0f;
    object->setupNormalCoordinates( normalsBuffer, transform->matrix() );
  }

  Qt3DQAttribute *texCoordsAttribute = tileGeometry->texCoordAttribute();
  if ( mExportTextures && texCoordsAttribute )
  {
    // Reuse vertex buffer data for texture coordinates
    const QVector<float> texCoords = getAttributeData<float>( texCoordsAttribute, verticesBytes );
    object->setupTextureCoordinates( texCoords );

    QgsTerrainTextureImage *textureImage = tileEntity->textureImage();
    const QImage img = textureImage->getImage();
    object->setTextureImage( img );
  }
}

void Qgs3DSceneExporter::parseDemTile( QgsTerrainTileEntity *tileEntity, const QString &layerName )
{
  Qt3DRender::QGeometryRenderer *mesh = findTypedComponent<Qt3DRender::QGeometryRenderer>( tileEntity );
  Qt3DCore::QTransform *transform = findTypedComponent<Qt3DCore::QTransform>( tileEntity );

  Qt3DQGeometry *geometry = mesh->geometry();
  DemTerrainTileGeometry *tileGeometry = qobject_cast<DemTerrainTileGeometry *>( geometry );
  if ( !tileGeometry )
  {
    QgsDebugError( "DemTerrainTileGeometry* is expected but something else was given" );
    return;
  }

  Qt3DQAttribute *positionAttribute = tileGeometry->positionAttribute();
  const QByteArray positionBytes = positionAttribute->buffer()->data();
  const QVector<float> positionBuffer = getAttributeData<float>( positionAttribute, positionBytes );

  Qt3DQAttribute *indexAttribute = tileGeometry->indexAttribute();
  const QByteArray indexBytes = indexAttribute->buffer()->data();
  const QVector<unsigned int> indexBuffer = getIndexData( indexAttribute, indexBytes );

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( layerName + QStringLiteral( "DEM_tile" ) ) );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );
  object->setupPositionCoordinates( positionBuffer, transform->matrix() );
  object->setupFaces( indexBuffer );

  Qt3DQAttribute *normalsAttributes = tileGeometry->normalAttribute();
  if ( mExportNormals && normalsAttributes )
  {
    const QByteArray normalsBytes = normalsAttributes->buffer()->data();
    const QVector<float> normalsBuffer = getAttributeData<float>( normalsAttributes, normalsBytes );
    object->setupNormalCoordinates( normalsBuffer, transform->matrix() );
  }

  Qt3DQAttribute *texCoordsAttribute = tileGeometry->texCoordsAttribute();
  if ( mExportTextures && texCoordsAttribute )
  {
    const QByteArray texCoordsBytes = texCoordsAttribute->buffer()->data();
    const QVector<float> texCoordsBuffer = getAttributeData<float>( texCoordsAttribute, texCoordsBytes );
    object->setupTextureCoordinates( texCoordsBuffer );

    QgsTerrainTextureImage *textureImage = tileEntity->textureImage();
    const QImage img = textureImage->getImage();
    object->setTextureImage( img );
  }
}

void Qgs3DSceneExporter::parseMeshTile( QgsTerrainTileEntity *tileEntity, const QString &layerName )
{
  QString objectNamePrefix = layerName;
  if ( objectNamePrefix != QString() )
    objectNamePrefix += QStringLiteral( "_" );

  const QList<Qt3DRender::QGeometryRenderer *> renderers = tileEntity->findChildren<Qt3DRender::QGeometryRenderer *>();
  for ( Qt3DRender::QGeometryRenderer *renderer : renderers )
  {
    Qgs3DExportObject *obj = processGeometryRenderer( renderer, objectNamePrefix );
    if ( !obj )
      continue;
    mObjects << obj;
  }
}

QVector<Qgs3DExportObject *> Qgs3DSceneExporter::processInstancedPointGeometry( Qt3DCore::QEntity *entity, const QString &objectNamePrefix )
{
  QVector<Qgs3DExportObject *> objects;
  const QList<Qt3DQGeometry *> geometriesList = entity->findChildren<Qt3DQGeometry *>();
  for ( Qt3DQGeometry *geometry : geometriesList )
  {
    Qt3DQAttribute *positionAttribute = findAttribute( geometry, Qt3DQAttribute::defaultPositionAttributeName(), Qt3DQAttribute::VertexAttribute );
    Qt3DQAttribute *indexAttribute = findAttribute( geometry, QString(), Qt3DQAttribute::IndexAttribute );
    if ( !positionAttribute || !indexAttribute )
    {
      QgsDebugError( QString( "Cannot export '%1' - geometry has no position or index attribute!" ).arg( objectNamePrefix ) );
      continue;
    }

    const QByteArray vertexBytes = positionAttribute->buffer()->data();
    const QByteArray indexBytes = indexAttribute->buffer()->data();
    if ( vertexBytes.isNull() || indexBytes.isNull() )
    {
      QgsDebugError( QString( "Geometry for '%1' has position or index attribute with empty data!" ).arg( objectNamePrefix ) );
      continue;
    }

    const QVector<float> positionData = getAttributeData<float>( positionAttribute, vertexBytes );
    const QVector<uint> indexData = getIndexData( indexAttribute, indexBytes );

    Qt3DQAttribute *instanceDataAttribute = findAttribute( geometry, QStringLiteral( "pos" ), Qt3DQAttribute::VertexAttribute );
    if ( !instanceDataAttribute )
    {
      QgsDebugError( QString( "Cannot export '%1' - geometry has no instanceData attribute!" ).arg( objectNamePrefix ) );
      continue;
    }
    const QByteArray instancePositionBytes = getData( instanceDataAttribute->buffer() );
    if ( instancePositionBytes.isNull() )
    {
      QgsDebugError( QString( "Geometry for '%1' has instanceData attribute with empty data!" ).arg( objectNamePrefix ) );
      continue;
    }
    QVector<float> instancePosition = getAttributeData<float>( instanceDataAttribute, instancePositionBytes );

    for ( int i = 0; i < instancePosition.size(); i += 3 )
    {
      Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "instance_point" ) ) );
      objects.push_back( object );
      QMatrix4x4 instanceTransform;
      instanceTransform.translate( instancePosition[i], instancePosition[i + 1], instancePosition[i + 2] );
      object->setupPositionCoordinates( positionData, instanceTransform );
      object->setupFaces( indexData );

      object->setSmoothEdges( mSmoothEdges );

      Qt3DQAttribute *normalsAttribute = findAttribute( geometry, Qt3DQAttribute::defaultNormalAttributeName(), Qt3DQAttribute::VertexAttribute );
      if ( mExportNormals && normalsAttribute )
      {
        // Reuse vertex bytes
        const QVector<float> normalsData = getAttributeData<float>( normalsAttribute, vertexBytes );
        object->setupNormalCoordinates( normalsData, instanceTransform );
      }
    }
  }

  return objects;
}

QVector<Qgs3DExportObject *> Qgs3DSceneExporter::processSceneLoaderGeometries( Qt3DRender::QSceneLoader *sceneLoader, const QString &objectNamePrefix )
{
  QVector<Qgs3DExportObject *> objects;
  Qt3DCore::QEntity *sceneLoaderParent = qobject_cast<Qt3DCore::QEntity *>( sceneLoader->parent() );
  Qt3DCore::QTransform *entityTransform = findTypedComponent<Qt3DCore::QTransform>( sceneLoaderParent );
  QMatrix4x4 sceneTransform;
  if ( entityTransform )
  {
    sceneTransform = entityTransform->matrix();
  }
  QStringList entityNames = sceneLoader->entityNames();
  for ( const QString &entityName : entityNames )
  {
    Qt3DRender::QGeometryRenderer *mesh = qobject_cast<Qt3DRender::QGeometryRenderer *>( sceneLoader->component( entityName, Qt3DRender::QSceneLoader::GeometryRendererComponent ) );
    Qgs3DExportObject *object = processGeometryRenderer( mesh, objectNamePrefix, sceneTransform );
    if ( !object )
      continue;
    objects.push_back( object );
  }
  return objects;
}

Qgs3DExportObject *Qgs3DSceneExporter::processGeometryRenderer( Qt3DRender::QGeometryRenderer *geomRenderer, const QString &objectNamePrefix, const QMatrix4x4 &sceneTransform )
{
  // We only export triangles for now
  if ( geomRenderer->primitiveType() != Qt3DRender::QGeometryRenderer::Triangles )
    return nullptr;

  Qt3DQGeometry *geometry = geomRenderer->geometry();
  if ( !geometry )
    return nullptr;

  // === Compute triangleIndexStartingIndiceToKeep according to duplicated features
  //
  // In the case of polygons, we have multiple feature geometries within the same geometry object (QgsTessellatedPolygonGeometry).
  // The QgsTessellatedPolygonGeometry class holds the list of all feature ids included.
  // To avoid exporting the same geometry (it can be included in multiple QgsTessellatedPolygonGeometry) more than once,
  // we keep a list of already exported fid and compare with the fid of the current QgsTessellatedPolygonGeometry.
  // As we cannot retrieve the specific geometry part for a featureId from the QgsTessellatedPolygonGeometry, we only reject
  // the geometry if all the featureid are already present.
  QVector<std::pair<uint, uint>> triangleIndexStartingIndiceToKeep;
  QgsTessellatedPolygonGeometry *tessGeom = dynamic_cast<QgsTessellatedPolygonGeometry *>( geometry );
  if ( tessGeom )
  {
    QVector<QgsFeatureId> featureIds = tessGeom->featureIds();

    QSet<QgsFeatureId> tempFeatToAdd;
    QVector<uint> triangleIndex = tessGeom->triangleIndexStartingIndices();

    for ( int idx = 0; idx < featureIds.size(); idx++ )
    {
      const QgsFeatureId feat = featureIds[idx];
      if ( !mExportedFeatureIds.contains( feat ) )
      {
        // add the feature (as it was unknown) to temp set and not to the mExportedFeatureIds (as featureIds can have the same id multiple times)
        tempFeatToAdd += feat;

        // keep the feature triangle indexes
        const uint startIdx = triangleIndex[idx] * 3;
        const uint endIdx = idx < triangleIndex.size() - 1 ? triangleIndex[idx + 1] * 3 : std::numeric_limits<uint>::max();

        if ( startIdx < endIdx ) // keep only valid intervals
          triangleIndexStartingIndiceToKeep.append( std::pair<uint, uint>( startIdx, endIdx ) );
      }
    }
    mExportedFeatureIds += tempFeatToAdd;

    if ( triangleIndexStartingIndiceToKeep.isEmpty() ) // all featureid are already exported
    {
      return nullptr;
    }
  }

  // === Compute inherited scale and translation from child entity to parent
  QMatrix4x4 transformMatrix = sceneTransform;
  QObject *parent = geomRenderer->parent();
  while ( parent )
  {
    Qt3DCore::QEntity *entity = qobject_cast<Qt3DCore::QEntity *>( parent );
    Qt3DCore::QTransform *transform = findTypedComponent<Qt3DCore::QTransform>( entity );
    if ( transform )
    {
      transformMatrix = transform->matrix() * transformMatrix;
    }
    parent = parent->parent();
  }

  Qt3DQAttribute *positionAttribute = nullptr;
  Qt3DQAttribute *indexAttribute = nullptr;
  QByteArray indexBytes, vertexBytes;
  QVector<uint> indexDataTmp;
  QVector<uint> indexData;
  QVector<float> positionData;

  // === Extract position data
  positionAttribute = findAttribute( geometry, Qt3DQAttribute::defaultPositionAttributeName(), Qt3DQAttribute::VertexAttribute );
  if ( !positionAttribute )
  {
    QgsDebugError( QString( "Cannot export '%1' - geometry has no position attribute!" ).arg( objectNamePrefix ) );
    return nullptr;
  }

  vertexBytes = getData( positionAttribute->buffer() );
  if ( vertexBytes.isNull() )
  {
    QgsDebugError( QString( "Will not export '%1' as geometry has empty position data!" ).arg( objectNamePrefix ) );
    return nullptr;
  }

  positionData = getAttributeData<float>( positionAttribute, vertexBytes );

  // === Search for face index data
  QVector<Qt3DQAttribute *> attributes = geometry->attributes();
  for ( Qt3DQAttribute *attribute : attributes )
  {
    if ( attribute->attributeType() == Qt3DQAttribute::IndexAttribute )
    {
      indexAttribute = attribute;
      indexBytes = getData( indexAttribute->buffer() );
      if ( indexBytes.isNull() )
      {
        QgsDebugError( QString( "Geometry for '%1' has index attribute with empty data!" ).arg( objectNamePrefix ) );
      }
      else
      {
        indexDataTmp = getIndexData( indexAttribute, indexBytes );
        break;
      }
    }
  }

  // for tessellated polygons that don't have index attributes, build them from positionData
  if ( !indexAttribute )
  {
    for ( uint i = 0; i < static_cast<uint>( positionData.size() / 3 ); ++i )
    {
      indexDataTmp.push_back( i );
    }
  }

  // === Filter face index data if needed
  if ( triangleIndexStartingIndiceToKeep.isEmpty() )
  {
    // when geometry is NOT a QgsTessellatedPolygonGeometry, no filter, take them all
    indexData.append( indexDataTmp );
  }
  else
  {
    // when geometry is a QgsTessellatedPolygonGeometry, filter according to triangleIndexStartingIndiceToKeep
    int intervalIdx = 0;
    const int triangleIndexStartingIndiceToKeepSize = triangleIndexStartingIndiceToKeep.size();
    const uint indexDataTmpSize = static_cast<uint>( indexDataTmp.size() );
    for ( uint i = 0; i < indexDataTmpSize; ++i )
    {
      uint idx = indexDataTmp[static_cast<int>( i )];
      // search for valid triangle index interval
      while ( intervalIdx < triangleIndexStartingIndiceToKeepSize
              && idx > triangleIndexStartingIndiceToKeep[intervalIdx].first
              && idx >= triangleIndexStartingIndiceToKeep[intervalIdx].second )
      {
        intervalIdx++;
      }

      // keep only the one within the triangle index interval
      if ( intervalIdx < triangleIndexStartingIndiceToKeepSize
           && idx >= triangleIndexStartingIndiceToKeep[intervalIdx].first
           && idx < triangleIndexStartingIndiceToKeep[intervalIdx].second )
      {
        indexData.push_back( idx );
      }
    }
  }

  // === Create Qgs3DExportObject
  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "mesh_geometry" ) ) );
  object->setupPositionCoordinates( positionData, transformMatrix );
  object->setupFaces( indexData );

  Qt3DQAttribute *normalsAttribute = findAttribute( geometry, Qt3DQAttribute::defaultNormalAttributeName(), Qt3DQAttribute::VertexAttribute );
  if ( mExportNormals && normalsAttribute )
  {
    // Reuse vertex bytes
    const QVector<float> normalsData = getAttributeData<float>( normalsAttribute, vertexBytes );
    object->setupNormalCoordinates( normalsData, transformMatrix );
  }

  Qt3DQAttribute *texCoordsAttribute = findAttribute( geometry, Qt3DQAttribute::defaultTextureCoordinateAttributeName(), Qt3DQAttribute::VertexAttribute );
  if ( mExportTextures && texCoordsAttribute )
  {
    // Reuse vertex bytes
    const QVector<float> texCoordsData = getAttributeData<float>( texCoordsAttribute, vertexBytes );
    object->setupTextureCoordinates( texCoordsData );
  }

  return object;
}

QVector<Qgs3DExportObject *> Qgs3DSceneExporter::processLines( Qt3DCore::QEntity *entity, const QString &objectNamePrefix )
{
  QVector<Qgs3DExportObject *> objs;
  const QList<Qt3DRender::QGeometryRenderer *> renderers = entity->findChildren<Qt3DRender::QGeometryRenderer *>();
  for ( Qt3DRender::QGeometryRenderer *renderer : renderers )
  {
    if ( renderer->primitiveType() != Qt3DRender::QGeometryRenderer::LineStripAdjacency )
      continue;
    Qt3DQGeometry *geom = renderer->geometry();
    Qt3DQAttribute *positionAttribute = findAttribute( geom, Qt3DQAttribute::defaultPositionAttributeName(), Qt3DQAttribute::VertexAttribute );
    Qt3DQAttribute *indexAttribute = findAttribute( geom, QString(), Qt3DQAttribute::IndexAttribute );
    if ( !positionAttribute || !indexAttribute )
    {
      QgsDebugError( QString( "Cannot export '%1' - geometry has no position or index attribute!" ).arg( objectNamePrefix ) );
      continue;
    }

    const QByteArray vertexBytes = getData( positionAttribute->buffer() );
    const QByteArray indexBytes = getData( indexAttribute->buffer() );
    if ( vertexBytes.isNull() || indexBytes.isNull() )
    {
      QgsDebugError( QString( "Geometry for '%1' has position or index attribute with empty data!" ).arg( objectNamePrefix ) );
      continue;
    }
    const QVector<float> positionData = getAttributeData<float>( positionAttribute, vertexBytes );
    const QVector<uint> indexData = getIndexData( indexAttribute, indexBytes );

    Qgs3DExportObject *exportObject = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "line" ) ) );
    exportObject->setType( Qgs3DExportObject::LineStrip );
    exportObject->setupPositionCoordinates( positionData, QMatrix4x4() );
    exportObject->setupLine( indexData );

    objs.push_back( exportObject );
  }
  return objs;
}

Qgs3DExportObject *Qgs3DSceneExporter::processPoints( Qt3DCore::QEntity *entity, const QString &objectNamePrefix )
{
  QVector<float> points;
  const QList<Qt3DRender::QGeometryRenderer *> renderers = entity->findChildren<Qt3DRender::QGeometryRenderer *>();
  for ( Qt3DRender::QGeometryRenderer *renderer : renderers )
  {
    Qt3DQGeometry *geometry = qobject_cast<QgsBillboardGeometry *>( renderer->geometry() );
    if ( !geometry )
      continue;
    Qt3DQAttribute *positionAttribute = findAttribute( geometry, Qt3DQAttribute::defaultPositionAttributeName(), Qt3DQAttribute::VertexAttribute );
    if ( !positionAttribute )
    {
      QgsDebugError( QString( "Cannot export '%1' - geometry has no position attribute!" ).arg( objectNamePrefix ) );
      continue;
    }
    const QByteArray positionBytes = getData( positionAttribute->buffer() );
    if ( positionBytes.isNull() )
    {
      QgsDebugError( QString( "Geometry for '%1' has position attribute with empty data!" ).arg( objectNamePrefix ) );
      continue;
    }
    const QVector<float> positions = getAttributeData<float>( positionAttribute, positionBytes );
    points << positions;
  }
  Qgs3DExportObject *obj = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "points" ) ) );
  obj->setType( Qgs3DExportObject::Points );
  obj->setupPositionCoordinates( points, QMatrix4x4() );
  return obj;
}

bool Qgs3DSceneExporter::save( const QString &sceneName, const QString &sceneFolderPath, int precision )
{
  if ( mObjects.isEmpty() )
  {
    return false;
  }

  const QString objFilePath = QDir( sceneFolderPath ).filePath( sceneName + QStringLiteral( ".obj" ) );
  const QString mtlFilePath = QDir( sceneFolderPath ).filePath( sceneName + QStringLiteral( ".mtl" ) );

  QFile file( objFilePath );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    QgsDebugError( QStringLiteral( "Scene can not be exported to '%1'. File access error." ).arg( objFilePath ) );
    return false;
  }
  QFile mtlFile( mtlFilePath );
  if ( !mtlFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    QgsDebugError( QStringLiteral( "Scene can not be exported to '%1'. File access error." ).arg( mtlFilePath ) );
    return false;
  }

  float maxfloat = std::numeric_limits<float>::max(), minFloat = std::numeric_limits<float>::lowest();
  float minX = maxfloat, minY = maxfloat, minZ = maxfloat, maxX = minFloat, maxY = minFloat, maxZ = minFloat;
  for ( Qgs3DExportObject *obj : qAsConst( mObjects ) )
  {
    obj->objectBounds( minX, minY, minZ, maxX, maxY, maxZ );
  }

  float diffX = 1.0f, diffY = 1.0f, diffZ = 1.0f;
  diffX = maxX - minX;
  diffY = maxY - minY;
  diffZ = maxZ - minZ;

  const float centerX = ( minX + maxX ) / 2.0f;
  const float centerY = ( minY + maxY ) / 2.0f;
  const float centerZ = ( minZ + maxZ ) / 2.0f;

  const float scale = std::max( diffX, std::max( diffY, diffZ ) );

  QTextStream out( &file );
  // set material library name
  const QString mtlLibName = sceneName + ".mtl";
  out << "mtllib " << mtlLibName << "\n";

  QTextStream mtlOut( &mtlFile );
  for ( Qgs3DExportObject *obj : qAsConst( mObjects ) )
  {
    if ( !obj )
      continue;
    // Set object name
    const QString material = obj->saveMaterial( mtlOut, sceneFolderPath );
    out << "o " << obj->name() << "\n";
    if ( material != QString() )
      out << "usemtl " << material << "\n";
    obj->saveTo( out, scale / mScale, QVector3D( centerX, centerY, centerZ ), precision );
  }

  QgsDebugMsgLevel( QStringLiteral( "Scene exported to '%1'" ).arg( objFilePath ), 2 );
  return true;
}

QString Qgs3DSceneExporter::getObjectName( const QString &name )
{
  QString ret = name;
  if ( usedObjectNamesCounter.contains( name ) )
  {
    ret = QStringLiteral( "%1%2" ).arg( name ).arg( usedObjectNamesCounter[name] );
    usedObjectNamesCounter[name]++;
  }
  else
    usedObjectNamesCounter[name] = 2;
  return ret;
}
