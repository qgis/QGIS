/***************************************************************************
  qgschunkboundsentity_p.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgschunkboundsentity_p.h"

#include <Qt3DRender/QBuffer>
#include <Qt3DExtras/QPhongMaterial>

#include "qgsaabb.h"


///@cond PRIVATE

LineMeshGeometry::LineMeshGeometry( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mVertexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
{
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );

  addAttribute( mPositionAttribute );
}

void LineMeshGeometry::setVertices( const QList<QVector3D> &vertices )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( vertices.size() * 3 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( const auto &v : vertices )
  {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
    mVertices.append( v );
  }

  mVertexBuffer->setData( vertexBufferData );
}


// ----------------


AABBMesh::AABBMesh( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometryRenderer( parent )
{
  setInstanceCount( 1 );
  setIndexOffset( 0 );
  setFirstInstance( 0 );
  setPrimitiveType( Qt3DRender::QGeometryRenderer::Lines );

  mLineMeshGeo = new LineMeshGeometry( this );
  setGeometry( mLineMeshGeo );
}

void AABBMesh::setBoxes( const QList<QgsAABB> &bboxes )
{
  QList<QVector3D> vertices;
  Q_FOREACH ( const QgsAABB &bbox, bboxes )
    vertices << bbox.verticesForLines();
  mLineMeshGeo->setVertices( vertices );
  setVertexCount( mLineMeshGeo->vertexCount() );
}


// ----------------


QgsChunkBoundsEntity::QgsChunkBoundsEntity( Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  mAabbMesh = new AABBMesh;
  addComponent( mAabbMesh );

  Qt3DExtras::QPhongMaterial *bboxesMaterial = new Qt3DExtras::QPhongMaterial;
  bboxesMaterial->setAmbient( Qt::red );
  addComponent( bboxesMaterial );
}

void QgsChunkBoundsEntity::setBoxes( const QList<QgsAABB> &bboxes )
{
  mAabbMesh->setBoxes( bboxes );
}

/// @endcond
