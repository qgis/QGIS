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

#include <QVector>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QComponent>
#include <Qt3DCore/QNode>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QPlaneGeometry>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMaterial>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QTextureImage>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QBufferDataGenerator>
#include <Qt3DRender/QBufferDataGeneratorPtr>
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

#include "qgstessellatedpolygongeometry.h"
#include "qgsterraintileentity_p.h"
#include "qgsterrainentity_p.h"
#include "qgschunknode_p.h"
#include "qgsterraingenerator.h"
#include "qgs3dmapsettings.h"
#include "qgsflatterraingenerator.h"
#include "qgsdemterraingenerator.h"
#include "qgsdemterraintileloader_p.h"
#include "qgsdemterraintilegeometry_p.h"
#include "qgs3dexportobject.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsmesh3dgeometry_p.h"
#include "qgsmeshlayer.h"
#include "qgsmesh3dentity_p.h"
#include "qgsmeshterraingenerator.h"
#include "qgsvectorlayer.h"
#include "qgsabstract3drenderer.h"
#include "qgsabstractvectorlayer3drenderer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgspolygon3dsymbol.h"
#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgsrulebased3drenderer.h"
#include "qgs3dutils.h"
#include "qgsbillboardgeometry.h"
#include "qgsimagetexture.h"

#include <numeric>

