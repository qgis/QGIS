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
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QTextureImage>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QBufferDataGenerator>
#include <Qt3DRender/QBufferDataGeneratorPtr>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QSceneLoader>

#include <Qt3DExtras/QCylinderGeometry>
#include <Qt3DExtras/QConeGeometry>
#include <Qt3DExtras/QSphereGeometry>
#include <Qt3DExtras/QCuboidGeometry>
#include <Qt3DExtras/QTorusGeometry>
#include <Qt3DExtras/QExtrudedTextMesh>

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
#include <Qt3DRender/qbufferdatagenerator.h>
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

#include <numeric>

template<typename T>
QVector<T> getAttributeData( Qt3DRender::QAttribute *attribute, QByteArray data )
{
//  QByteArray data = attribute->buffer()->data();
  uint bytesOffset = attribute->byteOffset();
  uint bytesStride = attribute->byteStride();
  uint vertexSize = attribute->vertexSize();

  QVector<T> result;
  for ( int i = bytesOffset; i < data.size(); i += bytesStride )
  {
    for ( unsigned int j = 0; j < vertexSize * sizeof( T ); j += sizeof( T ) )
    {
      // maybe a problem with indienness can happen?
      T v;
      char *vArr = ( char * )&v;
      for ( unsigned int k = 0; k < sizeof( T ); ++k )
      {
        vArr[k] = data.at( i + j + k );
      }
      result.push_back( v );
    }
  }
  return result;
}

template<typename T>
QVector<uint> getIndexData( QByteArray data )
{
  QVector<uint> result;
  for ( int i = 0; i < data.size(); i += sizeof( T ) )
  {
    // maybe a problem with indienness can happen?
    T v;
    char *vArr = ( char * )&v;
    for ( T k = 0; k < sizeof( T ); ++k )
    {
      vArr[k] = data.at( i + k );
    }
    result.push_back( ( uint ) v );
  }

  return result;
}

