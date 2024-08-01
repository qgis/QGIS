/***************************************************************************
                             qgsquantizedmeshtiles.cpp
                             ----------------------------
    begin                : May 2024
    copyright            : (C) 2024 by David Koňařík
    email                : dvdkon at konarici dot cz

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquantizedmeshtiles.h"

#include "qgsexception.h"
#include <algorithm>
#include <cstddef>
#include <qstringliteral.h>

///@cond PRIVATE

/**
 * \brief Utility class for safely reading data by chunks from a vector
 */
class VectorStream
{
  public:
    size_t mOffset = 0;
    VectorStream( const QByteArray &vec ) : mVec( vec ) {}
    const void *read( size_t bytes )
    {
      if ( ( size_t ) mVec.size() < mOffset + bytes )
      {
        throw QgsQuantizedMeshParsingException( "Tried to read beyond EOF when parsing quantized mesh tile" );
      }
      const void *data = mVec.data() + mOffset;
      mOffset += bytes;
      return data;
    }
    size_t remaining() const
    {
      return mVec.size() - mOffset;
    }
  private:
    const QByteArray &mVec;
};

///@endcond

// Copied from specification
static uint16_t zigZagDecode( uint16_t value )
{
  return ( value >> 1 ) ^ ( -( value & 1 ) );
}

static std::vector<uint32_t> parseU32OrU16Array( VectorStream &stream,
    bool isU32, size_t countMult )
{
  std::vector<uint32_t> values;
  if ( isU32 )
  {
    // Pad to multiple of 4
    stream.read( stream.mOffset % 4 );
    const uint32_t *countBase =
      reinterpret_cast<const uint32_t *>( stream.read( sizeof( uint32_t ) ) );
    size_t count = static_cast<size_t>( *countBase ) * countMult;
    values.resize( count );
    const uint32_t *valuesRaw = reinterpret_cast<const uint32_t *>(
                                  stream.read( sizeof( uint32_t ) * count ) );
    std::copy( valuesRaw, valuesRaw + count, values.begin() );
  }
  else
  {
    const uint32_t *countBase =
      reinterpret_cast<const uint32_t *>( stream.read( sizeof( uint32_t ) ) );
    size_t count = static_cast<size_t>( *countBase ) * countMult;
    values.resize( count );
    const uint16_t *valuesRaw = reinterpret_cast<const uint16_t *>(
                                  stream.read( sizeof( uint16_t ) * count ) );
    std::copy( valuesRaw, valuesRaw + count, values.begin() );
  }
  return values;
}

QgsQuantizedMeshTile::QgsQuantizedMeshTile( const QByteArray &data )
{
  // Check if machine is big endian
  uint16_t endiannessCheck = 0x1020;
  if ( reinterpret_cast<char *>( &endiannessCheck )[0] == 0x10 )
    throw QgsNotSupportedException(
      "Parsing quantized mesh tiles on big endian machines is not supported" );

  VectorStream stream( data );
  mHeader = *reinterpret_cast<const QgsQuantizedMeshHeader *>(
              stream.read( sizeof( QgsQuantizedMeshHeader ) ) );

  auto uArr = reinterpret_cast<const uint16_t *>(
                stream.read( sizeof( uint16_t ) * mHeader.vertexCount ) );
  auto vArr = reinterpret_cast<const uint16_t *>(
                stream.read( sizeof( uint16_t ) * mHeader.vertexCount ) );
  auto heightArr = reinterpret_cast<const uint16_t *>(
                     stream.read( sizeof( uint16_t ) * mHeader.vertexCount ) );

  uint16_t u = 0, v = 0, height = 0;
  mVertexCoords.resize( static_cast<size_t>( mHeader.vertexCount ) * 3 );
  for ( size_t i = 0; i < mHeader.vertexCount; i++ )
  {
    u += zigZagDecode( uArr[i] );
    v += zigZagDecode( vArr[i] );
    height += zigZagDecode( heightArr[i] );

    mVertexCoords[i * 3] = u;
    mVertexCoords[i * 3 + 1] = v;
    mVertexCoords[i * 3 + 2] = height;
  }

  bool vertexIndices32Bit = mHeader.vertexCount > 65536;
  mTriangleIndices =
    parseU32OrU16Array( stream, vertexIndices32Bit, 3 );
  uint32_t highest = 0;
  for ( auto &idx : mTriangleIndices )
  {
    uint32_t code = idx;
    idx = highest - code;
    if ( code == 0 )
      highest++;
  }

  mWestVertices =
    parseU32OrU16Array( stream, vertexIndices32Bit, 1 );
  mSouthVertices =
    parseU32OrU16Array( stream, vertexIndices32Bit, 1 );
  mEastVertices =
    parseU32OrU16Array( stream, vertexIndices32Bit, 1 );
  mNorthVertices =
    parseU32OrU16Array( stream, vertexIndices32Bit, 1 );

  while ( stream.remaining() > 0 )
  {
    uint8_t extensionId =
      *reinterpret_cast<const uint8_t *>( stream.read( sizeof( char ) ) );
    uint32_t length =
      *reinterpret_cast<const uint32_t *>( stream.read( sizeof( uint32_t ) ) );
    std::vector<char> data( length );
    const char *dataPtr = reinterpret_cast<const char *>( stream.read( length ) );
    std::copy( dataPtr, dataPtr + length, data.begin() );
    mExtensions[extensionId] = data;
  }
}