template<typename T>
QVector<T> getAttributeData( Qt3DRender::QAttribute *attribute, const QByteArray &data )
{
  const uint bytesOffset = attribute->byteOffset();
  const uint bytesStride = attribute->byteStride();
  const uint vertexSize = attribute->vertexSize();
  QVector<T> result;

  if ( bytesStride == 0 )
  {
    QgsDebugMsg( "bytesStride==0, the attribute probably was not set properly" );
    return result;
  }

  const char *pData = data.constData();
  for ( int i = bytesOffset; i < data.size(); i += bytesStride )
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

QVector<uint> getIndexData( Qt3DRender::QAttribute *indexAttribute, const QByteArray &data )
{
  switch ( indexAttribute->vertexBaseType() )
  {
    case Qt3DRender::QAttribute::VertexBaseType::Int:
      return _getIndexDataImplementation<int>( data );
    case Qt3DRender::QAttribute::VertexBaseType::UnsignedInt:
      return _getIndexDataImplementation<uint>( data );
    case Qt3DRender::QAttribute::VertexBaseType::Short:
      return _getIndexDataImplementation<short>( data );
    case Qt3DRender::QAttribute::VertexBaseType::UnsignedShort:
      return _getIndexDataImplementation<ushort>( data );
    case Qt3DRender::QAttribute::VertexBaseType::Byte:
      return _getIndexDataImplementation<char>( data );
    case Qt3DRender::QAttribute::VertexBaseType::UnsignedByte:
      return _getIndexDataImplementation<uchar>( data );
    default:
      QgsDebugMsg( "Probably trying to get index data using an attribute that has vertex data" );
      break;
  }
  return QVector<uint>();
}

QByteArray getData( Qt3DRender::QBuffer *buffer )
{
  QByteArray bytes = buffer->data();
  if ( bytes.isNull() )
  {
    QgsDebugMsg( "QBuffer is null" );
  }
  return bytes;
}

Qt3DRender::QAttribute *findAttribute( Qt3DRender::QGeometry *geometry, const QString &name, Qt3DRender::QAttribute::AttributeType type )
{
  for ( Qt3DRender::QAttribute *attribute : geometry->attributes() )
  {
    if ( attribute->attributeType() != type ) continue;
    if ( attribute->name() == name ) return attribute;
  }
  return nullptr;
}

template<typename Component>
Component *findTypedComponent( Qt3DCore::QEntity *entity )
{
  if ( entity == nullptr ) return nullptr;
  for ( Qt3DCore::QComponent *component : entity->components() )
  {
    Component *typedComponent = qobject_cast<Component *>( component );
    if ( typedComponent != nullptr )
      return typedComponent;
  }
  return nullptr;
}

bool Qgs3DSceneExporter::parseVectorLayerEntity( Qt3DCore::QEntity *entity, QgsVectorLayer *layer )
{
  QgsAbstract3DRenderer *abstractRenderer =  layer->renderer3D();
  const QString rendererType = abstractRenderer->type();

  if ( rendererType == "mesh" )
  {
    // TODO: handle mesh layers
  }
  else
  {
    QgsAbstractVectorLayer3DRenderer *abstractVectorRenderer = dynamic_cast< QgsAbstractVectorLayer3DRenderer *>( abstractRenderer );
    if ( rendererType == "rulebased" )
    {
      // Potential bug: meshes loaded using Qt3DRender::QSceneLoader will probably have wrong scale and translation
      const QList<Qt3DRender::QGeometryRenderer *> renderers = entity->findChildren<Qt3DRender::QGeometryRenderer *>();
      for ( Qt3DRender::QGeometryRenderer *renderer : renderers )
      {
        Qt3DCore::QEntity *parentEntity = qobject_cast<Qt3DCore::QEntity *>( renderer->parent() );
        if ( !parentEntity )
          continue;
        Qgs3DExportObject *object = processGeometryRenderer( renderer, layer->name() + QStringLiteral( "_" ) );
        if ( object == nullptr ) continue;
        if ( mExportTextures )
          processEntityMaterial( parentEntity, object );
        mObjects.push_back( object );
      }
      return true;
    }
    else
    {
      QgsVectorLayer3DRenderer *vectorLayerRenderer = dynamic_cast< QgsVectorLayer3DRenderer *>( abstractVectorRenderer );
      if ( vectorLayerRenderer )
      {
        const QgsAbstract3DSymbol *symbol = vectorLayerRenderer->symbol();
        const bool exported = symbol->exportGeometries( this, entity, layer->name() + QStringLiteral( "_" ) );
        return exported;
      }
      else
        return false;
    }
  }
  return false;
}

void Qgs3DSceneExporter::processEntityMaterial( Qt3DCore::QEntity *entity, Qgs3DExportObject *object )
{
  Qt3DExtras::QPhongMaterial *phongMaterial = findTypedComponent<Qt3DExtras::QPhongMaterial>( entity );
  if ( phongMaterial != nullptr )
  {
    QgsPhongMaterialSettings material = Qgs3DUtils::phongMaterialFromQt3DComponent( phongMaterial );
    object->setupMaterial( &material );
  }
  Qt3DExtras::QDiffuseSpecularMaterial *diffuseMapMaterial = findTypedComponent<Qt3DExtras::QDiffuseSpecularMaterial>( entity );

  if ( diffuseMapMaterial != nullptr )
  {
    const QVector<Qt3DRender::QAbstractTextureImage *> textureImages = diffuseMapMaterial->diffuse().value< Qt3DRender::QTexture2D * >()->textureImages();
    QgsImageTexture *imageTexture = nullptr;
    for ( Qt3DRender::QAbstractTextureImage *tex : textureImages )
    {
      imageTexture = dynamic_cast<QgsImageTexture *>( tex );
      if ( imageTexture != nullptr ) break;
    }
    if ( imageTexture != nullptr )
    {
      const QImage image = imageTexture->getImage();
      object->setTextureImage( image );
    }
  }
}

void Qgs3DSceneExporter::parseTerrain( QgsTerrainEntity *terrain, const  QString &layerName )
{
  const Qgs3DMapSettings &settings = terrain->map3D();
  if ( !settings.terrainRenderingEnabled() )
    return;

  QgsChunkNode *node = terrain->rootNode();

  QgsTerrainGenerator *generator = settings.terrainGenerator();
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
    // TODO: implement other terrain types
    case QgsTerrainGenerator::Mesh:
      terrainTile = getMeshTerrainEntity( terrain, node );
      parseMeshTile( terrainTile, layerName + QStringLiteral( "_" ) );
      break;
    case QgsTerrainGenerator::Online:
      break;
  }
  textureGenerator->setTextureSize( oldResolution );
}

QgsTerrainTileEntity *Qgs3DSceneExporter::getFlatTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node )
{
  QgsFlatTerrainGenerator *generator = dynamic_cast<QgsFlatTerrainGenerator *>( terrain->map3D().terrainGenerator() );
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
  QgsDemTerrainGenerator *generator = dynamic_cast<QgsDemTerrainGenerator *>( terrain->map3D().terrainGenerator()->clone() );
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
  QgsMeshTerrainGenerator *generator = dynamic_cast<QgsMeshTerrainGenerator *>( terrain->map3D().terrainGenerator() );
  QgsMeshTerrainTileLoader *loader = qobject_cast<QgsMeshTerrainTileLoader *>( generator->createChunkLoader( node ) );
  // TODO: export textures
  QgsTerrainTileEntity *tileEntity = qobject_cast<QgsTerrainTileEntity *>( loader->createEntity( this ) );
  return tileEntity;
}

