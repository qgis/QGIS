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
#include "qgstopologicalmesh.h"
#include "qgsmesheditor.h"
#include <qgsmessagelog.h>

#include <poly2tri.h>
#include <QSet>

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

QgsMeshVertexCirculator::QgsMeshVertexCirculator( const QgsTopologicalMesh &topologicalMesh, int vertexIndex ):
  mFaces( topologicalMesh.mMesh->faces )
  , mFacesNeighborhood( topologicalMesh.mFacesNeighborhood )
  ,  mVertexIndex( vertexIndex )
{
  if ( vertexIndex >= 0 && vertexIndex < topologicalMesh.mMesh->vertexCount() )
  {
    mCurrentFace = topologicalMesh.mVertexToface[vertexIndex];
    mIsValid = vertexPositionInFace( *topologicalMesh.mesh(), vertexIndex, mCurrentFace ) != -1;
  }
  else
  {
    mIsValid = false;
  }

  if ( mIsValid )
    mLastValidFace = mCurrentFace;
}

QgsMeshVertexCirculator::QgsMeshVertexCirculator( const QgsTopologicalMesh::TopologicalFaces &topologicalFaces, int faceIndex, int vertexIndex ):
  mFaces( topologicalFaces.mFaces )
  , mFacesNeighborhood( topologicalFaces.mFacesNeighborhood )
  , mVertexIndex( vertexIndex )
{
  const QgsMeshFace &face = topologicalFaces.mFaces.at( faceIndex );
  mIsValid = vertexPositionInFace( vertexIndex, face ) != -1;

  mCurrentFace = faceIndex;
  mLastValidFace = mCurrentFace;
}

