/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_memory_data_model.hpp"
#include <assert.h>
#include <math.h>
#include <cstring>
#include <algorithm>
#include <iterator>
#include "mdal_utils.hpp"

MDAL::MemoryDataset::MemoryDataset( MDAL::DatasetGroup *grp )
  : Dataset( grp )
  , mValues( group()->isScalar() ? valuesCount() : 2 * valuesCount(),
             std::numeric_limits<double>::quiet_NaN() )
{
  if ( group()->isOnVertices() )
    mActive = std::vector<int>( mesh()->facesCount(), 1 );
}

MDAL::MemoryDataset::~MemoryDataset() = default;

int *MDAL::MemoryDataset::active()
{
  return mActive.data();
}

double *MDAL::MemoryDataset::values()
{
  return mValues.data();
}

const int *MDAL::MemoryDataset::constActive() const
{
  return mActive.data();
}

const double *MDAL::MemoryDataset::constValues() const
{
  return mValues.data();
}

size_t MDAL::MemoryDataset::activeData( size_t indexStart, size_t count, int *buffer )
{
  if ( group()->isOnVertices() )
  {
    size_t nValues = mActive.size();

    if ( ( count < 1 ) || ( indexStart >= nValues ) )
      return 0;

    size_t copyValues = std::min( nValues - indexStart, count );
    memcpy( buffer, constActive() + indexStart, copyValues * sizeof( int ) );
    return copyValues;
  }
  else
  {
    memset( buffer, 1, count * sizeof( int ) );
  }

  return count;
}

size_t MDAL::MemoryDataset::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( group()->isScalar() ); //checked in C API interface
  size_t nValues = valuesCount();
  assert( mValues.size() == nValues );

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, constValues() + indexStart, copyValues * sizeof( double ) );
  return copyValues;
}

size_t MDAL::MemoryDataset::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( !group()->isScalar() ); //checked in C API interface
  size_t nValues = valuesCount();
  assert( mValues.size() == nValues * 2 );

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, constValues() + 2 * indexStart, 2 * copyValues * sizeof( double ) );
  return copyValues;
}

MDAL::MemoryMesh::MemoryMesh( const std::string &driverName,
                              size_t verticesCount,
                              size_t facesCount,
                              size_t faceVerticesMaximumCount,
                              MDAL::BBox extent,
                              const std::string &uri )
  : MDAL::Mesh( driverName,
                verticesCount,
                facesCount,
                faceVerticesMaximumCount,
                extent,
                uri )
{
}

std::unique_ptr<MDAL::MeshVertexIterator> MDAL::MemoryMesh::readVertices()
{
  std::unique_ptr<MDAL::MeshVertexIterator> it( new MemoryMeshVertexIterator( this ) );
  return it;
}

std::unique_ptr<MDAL::MeshFaceIterator> MDAL::MemoryMesh::readFaces()
{
  std::unique_ptr<MDAL::MeshFaceIterator> it( new MemoryMeshFaceIterator( this ) );
  return it;
}

MDAL::MemoryMesh::~MemoryMesh() = default;

MDAL::MemoryMeshVertexIterator::MemoryMeshVertexIterator( const MDAL::MemoryMesh *mesh )
  : mMemoryMesh( mesh )
{

}

MDAL::MemoryMeshVertexIterator::~MemoryMeshVertexIterator() = default;

size_t MDAL::MemoryMeshVertexIterator::next( size_t vertexCount, double *coordinates )
{
  assert( mMemoryMesh );
  assert( coordinates );

  size_t maxVertices = mMemoryMesh->verticesCount();

  if ( vertexCount > maxVertices )
    return 0;

  if ( mLastVertexIndex >= maxVertices )
    return 0;

  size_t i = 0;

  while ( true )
  {
    if ( mLastVertexIndex + i >= maxVertices )
      break;

    if ( i >= vertexCount )
      break;

    const Vertex v = mMemoryMesh->vertices[mLastVertexIndex + i];
    coordinates[3 * i] = v.x;
    coordinates[3 * i + 1] = v.y;
    coordinates[3 * i + 2] = v.z;

    ++i;
  }

  mLastVertexIndex += i;
  return i;
}

MDAL::MemoryMeshFaceIterator::MemoryMeshFaceIterator( const MDAL::MemoryMesh *mesh )
  : mMemoryMesh( mesh )
{
}

MDAL::MemoryMeshFaceIterator::~MemoryMeshFaceIterator() = default;

size_t MDAL::MemoryMeshFaceIterator::next(
  size_t faceOffsetsBufferLen, int *faceOffsetsBuffer,
  size_t vertexIndicesBufferLen, int *vertexIndicesBuffer )
{
  assert( mMemoryMesh );
  assert( faceOffsetsBuffer );
  assert( vertexIndicesBuffer );

  size_t maxFaces = mMemoryMesh->facesCount();
  size_t faceVerticesMaximumCount = mMemoryMesh->faceVerticesMaximumCount();
  size_t vertexIndex = 0;
  size_t faceIndex = 0;

  while ( true )
  {
    if ( vertexIndex + faceVerticesMaximumCount > vertexIndicesBufferLen )
      break;

    if ( faceIndex >= faceOffsetsBufferLen )
      break;

    if ( mLastFaceIndex + faceIndex >= maxFaces )
      break;

    const Face f = mMemoryMesh->faces[mLastFaceIndex + faceIndex];
    for ( size_t faceVertexIndex = 0; faceVertexIndex < f.size(); ++faceVertexIndex )
    {
      assert( vertexIndex < vertexIndicesBufferLen );
      vertexIndicesBuffer[vertexIndex] = static_cast<int>( f[faceVertexIndex] );
      ++vertexIndex;
    }

    assert( faceIndex < faceOffsetsBufferLen );
    faceOffsetsBuffer[faceIndex] = static_cast<int>( vertexIndex );
    ++faceIndex;
  }

  mLastFaceIndex += faceIndex;
  return faceIndex;
}
