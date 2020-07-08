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

#include <numeric>

template<typename T>
QVector<T> getAttributeData( Qt3DRender::QAttribute *attribute )
{
  QByteArray data = attribute->buffer()->data();
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

template<>
QVector<unsigned int> getAttributeData<unsigned int>( Qt3DRender::QAttribute *attribute )
{
  QByteArray data = attribute->buffer()->data();

  QVector<unsigned int> result;
  for ( int i = 0; i < data.size(); i += sizeof( unsigned int ) )
  {
    // maybe a problem with indienness can happen?
    unsigned int v;
    char *vArr = ( char * )&v;
    for ( int k = 0; k < ( int )sizeof( unsigned int ); ++k )
    {
      vArr[k] = data.at( i + k );
    }
    result.push_back( v );
  }

  return result;
}

QVector<float> createPlaneVertexData( float w, float h, const QSize &resolution )
{
  // Taken from Qt3D source with some modifications
  Q_ASSERT( w > 0.0f );
  Q_ASSERT( h > 0.0f );
  Q_ASSERT( resolution.width() >= 2 );
  Q_ASSERT( resolution.height() >= 2 );

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos, vec2 texCoord, vec3 normal, vec4 tangent
  QVector<float> data;

  const float x0 = -w / 2.0f;
  const float z0 = -h / 2.0f;
  const float dx = w / ( resolution.width() - 1 );
  const float dz = h / ( resolution.height() - 1 );

  // Iterate over z
  for ( int j = 0; j < resolution.height(); ++j )
  {
    const float z = z0 + static_cast<float>( j ) * dz;

    // Iterate over x
    for ( int i = 0; i < resolution.width(); ++i )
    {
      const float x = x0 + static_cast<float>( i ) * dx;

      // position
      data.push_back( x );
      data.push_back( 0.0 );
      data.push_back( z );
    }
  }

  return data;
}

QVector<float> createPlaneTexCoordsData( float w, float h, const QSize &resolution, bool mirrored )
{
  Q_ASSERT( w > 0.0f );
  Q_ASSERT( h > 0.0f );
  Q_ASSERT( resolution.width() >= 2 );
  Q_ASSERT( resolution.height() >= 2 );

  const int nVerts = resolution.width() * resolution.height();

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos, vec2 texCoord, vec3 normal, vec4 tangent
  const quint32 elementSize = 3 + 2 + 3 + 4;
  const quint32 stride = elementSize * sizeof( float );
  QVector<float> buffer;
  buffer.resize( stride * nVerts );

  const float du = 1.0 / ( resolution.width() - 1 );
  const float dv = 1.0 / ( resolution.height() - 1 );

  // Iterate over z
  for ( int j = 0; j < resolution.height(); ++j )
  {
    const float v = static_cast<float>( j ) * dv;

    // Iterate over x
    for ( int i = 0; i < resolution.width(); ++i )
    {
      const float u = static_cast<float>( i ) * du;

      // texture coordinates
      buffer.push_back( u );
      buffer.push_back( mirrored ? 1.0f - v : v );
    }
  }

  return buffer;
}

QVector<unsigned int> createPlaneIndexData( const QSize &resolution )
{
  // Create the index data. 2 triangles per rectangular face
  const int faces = 2 * ( resolution.width() - 1 ) * ( resolution.height() - 1 );
  const int indices = 3 * faces;
  Q_ASSERT( indices < std::numeric_limits<quint16>::max() );
  QVector<unsigned int> indexes;

  // Iterate over z
  for ( int j = 0; j < resolution.height() - 1; ++j )
  {
    const int rowStartIndex = j * resolution.width();
    const int nextRowStartIndex = ( j + 1 ) * resolution.width();

    // Iterate over x
    for ( int i = 0; i < resolution.width() - 1; ++i )
    {
      // Split quad into two triangles
      indexes.push_back( rowStartIndex + i );
      indexes.push_back( nextRowStartIndex + i );
      indexes.push_back( rowStartIndex + i + 1 );

      indexes.push_back( nextRowStartIndex + i );
      indexes.push_back( nextRowStartIndex + i + 1 );
      indexes.push_back( rowStartIndex + i + 1 );
    }
  }

  return indexes;
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

void Qgs3DSceneExporter::parseEntity( Qt3DCore::QEntity *entity )
{
  if ( entity == nullptr ) return;
  // check if the current entity is a terrain tile
  QgsTerrainTileEntity *terrainTile = qobject_cast<QgsTerrainTileEntity *>( entity );
  if ( terrainTile != nullptr )
  {
    parseEntity( terrainTile );
    return;
  }

  for ( Qt3DCore::QComponent *c : entity->components() )
  {
    Qt3DRender::QGeometryRenderer *comp = qobject_cast<Qt3DRender::QGeometryRenderer *>( c );
    if ( comp == nullptr ) continue;
    Qt3DRender::QGeometry *geom = comp->geometry();

    QgsTessellatedPolygonGeometry *tessellated = qobject_cast<QgsTessellatedPolygonGeometry *>( geom );
    if ( tessellated != nullptr )
    {
      process( tessellated );
      continue;
    }

    //    Qt3DExtras::QPlaneGeometry *plane = qobject_cast<Qt3DExtras::QPlaneGeometry *>( geom );
    //    if ( plane != nullptr )
    //    {
    //      process( plane );
    //      continue;
    //    }

  }
  for ( QObject *child : entity->children() )
  {
    Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>( child );
    if ( childEntity != nullptr ) parseEntity( childEntity );
  }
}

void Qgs3DSceneExporter::parseEntity( QgsTerrainEntity *terrain )
{
  const Qgs3DMapSettings &settings = terrain->map3D();
  QgsChunkNode *root = terrain->rootNode();

  // just use LoD0 for now
  int levelOfDetails = 0;
  QVector<QgsChunkNode *> leafs;
  leafs << root;
  for ( int i = 0; i < levelOfDetails; ++i )
  {
    QVector<QgsChunkNode *> nodes = leafs;
    leafs.clear();
    for ( QgsChunkNode *node : nodes )
    {
      node->ensureAllChildrenExist();
      QgsChunkNode *const *children = node->children();
      for ( int i = 0; i < 4; ++i )
      {
        if ( children[i] != nullptr )
        {
          leafs.push_back( children[i] );
        }
      }
    }
  }

  QgsTerrainGenerator *generator = settings.terrainGenerator();
  QgsTerrainTileEntity *terrainTile = nullptr;
  QgsTerrainTextureGenerator *textureGenerator = terrain->textureGenerator();
  QSize oldResolution = textureGenerator->textureSize();
  textureGenerator->setTextureSize( QSize( mTerrainTextureResolution, mTerrainTextureResolution ) );
  switch ( generator->type() )
  {
    case QgsTerrainGenerator::Dem:
      for ( QgsChunkNode *node : leafs )
      {
        terrainTile = getDemTerrainEntity( terrain, node );
        this->parseDemTile( terrainTile, textureGenerator );
      }
      break;
    case QgsTerrainGenerator::Flat:
      for ( QgsChunkNode *node : leafs )
      {
        terrainTile = getFlatTerrainEntity( terrain, node );
        this->parseFlatTile( terrainTile, textureGenerator );
      }
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
  QgsChunkLoader *loader = generator->createSynchronousChunkLoader( node );
  FlatTerrainChunkLoader *flatTerrainLoader = qobject_cast<FlatTerrainChunkLoader *>( loader );
  if ( flatTerrainLoader == nullptr ) return nullptr;
  // the entity we created should be deallocated when we deallocate the scene exporter
  Qt3DCore::QEntity *entity = flatTerrainLoader->createEntity( this );
  QgsTerrainTileEntity *tileEntity = qobject_cast<QgsTerrainTileEntity *>( entity );
  return tileEntity;
}

QgsTerrainTileEntity *Qgs3DSceneExporter::getDemTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node )
{
  QgsTerrainTileEntity *tileEntity = nullptr;
  // Just create a new tile (we don't need to export exact level of details as in the scene)
  // create the entity synchronously and then it will be deleted once our scene exporter instance is deallocated
  const Qgs3DMapSettings &map = terrain->map3D();
  QgsDemTerrainGenerator *generator = static_cast<QgsDemTerrainGenerator *>( map.terrainGenerator() );
  int oldResolution = generator->resolution();
  generator->setResolution( mTerrainResolution );
  QgsDemTerrainTileLoader *loader = qobject_cast<QgsDemTerrainTileLoader *>( generator->createSynchronousChunkLoader( node ) );
  tileEntity = qobject_cast<QgsTerrainTileEntity *>( loader->createEntity( this ) );
  generator->setResolution( oldResolution );
  return tileEntity;
}

void Qgs3DSceneExporter::parseFlatTile( QgsTerrainTileEntity *tileEntity, QgsTerrainTextureGenerator *textureGenerator )
{
  Qt3DRender::QGeometryRenderer *mesh = nullptr;
  Qt3DCore::QTransform *transform = nullptr;
  for ( Qt3DCore::QComponent *component : tileEntity->components() )
  {
    Qt3DRender::QGeometryRenderer *meshTyped = qobject_cast<Qt3DRender::QGeometryRenderer *>( component );
    if ( meshTyped != nullptr )
    {
      mesh = meshTyped;
      continue;
    }
    Qt3DCore::QTransform *transformTyped = qobject_cast<Qt3DCore::QTransform *>( component );
    if ( transformTyped != nullptr )
    {
      transform = transformTyped;
    }
  }

  Qt3DRender::QGeometry *geometry = mesh->geometry();
  Qt3DExtras::QPlaneGeometry *tileGeometry = qobject_cast<Qt3DExtras::QPlaneGeometry *>( geometry );
  if ( tileGeometry == nullptr )
  {
    qDebug() << "WARNING : " << "Qt3DExtras::QPlaneGeometry* is expected at " << __FILE__ << ":" << __LINE__;
    return;
  }

  float scale = transform->scale();
  QVector3D translation = transform->translation();

  QVector<float> positionBuffer = createPlaneVertexData( scale, scale, tileGeometry->resolution() );
  QVector<unsigned int> indexesBuffer = createPlaneIndexData( tileGeometry->resolution() );

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "Flat_tile" ), "", this );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );
  object->setupPositionCoordinates( positionBuffer, indexesBuffer, 1.0f, translation );

  if ( mExportNormals )
  {
    QVector<float> normalsBuffer;
    for ( int i = 0; i < positionBuffer.size(); i += 3 ) normalsBuffer << 0.0f << 1.0f << 0.0f;
    object->setupNormalCoordinates( normalsBuffer );
  }

  if ( mExportTextures )
  {
    QVector<float> texCoords = createPlaneTexCoordsData( 1.0f, 1.0f, tileGeometry->resolution(), false );
    object->setupTextureCoordinates( texCoords );

    QImage img = textureGenerator->renderSynchronously( tileEntity->textureImage()->imageExtent(),  tileEntity->textureImage()->imageDebugText() );
    object->setTextureImage( img );
  }
}

