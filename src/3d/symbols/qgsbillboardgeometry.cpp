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

#include "qgsbillboardgeometry.h"
#include "moc_qgsbillboardgeometry.cpp"

QgsBillboardGeometry::QgsBillboardGeometry( Qt3DCore::QNode *parent )
  : QGeometry( parent )
  , mVertexBuffer( new Qt3DQBuffer( this ) )
{
  setMode( Mode::PositionOnly );
}

void QgsBillboardGeometry::setMode( Mode mode )
{
  uint stride = 0;
  switch ( mode )
  {
    case Mode::PositionOnly:
      stride = 3 * sizeof( float );
      break;
    case Mode::PositionAndTextureData:
      stride = ( 3 + 2 + 2 ) * sizeof( float );
      break;
    case Mode::PositionAndTextureDataWithPixelOffsets:
      stride = ( 3 + 2 + 2 ) * sizeof( float ) + 2 * sizeof( int );
      break;
  }

  if ( mPositionAttribute && mPositionAttribute->byteStride() == stride )
  {
    // already in the target mode
    return;
  }
  else if ( mPositionAttribute )
  {
    removeAttribute( mPositionAttribute );
    delete mPositionAttribute;
    mPositionAttribute = nullptr;
  }

  mPositionAttribute = new Qt3DQAttribute( this );
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( stride );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  addAttribute( mPositionAttribute );

  switch ( mode )
  {
    case Mode::PositionOnly:
    {
      if ( mAtlasOffsetAttribute )
      {
        removeAttribute( mAtlasOffsetAttribute );
        delete mAtlasOffsetAttribute;
        mAtlasOffsetAttribute = nullptr;
      }
      if ( mAtlasSizeAttribute )
      {
        removeAttribute( mAtlasSizeAttribute );
        delete mAtlasSizeAttribute;
        mAtlasSizeAttribute = nullptr;
      }
      if ( mAtlasPixelOffsetAttribute )
      {
        removeAttribute( mAtlasPixelOffsetAttribute );
        delete mAtlasPixelOffsetAttribute;
        mAtlasPixelOffsetAttribute = nullptr;
      }
      break;
    }

    case Mode::PositionAndTextureData:
    {
      if ( mAtlasPixelOffsetAttribute )
      {
        removeAttribute( mAtlasPixelOffsetAttribute );
        delete mAtlasPixelOffsetAttribute;
        mAtlasPixelOffsetAttribute = nullptr;
      }

      mAtlasOffsetAttribute = new Qt3DQAttribute( this );
      mAtlasOffsetAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
      mAtlasOffsetAttribute->setBuffer( mVertexBuffer );
      mAtlasOffsetAttribute->setVertexBaseType( Qt3DQAttribute::Float );
      mAtlasOffsetAttribute->setVertexSize( 2 );
      mAtlasOffsetAttribute->setByteOffset( 3 * sizeof( float ) );
      mAtlasOffsetAttribute->setByteStride( stride );
      mAtlasOffsetAttribute->setName( QStringLiteral( "atlasOffset" ) );
      addAttribute( mAtlasOffsetAttribute );

      mAtlasSizeAttribute = new Qt3DQAttribute( this );
      mAtlasSizeAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
      mAtlasSizeAttribute->setBuffer( mVertexBuffer );
      mAtlasSizeAttribute->setVertexBaseType( Qt3DQAttribute::Float );
      mAtlasSizeAttribute->setVertexSize( 2 );
      mAtlasSizeAttribute->setByteOffset( ( 3 + 2 ) * sizeof( float ) );
      mAtlasSizeAttribute->setByteStride( stride );
      mAtlasSizeAttribute->setName( QStringLiteral( "atlasSize" ) );
      addAttribute( mAtlasSizeAttribute );
      break;
    }

    case Mode::PositionAndTextureDataWithPixelOffsets:
    {
      mAtlasOffsetAttribute = new Qt3DQAttribute( this );
      mAtlasOffsetAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
      mAtlasOffsetAttribute->setBuffer( mVertexBuffer );
      mAtlasOffsetAttribute->setVertexBaseType( Qt3DQAttribute::Float );
      mAtlasOffsetAttribute->setVertexSize( 2 );
      mAtlasOffsetAttribute->setByteOffset( 3 * sizeof( float ) );
      mAtlasOffsetAttribute->setByteStride( stride );
      mAtlasOffsetAttribute->setName( QStringLiteral( "atlasOffset" ) );
      addAttribute( mAtlasOffsetAttribute );

      mAtlasSizeAttribute = new Qt3DQAttribute( this );
      mAtlasSizeAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
      mAtlasSizeAttribute->setBuffer( mVertexBuffer );
      mAtlasSizeAttribute->setVertexBaseType( Qt3DQAttribute::Float );
      mAtlasSizeAttribute->setVertexSize( 2 );
      mAtlasSizeAttribute->setByteOffset( ( 3 + 2 ) * sizeof( float ) );
      mAtlasSizeAttribute->setByteStride( stride );
      mAtlasSizeAttribute->setName( QStringLiteral( "atlasSize" ) );
      addAttribute( mAtlasSizeAttribute );

      mAtlasPixelOffsetAttribute = new Qt3DQAttribute( this );
      mAtlasPixelOffsetAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
      mAtlasPixelOffsetAttribute->setBuffer( mVertexBuffer );
      mAtlasPixelOffsetAttribute->setVertexBaseType( Qt3DQAttribute::Int );
      mAtlasPixelOffsetAttribute->setVertexSize( 2 );
      mAtlasPixelOffsetAttribute->setByteOffset( ( 3 + 2 + 2 ) * sizeof( float ) );
      mAtlasPixelOffsetAttribute->setByteStride( stride );
      mAtlasPixelOffsetAttribute->setName( QStringLiteral( "pixelOffset" ) );
      addAttribute( mAtlasPixelOffsetAttribute );
      break;
    }
  }
}