Qgs3DSceneExporter::Qgs3DSceneExporter( Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mSmoothEdges( false )
  , mTerrainResolution( 128 )
  , mExportNormals( true )
  , mExportTextures( false )
  , mTerrainTextureResolution( 512 )
  , mScale( 1.0f )
{

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
  QString rendererType = abstractRenderer->type();

  if ( rendererType == "mesh" )
  {
    // TODO: handle mesh layer
  }
  else
  {
    QgsAbstractVectorLayer3DRenderer *abstractVectorRenderer = dynamic_cast< QgsAbstractVectorLayer3DRenderer *>( abstractRenderer );
    if ( rendererType == "rulebased" )
    {
      // TODO: handle rule based renderers
    }
    else
    {
      QgsVectorLayer3DRenderer *vectorLayerRenderer = dynamic_cast< QgsVectorLayer3DRenderer *>( abstractVectorRenderer );
      const QgsAbstract3DSymbol *symbol = vectorLayerRenderer->symbol();
      QString symbolType = symbol->type();
      if ( symbolType == "polygon" )
      {
        const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( symbol );
        QList<QgsTessellatedPolygonGeometry *> geometries = entity->findChildren<QgsTessellatedPolygonGeometry *>();
        for ( QgsTessellatedPolygonGeometry *polygonGeometry : geometries )
          processPolygonGeometry( polygonGeometry, polygonSymbol );
        return geometries.size() != 0;
      }
      else if ( symbolType == "line" )
      {
        const QgsLine3DSymbol *lineSymbol = dynamic_cast<const QgsLine3DSymbol *>( symbol );
        if ( lineSymbol->renderAsSimpleLines() )
        {
          //TODO: handle simple line geometries in some way
        }
        else
        {
          QList<QgsTessellatedPolygonGeometry *> geometries = entity->findChildren<QgsTessellatedPolygonGeometry *>();
          for ( QgsTessellatedPolygonGeometry *lineGeometry : geometries )
            processBufferedLineGeometry( lineGeometry, lineSymbol );
          return geometries.size() != 0;
        }
      }
      else if ( symbolType == "point" )
      {
        const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( symbol );
        if ( pointSymbol->shape() == QgsPoint3DSymbol::Model )
        {
          Qt3DRender::QSceneLoader *sceneLoader = entity->findChild<Qt3DRender::QSceneLoader *>();
          if (sceneLoader != nullptr) processSceneLoaderGeometry( sceneLoader, pointSymbol );
          else {
            QList<Qt3DRender::QMesh *> meshes = entity->findChildren<Qt3DRender::QMesh *>();
            for (Qt3DRender::QMesh * mesh : meshes)
              processMeshGeometry(mesh, pointSymbol);
          }
          return true;
        }
        else if ( pointSymbol->shape() == QgsPoint3DSymbol::Billboard )
        {
        }
        else
        {
          processInstancedPointGeometry( entity, pointSymbol );
          return true;
        }
      }
    }
  }
  return false;
}

void Qgs3DSceneExporter::parseTerrain( QgsTerrainEntity *terrain )
{
  const Qgs3DMapSettings &settings = terrain->map3D();
  QgsChunkNode *node = terrain->rootNode();

  QgsTerrainGenerator *generator = settings.terrainGenerator();
  QgsTerrainTileEntity *terrainTile = nullptr;
  QgsTerrainTextureGenerator *textureGenerator = terrain->textureGenerator();
  QSize oldResolution = textureGenerator->textureSize();
  textureGenerator->setTextureSize( QSize( mTerrainTextureResolution, mTerrainTextureResolution ) );
  switch ( generator->type() )
  {
    case QgsTerrainGenerator::Dem:
      terrainTile = getDemTerrainEntity( terrain, node );
      this->parseDemTile( terrainTile );
      break;
    case QgsTerrainGenerator::Flat:
      terrainTile = getFlatTerrainEntity( terrain, node );
      this->parseFlatTile( terrainTile );
      break;
    // TODO: implement other terrain types
    case QgsTerrainGenerator::Mesh:
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

void Qgs3DSceneExporter::parseFlatTile( QgsTerrainTileEntity *tileEntity )
{
  Qt3DRender::QGeometryRenderer *mesh = findTypedComponent<Qt3DRender::QGeometryRenderer>( tileEntity );
  Qt3DCore::QTransform *transform = findTypedComponent<Qt3DCore::QTransform>( tileEntity );

  Qt3DRender::QGeometry *geometry = mesh->geometry();
  Qt3DExtras::QPlaneGeometry *tileGeometry = qobject_cast<Qt3DExtras::QPlaneGeometry *>( geometry );
  if ( tileGeometry == nullptr )
  {
    qDebug() << "WARNING : " << "Qt3DExtras::QPlaneGeometry* is expected at " << __FILE__ << ":" << __LINE__;
    return;
  }

  float scale = transform->scale();
  QVector3D translation = transform->translation();

  // Generate vertice data
  Qt3DRender::QAttribute *positionAttribute = tileGeometry->positionAttribute();
  Qt3DRender::QBufferDataGeneratorPtr positionDataGenerator =  positionAttribute->buffer()->dataGenerator();
  QByteArray verticesBytes = positionDataGenerator->operator()();
  QVector<float> positionBuffer = getAttributeData<float>( positionAttribute, verticesBytes );

  // Generate index data
  Qt3DRender::QAttribute *indexAttribute = tileGeometry->indexAttribute();
  Qt3DRender::QBufferDataGeneratorPtr indexDataGenerator =  indexAttribute->buffer()->dataGenerator();
  QByteArray indexBytes = indexDataGenerator->operator()();
  QVector<uint> indexesBuffer = getIndexData<quint16>( indexBytes );

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "Flat_tile" ), "", this );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );
  object->setupPositionCoordinates( positionBuffer, indexesBuffer, scale, translation );

  if ( mExportNormals )
  {
    // Evert
    QVector<float> normalsBuffer;
    for ( int i = 0; i < positionBuffer.size(); i += 3 ) normalsBuffer << 0.0f << 1.0f << 0.0f;
    object->setupNormalCoordinates( normalsBuffer );
  }

  if ( mExportTextures )
  {
    // Reuse vertex buffer data for texture coordinates
    Qt3DRender::QAttribute *texCoordsAttribute = tileGeometry->texCoordAttribute();
    QVector<float> texCoords = getAttributeData<float>( texCoordsAttribute, verticesBytes );
    object->setupTextureCoordinates( texCoords );

    QgsTerrainTextureImage *textureImage = tileEntity->textureImage();
    QImage img = textureImage->getImage();
    object->setTextureImage( img );
  }
}

