/***************************************************************************
  qgs3dwiredmesh_p.cpp
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Jean Felder
  Email                : jean dot felder at oslandia dot com
  ***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
 ***************************************************************************/

#include "qgs3dwiredmesh_p.h"
#include "moc_qgs3dwiredmesh_p.cpp"

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QGeometry>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QGeometry>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
#endif

#include "qgsaabb.h"


///@cond PRIVATE

Qgs3DWiredMesh::Qgs3DWiredMesh( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometryRenderer( parent )
  , mPositionAttribute( new Qt3DQAttribute( this ) )
  , mVertexBuffer( new Qt3DQBuffer( this ) )
{
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );

  mGeom = new Qt3DQGeometry( this );
  mGeom->addAttribute( mPositionAttribute );

  setInstanceCount( 1 );
  setIndexOffset( 0 );
  setFirstInstance( 0 );
  setPrimitiveType( Qt3DRender::QGeometryRenderer::Lines );
  setGeometry( mGeom );
}

Qgs3DWiredMesh::~Qgs3DWiredMesh() = default;

void Qgs3DWiredMesh::setVertices( const QList<QVector3D> &vertices )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( static_cast<int>( static_cast<long>( vertices.size() ) * 3 * sizeof( float ) ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( const QVector3D &v : std::as_const( vertices ) )
  {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
  }

  mVertexBuffer->setData( vertexBufferData );
  setVertexCount( vertices.count() );
}

void Qgs3DWiredMesh::setVertices( const QList<QgsAABB> &bboxes )
{
  QList<QVector3D> vertices;
  for ( const QgsAABB &bbox : bboxes )
    vertices << bbox.verticesForLines();

  setVertices( vertices );
}

/// @endcond