QgsMeshVertexCirculator::QgsMeshVertexCirculator( const QgsTopologicalMesh::TopologicalFaces &topologicalFaces, int vertexIndex ): mFaces( topologicalFaces.mFaces )
  , mFacesNeighborhood( topologicalFaces.mFacesNeighborhood )
  , mVertexIndex( vertexIndex )
{

  mCurrentFace = topologicalFaces.mVerticesToFace.value( vertexIndex, -1 );
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
    if ( currentPos == -1 )
      return -1;

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
    if ( currentPos == -1 )
      return -1;

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

int QgsMeshVertexCirculator::oppositeVertexClockWise() const
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

int QgsMeshVertexCirculator::oppositeVertexCounterClockWise() const
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
    //if the boundary id a free vertex in the destination mesh, no need to check
    if ( mVertexToface.at( boundary ) == -1 )
      continue;

    int indexOfStartinFace = topologicalFaces.mVerticesToFace.value( boundary );

    QgsMeshVertexCirculator newFacesCirculator( topologicalFaces, indexOfStartinFace, boundary );
    //search for face boundary on clockwise side of new faces
    newFacesCirculator.goBoundaryClockwise();
    int oppositeVertexForNewFace = newFacesCirculator.oppositeVertexClockWise();
    if ( mVertexToface.at( oppositeVertexForNewFace ) == -1 )
      continue;

    QgsMeshVertexCirculator meshCirculator = vertexCirculator( boundary );
    meshCirculator.goBoundaryCounterClockwise();
    int oppositeVertexForMeshFace = meshCirculator.oppositeVertexCounterClockWise();

    const QgsMeshFace &newFaceBoundary = newFacesCirculator.currentFace();
    int boundaryPositonInNewFace = vertexPositionInFace( boundary, newFaceBoundary );

    if ( oppositeVertexForMeshFace != oppositeVertexForNewFace )
    {
      changes.mFacesNeighborhoodToAdd[newFacesCirculator.currentFaceIndex()][boundaryPositonInNewFace] = -1 ;
    }
    else
    {
      const QgsMeshFace &meshFaceBoundary = meshCirculator.currentFace();
      int boundaryPositionInMeshFace = vertexPositionInFace( meshCirculator.oppositeVertexCounterClockWise(), meshFaceBoundary );

      changes.mNeighborhoodChanges.append( std::array<int, 4>(
      {
        meshCirculator.currentFaceIndex(),
        boundaryPositionInMeshFace,
        -1,
        changes.addedFaceIndexInMesh( newFacesCirculator.currentFaceIndex() )
      } ) );

      changes.mFacesNeighborhoodToAdd[newFacesCirculator.currentFaceIndex()][boundaryPositonInNewFace] = meshCirculator.currentFaceIndex();
    }
  }

  for ( int f = 0; f < changes.mFacesToAdd.count(); ++f )
    for ( int n = 0; n < changes.mFacesToAdd.at( f ).count(); ++n )
      if ( changes.mFacesNeighborhoodToAdd.at( f ).at( n ) == -1 )
        changes.mFacesNeighborhoodToAdd[f][n] = changes.addedFaceIndexInMesh( topologicalFaces.mFacesNeighborhood.at( f ).at( n ) );

  const QList<int> &verticesToFaceToChange = topologicalFaces.mVerticesToFace.keys();
  for ( const int vtc : verticesToFaceToChange )
    if ( mVertexToface.at( vtc ) == -1 )
      changes.mVerticesToFaceChanges.append( {vtc,
                                              mVertexToface.at( vtc ),
                                              changes.addedFaceIndexInMesh( topologicalFaces.mVerticesToFace.value( vtc ) ) } );

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
    mVertexToface.resize( newSize );
  }

  if ( !changes.mFacesToAdd.empty() > 0 )
  {
    int newSize = mMesh->faceCount() + changes.mFacesToAdd.count();
    mMesh->faces.resize( newSize );
    mFacesNeighborhood.resize( newSize );
  }

  for ( int i = 0; i < changes.mFacesToRemove.count(); ++i )
  {
    mMesh->faces[changes.removedFaceIndexInmesh( i )] = QgsMeshFace();
    mFacesNeighborhood[changes.removedFaceIndexInmesh( i )] = FaceNeighbors();//changes.facesNeighborhoodToRemove[i];
  }

  for ( int i = 0; i < changes.mVerticesToRemoveIndexes.count(); ++i )
  {
    int vertexIndex = changes.mVerticesToRemoveIndexes.at( i );
    mMesh->vertices[vertexIndex] = QgsMeshVertex();
    mVertexToface[vertexIndex] = -1;
  }

  for ( int i = 0; i < changes.mVerticesToAdd.count(); ++i )
  {
    mMesh->vertices[initialVerticesCount + i] = changes.mVerticesToAdd.at( i );
    mVertexToface[initialVerticesCount + i] = changes.mVertexToFaceToAdd.at( i );
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
    mVertexToface[vertexToFaceChange.at( 0 )] = vertexToFaceChange.at( 2 );
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
    mMesh->faces[changes.removedFaceIndexInmesh( i )] = changes.mFacesToRemove.at( i );
    mFacesNeighborhood[changes.removedFaceIndexInmesh( i )] = changes.mFacesNeighborhoodToRemove.at( i );
  }

  for ( int i = 0; i < changes.mVerticesToRemoveIndexes.count(); ++i )
  {
    int vertexIndex = changes.mVerticesToRemoveIndexes.at( i );
    mMesh->vertices[vertexIndex] = changes.mRemovedVertices.at( i );
    mVertexToface[vertexIndex] = changes.mVerticesToFaceRemoved.at( i );
  }

  int verticesToFaceChangesCount = changes.mVerticesToFaceChanges.count();
  for ( int i = 0; i < verticesToFaceChangesCount; ++i )
  {
    const std::array<int, 3> vertexToFaceChange = changes.mVerticesToFaceChanges.at( verticesToFaceChangesCount - i - 1 );
    mVertexToface[vertexToFaceChange.at( 0 )] = vertexToFaceChange.at( 1 );
  }

  if ( !changes.mFacesToAdd.empty() > 0 )
  {
    int newSize = mMesh->faceCount() - changes.mFacesToAdd.count();
    mMesh->faces.resize( newSize );
    mFacesNeighborhood.resize( newSize );
  }

  if ( !changes.mVerticesToAdd.isEmpty() )
  {
    int newSize = mMesh->vertexCount() - changes.mVerticesToAdd.count();
    mMesh->vertices.resize( newSize );
    mVertexToface.resize( newSize );
  }
}

QgsMeshVertexCirculator QgsTopologicalMesh::vertexCirculator( int vertexIndex ) const
{
  return QgsMeshVertexCirculator( *this, vertexIndex );
}

