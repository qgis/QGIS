/***************************************************************************
  qgistindemterraintilegeometry_p.cpp
  --------------------------------------
  Date                 : october 2019
  Copyright            : (C) 2019 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstindemterraintilegeometry_p.h"
#include "qgstindemterraintileloader_p.h"
#include <Qt3DRender/qattribute.h>
#include <Qt3DRender/qbuffer.h>

#include <Qt3DRender/qbufferdatagenerator.h>

///@cond PRIVATE

using namespace Qt3DRender;

static QByteArray createPlaneVertexData( QgsTriangularMeshTile mesh, float vertScale )
{
  const int nVerts = mesh.verticesCount();

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos, vec2 texCoord, vec3 normal, vec4 tangent
  const quint32 elementSize = 3 + 2 + 3;
  const quint32 stride = elementSize * sizeof( float );
  QByteArray bufferBytes;
  bufferBytes.resize( stride * nVerts );
  QgsRectangle realExtent = mesh.realTileExtent();
  double w = realExtent.width();
  double h = realExtent.height() ;
  double x0 = realExtent.xMinimum();
  double y0 = realExtent.yMaximum();

  float *fptr = reinterpret_cast<float *>( bufferBytes.data() );

  for ( int i = 0; i < nVerts; i++ )
  {
    const QgsMeshVertex &vert = mesh.vertex( i );
    *fptr++ = float( vert.x() );
    *fptr++ = float( vert.z() ) * vertScale ;
    *fptr++ = float( -vert.y() );

    *fptr++ = float( ( vert.x() - x0 ) / w );
    *fptr++ = float( ( y0 - vert.y() ) / h );


    QVector3D normal = mesh.vertexUnitNormalVector( i );
    normal = QVector3D( normal.x(), -normal.y(), normal.z() * vertScale );
    normal.normalized();

    *fptr++ = normal.x();
    *fptr++ = normal.z();
    *fptr++ = normal.y();
  }

  return bufferBytes;
}

static QByteArray createPlaneIndexData( QgsTriangularMeshTile mesh )
{

  const int faces = mesh.faceCount();
  const quint32 indices = static_cast<quint32>( 3 * faces );
  Q_ASSERT( indices < std::numeric_limits<quint32>::max() );
  QByteArray indexBytes;
  indexBytes.resize( int( indices * sizeof( quint32 ) ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  for ( int i = 0; i < faces; ++i )
  {

    const QgsMeshFace &face = mesh.triangle( i );
    for ( int i = 0; i < 3; ++i )
      *indexPtr++ = quint32( face.at( i ) );
  }

  return indexBytes;
}

;

//! Generates vertex buffer for DEM terrain tiles
class TinPlaneVertexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit TinPlaneVertexBufferFunctor( QgsTriangularMeshTile mesh,  float vertScale )
      : mMesh( mesh ),
        mVertScale( vertScale )
    {}

    QByteArray operator()() final
    {
      return createPlaneVertexData( mMesh, mVertScale );
    }

    bool operator ==( const QBufferDataGenerator &other ) const final
    {
      const TinPlaneVertexBufferFunctor *otherFunctor = functor_cast<TinPlaneVertexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mMesh == mMesh );
      return false;
    }

    QT3D_FUNCTOR( TinPlaneVertexBufferFunctor )

  private:

    QgsTriangularMeshTile mMesh;
    float mVertScale;

};



//! Generates index buffer for DEM terrain tiles
class TinPlaneIndexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit TinPlaneIndexBufferFunctor( QgsTriangularMeshTile mesh )
      : mMesh( mesh )
    {}

    QByteArray operator()() final
    {
      return createPlaneIndexData( mMesh );
    }

    bool operator ==( const QBufferDataGenerator &other ) const final
    {
      const TinPlaneIndexBufferFunctor *otherFunctor = functor_cast<TinPlaneIndexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mMesh == mMesh );
      return false;
    }

    QT3D_FUNCTOR( TinPlaneIndexBufferFunctor )

  private:
    QgsTriangularMeshTile mMesh;
};


QgsTinDemTerrainTileGeometry_p::QgsTinDemTerrainTileGeometry_p( QgsTriangularMeshTile meshTile, float verticaleScale, QgsTinDemTerrainTileGeometry_p::QNode *parent ):
  QGeometry( parent ),
  mTriangularMeshTile( meshTile ),
  mVertScale( verticaleScale )
{
  init();
}


void QgsTinDemTerrainTileGeometry_p::init()
{
  mPositionAttribute = new QAttribute( this );
  mNormalAttribute = new QAttribute( this );
  mTexCoordAttribute = new QAttribute( this );
  mIndexAttribute = new QAttribute( this );
  mVertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );
  mIndexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::IndexBuffer, this );


  const int stride = ( 3 + 2 + 3 ) * sizeof( float );
  const uint nVerts = uint( mTriangularMeshTile.verticesCount() );

  mPositionAttribute->setName( QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setVertexBaseType( QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setAttributeType( QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setByteStride( stride );
  mPositionAttribute->setCount( nVerts );

  mTexCoordAttribute->setName( QAttribute::defaultTextureCoordinateAttributeName() );
  mTexCoordAttribute->setVertexBaseType( QAttribute::Float );
  mTexCoordAttribute->setVertexSize( 2 );
  mTexCoordAttribute->setAttributeType( QAttribute::VertexAttribute );
  mTexCoordAttribute->setBuffer( mVertexBuffer );
  mTexCoordAttribute->setByteStride( stride );
  mTexCoordAttribute->setByteOffset( 3 * sizeof( float ) );
  mTexCoordAttribute->setCount( nVerts );

  mNormalAttribute->setName( QAttribute::defaultNormalAttributeName() );
  mNormalAttribute->setVertexBaseType( QAttribute::Float );
  mNormalAttribute->setVertexSize( 3 );
  mNormalAttribute->setAttributeType( QAttribute::VertexAttribute );
  mNormalAttribute->setBuffer( mVertexBuffer );
  mNormalAttribute->setByteStride( stride );
  mNormalAttribute->setByteOffset( 5 * sizeof( float ) );
  mNormalAttribute->setCount( nVerts );

  mIndexAttribute->setAttributeType( QAttribute::IndexAttribute );
  mIndexAttribute->setVertexBaseType( QAttribute::UnsignedInt );
  mIndexAttribute->setBuffer( mIndexBuffer );

  // Each primitive has 3 vertives
  mIndexAttribute->setCount( uint( mTriangularMeshTile.faceCount() ) * 3 );

  // switched to setting data instead of just setting data generators because we also need the buffers
  // available for ray-mesh intersections and we can't access the private copy of data in Qt (if there is any)
  mVertexBuffer->setData( TinPlaneVertexBufferFunctor( mTriangularMeshTile, mVertScale )() );
  mIndexBuffer->setData( TinPlaneIndexBufferFunctor( mTriangularMeshTile )() );

  addAttribute( mPositionAttribute );
  addAttribute( mTexCoordAttribute );
  addAttribute( mNormalAttribute );
  addAttribute( mIndexAttribute );
}


/// @endcond