void Qgs3DSceneExporter::parseDemTile( QgsTerrainTileEntity *tileEntity )
{
  Qt3DRender::QGeometryRenderer *mesh = findTypedComponent<Qt3DRender::QGeometryRenderer>( tileEntity );
  Qt3DCore::QTransform *transform = findTypedComponent<Qt3DCore::QTransform>( tileEntity );

  Qt3DRender::QGeometry *geometry = mesh->geometry();
  DemTerrainTileGeometry *tileGeometry = qobject_cast<DemTerrainTileGeometry *>( geometry );
  if ( tileGeometry == nullptr )
  {
    qDebug() << "WARNING : " << "DemTerrainTileGeometry* is expected at " << __FILE__ << ":" << __LINE__;
    return;
  }

  float scale = transform->scale();
  QVector3D translation = transform->translation();

  Qt3DRender::QAttribute *positionAttribute = tileGeometry->positionAttribute();
  QByteArray positionBytes = positionAttribute->buffer()->data();
  QVector<float> positionBuffer = getAttributeData<float>( positionAttribute, positionBytes );

  Qt3DRender::QAttribute *indexAttribute = tileGeometry->indexAttribute();
  QByteArray indexBytes = indexAttribute->buffer()->data();
  QVector<unsigned int> indexBuffer = getIndexData<uint>( indexBytes );

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "DEM_tile" ), "", this );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );
  object->setupPositionCoordinates( positionBuffer, indexBuffer, scale, translation );

  if ( mExportNormals )
  {
    Qt3DRender::QAttribute *normalsAttributes = tileGeometry->normalAttribute();
    QByteArray normalsBytes = normalsAttributes->buffer()->data();
    QVector<float> normalsBuffer = getAttributeData<float>( normalsAttributes, normalsBytes );
    object->setupNormalCoordinates( normalsBuffer );
  }

  if ( mExportTextures )
  {
    Qt3DRender::QAttribute *texCoordsAttribute = tileGeometry->texCoordsAttribute();
    QByteArray texCoordsBytes = texCoordsAttribute->buffer()->data();
    QVector<float> texCoordsBuffer = getAttributeData<float>( texCoordsAttribute, texCoordsBytes );
    object->setupTextureCoordinates( texCoordsBuffer );

    QgsTerrainTextureImage *textureImage = tileEntity->textureImage();
    QImage img = textureImage->getImage();
    object->setTextureImage( img );
  }
}

