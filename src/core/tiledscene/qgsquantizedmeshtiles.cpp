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
#include <qdebug.h>
#include <qglobal.h>
#include <qstringliteral.h>
#include <qvector3d.h>

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

constexpr uint8_t normalsExtensionId = 1;

// Algorithm from http://jcgt.org/published/0003/02/01/
static QVector3D oct16Decode( uint8_t x, uint8_t y )
{
  if ( x == 0 && y == 0 )
    return QVector3D( 0, 0, 0 );
  float fx = x / 255.0f * 2.0f - 1.0f;
  float fy = y / 255.0f * 2.0f - 1.0f;
  QVector3D decoded( fx, fy, 1.0f - abs( fx ) - abs( fy ) );
  if ( decoded.z() < 0 )
  {
    decoded.setX( ( 1.0f - abs( fy ) ) * ( fx >= 0 ? 1.0f : -1.0f ) );
    decoded.setY( ( 1.0f - abs( fx ) ) * ( fy >= 0 ? 1.0f : -1.0f ) );
  }
  decoded.normalize();
  return decoded;
}

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

    if ( extensionId == normalsExtensionId )
    {
      mNormalCoords.reserve( mHeader.vertexCount * 3 );
      for ( size_t i = 0; i < mHeader.vertexCount; i++ )
      {
        auto normal = oct16Decode(
                        *reinterpret_cast<const uint8_t *>( stream.read( sizeof( char ) ) ),
                        *reinterpret_cast<const uint8_t *>( stream.read( sizeof( char ) ) ) );
        mNormalCoords.insert( mNormalCoords.end(), {normal.x(), normal.y(), normal.z()} );
      }
      continue;
    }

    std::vector<char> data( length );
    const char *dataPtr = reinterpret_cast<const char *>( stream.read( length ) );
    std::copy( dataPtr, dataPtr + length, data.begin() );
    mExtensions[extensionId] = std::move( data );
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

void QgsQuantizedMeshTile::generateNormals()
{
  auto vertexAsVector = [ this ]( size_t idx )
  {
    Q_ASSERT( idx * 3 + 2 < mVertexCoords.size() );
    return QVector3D( mVertexCoords[idx * 3], mVertexCoords[idx * 3 + 1], mVertexCoords[idx * 3 + 2] );
  };

  Q_ASSERT( mNormalCoords.size() == 0 );
  mNormalCoords.resize( mVertexCoords.size(), 0.0 );

  // Sum up contributing normals from all triangles
  for ( size_t i = 0; i < mTriangleIndices.size(); i += 3 )
  {
    std::array<size_t, 3> vtxIdxs {mTriangleIndices[i], mTriangleIndices[i + 1], mTriangleIndices[i + 2]};
    auto a = vertexAsVector( vtxIdxs[0] );
    auto b = vertexAsVector( vtxIdxs[1] );
    auto c = vertexAsVector( vtxIdxs[2] );
    auto n = QVector3D::crossProduct( b - a, c - a );
    n.normalize();
    for ( auto vtx : vtxIdxs )
    {
      mNormalCoords[vtx * 3] += n.x();
      mNormalCoords[vtx * 3 + 1] += n.y();
      mNormalCoords[vtx * 3 + 2] += n.z();
    }
  }

  // Normalize (average over triangles)
  for ( size_t i = 0; i < mNormalCoords.size(); i += 3 )
  {
    QVector3D n( mNormalCoords[i], mNormalCoords[i + 1], mNormalCoords[i + 2] );
    n.normalize();
    mNormalCoords[i] = n.x();
    mNormalCoords[i + 1] = n.y();
    mNormalCoords[i + 2] = n.z();
  }
}