void QgsQuantizedMeshTile::removeDegenerateTriangles()
{
  std::vector<uint32_t> newTriangleIndices;
  for ( size_t i = 0; i < mTriangleIndices.size(); i += 3 )
  {
    uint32_t a = mTriangleIndices[i];
    uint32_t b = mTriangleIndices[i + 1];
    uint32_t c = mTriangleIndices[i + 2];
    if ( a != b && b != c && a != c )
    {
      newTriangleIndices.insert( newTriangleIndices.end(), {a, b, c} );
    }
  }
  mTriangleIndices = newTriangleIndices;
}

tinygltf::Model QgsQuantizedMeshTile::toGltf()
{
  tinygltf::Model model;

  tinygltf::Buffer vertexBuffer;
  vertexBuffer.data.resize( mVertexCoords.size() * sizeof( float ) );
  std::vector<double> coordMinimums = {32767, 32767, 32767};
  std::vector<double> coordMaximums = {0, 0, 0};
  for ( size_t i = 0; i < mVertexCoords.size(); i++ )
  {
    double coord = mVertexCoords[i] / 32767.0; // Rescale to 0.0 -- 1.0;
    // Rescale Z to height in meters
    if ( i % 3 == 2 )
    {
      coord = ( coord * ( mHeader.MaximumHeight - mHeader.MinimumHeight ) ) + mHeader.MinimumHeight;
    }
    ( ( float * ) vertexBuffer.data.data() )[i] = ( float )coord;
    if ( coordMinimums[i % 3] > coord )
      coordMinimums[i % 3] = coord;
    if ( coordMaximums[i % 3] < coord )
      coordMaximums[i % 3] = coord;
  }
  model.buffers.push_back( vertexBuffer );

  tinygltf::Buffer triangleBuffer;
  triangleBuffer.data.resize( mTriangleIndices.size() * sizeof( uint32_t ) );
  const char *triData = reinterpret_cast<const char *>( mTriangleIndices.data() );
  std::copy( triData, triData + triangleBuffer.data.size(), triangleBuffer.data.begin() );
  model.buffers.push_back( triangleBuffer );

  tinygltf::BufferView vertexBufferView;
  vertexBufferView.buffer = 0;
  vertexBufferView.byteLength = vertexBuffer.data.size();
  vertexBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
  model.bufferViews.push_back( vertexBufferView );

  tinygltf::BufferView triangleBufferView;
  triangleBufferView.buffer = 1;
  triangleBufferView.byteLength = triangleBuffer.data.size();
  triangleBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
  model.bufferViews.push_back( triangleBufferView );

  tinygltf::Accessor vertexAccessor;
  vertexAccessor.bufferView = 0;
  vertexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
  vertexAccessor.count = mVertexCoords.size() / 3;
  vertexAccessor.type = TINYGLTF_TYPE_VEC3;
  vertexAccessor.minValues = coordMinimums;
  vertexAccessor.maxValues = coordMaximums;
  model.accessors.push_back( vertexAccessor );

  tinygltf::Accessor triangleAccessor;
  triangleAccessor.bufferView = 1;
  triangleAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
  triangleAccessor.count = mTriangleIndices.size();
  triangleAccessor.type = TINYGLTF_TYPE_SCALAR;
  model.accessors.push_back( triangleAccessor );

  tinygltf::Mesh mesh;
  tinygltf::Primitive primitive;
  primitive.attributes["POSITION"] = 0;
  primitive.indices = 1;
  primitive.mode = TINYGLTF_MODE_TRIANGLES;
  mesh.primitives.push_back( primitive );
  model.meshes.push_back( mesh );

  tinygltf::Node node;
  node.mesh = 0;
  model.nodes.push_back( node );

  tinygltf::Scene scene;
  scene.nodes.push_back( 0 );
  model.scenes.push_back( scene );
  model.defaultScene = 0;

  return model;
}