void Qgs3DSceneExporter::processPolygonGeometry( QgsTessellatedPolygonGeometry *geom, const QgsPolygon3DSymbol *polygonSymbol )
{
  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "polygon_geometry" ), "", this );
  mObjects.push_back( object );

  Qt3DRender::QAttribute *positionAttribute = geom->mPositionAttribute;
  QByteArray positionBytes = positionAttribute->buffer()->data();
  QVector<float> positionData = getAttributeData<float>( positionAttribute, positionBytes );
  object->setupPositionCoordinates( positionData );

  if ( mExportNormals )
  {
    Qt3DRender::QAttribute *normalsAttribute = geom->mNormalAttribute;
    QByteArray normalsBytes = normalsAttribute->buffer()->data();
    QVector<float> normalsData = getAttributeData<float>( normalsAttribute, normalsBytes );
    object->setupNormalCoordinates( normalsData );
  }
  QgsPhongMaterialSettings material =  polygonSymbol->material();
  QColor diffuse = material.diffuse();
  QColor specular = material.specular();
  QColor ambient = material.ambient();
  float shininess = material.shininess();
  object->setMaterialParameter( QString( "Kd" ), QString( "%1 %2 %3" ).arg( diffuse.redF() ).arg( diffuse.greenF() ).arg( diffuse.blueF() ) );
  object->setMaterialParameter( QString( "Ka" ), QString( "%1 %2 %3" ).arg( ambient.redF() ).arg( ambient.greenF() ).arg( ambient.blueF() ) );
  object->setMaterialParameter( QString( "Ks" ), QString( "%1 %2 %3" ).arg( specular.redF() ).arg( specular.greenF() ).arg( specular.blueF() ) );
  object->setMaterialParameter( QString( "Ns" ), QString( "%1" ).arg( shininess ) );
  // TODO: handle textures
}

void Qgs3DSceneExporter::processBufferedLineGeometry( QgsTessellatedPolygonGeometry *geom, const QgsLine3DSymbol *lineSymbol )
{
  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "line_geometry" ), "", this );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );

  Qt3DRender::QAttribute *positionAttribute = geom->mPositionAttribute;
  QByteArray positionBytes = positionAttribute->buffer()->data();
  QVector<float> positionData = getAttributeData<float>( positionAttribute, positionBytes );
  object->setupPositionCoordinates( positionData );

  if ( mExportNormals )
  {
    Qt3DRender::QAttribute *normalsAttribute = geom->mNormalAttribute;
    QByteArray normalsBytes = normalsAttribute->buffer()->data();
    QVector<float> normalsData = getAttributeData<float>( normalsAttribute, normalsBytes );
    object->setupNormalCoordinates( normalsData );
  }

  QgsPhongMaterialSettings material = lineSymbol->material();
  QColor diffuse = material.diffuse();
  QColor specular = material.specular();
  QColor ambient = material.ambient();
  float shininess = material.shininess();
  object->setMaterialParameter( QString( "Kd" ), QString( "%1 %2 %3" ).arg( diffuse.redF() ).arg( diffuse.greenF() ).arg( diffuse.blueF() ) );
  object->setMaterialParameter( QString( "Ka" ), QString( "%1 %2 %3" ).arg( ambient.redF() ).arg( ambient.greenF() ).arg( ambient.blueF() ) );
  object->setMaterialParameter( QString( "Ks" ), QString( "%1 %2 %3" ).arg( specular.redF() ).arg( specular.greenF() ).arg( specular.blueF() ) );
  object->setMaterialParameter( QString( "Ns" ), QString( "%1" ).arg( shininess ) );
}

