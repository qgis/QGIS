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
#include "qgsraycastingutils_p.h"
#include <QMatrix4x4>

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

static bool intersectionTriangles( const QByteArray &vertexBuf, const int &stride, const QgsRayCastingUtils::Ray3D &r, const QMatrix4x4 &worldTransform, QVector3D &intPt, int &triangleIndex )
{
  const float *vertices = reinterpret_cast<const float *>( vertexBuf.constData() );

  const int vertexCount = vertexBuf.size() / stride;
  const int triangleCount = vertexCount / 3;

  QVector3D intersectionPt, minIntersectionPt;
  float minDistance = -1;

  const int vertexSize = stride / sizeof( float );
  const int triangleSize = 3 * vertexSize;
  for ( int i = 0; i < triangleCount; ++i )
  {
    const int v0 = i * triangleSize, v1 = i * triangleSize + vertexSize, v2 = i * triangleSize + vertexSize + vertexSize;

    const QVector3D a( vertices[v0], vertices[v0 + 1], vertices[v0 + 2] );
    const QVector3D b( vertices[v1], vertices[v1 + 1], vertices[v1 + 2] );
    const QVector3D c( vertices[v2], vertices[v2 + 1], vertices[v2 + 2] );

    // Currently the worldTransform only has vertical offset, so this could be optimized by applying the transform
    // to the ray and the resulting intersecting point instead of all triangles
    // Need to check for potential performance gains.
    const QVector3D tA = worldTransform * a;
    const QVector3D tB = worldTransform * b;
    const QVector3D tC = worldTransform * c;

    QVector3D uvw;
    float t = 0;

    // We're testing both triangle orientations here and ignoring the culling mode.
    // We should probably respect the culling mode used for the entity and perform a
    // single test using the properly oriented triangle.
    if ( QgsRayCastingUtils::rayTriangleIntersection( r, tA, tB, tC, uvw, t ) ||
         QgsRayCastingUtils::rayTriangleIntersection( r, tA, tC, tB, uvw, t ) )
    {
      intersectionPt = r.point( t * r.distance() );
      const float distance = r.projectedDistance( intersectionPt );

      // we only want the first intersection of the ray with the mesh (closest to the ray origin)
      if ( minDistance == -1 || distance < minDistance )
      {
        triangleIndex = static_cast<int>( i );
        minDistance = distance;
        minIntersectionPt = intersectionPt;
      }
    }
  }

  if ( minDistance != -1 )
  {
    intPt = minIntersectionPt;
    return true;
  }
  else
    return false;
}

void QgsTessellatedPolygonGeometry::setPolygons( const QList<QgsPolygon *> &polygons, const QList<QgsFeatureId> &featureIds, const QgsPointXY &origin, float extrusionHeight, const QList<float> &extrusionHeightPerPolygon )
{
  Q_ASSERT( polygons.count() == featureIds.count() );
  mTriangleIndexStartingIndices.reserve( polygons.count() );
  mTriangleIndexFids.reserve( polygons.count() );

  QgsTessellator tessellator( origin.x(), origin.y(), mWithNormals, mInvertNormals, mAddBackFaces, false, mAddTextureCoords );
  for ( int i = 0; i < polygons.count(); ++i )
  {
    Q_ASSERT( tessellator.dataVerticesCount() % 3 == 0 );
    const uint startingTriangleIndex = static_cast<uint>( tessellator.dataVerticesCount() / 3 );
    mTriangleIndexStartingIndices.append( startingTriangleIndex );
    mTriangleIndexFids.append( featureIds[i] );

    QgsPolygon *polygon = polygons.at( i );
    const float extr = extrusionHeightPerPolygon.isEmpty() ? extrusionHeight : extrusionHeightPerPolygon.at( i );
    tessellator.addPolygon( *polygon, extr );
  }

  qDeleteAll( polygons );

  const QByteArray data( ( const char * )tessellator.data().constData(), tessellator.data().count() * sizeof( float ) );
  const int nVerts = data.count() / tessellator.stride();

  mVertexBuffer->setData( data );
  mPositionAttribute->setCount( nVerts );
  if ( mNormalAttribute )
    mNormalAttribute->setCount( nVerts );
  if ( mAddTextureCoords )
    mTextureCoordsAttribute->setCount( nVerts );
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

bool QgsTessellatedPolygonGeometry::rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QMatrix4x4 &worldTransform, QVector3D &intersectionPoint, QgsFeatureId &fid )
{
  int triangleIndex = -1;
  const int stride = static_cast<int>( mPositionAttribute->byteStride() );
  bool success = intersectionTriangles( mVertexBuffer->data(), stride, ray, worldTransform, intersectionPoint, triangleIndex );

  fid = success ? triangleIndexToFeatureId( triangleIndex ) : FID_NULL;

  return success;
}

// run binary search on a sorted array, return index i where data[i] <= v < data[i+1]
static int binary_search( uint v, const uint *data, int count )
{
  int idx0 = 0;
  int idx1 = count - 1;

  if ( v < data[0] )
    return -1;  // not in the array

  if ( v >= data[count - 1] )
    return count - 1;  // for larger values the last bin is returned

  while ( idx0 != idx1 )
  {
    const int idxPivot = ( idx0 + idx1 ) / 2;
    const uint pivot = data[idxPivot];
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
  const int i = binary_search( triangleIndex, mTriangleIndexStartingIndices.constData(), mTriangleIndexStartingIndices.count() );
  return i != -1 ? mTriangleIndexFids[i] : FID_NULL;
}
