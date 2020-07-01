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
    for ( int j = 0; j < vertexSize * sizeof( T ); j += sizeof( T ) )
    {
      // maybe a problem with indienness can happen?
      T v;
      char *vArr = ( char * )&v;
      for ( int k = 0; k < sizeof( T ); ++k )
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
    for ( int k = 0; k < sizeof( unsigned int ); ++k )
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

  const int nVerts = resolution.width() * resolution.height();

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos, vec2 texCoord, vec3 normal, vec4 tangent
  const quint32 elementSize = 3 + 2 + 3 + 4;
  const quint32 stride = elementSize * sizeof( float );
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

QVector<int> createPlaneIndexData( const QSize &resolution )
{
  // Create the index data. 2 triangles per rectangular face
  const int faces = 2 * ( resolution.width() - 1 ) * ( resolution.height() - 1 );
  const int indices = 3 * faces;
  Q_ASSERT( indices < std::numeric_limits<quint16>::max() );
  QVector<int> indexes;

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

Qgs3DSceneExporter::Qgs3DSceneExporter( )
  : mVertxPosition()
  , mIndexes()
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
  QgsChunkNode *root = terrain->rootNode();
  QgsTerrainGenerator *generator = terrain->map3D().terrainGenerator();
  switch ( generator->type() )
  {
    case QgsTerrainGenerator::Dem:
      generateDemTerrain( terrain, root );
      break;
    case QgsTerrainGenerator::Flat:
      generateFlatTerrain( terrain, root );
      break;
  }
}

void Qgs3DSceneExporter::generateFlatTerrain( QgsTerrainEntity *terrain, QgsChunkNode *node )
{
  QgsFlatTerrainGenerator *generator = dynamic_cast<QgsFlatTerrainGenerator *>( terrain->map3D().terrainGenerator() );
  QgsChunkLoader *loader = generator->createChunkLoader( node );
  FlatTerrainChunkLoader *floatTerrainLoader = qobject_cast<FlatTerrainChunkLoader *>( loader );
  if ( floatTerrainLoader == nullptr ) return;
  Qt3DCore::QEntity *entity = floatTerrainLoader->createEntity( nullptr );
  QgsTerrainTileEntity *tileEntity = qobject_cast<QgsTerrainTileEntity *>( entity );
  if ( tileEntity != nullptr ) parseFlatTile( tileEntity );
  if ( entity != nullptr ) delete entity;
}

static void _heightMapMinMax( const QByteArray &heightMap, float &zMin, float &zMax )
{
  const float *zBits = ( const float * ) heightMap.constData();
  int zCount = heightMap.count() / sizeof( float );
  bool first = true;

  zMin = zMax = std::numeric_limits<float>::quiet_NaN();
  for ( int i = 0; i < zCount; ++i )
  {
    float z = zBits[i];
    if ( std::isnan( z ) )
      continue;
    if ( first )
    {
      zMin = zMax = z;
      first = false;
    }
    zMin = std::min( zMin, z );
    zMax = std::max( zMax, z );
  }
}

QgsTerrainTileEntity *Qgs3DSceneExporter::createDEMTileEntity( QgsTerrainEntity *terrain, QgsChunkNode *node )
{
  const Qgs3DMapSettings &map = terrain->map3D();
  QgsDemTerrainGenerator *generator = static_cast<QgsDemTerrainGenerator *>( map.terrainGenerator() );
  QgsDemHeightMapGenerator *heightMapGenerator = generator->heightMapGenerator();
  float resolution = generator->resolution();
//  heightMapGenerator->set
  QByteArray heightMap = heightMapGenerator->renderSynchronously( node->tileX(), node->tileY(), node->tileZ() );
  float skirtHeight = generator->skirtHeight();

  float zMin, zMax;
  _heightMapMinMax( heightMap, zMin, zMax );

  if ( std::isnan( zMin ) || std::isnan( zMax ) )
  {
    // no data available for this tile
    return nullptr;
  }

  QgsRectangle extent = map.terrainGenerator()->tilingScheme().tileToExtent( node->tileX(), node->tileY(), node->tileZ() ); //node->extent;
  double x0 = extent.xMinimum() - map.origin().x();
  double y0 = extent.yMinimum() - map.origin().y();
  double side = extent.width();
  double half = side / 2;

  QgsTerrainTileEntity *entity = new QgsTerrainTileEntity( node->tileId() );

  // create geometry renderer

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new DemTerrainTileGeometry( resolution, side, map.terrainVerticalScale(), skirtHeight, heightMap, mesh ) );
  entity->addComponent( mesh ); // takes ownership if the component has no parent

  // create material
  // TODO: create texture component
//      createTextureComponent( entity, map.isTerrainShadingEnabled(), map.terrainShadingMaterial() );

  // create transform

  Qt3DCore::QTransform *transform = nullptr;
  transform = new Qt3DCore::QTransform();
  entity->addComponent( transform );

  transform->setScale( side );
  transform->setTranslation( QVector3D( x0 + half, 0, - ( y0 + half ) ) );

  node->setExactBbox( QgsAABB( x0, zMin * map.terrainVerticalScale(), -y0, x0 + side, zMax * map.terrainVerticalScale(), -( y0 + side ) ) );

  entity->setEnabled( false );
//    entity->setParent( nullptr );

  return entity;
}