void Qgs3DSceneExporter::parseFlatTile( QgsTerrainTileEntity *tileEntity, const QString &layerName )
{
  Qt3DRender::QGeometryRenderer *mesh = findTypedComponent<Qt3DRender::QGeometryRenderer>( tileEntity );
  Qt3DCore::QTransform *transform = findTypedComponent<Qt3DCore::QTransform>( tileEntity );

  Qt3DRender::QGeometry *geometry = mesh->geometry();
  Qt3DExtras::QPlaneGeometry *tileGeometry = qobject_cast<Qt3DExtras::QPlaneGeometry *>( geometry );
  if ( tileGeometry == nullptr )
  {
    QgsDebugMsg( "Qt3DExtras::QPlaneGeometry* is expected but something else was given" );
    return;
  }

  const float scale = transform->scale();
  const QVector3D translation = transform->translation();

  // Generate vertice data
  Qt3DRender::QAttribute *positionAttribute = tileGeometry->positionAttribute();
  const QByteArray verticesBytes = getData( positionAttribute->buffer() );
  const QVector<float> positionBuffer = getAttributeData<float>( positionAttribute, verticesBytes );

  // Generate index data
  Qt3DRender::QAttribute *indexAttribute = tileGeometry->indexAttribute();
  const QByteArray indexBytes = getData( indexAttribute->buffer() );
  const QVector<uint> indexesBuffer = getIndexData( indexAttribute,  indexBytes );

  QString objectNamePrefix = layerName;
  if ( objectNamePrefix != QString() ) objectNamePrefix += QString();

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "Flat_tile" ) ) );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );
  object->setupPositionCoordinates( positionBuffer, scale, translation );
  object->setupFaces( indexesBuffer );

  if ( mExportNormals )
  {
    // Everts
    QVector<float> normalsBuffer;
    for ( int i = 0; i < positionBuffer.size(); i += 3 ) normalsBuffer << 0.0f << 1.0f << 0.0f;
    object->setupNormalCoordinates( normalsBuffer );
  }

  Qt3DRender::QAttribute *texCoordsAttribute = tileGeometry->texCoordAttribute();
  if ( mExportTextures && texCoordsAttribute != nullptr )
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

  Qt3DRender::QGeometry *geometry = mesh->geometry();
  DemTerrainTileGeometry *tileGeometry = qobject_cast<DemTerrainTileGeometry *>( geometry );
  if ( tileGeometry == nullptr )
  {
    QgsDebugMsg( "DemTerrainTileGeometry* is expected but something else was given" );
    return;
  }

  const float scale = transform->scale();
  const QVector3D translation = transform->translation();

  Qt3DRender::QAttribute *positionAttribute = tileGeometry->positionAttribute();
  const QByteArray positionBytes = positionAttribute->buffer()->data();
  const QVector<float> positionBuffer = getAttributeData<float>( positionAttribute, positionBytes );

  Qt3DRender::QAttribute *indexAttribute = tileGeometry->indexAttribute();
  const QByteArray indexBytes = indexAttribute->buffer()->data();
  const QVector<unsigned int> indexBuffer = getIndexData( indexAttribute, indexBytes );

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( layerName + QStringLiteral( "DEM_tile" ) ) );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );
  object->setupPositionCoordinates( positionBuffer, scale, translation );
  object->setupFaces( indexBuffer );

  Qt3DRender::QAttribute *normalsAttributes = tileGeometry->normalAttribute();
  if ( mExportNormals && normalsAttributes != nullptr )
  {
    const QByteArray normalsBytes = normalsAttributes->buffer()->data();
    const QVector<float> normalsBuffer = getAttributeData<float>( normalsAttributes, normalsBytes );
    object->setupNormalCoordinates( normalsBuffer );
  }

  Qt3DRender::QAttribute *texCoordsAttribute = tileGeometry->texCoordsAttribute();
  if ( mExportTextures && texCoordsAttribute != nullptr )
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
  if ( objectNamePrefix != QString() ) objectNamePrefix += QStringLiteral( "_" );

  const QList<Qt3DRender::QGeometryRenderer *> renderers = tileEntity->findChildren<Qt3DRender::QGeometryRenderer *>();
  for ( Qt3DRender::QGeometryRenderer *renderer : renderers )
  {
    Qgs3DExportObject *obj = processGeometryRenderer( renderer, objectNamePrefix );
    if ( obj == nullptr ) continue;
    mObjects << obj;
  }
}

