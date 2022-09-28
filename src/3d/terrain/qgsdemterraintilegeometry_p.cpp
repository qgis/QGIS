/***************************************************************************
  qgsdemterraintilegeometry_p.cpp
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

#include "qgsdemterraintilegeometry_p.h"
#include <QMatrix4x4>


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAbstractFunctor>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QAbstractFunctor Qt3DQAbstractFunctor;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QAbstractFunctor>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QAbstractFunctor Qt3DQAbstractFunctor;
#endif
#include <limits>
#include <cmath>
#include "qgsraycastingutils_p.h"
#include "qgis.h"

///@cond PRIVATE

using namespace Qt3DRender;

static QByteArray createPlaneVertexData( int res, float side, float vertScale, float skirtHeight, const QByteArray &heights )
{
  Q_ASSERT( res >= 2 );
  Q_ASSERT( heights.count() == res * res * static_cast<int>( sizeof( float ) ) );

  const float *zData = ( const float * ) heights.constData();
  const float *zBits = zData;

  const int nVerts = ( res + 2 ) * ( res + 2 );

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos, vec2 texCoord, vec3 normal, vec4 tangent
  const quint32 elementSize = 3 + 2 + 3;
  const quint32 stride = elementSize * sizeof( float );
  QByteArray bufferBytes;
  bufferBytes.resize( stride * nVerts );
  float *fptr = reinterpret_cast<float *>( bufferBytes.data() );

  float w = 1, h = 1;
  QSize resolution( res, res );
  const float x0 = -w / 2.0f;
  const float z0 = -h / 2.0f;
  const float dx = w / ( resolution.width() - 1 );
  const float dz = h / ( resolution.height() - 1 );
  const float du = 1.0 / ( resolution.width() - 1 );
  const float dv = 1.0 / ( resolution.height() - 1 );

  // the height of vertices with no-data value... the value should not really matter
  // as we do not create valid triangles that would use such vertices
  const float noDataHeight = 0;

  const int iMax = resolution.width() - 1;
  const int jMax = resolution.height() - 1;

  // Iterate over z
  for ( int j = -1; j <= resolution.height(); ++j )
  {
    int jBound = std::clamp( j, 0, jMax );
    const float z = z0 + static_cast<float>( jBound ) * dz;
    const float v = static_cast<float>( jBound ) * dv;

    // Iterate over x
    for ( int i = -1; i <= resolution.width(); ++i )
    {
      int iBound = std::clamp( i, 0, iMax );
      const float x = x0 + static_cast<float>( iBound ) * dx;
      const float u = static_cast<float>( iBound ) * du;

      float height;
      if ( i == iBound && j == jBound )
        height = *zBits++;
      else
        height = zData[ jBound * resolution.width() + iBound ] - skirtHeight;

      if ( std::isnan( height ) )
        height = noDataHeight;

      // position
      *fptr++ = x;
      *fptr++ = height / side * vertScale;
      *fptr++ = z;

      // texture coordinates
      *fptr++ = u;
      *fptr++ = v;

      // calculate normal coordinates
#define zAt( ii, jj )  zData[ jj * resolution.width() + ii ] * vertScale
      float zi0 = zAt( std::clamp( i - 1, 0, iMax ), jBound );
      float zi1 = zAt( std::clamp( i + 1, 0, iMax ), jBound );
      float zj0 = zAt( iBound, std::clamp( j - 1, 0, jMax ) );
      float zj1 = zAt( iBound, std::clamp( j + 1, 0, jMax ) );

      QVector3D n;
      if ( std::isnan( zi0 ) || std::isnan( zi1 ) || std::isnan( zj0 ) || std::isnan( zj1 ) )
        n = QVector3D( 0, 1, 0 );
      else
      {
        float di, dj;
        float zij = height * vertScale;

        if ( i == 0 )
          di = 2 * ( zij - zi1 );
        else if ( i == iMax )
          di = 2 * ( zi0 - zij );
        else
          di = zi0 - zi1;

        if ( j == 0 )
          dj = 2 * ( zij - zj1 );
        else if ( j == jMax )
          dj = 2 * ( zj0 - zij );
        else
          dj = zj0 - zj1;

        n = QVector3D( di, 2 * side / res, dj );
        n.normalize();
      }

      *fptr++ = n.x();
      *fptr++ = n.y();
      *fptr++ = n.z();
    }
  }

  return bufferBytes;
}

inline int ijToHeightMapIndex( int i, int j, int resX, int resZ )
{
  i = std::clamp( i, 1, resX ) - 1;
  j = std::clamp( j, 1, resZ ) - 1;
  return j * resX + i;
}

static bool hasNoData( int i, int j, const float *heightMap, int resX, int resZ )
{
  return std::isnan( heightMap[ ijToHeightMapIndex( i, j, resX, resZ ) ] ) ||
         std::isnan( heightMap[ ijToHeightMapIndex( i + 1, j, resX, resZ ) ] ) ||
         std::isnan( heightMap[ ijToHeightMapIndex( i, j + 1, resX, resZ ) ] ) ||
         std::isnan( heightMap[ ijToHeightMapIndex( i + 1, j + 1, resX, resZ ) ] );
}

static QByteArray createPlaneIndexData( int res, const QByteArray &heightMap )
{
  QSize resolution( res, res );
  int numVerticesX = resolution.width() + 2;
  int numVerticesZ = resolution.height() + 2;

  // Create the index data. 2 triangles per rectangular face
  const int faces = 2 * ( numVerticesX - 1 ) * ( numVerticesZ - 1 );
  const quint32 indices = 3 * faces;
  Q_ASSERT( indices < std::numeric_limits<quint32>::max() );
  QByteArray indexBytes;
  indexBytes.resize( indices * sizeof( quint32 ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  const float *heightMapFloat = reinterpret_cast<const float *>( heightMap.constData() );

  // Iterate over z
  for ( int j = 0; j < numVerticesZ - 1; ++j )
  {
    const int rowStartIndex = j * numVerticesX;
    const int nextRowStartIndex = ( j + 1 ) * numVerticesX;

    // Iterate over x
    for ( int i = 0; i < numVerticesX - 1; ++i )
    {
      if ( hasNoData( i, j, heightMapFloat, res, res ) )
      {
        // at least one corner of the quad has no-data value
        // so let's make two invalid triangles
        *indexPtr++ = rowStartIndex + i;
        *indexPtr++ = rowStartIndex + i;
        *indexPtr++ = rowStartIndex + i;

        *indexPtr++ = rowStartIndex + i;
        *indexPtr++ = rowStartIndex + i;
        *indexPtr++ = rowStartIndex + i;
        continue;
      }

      // Split quad into two triangles
      *indexPtr++ = rowStartIndex + i;
      *indexPtr++ = nextRowStartIndex + i;
      *indexPtr++ = rowStartIndex + i + 1;

      *indexPtr++ = nextRowStartIndex + i;
      *indexPtr++ = nextRowStartIndex + i + 1;
      *indexPtr++ = rowStartIndex + i + 1;
    }
  }

  return indexBytes;
}

// QAbstractFunctor marked as deprecated in 5.15, but undeprecated for Qt 6.0. TODO -- remove when we require 6.0
Q_NOWARN_DEPRECATED_PUSH

//! Generates vertex buffer for DEM terrain tiles
class PlaneVertexBufferFunctor : public Qt3DQAbstractFunctor
{
  public:
    explicit PlaneVertexBufferFunctor( int resolution, float side, float vertScale, float skirtHeight, const QByteArray &heightMap )
      : mResolution( resolution )
      , mSide( side )
      , mVertScale( vertScale )
      , mSkirtHeight( skirtHeight )
      , mHeightMap( heightMap )
    {}

    QByteArray operator()()
    {
      return createPlaneVertexData( mResolution, mSide, mVertScale, mSkirtHeight, mHeightMap );
    }

    bool operator ==( const Qt3DQAbstractFunctor &other ) const
    {
      const PlaneVertexBufferFunctor *otherFunctor = functor_cast<PlaneVertexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mResolution == mResolution &&
                 otherFunctor->mSide == mSide &&
                 otherFunctor->mVertScale == mVertScale &&
                 otherFunctor->mSkirtHeight == mSkirtHeight &&
                 otherFunctor->mHeightMap == mHeightMap );
      return false;
    }

    QT3D_FUNCTOR( PlaneVertexBufferFunctor )

  private:
    int mResolution;
    float mSide;
    float mVertScale;
    float mSkirtHeight;
    QByteArray mHeightMap;
};


//! Generates index buffer for DEM terrain tiles
class PlaneIndexBufferFunctor: public Qt3DQAbstractFunctor
{
  public:
    explicit PlaneIndexBufferFunctor( int resolution, const QByteArray &heightMap )
      : mResolution( resolution )
      , mHeightMap( heightMap )
    {}

    QByteArray operator()()
    {
      return createPlaneIndexData( mResolution, mHeightMap );
    }

    bool operator ==( const Qt3DQAbstractFunctor &other ) const
    {
      const PlaneIndexBufferFunctor *otherFunctor = functor_cast<PlaneIndexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mResolution == mResolution );
      return false;
    }

    QT3D_FUNCTOR( PlaneIndexBufferFunctor )

  private:
    int mResolution;
    QByteArray mHeightMap;
};

Q_NOWARN_DEPRECATED_POP

// ------------


DemTerrainTileGeometry::DemTerrainTileGeometry( int resolution, float side, float vertScale, float skirtHeight, const QByteArray &heightMap, DemTerrainTileGeometry::QNode *parent )
  : QGeometry( parent )
  , mResolution( resolution )
  , mSide( side )
  , mVertScale( vertScale )
  , mSkirtHeight( skirtHeight )
  , mHeightMap( heightMap )
{
  init();
}

static bool intersectionDemTriangles( const QByteArray &vertexBuf, const QByteArray &indexBuf, const QgsRayCastingUtils::Ray3D &r, const QMatrix4x4 &worldTransform, QVector3D &intPt )
{
  // WARNING! this code is specific to how vertex buffers are built for DEM tiles,
  // it is not usable for any mesh...

  const float *vertices = reinterpret_cast<const float *>( vertexBuf.constData() );
  const uint *indices = reinterpret_cast<const uint *>( indexBuf.constData() );
#ifdef QGISDEBUG
  int vertexCnt = vertexBuf.count() / sizeof( float );
  Q_ASSERT( vertexCnt % 8 == 0 );
#endif
  int indexCnt = indexBuf.count() / sizeof( uint );
  Q_ASSERT( indexCnt % 3 == 0 );
  int triangleCount = indexCnt / 3;

  QVector3D intersectionPt, minIntersectionPt;
  float distance;
  float minDistance = -1;

  for ( int i = 0; i < triangleCount; ++i )
  {
    int v0 = indices[i * 3], v1 = indices[i * 3 + 1], v2 = indices[i * 3 + 2];
    QVector3D a( vertices[v0 * 8], vertices[v0 * 8 + 1], vertices[v0 * 8 + 2] );
    QVector3D b( vertices[v1 * 8], vertices[v1 * 8 + 1], vertices[v1 * 8 + 2] );
    QVector3D c( vertices[v2 * 8], vertices[v2 * 8 + 1], vertices[v2 * 8 + 2] );

    const QVector3D tA = worldTransform * a;
    const QVector3D tB = worldTransform * b;
    const QVector3D tC = worldTransform * c;

    QVector3D uvw;
    float t = 0;
    if ( QgsRayCastingUtils::rayTriangleIntersection( r, tA, tB, tC, uvw, t ) )
    {
      intersectionPt = r.point( t * r.distance() );
      distance = r.projectedDistance( intersectionPt );

      // we only want the first intersection of the ray with the mesh (closest to the ray origin)
      if ( minDistance == -1 || distance < minDistance )
      {
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

bool DemTerrainTileGeometry::rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QMatrix4x4 &worldTransform, QVector3D &intersectionPoint )
{
  return intersectionDemTriangles( mVertexBuffer->data(), mIndexBuffer->data(), ray, worldTransform, intersectionPoint );
}

void DemTerrainTileGeometry::init()
{
  mPositionAttribute = new Qt3DQAttribute( this );
  mNormalAttribute = new Qt3DQAttribute( this );
  mTexCoordAttribute = new Qt3DQAttribute( this );
  mIndexAttribute = new Qt3DQAttribute( this );
  mVertexBuffer = new Qt3DQBuffer( this );
  mIndexBuffer = new Qt3DQBuffer( this );

  int nVertsX = mResolution + 2;
  int nVertsZ = mResolution + 2;
  const int nVerts = nVertsX * nVertsZ;
  const int stride = ( 3 + 2 + 3 ) * sizeof( float );
  const int faces = 2 * ( nVertsX - 1 ) * ( nVertsZ - 1 );

  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setByteStride( stride );
  mPositionAttribute->setCount( nVerts );

  mTexCoordAttribute->setName( Qt3DQAttribute::defaultTextureCoordinateAttributeName() );
  mTexCoordAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mTexCoordAttribute->setVertexSize( 2 );
  mTexCoordAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mTexCoordAttribute->setBuffer( mVertexBuffer );
  mTexCoordAttribute->setByteStride( stride );
  mTexCoordAttribute->setByteOffset( 3 * sizeof( float ) );
  mTexCoordAttribute->setCount( nVerts );

  mNormalAttribute->setName( Qt3DQAttribute::defaultNormalAttributeName() );
  mNormalAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mNormalAttribute->setVertexSize( 3 );
  mNormalAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mNormalAttribute->setBuffer( mVertexBuffer );
  mNormalAttribute->setByteStride( stride );
  mNormalAttribute->setByteOffset( 5 * sizeof( float ) );
  mNormalAttribute->setCount( nVerts );

  mIndexAttribute->setAttributeType( Qt3DQAttribute::IndexAttribute );
  mIndexAttribute->setVertexBaseType( Qt3DQAttribute::UnsignedInt );
  mIndexAttribute->setBuffer( mIndexBuffer );

  // Each primitive has 3 vertives
  mIndexAttribute->setCount( faces * 3 );

  // switched to setting data instead of just setting data generators because we also need the buffers
  // available for ray-mesh intersections and we can't access the private copy of data in Qt (if there is any)

  mVertexBuffer->setData( PlaneVertexBufferFunctor( mResolution, mSide, mVertScale, mSkirtHeight, mHeightMap )() );
  mIndexBuffer->setData( PlaneIndexBufferFunctor( mResolution, mHeightMap )() );

  addAttribute( mPositionAttribute );
  addAttribute( mTexCoordAttribute );
  addAttribute( mNormalAttribute );
  addAttribute( mIndexAttribute );
}

/// @endcond
