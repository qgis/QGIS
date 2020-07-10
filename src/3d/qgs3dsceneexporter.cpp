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
QVector<T> getIndexData( QByteArray data )
{
  QVector<T> result;
  for ( int i = 0; i < data.size(); i += sizeof( T ) )
  {
    // maybe a problem with indienness can happen?
    T v;
    char *vArr = ( char * )&v;
    for ( T k = 0; k < sizeof( T ); ++k )
    {
      vArr[k] = data.at( i + k );
    }
    result.push_back( v );
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

void Qgs3DSceneExporter::parseVectorLayerEntity( Qt3DCore::QEntity *entity )
{
  if ( entity == nullptr )
    return;
  // We iterate over every component and find components that represent a tessellated geometry
  for ( Qt3DCore::QComponent *c : entity->components() )
  {
    Qt3DRender::QGeometryRenderer *comp = qobject_cast<Qt3DRender::QGeometryRenderer *>( c );
    if ( comp == nullptr )
      continue;
    Qt3DRender::QGeometry *geom = comp->geometry();

    QgsTessellatedPolygonGeometry *tessellated = qobject_cast<QgsTessellatedPolygonGeometry *>( geom );
    if ( tessellated != nullptr )
    {
      processPolygonGeometry( tessellated );
      continue;
    }

//    pocessPoistionAttributes(geom);
  }

  for ( QObject *child : entity->children() )
  {
    Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>( child );
    if ( childEntity != nullptr )
      parseVectorLayerEntity( childEntity );
  }
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

template<typename Component>
Component *findTypedComponent( Qt3DCore::QEntity *entity )
{
  for ( Qt3DCore::QComponent *component : entity->components() )
  {
    Component *typedComponent = qobject_cast<Component *>( component );
    if ( typedComponent != nullptr )
      return typedComponent;
  }
  return nullptr;
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
  QVector<quint16> indexesBuffer = getIndexData<quint16>( indexBytes );

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

void Qgs3DSceneExporter::processPolygonGeometry( QgsTessellatedPolygonGeometry *geom )
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
  // TODO: handle textures
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