void Qgs3DSceneExporter::generateDemTerrain( QgsTerrainEntity *terrain, QgsChunkNode *node )
{
  QgsChunkNode *const *children = node->children();
  bool isLeaf = true;
  for ( int i = 0; i < 4; ++i )
  {
    if ( children[i] == nullptr ) continue;
    generateDemTerrain( terrain, children[i] );
    isLeaf = false;
  }
  if ( !isLeaf ) return;

  if ( node->entity() != nullptr )
  {
    // read tile data as displayed in the scene
    Qt3DCore::QEntity *entity = node->entity();
    qDebug() << "Loading existing entity";
    QgsTerrainTileEntity *tileEntity = qobject_cast<QgsTerrainTileEntity *>( entity );
    if ( tileEntity != nullptr ) parseDemTile( tileEntity );
  }
  else
  {
    // create the entity synchronously and then delete it
    QgsTerrainTileEntity *entity = createDEMTileEntity( terrain, node );
    if ( entity != nullptr ) parseDemTile( entity );
    if ( entity != nullptr ) delete entity;
  }
}

void Qgs3DSceneExporter::parseFlatTile( QgsTerrainTileEntity *tileEntity )
{
  qDebug() << "Parsing Terrain tile entity";
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
  QVector<int> indexesBuffer = createPlaneIndexData( tileGeometry->resolution() );

  int startIndex = mVertxPosition.size() / 3 + 1;
  for ( int i : indexesBuffer ) mIndexes << startIndex + i;
  for ( int i = 0; i < positionBuffer.size(); i += 3 )
  {
    for ( int j = 0; j < 3; ++j )
    {
      mVertxPosition << positionBuffer[i + j] + translation[j];
    }
  }
}

void Qgs3DSceneExporter::parseDemTile( QgsTerrainTileEntity *tileEntity )
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
    qDebug() << "WARNING : " << "Qt3DExtras::QPlaneGeometry* is expected at " << __FILE__ << ":" << __LINE__;
    return;
  }

  float scale = transform->scale();
  QVector3D translation = transform->translation();

  QVector<float> positionBuffer = getAttributeData<float>( tileGeometry->positionAttribute() );
  QVector<unsigned int> indexBuffer = getAttributeData<unsigned int>( tileGeometry->indexAttribute() );

  int startIndex = mVertxPosition.size() / 3 + 1;
  for ( int i : indexBuffer ) mIndexes << startIndex + i;

  for ( int i = 0; i < positionBuffer.size(); i += 3 )
  {
    for ( int j = 0; j < 3; ++j )
    {
      mVertxPosition << positionBuffer[i + j] * scale + translation[j];
    }
  }
}

