/***************************************************************************
  qgstopologicalmesh.cpp - QgsTopologicalMesh

 ---------------------
 begin                : 18.6.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgstopologicalmesh.h"
#include "qgsmesheditor.h"
#include "qgsmessagelog.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"

#include <poly2tri.h>
#include <QSet>
#include <QQueue>

static int vertexPositionInFace( int vertexIndex, const QgsMeshFace &face )
{
  return face.indexOf( vertexIndex );
}

static int vertexPositionInFace( const QgsMesh &mesh, int vertexIndex, int faceIndex )
{
  if ( faceIndex < 0 || faceIndex >= mesh.faceCount() )
    return -1;

  return vertexPositionInFace( vertexIndex, mesh.face( faceIndex ) );
}

static double crossProduct( int centralVertex, int vertex1, int vertex2, const QgsMesh &mesh )
{
  QgsMeshVertex vc = mesh.vertices.at( centralVertex );
  QgsMeshVertex v1 = mesh.vertices.at( vertex1 );
  QgsMeshVertex v2 = mesh.vertices.at( vertex2 );

  double ux1 = v1.x() - vc.x();
  double uy1 = v1.y() - vc.y();
  double vx1 = v2.x() - vc.x();
  double vy1 = v2.y() - vc.y();

  return ux1 * vy1 - uy1 * vx1;
}


QgsMeshVertexCirculator::QgsMeshVertexCirculator( const QgsTopologicalMesh &topologicalMesh, int vertexIndex )
  : mFaces( topologicalMesh.mMesh->faces )
  , mFacesNeighborhood( topologicalMesh.mFacesNeighborhood )
  ,  mVertexIndex( vertexIndex )
{
  if ( vertexIndex >= 0 && vertexIndex < topologicalMesh.mMesh->vertexCount() )
  {
    mCurrentFace = topologicalMesh.mVertexToFace[vertexIndex];
    mIsValid = vertexPositionInFace( *topologicalMesh.mesh(), vertexIndex, mCurrentFace ) != -1;
  }
  else
  {
    mIsValid = false;
  }

  if ( mIsValid )
    mLastValidFace = mCurrentFace;
}

QgsMeshVertexCirculator::QgsMeshVertexCirculator( const QgsTopologicalMesh::TopologicalFaces &topologicalFaces, int faceIndex, int vertexIndex )
  : mFaces( topologicalFaces.mFaces )
  , mFacesNeighborhood( topologicalFaces.mFacesNeighborhood )
  , mVertexIndex( vertexIndex )
{
  const QgsMeshFace &face = topologicalFaces.mFaces.at( faceIndex );
  mIsValid = vertexPositionInFace( vertexIndex, face ) != -1;

  mCurrentFace = faceIndex;
  mLastValidFace = mCurrentFace;
}

QgsMeshVertexCirculator::QgsMeshVertexCirculator( const QgsTopologicalMesh::TopologicalFaces &topologicalFaces, int vertexIndex )
  : mFaces( topologicalFaces.mFaces )
  , mFacesNeighborhood( topologicalFaces.mFacesNeighborhood )
  , mVertexIndex( vertexIndex )
{
  if ( topologicalFaces.mVerticesToFace.contains( vertexIndex ) )
    mCurrentFace = topologicalFaces.mVerticesToFace.values( vertexIndex ).first();
  mLastValidFace = mCurrentFace;
  mIsValid = mCurrentFace != -1;
}

int QgsMeshVertexCirculator::turnCounterClockwise() const
{
  if ( mCurrentFace == -1 )
    mCurrentFace = mLastValidFace;
  else
  {
    int currentPos = positionInCurrentFace();
    Q_ASSERT( currentPos != -1 );

    const QgsMeshFace &currentFace = mFaces.at( mCurrentFace );
    int faceSize = currentFace.size();
    mLastValidFace = mCurrentFace;
    mCurrentFace = mFacesNeighborhood.at( mCurrentFace ).at( ( currentPos + faceSize - 1 ) % currentFace.count() );
  }

  return mCurrentFace;
}

int QgsMeshVertexCirculator::turnClockwise() const
{
  if ( mCurrentFace == -1 )
    mCurrentFace = mLastValidFace;
  else
  {
    int currentPos = positionInCurrentFace();
    Q_ASSERT( currentPos != -1 );

    const QgsMeshFace &currentFace = mFaces.at( mCurrentFace );
    int faceSize = currentFace.size();
    mLastValidFace = mCurrentFace;
    mCurrentFace = mFacesNeighborhood.at( mCurrentFace ).at( ( currentPos ) % faceSize );
  }

  return mCurrentFace;
}

int QgsMeshVertexCirculator::currentFaceIndex() const
{
  return mCurrentFace;
}

QgsMeshFace QgsMeshVertexCirculator::currentFace() const
{
  if ( mCurrentFace != -1 )
    return mFaces.at( mCurrentFace );
  else
    return QgsMeshFace();
}

bool QgsMeshVertexCirculator::goBoundaryClockwise() const
{
  if ( !isValid() )
    return false;

  if ( mCurrentFace == -1 )
    mCurrentFace = mLastValidFace;

  int firstFace = mCurrentFace;

  while ( turnClockwise() != -1 && currentFaceIndex() != firstFace ) {}
  if ( mCurrentFace == firstFace )
    return false; // a complete turn, so the vertex is not a boundary vertex, something went wrong

  return ( turnCounterClockwise() != -1 );
}

bool QgsMeshVertexCirculator::goBoundaryCounterClockwise() const
{
  if ( !isValid() )
    return false;

  if ( mCurrentFace == -1 )
    mCurrentFace = mLastValidFace;

  int firstFace = mCurrentFace;

  while ( turnCounterClockwise() != -1 && currentFaceIndex() != firstFace ) {}
  if ( mCurrentFace == firstFace )
    return false; // a complete turn, so the vertex is not a boundary vertex, something went wrong

  return ( turnClockwise() != -1 );
}

int QgsMeshVertexCirculator::oppositeVertexClockwise() const
{
  if ( mCurrentFace == -1 )
    return -1;

  const QgsMeshFace &face = currentFace();

  if ( face.isEmpty() )
    return -1;

  int vertexPosition = vertexPositionInFace( mVertexIndex, currentFace() );

  if ( vertexPosition == -1 )
    return -1;

  return face.at( ( vertexPosition + 1 ) % face.count() );
}

int QgsMeshVertexCirculator::oppositeVertexCounterClockwise() const
{
  if ( mCurrentFace == -1 )
    return -1;

  const QgsMeshFace &face = currentFace();

  if ( face.isEmpty() )
    return -1;

  int vertexPosition = vertexPositionInFace( mVertexIndex, currentFace() );

  if ( vertexPosition == -1 )
    return -1;

  return face.at( ( vertexPosition - 1 + face.count() ) % face.count() );
}

bool QgsMeshVertexCirculator::isValid() const
{
  return mIsValid;
}

QList<int> QgsMeshVertexCirculator::facesAround() const
{
  QList<int> ret;
  if ( !isValid() )
    return ret;

  if ( mCurrentFace != -1 )
    ret.append( mCurrentFace );
  while ( turnCounterClockwise() != ret.first() && currentFaceIndex() != -1 )
    ret.append( currentFaceIndex() );


  if ( currentFaceIndex() == -1 )  //we encounter a boundary, restart with other direction
  {
    ret.clear();
    if ( turnClockwise() == -1 )
      return ret;
    ret.append( currentFaceIndex() );
    while ( turnClockwise() != -1 )
    {
      ret.append( currentFaceIndex() );
    }
  }

  return ret;
}

int QgsMeshVertexCirculator::degree() const
{
  if ( mDegree != -1 )
    return mDegree;

  mDegree = 0;
  if ( !mIsValid )
    return mDegree;

  // if we are on the boundary, we count one more to take account of the circulator will
  // not cover the last vertex (the other vertex on boundary)
  if ( goBoundaryCounterClockwise() )
    mDegree = 2;
  else
    mDegree = 1;

  int firstFace = currentFaceIndex();

  while ( turnClockwise() != firstFace && currentFaceIndex() != -1 )
    ++mDegree;

  return mDegree;
}


int QgsMeshVertexCirculator::positionInCurrentFace() const
{
  if ( mCurrentFace < 0 || mCurrentFace > mFaces.count() )
    return -1;

  return vertexPositionInFace( mVertexIndex, mFaces.at( mCurrentFace ) );
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::addFaces( const QgsTopologicalMesh::TopologicalFaces &topologicalFaces )
{
  Changes changes;
  changes.mFacesToAdd = topologicalFaces.mFaces;
  changes.mAddedFacesFirstIndex = mMesh->faceCount();

  changes.mFacesNeighborhoodToAdd.resize( changes.mFacesToAdd.count() );
  for ( int i = 0; i < changes.mFacesToAdd.count(); ++i )
    changes.mFacesNeighborhoodToAdd[i] = QVector<int>( changes.mFacesToAdd.at( i ).count(), -1 );

  for ( int boundary : topologicalFaces.mBoundaries )
  {
    //if the boundary is a free vertex in the destination mesh, no need to check
    if ( mVertexToFace.at( boundary ) == -1 )
      continue;

    const QList<int> &linkedFaces = topologicalFaces.mVerticesToFace.values( boundary );
    for ( int linkedFace : linkedFaces )
    {
      QgsMeshVertexCirculator newFacesCirculator( topologicalFaces, linkedFace, boundary );
      //search for face boundary on clockwise side of new faces
      newFacesCirculator.goBoundaryClockwise();
      int oppositeVertexForNewFace = newFacesCirculator.oppositeVertexClockwise();
      if ( mVertexToFace.at( oppositeVertexForNewFace ) == -1 )
        continue;

      QgsMeshVertexCirculator meshCirculator = vertexCirculator( boundary );
      meshCirculator.goBoundaryCounterClockwise();
      int oppositeVertexForMeshFace = meshCirculator.oppositeVertexCounterClockwise();

      const QgsMeshFace &newFaceBoundary = newFacesCirculator.currentFace();
      int boundaryPositionInNewFace = vertexPositionInFace( boundary, newFaceBoundary );

      if ( oppositeVertexForMeshFace != oppositeVertexForNewFace )
      {
        changes.mFacesNeighborhoodToAdd[newFacesCirculator.currentFaceIndex()][boundaryPositionInNewFace] = -1 ;
      }
      else
      {
        const QgsMeshFace &meshFaceBoundary = meshCirculator.currentFace();
        int boundaryPositionInMeshFace = vertexPositionInFace( meshCirculator.oppositeVertexCounterClockwise(), meshFaceBoundary );

        changes.mNeighborhoodChanges.append( std::array<int, 4>(
        {
          meshCirculator.currentFaceIndex(),
          boundaryPositionInMeshFace,
          -1,
          changes.addedFaceIndexInMesh( newFacesCirculator.currentFaceIndex() )
        } ) );

        changes.mFacesNeighborhoodToAdd[newFacesCirculator.currentFaceIndex()][boundaryPositionInNewFace] = meshCirculator.currentFaceIndex();
      }
    }
  }

  for ( int f = 0; f < changes.mFacesToAdd.count(); ++f )
    for ( int n = 0; n < changes.mFacesToAdd.at( f ).count(); ++n )
      if ( changes.mFacesNeighborhoodToAdd.at( f ).at( n ) == -1 )
        changes.mFacesNeighborhoodToAdd[f][n] = changes.addedFaceIndexInMesh( topologicalFaces.mFacesNeighborhood.at( f ).at( n ) );

  const QList<int> &verticesToFaceToChange = topologicalFaces.mVerticesToFace.keys();
  for ( const int vtc : verticesToFaceToChange )
    if ( mVertexToFace.at( vtc ) == -1 )
      changes.mVerticesToFaceChanges.append( {vtc,
                                              mVertexToFace.at( vtc ),
                                              changes.addedFaceIndexInMesh( topologicalFaces.mVerticesToFace.values( vtc ).first() ) } );

  applyChanges( changes );

  return changes;
}

void QgsTopologicalMesh::applyChanges( const QgsTopologicalMesh::Changes &changes )
{
  int initialVerticesCount = mMesh->vertices.count();
  if ( !changes.mVerticesToAdd.empty() )
  {
    int newSize = mMesh->vertices.count() + changes.mVerticesToAdd.count();
    mMesh->vertices.resize( newSize );
    mVertexToFace.resize( newSize );
  }

  if ( !changes.mFacesToAdd.empty() )
  {
    int newSize = mMesh->faceCount() + changes.mFacesToAdd.count();
    mMesh->faces.resize( newSize );
    mFacesNeighborhood.resize( newSize );
  }

  for ( int i = 0; i < changes.mFacesToRemove.count(); ++i )
  {
    mMesh->faces[changes.removedFaceIndexInMesh( i )] = QgsMeshFace();
    mFacesNeighborhood[changes.removedFaceIndexInMesh( i )] = FaceNeighbors();//changes.facesNeighborhoodToRemove[i];
  }

  for ( int i = 0; i < changes.mVerticesToRemoveIndexes.count(); ++i )
  {
    int vertexIndex = changes.mVerticesToRemoveIndexes.at( i );
    if ( mVertexToFace.at( vertexIndex ) == -1 )
      dereferenceAsFreeVertex( vertexIndex );
    mMesh->vertices[vertexIndex] = QgsMeshVertex();
    mVertexToFace[vertexIndex] = -1;
  }

  for ( int i = 0; i < changes.mVerticesToAdd.count(); ++i )
  {
    mMesh->vertices[initialVerticesCount + i] = changes.mVerticesToAdd.at( i );
    mVertexToFace[initialVerticesCount + i] = changes.mVertexToFaceToAdd.at( i );
    if ( changes.mVertexToFaceToAdd.at( i ) == -1 )
      referenceAsFreeVertex( initialVerticesCount + i );
  }

  for ( int i = 0; i < changes.mFacesToAdd.count(); ++i )
  {
    mMesh->faces[changes.addedFaceIndexInMesh( i )] = changes.mFacesToAdd.at( i );
    mFacesNeighborhood[changes.addedFaceIndexInMesh( i )] = changes.mFacesNeighborhoodToAdd.at( i );
  }

  for ( const std::array<int, 4> neigborChange : std::as_const( changes.mNeighborhoodChanges ) )
  {
    const int faceIndex = neigborChange.at( 0 );
    const int positionInFace = neigborChange.at( 1 );
    const int valueToApply = neigborChange.at( 3 );
    mFacesNeighborhood[faceIndex][positionInFace] = valueToApply;
  }

  for ( const std::array<int, 3> vertexToFaceChange : std::as_const( changes.mVerticesToFaceChanges ) )
  {
    int vertexIndex = vertexToFaceChange.at( 0 );
    mVertexToFace[vertexToFaceChange.at( 0 )] = vertexToFaceChange.at( 2 );

    if ( vertexToFaceChange.at( 2 ) == -1 &&
         vertexToFaceChange.at( 1 ) != -1 &&
         !mMesh->vertices.at( vertexIndex ).isEmpty() )
      referenceAsFreeVertex( vertexIndex );

    if ( vertexToFaceChange.at( 1 ) == -1 && vertexToFaceChange.at( 2 ) != -1 )
      dereferenceAsFreeVertex( vertexIndex );
  }

  for ( int i = 0; i < changes.mChangeCoordinateVerticesIndexes.count(); ++i )
  {
    int vertexIndex = changes.mChangeCoordinateVerticesIndexes.at( i );
    if ( !changes.mNewZValues.isEmpty() )
      mMesh->vertices[vertexIndex].setZ( changes.mNewZValues.at( i ) );
    if ( !changes.mNewXYValues.isEmpty() )
    {
      const QgsPointXY &pt = changes.mNewXYValues.at( i );
      mMesh->vertices[vertexIndex].setX( pt.x() );
      mMesh->vertices[vertexIndex].setY( pt.y() );
    }
  }
}

void QgsTopologicalMesh::reverseChanges( const QgsTopologicalMesh::Changes &changes )
{
  for ( const std::array<int, 4> neigborChange : std::as_const( changes.mNeighborhoodChanges ) )
  {
    const int faceIndex = neigborChange.at( 0 );
    const int positionInFace = neigborChange.at( 1 );
    const int valueToApply = neigborChange.at( 2 );
    mFacesNeighborhood[faceIndex][positionInFace] = valueToApply;
  }

  for ( int i = 0; i < changes.mFacesToRemove.count(); ++i )
  {
    mMesh->faces[changes.removedFaceIndexInMesh( i )] = changes.mFacesToRemove.at( i );
    mFacesNeighborhood[changes.removedFaceIndexInMesh( i )] = changes.mFacesNeighborhoodToRemove.at( i );
  }

  for ( int i = 0; i < changes.mVerticesToRemoveIndexes.count(); ++i )
  {
    int vertexIndex = changes.mVerticesToRemoveIndexes.at( i );
    mMesh->vertices[vertexIndex] = changes.mRemovedVertices.at( i );
    mVertexToFace[vertexIndex] = changes.mVerticesToFaceRemoved.at( i );
    if ( mVertexToFace.at( vertexIndex ) == -1 )
      referenceAsFreeVertex( vertexIndex );
  }

  int verticesToFaceChangesCount = changes.mVerticesToFaceChanges.count();
  for ( int i = 0; i < verticesToFaceChangesCount; ++i )
  {
    const std::array<int, 3> vertexToFaceChange = changes.mVerticesToFaceChanges.at( verticesToFaceChangesCount - i - 1 );
    int vertexIndex = vertexToFaceChange.at( 0 );
    mVertexToFace[vertexIndex] = vertexToFaceChange.at( 1 );

    if ( vertexToFaceChange.at( 2 ) == -1 && vertexToFaceChange.at( 1 ) != -1 )
      dereferenceAsFreeVertex( vertexIndex );

    if ( vertexToFaceChange.at( 1 ) == -1 &&
         vertexToFaceChange.at( 2 ) != -1 &&
         !mMesh->vertex( vertexIndex ).isEmpty() )
      referenceAsFreeVertex( vertexIndex );
  }

  if ( !changes.mFacesToAdd.empty() )
  {
    int newSize = mMesh->faceCount() - changes.mFacesToAdd.count();
    mMesh->faces.resize( newSize );
    mFacesNeighborhood.resize( newSize );
  }

  if ( !changes.mVerticesToAdd.isEmpty() )
  {
    int newSize = mMesh->vertexCount() - changes.mVerticesToAdd.count();

    for ( int i = newSize; i < mMesh->vertexCount(); ++i )
      if ( mVertexToFace.at( i ) == -1 )
        dereferenceAsFreeVertex( i );

    mMesh->vertices.resize( newSize );
    mVertexToFace.resize( newSize );
  }

  for ( int i = 0; i < changes.mChangeCoordinateVerticesIndexes.count(); ++i )
  {
    int vertexIndex = changes.mChangeCoordinateVerticesIndexes.at( i );
    if ( !changes.mOldZValues.isEmpty() )
      mMesh->vertices[vertexIndex].setZ( changes.mOldZValues.at( i ) );
    if ( !changes.mOldXYValues.isEmpty() )
    {
      const QgsPointXY &pt = changes.mOldXYValues.at( i );
      mMesh->vertices[vertexIndex].setX( pt.x() );
      mMesh->vertices[vertexIndex].setY( pt.y() );
    }
  }
}

QgsMeshVertexCirculator QgsTopologicalMesh::vertexCirculator( int vertexIndex ) const
{
  return QgsMeshVertexCirculator( *this, vertexIndex );
}

QSet<int> QgsTopologicalMesh::concernedFacesBy( const QList<int> faceIndexes ) const
{
  QSet<int> faces;
  for ( const int faceIndex : faceIndexes )
  {
    const QgsMeshFace &face = mMesh->face( faceIndex );
    for ( int i = 0; i < face.count(); ++i )
      faces.unite( qgis::listToSet( facesAroundVertex( face.at( i ) ) ) );
  }
  return faces;
}

void QgsTopologicalMesh::dereferenceAsFreeVertex( int vertexIndex )
{
  mFreeVertices.remove( vertexIndex );
}

void QgsTopologicalMesh::referenceAsFreeVertex( int vertexIndex )
{
  // QSet used to retrieve free vertex without going through all the vertices container.
  // But maybe later we could use more sophisticated reference (spatial index?), to retrieve free vertex in an extent
  mFreeVertices.insert( vertexIndex );
}

QgsMeshEditingError QgsTopologicalMesh::checkConsistency() const
{
  for ( int faceIndex = 0 ; faceIndex < mMesh->faces.count( ); ++faceIndex )
  {
    const QgsMeshFace &face = mMesh->faces.at( faceIndex );
    const FaceNeighbors &neighborhood = mFacesNeighborhood.at( faceIndex );
    if ( face.count() != neighborhood.count() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, faceIndex );
    for ( int i = 0; i < face.count(); ++i )
    {
      int vertexIndex = face.at( i );
      // check if each vertices is linked to a face (not free vertex)
      if ( mVertexToFace.at( vertexIndex ) == -1 )
        return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, vertexIndex );

      int neighborIndex = neighborhood.at( i );
      if ( neighborIndex != -1 )
      {
        const QgsMeshFace &neighborFace = mMesh->faces.at( neighborIndex );
        if ( neighborFace.isEmpty() )
          return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, faceIndex );
        int neighborSize = neighborFace.size();
        const FaceNeighbors &neighborhoodOfNeighbor = mFacesNeighborhood.at( neighborIndex );
        int posInNeighbor = vertexPositionInFace( *mMesh, vertexIndex, neighborIndex );
        if ( neighborhoodOfNeighbor.isEmpty() || neighborhoodOfNeighbor.at( ( posInNeighbor + neighborSize - 1 ) % neighborSize ) != faceIndex )
          return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, faceIndex );
      }
    }
  }

  for ( int vertexIndex = 0; vertexIndex < mMesh->vertexCount(); ++vertexIndex )
  {
    if ( mVertexToFace.at( vertexIndex ) != -1 )
    {
      if ( !mMesh->face( mVertexToFace.at( vertexIndex ) ).contains( vertexIndex ) )
        return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, vertexIndex );

      if ( facesAroundVertex( vertexIndex ).count() == 0 )
        return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, vertexIndex );
    }
  }

  return QgsMeshEditingError();
}

QgsMeshEditingError QgsTopologicalMesh::checkTopology( const QgsMesh &mesh, int maxVerticesPerFace )
{
  QgsMesh temp = mesh;
  QgsMeshEditingError error;
  createTopologicalMesh( &temp, maxVerticesPerFace, error );
  return error;
}

QgsMesh *QgsTopologicalMesh::mesh() const
{
  return mMesh;
}

int QgsTopologicalMesh::firstFaceLinked( int vertexIndex ) const
{
  if ( vertexIndex < 0 || vertexIndex >= mMesh->vertexCount() )
    return -1;
  return mVertexToFace.at( vertexIndex );
}

bool QgsTopologicalMesh::isVertexOnBoundary( int vertexIndex ) const
{
  QgsMeshVertexCirculator circulator = vertexCirculator( vertexIndex );

  if ( circulator.isValid() )
    return circulator.goBoundaryClockwise();

  return false;
}

bool QgsTopologicalMesh::isVertexFree( int vertexIndex ) const
{
  if ( vertexIndex < 0 || vertexIndex >= mMesh->vertexCount() )
    return false;

  if ( mMesh->vertices.at( vertexIndex ).isEmpty() )
    return false;

  return mVertexToFace.at( vertexIndex ) == -1;
}

QList<int> QgsTopologicalMesh::freeVerticesIndexes() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  return mFreeVertices.values();
#else
  return QList<int>( mFreeVertices.begin(), mFreeVertices.end() );
#endif
}

QgsMeshEditingError QgsTopologicalMesh::counterClockwiseFaces( QgsMeshFace &face, QgsMesh *mesh )
{
  // First check if the face is convex and put it counter clockwise
  // If the index are not well ordered (edges intersect), invalid face --> return false
  int faceSize = face.count();
  if ( faceSize < 3 )
    return QgsMeshEditingError( Qgis::MeshEditingErrorType::FlatFace, -1 );

  int direction = 0;
  for ( int i = 0; i < faceSize; ++i )
  {
    int iv0 =  face[i];
    int iv1 = face[( i + 1 ) % faceSize];
    int iv2 = face[( i + 2 ) % faceSize];

    if ( iv0 < 0 || iv0 >= mesh->vertexCount() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, iv0 );

    if ( iv1 < 0 || iv1 >= mesh->vertexCount() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, iv1 );

    if ( iv2 < 0 || iv2 >= mesh->vertexCount() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, iv2 );

    const QgsMeshVertex &v0 = mesh->vertices.at( iv0 ) ;
    const QgsMeshVertex &v1 = mesh->vertices.at( iv1 ) ;
    const QgsMeshVertex &v2 = mesh->vertices.at( iv2 ) ;

    if ( v0.isEmpty() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, iv0 );

    if ( v1.isEmpty() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, iv1 );

    if ( v2.isEmpty() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, iv2 );

    double crossProd = crossProduct( iv1, iv0, iv2, *mesh ); //if cross product>0, we have two edges clockwise
    if ( direction != 0 && crossProd * direction < 0 )   // We have a convex face or a (partially) flat face
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, -1 );
    else if ( crossProd == 0 )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::FlatFace, -1 );
    else if ( direction == 0 && crossProd != 0 )
      direction = crossProd / std::fabs( crossProd );
  }

  if ( direction > 0 )// clockwise --> reverse the order of the index;
  {
    for ( int i = 0; i < faceSize / 2; ++i )
    {
      int temp = face[i];
      face[i] = face.at( faceSize - i - 1 );
      face[faceSize - i - 1] = temp;
    }
  }

  return QgsMeshEditingError( Qgis::MeshEditingErrorType::NoError, -1 );
}

void QgsTopologicalMesh::reindex()
{
  QVector<int> oldToNewIndex( mMesh->vertices.count(), -1 );
  int verticesTotalCount = mMesh->vertexCount();
  int oldIndex = 0;
  int newIndex = 0;
  while ( oldIndex < verticesTotalCount )
  {
    if ( mMesh->vertex( oldIndex ).isEmpty() )
    {
      oldIndex++;
    }
    else
    {
      oldToNewIndex[oldIndex] = newIndex;
      if ( oldIndex != newIndex )
        mMesh->vertices[newIndex] = mMesh->vertices[oldIndex];
      oldToNewIndex[oldIndex] = newIndex;
      oldIndex++;
      newIndex++;
    }
  }

  mMesh->vertices.resize( newIndex );

  oldIndex = 0;
  newIndex = 0;
  int facesTotalCount = mMesh->faceCount();
  while ( oldIndex < facesTotalCount )
  {
    if ( mMesh->face( oldIndex ).isEmpty() )
      oldIndex++;
    else
    {
      if ( oldIndex != newIndex )
        mMesh->faces[newIndex] = mMesh->faces[oldIndex];
      QgsMeshFace &face = mMesh->faces[newIndex];
      for ( int i = 0; i < face.count(); ++i )
        face[i] = oldToNewIndex[face.at( i )];
      newIndex++;
      oldIndex++;
    }
  }

  mMesh->faces.resize( newIndex );

  mVertexToFace.clear();
  mFacesNeighborhood.clear();
}

bool QgsTopologicalMesh::renumber()
{
  QVector<int> oldToNewVerticesIndexes;
  if ( !renumberVertices( oldToNewVerticesIndexes ) )
    return false;


  QVector<int> oldToNewFacesIndexes;

  if ( !renumberFaces( oldToNewFacesIndexes ) )
    return false;

  // If we are here, we can apply the renumbering

  QVector<QgsMeshVertex> tempVertices( mMesh->vertices.count() );
  for ( int i = 0; i < oldToNewVerticesIndexes.count(); ++i )
  {
    tempVertices[oldToNewVerticesIndexes.at( i )] = mMesh->vertex( i );
  }
  mMesh->vertices = tempVertices;

  QVector<QgsMeshFace> tempFaces( mMesh->faces.count() );
  for ( int i = 0; i < oldToNewFacesIndexes.count(); ++i )
  {
    tempFaces[oldToNewFacesIndexes.at( i )] = mMesh->face( i );

    QgsMeshFace &face = tempFaces[oldToNewFacesIndexes.at( i )];

    for ( int fi = 0; fi < face.count(); ++fi )
    {
      face[fi] = oldToNewVerticesIndexes.at( face.at( fi ) );
    }
  }

  mMesh->faces = tempFaces;

  return true;

}

bool QgsTopologicalMesh::renumberVertices( QVector<int> &oldToNewIndex ) const
{
  std::vector<QgsMeshVertexCirculator> circulators;
  circulators.reserve( mMesh->vertices.count() );
  int minDegree = std::numeric_limits<int>::max();
  int minDegreeVertex = -1;

  QQueue<int> queue;
  QSet<int> nonThreadedVertex;
  oldToNewIndex = QVector<int> ( mMesh->vertexCount(), -1 );
  for ( int i = 0; i < mMesh->vertexCount(); ++i )
  {
    circulators.emplace_back( *this, i );
    const QgsMeshVertexCirculator &circulator = circulators.back();
    if ( circulators.back().degree() < minDegree )
    {
      minDegreeVertex = circulators.size() - 1;
      minDegree = circulator.degree();
    }
    nonThreadedVertex.insert( i );
  }

  auto sortedNeighbor = [ = ]( QList<int> &neighbors, int index )
  {
    const QgsMeshVertexCirculator &circ = circulators.at( index );

    if ( !circ.isValid() )
      return;

    circ.goBoundaryCounterClockwise();
    neighbors.append( circ.oppositeVertexCounterClockwise() );

    int firsrFace = circ.currentFaceIndex();
    do
    {
      int neighborIndex = circ.oppositeVertexClockwise();
      int degree = circulators.at( neighborIndex ).degree();
      QList<int>::Iterator it = neighbors.begin();
      while ( it != neighbors.end() )
      {
        if ( degree <= circulators.at( *it ).degree() )
        {
          neighbors.insert( it, neighborIndex );
          break;
        }
        it++;
      }
      if ( it == neighbors.end() )
        neighbors.append( neighborIndex );
    }
    while ( circ.turnClockwise() != firsrFace && circ.currentFaceIndex() != -1 );
  };

  int newIndex = 0;
  int currentVertex = minDegreeVertex;
  nonThreadedVertex.remove( minDegreeVertex );

  while ( newIndex < mMesh->vertexCount() )
  {
    if ( oldToNewIndex[currentVertex] == -1 )
      oldToNewIndex[currentVertex] = newIndex++;

    if ( circulators.at( currentVertex ).isValid() )
    {
      QList<int> neighbors;
      sortedNeighbor( neighbors, currentVertex );

      for ( const int i : std::as_const( neighbors ) )
        if ( oldToNewIndex.at( i ) == -1 && nonThreadedVertex.contains( i ) )
        {
          queue.enqueue( i );
          nonThreadedVertex.remove( i );
        }
    }

    if ( queue.isEmpty() )
    {
      if ( nonThreadedVertex.isEmpty() && newIndex < mMesh->vertexCount() )
        return false;

      const QList<int> remainingVertex = qgis::setToList( nonThreadedVertex );
      int minRemainingDegree = std::numeric_limits<int>::max();
      int minRemainingVertex = -1;
      for ( const int i : remainingVertex )
      {
        int degree = circulators.at( i ).degree();
        if ( degree < minRemainingDegree )
        {
          minRemainingDegree = degree;
          minRemainingVertex = i;
        }
      }
      currentVertex = minRemainingVertex;
      nonThreadedVertex.remove( currentVertex );
    }
    else
    {
      currentVertex = queue.dequeue();
    }
  }

  return true;
}

bool QgsTopologicalMesh::renumberFaces( QVector<int> &oldToNewIndex ) const
{
  QQueue<int> queue;
  QSet<int> nonThreadedFaces;

  oldToNewIndex = QVector<int>( mMesh->faceCount(), -1 );

  QVector<int> faceDegrees( mMesh->faceCount(), 0 );

  int minDegree = std::numeric_limits<int>::max();
  int minDegreeFace = -1;
  for ( int faceIndex = 0; faceIndex < mMesh->faceCount(); ++faceIndex )
  {
    const FaceNeighbors &neighbors = mFacesNeighborhood.at( faceIndex );

    int degree = 0;
    for ( int n = 0; n < neighbors.size(); ++n )
    {
      if ( neighbors.at( n ) != -1 )
        degree++;
    }

    if ( degree < minDegree )
    {
      minDegree = degree;
      minDegreeFace = faceIndex;
    }

    faceDegrees[faceIndex] = degree;
    nonThreadedFaces.insert( faceIndex );
  }

  int newIndex = 0;
  int currentFace = minDegreeFace;
  nonThreadedFaces.remove( minDegreeFace );

  auto sortedNeighbor = [ = ]( QList<int> &neighbors, int index )
  {
    const FaceNeighbors &neighborhood = mFacesNeighborhood.at( index );

    for ( int i = 0; i < neighborhood.count(); ++i )
    {
      int neighborIndex = neighborhood.at( i );
      if ( neighborIndex == -1 )
        continue;

      int degree = faceDegrees.at( neighborIndex );
      if ( neighbors.isEmpty() )
        neighbors.append( neighborIndex );
      else
      {
        QList<int>::Iterator it = neighbors.begin();
        while ( it != neighbors.end() )
        {
          if ( degree <= faceDegrees.at( *it ) )
          {
            neighbors.insert( it, neighborIndex );
            break;
          }
          it++;
        }
        if ( it == neighbors.end() )
          neighbors.append( neighborIndex );
      }
    }
  };

  while ( newIndex < mMesh->faceCount() )
  {
    if ( oldToNewIndex[currentFace] == -1 )
      oldToNewIndex[currentFace] = newIndex++;

    QList<int> neighbors;
    sortedNeighbor( neighbors, currentFace );

    for ( const int i : std::as_const( neighbors ) )
      if ( oldToNewIndex.at( i ) == -1 && nonThreadedFaces.contains( i ) )
      {
        queue.enqueue( i );
        nonThreadedFaces.remove( i );
      }

    if ( queue.isEmpty() )
    {
      if ( nonThreadedFaces.isEmpty() && newIndex < mMesh->faceCount() )
        return false;

      const QList<int> remainingFace = qgis::setToList( nonThreadedFaces );
      int minRemainingDegree = std::numeric_limits<int>::max();
      int minRemainingFace = -1;
      for ( const int i : remainingFace )
      {
        int degree = faceDegrees.at( i );
        if ( degree < minRemainingDegree )
        {
          minRemainingDegree = degree;
          minRemainingFace = i;
        }
      }
      currentFace = minRemainingFace;
      nonThreadedFaces.remove( currentFace );
    }
    else
    {
      currentFace = queue.dequeue();
    }

  }

  return true;
}

QVector<QgsMeshFace> QgsTopologicalMesh::Changes::addedFaces() const
{
  return mFacesToAdd;
}

QVector<QgsMeshFace> QgsTopologicalMesh::Changes::removedFaces() const
{
  return mFacesToRemove;
}

QList<int> QgsTopologicalMesh::Changes::removedFaceIndexes() const
{
  return mFaceIndexesToRemove;
}

QVector<QgsMeshVertex> QgsTopologicalMesh::Changes::addedVertices() const
{
  return mVerticesToAdd;
}

QList<int> QgsTopologicalMesh::Changes::changedCoordinatesVerticesIndexes() const
{
  return mChangeCoordinateVerticesIndexes;
}

QList<double> QgsTopologicalMesh::Changes::newVerticesZValues() const
{
  return mNewZValues;
}

QList<QgsPointXY> QgsTopologicalMesh::Changes::newVerticesXYValues() const
{
  return mNewXYValues;
}

QList<QgsPointXY> QgsTopologicalMesh::Changes::oldVerticesXYValues() const
{
  return mOldXYValues;
}

QList<int> QgsTopologicalMesh::Changes::nativeFacesIndexesGeometryChanged() const
{
  return mNativeFacesIndexesGeometryChanged;
}

bool QgsTopologicalMesh::Changes::isEmpty() const
{
  return ( mFaceIndexesToRemove.isEmpty() &&
           mFacesToAdd.isEmpty() &&
           mFacesNeighborhoodToAdd.isEmpty() &&
           mFacesToRemove.isEmpty() &&
           mFacesNeighborhoodToRemove.isEmpty() &&
           mNeighborhoodChanges.isEmpty() &&
           mVerticesToAdd.isEmpty() &&
           mVertexToFaceToAdd.isEmpty() &&
           mVerticesToRemoveIndexes.isEmpty() &&
           mRemovedVertices.isEmpty() &&
           mVerticesToFaceRemoved.isEmpty() &&
           mVerticesToFaceChanges.isEmpty() &&
           mChangeCoordinateVerticesIndexes.isEmpty() &&
           mNewZValues.isEmpty() &&
           mOldZValues.isEmpty() &&
           mNewXYValues.isEmpty() &&
           mOldXYValues.isEmpty() &&
           mNativeFacesIndexesGeometryChanged.isEmpty() );
}

QList<int> QgsTopologicalMesh::Changes::verticesToRemoveIndexes() const
{
  return mVerticesToRemoveIndexes;
}

int QgsTopologicalMesh::Changes::addedFaceIndexInMesh( int internalIndex ) const
{
  if ( internalIndex == -1 )
    return -1;

  return internalIndex + mAddedFacesFirstIndex;
}

int QgsTopologicalMesh::Changes::removedFaceIndexInMesh( int internalIndex ) const
{
  if ( internalIndex == -1 )
    return -1;

  return mFaceIndexesToRemove.at( internalIndex );
}

void QgsTopologicalMesh::Changes::clearChanges()
{
  mAddedFacesFirstIndex = 0;
  mFaceIndexesToRemove.clear();
  mFacesToAdd.clear();
  mFacesNeighborhoodToAdd.clear();
  mFacesToRemove.clear();
  mFacesNeighborhoodToRemove.clear();
  mNeighborhoodChanges.clear();

  mVerticesToAdd.clear();
  mVertexToFaceToAdd.clear();
  mVerticesToRemoveIndexes.clear();
  mRemovedVertices.clear();
  mVerticesToFaceRemoved.clear();
  mVerticesToFaceChanges.clear();

  mChangeCoordinateVerticesIndexes.clear();
  mNewZValues.clear();
  mOldZValues.clear();
  mNewXYValues.clear();
  mOldXYValues.clear();
  mNativeFacesIndexesGeometryChanged.clear();
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::addFreeVertex( const QgsMeshVertex &vertex )
{
  Changes changes;
  changes.mVerticesToAdd.append( vertex );
  changes.mVertexToFaceToAdd.append( -1 );

  mMesh->vertices.append( vertex );
  mVertexToFace.append( -1 );
  referenceAsFreeVertex( mMesh->vertices.count() - 1 );

  return changes;
}

// Returns the orientation of the polygon formed by mesh vertices, <0 counter clockwise; >0 clockwise
static double vertexPolygonOrientation( const QgsMesh &mesh, const QList<int> &vertexIndexes )
{
  if ( vertexIndexes.count() < 3 )
    return 0.0;

  int hullDomainVertexPos = -1;
  double xMin = std::numeric_limits<double>::max();
  double yMin = std::numeric_limits<double>::max();
  for ( int i = 0; i < vertexIndexes.count(); ++i )
  {
    const QgsMeshVertex &vertex = mesh.vertices.at( vertexIndexes.at( i ) );
    if ( xMin >= vertex.x() && yMin > vertex.y() )
    {
      hullDomainVertexPos = i;
      xMin = vertex.x();
      yMin = vertex.y();
    }
  }

  if ( hullDomainVertexPos >= 0 )
  {
    int iv1 = vertexIndexes.at( ( hullDomainVertexPos - 1 + vertexIndexes.count() ) % vertexIndexes.count() );
    int iv2 = vertexIndexes.at( ( hullDomainVertexPos + 1 ) % vertexIndexes.count() );
    int ivc = vertexIndexes.at( ( hullDomainVertexPos ) );
    double cp = crossProduct( ivc, iv1, iv2, mesh );
    return cp;
  }

  return 0.0;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::removeVertexFillHole( int vertexIndex )
{
  if ( vertexIndex >= mVertexToFace.count() )
    return Changes();

  if ( mVertexToFace.at( vertexIndex ) == -1 ) //it is a free vertex
  {
    Changes changes;
    changes.mRemovedVertices.append( mMesh->vertices.at( vertexIndex ) );
    changes.mVerticesToRemoveIndexes.append( vertexIndex );
    changes.mVerticesToFaceRemoved.append( -1 );
    dereferenceAsFreeVertex( vertexIndex );
    mMesh->vertices[vertexIndex] = QgsMeshVertex();
    return changes;
  }

  //search concerned faces
  QgsMeshVertexCirculator circulator = vertexCirculator( vertexIndex );
  circulator.goBoundaryClockwise();
  QList<int> boundariesVertexIndex;
  QList<int> associateFaceToBoundaries;
  QList<int> removedFacesIndexes;
  QSet<int> boundaryInGlobalMesh;

  do
  {
    removedFacesIndexes.append( circulator.currentFaceIndex() );
    boundariesVertexIndex.append( circulator.oppositeVertexClockwise() );
    Q_ASSERT( !mMesh->vertices.at( boundariesVertexIndex.last() ).isEmpty() );
    const QgsMeshFace &currentFace = circulator.currentFace();
    associateFaceToBoundaries.append( mFacesNeighborhood.at( circulator.currentFaceIndex() ).at(
                                        vertexPositionInFace( boundariesVertexIndex.last(), currentFace ) ) );

    if ( currentFace.count() > 3 ) // quad or more, need other vertices
    {
      int posInface = vertexPositionInFace( vertexIndex, currentFace );
      for ( int i = 2; i < currentFace.count() - 1; ++i )
      {
        boundariesVertexIndex.append( currentFace.at( ( posInface + i ) % currentFace.count() ) );
        Q_ASSERT( !mMesh->vertices.at( boundariesVertexIndex.last() ).isEmpty() );
        associateFaceToBoundaries.append( mFacesNeighborhood.at( circulator.currentFaceIndex() ).at(
                                            vertexPositionInFace( boundariesVertexIndex.last(), currentFace ) ) );
      }
    }
  }
  while ( circulator.turnCounterClockwise() != -1 && circulator.currentFaceIndex() != removedFacesIndexes.first() );

  bool boundaryFill = false;
  if ( circulator.currentFaceIndex() == -1 ) //we are on boundary of the mesh
  {
    boundaryFill = true;
    //we need to add last vertex/boundary faces that was not added because we are on mesh boundary
    circulator.goBoundaryCounterClockwise();
    int lastVertexIndex = circulator.oppositeVertexCounterClockwise();
    boundariesVertexIndex.append( lastVertexIndex );

    // but we can be on the case where the last vertex share an edge with the first point,
    // in the case, the associate face on boundarie is not -1, but the face on the other side of the edge
    QgsMeshVertexCirculator boundaryCirculator = vertexCirculator( lastVertexIndex );
    boundaryCirculator.goBoundaryCounterClockwise();
    if ( boundaryCirculator.oppositeVertexCounterClockwise() == boundariesVertexIndex.first() )
    {
      associateFaceToBoundaries.append( boundaryCirculator.currentFaceIndex() );
      boundaryFill = false; //we are not a boundary fill anymore
    }
    else
      associateFaceToBoundaries.append( -1 );

    for ( const int index : std::as_const( boundariesVertexIndex ) )
    {
      if ( isVertexOnBoundary( index ) )
        boundaryInGlobalMesh.insert( index );
    }
  }

  int currentVertexToFace = mVertexToFace.at( vertexIndex );
  // here, we use the method removeFaces that effectivly removes and then constructs changes
  Changes changes = removeFaces( removedFacesIndexes );

  QList<QList<int>> holes;
  QList<QList<int>> associateMeshFacesToHoles;

  bool cancelOperation = false;

  if ( boundaryFill )
  {
    // The hole is not a closed polygon, we need to close it, but the closing segment can intersect another segments/vertices.
    // In this case we consider as many polygons as necessary.

    int startPos = 0;
    int finalPos = boundariesVertexIndex.count() - 1;
    QList<int> uncoveredVertex;

    QList<int> partToCheck = boundariesVertexIndex.mid( startPos, finalPos - startPos + 1 );
    QList<int> associateFacePart = associateFaceToBoundaries.mid( startPos, finalPos - startPos + 1 );
    while ( startPos < finalPos && !partToCheck.isEmpty() )
    {
      // check if we intersect an edge between first and second
      int secondPos = partToCheck.count() - 1;
      const QgsPoint &closingSegmentExtremety1 = mMesh->vertex( partToCheck.at( 0 ) );
      const QgsPoint &closingSegmentExtremety2 = mMesh->vertex( partToCheck.last() );
      bool isEdgeIntersect = false;
      for ( int i = 1; i < secondPos - 1; ++i )
      {
        const QgsPoint &p1 = mMesh->vertex( partToCheck.at( i ) );
        const QgsPoint &p2 = mMesh->vertex( partToCheck.at( i + 1 ) );
        bool isLineIntersection;
        QgsPoint intersectPoint;
        isEdgeIntersect = QgsGeometryUtils::segmentIntersection( closingSegmentExtremety1, closingSegmentExtremety2, p1, p2, intersectPoint, isLineIntersection, 1e-8, true );
        if ( isEdgeIntersect )
          break;
      }

      int index = partToCheck.at( 0 );
      if ( boundaryInGlobalMesh.contains( index ) && index != boundariesVertexIndex.at( 0 ) )
      {
        cancelOperation = true;
        break;
      }

      // if uncovered vertex is a boundary vertex in the global mesh (except first that is always a boundary in the global mesh)
      // This operation will leads to a unique shared vertex that is not allowed, you have to cancel the operation
      if ( isEdgeIntersect || vertexPolygonOrientation( *mMesh, partToCheck ) >= 0 )
      {
        partToCheck.removeLast();
        associateFacePart.removeAt( associateFacePart.count() - 2 );
        if ( partToCheck.count() == 1 )
        {
          uncoveredVertex.append( index );
          startPos = startPos + 1;
          partToCheck = boundariesVertexIndex.mid( startPos, finalPos - startPos + 1 );
          associateFacePart = associateFaceToBoundaries.mid( startPos, finalPos - startPos + 1 );
        }
      }
      else
      {
        // store the well defined hole
        holes.append( partToCheck );
        associateMeshFacesToHoles.append( associateFacePart );

        startPos = startPos + partToCheck.count() - 1;
        uncoveredVertex.append( partToCheck.at( 0 ) );
        partToCheck = boundariesVertexIndex.mid( startPos, finalPos - startPos + 1 );
        associateFacePart = associateFaceToBoundaries.mid( startPos, finalPos - startPos + 1 );
      }
    }
  }
  else
  {
    holes.append( boundariesVertexIndex );
    associateMeshFacesToHoles.append( associateFaceToBoundaries );
  }

  if ( cancelOperation )
  {
    reverseChanges( changes );
    return Changes();
  }

  Q_ASSERT( holes.count() == associateMeshFacesToHoles.count() );

  changes.mRemovedVertices.append( mMesh->vertices.at( vertexIndex ) );
  changes.mVerticesToRemoveIndexes.append( vertexIndex );
  changes.mVerticesToFaceRemoved.append( currentVertexToFace );
  // these changes contain information that will lead to reference the removed vertex as free vertex when reverse/reapply
  dereferenceAsFreeVertex( vertexIndex );
  mMesh->vertices[vertexIndex] = QgsMeshVertex();
  mVertexToFace[vertexIndex] = -1;

  int oldFacesCount = mMesh->faceCount();
  for ( int h = 0; h < holes.count(); ++h )
  {
    const QList<int> &holeVertices = holes.at( h );
    const QList<int> &associateMeshFacesToHole = associateMeshFacesToHoles.at( h );
    QHash<p2t::Point *, int> mapPoly2TriPointToVertex;
    std::vector<p2t::Point *> holeToFill( holeVertices.count() );
    try
    {
      for ( int i = 0; i < holeVertices.count(); ++i )
      {
        const QgsMeshVertex &vertex = mMesh->vertex( holeVertices.at( i ) );
        holeToFill[i] = new p2t::Point( vertex.x(), vertex.y() );
        mapPoly2TriPointToVertex.insert( holeToFill[i], holeVertices.at( i ) );
      }

      std::unique_ptr<p2t::CDT> cdt( new p2t::CDT( holeToFill ) );

      cdt->Triangulate();
      std::vector<p2t::Triangle *> triangles = cdt->GetTriangles();
      QVector<QgsMeshFace> newFaces( triangles.size() );
      for ( size_t i = 0; i < triangles.size(); ++i )
      {
        QgsMeshFace &face = newFaces[i];
        face.resize( 3 );
        for ( int j = 0; j < 3; j++ )
        {
          int vertInd = mapPoly2TriPointToVertex.value( triangles.at( i )->GetPoint( j ), -1 );
          if ( vertInd == -1 )
            throw std::exception();
          Q_ASSERT( !mMesh->vertices.at( vertInd ).isEmpty() );
          face[j] = vertInd;
        }
      }

      QgsMeshEditingError error;
      QgsTopologicalMesh::TopologicalFaces topologicalFaces = createNewTopologicalFaces( newFaces, false, error );
      if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
        throw std::exception();
      int newFaceIndexStartIndex = mMesh->faceCount();
      QgsTopologicalMesh::Changes addChanges;
      addChanges.mAddedFacesFirstIndex = newFaceIndexStartIndex;
      addChanges.mFacesToAdd = topologicalFaces.meshFaces();
      addChanges.mFacesNeighborhoodToAdd = topologicalFaces.mFacesNeighborhood;

      // vertices to face changes
      const QList<int> &verticesToFaceToChange = topologicalFaces.mVerticesToFace.keys();
      for ( const int vtc : verticesToFaceToChange )
        if ( mVertexToFace.at( vtc ) == -1 )
          addChanges.mVerticesToFaceChanges.append( {vtc,
              mVertexToFace.at( vtc ),
              addChanges.addedFaceIndexInMesh( topologicalFaces.mVerticesToFace.values( vtc ).first() ) } );


      // reindex neighborhood for new faces
      for ( int i = 0; i < topologicalFaces.mFaces.count(); ++i )
      {
        FaceNeighbors &faceNeighbors = addChanges.mFacesNeighborhoodToAdd[i];
        faceNeighbors = topologicalFaces.mFacesNeighborhood.at( i );
        for ( int n = 0; n < faceNeighbors.count(); ++n )
        {
          if ( faceNeighbors.at( n ) != -1 )
            faceNeighbors[n] += newFaceIndexStartIndex; //reindex internal neighborhood
        }
      }

      // link neighborhood for boundaries of each side
      for ( int i = 0 ; i < holeVertices.count(); ++i )
      {
        int vertexHoleIndex = holeVertices.at( i );
        int meshFaceBoundaryIndex = associateMeshFacesToHole.at( i );

        const QgsMeshVertexCirculator circulator = QgsMeshVertexCirculator( topologicalFaces, vertexHoleIndex );
        circulator.goBoundaryClockwise();
        int newFaceBoundaryLocalIndex = circulator.currentFaceIndex();
        int newFaceBoundaryIndexInMesh = circulator.currentFaceIndex() + newFaceIndexStartIndex;
        const QgsMeshFace &newFace = circulator.currentFace();
        int positionInNewFaces = vertexPositionInFace( vertexHoleIndex, newFace );

        if ( meshFaceBoundaryIndex != -1 )
        {
          const QgsMeshFace meshFace = mMesh->face( meshFaceBoundaryIndex );
          int positionInMeshFaceBoundary = vertexPositionInFace( *mMesh, vertexHoleIndex, meshFaceBoundaryIndex );
          positionInMeshFaceBoundary = ( positionInMeshFaceBoundary - 1 + meshFace.count() ) % meshFace.count(); //take the position just before

          addChanges.mNeighborhoodChanges.append( {meshFaceBoundaryIndex, positionInMeshFaceBoundary, -1, newFaceBoundaryIndexInMesh} );
        }

        addChanges.mFacesNeighborhoodToAdd[newFaceBoundaryLocalIndex][positionInNewFaces] = meshFaceBoundaryIndex;
      }

      applyChanges( addChanges );

      changes.mFacesToAdd.append( addChanges.mFacesToAdd );
      changes.mFacesNeighborhoodToAdd.append( addChanges.mFacesNeighborhoodToAdd );
      //for each neighborhood change, check if a corresponding change already exist and merge them, if not just append
      for ( const std::array<int, 4> &neighborChangeToAdd : std::as_const( addChanges.mNeighborhoodChanges ) )
      {
        bool merged = false;
        for ( std::array<int, 4> &existingNeighborChange : changes.mNeighborhoodChanges )
        {
          if ( existingNeighborChange.at( 0 ) == neighborChangeToAdd.at( 0 ) &&
               existingNeighborChange.at( 1 ) == neighborChangeToAdd.at( 1 ) )
          {
            merged = true;
            Q_ASSERT( existingNeighborChange.at( 3 ) == neighborChangeToAdd.at( 2 ) );
            existingNeighborChange[3] = neighborChangeToAdd.at( 3 );
          }
        }
        if ( !merged )
          changes.mNeighborhoodChanges.append( neighborChangeToAdd );
      }
      //for each vertex to face change, check if a corresponding change already exist and merge them, if not just append
      for ( const std::array<int, 3> &verticesToFaceToAdd : std::as_const( addChanges.mVerticesToFaceChanges ) )
      {
        bool merged = false;
        for ( std::array<int, 3> &existingVerticesToFace : changes.mVerticesToFaceChanges )
        {
          if ( existingVerticesToFace.at( 0 ) == verticesToFaceToAdd.at( 0 ) )
          {
            merged = true;
            Q_ASSERT( existingVerticesToFace.at( 2 ) == verticesToFaceToAdd.at( 1 ) );
            existingVerticesToFace[2] = verticesToFaceToAdd.at( 2 );
          }
        }
        if ( !merged )
          changes.mVerticesToFaceChanges.append( verticesToFaceToAdd );
      }

      qDeleteAll( holeToFill );
    }
    catch ( ... )
    {
      qDeleteAll( holeToFill );
      QgsMessageLog::logMessage( QObject::tr( "Triangulation failed. Skipping hole…" ), QObject::tr( "Mesh Editing" ) );
    }
  }
  changes.mAddedFacesFirstIndex = oldFacesCount;



  changes.mAddedFacesFirstIndex = oldFacesCount;

  return changes;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::removeVertices( const QList<int> &vertices )
{
  QSet<int> facesIndex;
  //Search for associated faces
  for ( int vertexIndex : vertices )
    facesIndex.unite( qgis::listToSet( facesAroundVertex( vertexIndex ) ) );

  // remove the faces
  Changes changes = removeFaces( facesIndex.values() );

  // removes the vertices
  for ( int vertexIndex : vertices )
  {
    int currentVertexToFace = mVertexToFace.at( vertexIndex );
    // here, we use the method removeFaces that effectivly removes and then constructs changes
    changes.mRemovedVertices.append( mMesh->vertices.at( vertexIndex ) );
    changes.mVerticesToRemoveIndexes.append( vertexIndex );
    changes.mVerticesToFaceRemoved.append( currentVertexToFace );
    // these changes contain information that will lead to reference the removed vertex as free vertex when reverse/reapply
    dereferenceAsFreeVertex( vertexIndex );
    mMesh->vertices[vertexIndex] = QgsMeshVertex();
    mVertexToFace[vertexIndex] = -1;
  }

  return changes;
}

QgsMeshEditingError QgsTopologicalMesh::facesCanBeAdded( const QgsTopologicalMesh::TopologicalFaces &topologicFaces ) const
{
  QList<int> boundariesToCheckClockwiseInNewFaces = topologicFaces.mBoundaries;
  QList<std::array<int, 2>> boundariesToCheckCounterClockwiseInNewFaces; //couple boundary / associate face in topologicFaces.mVerticesToFace
  QList<int> uniqueSharedVertexBoundary;


  // Go through the boundary and search for opposite boundary vertex clockwise in new faces and compare
  // with boundary opposite vertices on the mesh
  // If, in the mesh, the opposite vertex counter clockwise is not the same , another check will be done
  // later with counter clockwise in new faces
  // If, in the mesh, the opposite vertex clockwise is the same, this is an error
  while ( !boundariesToCheckClockwiseInNewFaces.isEmpty() )
  {
    int boundary = boundariesToCheckClockwiseInNewFaces.takeLast();

    const QList<int> &linkedFaces = topologicFaces.mVerticesToFace.values( boundary );

    for ( int const linkedFace : linkedFaces )
    {

      //if the boundary is a free vertex in the destination mesh, no need to check
      if ( mVertexToFace.at( boundary ) == -1 )
        continue;

      QgsMeshVertexCirculator newFacescirculator( topologicFaces, linkedFace, boundary );
      QgsMeshVertexCirculator meshCirculator = vertexCirculator( boundary );

      if ( !newFacescirculator.isValid() )
        return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, boundary );

      //Get the opposite vertex on the clockwise side with new faces block
      if ( !newFacescirculator.goBoundaryClockwise() )
        return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, boundary );

      int oppositeVertexInNewFaces = newFacescirculator.oppositeVertexClockwise();

      if ( !meshCirculator.goBoundaryCounterClockwise() )
        return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, boundary );

      int oppositeVertexCCWInMesh = meshCirculator.oppositeVertexCounterClockwise();

      if ( oppositeVertexCCWInMesh == oppositeVertexInNewFaces ) //this boundary is OK, continue wit next one
        continue;
      else
      {
        //to avoid manifold face that could pass through the check, compare not only the boundary edges but also with the opposite internal edge in new face
        const QgsMeshFace &newFaceOnBoundary = newFacescirculator.currentFace();
        int faceSize = newFaceOnBoundary.size();
        int posInNewFace = vertexPositionInFace( boundary, newFaceOnBoundary );
        int previousVertexIndex = ( posInNewFace + faceSize - 1 ) % faceSize;
        if ( newFaceOnBoundary.at( previousVertexIndex ) == oppositeVertexCCWInMesh )
          return QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, newFacescirculator.currentFaceIndex() );
      }

      meshCirculator.goBoundaryClockwise();

      int oppositeVertexCWInMesh = meshCirculator.oppositeVertexClockwise();

      if ( oppositeVertexCWInMesh == oppositeVertexInNewFaces )
        return QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, newFacescirculator.currentFaceIndex() );

      //if we are here we need more checks
      boundariesToCheckCounterClockwiseInNewFaces.append( {boundary, linkedFace} );
    }
  }

  // Check now with opposite boundary vertex counter clockwise in new faces
  while ( !boundariesToCheckCounterClockwiseInNewFaces.isEmpty() )
  {
    std::array<int, 2> boundaryLinkedface = boundariesToCheckCounterClockwiseInNewFaces.takeLast();
    int boundary = boundaryLinkedface.at( 0 );
    int linkedFace = boundaryLinkedface.at( 1 );

    QgsMeshVertexCirculator newFacescirculator( topologicFaces, linkedFace, boundary );
    QgsMeshVertexCirculator meshCirculator = vertexCirculator( boundary );

    if ( !newFacescirculator.goBoundaryCounterClockwise() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, boundary );

    int oppositeVertexInNewFaces = newFacescirculator.oppositeVertexCounterClockwise();

    if ( !meshCirculator.goBoundaryClockwise() )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, boundary );

    int oppositeVertexCWInMesh = meshCirculator.oppositeVertexClockwise();

    if ( oppositeVertexCWInMesh == oppositeVertexInNewFaces ) //this boundary is OK, continue with next one
      continue;

    meshCirculator.goBoundaryCounterClockwise();

    int oppositeVertexCCWInMesh = meshCirculator.oppositeVertexCounterClockwise();

    if ( oppositeVertexCCWInMesh == oppositeVertexInNewFaces )
      return QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, newFacescirculator.currentFaceIndex() );

    uniqueSharedVertexBoundary.append( boundary );
  }

  if ( !uniqueSharedVertexBoundary.isEmpty() )
    return QgsMeshEditingError( Qgis::MeshEditingErrorType::UniqueSharedVertex, uniqueSharedVertexBoundary.first() );

  // Check if internal vertices of new faces block are free in the mesh
  QSet<int> boundaryVertices = qgis::listToSet( topologicFaces.mBoundaries );
  for ( const QgsMeshFace &newFace : std::as_const( topologicFaces.mFaces ) )
  {
    for ( const int vertexIndex : newFace )
    {
      if ( boundaryVertices.contains( vertexIndex ) )
        continue;
      if ( mVertexToFace.at( vertexIndex ) != -1 )
        return QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, vertexIndex );
    }
  }

  return QgsMeshEditingError();
}

void QgsTopologicalMesh::TopologicalFaces::clear()
{
  mFaces.clear();
  mFacesNeighborhood.clear();
  mVerticesToFace.clear();
  mBoundaries.clear();
}

QVector<QgsTopologicalMesh::FaceNeighbors> QgsTopologicalMesh::TopologicalFaces::facesNeighborhood() const
{
  return mFacesNeighborhood;
}

int QgsTopologicalMesh::TopologicalFaces::vertexToFace( int vertexIndex ) const
{
  if ( mVerticesToFace.contains( vertexIndex ) )
    return mVerticesToFace.values( vertexIndex ).at( 0 );

  return -1;
}

QgsTopologicalMesh QgsTopologicalMesh::createTopologicalMesh( QgsMesh *mesh, int maxVerticesPerFace, QgsMeshEditingError &error )
{
  QgsTopologicalMesh topologicMesh;
  topologicMesh.mMesh = mesh;
  topologicMesh.mVertexToFace = QVector<int>( mesh->vertexCount(), -1 );
  topologicMesh.mMaximumVerticesPerFace = maxVerticesPerFace;
  error.errorType = Qgis::MeshEditingErrorType::NoError;

  for ( int i = 0; i < mesh->faceCount(); ++i )
  {
    if ( mesh->face( i ).isEmpty() )
      continue;
    if ( maxVerticesPerFace != 0 && mesh->face( i ).count() > maxVerticesPerFace )
    {
      error = QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, i );
      break;
    }

    error = counterClockwiseFaces( mesh->faces[i], mesh );
    if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    {
      if ( error.errorType == Qgis::MeshEditingErrorType::InvalidFace || error.errorType == Qgis::MeshEditingErrorType::FlatFace )
        error.elementIndex = i;
      break;
    }
  }

  if ( error.errorType == Qgis::MeshEditingErrorType::NoError )
  {
    TopologicalFaces subMesh = createTopologicalFaces( mesh->faces, &topologicMesh.mVertexToFace, error, false );
    topologicMesh.mFacesNeighborhood = subMesh.mFacesNeighborhood;

    for ( int i = 0; i < topologicMesh.mMesh->vertexCount(); ++i )
    {
      if ( topologicMesh.mVertexToFace.at( i ) == -1 )
        topologicMesh.mFreeVertices.insert( i );
    }
  }

  return topologicMesh;
}

QgsTopologicalMesh::TopologicalFaces QgsTopologicalMesh::createNewTopologicalFaces( const QVector<QgsMeshFace> &faces,  bool uniqueSharedVertexAllowed, QgsMeshEditingError &error )
{
  return createTopologicalFaces( faces, nullptr, error, uniqueSharedVertexAllowed );
}


QgsTopologicalMesh::TopologicalFaces QgsTopologicalMesh::createTopologicalFaces(
  const QVector<QgsMeshFace> &faces,
  QVector<int> *globalVertexToFace,
  QgsMeshEditingError &error,
  bool allowUniqueSharedVertex )
{
  int facesCount = faces.count();
  QVector<FaceNeighbors> faceTopologies;
  QMultiHash<int, int> verticesToFace;

  error = QgsMeshEditingError();
  TopologicalFaces ret;

  // Contains for each vertex a map (opposite vertex # edge) --> face index
  // when turning counter clockwise, if v1 first vertex and v2 second one, [v1][v2]--> neighbor face
  QMap<int, QMap<int, int>> verticesToNeighbor;

  for ( int faceIndex = 0; faceIndex < facesCount; ++faceIndex )
  {
    const QgsMeshFace &face = faces.at( faceIndex );
    int faceSize = face.count();
    //Fill vertices to neighbor faces
    for ( int i = 0; i < faceSize; ++i )
    {
      int v1 = face[i % faceSize];
      int v2 = face[( i + 1 ) % faceSize];
      if ( verticesToNeighbor[v2].contains( v1 ) )
      {
        error = QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, faceIndex );
        return ret;
      }
      else
        verticesToNeighbor[v2].insert( v1, faceIndex );
    }
  }

  faceTopologies = QVector<FaceNeighbors>( faces.count() );

  QSet<int> boundaryVertices;

  for ( int faceIndex = 0; faceIndex < facesCount; ++faceIndex )
  {
    const QgsMeshFace &face = faces.at( faceIndex );
    int faceSize = face.size();
    FaceNeighbors &faceTopology = faceTopologies[faceIndex];
    faceTopology.resize( faceSize );

    for ( int i = 0; i < faceSize; ++i )
    {
      int v1 = face.at( i );
      int v2 = face.at( ( i + 1 ) % faceSize );

      if ( globalVertexToFace )
      {
        if ( ( *globalVertexToFace )[v1] == -1 )
          ( *globalVertexToFace )[v1] = faceIndex ;
      }
      else
      {
        if ( allowUniqueSharedVertex || !verticesToFace.contains( v1 ) )
          verticesToFace.insert( v1, faceIndex ) ;
      }

      QMap<int, int> &edges = verticesToNeighbor[v1];
      if ( edges.contains( v2 ) )
        faceTopology[i] = edges.value( v2 );
      else
      {
        faceTopology[i] = -1;

        if ( !allowUniqueSharedVertex )
        {
          if ( boundaryVertices.contains( v1 ) )
          {
            error = QgsMeshEditingError( Qgis::MeshEditingErrorType::UniqueSharedVertex, v1 ); // if a vertices is more than one time in the boundary, that means faces share only one vertices
            return ret;
          }
        }
        boundaryVertices.insert( v1 );
      }
    }
  }

  ret.mFaces = faces;
  ret.mFacesNeighborhood = faceTopologies;
  ret.mBoundaries = boundaryVertices.values();
  ret.mVerticesToFace = verticesToFace;
  return ret;
}

QVector<int> QgsTopologicalMesh::neighborsOfFace( int faceIndex ) const
{
  return mFacesNeighborhood.at( faceIndex );
}

QList<int> QgsTopologicalMesh::facesAroundVertex( int vertexIndex ) const
{
  QgsMeshVertexCirculator circ = vertexCirculator( vertexIndex );

  return circ.facesAround();
}

QgsMeshEditingError QgsTopologicalMesh::facesCanBeRemoved( const QList<int> facesIndexes )
{
  QSet<int> removedFaces = qgis::listToSet( facesIndexes );
  QSet<int> concernedFaces = concernedFacesBy( facesIndexes );

  for ( const int f : std::as_const( removedFaces ) )
    concernedFaces.remove( f );

  QVector<QgsMeshFace> remainingFaces;
  remainingFaces.reserve( concernedFaces.count() );
  for ( const int f : std::as_const( concernedFaces ) )
    remainingFaces.append( mMesh->face( f ) );

  QgsMeshEditingError error;
  createTopologicalFaces( remainingFaces, nullptr, error, false );

  return error;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::removeFaces( const QList<int> facesIndexesToRemove )
{
  Changes changes;
  changes.mFaceIndexesToRemove = facesIndexesToRemove;
  changes.mFacesToRemove.resize( facesIndexesToRemove.count() );
  changes.mFacesNeighborhoodToRemove.resize( facesIndexesToRemove.count() );

  QSet<int> indexSet = qgis::listToSet( facesIndexesToRemove );
  QSet<int> threatedVertex;

  for ( int i = 0; i < facesIndexesToRemove.count(); ++i )
  {
    const int faceIndex = facesIndexesToRemove.at( i );
    const QgsMeshFace &face = mMesh->face( faceIndex );
    changes.mFacesToRemove[i] = face;
    const FaceNeighbors &neighborhood = mFacesNeighborhood.at( faceIndex );
    changes.mFacesNeighborhoodToRemove[i] = neighborhood;
    for ( int j = 0; j < face.count(); ++j )
    {
      //change the neighborhood for each neighbor face
      int neighborIndex = neighborhood.at( j );
      if ( neighborIndex != -1 && !indexSet.contains( neighborIndex ) )
      {
        int positionInNeighbor = mFacesNeighborhood.at( neighborIndex ).indexOf( faceIndex );
        changes.mNeighborhoodChanges.append( {neighborIndex, positionInNeighbor, faceIndex, -1} );
      }

      //change vertexToFace
      int vertexIndex = face.at( j );
      if ( !threatedVertex.contains( vertexIndex ) && indexSet.contains( mVertexToFace.at( vertexIndex ) ) )
      {
        int oldValue = mVertexToFace.at( vertexIndex );
        //look for another face linked to this vertex
        int refValue = -1;
        if ( neighborIndex != -1 && !indexSet.contains( neighborIndex ) ) //if exist, simpler to take it
          refValue = neighborIndex;
        else
        {
          QList<int> aroundFaces = facesAroundVertex( vertexIndex );
          aroundFaces.removeOne( faceIndex );
          if ( !aroundFaces.isEmpty() )
          {
            while ( !aroundFaces.isEmpty() && refValue == -1 )
            {
              if ( !indexSet.contains( aroundFaces.first() ) )
                refValue = aroundFaces.first();
              else
                aroundFaces.removeFirst();
            }
          }
        }
        changes.mVerticesToFaceChanges.append( {vertexIndex, oldValue, refValue} );
        threatedVertex.insert( vertexIndex );
      }
    }
  }

  applyChanges( changes );

  return changes;
}

bool QgsTopologicalMesh::eitherSideFacesAndVertices( int vertexIndex1,
    int vertexIndex2,
    int &face1,
    int &face2,
    int &neighborVertex1InFace1,
    int &neighborVertex1InFace2,
    int &neighborVertex2inFace1,
    int &neighborVertex2inFace2 ) const
{
  QgsMeshVertexCirculator circulator1 = vertexCirculator( vertexIndex1 );
  QgsMeshVertexCirculator circulator2 = vertexCirculator( vertexIndex2 );

  circulator1.goBoundaryClockwise();
  int firstFace1 = circulator1.currentFaceIndex();
  circulator2.goBoundaryClockwise();
  int firstFace2 = circulator2.currentFaceIndex();

  if ( circulator1.oppositeVertexCounterClockwise() != vertexIndex2 )
    while ( circulator1.turnCounterClockwise() != -1 &&
            circulator1.currentFaceIndex() != firstFace1 &&
            circulator1.oppositeVertexCounterClockwise() != vertexIndex2 ) {}

  if ( circulator2.oppositeVertexCounterClockwise() != vertexIndex1 )
    while ( circulator2.turnCounterClockwise() != -1 &&
            circulator2.currentFaceIndex() != firstFace2 &&
            circulator2.oppositeVertexCounterClockwise() != vertexIndex1 ) {}

  if ( circulator1.oppositeVertexCounterClockwise() != vertexIndex2
       || circulator2.oppositeVertexCounterClockwise() != vertexIndex1 )
    return false;

  face1 = circulator1.currentFaceIndex();
  face2 = circulator2.currentFaceIndex();

  neighborVertex1InFace1 = circulator1.oppositeVertexClockwise();
  circulator1.turnCounterClockwise();
  neighborVertex1InFace2 = circulator1.oppositeVertexCounterClockwise();

  neighborVertex2inFace2 = circulator2.oppositeVertexClockwise();
  circulator2.turnCounterClockwise();
  neighborVertex2inFace1 = circulator2.oppositeVertexCounterClockwise();

  return true;
}

bool QgsTopologicalMesh::edgeCanBeFlipped( int vertexIndex1, int vertexIndex2 ) const
{
  int faceIndex1;
  int faceIndex2;
  int oppositeVertexFace1;
  int oppositeVertexFace2;
  int supposedOppositeVertexFace1;
  int supposedoppositeVertexFace2;

  bool result = eitherSideFacesAndVertices(
                  vertexIndex1,
                  vertexIndex2,
                  faceIndex1,
                  faceIndex2,
                  oppositeVertexFace1,
                  supposedoppositeVertexFace2,
                  supposedOppositeVertexFace1,
                  oppositeVertexFace2 );

  if ( !result ||
       faceIndex1 < 0 ||
       faceIndex2 < 0 ||
       oppositeVertexFace1 < 0 ||
       oppositeVertexFace2 < 0 ||
       supposedOppositeVertexFace1 != oppositeVertexFace1 ||
       supposedoppositeVertexFace2 != oppositeVertexFace2 )
    return false;

  const QgsMeshFace &face1 = mMesh->face( faceIndex1 );
  const QgsMeshFace &face2 = mMesh->face( faceIndex2 );


  if ( face1.count() != 3 || face2.count() != 3 )
    return false;

  double crossProduct1 = crossProduct( vertexIndex1, oppositeVertexFace1, oppositeVertexFace2, *mMesh );
  double crossProduct2 = crossProduct( vertexIndex2, oppositeVertexFace1, oppositeVertexFace2, *mMesh );

  return crossProduct1 * crossProduct2 < 0;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::flipEdge( int vertexIndex1, int vertexIndex2 )
{
  int faceIndex1;
  int faceIndex2;
  int oppositeVertexFace1;
  int oppositeVertexFace2;
  int supposedOppositeVertexFace1;
  int supposedoppositeVertexFace2;

  bool result = eitherSideFacesAndVertices(
                  vertexIndex1,
                  vertexIndex2,
                  faceIndex1,
                  faceIndex2,
                  oppositeVertexFace1,
                  supposedoppositeVertexFace2,
                  supposedOppositeVertexFace1,
                  oppositeVertexFace2 );

  if ( !result ||
       faceIndex1 < 0 ||
       faceIndex2 < 0 ||
       oppositeVertexFace1 < 0 ||
       oppositeVertexFace2 < 0 ||
       supposedOppositeVertexFace1 != oppositeVertexFace1 ||
       supposedoppositeVertexFace2 != oppositeVertexFace2 )
    return Changes();


  Changes changes;

  const QgsMeshFace &face1 = mMesh->face( faceIndex1 );
  const QgsMeshFace &face2 = mMesh->face( faceIndex2 );

  Q_ASSERT( face1.count() == 3 );
  Q_ASSERT( face2.count() == 3 );

  int pos1 = vertexPositionInFace( vertexIndex1, face1 );
  int pos2 = vertexPositionInFace( vertexIndex2, face2 );

  int neighborFace1 = mFacesNeighborhood.at( faceIndex1 ).at( pos1 );
  int posInNeighbor1 = vertexPositionInFace( *mMesh, oppositeVertexFace1, neighborFace1 );
  int neighborFace2 = mFacesNeighborhood.at( faceIndex1 ).at( ( pos1 + 1 ) % 3 );
  int posInNeighbor2 = vertexPositionInFace( *mMesh, vertexIndex2, neighborFace2 );
  int neighborFace3 = mFacesNeighborhood.at( faceIndex2 ).at( pos2 );
  int posInNeighbor3 = vertexPositionInFace( *mMesh, oppositeVertexFace2, neighborFace3 );
  int neighborFace4 = mFacesNeighborhood.at( faceIndex2 ).at( ( pos2 + 1 ) % 3 );
  int posInNeighbor4 = vertexPositionInFace( *mMesh, vertexIndex1, neighborFace4 );

  changes.mFaceIndexesToRemove.append( faceIndex1 );
  changes.mFaceIndexesToRemove.append( faceIndex2 );
  changes.mFacesToRemove.append( face1 );
  changes.mFacesToRemove.append( face2 );
  changes.mFacesNeighborhoodToRemove.append( mFacesNeighborhood.at( faceIndex1 ) );
  changes.mFacesNeighborhoodToRemove.append( mFacesNeighborhood.at( faceIndex2 ) );
  int startIndex = mMesh->faceCount();
  changes.mAddedFacesFirstIndex = startIndex;
  changes.mFacesToAdd.append( {oppositeVertexFace1, oppositeVertexFace2, vertexIndex1} );
  changes.mFacesToAdd.append( {oppositeVertexFace2, oppositeVertexFace1, vertexIndex2} );
  changes.mFacesNeighborhoodToAdd.append( {startIndex + 1,
                                          mFacesNeighborhood.at( faceIndex2 ).at( ( pos2 + 1 ) % 3 ),
                                          mFacesNeighborhood.at( faceIndex1 ).at( pos1 )} );
  changes.mFacesNeighborhoodToAdd.append( {startIndex,
                                          mFacesNeighborhood.at( faceIndex1 ).at( ( pos1 + 1 ) % 3 ),
                                          mFacesNeighborhood.at( faceIndex2 ).at( pos2 )} );

  if ( neighborFace1 >= 0 )
    changes.mNeighborhoodChanges.append( {neighborFace1, posInNeighbor1, faceIndex1, startIndex} );
  if ( neighborFace2 >= 0 )
    changes.mNeighborhoodChanges.append( {neighborFace2, posInNeighbor2, faceIndex1, startIndex + 1} );
  if ( neighborFace3 >= 0 )
    changes.mNeighborhoodChanges.append( {neighborFace3, posInNeighbor3, faceIndex2, startIndex + 1} );
  if ( neighborFace4 >= 0 )
    changes.mNeighborhoodChanges.append( {neighborFace4, posInNeighbor4, faceIndex2, startIndex} );


  if ( mVertexToFace.at( vertexIndex1 ) == faceIndex1 || mVertexToFace.at( vertexIndex1 ) == faceIndex2 )
    changes.mVerticesToFaceChanges.append( {vertexIndex1,  mVertexToFace.at( vertexIndex1 ), startIndex} );
  if ( mVertexToFace.at( vertexIndex2 ) == faceIndex1 || mVertexToFace.at( vertexIndex2 ) == faceIndex2 )
    changes.mVerticesToFaceChanges.append( {vertexIndex2,  mVertexToFace.at( vertexIndex2 ), startIndex + 1} );

  if ( mVertexToFace.at( oppositeVertexFace1 ) == faceIndex1 )
    changes.mVerticesToFaceChanges.append( {oppositeVertexFace1,  faceIndex1, startIndex} );

  if ( mVertexToFace.at( oppositeVertexFace2 ) == faceIndex2 )
    changes.mVerticesToFaceChanges.append( {oppositeVertexFace2,  faceIndex2, startIndex + 1} );

  applyChanges( changes );

  return changes;
}

bool QgsTopologicalMesh::canBeMerged( int vertexIndex1, int vertexIndex2 ) const
{
  int faceIndex1;
  int faceIndex2;
  int neighborVertex1InFace1;
  int neighborVertex1InFace2;
  int neighborVertex2inFace1;
  int neighborVertex2inFace2;

  bool result = eitherSideFacesAndVertices(
                  vertexIndex1,
                  vertexIndex2,
                  faceIndex1,
                  faceIndex2,
                  neighborVertex1InFace1,
                  neighborVertex1InFace2,
                  neighborVertex2inFace1,
                  neighborVertex2inFace2 );

  if ( !result ||
       faceIndex1 < 0 ||
       faceIndex2 < 0 )
    return false;

  const QgsMeshFace &face1 = mMesh->face( faceIndex1 );
  const QgsMeshFace &face2 = mMesh->face( faceIndex2 );

  if ( face1.count() + face2.count() - 2 > mMaximumVerticesPerFace )
    return false;

  QgsMeshVertex v1 = mMesh->vertices.at( vertexIndex1 );
  QgsMeshVertex v2 = mMesh->vertices.at( vertexIndex2 );
  QgsMeshVertex nv11 = mMesh->vertices.at( neighborVertex1InFace1 );
  QgsMeshVertex nv12 = mMesh->vertices.at( neighborVertex1InFace2 );
  QgsMeshVertex nv21 = mMesh->vertices.at( neighborVertex2inFace1 );
  QgsMeshVertex nv22 = mMesh->vertices.at( neighborVertex2inFace2 );

  double crossProduct1 = crossProduct( vertexIndex1, neighborVertex1InFace1, neighborVertex1InFace2, *mMesh );
  double crossProduct2 = crossProduct( vertexIndex2, neighborVertex2inFace1, neighborVertex2inFace2, *mMesh );

  return crossProduct1 * crossProduct2 < 0;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::merge( int vertexIndex1, int vertexIndex2 )
{
  int faceIndex1;
  int faceIndex2;
  int neighborVertex1InFace1;
  int neighborVertex1InFace2;
  int neighborVertex2inFace1;
  int neighborVertex2inFace2;

  bool result = eitherSideFacesAndVertices(
                  vertexIndex1,
                  vertexIndex2,
                  faceIndex1,
                  faceIndex2,
                  neighborVertex1InFace1,
                  neighborVertex1InFace2,
                  neighborVertex2inFace1,
                  neighborVertex2inFace2 );

  if ( !result ||
       faceIndex1 < 0 ||
       faceIndex2 < 0 )
    return Changes();

  Changes changes;

  const QgsMeshFace &face1 = mMesh->face( faceIndex1 );
  const QgsMeshFace &face2 = mMesh->face( faceIndex2 );
  int faceSize1 = face1.count();
  int faceSize2 = face2.count();

  int pos1 = vertexPositionInFace( vertexIndex1, face1 );
  int pos2 = vertexPositionInFace( vertexIndex2, face2 );

  changes.mFaceIndexesToRemove.append( faceIndex1 );
  changes.mFaceIndexesToRemove.append( faceIndex2 );
  changes.mFacesToRemove.append( face1 );
  changes.mFacesToRemove.append( face2 );
  changes.mFacesNeighborhoodToRemove.append( mFacesNeighborhood.at( faceIndex1 ) );
  changes.mFacesNeighborhoodToRemove.append( mFacesNeighborhood.at( faceIndex2 ) );
  int startIndex = mMesh->faceCount();
  changes.mAddedFacesFirstIndex = startIndex;

  QgsMeshFace newface;
  FaceNeighbors newNeighborhood;
  for ( int i = 0; i < faceSize1 - 1; ++i )
  {
    int currentPos = ( pos1 + i ) % faceSize1;
    newface.append( face1.at( currentPos ) ); //add vertex of face1

    int currentNeighbor = mFacesNeighborhood.at( faceIndex1 ).at( currentPos );
    newNeighborhood.append( currentNeighbor );

    if ( currentNeighbor != -1 )
    {
      int currentPosInNeighbor = vertexPositionInFace( *mMesh, face1.at( ( currentPos + 1 ) % faceSize1 ), currentNeighbor );
      changes.mNeighborhoodChanges.append( {currentNeighbor, currentPosInNeighbor, faceIndex1, startIndex} );
    }
  }
  for ( int i = 0; i < faceSize2 - 1; ++i )
  {
    int currentPos = ( pos2 + i ) % faceSize2;
    newface.append( face2.at( currentPos ) ); //add vertex of face2

    int currentNeighbor = mFacesNeighborhood.at( faceIndex2 ).at( currentPos );
    newNeighborhood.append( currentNeighbor );

    if ( currentNeighbor != -1 )
    {
      int currentPosInNeighbor = vertexPositionInFace( *mMesh, face2.at( ( currentPos + 1 ) % faceSize2 ), currentNeighbor );
      changes.mNeighborhoodChanges.append( {currentNeighbor, currentPosInNeighbor, faceIndex2, startIndex} );
    }
  }

  for ( int i = 0; i < faceSize1; ++i )
    if ( mVertexToFace.at( face1.at( i ) ) == faceIndex1 )
      changes.mVerticesToFaceChanges.append( {face1.at( i ), faceIndex1, startIndex} );

  for ( int i = 0; i < faceSize2; ++i )
    if ( mVertexToFace.at( face2.at( i ) ) == faceIndex2 )
      changes.mVerticesToFaceChanges.append( {face2.at( i ), faceIndex2, startIndex} );

  changes.mFacesToAdd.append( newface );
  changes.mFacesNeighborhoodToAdd.append( newNeighborhood );

  applyChanges( changes );

  return changes;
}

bool QgsTopologicalMesh::canBeSplit( int faceIndex ) const
{
  const QgsMeshFace face = mMesh->face( faceIndex );

  return face.count() == 4;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::splitFace( int faceIndex )
{
  //search for the spliited angle (greater angle)
  const QgsMeshFace &face = mMesh->face( faceIndex );
  int faceSize = face.count();

  Q_ASSERT( faceSize == 4 );

  double maxAngle = 0;
  int splitVertexPos = -1;
  for ( int i = 0; i < faceSize; ++i )
  {
    QgsVector vect1( mMesh->vertex( face.at( i ) ) - mMesh->vertex( face.at( ( i + 1 ) % faceSize ) ) );
    QgsVector vect2( mMesh->vertex( face.at( ( i + 2 ) % faceSize ) ) - mMesh->vertex( face.at( ( i + 1 ) % faceSize ) ) );

    double angle = std::abs( vect1.angle( vect2 ) );
    angle = std::min( angle, 2.0 * M_PI - angle );
    if ( angle > maxAngle )
    {
      maxAngle = angle;
      splitVertexPos = ( i + 1 ) % faceSize;
    }
  }

  Changes changes;

  const QgsMeshFace newFace1 = {face.at( splitVertexPos ),
                                face.at( ( splitVertexPos + 1 ) % faceSize ),
                                face.at( ( splitVertexPos + 2 ) % faceSize )
                               };

  const QgsMeshFace newFace2 = {face.at( splitVertexPos ),
                                face.at( ( splitVertexPos + 2 ) % faceSize ),
                                face.at( ( splitVertexPos + 3 ) % faceSize )
                               };

  QVector<int> neighborIndex( faceSize );
  QVector<int> posInNeighbor( faceSize );

  for ( int i = 0; i < faceSize; ++i )
  {
    neighborIndex[i] = mFacesNeighborhood.at( faceIndex ).at( ( splitVertexPos + i ) % faceSize );
    posInNeighbor[i] = vertexPositionInFace( *mMesh,  face.at( ( splitVertexPos + i + 1 ) % faceSize ), neighborIndex[i] );
  }

  changes.mFaceIndexesToRemove.append( faceIndex );
  changes.mFacesToRemove.append( face );
  changes.mFacesNeighborhoodToRemove.append( mFacesNeighborhood.at( faceIndex ) );
  int startIndex = mMesh->faceCount();
  changes.mAddedFacesFirstIndex = startIndex;
  changes.mFacesToAdd.append( newFace1 );
  changes.mFacesToAdd.append( newFace2 );

  changes.mFacesNeighborhoodToAdd.append( {mFacesNeighborhood.at( faceIndex ).at( splitVertexPos ),
                                          mFacesNeighborhood.at( faceIndex ).at( ( splitVertexPos + 1 ) % faceSize ),
                                          startIndex + 1} );
  changes.mFacesNeighborhoodToAdd.append( {startIndex,
                                          mFacesNeighborhood.at( faceIndex ).at( ( splitVertexPos + 2 ) % faceSize ),
                                          mFacesNeighborhood.at( faceIndex ).at( ( splitVertexPos + 3 ) % faceSize )} );

  for ( int i = 0; i < faceSize; ++i )
  {
    if ( neighborIndex[i] >= 0 )
      changes.mNeighborhoodChanges.append( {neighborIndex[i], posInNeighbor[i], faceIndex, startIndex + int( i / 2 )} );

    int vertexIndex = face.at( ( splitVertexPos + i ) % faceSize );
    if ( mVertexToFace.at( vertexIndex ) == faceIndex )
      changes.mVerticesToFaceChanges.append( {vertexIndex, faceIndex, startIndex + int( i / 2 )} );
  }

  applyChanges( changes );

  return changes;
}


QgsTopologicalMesh::Changes QgsTopologicalMesh::addVertexInFace( int includingFaceIndex, const QgsMeshVertex &vertex )
{
  Changes changes;
  changes.mVerticesToAdd.append( vertex );
  changes.mVertexToFaceToAdd.append( -1 );

  mMesh->vertices.append( vertex );
  mVertexToFace.append( -1 );
  changes.mAddedFacesFirstIndex = mMesh->faceCount();

  const QgsMeshFace includingFace = mMesh->face( includingFaceIndex );
  const FaceNeighbors includingFaceNeighborhood = mFacesNeighborhood.at( includingFaceIndex );
  int includingFaceSize = includingFace.count();

  for ( int i = 0; i < includingFaceSize; ++i )
  {
    // add a new face
    QgsMeshFace face( 3 );
    face[0] = mMesh->vertexCount() - 1;
    face[1] = includingFace.at( i );
    face[2] = includingFace.at( ( i + 1 ) % includingFaceSize );
    mMesh->faces.append( face );
    changes.mFacesToAdd.append( face );

    int currentVertexIndex = includingFace.at( i );
    if ( mVertexToFace.at( currentVertexIndex ) == includingFaceIndex )
    {
      int newFaceIndex = mMesh->faceCount() - 1;
      mVertexToFace[currentVertexIndex] = newFaceIndex;
      changes.mVerticesToFaceChanges.append( {currentVertexIndex, includingFaceIndex, newFaceIndex} );
    }

    int includingFaceNeighbor = includingFaceNeighborhood.at( i );
    FaceNeighbors neighbors(
    {
      changes.mAddedFacesFirstIndex + ( i + includingFaceSize - 1 ) % includingFaceSize,
      includingFaceNeighbor,
      changes.mAddedFacesFirstIndex + ( i + includingFaceSize + 1 ) % includingFaceSize
    } );
    mFacesNeighborhood.append( neighbors );
    changes.mFacesNeighborhoodToAdd.append( neighbors );

    if ( includingFaceNeighbor != -1 )
    {
      int indexInNeighbor = vertexPositionInFace( *mMesh, includingFace.at( ( i + 1 ) % includingFaceSize ), includingFaceNeighbor );
      int oldValue = mFacesNeighborhood[includingFaceNeighbor][indexInNeighbor];
      mFacesNeighborhood[includingFaceNeighbor][indexInNeighbor] = changes.mAddedFacesFirstIndex + i;
      changes.mNeighborhoodChanges.append( {includingFaceNeighbor, indexInNeighbor, oldValue, changes.mAddedFacesFirstIndex + i} );
    }
  }

  changes.mFacesToRemove.append( includingFace );
  changes.mFaceIndexesToRemove.append( includingFaceIndex );
  changes.mFacesNeighborhoodToRemove.append( includingFaceNeighborhood );

  mFacesNeighborhood[includingFaceIndex] = FaceNeighbors();
  mMesh->faces[includingFaceIndex] = QgsMeshFace();
  mVertexToFace[mVertexToFace.count() - 1] = mMesh->faceCount() - 1;
  changes.mVertexToFaceToAdd[changes.mVertexToFaceToAdd.count() - 1] = mMesh->faceCount() - 1 ;

  return changes;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::insertVertexInFacesEdge( int faceIndex, int position, const QgsMeshVertex &vertexToInsert )
{
  const QgsMeshFace face1 = mMesh->face( faceIndex );

  Changes changes;
  changes.mVerticesToAdd.append( vertexToInsert );
  changes.mAddedFacesFirstIndex = mMesh->faceCount();

  // triangulate the first face
  int newVertexPositionInFace1 = position + 1;

  auto triangulate = [this, &changes]( int removedFaceIndex, const QgsMeshVertex & newVertex, int newVertexPosition, QVector<int> &edgeFacesIndexes )->bool
  {
    const QgsMeshFace &initialFace = mMesh->face( removedFaceIndex );
    changes.mFacesToRemove.append( initialFace );
    changes.mFaceIndexesToRemove.append( removedFaceIndex );
    changes.mFacesNeighborhoodToRemove.append( mFacesNeighborhood.at( removedFaceIndex ) );
    const int addedVertexIndex = mMesh->vertexCount();

    int faceStartGlobalIndex = mMesh->faceCount() + changes.mFacesToAdd.count();
    int localStartIndex = changes.mFacesToAdd.count();

    QVector<int> newBoundary = initialFace;
    newBoundary.insert( newVertexPosition, addedVertexIndex );

    try
    {
      QHash<p2t::Point *, int> mapPoly2TriPointToVertex;
      std::vector<p2t::Point *> faceToFill( newBoundary.count() );
      for ( int i = 0; i < newBoundary.count(); ++i )
      {
        QgsMeshVertex vert;

        if ( newBoundary.at( i ) == addedVertexIndex )
          vert = newVertex;
        else
          vert = mMesh->vertex( newBoundary.at( i ) );

        faceToFill[i] = new p2t::Point( vert.x(), vert.y() );
        mapPoly2TriPointToVertex.insert( faceToFill[i], newBoundary.at( i ) );
      }

      std::unique_ptr<p2t::CDT> cdt( new p2t::CDT( faceToFill ) );
      cdt->Triangulate();
      std::vector<p2t::Triangle *> triangles = cdt->GetTriangles();
      QVector<QgsMeshFace> newFaces( triangles.size() );
      for ( size_t i = 0; i < triangles.size(); ++i )
      {
        QgsMeshFace &face = newFaces[i];
        face.resize( 3 );
        for ( int j = 0; j < 3; j++ )
        {
          int vertInd = mapPoly2TriPointToVertex.value( triangles.at( i )->GetPoint( j ), -1 );
          if ( vertInd == -1 )
            throw std::exception();
          face[j] = vertInd;
        }
      }

      QgsMeshEditingError error;
      QgsTopologicalMesh::TopologicalFaces topologicalFaces = createNewTopologicalFaces( newFaces, false, error );
      if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
        throw std::exception();

      changes.mFacesToAdd.append( topologicalFaces.meshFaces() );
      changes.mFacesNeighborhoodToAdd.append( topologicalFaces.mFacesNeighborhood );

      // vertices to face changes
      const QList<int> &verticesToFaceToChange = topologicalFaces.mVerticesToFace.keys();
      for ( const int vtc : verticesToFaceToChange )
        if ( vtc != addedVertexIndex && mVertexToFace.at( vtc ) == removedFaceIndex )
          changes.mVerticesToFaceChanges.append(
        {
          vtc,
          removedFaceIndex,
          topologicalFaces.vertexToFace( vtc ) + faceStartGlobalIndex
        } );

      // reindex neighborhood for new faces
      for ( int i = 0; i < topologicalFaces.mFaces.count(); ++i )
      {
        FaceNeighbors &faceNeighbors = changes.mFacesNeighborhoodToAdd[i + localStartIndex];
        for ( int n = 0; n < faceNeighbors.count(); ++n )
        {
          if ( faceNeighbors.at( n ) != -1 )
            faceNeighbors[n] += faceStartGlobalIndex; //reindex internal neighborhood
        }
      }

      edgeFacesIndexes.resize( 2 );
      // link neighborhood for boundaries of each side
      for ( int i = 0 ; i < newBoundary.count(); ++i )
      {
        int vertexIndex = newBoundary.at( i );
        QgsMeshVertexCirculator circulator = QgsMeshVertexCirculator( topologicalFaces, vertexIndex );
        circulator.goBoundaryClockwise();
        int newFaceBoundaryLocalIndex = localStartIndex + circulator.currentFaceIndex();

        int newFaceBoundaryIndexInMesh = faceStartGlobalIndex;

        int meshFaceBoundaryIndex;
        if ( i == newVertexPosition )
        {
          meshFaceBoundaryIndex = -1; //face that are on the opposite side of the edge, filled later
          edgeFacesIndexes[0] =  newFaceBoundaryLocalIndex;
        }
        else if ( i == ( newVertexPosition + newBoundary.count() - 1 ) % newBoundary.count() )
        {
          meshFaceBoundaryIndex = -1; //face that are on the opposite side of the edge, filled later
          edgeFacesIndexes[1] =  newFaceBoundaryLocalIndex;
        }
        else
          meshFaceBoundaryIndex = mFacesNeighborhood.at( removedFaceIndex ).at( vertexPositionInFace( vertexIndex, initialFace ) );

        const QgsMeshFace &newFace = circulator.currentFace();
        int positionInNewFaces = vertexPositionInFace( vertexIndex, newFace );

        if ( meshFaceBoundaryIndex != -1 )
        {
          const QgsMeshFace meshFace = mMesh->face( meshFaceBoundaryIndex );
          int positionInMeshFaceBoundary = vertexPositionInFace( *mMesh, vertexIndex, meshFaceBoundaryIndex );
          positionInMeshFaceBoundary = ( positionInMeshFaceBoundary - 1 + meshFace.count() ) % meshFace.count(); //take the position just before

          changes.mNeighborhoodChanges.append( {meshFaceBoundaryIndex,
                                                positionInMeshFaceBoundary,
                                                removedFaceIndex,
                                                newFaceBoundaryIndexInMesh +
                                                circulator.currentFaceIndex()} );
        }

        changes.mFacesNeighborhoodToAdd[newFaceBoundaryLocalIndex][positionInNewFaces] = meshFaceBoundaryIndex;
      }

      qDeleteAll( faceToFill );
    }
    catch ( ... )
    {
      return false;
    }

    return true;
  };

  QVector<int> edgeFacesIndexes;
  if ( !triangulate( faceIndex, vertexToInsert, newVertexPositionInFace1, edgeFacesIndexes ) )
    return Changes();

  changes.mVertexToFaceToAdd.append( edgeFacesIndexes.at( 0 ) + changes.mAddedFacesFirstIndex );

  int addedVertexIndex = mMesh->vertexCount();

  //if exist, triangulate the second face if exists
  int face2Index = mFacesNeighborhood.at( faceIndex ).at( position );
  if ( face2Index != -1 )
  {
    const QgsMeshFace &face2 = mMesh->face( face2Index );
    int vertexPositionInFace2 = vertexPositionInFace( face1.at( position ), face2 );
    QVector<int> edgeFacesIndexesFace2;
    if ( !triangulate( face2Index, vertexToInsert, vertexPositionInFace2, edgeFacesIndexesFace2 ) )
      return Changes();

    //link neighborhood with other side
    const QgsMeshFace &firstFaceSide1 = changes.mFacesToAdd.at( edgeFacesIndexes.at( 0 ) );
    int pos1InFaceSide1 = vertexPositionInFace( addedVertexIndex, firstFaceSide1 );

    const QgsMeshFace &secondFaceSide1 = changes.mFacesToAdd.at( edgeFacesIndexes.at( 1 ) );
    int pos2InFaceSide1 = vertexPositionInFace( addedVertexIndex, secondFaceSide1 );
    pos2InFaceSide1 = ( pos2InFaceSide1 + secondFaceSide1.size() - 1 ) % secondFaceSide1.size();

    const QgsMeshFace &firstFaceSide2 = changes.mFacesToAdd.at( edgeFacesIndexesFace2.at( 0 ) );
    int pos1InFaceSide2 = vertexPositionInFace( addedVertexIndex, firstFaceSide2 );

    const QgsMeshFace &secondFaceSide2 = changes.mFacesToAdd.at( edgeFacesIndexesFace2.at( 1 ) );
    int pos2InFaceSide2 = vertexPositionInFace( addedVertexIndex, secondFaceSide2 );
    pos2InFaceSide2 = ( pos2InFaceSide2 + secondFaceSide1.size() - 1 ) % secondFaceSide1.size();

    changes.mFacesNeighborhoodToAdd[edgeFacesIndexes.at( 0 )][pos1InFaceSide1] = edgeFacesIndexesFace2.at( 1 ) + changes.mAddedFacesFirstIndex;
    changes.mFacesNeighborhoodToAdd[edgeFacesIndexes.at( 1 )][pos2InFaceSide1] = edgeFacesIndexesFace2.at( 0 ) + changes.mAddedFacesFirstIndex;
    changes.mFacesNeighborhoodToAdd[edgeFacesIndexesFace2.at( 0 )][pos1InFaceSide2] = edgeFacesIndexes.at( 1 ) + changes.mAddedFacesFirstIndex;
    changes.mFacesNeighborhoodToAdd[edgeFacesIndexesFace2.at( 1 )][pos2InFaceSide2] = edgeFacesIndexes.at( 0 ) + changes.mAddedFacesFirstIndex;
  }

  applyChanges( changes );
  return changes;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::changeZValue( const QList<int> &verticesIndexes, const QList<double> &newValues )
{
  Q_ASSERT( verticesIndexes.count() == newValues.count() );
  Changes changes;
  changes.mChangeCoordinateVerticesIndexes.reserve( verticesIndexes.count() );
  changes.mNewZValues.reserve( verticesIndexes.count() );
  changes.mOldZValues.reserve( verticesIndexes.count() );
  for ( int i = 0; i < verticesIndexes.count(); ++i )
  {
    changes.mChangeCoordinateVerticesIndexes.append( verticesIndexes.at( i ) );
    changes.mOldZValues.append( mMesh->vertices.at( verticesIndexes.at( i ) ).z() );
    changes.mNewZValues.append( newValues.at( i ) );
  }

  applyChanges( changes );

  return changes;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::changeXYValue( const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues )
{
  Q_ASSERT( verticesIndexes.count() == newValues.count() );
  Changes changes;
  changes.mChangeCoordinateVerticesIndexes.reserve( verticesIndexes.count() );
  changes.mNewXYValues.reserve( verticesIndexes.count() );
  changes.mOldXYValues.reserve( verticesIndexes.count() );
  QSet<int> concernedFace;
  for ( int i = 0; i < verticesIndexes.count(); ++i )
  {
    changes.mChangeCoordinateVerticesIndexes.append( verticesIndexes.at( i ) );
    changes.mOldXYValues.append( mMesh->vertices.at( verticesIndexes.at( i ) ) );
    changes.mNewXYValues.append( newValues.at( i ) );
    concernedFace.unite( qgis::listToSet( facesAroundVertex( verticesIndexes.at( i ) ) ) );
  }

  changes.mNativeFacesIndexesGeometryChanged = concernedFace.values();

  applyChanges( changes );

  return changes;
}
