/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_memory_data_model.hpp"
#include <assert.h>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <iterator>
#include "mdal_utils.hpp"

MDAL::MemoryDataset2D::MemoryDataset2D( MDAL::DatasetGroup *grp, bool hasActiveFlag )
  : Dataset2D( grp )
  , mValues( group()->isScalar() ? valuesCount() : 2 * valuesCount(),
             std::numeric_limits<double>::quiet_NaN() )
{
  setSupportsActiveFlag( hasActiveFlag );
  if ( hasActiveFlag )
  {
    assert( grp->dataLocation() == MDAL_DataLocation::DataOnVertices );
    mActive = std::vector<int>( mesh()->facesCount(), 1 );
  }
}

MDAL::MemoryDataset2D::~MemoryDataset2D() = default;

size_t MDAL::MemoryDataset2D::activeData( size_t indexStart, size_t count, int *buffer )
{
  assert( supportsActiveFlag() );
  size_t nValues = mActive.size();

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, mActive.data() + indexStart, copyValues * sizeof( int ) );
  return copyValues;
}

void MDAL::MemoryDataset2D::activateFaces( MDAL::MemoryMesh *mesh )
{
  assert( mesh );
  assert( supportsActiveFlag() );
  assert( group()->dataLocation() == MDAL_DataLocation::DataOnVertices );

  bool isScalar = group()->isScalar();

  // Activate only Faces that do all Vertex's outputs with some data
  const size_t nFaces = mesh->facesCount();

  const Faces &faces = mesh->faces();
  for ( unsigned int idx = 0; idx < nFaces; ++idx )
  {
    const Face &elem = faces.at( idx );
    const std::size_t elemSize = elem.size();
    for ( size_t i = 0; i < elemSize; ++i )
    {
      const size_t vertexIndex = elem[i];
      if ( isScalar )
      {
        const double val = mValues[vertexIndex];
        if ( std::isnan( val ) )
        {
          mActive[idx] = 0; //NOT ACTIVE
          break;
        }
      }
      else
      {
        const double x = mValues[2 * vertexIndex];
        const double y = mValues[2 * vertexIndex + 1];
        if ( std::isnan( x ) || std::isnan( y ) )
        {
          mActive[idx] = 0; //NOT ACTIVE
          break;
        }
      }
    }
  }
}

void MDAL::MemoryDataset2D::setActive( const int *activeBuffer )
{
  assert( supportsActiveFlag() );
  memcpy( mActive.data(), activeBuffer, sizeof( int ) * mesh()->facesCount() );
}

size_t MDAL::MemoryDataset2D::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( group()->isScalar() ); //checked in C API interface
  size_t nValues = valuesCount();
  assert( mValues.size() == nValues );

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, mValues.data() + indexStart, copyValues * sizeof( double ) );
  return copyValues;
}

size_t MDAL::MemoryDataset2D::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( !group()->isScalar() ); //checked in C API interface
  size_t nValues = valuesCount();
  assert( mValues.size() == nValues * 2 );

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, mValues.data() + 2 * indexStart, 2 * copyValues * sizeof( double ) );
  return copyValues;
}

MDAL::MemoryMesh::MemoryMesh( const std::string &driverName,
                              size_t faceVerticesMaximumCount,
                              const std::string &uri )
  : MDAL::Mesh( driverName,
                faceVerticesMaximumCount,
                uri )
{}

std::unique_ptr<MDAL::MeshVertexIterator> MDAL::MemoryMesh::readVertices()
{
  std::unique_ptr<MDAL::MeshVertexIterator> it( new MemoryMeshVertexIterator( this ) );
  return it;
}

std::unique_ptr<MDAL::MeshEdgeIterator> MDAL::MemoryMesh::readEdges()
{
  std::unique_ptr<MDAL::MeshEdgeIterator> it( new MemoryMeshEdgeIterator( this ) );
  return it;
}

std::unique_ptr<MDAL::MeshFaceIterator> MDAL::MemoryMesh::readFaces()
{
  std::unique_ptr<MDAL::MeshFaceIterator> it( new MemoryMeshFaceIterator( this ) );
  return it;
}

void MDAL::MemoryMesh::setVertices( Vertices vertices )
{
  mExtent = MDAL::computeExtent( vertices );
  mVertices = std::move( vertices );
}

void MDAL::MemoryMesh::setFaces( MDAL::Faces faces )
{
  mFaces = std::move( faces );
}

void MDAL::MemoryMesh::setEdges( MDAL::Edges edges )
{
  mEdges = std::move( edges );
}

MDAL::BBox MDAL::MemoryMesh::extent() const
{
  return mExtent;
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
    vertexCount = maxVertices;

  if ( mLastVertexIndex >= maxVertices )
    return 0;

  size_t i = 0;
  const Vertices &vertices = mMemoryMesh->vertices();

  while ( true )
  {
    if ( mLastVertexIndex + i >= maxVertices )
      break;

    if ( i >= vertexCount )
      break;

    const Vertex &v = vertices[mLastVertexIndex + i];
    coordinates[3 * i] = v.x;
    coordinates[3 * i + 1] = v.y;
    coordinates[3 * i + 2] = v.z;

    ++i;
  }

  mLastVertexIndex += i;
  return i;
}

MDAL::MemoryMeshEdgeIterator::MemoryMeshEdgeIterator( const MDAL::MemoryMesh *mesh )
  : mMemoryMesh( mesh )
{
}

MDAL::MemoryMeshEdgeIterator::~MemoryMeshEdgeIterator() = default;

size_t MDAL::MemoryMeshEdgeIterator::next( size_t edgeCount,
    int *startVertexIndices,
    int *endVertexIndices )
{
  assert( mMemoryMesh );
  assert( startVertexIndices );
  assert( endVertexIndices );

  size_t maxEdges = mMemoryMesh->edgesCount();
  const Edges &edges = mMemoryMesh->edges();

  if ( edgeCount > maxEdges )
    edgeCount = maxEdges;

  if ( mLastEdgeIndex >= maxEdges )
    return 0;

  size_t i = 0;

  while ( true )
  {
    if ( mLastEdgeIndex + i >= maxEdges )
      break;

    if ( i >= edgeCount )
      break;

    const Edge &e = edges[mLastEdgeIndex + i];
    startVertexIndices[i] = e.startVertex;
    endVertexIndices[i] = e.endVertex;

    ++i;
  }

  mLastEdgeIndex += i;
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
  const Faces &faces = mMemoryMesh->faces();

  while ( true )
  {
    if ( vertexIndex + faceVerticesMaximumCount > vertexIndicesBufferLen )
      break;

    if ( faceIndex >= faceOffsetsBufferLen )
      break;

    if ( mLastFaceIndex + faceIndex >= maxFaces )
      break;

    const Face &f = faces[mLastFaceIndex + faceIndex];
    const std::size_t faceSize = f.size();
    for ( size_t faceVertexIndex = 0; faceVertexIndex < faceSize; ++faceVertexIndex )
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