bool QgsTopologicalMesh::facesCanBeJoinedWithCommonIndex( const QgsMeshFace &face1, const QgsMeshFace &face2, int commonIndex )
{
  int commonVertexPosition1 = vertexPositionInFace( commonIndex, face1 );
  int commonVertexPosition2 = vertexPositionInFace( commonIndex, face2 );

  bool canBejoined = ( face1.at( ( commonVertexPosition1 + 1 ) % face1.size() ) == face2.at( ( commonVertexPosition2 - 1 + face2.size() ) % face2.size() ) ) ||
                     ( face1.at( ( commonVertexPosition1 - 1 + face1.size() ) % face1.size() ) == face2.at( ( commonVertexPosition2 + 1 ) % face2.size() ) );


  return canBejoined;
}

QSet<int> QgsTopologicalMesh::concernedFacesBy( const QList<int> faceIndexes ) const
{
  QSet<int> faces;
  for ( const int faceIndex : faceIndexes )
  {
    const QgsMeshFace &face = mMesh->face( faceIndex );
    for ( int i = 0; i < face.count(); ++i )
      faces.unite( facesAroundVertex( face.at( i ) ).toSet() );
  }
  return faces;
}

QgsMesh *QgsTopologicalMesh::mesh() const
{
  return mMesh;
}