void Qgs3DSceneExporter::processInstancedPointGeometry( Qt3DCore::QEntity *entity, const QgsPoint3DSymbol *pointSymbol )
{
  QList<Qt3DRender::QGeometry *> geometriesList =  entity->findChildren<Qt3DRender::QGeometry *>();
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
    Qt3DRender::QBufferDataGeneratorPtr vertexDataGenerator = positionAttribute->buffer()->dataGenerator();
    Qt3DRender::QBufferDataGeneratorPtr indexDataGenerator = indexAttribute->buffer()->dataGenerator();
    QByteArray vertexBytes = vertexDataGenerator->operator()();
    QByteArray indexBytes = indexDataGenerator->operator()();
    QVector<float> positionData = getAttributeData<float>( positionAttribute, vertexBytes );
    QVector<uint> indexData = getIndexData<quint16>( indexBytes );

    Qt3DRender::QAttribute *instanceDataAttribute = findAttribute( geometry,  QStringLiteral( "pos" ), Qt3DRender::QAttribute::VertexAttribute );
    Qt3DRender::QBuffer *instanceDataBuffer = instanceDataAttribute->buffer();
    QVector<float> instancePosition = getAttributeData<float>( instanceDataAttribute, instanceDataBuffer->data() );
    for ( int i = 0; i < instancePosition.size(); i += 3 )
    {
      Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "shape_geometry" ), "", this );
      mObjects.push_back( object );
      object->setupPositionCoordinates( positionData, indexData, 1.0f, QVector3D( instancePosition[i], instancePosition[i + 1], instancePosition[i + 2] ) );

      object->setSmoothEdges( mSmoothEdges );

      if ( mExportNormals )
      {
        Qt3DRender::QAttribute *normalsAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultNormalAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
        // Reuse vertex bytes
        QVector<float> normalsData = getAttributeData<float>( normalsAttribute, vertexBytes );
        object->setupNormalCoordinates( normalsData );
      }

      QgsPhongMaterialSettings material = pointSymbol->material();
      QColor diffuse = material.diffuse();
      QColor specular = material.specular();
      QColor ambient = material.ambient();
      float shininess = material.shininess();
      object->setMaterialParameter( QString( "Kd" ), QString( "%1 %2 %3" ).arg( diffuse.redF() ).arg( diffuse.greenF() ).arg( diffuse.blueF() ) );
      object->setMaterialParameter( QString( "Ka" ), QString( "%1 %2 %3" ).arg( ambient.redF() ).arg( ambient.greenF() ).arg( ambient.blueF() ) );
      object->setMaterialParameter( QString( "Ks" ), QString( "%1 %2 %3" ).arg( specular.redF() ).arg( specular.greenF() ).arg( specular.blueF() ) );
      object->setMaterialParameter( QString( "Ns" ), QString( "%1" ).arg( shininess ) );
    }
  }
}

