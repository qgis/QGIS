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

#include "qgsbillboardgeometry.h"

#include <QString>
#include <QVector3D>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>

#include "moc_qgsbillboardgeometry.cpp"

using namespace Qt::StringLiterals;

QgsBillboardGeometry::QgsBillboardGeometry( Qt3DCore::QNode *parent )
  : QGeometry( parent )
  , mVertexBuffer( new Qt3DCore::QBuffer( this ) )
{
  static constexpr float quadVertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
  Qt3DCore::QBuffer *quadVertexBuffer = new Qt3DCore::QBuffer( this );
  quadVertexBuffer->setData( QByteArray( reinterpret_cast<const char *>( quadVertices ), sizeof( quadVertices ) ) );

  Qt3DCore::QAttribute *quadVertexAttribute = new Qt3DCore::QAttribute( this );
  quadVertexAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  quadVertexAttribute->setBuffer( quadVertexBuffer );
  quadVertexAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  quadVertexAttribute->setVertexSize( 2 );
  quadVertexAttribute->setByteOffset( 0 );
  quadVertexAttribute->setByteStride( 2 * sizeof( float ) );
  quadVertexAttribute->setCount( 4 );
  quadVertexAttribute->setName( u"vertexPosition"_s );
  addAttribute( quadVertexAttribute );

  setAttributes( Attribute::Position );
}

void QgsBillboardGeometry::setAttributes( QgsBillboardGeometry::Attributes attributes )
{
  // position must always be present
  Q_ASSERT( attributes.testFlag( Attribute::Position ) );

  if ( mAttributes == attributes )
    return;

  mAttributes = attributes;

  // position is always present
  uint stride = 3 * sizeof( float );
  if ( attributes.testFlag( Attribute::Size ) )
  {
    stride += 2 * sizeof( float );
  }
  if ( attributes.testFlag( Attribute::TextureData ) )
  {
    stride += ( 2 + 2 ) * sizeof( float );
  }
  if ( attributes.testFlag( Attribute::PixelOffsets ) )
  {
    stride += 2 * sizeof( int );
  }

  if ( mPositionAttribute )
  {
    removeAttribute( mPositionAttribute );
    delete mPositionAttribute;
    mPositionAttribute = nullptr;
  }

  uint offset = 0;
  mPositionAttribute = new Qt3DCore::QAttribute( this );
  mPositionAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setByteOffset( offset );
  mPositionAttribute->setByteStride( stride );
  mPositionAttribute->setDivisor( 1 );
  mPositionAttribute->setName( "instancePosition" );
  addAttribute( mPositionAttribute );
  setBoundingVolumePositionAttribute( mPositionAttribute );
  offset += 3 * sizeof( float );

  if ( attributes.testFlag( Attribute::Size ) )
  {
    mSizeAttribute = new Qt3DCore::QAttribute( this );
    mSizeAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
    mSizeAttribute->setBuffer( mVertexBuffer );
    mSizeAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
    mSizeAttribute->setVertexSize( 2 );
    mSizeAttribute->setByteOffset( offset );
    mSizeAttribute->setByteStride( stride );
    mSizeAttribute->setDivisor( 1 );
    mSizeAttribute->setName( u"instanceSize"_s );
    addAttribute( mSizeAttribute );
    offset += 2 * sizeof( float );
  }
  else
  {
    removeAttribute( mSizeAttribute );
    delete mSizeAttribute;
    mSizeAttribute = nullptr;
  }

  if ( attributes.testFlag( Attribute::TextureData ) )
  {
    mAtlasOffsetAttribute = new Qt3DCore::QAttribute( this );
    mAtlasOffsetAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
    mAtlasOffsetAttribute->setBuffer( mVertexBuffer );
    mAtlasOffsetAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
    mAtlasOffsetAttribute->setVertexSize( 2 );
    mAtlasOffsetAttribute->setByteOffset( offset );
    mAtlasOffsetAttribute->setByteStride( stride );
    mAtlasOffsetAttribute->setDivisor( 1 );
    mAtlasOffsetAttribute->setName( u"atlasOffset"_s );
    addAttribute( mAtlasOffsetAttribute );
    offset += 2 * sizeof( float );

    mAtlasSizeAttribute = new Qt3DCore::QAttribute( this );
    mAtlasSizeAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
    mAtlasSizeAttribute->setBuffer( mVertexBuffer );
    mAtlasSizeAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
    mAtlasSizeAttribute->setVertexSize( 2 );
    mAtlasSizeAttribute->setByteOffset( offset );
    mAtlasSizeAttribute->setByteStride( stride );
    mAtlasSizeAttribute->setDivisor( 1 );
    mAtlasSizeAttribute->setName( u"atlasSize"_s );
    addAttribute( mAtlasSizeAttribute );
    offset += 2 * sizeof( float );
  }
  else
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
  }

  if ( attributes.testFlag( Attribute::PixelOffsets ) )
  {
    mAtlasPixelOffsetAttribute = new Qt3DCore::QAttribute( this );
    mAtlasPixelOffsetAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
    mAtlasPixelOffsetAttribute->setBuffer( mVertexBuffer );
    mAtlasPixelOffsetAttribute->setVertexBaseType( Qt3DCore::QAttribute::Int );
    mAtlasPixelOffsetAttribute->setVertexSize( 2 );
    mAtlasPixelOffsetAttribute->setByteOffset( offset );
    mAtlasPixelOffsetAttribute->setByteStride( stride );
    mAtlasPixelOffsetAttribute->setDivisor( 1 );
    mAtlasPixelOffsetAttribute->setName( u"pixelOffset"_s );
    addAttribute( mAtlasPixelOffsetAttribute );
    offset += 2 * sizeof( int );
  }
  else
  {
    removeAttribute( mAtlasPixelOffsetAttribute );
    delete mAtlasPixelOffsetAttribute;
    mAtlasPixelOffsetAttribute = nullptr;
  }
}

void QgsBillboardGeometry::setPositions( const QVector<QVector3D> &vertices )
{
  setAttributes( Attribute::Position );

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
  mPositionAttribute->setCount( mVertexCount );

  emit countChanged( mVertexCount );
}

void QgsBillboardGeometry::setPositionsAndSizes( const QVector<QVector3D> &positions, const QVector<QSizeF> &sizes )
{
  Attributes attributes = Attribute::Position;
  attributes.setFlag( Attribute::Size );
  setAttributes( attributes );

  const qsizetype size = std::min( positions.size(), sizes.size() );
  QByteArray vertexBufferData;
  vertexBufferData.resize( size * ( 3 + 2 ) * sizeof( float ) );
  float *rawArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( qsizetype i = 0; i < size; ++i )
  {
    rawArray[idx++] = positions[i].x();
    rawArray[idx++] = positions[i].y();
    rawArray[idx++] = positions[i].z();

    rawArray[idx++] = sizes[i].width();
    rawArray[idx++] = sizes[i].height();
  }

  mVertexCount = size;
  mVertexBuffer->setData( vertexBufferData );
  mPositionAttribute->setCount( mVertexCount );
  mSizeAttribute->setCount( mVertexCount );

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

template<typename VertexType> QByteArray createVertexBuffer( const QVector<QgsBillboardGeometry::BillboardAtlasData> &billboards )
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
  Attributes attributes = Attribute::Position;
  attributes.setFlag( Attribute::TextureData, true );
  attributes.setFlag( Attribute::PixelOffsets, includePixelOffsets );
  setAttributes( attributes );

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
  mPositionAttribute->setCount( mVertexCount );

  emit countChanged( mVertexCount );
}

int QgsBillboardGeometry::count() const
{
  return mVertexCount;
}