void QgsBillboardGeometry::setPositions( const QVector<QVector3D> &vertices )
{
  setMode( Mode::PositionOnly );

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

///@cond PRIVATE
#pragma pack( push, 1 )
struct BillboardVertex
{
    float position[3];
    float textureAtlasOffset[2];
    float textureAtlasSize[2];
};

struct BillboardVertexWithPixelOffset : BillboardVertex
{
    int pixelOffset[2];
};
#pragma pack( pop )

template<typename VertexType>
QByteArray createVertexBuffer( const QVector<QgsBillboardGeometry::BillboardAtlasData> &billboards )
{
  QByteArray buffer;
  buffer.resize( billboards.size() * sizeof( VertexType ) );
  auto *vertexData = reinterpret_cast<VertexType *>( buffer.data() );

  int idx = 0;
  for ( const QgsBillboardGeometry::BillboardAtlasData &billboard : billboards )
  {
    VertexType &vertex = vertexData[idx++];

    vertex.position[0] = billboard.position.x();
    vertex.position[1] = billboard.position.y();
    vertex.position[2] = billboard.position.z();

    vertex.textureAtlasOffset[0] = billboard.textureAtlasOffset.x();
    vertex.textureAtlasOffset[1] = billboard.textureAtlasOffset.y();

    vertex.textureAtlasSize[0] = billboard.textureAtlasSize.x();
    vertex.textureAtlasSize[1] = billboard.textureAtlasSize.y();

    if constexpr ( std::is_same_v<VertexType, BillboardVertexWithPixelOffset> )
    {
      vertex.pixelOffset[0] = billboard.pixelOffset.x();
      vertex.pixelOffset[1] = billboard.pixelOffset.y();
    }
  }
  return buffer;
}
///@endcond

void QgsBillboardGeometry::setBillboardData( const QVector<BillboardAtlasData> &billboards, bool includePixelOffsets )
{
  if ( includePixelOffsets )
    setMode( Mode::PositionAndTextureDataWithPixelOffsets );
  else
    setMode( Mode::PositionAndTextureData );


  QByteArray vertexBufferData;
  if ( includePixelOffsets )
  {
    vertexBufferData = createVertexBuffer<BillboardVertexWithPixelOffset>( billboards );
  }
  else
  {
    vertexBufferData = createVertexBuffer<BillboardVertex>( billboards );
  }

  mVertexCount = billboards.count();
  mVertexBuffer->setData( vertexBufferData );

  emit countChanged( mVertexCount );
}

int QgsBillboardGeometry::count() const
{
  return mVertexCount;
}
