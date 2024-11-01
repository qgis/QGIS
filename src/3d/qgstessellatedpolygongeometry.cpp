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
#include "moc_qgstessellatedpolygongeometry.cpp"
#include "qgsraycastingutils_p.h"
#include "qgsmessagelog.h"

#include <QMatrix4x4>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
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

#include "qgstessellator.h"
#include "qgspolygon.h"

QgsTessellatedPolygonGeometry::QgsTessellatedPolygonGeometry( bool _withNormals, bool _invertNormals, bool _addBackFaces, bool _addTextureCoords, QNode *parent )
  : QGeometry( parent )
  , mWithNormals( _withNormals )
  , mInvertNormals( _invertNormals )
  , mAddBackFaces( _addBackFaces )
  , mAddTextureCoords( _addTextureCoords )
{
  mVertexBuffer = new Qt3DQBuffer( this );

  const QgsTessellator tmpTess( 0, 0, mWithNormals, false, false, false, mAddTextureCoords );
  const int stride = tmpTess.stride();

  mPositionAttribute = new Qt3DQAttribute( this );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setByteStride( stride );
  mPositionAttribute->setByteOffset( 0 );
  addAttribute( mPositionAttribute );

  if ( mWithNormals )
  {
    mNormalAttribute = new Qt3DQAttribute( this );
    mNormalAttribute->setName( Qt3DQAttribute::defaultNormalAttributeName() );
    mNormalAttribute->setVertexBaseType( Qt3DQAttribute::Float );
    mNormalAttribute->setVertexSize( 3 );
    mNormalAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
    mNormalAttribute->setBuffer( mVertexBuffer );
    mNormalAttribute->setByteStride( stride );
    mNormalAttribute->setByteOffset( 3 * sizeof( float ) );
    addAttribute( mNormalAttribute );
  }
  if ( mAddTextureCoords )
  {
    mTextureCoordsAttribute = new Qt3DQAttribute( this );
    mTextureCoordsAttribute->setName( Qt3DQAttribute::defaultTextureCoordinateAttributeName() );
    mTextureCoordsAttribute->setVertexBaseType( Qt3DQAttribute::Float );
    mTextureCoordsAttribute->setVertexSize( 2 );
    mTextureCoordsAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
    mTextureCoordsAttribute->setBuffer( mVertexBuffer );
    mTextureCoordsAttribute->setByteStride( stride );
    mTextureCoordsAttribute->setByteOffset( mWithNormals ? 6 * sizeof( float ) : 3 * sizeof( float ) );
    addAttribute( mTextureCoordsAttribute );
  }
}

void QgsTessellatedPolygonGeometry::setData( const QByteArray &vertexBufferData, int vertexCount, const QVector<QgsFeatureId> &triangleIndexFids, const QVector<uint> &triangleIndexStartingIndices )
{
  mTriangleIndexStartingIndices = triangleIndexStartingIndices;
  mTriangleIndexFids = triangleIndexFids;

  mVertexBuffer->setData( vertexBufferData );
  mPositionAttribute->setCount( vertexCount );
  if ( mNormalAttribute )
    mNormalAttribute->setCount( vertexCount );
  if ( mTextureCoordsAttribute )
    mTextureCoordsAttribute->setCount( vertexCount );
}

// run binary search on a sorted array, return index i where data[i] <= v < data[i+1]
static int binary_search( uint v, const uint *data, int count )
{
  int idx0 = 0;
  int idx1 = count - 1;

  if ( v < data[0] )
    return -1; // not in the array

  if ( v >= data[count - 1] )
    return count - 1; // for larger values the last bin is returned

  while ( idx0 != idx1 )
  {
    const int idxPivot = ( idx0 + idx1 ) / 2;
    const uint pivot = data[idxPivot];
    if ( pivot <= v )
    {
      if ( data[idxPivot + 1] > v )
        return idxPivot; // we're done!
      else               // continue searching values greater than the pivot
        idx0 = idxPivot;
    }
    else // continue searching values lower than the pivot
      idx1 = idxPivot;
  }
  return idx0;
}


QgsFeatureId QgsTessellatedPolygonGeometry::triangleIndexToFeatureId( uint triangleIndex ) const
{
  const int i = binary_search( triangleIndex, mTriangleIndexStartingIndices.constData(), mTriangleIndexStartingIndices.count() );
  return i != -1 ? mTriangleIndexFids[i] : FID_NULL;
}
