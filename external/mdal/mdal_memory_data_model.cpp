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
#include "mdal_logger.hpp"
#include "mdal.h"

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

MDAL::MemoryDataset3D::MemoryDataset3D(
  DatasetGroup *grp,
  size_t volumes,
  size_t maxVerticalLevelCount,
  const int *verticalLevelCounts,
  const double *verticalExtrusions
) : Dataset3D( grp, volumes, maxVerticalLevelCount ),
  mValues( group()->isScalar() ? volumes : 2 * volumes,
           std::numeric_limits<double>::quiet_NaN() ),
  mFaceToVolume( grp->mesh()->facesCount(), 0 ),
  mVerticalLevelCounts( verticalLevelCounts, verticalLevelCounts + grp->mesh()->facesCount() ),
  mVerticalExtrusions( verticalExtrusions, verticalExtrusions + grp->mesh()->facesCount() + volumes )
{
  updateIndices();
}

MDAL::MemoryDataset3D::~MemoryDataset3D() = default;

void MDAL::MemoryDataset3D::updateIndices()
{
  size_t offset = 0;
  for ( size_t i = 0; i < mVerticalLevelCounts.size(); i++ )
  {
    mFaceToVolume[i] = offset;
    offset += mVerticalLevelCounts[i];
    if ( offset > volumesCount() )
    {
      MDAL::Log::error( Err_InvalidData, "Incompatible volume count" );
      return;
    }
  }
}

size_t MDAL::MemoryDataset3D::verticalLevelCountData( size_t indexStart, size_t count, int *buffer )
{
  size_t nValues = group()->mesh()->facesCount();
  assert( mVerticalLevelCounts.size() == nValues );

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, mVerticalLevelCounts.data() + indexStart, copyValues * sizeof( int ) );
  return copyValues;
}

size_t MDAL::MemoryDataset3D::verticalLevelData( size_t indexStart, size_t count, double *buffer )
{
  size_t nValues = group()->mesh()->facesCount() + valuesCount();
  assert( mVerticalExtrusions.size() == nValues );

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, mVerticalExtrusions.data() + indexStart, copyValues * sizeof( double ) );
  return copyValues;
}

size_t MDAL::MemoryDataset3D::faceToVolumeData( size_t indexStart, size_t count, int *buffer )
{
  size_t nValues = group()->mesh()->facesCount();
  assert( mFaceToVolume.size() == nValues );

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, mFaceToVolume.data() + indexStart, copyValues * sizeof( int ) );
  return copyValues;
}

size_t MDAL::MemoryDataset3D::scalarVolumesData( size_t indexStart, size_t count, double *buffer )
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

size_t MDAL::MemoryDataset3D::vectorVolumesData( size_t indexStart, size_t count, double *buffer )
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

void MDAL::MemoryMesh::addVertices( size_t vertexCount, double *coordinates )
{
  size_t coordinateIndex = 0;
  size_t vertexIndex = mVertices.size();
  size_t totalVertexCount = vertexIndex + vertexCount;
  mVertices.resize( totalVertexCount );
  for ( ; vertexIndex < totalVertexCount; ++vertexIndex )
  {
    Vertex vertex;
    vertex.x = coordinates[coordinateIndex];
    ++coordinateIndex;
    vertex.y = coordinates[coordinateIndex];
    ++coordinateIndex;
    vertex.z = coordinates[coordinateIndex];
    ++coordinateIndex;

    mVertices[vertexIndex] = std::move( vertex );
  }

  mExtent = computeExtent( mVertices );
}

void MDAL::MemoryMesh::addFaces( size_t faceCount, size_t driverMaxVerticesPerFace, int *faceSizes, int *vertexIndices )
{
  size_t indicesIndex = 0;
  Faces newFaces( faceCount );
  for ( size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex )
  {
    size_t faceSize = faceSizes[faceIndex];
    if ( faceSize > driverMaxVerticesPerFace )
    {
      MDAL::Log::error( Err_InvalidData, "Incompatible faces count" );
      return;
    }

    if ( faceSize > faceVerticesMaximumCount() )
      setFaceVerticesMaximumCount( faceSize );

    Face face( faceSize );
    for ( size_t i = 0; i < faceSize; ++i )
    {
      const int indice = vertexIndices[indicesIndex + i];
      if ( indice < 0 )
      {
        MDAL::Log::error( Err_InvalidData, "Invalid vertex index when adding faces" );
        return;
      }
      size_t indiceU = static_cast< size_t >( indice );
      if ( indiceU < mVertices.size() )
        face[i] = indiceU;
      else
      {
        MDAL::Log::error( Err_InvalidData, "Invalid vertex index when adding faces" );
        return;
      }
    }
    indicesIndex = indicesIndex + faceSize;
    newFaces[faceIndex] = std::move( face );
  }

  // if everything is ok
  std::move( newFaces.begin(), newFaces.end(), std::back_inserter( mFaces ) );
}

void MDAL::MemoryMesh::addEdges( size_t edgeCount, int *startVertexIndices, int *endVertexIndices )
{
  int maxVertex = mVertices.size();
  for ( size_t edgeIndex = 0 ; edgeIndex < edgeCount; ++edgeIndex )
  {
    Edge edge;
    if ( startVertexIndices[edgeIndex] >= maxVertex || endVertexIndices[edgeIndex] >= maxVertex )
    {
      MDAL::Log::error( Err_InvalidData, "Invalid vertex index when adding edges" );
      return;
    }
    edge.startVertex = startVertexIndices[edgeIndex];
    edge.endVertex = endVertexIndices[edgeIndex];

    mEdges.push_back( std::move( edge ) );
  }
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
    startVertexIndices[i] = MDAL::toInt( e.startVertex );
    endVertexIndices[i] = MDAL::toInt( e.endVertex );

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