void Qgs3DSceneExporter::processSceneLoaderGeometry( Qt3DRender::QSceneLoader *sceneLoader, const QgsPoint3DSymbol *pointSymbol )
{
  Qt3DCore::QEntity *sceneLoaderParent = qobject_cast<Qt3DCore::QEntity *>( sceneLoader->parent() );
  Qt3DCore::QTransform *entityTransform = findTypedComponent<Qt3DCore::QTransform>( sceneLoaderParent );
  float entityScale = 1.0f;
  QVector3D entityTranslation( 0.0f, 0.0f, 0.0f );
  if ( entityTransform != nullptr )
  {
    entityScale = entityTransform->scale();
    entityTranslation = entityTransform->translation();
  }
  for ( QString entityName : sceneLoader->entityNames() )
  {
    Qt3DRender::QGeometryRenderer *mesh = qobject_cast<Qt3DRender::QGeometryRenderer *>( sceneLoader->component( entityName, Qt3DRender::QSceneLoader::GeometryRendererComponent ) );
    Qt3DRender::QGeometry *geometry = mesh->geometry();

    Qt3DRender::QAttribute *positionAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultPositionAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
    Qt3DRender::QAttribute *indexAttribute = nullptr;
    for ( Qt3DRender::QAttribute *attribute : geometry->attributes() )
    {
      if ( attribute->attributeType() == Qt3DRender::QAttribute::IndexAttribute )
        indexAttribute = attribute;
    }
    if ( positionAttribute == nullptr || indexAttribute == nullptr )
    {
      qDebug() << "Mesh with null data";
      continue;
    }
    Qt3DRender::QBufferDataGeneratorPtr vertexDataGenerator = positionAttribute->buffer()->dataGenerator();
    Qt3DRender::QBufferDataGeneratorPtr indexDataGenerator = indexAttribute->buffer()->dataGenerator();

    QByteArray vertexBytes;
    QByteArray indexBytes;
    if ( vertexDataGenerator.isNull() ) vertexBytes = positionAttribute->buffer()->data();
    else vertexBytes = vertexDataGenerator->operator()();
    if ( indexDataGenerator.isNull() ) indexBytes = indexAttribute->buffer()->data();
    else indexBytes = indexDataGenerator->operator()();

    QVector<float> positionData = getAttributeData<float>( positionAttribute, vertexBytes );
    QVector<uint> indexData;
    if ( indexAttribute->vertexBaseType() == Qt3DRender::QAttribute::VertexBaseType::UnsignedInt ) indexData = getIndexData<quint32>( indexBytes );
    if ( indexAttribute->vertexBaseType() == Qt3DRender::QAttribute::VertexBaseType::UnsignedShort ) indexData = getIndexData<quint16>( indexBytes );
    Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "shape_geometry" ), "", this );
    mObjects.push_back( object );

    Qt3DCore::QTransform *transform = qobject_cast<Qt3DCore::QTransform *>( sceneLoader->component( entityName, Qt3DRender::QSceneLoader::TransformComponent ) );
    float scale = 1.0f;
    QVector3D translation( 0.0f, 0.0f, 0.0f );

    if ( transform != nullptr )
    {
      scale = transform->scale();
      translation = transform->translation();
    }

    object->setupPositionCoordinates( positionData, indexData, scale * entityScale, translation + entityTranslation );

    object->setSmoothEdges( mSmoothEdges );

    if ( mExportNormals )
    {
      Qt3DRender::QAttribute *normalsAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultNormalAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
      // Reuse vertex bytes
      QVector<float> normalsData = getAttributeData<float>( normalsAttribute, vertexBytes );
      object->setupNormalCoordinates( normalsData );
    }

    QgsPhongMaterialSettings material = pointSymbol->material();
    QColor diffuse = material.diffuse();
    QColor specular = material.specular();
    QColor ambient = material.ambient();
    float shininess = material.shininess();
    object->setMaterialParameter( QString( "Kd" ), QString( "%1 %2 %3" ).arg( diffuse.redF() ).arg( diffuse.greenF() ).arg( diffuse.blueF() ) );
    object->setMaterialParameter( QString( "Ka" ), QString( "%1 %2 %3" ).arg( ambient.redF() ).arg( ambient.greenF() ).arg( ambient.blueF() ) );
    object->setMaterialParameter( QString( "Ks" ), QString( "%1 %2 %3" ).arg( specular.redF() ).arg( specular.greenF() ).arg( specular.blueF() ) );
    object->setMaterialParameter( QString( "Ns" ), QString( "%1" ).arg( shininess ) );
  }
}