void Qgs3DSceneExporter::parseDemTile( QgsTerrainTileEntity *tileEntity, QgsTerrainTextureGenerator *textureGenerator )
{
  Qt3DRender::QGeometryRenderer *mesh = nullptr;
  Qt3DCore::QTransform *transform = nullptr;
  for ( Qt3DCore::QComponent *component : tileEntity->components() )
  {
    Qt3DRender::QGeometryRenderer *meshTyped = qobject_cast<Qt3DRender::QGeometryRenderer *>( component );
    if ( meshTyped != nullptr )
    {
      mesh = meshTyped;
      continue;
    }
    Qt3DCore::QTransform *transformTyped = qobject_cast<Qt3DCore::QTransform *>( component );
    if ( transformTyped != nullptr )
    {
      transform = transformTyped;
    }
  }

  Qt3DRender::QGeometry *geometry = mesh->geometry();
  DemTerrainTileGeometry *tileGeometry = qobject_cast<DemTerrainTileGeometry *>( geometry );
  if ( tileGeometry == nullptr )
  {
    qDebug() << "WARNING : " << "DemTerrainTileGeometry* is expected at " << __FILE__ << ":" << __LINE__;
    return;
  }

  float scale = transform->scale();
  QVector3D translation = transform->translation();

  QVector<float> positionBuffer = getAttributeData<float>( tileGeometry->positionAttribute() );
  QVector<unsigned int> indexBuffer = getAttributeData<unsigned int>( tileGeometry->indexAttribute() );

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "DEM_tile" ), "", this );
  mObjects.push_back( object );

  object->setSmoothEdges( mSmoothEdges );
  object->setupPositionCoordinates( positionBuffer, indexBuffer, scale, translation );

  if ( mExportNormals )
  {
    QVector<float> normalsBuffer = getAttributeData<float>( tileGeometry->normalAttribute() );
    object->setupNormalCoordinates( normalsBuffer );
  }

  if ( mExportTextures )
  {
    QVector<float> texCoordsBuffer = getAttributeData<float>( tileGeometry->texCoordsAttribute() );
    object->setupTextureCoordinates( texCoordsBuffer );

    QImage img = textureGenerator->renderSynchronously( tileEntity->textureImage()->imageExtent(),  tileEntity->textureImage()->imageDebugText() );
    object->setTextureImage( img );
  }
}

void Qgs3DSceneExporter::processAttribute( Qt3DRender::QAttribute *attribute )
{
  QVector<float> floatData = getAttributeData<float>( attribute );

  Qgs3DExportObject *object = new Qgs3DExportObject( getObjectName( "geometry" ), "", this );
  mObjects.push_back( object );

  object->setupPositionCoordinates( floatData );
}

void Qgs3DSceneExporter::process( QgsTessellatedPolygonGeometry *geom )
{
  // Just use position attributes for now
  processAttribute( geom->mPositionAttribute );
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
    ret = usedObjectNamesCounter[name];
    usedObjectNamesCounter[name]++;
  }
  else
    usedObjectNamesCounter[name] = 2;
  return ret;
}