tinygltf::Model QgsQuantizedMeshTile::toGltf( bool addSkirt, double skirtDepth, bool withTextureCoords )
{
  tinygltf::Model model;

  tinygltf::Buffer vertexBuffer;
  vertexBuffer.data.resize( mVertexCoords.size() * sizeof( float ) );
  std::vector<double> coordMinimums = {32767, 32767, mHeader.MaximumHeight};
  std::vector<double> coordMaximums = {0, 0, mHeader.MinimumHeight};

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
  tinygltf::Buffer triangleBuffer;
  triangleBuffer.data.resize( mTriangleIndices.size() * sizeof( uint32_t ) );
  const char *triData = reinterpret_cast<const char *>( mTriangleIndices.data() );
  std::copy( triData, triData + triangleBuffer.data.size(), triangleBuffer.data.begin() );

  if ( addSkirt )
  {
    // We first need to sort the edge-indices by coordinate to later create a quad for each "gap"
    std::sort( mWestVertices.begin(), mWestVertices.end(), [&]( uint32_t a, uint32_t b )
    {
      return mVertexCoords[a * 3 + 1] < mVertexCoords[b * 3 + 1];
    } );
    std::sort( mSouthVertices.begin(), mSouthVertices.end(), [&]( uint32_t a, uint32_t b )
    {
      return mVertexCoords[a * 3] > mVertexCoords[b * 3];
    } );
    std::sort( mEastVertices.begin(), mEastVertices.end(), [&]( uint32_t a, uint32_t b )
    {
      return mVertexCoords[a * 3 + 1] > mVertexCoords[b * 3 + 1];
    } );
    std::sort( mNorthVertices.begin(), mNorthVertices.end(), [&]( uint32_t a, uint32_t b )
    {
      return mVertexCoords[a * 3] < mVertexCoords[b * 3];
    } );

    size_t edgeVertexCount = mWestVertices.size() + mSouthVertices.size() + mEastVertices.size() + mNorthVertices.size();
    size_t skirtBottomCoordCount =
      ( ( mWestVertices.size() > 1 ) +
        ( mSouthVertices.size() > 1 ) +
        ( mEastVertices.size() > 1 ) +
        ( mNorthVertices.size() > 1 ) ) * 6;
    // Add new vertex for each existing edge vertex, projected to Z = minHeight
    coordMinimums[2] = mHeader.MinimumHeight - skirtDepth;
    size_t skirtVerticesIdxStart = mVertexCoords.size() / 3;
    vertexBuffer.data.resize( vertexBuffer.data.size() + ( edgeVertexCount * 3 + skirtBottomCoordCount ) * sizeof( float ) );
    float *skirtVertexCoords = ( float * )( vertexBuffer.data.data() + ( skirtVerticesIdxStart * 3 * sizeof( float ) ) );
    auto addSkirtVertices = [&]( const std::vector<uint32_t> &idxs )
    {
      size_t startIdx = ( ( uint8_t * ) skirtVertexCoords - vertexBuffer.data.data() ) / sizeof( float ) / 3;
      for ( uint32_t idx : idxs )
      {
        *skirtVertexCoords++ = mVertexCoords[idx * 3] / 32767.0f;
        *skirtVertexCoords++ = mVertexCoords[idx * 3 + 1] / 32767.0f;
        *skirtVertexCoords++ = mHeader.MinimumHeight;
      }

      if ( idxs.size() > 1 )
      {
        // Add two vertices at two corners, skirtDepth below the tile bottom
        *skirtVertexCoords++ = mVertexCoords[idxs[0] * 3] / 32767.0f;
        *skirtVertexCoords++ = mVertexCoords[idxs[0] * 3 + 1] / 32767.0f;
        *skirtVertexCoords++ = mHeader.MinimumHeight - skirtDepth;

        *skirtVertexCoords++ = mVertexCoords[idxs[idxs.size() - 1] * 3] / 32767.0f;
        *skirtVertexCoords++ = mVertexCoords[idxs[idxs.size() - 1] * 3 + 1] / 32767.0f;
        *skirtVertexCoords++ = mHeader.MinimumHeight - skirtDepth;
      }

      return startIdx;
    };
    size_t westBottomVerticesIdx = addSkirtVertices( mWestVertices );
    size_t southBottomVerticesIdx = addSkirtVertices( mSouthVertices );
    size_t eastBottomVerticesIdx = addSkirtVertices( mEastVertices );
    size_t northBottomVerticesIdx = addSkirtVertices( mNorthVertices );
    // Check that we didn't miscalculate buffer size
    Q_ASSERT( skirtVertexCoords == ( float * )( vertexBuffer.data.data() + vertexBuffer.data.size() ) );

    // Add skirt triangles (a trapezoid for each pair of edge vertices)
    size_t skirtTrianglesStartIdx = triangleBuffer.data.size();
    size_t edgeQuadCount =
      // For 0/1 point we have 0 quads, for N we have N-1, and an additional one for skirtDepth
      ( mWestVertices.size() > 1 ? mWestVertices.size() : 0 ) +
      ( mSouthVertices.size() > 1 ? mSouthVertices.size() : 0 ) +
      ( mEastVertices.size() > 1 ? mEastVertices.size() : 0 ) +
      ( mNorthVertices.size() > 1 ? mNorthVertices.size() : 0 );
    triangleBuffer.data.resize( triangleBuffer.data.size() + edgeQuadCount * 6 * sizeof( uint32_t ) );
    uint32_t *skirtTriangles = ( uint32_t * )( triangleBuffer.data.data() + skirtTrianglesStartIdx );
    auto addSkirtTriangles = [&]( const std::vector<uint32_t> &topIdxs, size_t bottomVertexIdxStart )
    {
      size_t bottomVertexIdx = bottomVertexIdxStart;
      for ( size_t i = 1; i < topIdxs.size(); i++ )
      {
        uint32_t topVertex1 = topIdxs[i - 1];
        uint32_t topVertex2 = topIdxs[i];
        uint32_t bottomVertex1 = bottomVertexIdx;
        uint32_t bottomVertex2 = ++bottomVertexIdx;

        *skirtTriangles++ = bottomVertex1;
        *skirtTriangles++ = topVertex1;
        *skirtTriangles++ = topVertex2;

        *skirtTriangles++ = bottomVertex2;
        *skirtTriangles++ = bottomVertex1;
        *skirtTriangles++ = topVertex2;
      }

      if ( topIdxs.size() > 1 )
      {
        uint32_t topVertex1 = bottomVertexIdxStart;
        uint32_t topVertex2 = bottomVertexIdxStart + topIdxs.size() - 1;
        uint32_t bottomVertex1 = bottomVertexIdxStart + topIdxs.size();
        uint32_t bottomVertex2 = bottomVertexIdxStart + topIdxs.size() + 1;

        *skirtTriangles++ = bottomVertex1;
        *skirtTriangles++ = topVertex1;
        *skirtTriangles++ = topVertex2;

        *skirtTriangles++ = bottomVertex2;
        *skirtTriangles++ = bottomVertex1;
        *skirtTriangles++ = topVertex2;
      }
    };
    addSkirtTriangles( mWestVertices, westBottomVerticesIdx );
    addSkirtTriangles( mSouthVertices, southBottomVerticesIdx );
    addSkirtTriangles( mEastVertices, eastBottomVerticesIdx );
    addSkirtTriangles( mNorthVertices, northBottomVerticesIdx );
    Q_ASSERT( skirtTriangles == ( uint32_t * )( triangleBuffer.data.data() + triangleBuffer.data.size() ) );
  }

  model.buffers.push_back( vertexBuffer );
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
  vertexAccessor.count = vertexBuffer.data.size() / sizeof( float ) / 3;
  vertexAccessor.type = TINYGLTF_TYPE_VEC3;
  vertexAccessor.minValues = coordMinimums;
  vertexAccessor.maxValues = coordMaximums;
  model.accessors.push_back( vertexAccessor );

  tinygltf::Accessor triangleAccessor;
  triangleAccessor.bufferView = 1;
  triangleAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
  triangleAccessor.count = triangleBuffer.data.size() / sizeof( uint32_t );
  triangleAccessor.type = TINYGLTF_TYPE_SCALAR;
  model.accessors.push_back( triangleAccessor );

  tinygltf::Mesh mesh;
  tinygltf::Primitive primitive;
  primitive.attributes["POSITION"] = 0;
  primitive.indices = 1;
  primitive.mode = TINYGLTF_MODE_TRIANGLES;

  if ( mNormalCoords.size() )
  {
    tinygltf::Buffer normalBuffer;
    normalBuffer.data.resize( vertexBuffer.data.size() );
    // Copy explicit normals, leave rest zeroed
    size_t explicitNormalsBytes = mNormalCoords.size() * sizeof( float );
    Q_ASSERT( vertexBuffer.data.size() >= explicitNormalsBytes );
    const char *normData = reinterpret_cast<const char *>( mNormalCoords.data() );
    std::copy( normData, normData + explicitNormalsBytes, normalBuffer.data.begin() );
    memset( normalBuffer.data.data() + explicitNormalsBytes, 0, normalBuffer.data.size() - explicitNormalsBytes );
    model.buffers.push_back( normalBuffer );

    tinygltf::BufferView normalBufferView;
    normalBufferView.buffer = model.buffers.size() - 1;
    normalBufferView.byteLength = normalBuffer.data.size();
    normalBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    model.bufferViews.push_back( normalBufferView );

    std::vector<double> normalMinimums = {1, 1, 1};
    std::vector<double> normalMaximums = {-1, -1, -1};

    for ( size_t i = 0; i < mNormalCoords.size(); i++ )
    {
      float coord = mNormalCoords[i];
      if ( normalMinimums[i % 3] > coord )
        normalMinimums[i % 3] = coord;
      if ( normalMaximums[i % 3] < coord )
        normalMaximums[i % 3] = coord;
    }

    tinygltf::Accessor normalAccessor;
    normalAccessor.bufferView = model.bufferViews.size() - 1;
    normalAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    normalAccessor.count = normalBuffer.data.size() / sizeof( float ) / 3;
    normalAccessor.type = TINYGLTF_TYPE_VEC3;
    normalAccessor.minValues = normalMinimums;
    normalAccessor.maxValues = normalMaximums;
    model.accessors.push_back( normalAccessor );

    primitive.attributes["NORMAL"] = model.accessors.size() - 1;
  }

  if ( withTextureCoords )
  {
    // Create texture coordinates matching X, Y

    tinygltf::Buffer textureCoordBuffer;
    textureCoordBuffer.data.resize( vertexBuffer.data.size() / 3 * 2 );
    std::vector<double> texCoordMinimums = {1.0, 1.0};
    std::vector<double> texCoordMaximums = {0.0, 0.0};
    auto textureCoordFloats = reinterpret_cast<float *>( textureCoordBuffer.data.data() );

    auto addTexCoordForVertex = [&]( size_t vertexIdx )
    {
      double u = mVertexCoords[vertexIdx * 3] / 32767.0;
      // V coord needs to be flipped for terrain for some reason
      double v = 1.0 - ( mVertexCoords[vertexIdx * 3 + 1] / 32767.0 );
      if ( texCoordMinimums[0] > u ) texCoordMinimums[0] = u;
      if ( texCoordMinimums[1] > v ) texCoordMinimums[1] = v;
      if ( texCoordMaximums[0] < u ) texCoordMaximums[0] = u;
      if ( texCoordMaximums[1] < v ) texCoordMaximums[1] = v;
      *textureCoordFloats++ = u;
      *textureCoordFloats++ = v;
    };

    for ( size_t i = 0; i < mVertexCoords.size() / 3; i++ )
    {
      addTexCoordForVertex( i );
    }

    if ( addSkirt )
    {
      // Add UV for generated bottom vertices matching the top edge vertices
      auto addSkirtVertexUVs = [&]( const std::vector<uint32_t> &idxs )
      {
        for ( uint32_t idx : idxs )
        {
          addTexCoordForVertex( idx );
        }

        if ( idxs.size() > 1 )
        {
          // The two bottom corner vertices get UVs too
          addTexCoordForVertex( idxs[0] );
          addTexCoordForVertex( idxs[idxs.size() - 1] );
        }
      };
      addSkirtVertexUVs( mWestVertices );
      addSkirtVertexUVs( mSouthVertices );
      addSkirtVertexUVs( mEastVertices );
      addSkirtVertexUVs( mNorthVertices );
    }

    Q_ASSERT( textureCoordFloats == ( float * )( textureCoordBuffer.data.data() + textureCoordBuffer.data.size() ) );

    model.buffers.push_back( textureCoordBuffer );

    tinygltf::BufferView textureCoordBufferView;
    textureCoordBufferView.buffer = model.buffers.size() - 1;
    //textureCoordBufferView.buffer = vertexBufferView.buffer; // Reuse vertex coords
    textureCoordBufferView.byteLength = textureCoordBuffer.data.size();
    //textureCoordBufferView.byteLength = vertexBuffer.data.size();
    //textureCoordBufferView.byteStride = sizeof(float) * 3;
    textureCoordBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    model.bufferViews.push_back( textureCoordBufferView );

    tinygltf::Accessor textureCoordAccessor;
    textureCoordAccessor.bufferView = model.bufferViews.size() - 1;
    textureCoordAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    textureCoordAccessor.count = vertexAccessor.count;
    textureCoordAccessor.type = TINYGLTF_TYPE_VEC2;
    textureCoordAccessor.minValues = { texCoordMinimums[0], texCoordMinimums[1] };
    textureCoordAccessor.maxValues = { texCoordMaximums[0], texCoordMaximums[1] };
    model.accessors.push_back( textureCoordAccessor );

    primitive.attributes["TEXCOORD_0"] = model.accessors.size() - 1;
  }

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

QgsMesh QgsQuantizedMeshTile::toMesh( QgsRectangle tileBounds )
{
  QgsMesh mesh;

  mesh.vertices.reserve( mVertexCoords.size() / 3 );
  for ( size_t i = 0; i < mVertexCoords.size(); i += 3 )
  {
    // Rescale to X,Y in CRS and Z in meters
    double x = ( mVertexCoords[i] / 32767.0 ) * ( tileBounds.width() ) + tileBounds.xMinimum();
    double y = ( mVertexCoords[i + 1] / 32767.0 ) * ( tileBounds.height() ) + tileBounds.yMinimum();
    double z = ( mVertexCoords[i + 2] / 32767.0 ) * ( mHeader.MaximumHeight - mHeader.MinimumHeight ) + mHeader.MinimumHeight;
    mesh.vertices.push_back( {x, y, z} );
  }

  mesh.faces.reserve( mTriangleIndices.size() / 3 );
  for ( size_t i = 0; i < mTriangleIndices.size(); i += 3 )
  {
    mesh.faces.push_back(
    {
      static_cast<int>( mTriangleIndices[i] ),
      static_cast<int>( mTriangleIndices[i + 1] ),
      static_cast<int>( mTriangleIndices[i + 2] ),
    } );
  }

  return mesh;
}
