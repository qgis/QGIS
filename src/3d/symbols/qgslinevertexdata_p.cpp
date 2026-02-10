/***************************************************************************
  qgslinevertexdata_p.cpp
  --------------------------------------
  Date                 : Apr 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslinevertexdata_p.h"

#include "qgs3dutils.h"
#include "qgsabstractterrainsettings.h"
#include "qgslinestring.h"
#include "qgslogger.h"

#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

/// @cond PRIVATE


QgsLineVertexData::QgsLineVertexData()
{
  // the first index is invalid, we use it for primitive restart
  vertices << QVector3D();
}

void QgsLineVertexData::init( Qgis::AltitudeClamping clamping, Qgis::AltitudeBinding binding, float height, const Qgs3DRenderContext &context, const QgsVector3D &chunkOrigin )
{
  altClamping = clamping;
  altBinding = binding;
  baseHeight = height;
  renderContext = context;
  origin = chunkOrigin;
}

QByteArray QgsLineVertexData::createVertexBuffer()
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( vertices.size() * 3 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( const auto &v : std::as_const( vertices ) )
  {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
  }
  return vertexBufferData;
}

QByteArray QgsLineVertexData::createIndexBuffer()
{
  QByteArray indexBufferData;
  indexBufferData.resize( indexes.size() * sizeof( int ) );
  unsigned int *rawIndexArray = reinterpret_cast<unsigned int *>( indexBufferData.data() );
  int idx = 0;
  for ( unsigned int indexVal : std::as_const( indexes ) )
  {
    rawIndexArray[idx++] = indexVal;
  }
  return indexBufferData;
}

Qt3DCore::QGeometry *QgsLineVertexData::createGeometry( Qt3DCore::QNode *parent )
{
  Qt3DCore::QBuffer *vertexBuffer = new Qt3DCore::QBuffer( parent );
  vertexBuffer->setData( createVertexBuffer() );

  Qt3DCore::QBuffer *indexBuffer = new Qt3DCore::QBuffer( parent );
  indexBuffer->setData( createIndexBuffer() );

  QgsDebugMsgLevel( QString( "vertex buffer %1 MB  index buffer %2 MB " ).arg( vertexBuffer->data().count() / 1024. / 1024. ).arg( indexBuffer->data().count() / 1024. / 1024. ), 2 );

  Qt3DCore::QAttribute *positionAttribute = new Qt3DCore::QAttribute( parent );
  positionAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setByteStride( 3 * sizeof( float ) );
  positionAttribute->setByteOffset( 0 );
  positionAttribute->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );

  Qt3DCore::QAttribute *indexAttribute = new Qt3DCore::QAttribute( parent );
  indexAttribute->setAttributeType( Qt3DCore::QAttribute::IndexAttribute );
  indexAttribute->setBuffer( indexBuffer );
  indexAttribute->setByteOffset( 0 );
  indexAttribute->setByteStride( sizeof( uint ) );
  indexAttribute->setVertexBaseType( Qt3DCore::QAttribute::UnsignedInt );

  Qt3DCore::QGeometry *geom = new Qt3DCore::QGeometry;
  geom->addAttribute( positionAttribute );
  geom->addAttribute( indexAttribute );

  return geom;
}

void QgsLineVertexData::addLineString( const QgsLineString &lineString, float extraHeightOffset, bool closePolygon )
{
  if ( withAdjacency )
    indexes << vertices.count(); // add the following vertex (for adjacency)

  QgsPoint centroid;
  switch ( altBinding )
  {
    case Qgis::AltitudeBinding::Vertex:
      break;
    case Qgis::AltitudeBinding::Centroid:
      centroid = lineString.centroid();
      break;
  }

  const int firstIndex = vertices.count();

  for ( int i = 0; i < lineString.vertexCount(); ++i )
  {
    QgsPoint p = lineString.pointN( i );
    if ( geocentricCoordinates )
    {
      // TODO: implement altitude clamping when dealing with geocentric coordinates
      // where Z coordinate is not altitude and can't be used directly...
      vertices << QVector3D( static_cast<float>( p.x() - origin.x() ), static_cast<float>( p.y() - origin.y() ), static_cast<float>( p.z() - origin.z() ) );
    }
    else
    {
      const float z = Qgs3DUtils::clampAltitude( p, altClamping, altBinding, baseHeight + extraHeightOffset, centroid, renderContext );
      vertices << QVector3D( static_cast<float>( p.x() - origin.x() ), static_cast<float>( p.y() - origin.y() ), static_cast<float>( z - origin.z() ) );
    }
    indexes << vertices.count() - 1;
  }

  if ( closePolygon )
    indexes << firstIndex; // repeat the first vertex

  if ( withAdjacency )
    indexes << vertices.count() - 1; // add the last vertex (for adjacency)

  indexes << 0; // add primitive restart
}

void QgsLineVertexData::addVerticalLines( const QgsLineString &lineString, float verticalLength, float extraHeightOffset )
{
  QgsPoint centroid;
  switch ( altBinding )
  {
    case Qgis::AltitudeBinding::Vertex:
      break;
    case Qgis::AltitudeBinding::Centroid:
      centroid = lineString.centroid();
      break;
  }

  for ( int i = 0; i < lineString.vertexCount(); ++i )
  {
    QgsPoint p = lineString.pointN( i );
    float z = Qgs3DUtils::clampAltitude( p, altClamping, altBinding, baseHeight + extraHeightOffset, centroid, renderContext );
    float z2 = z + verticalLength;

    if ( withAdjacency )
      indexes << vertices.count(); // add the following vertex (for adjacency)

    vertices << QVector3D( static_cast<float>( p.x() - origin.x() ), static_cast<float>( p.y() - origin.y() ), z );
    indexes << vertices.count() - 1;
    vertices << QVector3D( static_cast<float>( p.x() - origin.x() ), static_cast<float>( p.y() - origin.y() ), z2 );
    indexes << vertices.count() - 1;

    if ( withAdjacency )
      indexes << vertices.count() - 1; // add the last vertex (for adjacency)

    indexes << 0; // add primitive restart
  }
}


/// @endcond
