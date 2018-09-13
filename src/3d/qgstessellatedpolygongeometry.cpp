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

void QgsTessellatedPolygonGeometry::setPolygons( const QList<QgsPolygon *> &polygons, const QList<QgsFeatureId> &featureIds, const QgsPointXY &origin, float extrusionHeight, const QList<float> &extrusionHeightPerPolygon )
{
  Q_ASSERT( polygons.count() == featureIds.count() );
  mTriangleIndexStartingIndices.reserve( polygons.count() );
  mTriangleIndexFids.reserve( polygons.count() );

  QgsTessellator tessellator( origin.x(), origin.y(), mWithNormals, mInvertNormals, mAddBackFaces );
  for ( int i = 0; i < polygons.count(); ++i )
  {
    Q_ASSERT( tessellator.dataVerticesCount() % 3 == 0 );
    uint startingTriangleIndex = static_cast<uint>( tessellator.dataVerticesCount() / 3 );
    mTriangleIndexStartingIndices.append( startingTriangleIndex );
    mTriangleIndexFids.append( featureIds[i] );

    QgsPolygon *polygon = polygons.at( i );
    float extr = extrusionHeightPerPolygon.isEmpty() ? extrusionHeight : extrusionHeightPerPolygon.at( i );
    tessellator.addPolygon( *polygon, extr );
  }

  qDeleteAll( polygons );

  QByteArray data( ( const char * )tessellator.data().constData(), tessellator.data().count() * sizeof( float ) );
  int nVerts = data.count() / tessellator.stride();

  mVertexBuffer->setData( data );
  mPositionAttribute->setCount( nVerts );
  if ( mNormalAttribute )
    mNormalAttribute->setCount( nVerts );
}


// run binary search on a sorted array, return index i where data[i] <= x < data[i+1]
static int binary_search( uint v, const uint *data, int count )
{
  int idx0 = 0;
  int idx1 = count - 1;

  if ( v < data[0] )
    return -1;  // not in the array

  while ( idx0 != idx1 )
  {
    int idxPivot = ( idx0 + idx1 ) / 2;
    uint pivot = data[idxPivot];
    if ( pivot <= v )
    {
      if ( data[idxPivot + 1] > v )
        return idxPivot;   // we're done!
      else  // continue searching values greater than the pivot
        idx0 = idxPivot;
    }
    else   // continue searching values lower than the pivot
      idx1 = idxPivot;
  }
  return idx0;
}


QgsFeatureId QgsTessellatedPolygonGeometry::triangleIndexToFeatureId( uint triangleIndex ) const
{
  int i = binary_search( triangleIndex, mTriangleIndexStartingIndices.constData(), mTriangleIndexStartingIndices.count() );
  Q_ASSERT( i != -1 );
  return mTriangleIndexFids[i];
}