QgsMeshEditingError QgsTopologicalMesh::counterClockWiseFaces( QgsMeshFace &face, QgsMesh *mesh )
{
  // First check if the face is convex and put it counter clockwise
  // If the index are not well ordered (edges intersect), invalid face --> return false
  int faceSize = face.count();
  if ( faceSize < 3 )
    return QgsMeshEditingError( QgsMeshEditingError::FlatFace, -1 );

  int direction = 0;
  for ( int i = 0; i < faceSize; ++i )
  {
    int iv0 =  face[i];
    int iv1 = face[( i + 1 ) % faceSize];
    int iv2 = face[( i + 2 ) % faceSize];

    if ( iv0 < 0 || iv0 >= mesh->vertexCount() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, iv0 );

    if ( iv1 < 0 || iv1 >= mesh->vertexCount() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, iv1 );

    if ( iv2 < 0 || iv2 >= mesh->vertexCount() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, iv2 );

    const QgsMeshVertex &v0 = mesh->vertices.at( iv0 ) ;
    const QgsMeshVertex &v1 = mesh->vertices.at( iv1 ) ;
    const QgsMeshVertex &v2 = mesh->vertices.at( iv2 ) ;

    if ( v0.isEmpty() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, iv0 );

    if ( v1.isEmpty() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, iv1 );

    if ( v2.isEmpty() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, iv2 );

    double ux = v0.x() - v1.x();
    double uy = v0.y() - v1.y();
    double vx = v2.x() - v1.x();
    double vy = v2.y() - v1.y();

    double crossProduct = ux * vy - uy * vx; //if cross product>0, we have two edges clockwise
    if ( direction != 0 && crossProduct * direction < 0 )   // We have a convex face or a (partialy) flat face
      return QgsMeshEditingError( QgsMeshEditingError::InvalidFace, -1 );
    else if ( crossProduct == 0 )
      return QgsMeshEditingError( QgsMeshEditingError::FlatFace, -1 );
    else if ( direction == 0 && crossProduct != 0 )
      direction = crossProduct / std::fabs( crossProduct );
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

  return QgsMeshEditingError( QgsMeshEditingError::NoError, -1 );
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

  mVertexToface.clear();
  mFacesNeighborhood.clear();
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

int QgsTopologicalMesh::Changes::addedFaceIndexInMesh( int internalIndex ) const
{
  if ( internalIndex == -1 )
    return -1;

  return internalIndex + mAddedFacesFirstIndex;
}

int QgsTopologicalMesh::Changes::removedFaceIndexInmesh( int internalIndex ) const
{
  if ( internalIndex == -1 )
    return -1;

  return mFaceIndexesToRemove.at( internalIndex );
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::addFreeVertex( const QgsMeshVertex &vertex )
{
  Changes changes;
  changes.mVerticesToAdd.append( vertex );
  changes.mVertexToFaceToAdd.append( -1 );

  mMesh->vertices.append( vertex );
  mVertexToface.append( -1 );

  return changes;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::removeVertex( int vertexIndex,  bool fillHole )
{
  if ( mVertexToface.at( vertexIndex ) == -1 )
  {
    Changes changes;
    changes.mRemovedVertices.append( mMesh->vertices.at( vertexIndex ) );
    changes.mVerticesToRemoveIndexes.append( vertexIndex );
    changes.mVerticesToFaceRemoved.append( -1 );
    mMesh->vertices[vertexIndex] = QgsMeshVertex();
    return changes;
  }

  //search concerned faces
  QgsMeshVertexCirculator circulator = vertexCirculator( vertexIndex );
  circulator.goBoundaryClockwise();
  QList<int> boundariesVertexIndex;
  QList<int> associateFaceToBoundaries;
  QList<int> removedFacesindexes;
  //removedFacesindexes.append( circulator.currentFaceIndex() );
  //boundariesVertexIndex.append( circulator.oppositeVertexClockWise() );
  do
    //while ( )
  {
    removedFacesindexes.append( circulator.currentFaceIndex() );
    boundariesVertexIndex.append( circulator.oppositeVertexClockWise() );
    const QgsMeshFace &currentFace = circulator.currentFace();
    associateFaceToBoundaries.append( mFacesNeighborhood.at( circulator.currentFaceIndex() ).at(
                                        vertexPositionInFace( boundariesVertexIndex.last(), currentFace ) ) );

    if ( currentFace.count() > 3 ) // quad or more, need other vertices
    {
      int posInface = vertexPositionInFace( vertexIndex, currentFace );
      for ( int i = 2; i < currentFace.count() - 1; ++i )
      {
        boundariesVertexIndex.append( currentFace.at( ( posInface + i ) % currentFace.count() ) );
        associateFaceToBoundaries.append( mFacesNeighborhood.at( circulator.currentFaceIndex() ).at(
                                            vertexPositionInFace( boundariesVertexIndex.last(), currentFace ) ) );
      }
    }
  }
  while ( circulator.turnCounterClockwise() != -1 && circulator.currentFaceIndex() != removedFacesindexes.first() );

  if ( circulator.currentFaceIndex() == -1 ) //we are on boundary, for now do not fill hole
    fillHole = false;

  //Remove duplicated boundaries
  for ( int i = 0; i < boundariesVertexIndex.count(); ++i )
  {
    int otherPosition = boundariesVertexIndex.indexOf( boundariesVertexIndex.at( i ), i + 1 );
    if ( otherPosition != -1 )
    {
      boundariesVertexIndex.removeAt( otherPosition );
      associateFaceToBoundaries.removeAt( otherPosition );
    }
  }

  int currentVertexToFace = mVertexToface.at( vertexIndex );
  Changes changes = removeFaces( removedFacesindexes );
  changes.mRemovedVertices.append( mMesh->vertices.at( vertexIndex ) );
  changes.mVerticesToRemoveIndexes.append( vertexIndex );
  changes.mVerticesToFaceRemoved.append( currentVertexToFace );

  mMesh->vertices[vertexIndex] = QgsMeshVertex();
  mVertexToface[vertexIndex] = -1;

  if ( fillHole )
  {
    QHash<p2t::Point *, int> mapPoly2TriPointToVertex;
    std::vector<p2t::Point *> holeToFill( boundariesVertexIndex.count() );
    for ( int i = 0; i < boundariesVertexIndex.count(); ++i )
    {
      const QgsMeshVertex &vertex = mMesh->vertex( boundariesVertexIndex.at( i ) );
      holeToFill[i] = new p2t::Point( vertex.x(), vertex.y() );
      mapPoly2TriPointToVertex.insert( holeToFill[i], boundariesVertexIndex.at( i ) );
    }

    std::unique_ptr<p2t::CDT> cdt( new p2t::CDT( holeToFill ) );

    try
    {
      cdt->Triangulate();
      std::vector<p2t::Triangle *> triangles = cdt->GetTriangles();
      QVector<QgsMeshFace> newFaces( triangles.size() );
      for ( size_t i = 0; i < triangles.size(); ++i )
      {
        QgsMeshFace &face = newFaces[i];
        face.resize( 3 );
        for ( int j = 0; j < 3; j++ )
        {
          int vertexIndex = mapPoly2TriPointToVertex.value( triangles.at( i )->GetPoint( j ), -1 );
          if ( vertexIndex == -1 )
            throw std::exception();
          face[j] = vertexIndex;
        }
      }

      QgsMeshEditingError error;
      QgsTopologicalMesh::TopologicalFaces topologicalFaces = createNewTopologicalFaces( newFaces, error );
      if ( error.errorType != QgsMeshEditingError::NoError )
        throw std::exception();
      int newFaceIndexStartIndex = mMesh->faceCount();
      QgsTopologicalMesh::Changes addChanges;
      addChanges.mAddedFacesFirstIndex = newFaceIndexStartIndex;
      addChanges.mFacesToAdd = topologicalFaces.meshFaces();
      addChanges.mFacesNeighborhoodToAdd = topologicalFaces.mFacesNeighborhood;

      // vertices to face changes
      const QList<int> &verticesToFaceToChange = topologicalFaces.mVerticesToFace.keys();
      for ( const int vtc : verticesToFaceToChange )
        if ( mVertexToface.at( vtc ) == -1 )
          addChanges.mVerticesToFaceChanges.append( {vtc,
              mVertexToface.at( vtc ),
              addChanges.addedFaceIndexInMesh( topologicalFaces.mVerticesToFace.value( vtc ) ) } );


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
      for ( int i = 0; i < boundariesVertexIndex.count(); ++i )
      {
        int vertexBoundaryIndex = boundariesVertexIndex.at( i );

        int meshFaceBoundaryIndex = associateFaceToBoundaries.at( i );

        QgsMeshVertexCirculator circulator = QgsMeshVertexCirculator( topologicalFaces, vertexBoundaryIndex );
        circulator.goBoundaryClockwise();
        int newFaceBoundaryLocalIndex = circulator.currentFaceIndex();
        int newFaceBoundaryIndexInMesh = circulator.currentFaceIndex() + newFaceIndexStartIndex;
        const QgsMeshFace &newFace = circulator.currentFace();
        int positionInNewFaces = vertexPositionInFace( vertexBoundaryIndex, newFace );

        if ( meshFaceBoundaryIndex != -1 )
        {
          const QgsMeshFace meshFace = mMesh->face( meshFaceBoundaryIndex );
          int positionInMeshFaceBoundary = vertexPositionInFace( *mMesh, vertexBoundaryIndex, meshFaceBoundaryIndex );
          positionInMeshFaceBoundary = ( positionInMeshFaceBoundary - 1 + meshFace.count() ) % meshFace.count(); //take the position just before

          addChanges.mNeighborhoodChanges.append( {meshFaceBoundaryIndex, positionInMeshFaceBoundary, -1, newFaceBoundaryIndexInMesh} );
        }

        addChanges.mFacesNeighborhoodToAdd[newFaceBoundaryLocalIndex][positionInNewFaces] = meshFaceBoundaryIndex;

      }

      applyChanges( addChanges );

      changes.mFacesToAdd.append( addChanges.mFacesToAdd );
      changes.mAddedFacesFirstIndex = addChanges.mAddedFacesFirstIndex;
      changes.mFacesNeighborhoodToAdd.append( addChanges.mFacesNeighborhoodToAdd );
      changes.mNeighborhoodChanges.append( addChanges.mNeighborhoodChanges );
      changes.mVerticesToFaceChanges.append( addChanges.mVerticesToFaceChanges );
    }
    catch ( ... )
    {
      QgsMessageLog::logMessage( QObject::tr( "Triangulation failed. Skipping hole…" ), QObject::tr( "Mesh Editing" ) );
    }

    qDeleteAll( holeToFill );
  }

  return changes;

}

QgsMeshEditingError QgsTopologicalMesh::canFacesBeAdded( const QgsTopologicalMesh::TopologicalFaces &topologicFaces ) const
{
  QList<int> boundaryToCheck = topologicFaces.mBoundaries;
  // go throught the boundary to check if there is a unique shared vertex

  while ( !boundaryToCheck.isEmpty() )
  {
    int boundary = boundaryToCheck.takeLast();

    //if the boundary id a free vertex in the destination mesh, no need to check
    if ( mVertexToface.at( boundary ) == -1 )
      continue;

    int faceIndex = topologicFaces.mVerticesToFace.value( boundary );

    QgsMeshVertexCirculator newFacescirculator( topologicFaces, faceIndex, boundary );
    QgsMeshVertexCirculator meshCirculator = vertexCirculator( boundary );

    if ( !newFacescirculator.isValid() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidFace, faceIndex );

    //search for face boundary on clockwise side of new faces
    if ( !newFacescirculator.goBoundaryClockwise() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidFace, faceIndex );
    const QgsMeshFace &newFaceBoundaryCW = newFacescirculator.currentFace();
    if ( newFaceBoundaryCW.isEmpty() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidFace, faceIndex );

    //search for face boundary on COUNTER clockwise side of existing faces
    if ( !meshCirculator.goBoundaryCounterClockwise() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, boundary );
    const QgsMeshFace &existingFaceBoundaryCCW = meshCirculator.currentFace();
    if ( existingFaceBoundaryCCW.isEmpty() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, boundary );

    if ( facesCanBeJoinedWithCommonIndex( newFaceBoundaryCW, existingFaceBoundaryCCW, boundary ) )
    {
      // Face can be joined
      // Remove other common vertex before continuing
      boundaryToCheck.removeOne( newFacescirculator.oppositeVertexClockWise() );
      continue;
    }

    // Now check with other direction

    //search for face boundary on COUNTER clockwise side of new faces
    if ( !newFacescirculator.goBoundaryCounterClockwise() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidFace, faceIndex );
    const QgsMeshFace &newFaceBoundaryCCW = newFacescirculator.currentFace();
    if ( newFaceBoundaryCCW.isEmpty() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidFace, faceIndex );

    //search for face boundary on clockwise side of existing faces
    if ( !meshCirculator.goBoundaryClockwise() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, boundary );
    const QgsMeshFace &existingFaceBoundaryCW = meshCirculator.currentFace();
    if ( existingFaceBoundaryCW.isEmpty() )
      return QgsMeshEditingError( QgsMeshEditingError::InvalidVertex, boundary );

    if ( facesCanBeJoinedWithCommonIndex( newFaceBoundaryCCW, existingFaceBoundaryCW, boundary ) )
    {
      // Face can be joined
      // Remove other common vertex before continuing
      boundaryToCheck.removeOne( newFacescirculator.oppositeVertexCounterClockWise() );
      continue;
    }

    //if we are here, face share only one vertices
    return QgsMeshEditingError( QgsMeshEditingError::UniqueSharedVertex, boundary );
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

QgsTopologicalMesh QgsTopologicalMesh::createTopologicalMesh( QgsMesh *mesh, QgsMeshEditingError &error )
{
  QgsTopologicalMesh topologicMesh;
  topologicMesh.mMesh = mesh;
  topologicMesh.mVertexToface = QVector<int>( mesh->vertexCount(), -1 );
  error.errorType = QgsMeshEditingError::NoError;

  for ( int i = 0; i < mesh->faceCount(); ++i )
  {
    error = counterClockWiseFaces( mesh->faces[i], mesh );
    if ( error.errorType != QgsMeshEditingError::NoError )
    {
      if ( error.errorType == QgsMeshEditingError::InvalidFace || error.errorType == QgsMeshEditingError::FlatFace )
        error.elementIndex = i;
      break;
    }
  }

  if ( error.errorType == QgsMeshEditingError::NoError )
  {
    TopologicalFaces subMesh = topologicMesh.createTopologicalFaces( mesh->faces, error, false, true );
    topologicMesh.mFacesNeighborhood = subMesh.mFacesNeighborhood;
  }

  return topologicMesh;
}

QgsTopologicalMesh::TopologicalFaces QgsTopologicalMesh::createNewTopologicalFaces( const QVector<QgsMeshFace> &faces, QgsMeshEditingError &error )
{
  return createTopologicalFaces( faces, error, true, false );
}


QgsTopologicalMesh::TopologicalFaces QgsTopologicalMesh::createTopologicalFaces(
  const QVector<QgsMeshFace> &faces,
  QgsMeshEditingError &error,
  bool allowUniqueSharedVertex,
  bool writeInVertices )
{
  int facesCount = faces.count();
  QVector<FaceNeighbors> faceTopologies;
  QHash<int, int> verticesToFace;

  // Contains for each vertex a map (opposite vertex # edge) --> face index
  // when turning counter clockwise if, v1 first vertex and v2 second one, [v1][v2]--> neighbor face
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
      verticesToNeighbor[v2].insert( v1, faceIndex );
    }
  }

  faceTopologies = QVector<FaceNeighbors>( faces.count() );

  QSet<int> boundaryVertices;
  TopologicalFaces ret;

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

      if ( writeInVertices )
      {
        if ( mVertexToface[v1] == -1 )
          mVertexToface[v1] = faceIndex ;
      }
      else
      {
        if ( !verticesToFace.contains( v1 ) )
          verticesToFace[v1] = faceIndex ;
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
            error = QgsMeshEditingError( QgsMeshEditingError::UniqueSharedVertex, v1 ); // if a vertices is more than one time in the boudary, that means faces share only one vertices
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

QList<int> QgsTopologicalMesh::neighborsOfFace( int faceIndex ) const
{
  return mFacesNeighborhood.at( faceIndex ).toList();
}

QList<int> QgsTopologicalMesh::facesAroundVertex( int vertexIndex ) const
{
  QgsMeshVertexCirculator circ = vertexCirculator( vertexIndex );

  QList<int> ret;
  if ( !circ.isValid() || circ.currentFaceIndex() == -1 )
    return ret;

  // try ccw first
  ret.append( circ.currentFaceIndex() );
  while ( circ.turnCounterClockwise() != ret.first() && circ.currentFaceIndex() != -1 )
  {
    ret.append( circ.currentFaceIndex() );
  }

  if ( circ.currentFaceIndex() == -1 )  //we encounter a boundary, restart with other direction
  {
    ret.clear();
    if ( circ.turnClockwise() == -1 )
      return ret;
    ret.append( circ.currentFaceIndex() );
    while ( circ.turnClockwise() != -1 )
    {
      ret.append( circ.currentFaceIndex() );
    }
  }

  return ret;
}

QgsMeshEditingError QgsTopologicalMesh::canFacesBeRemoved( const QList<int> facesIndexes )
{
  QSet<int> removedFaces = facesIndexes.toSet();
  QSet<int> concernedFaces = concernedFacesBy( facesIndexes );

  for ( const int f : std::as_const( removedFaces ) )
    concernedFaces.remove( f );

  QVector<QgsMeshFace> remainingFaces;
  remainingFaces.reserve( concernedFaces.count() );
  for ( const int f : std::as_const( concernedFaces ) )
    remainingFaces.append( mMesh->face( f ) );

  QgsMeshEditingError error;
  createTopologicalFaces( remainingFaces, error, false, false );

  return error;
}

QgsTopologicalMesh::Changes QgsTopologicalMesh::removeFaces( const QList<int> facesIndexes )
{
  Changes changes;
  changes.mFaceIndexesToRemove = facesIndexes;
  changes.mFacesToRemove.resize( facesIndexes.count() );
  changes.mFacesNeighborhoodToRemove.resize( facesIndexes.count() );

  QSet<int> indexSet = facesIndexes.toSet();
  QSet<int> threatedVertex;

  for ( int i = 0; i < facesIndexes.count(); ++i )
  {
    const int faceIndex = facesIndexes.at( i );
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
      if ( !threatedVertex.contains( vertexIndex ) && indexSet.contains( mVertexToface.at( vertexIndex ) ) )
      {
        int oldValue = mVertexToface.at( vertexIndex );
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

QgsTopologicalMesh::Changes QgsTopologicalMesh::addVertexInface( int includingFaceIndex, const QgsMeshVertex &vertex )
{
  Changes changes = addFreeVertex( vertex );
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
    if ( mVertexToface.at( currentVertexIndex ) == includingFaceIndex )
    {
      int newFaceIndex = mMesh->faceCount() - 1;
      mVertexToface[currentVertexIndex] = newFaceIndex;
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
  mVertexToface[mVertexToface.count() - 1] = mMesh->faceCount() - 1;
  changes.mVertexToFaceToAdd[changes.mVertexToFaceToAdd.count() - 1] = mMesh->faceCount() - 1 ;

  return changes;
}