QVector<Qgs3DExportObject *> Qgs3DSceneExporter::processInstancedPointGeometry( Qt3DCore::QEntity *entity, const QString &objectNamePrefix )
{
  QVector<Qgs3DExportObject *> objects;
  const QList<Qt3DRender::QGeometry *> geometriesList =  entity->findChildren<Qt3DRender::QGeometry *>();
  for ( Qt3DRender::QGeometry *geometry : geometriesList )
  {
    Qt3DRender::QAttribute *positionAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultPositionAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
    Qt3DRender::QAttribute *indexAttribute = nullptr;
    for ( Qt3DRender::QAttribute *attribute : geometry->attributes() )
    {
      if ( attribute->attributeType() == Qt3DRender::QAttribute::IndexAttribute )
        indexAttribute = attribute;
    }
    if ( positionAttribute == nullptr || indexAttribute == nullptr )
      continue;
    const QByteArray vertexBytes = getData( positionAttribute->buffer() );
    const QByteArray indexBytes = getData( indexAttribute->buffer() );
    const QVector<float> positionData = getAttributeData<float>( positionAttribute, vertexBytes );
    const QVector<uint> indexData = getIndexData( indexAttribute, indexBytes );

    Qt3DRender::QAttribute *instanceDataAttribute = findAttribute( geometry,  QStringLiteral( "pos" ), Qt3DRender::QAttribute::VertexAttribute );
    const QByteArray instancePositionBytes = getData( instanceDataAttribute->buffer() );
    QVector<float> instancePosition = getAttributeData<float>( instanceDataAttribute, instancePositionBytes );
    for ( int i = 0; i < instancePosition.size(); i += 3 )
    {
      Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "shape_geometry" ) ) );
      objects.push_back( object );
      object->setupPositionCoordinates( positionData, 1.0f, QVector3D( instancePosition[i], instancePosition[i + 1], instancePosition[i + 2] ) );
      object->setupFaces( indexData );

      object->setSmoothEdges( mSmoothEdges );

      Qt3DRender::QAttribute *normalsAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultNormalAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
      if ( mExportNormals && normalsAttribute != nullptr )
      {
        // Reuse vertex bytes
        const QVector<float> normalsData = getAttributeData<float>( normalsAttribute, vertexBytes );
        object->setupNormalCoordinates( normalsData );
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
  float sceneScale = 1.0f;
  QVector3D sceneTranslation( 0.0f, 0.0f, 0.0f );
  if ( entityTransform != nullptr )
  {
    sceneScale = entityTransform->scale();
    sceneTranslation = entityTransform->translation();
  }
  for ( const QString &entityName : sceneLoader->entityNames() )
  {
    Qt3DRender::QGeometryRenderer *mesh = qobject_cast<Qt3DRender::QGeometryRenderer *>( sceneLoader->component( entityName, Qt3DRender::QSceneLoader::GeometryRendererComponent ) );
    Qgs3DExportObject *object = processGeometryRenderer( mesh, objectNamePrefix, sceneScale, sceneTranslation );
    if ( object == nullptr ) continue;
    objects.push_back( object );
  }
  return objects;
}

