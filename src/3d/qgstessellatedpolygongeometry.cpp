/***************************************************************************
  qgstessellatedpolygongeometry.cpp
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

#include "qgstessellatedpolygongeometry.h"

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QBufferDataGenerator>

#include "qgstessellator.h"

#include "qgspoint.h"
#include "qgspolygon.h"


QgsTessellatedPolygonGeometry::QgsTessellatedPolygonGeometry( QNode *parent )
  : Qt3DRender::QGeometry( parent )
{
  mVertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );

  QgsTessellator tmpTess( 0, 0, mWithNormals );
  const int stride = tmpTess.stride();

  mPositionAttribute = new Qt3DRender::QAttribute( this );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setByteStride( stride );
  addAttribute( mPositionAttribute );

  if ( mWithNormals )
  {
    mNormalAttribute = new Qt3DRender::QAttribute( this );
    mNormalAttribute->setName( Qt3DRender::QAttribute::defaultNormalAttributeName() );
    mNormalAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
    mNormalAttribute->setVertexSize( 3 );
    mNormalAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
    mNormalAttribute->setBuffer( mVertexBuffer );
    mNormalAttribute->setByteStride( stride );
    mNormalAttribute->setByteOffset( 3 * sizeof( float ) );
    addAttribute( mNormalAttribute );
  }
}

QgsTessellatedPolygonGeometry::~QgsTessellatedPolygonGeometry()
{
  qDeleteAll( mPolygons );
}

void QgsTessellatedPolygonGeometry::setPolygons( const QList<QgsPolygon *> &polygons, const QgsPointXY &origin, float extrusionHeight, const QList<float> &extrusionHeightPerPolygon )
{
  qDeleteAll( mPolygons );
  mPolygons = polygons;

  QgsTessellator tesselator( origin.x(), origin.y(), mWithNormals );
  for ( int i = 0; i < polygons.count(); ++i )
  {
    QgsPolygon *polygon = polygons.at( i );
    float extr = extrusionHeightPerPolygon.isEmpty() ? extrusionHeight : extrusionHeightPerPolygon.at( i );
    tesselator.addPolygon( *polygon, extr );
  }

  QByteArray data( ( const char * )tesselator.data().constData(), tesselator.data().count() * sizeof( float ) );
  int nVerts = data.count() / tesselator.stride();

  mVertexBuffer->setData( data );
  mPositionAttribute->setCount( nVerts );
  if ( mNormalAttribute )
    mNormalAttribute->setCount( nVerts );
}
