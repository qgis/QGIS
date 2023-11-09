/***************************************************************************
  qgsbillboardgeometry.cpp
  --------------------------------------
  Date                 : Jul 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QVector3D>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
#endif

#include "qgsbillboardgeometry.h"

QgsBillboardGeometry::QgsBillboardGeometry( Qt3DCore::QNode *parent )
  : QGeometry( parent )
  , mPositionAttribute( new Qt3DQAttribute( this ) )
  , mVertexBuffer( new Qt3DQBuffer( this ) )
{

  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( 3 * sizeof( float ) );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );

  addAttribute( mPositionAttribute );

}

void QgsBillboardGeometry::setPoints( const QVector<QVector3D> &vertices )
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
  }

  mVertexCount = vertices.count();
  mVertexBuffer->setData( vertexBufferData );

  emit countChanged( mVertexCount );

}

int QgsBillboardGeometry::count() const
{
  return mVertexCount;
}


