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

    Qt3DExtras::QPlaneGeometry *plane = qobject_cast<Qt3DExtras::QPlaneGeometry *>( geom );
    if ( plane != nullptr )
    {
      process( plane );
      continue;
    }

  }
  for ( QObject *child : entity->children() )
  {
    Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>( child );
    if ( childEntity != nullptr ) parseEntity( childEntity );
  }
}

void Qgs3DSceneExporter::parseEntity( QgsTerrainEntity *terrain )
{
  if ( terrain->map3D().terrainGenerator()->type() != QgsTerrainGenerator::Flat )
    return;

  qDebug() << "Parsing flat terrain";

  QList<QgsChunkNode *> lst = terrain->activeNodes();
  for ( QgsChunkNode *n : lst )
  {
    Qt3DCore::QEntity *entity = n->entity();
    QgsTerrainTileEntity *terrainTile = qobject_cast<QgsTerrainTileEntity *>( entity );
    if ( terrainTile != nullptr ) parseEntity( terrainTile );
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

void Qgs3DSceneExporter::parseEntity( QgsTerrainTileEntity *tileEntity )
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

void Qgs3DSceneExporter::saveToFile( const QString &filePath )
{
  QFile file( filePath );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    return;

  QTextStream out( &file );

  // Construct vertices
  for ( int i = 0; i < mVertxPosition.size(); i += 3 )
  {
    out << "v " << mVertxPosition[i] << " " << mVertxPosition[i + 1] << " " << mVertxPosition[i + 2] << "\n";
  }

  // Construct faces
  for ( int i = 0; i < mIndexes.size(); i += 3 )
  {
    out << "f " << mIndexes[i] << " " << mIndexes[i + 1] << " " << mIndexes[i + 2] << "\n";
  }
}