void Qgs3DSceneExporter::processMeshGeometry( Qt3DRender::QMesh *mesh, const QgsPoint3DSymbol *pointSymbol )
{
  Qt3DCore::QEntity *meshParent = qobject_cast<Qt3DCore::QEntity *>( mesh->parent() );
  Qt3DCore::QTransform *entityTransform = findTypedComponent<Qt3DCore::QTransform>( meshParent );
  float entityScale = 1.0f;
  QVector3D entityTranslation( 0.0f, 0.0f, 0.0f );
  if ( entityTransform != nullptr )
  {
    entityScale = entityTransform->scale();
    entityTranslation = entityTransform->translation();
  }
  Qt3DRender::QGeometry *geometry = mesh->geometry();

  Qt3DRender::QAttribute *positionAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultPositionAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
  Qt3DRender::QAttribute *indexAttribute = nullptr;
  for ( Qt3DRender::QAttribute *attribute : geometry->attributes() )
  {
    if ( attribute->attributeType() == Qt3DRender::QAttribute::IndexAttribute )
      indexAttribute = attribute;
  }
  if ( positionAttribute == nullptr || indexAttribute == nullptr )
  {
    qDebug() << "Mesh with null data";
    return;
  }
  Qt3DRender::QBufferDataGeneratorPtr vertexDataGenerator = positionAttribute->buffer()->dataGenerator();
  Qt3DRender::QBufferDataGeneratorPtr indexDataGenerator = indexAttribute->buffer()->dataGenerator();

  QByteArray vertexBytes;
  QByteArray indexBytes;
  if ( vertexDataGenerator.isNull() ) vertexBytes = positionAttribute->buffer()->data();
  else vertexBytes = vertexDataGenerator->operator()();
  if ( indexDataGenerator.isNull() ) indexBytes = indexAttribute->buffer()->data();
  else indexBytes = indexDataGenerator->operator()();

  QVector<float> positionData = getAttributeData<float>( positionAttribute, vertexBytes );
  QVector<uint> indexData;
  if ( indexAttribute->vertexBaseType() == Qt3DRender::QAttribute::VertexBaseType::UnsignedInt ) indexData = getIndexData<quint32>( indexBytes );
  if ( indexAttribute->vertexBaseType() == Qt3DRender::QAttribute::VertexBaseType::UnsignedShort ) indexData = getIndexData<quint16>( indexBytes );
  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "shape_geometry" ), "", this );
  mObjects.push_back( object );

  object->setupPositionCoordinates( positionData, indexData, entityScale, entityTranslation );

  object->setSmoothEdges( mSmoothEdges );

  if ( mExportNormals )
  {
    Qt3DRender::QAttribute *normalsAttribute = findAttribute( geometry, Qt3DRender::QAttribute::defaultNormalAttributeName(), Qt3DRender::QAttribute::VertexAttribute );
    // Reuse vertex bytes
    QVector<float> normalsData = getAttributeData<float>( normalsAttribute, vertexBytes );
    object->setupNormalCoordinates( normalsData );
  }

  QgsPhongMaterialSettings material = pointSymbol->material();
  QColor diffuse = material.diffuse();
  QColor specular = material.specular();
  QColor ambient = material.ambient();
  float shininess = material.shininess();
  object->setMaterialParameter( QString( "Kd" ), QString( "%1 %2 %3" ).arg( diffuse.redF() ).arg( diffuse.greenF() ).arg( diffuse.blueF() ) );
  object->setMaterialParameter( QString( "Ka" ), QString( "%1 %2 %3" ).arg( ambient.redF() ).arg( ambient.greenF() ).arg( ambient.blueF() ) );
  object->setMaterialParameter( QString( "Ks" ), QString( "%1 %2 %3" ).arg( specular.redF() ).arg( specular.greenF() ).arg( specular.blueF() ) );
  object->setMaterialParameter( QString( "Ns" ), QString( "%1" ).arg( shininess ) );
}

void Qgs3DSceneExporter::save( const QString &sceneName, const QString &sceneFolderPath )
{
  QString objFilePath = QDir( sceneFolderPath ).filePath( sceneName + ".obj" );
  QString mtlFilePath = QDir( sceneFolderPath ).filePath( sceneName + ".mtl" );

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

  float centerX = ( minX + maxX ) / 2.0f;
  float centerY = ( minY + maxY ) / 2.0f;
  float centerZ = ( minZ + maxZ ) / 2.0f;

  float scale = std::max( diffX, std::max( diffY, diffZ ) );

  QTextStream out( &file );
  // set material library name
  QString mtlLibName = sceneName + ".mtl";
  out << "mtllib " << mtlLibName << "\n";

  QTextStream mtlOut( &mtlFile );
  for ( Qgs3DExportObject *obj : mObjects )
  {
    // Set object name
    QString material = obj->saveMaterial( mtlOut, sceneFolderPath );
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
    ret = QString( "%1%2" ).arg( name ).arg( usedObjectNamesCounter[name] );
    usedObjectNamesCounter[name]++;
  }
  else
    usedObjectNamesCounter[name] = 2;
  return ret;
}