void Qgs3DSceneExporter::processAttribute( Qt3DRender::QAttribute *attribute )
{
  QVector<float> floatData = getAttributeData<float>( attribute );

  int currentIndex = mIndexes.size() + 1;

  for ( int i = 0; i < floatData.size(); i += 3 )
  {
    mVertxPosition << floatData[i] << floatData[i + 1] << floatData[i + 2];
    mIndexes << currentIndex;
    ++currentIndex;
  }
}

void Qgs3DSceneExporter::process( QgsTessellatedPolygonGeometry *geom )
{
  // Just use position attributes for now
  processAttribute( geom->mPositionAttribute );
}

void Qgs3DSceneExporter::process( Qt3DExtras::QPlaneGeometry *geom )
{
  for ( Qt3DRender::QAttribute *attribute : geom->attributes() )
  {
    if ( attribute->name() == Qt3DRender::QAttribute::defaultPositionAttributeName() )
    {
      qDebug() << "Processing plane gemetry attribute";
      processAttribute( attribute );
    }
  }
}

void Qgs3DSceneExporter::saveToFile( const QString &filePath )
{
  QFile file( filePath );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    return;

  float maxX = std::numeric_limits<float>::min(), maxY = std::numeric_limits<float>::min(), maxZ = std::numeric_limits<float>::min();
  float minX = std::numeric_limits<float>::max(), minY = std::numeric_limits<float>::max(), minZ = std::numeric_limits<float>::max();
  for ( int i = 0; i < mVertxPosition.size(); i += 3 )
  {
    if ( mVertxPosition[i + 1] < 0 ) continue;
    if ( mVertxPosition[i] > maxX ) maxX = mVertxPosition[i];
    if ( mVertxPosition[i + 1] > maxY ) maxY = mVertxPosition[i + 1];
    if ( mVertxPosition[i + 2] > maxZ ) maxZ = mVertxPosition[i + 2];
    if ( mVertxPosition[i] < minX ) minX = mVertxPosition[i];
    if ( mVertxPosition[i + 1] < minY ) minY = mVertxPosition[i + 1];
    if ( mVertxPosition[i + 2] < minZ ) minZ = mVertxPosition[i + 2];
  }

  float diffX = 1.0f, diffY = 1.0f, diffZ = 1.0f;
  if ( mVertxPosition.size() >= 3 )
  {
    diffX = maxX - minX;
    diffY = maxY - minY;
    diffZ = maxZ - minZ;
  }

  float centerX = ( minX + maxX ) / 2.0f;
  float centerY = ( minY + maxY ) / 2.0f;
  float centerZ = ( minZ + maxZ ) / 2.0f;

  qDebug() << "Vertical span : " << diffY;

  float scale = diffY;//qMax( diffX, qMax( diffY, diffZ ) );

  QTextStream out( &file );

  QSet<int> ignored;

  // Construct vertices
  for ( int i = 0; i < mVertxPosition.size(); i += 3 )
  {
    // for now just ignore some vertex positions
    if ( mVertxPosition[i + 1] < 0 ) ignored.insert( i / 3 );
    out << "v ";
    out << ( mVertxPosition[i] - centerX ) / scale << " ";
    out << ( mVertxPosition[i + 1] - centerY ) / scale << " ";
    out << ( mVertxPosition[i + 2] - centerZ ) / scale << "\n";
  }

  // Construct faces
  for ( int i = 0; i < mIndexes.size(); i += 3 )
  {
    if ( ignored.contains( mIndexes[i] ) || ignored.contains( mIndexes[i + 1] ) || ignored.contains( mIndexes[i + 2] ) ) continue;
    out << "f " << mIndexes[i] << " " << mIndexes[i + 1] << " " << mIndexes[i + 2] << "\n";
  }
  qDebug() << "Ignored count : " << ignored.size();
}
