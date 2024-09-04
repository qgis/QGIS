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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
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

#include "qgslogger.h"
#include "qgs3dutils.h"
#include "qgslinestring.h"

/// @cond PRIVATE


QgsLineVertexData::QgsLineVertexData()
{
  // the first index is invalid, we use it for primitive restart
  vertices << QVector3D();
}

void QgsLineVertexData::init( Qgis::AltitudeClamping clamping, Qgis::AltitudeBinding binding, float height, const Qgs3DRenderContext &context )
{
  altClamping = clamping;
  altBinding = binding;
  baseHeight = height;
  renderContext = context;
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

Qt3DQGeometry *QgsLineVertexData::createGeometry( Qt3DCore::QNode *parent )
{
  Qt3DQBuffer *vertexBuffer = new Qt3DQBuffer( parent );
  vertexBuffer->setData( createVertexBuffer() );

  Qt3DQBuffer *indexBuffer = new Qt3DQBuffer( parent );
  indexBuffer->setData( createIndexBuffer() );

  QgsDebugMsgLevel( QString( "vertex buffer %1 MB  index buffer %2 MB " ).arg( vertexBuffer->data().count() / 1024. / 1024. ).arg( indexBuffer->data().count() / 1024. / 1024. ), 2 );

  Qt3DQAttribute *positionAttribute = new Qt3DQAttribute( parent );
  positionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setByteStride( 3 * sizeof( float ) );
  positionAttribute->setByteOffset( 0 );
  positionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );

  Qt3DQAttribute *indexAttribute = new Qt3DQAttribute( parent );
  indexAttribute->setAttributeType( Qt3DQAttribute::IndexAttribute );
  indexAttribute->setBuffer( indexBuffer );
  indexAttribute->setByteOffset( 0 );
  indexAttribute->setByteStride( sizeof( uint ) );
  indexAttribute->setVertexBaseType( Qt3DQAttribute::UnsignedInt );

  Qt3DQGeometry *geom = new Qt3DQGeometry;
  geom->addAttribute( positionAttribute );
  geom->addAttribute( indexAttribute );

  return geom;
}

void QgsLineVertexData::addLineString( const QgsLineString &lineString, float extraHeightOffset )
{
  if ( withAdjacency )
    indexes << vertices.count();  // add the following vertex (for adjacency)

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

    vertices << QVector3D( static_cast< float >( p.x() - renderContext.origin().x() ),
                           z,
                           static_cast< float >( -( p.y() - renderContext.origin().y() ) ) );
    indexes << vertices.count() - 1;
  }

  if ( withAdjacency )
    indexes << vertices.count() - 1;  // add the last vertex (for adjacency)

  indexes << 0;  // add primitive restart
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
      indexes << vertices.count();  // add the following vertex (for adjacency)

    vertices << QVector3D( static_cast< float >( p.x() - renderContext.origin().x() ),
                           z,
                           static_cast< float >( -( p.y() - renderContext.origin().y() ) ) );
    indexes << vertices.count() - 1;
    vertices << QVector3D( static_cast< float >( p.x() - renderContext.origin().x() ),
                           z2,
                           static_cast< float >( -( p.y() - renderContext.origin().y() ) ) );
    indexes << vertices.count() - 1;

    if ( withAdjacency )
      indexes << vertices.count() - 1;  // add the last vertex (for adjacency)

    indexes << 0;  // add primitive restart
  }
}


/// @endcond