Qgs3DExportObject *Qgs3DSceneExporter::processGeometryRenderer( Qt3DRender::QGeometryRenderer *mesh, const QString &objectNamePrefix, float sceneScale, QVector3D sceneTranslation )
{
  // We only export triangles for now
  if ( mesh->primitiveType() != Qt3DRender::QGeometryRenderer::Triangles ) return nullptr;

  float scale = 1.0f;
  QVector3D translation( 0.0f, 0.0f, 0.0f );
  QObject *parent = mesh->parent();
  while ( parent != nullptr )
  {
    Qt3DCore::QEntity *entity = qobject_cast<Qt3DCore::QEntity *>( parent );
    Qt3DCore::QTransform *transform = findTypedComponent<Qt3DCore::QTransform>( entity );
    if ( transform != nullptr )
    {
      scale *= transform->scale();
      translation += transform->translation();
    }
    parent = parent->parent();
  }

  Qt3DRender::QGeometry *geometry = mesh->geometry();

  Qt3DRender::QAttribute *positionAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultPositionAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
  Qt3DRender::QAttribute *indexAttribute = nullptr;
  QByteArray indexBytes, vertexBytes;
  QVector<uint> indexData;
  QVector<float> positionData;
  for ( Qt3DRender::QAttribute *attribute : geometry->attributes() )
  {
    if ( attribute->attributeType() == Qt3DRender::QAttribute::IndexAttribute )
      indexAttribute = attribute;
  }

  if ( indexAttribute != nullptr )
  {
    indexBytes = getData( indexAttribute->buffer() );
    indexData = getIndexData( indexAttribute, indexBytes );
  }

  if ( positionAttribute != nullptr )
  {
    vertexBytes = getData( positionAttribute->buffer() );
    positionData = getAttributeData<float>( positionAttribute, vertexBytes );
  }

//   For tessellated polygons that don't have index attributes
  if ( positionAttribute != nullptr && indexAttribute == nullptr )
  {
    for ( int i = 0; i < positionData.size() / 3; ++i )
      indexData.push_back( i );
  }

  if ( positionAttribute == nullptr )
  {
    QgsDebugMsg( "Geometry renderer with null data was being processed" );
    return nullptr;
  }

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "mesh_geometry" ) ) );
  object->setupPositionCoordinates( positionData, scale * sceneScale, translation + sceneTranslation );
  object->setupFaces( indexData );

  Qt3DRender::QAttribute *normalsAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultNormalAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
  if ( mExportNormals && normalsAttribute != nullptr )
  {
    // Reuse vertex bytes
    const QVector<float> normalsData = getAttributeData<float>( normalsAttribute, vertexBytes );
    object->setupNormalCoordinates( normalsData );
  }

  Qt3DRender::QAttribute *texCoordsAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultTextureCoordinateAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
  if ( mExportTextures && texCoordsAttribute != nullptr )
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
    if ( renderer->primitiveType() != Qt3DRender::QGeometryRenderer::LineStripAdjacency ) continue;
    Qt3DRender::QGeometry *geom = renderer->geometry();
    Qt3DRender::QAttribute *positionAttribute = findAttribute( geom, Qt3DRender::QAttribute::defaultPositionAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
    Qt3DRender::QAttribute *indexAttribute = nullptr;
    for ( Qt3DRender::QAttribute *attribute : geom->attributes() )
    {
      if ( attribute->attributeType() == Qt3DRender::QAttribute::IndexAttribute )
      {
        indexAttribute = attribute;
        break;
      }
    }
    if ( positionAttribute == nullptr || indexAttribute == nullptr )
    {
      QgsDebugMsg( "Position or index attribute was not found" );
      continue;
    }

    const QByteArray vertexBytes = getData( positionAttribute->buffer() );
    const QByteArray indexBytes = getData( indexAttribute->buffer() );
    const QVector<float> positionData = getAttributeData<float>( positionAttribute, vertexBytes );
    const QVector<uint> indexData = getIndexData( indexAttribute, indexBytes );

    Qgs3DExportObject *exportObject = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "line" ) ) );
    exportObject->setType( Qgs3DExportObject::LineStrip );
    exportObject->setupPositionCoordinates( positionData );
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
    Qt3DRender::QGeometry *geometry = qobject_cast<QgsBillboardGeometry *>( renderer->geometry() );
    if ( geometry == nullptr )
      continue;
    Qt3DRender::QAttribute *positionAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultPositionAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
    const QByteArray positionBytes = getData( positionAttribute->buffer() );
    if ( positionBytes.size() == 0 )
      continue;
    const QVector<float> positions = getAttributeData<float>( positionAttribute, positionBytes );
    points << positions;
  }
  Qgs3DExportObject *obj = new Qgs3DExportObject( getObjectName( objectNamePrefix + QStringLiteral( "points" ) ) );
  obj->setType( Qgs3DExportObject::Points );
  obj->setupPositionCoordinates( points );
  return obj;
}

void Qgs3DSceneExporter::save( const QString &sceneName, const QString &sceneFolderPath )
{
  const QString objFilePath = QDir( sceneFolderPath ).filePath( sceneName + QStringLiteral( ".obj" ) );
  const QString mtlFilePath = QDir( sceneFolderPath ).filePath( sceneName + QStringLiteral( ".mtl" ) );

  QFile file( objFilePath );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    return;
  QFile mtlFile( mtlFilePath );
  if ( !mtlFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    return;

  float maxfloat = std::numeric_limits<float>::max(), minFloat = std::numeric_limits<float>::lowest();
  float minX = maxfloat, minY = maxfloat, minZ = maxfloat, maxX = minFloat, maxY = minFloat, maxZ = minFloat;
  for ( Qgs3DExportObject *obj : mObjects ) obj->objectBounds( minX, minY, minZ, maxX, maxY, maxZ );

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
  for ( Qgs3DExportObject *obj : mObjects )
  {
    if ( obj == nullptr ) continue;
    // Set object name
    const QString material = obj->saveMaterial( mtlOut, sceneFolderPath );
    out << "o " << obj->name() << "\n";
    if ( material != QString() )
      out << "usemtl " << material << "\n";
    obj->saveTo( out, scale / mScale, QVector3D( centerX, centerY, centerZ ) );
  }
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
