/***************************************************************************
  qgsmeshadvancedediting.cpp - QgsMeshAdvancedEditing

 ---------------------
 begin                : 9.7.2021
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
#include "qgsmeshadvancedediting.h"

#include "qgis.h"
#include "qgsmesheditor.h"
#include "poly2tri.h"

#include "qgsmeshlayer.h"
#include "qgsexpression.h"
#include "qgsexpressioncontextutils.h"


QgsMeshAdvancedEditing::QgsMeshAdvancedEditing() = default;

QgsMeshAdvancedEditing::~QgsMeshAdvancedEditing() = default;

void QgsMeshAdvancedEditing::setInputVertices( const QList<int> verticesIndexes )
{
  mInputVertices = verticesIndexes;
}

void QgsMeshAdvancedEditing::setInputFaces( const QList<int> faceIndexes )
{
  mInputFaces = faceIndexes;
}

QString QgsMeshAdvancedEditing::message() const
{
  return mMessage;
}

void QgsMeshAdvancedEditing::clear()
{
  mInputVertices.clear();
  mInputFaces.clear();
  mMessage.clear();
  mIsFinished = false;
  clearChanges();
}

bool QgsMeshAdvancedEditing::isFinished() const
{
  return mIsFinished;
}

QString QgsMeshAdvancedEditing::text() const
{
  return QString();
}

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

QgsMeshEditRefineFaces::QgsMeshEditRefineFaces() = default;

QgsTopologicalMesh::Changes QgsMeshEditRefineFaces::apply( QgsMeshEditor *meshEditor )
{
  QSet<int> facesToRefine = qgis::listToSet( mInputFaces );
  QHash<int, FaceRefinement> facesRefinement;
  QHash<int, BorderFace> borderFaces;

  createNewVerticesAndRefinedFaces( meshEditor, facesToRefine, facesRefinement );
  if ( !createNewBorderFaces( meshEditor, facesToRefine, facesRefinement, borderFaces ) )
    return QgsTopologicalMesh::Changes();

  // create new vertices
  const QgsMesh &nativeMesh =  *meshEditor->topologicalMesh().mesh();
  mAddedFacesFirstIndex = nativeMesh.faceCount();

  mFaceIndexesToRemove = facesRefinement.keys();
  mFaceIndexesToRemove.append( borderFaces.keys() );
  mFacesToRemove.resize( mFaceIndexesToRemove.size() );
  mFacesNeighborhoodToRemove.resize( mFaceIndexesToRemove.size() );
  for ( int i = 0; i < mFaceIndexesToRemove.count(); ++i )
  {
    int  faceIndexToRemove = mFaceIndexesToRemove.at( i );
    mFacesToRemove[i] = nativeMesh.face( faceIndexToRemove );
    mFacesNeighborhoodToRemove[i] = meshEditor->topologicalMesh().neighborsOfFace( faceIndexToRemove );
  }

  meshEditor->topologicalMesh().applyChanges( *this );

  mIsFinished = true;

  return *this;
}

void QgsMeshEditRefineFaces::createNewVerticesAndRefinedFaces( QgsMeshEditor *meshEditor,
    QSet<int> &facesToRefine,
    QHash<int, FaceRefinement> &facesRefinement )
{
  const QgsTopologicalMesh &topology = meshEditor->topologicalMesh();
  const QgsMesh &mesh = *meshEditor->topologicalMesh().mesh();

  int startingVertexIndex = mesh.vertexCount();
  int startingGlobalFaceIndex = mesh.faceCount();

  for ( const int faceIndex : std::as_const( mInputFaces ) )
  {
    FaceRefinement refinement;

    const QgsMeshFace &face = mesh.face( faceIndex );
    int faceSize = face.size();

    QVector<int> addedVerticesIndex( faceSize, -1 );

    if ( faceSize == 3 || faceSize == 4 )
    {
      refinement.newVerticesLocalIndex.reserve( faceSize );
      refinement.refinedFaceNeighbor.reserve( faceSize );
      refinement.borderFaceNeighbor.reserve( faceSize );
      const QVector<int> &neighbors = topology.neighborsOfFace( faceIndex );

      double zValueSum = 0;

      for ( int positionInFace = 0; positionInFace < faceSize; ++positionInFace )
      {
        refinement.refinedFaceNeighbor.append( false );
        refinement.borderFaceNeighbor.append( false );

        int neighborFaceIndex = neighbors.at( positionInFace );
        bool needCreateVertex = true;
        if ( neighborFaceIndex != -1 && facesToRefine.contains( neighborFaceIndex ) )
        {
          int neighborFaceSize = mesh.face( neighborFaceIndex ).size();
          int positionVertexInNeighbor = vertexPositionInFace( mesh, face.at( positionInFace ), neighborFaceIndex );
          positionVertexInNeighbor = ( positionVertexInNeighbor + neighborFaceSize - 1 ) % neighborFaceSize;
          refinement.refinedFaceNeighbor[positionInFace] = true;
          QHash<int, FaceRefinement>::iterator it = facesRefinement.find( neighborFaceIndex );
          if ( it != facesRefinement.end() )
          {
            FaceRefinement &neighborRefinement = it.value();
            int existingVertexLocalIndex = neighborRefinement.newVerticesLocalIndex.at( positionVertexInNeighbor );
            refinement.newVerticesLocalIndex.append( existingVertexLocalIndex );
            needCreateVertex = false;
            zValueSum += mVerticesToAdd.at( existingVertexLocalIndex ).z();
          }
        }

        if ( needCreateVertex )
        {
          const QgsMeshVertex &vertex1 = mesh.vertex( face.at( positionInFace ) );
          const QgsMeshVertex &vertex2 = mesh.vertex( face.at( ( positionInFace + 1 ) % faceSize ) );

          refinement.newVerticesLocalIndex.append( mVerticesToAdd.count() );
          addedVerticesIndex[positionInFace] = mVerticesToAdd.count();

          mVerticesToAdd.append( QgsMeshVertex( ( vertex1.x() + vertex2.x() ) / 2,
                                                ( vertex1.y() + vertex2.y() ) / 2,
                                                ( vertex1.z() + vertex2.z() ) / 2 ) );

          zValueSum += mVerticesToAdd.last().z();
          mVertexToFaceToAdd.append( -1 );

        }
      }

      int faceStartIndex = startingGlobalFaceIndex + mFacesToAdd.count();

      if ( faceSize == 3 )
      {
        for ( int i = 0; i < faceSize; ++i )
        {
          QgsMeshFace newFace( {face.at( i ),
                                refinement.newVerticesLocalIndex.at( i ) + startingVertexIndex,
                                refinement.newVerticesLocalIndex.at( ( i + faceSize - 1 ) % faceSize ) + startingVertexIndex} );
          refinement.newFacesChangesIndex.append( mFacesToAdd.count() );
          mFacesToAdd.append( newFace );
          mFacesNeighborhoodToAdd.append( {-1, faceStartIndex + 3, -1} );

        }
        QgsMeshFace newFace( {refinement.newVerticesLocalIndex.at( 0 ) + startingVertexIndex,
                              refinement.newVerticesLocalIndex.at( 1 ) + startingVertexIndex,
                              refinement.newVerticesLocalIndex.at( ( 2 ) % faceSize ) + startingVertexIndex} );
        refinement.newFacesChangesIndex.append( mFacesToAdd.count() );
        mFacesToAdd.append( newFace );
        mFacesNeighborhoodToAdd.append( {faceStartIndex + 1, faceStartIndex + 2, faceStartIndex} );
      }

      if ( faceSize == 4 )
      {
        int centerVertexIndex = mVerticesToAdd.count() + startingVertexIndex;
        refinement.newCenterVertexIndex = mVerticesToAdd.count();
        QgsMeshVertex centerVertex = QgsMeshUtils::centroid( face, mesh.vertices );
        mVerticesToAdd.append( QgsMeshVertex( centerVertex.x(), centerVertex.y(), zValueSum / 4 ) );

        for ( int i = 0; i < faceSize; ++i )
        {
          QgsMeshFace newFace( {face.at( i ),
                                refinement.newVerticesLocalIndex.at( i ) + startingVertexIndex,
                                centerVertexIndex,
                                refinement.newVerticesLocalIndex.at( ( i + faceSize - 1 ) % faceSize ) + startingVertexIndex} );
          refinement.newFacesChangesIndex.append( mFacesToAdd.count() );
          mFacesToAdd.append( newFace );
          mFacesNeighborhoodToAdd.append( {-1, faceStartIndex + ( i + 1 ) % 4, faceStartIndex + ( i + 3 ) % 4, -1} );
        }

        mVertexToFaceToAdd.append( mFacesToAdd.count() + startingGlobalFaceIndex - 1 );
      }
      else
        refinement.newCenterVertexIndex = -1;

      facesRefinement.insert( faceIndex, refinement );
    }
    else
    {
      //not 3 or 4 vertices, we do not refine this face
      facesToRefine.remove( faceIndex );
    }

    //look for vertexToFace
    for ( int positionInFace = 0; positionInFace < faceSize; ++positionInFace )
    {
      if ( addedVerticesIndex.at( positionInFace ) != -1 )
      {
        mVertexToFaceToAdd[addedVerticesIndex.at( positionInFace )] =
          refinement.newFacesChangesIndex.at( positionInFace ) + startingGlobalFaceIndex;
      }

      int vertexIndex = face.at( positionInFace );
      if ( topology.firstFaceLinked( vertexIndex ) == faceIndex )
        mVerticesToFaceChanges.append( {vertexIndex, faceIndex, refinement.newFacesChangesIndex.at( positionInFace ) + startingGlobalFaceIndex} );
    }
  }

  //all new refined faces are in place, we can build their neighborhood with other new refined faces
  for ( QHash<int, FaceRefinement>::iterator it = facesRefinement.begin(); it != facesRefinement.end(); ++it )
  {
    int faceIndex = it.key();
    FaceRefinement &faceRefinement = it.value();
    const QgsMeshFace &face = mesh.face( faceIndex );
    int faceSize = face.size();

    const QVector<int> &neighbors = topology.neighborsOfFace( faceIndex );

    for ( int positionInFace = 0; positionInFace < faceSize; ++positionInFace )
    {
      if ( faceRefinement.refinedFaceNeighbor.at( positionInFace ) )
      {
        int neighborIndex = neighbors.at( positionInFace );
        int firstVertexIndex = face.at( positionInFace );
        int secondVertexIndex = faceRefinement.newVerticesLocalIndex.at( positionInFace ) + startingVertexIndex;

        const FaceRefinement &otherRefinement = facesRefinement.value( neighborIndex );
        const QgsMeshFace &otherFace = mesh.face( neighborIndex );
        int otherFaceSize = otherFace.size();
        int otherPositionInface = ( vertexPositionInFace( firstVertexIndex, otherFace ) + otherFaceSize - 1 ) % otherFaceSize;

        int newFace1ChangesIndex = faceRefinement.newFacesChangesIndex.at( ( positionInFace ) );
        const QgsMeshFace &newFace1 = mFacesToAdd.at( newFace1ChangesIndex );
        int positionInNewface1Index = vertexPositionInFace( firstVertexIndex, newFace1 );

        int newFace2ChangesIndex = faceRefinement.newFacesChangesIndex.at( ( positionInFace + 1 ) % faceSize );
        const QgsMeshFace &newFace2 = mFacesToAdd.at( newFace2ChangesIndex );
        int positionInNewface2Index = vertexPositionInFace( secondVertexIndex, newFace2 );

        int otherNewFace1ChangesIndex = otherRefinement.newFacesChangesIndex.at( ( otherPositionInface ) % otherFaceSize );
        int otherNewFace2ChangesIndex = otherRefinement.newFacesChangesIndex.at( ( otherPositionInface + 1 ) % otherFaceSize );

        mFacesNeighborhoodToAdd[newFace1ChangesIndex][positionInNewface1Index] = otherNewFace2ChangesIndex + startingGlobalFaceIndex;
        mFacesNeighborhoodToAdd[newFace2ChangesIndex][positionInNewface2Index] = otherNewFace1ChangesIndex + startingGlobalFaceIndex;
      }
    }
  }
}

bool QgsMeshEditRefineFaces::createNewBorderFaces( QgsMeshEditor *meshEditor,
    const QSet<int> &facesToRefine,
    QHash<int, FaceRefinement> &facesRefinement,
    QHash<int, BorderFace> &borderFaces )
{
  const QgsTopologicalMesh &topology = meshEditor->topologicalMesh();
  const QgsMesh &mesh = *meshEditor->topologicalMesh().mesh();

  int startingVertexIndex = mesh.vertexCount();
  int startingFaceChangesGlobalIndex = mesh.faceCount();

  // first create  the border faces
  for ( int faceIndexToRefine : facesToRefine )
  {
    const QgsMeshFace &faceToRefine = mesh.face( faceIndexToRefine );
    int faceToRefineSize = faceToRefine.size();

    const QVector<int> &neighbors = topology.neighborsOfFace( faceIndexToRefine );

    QHash<int, FaceRefinement>::iterator itFace = facesRefinement.find( faceIndexToRefine );

    if ( itFace == facesRefinement.end() )
      Q_ASSERT( false ); // That could not happen

    FaceRefinement &refinement = itFace.value();

    for ( int posInFaceToRefine = 0; posInFaceToRefine < faceToRefineSize; ++posInFaceToRefine )
    {
      int neighborFaceIndex = neighbors.at( posInFaceToRefine );
      if ( neighborFaceIndex != -1 && !facesToRefine.contains( neighborFaceIndex ) )
      {
        const QgsMeshFace &neighborFace = mesh.face( neighborFaceIndex );
        int neighborFaceSize = neighborFace.size();
        int positionInNeighbor = vertexPositionInFace( mesh, faceToRefine.at( posInFaceToRefine ), neighborFaceIndex );
        positionInNeighbor = ( positionInNeighbor + neighborFaceSize - 1 ) % neighborFaceSize;

        QHash<int, BorderFace>::iterator it = borderFaces.find( neighborFaceIndex );
        if ( it == borderFaces.end() ) //not present for now--> create a border face
        {
          BorderFace borderFace;
          for ( int i = 0; i < neighborFaceSize; ++i )
          {
            borderFace.unchangeFacesNeighbor.append( false );
            borderFace.borderFacesNeighbor.append( false );
            if ( i == positionInNeighbor )
            {
              borderFace.refinedFacesNeighbor.append( true );
              borderFace.newVerticesLocalIndex.append( refinement.newVerticesLocalIndex.at( posInFaceToRefine ) );
            }
            else
            {
              borderFace.refinedFacesNeighbor.append( false );
              borderFace.newVerticesLocalIndex.append( -1 );
            }
          }
          borderFaces.insert( neighborFaceIndex, borderFace );
        }
        else
        {
          BorderFace &borderFace = it.value();
          for ( int i = 0; i < neighborFaceSize; ++i )
          {
            if ( i == positionInNeighbor )
            {
              borderFace.unchangeFacesNeighbor[i] = false;
              borderFace.borderFacesNeighbor[i] = false;
              borderFace.refinedFacesNeighbor[i] = true;
              borderFace.newVerticesLocalIndex[i] = refinement.newVerticesLocalIndex.at( posInFaceToRefine );
            }
          }
        }
      }
    }
  }

  // now build information about neighbors
  for ( QHash<int, BorderFace>::iterator it = borderFaces.begin(); it != borderFaces.end(); ++it )
  {
    int faceIndex = it.key();
    BorderFace &borderFace = it.value();

    const QgsMeshFace &face = mesh.face( faceIndex );
    int faceSize = face.size();

    const QVector<int> &neighbors = topology.neighborsOfFace( faceIndex );
    for ( int posInFace = 0; posInFace < faceSize; ++posInFace )
    {
      int neighborIndex = neighbors.at( posInFace );

      if ( neighborIndex != -1 )
      {
        const QgsMeshFace &neighborFace = mesh.face( neighborIndex );
        int neighborFaceSize = neighborFace.size();
        int posInNeighbor = vertexPositionInFace( mesh, face.at( posInFace ), neighborIndex );
        posInNeighbor = ( posInNeighbor - 1 + neighborFaceSize ) % neighborFaceSize;

        QHash<int, FaceRefinement>::iterator itRefinement = facesRefinement.find( neighborIndex );
        if ( itRefinement != facesRefinement.end() )
        {
          FaceRefinement &neighborRefinement = itRefinement.value();
          neighborRefinement.borderFaceNeighbor[posInNeighbor] = true;
          borderFace.refinedFacesNeighbor[posInFace] = true;
          continue;
        }

        QHash<int, BorderFace>::iterator itNeighborBorder = borderFaces.find( neighborIndex );
        if ( itNeighborBorder == borderFaces.end() )
          borderFace.unchangeFacesNeighbor[posInFace] = true;
        else
        {
          BorderFace &neighborBorderFace = itNeighborBorder.value();
          neighborBorderFace.borderFacesNeighbor[posInNeighbor] = true;
          borderFace.borderFacesNeighbor[posInFace] = true;
          continue;
        }
      }

      borderFace.unchangeFacesNeighbor[posInFace] = true;

    }
  }

// create new faces for each border faces
  for ( QHash<int, BorderFace>::iterator it = borderFaces.begin(); it != borderFaces.end(); ++it )
  {
    int faceIndex = it.key();
    BorderFace &borderFace = it.value();

    const QgsMeshFace &face = mesh.face( faceIndex );
    int faceSize = face.size();

    QHash<p2t::Point *, int> mapPoly2TriPointToVertex;
    std::vector<p2t::Point *> points;
    for ( int i = 0; i < faceSize; ++i )
    {
      const QgsMeshVertex &vert = mesh.vertex( face.at( i ) );
      points.push_back( new p2t::Point( vert.x(), vert.y() ) );
      mapPoly2TriPointToVertex.insert( points.back(),  face.at( i ) );
      if ( borderFace.refinedFacesNeighbor.at( i ) )
      {
        int localVertexIndex = borderFace.newVerticesLocalIndex.at( i );
        const QgsMeshVertex &newVert = mVerticesToAdd.at( localVertexIndex );
        points.push_back( new p2t::Point( newVert.x(), newVert.y() ) );
        mapPoly2TriPointToVertex.insert( points.back(),  localVertexIndex + startingVertexIndex );
      }
    }

    try
    {
      std::unique_ptr<p2t::CDT> cdt( new p2t::CDT( points ) );
      cdt->Triangulate();
      std::vector<p2t::Triangle *> triangles = cdt->GetTriangles();
      QVector<QgsMeshFace> faces( triangles.size() );
      for ( size_t i = 0; i < triangles.size(); ++i )
      {
        QgsMeshFace &triangle = faces[i];
        triangle.resize( 3 );
        QVector<QgsMeshVertex> vertices( 3 );
        for ( int j = 0; j < 3; j++ )
        {
          int vertInd = mapPoly2TriPointToVertex.value( triangles.at( i )->GetPoint( j ), -1 );
          if ( vertInd == -1 )
            throw std::exception();;
          triangle[j] = vertInd;
          if ( vertInd >= startingVertexIndex )
            vertices[j] = mVerticesToAdd.at( vertInd - startingVertexIndex );
          else
            vertices[j] = mesh.vertex( vertInd );
        }
        QgsMeshUtils::setCounterClockwise( triangle, vertices[0], vertices[1], vertices[2] );
      }

      int startingFaceIndex = mesh.faceCount() + mFacesToAdd.count();

      QgsMeshEditingError error;
      QgsTopologicalMesh::TopologicalFaces topologicalFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, false, error );
      QVector<QgsTopologicalMesh::FaceNeighbors> neighborhood = topologicalFaces.facesNeighborhood();

      // reindex internal neighborhod
      for ( int i = 0; i < neighborhood.count(); ++i )
      {
        QgsTopologicalMesh::FaceNeighbors &neighbors = neighborhood[i];
        for ( int j = 0; j < neighbors.count(); ++j )
        {
          if ( neighbors[j] != -1 ) //internal neighborhood
            neighbors[j] = neighbors[j] + startingFaceIndex;
        }
      }

      QVector<int> neighborOfFace = topology.neighborsOfFace( faceIndex );

      // connect neighboring with refined faces, other border face and unchanged faces
      for ( int positionInFace = 0; positionInFace < faceSize; ++positionInFace )
      {
        if ( borderFace.refinedFacesNeighbor.at( positionInFace ) )
        {
          //here we have two edges to treat
          QVector<int> vertexIndexes( 2 );
          QVector<int> localFaceIndex( 2 );
          vertexIndexes[0] = face.at( positionInFace );
          vertexIndexes[1] = borderFace.newVerticesLocalIndex.at( positionInFace ) + startingVertexIndex;

          int refinedFaceIndex = neighborOfFace.at( positionInFace );
          const FaceRefinement &faceRefinement = facesRefinement.value( refinedFaceIndex );
          const QgsMeshFace &refinedFace = mesh.face( refinedFaceIndex );
          int refinedFaceSize = refinedFace.size();
          int positionInRefinedFace = ( vertexPositionInFace( vertexIndexes[0], refinedFace ) + refinedFaceSize - 1 ) % refinedFaceSize;

          for ( int i = 0; i < 2; ++i )
          {
            QgsMeshVertexCirculator circulator( topologicalFaces, vertexIndexes.at( i ) );
            circulator.goBoundaryClockwise();
            localFaceIndex[i] = circulator.currentFaceIndex();

            // new refined faces are in place, so we can link neighborhood
            QgsTopologicalMesh::FaceNeighbors &neighborsNewFace = neighborhood[localFaceIndex.at( i )];
            const QgsMeshFace newFace = faces.at( localFaceIndex.at( i ) );
            int positionInNewFace =  vertexPositionInFace( vertexIndexes.at( i ), newFace );
            int newFaceRefinedIndexInChanges = faceRefinement.newFacesChangesIndex.at( ( positionInRefinedFace + ( 1 - i ) ) % refinedFaceSize ) ;
            neighborsNewFace[positionInNewFace] = newFaceRefinedIndexInChanges + startingFaceChangesGlobalIndex;

            QgsTopologicalMesh::FaceNeighbors &neighborsRefinedFace = mFacesNeighborhoodToAdd[newFaceRefinedIndexInChanges];
            const QgsMeshFace &newRefinedFace = mFacesToAdd.at( newFaceRefinedIndexInChanges );
            int newRefinedFaceSize = newRefinedFace.size();
            int positionInNewRefinedChange = ( vertexPositionInFace( vertexIndexes.at( i ), newRefinedFace ) + newRefinedFaceSize - 1 ) % newRefinedFaceSize;
            neighborsRefinedFace[positionInNewRefinedChange] = localFaceIndex.at( i ) + startingFaceIndex;
          }

          borderFace.edgeFace.append( localFaceIndex.at( 0 ) + startingFaceIndex );
        }

        if ( borderFace.borderFacesNeighbor.at( positionInFace ) )
        {
          int vertexIndex = face.at( positionInFace );
          QgsMeshVertexCirculator circulator( topologicalFaces, vertexIndex );
          circulator.goBoundaryClockwise();
          int localFaceIndex = circulator.currentFaceIndex();

          // all new border faces are not in place, so store information for later
          borderFace.edgeFace.append( localFaceIndex + startingFaceIndex );
        }

        if ( borderFace.unchangeFacesNeighbor.at( positionInFace ) )
        {
          int vertexIndex = face.at( positionInFace );
          QgsMeshVertexCirculator circulator( topologicalFaces, vertexIndex );
          circulator.goBoundaryClockwise();
          int localFaceIndex = circulator.currentFaceIndex();

          const QgsMeshFace &newFace = faces.at( localFaceIndex );
          int positionInNewface = vertexPositionInFace( vertexIndex, newFace );
          QgsTopologicalMesh::FaceNeighbors &neighborsNewFace = neighborhood[localFaceIndex];
          int unchangedFaceIndex = neighborOfFace.at( positionInFace );
          neighborsNewFace[positionInNewface] = unchangedFaceIndex;

          if ( unchangedFaceIndex != -1 )
          {
            const QgsMeshFace &unchangedFace = mesh.face( unchangedFaceIndex );
            int unchangedFaceSize = unchangedFace.size();
            int positionInUnchangedFace = ( vertexPositionInFace( vertexIndex, unchangedFace ) + unchangedFaceSize - 1 ) % unchangedFaceSize;
            mNeighborhoodChanges.append( {unchangedFaceIndex, positionInUnchangedFace, faceIndex, localFaceIndex + startingFaceIndex} );
          }

          borderFace.edgeFace.append( localFaceIndex + startingFaceIndex );
        }
      }

      mFacesToAdd.append( faces );
      mFacesNeighborhoodToAdd.append( neighborhood );

      for ( p2t::Point *pt : points )
        delete pt;

    }
    catch ( ... )
    {
      return false;
    }
  }

  //all border faces are in place, now it is possible to finalize with completing their nieghborhood with other border faces
  for ( QHash<int, BorderFace>::iterator it = borderFaces.begin(); it != borderFaces.end(); ++it )
  {
    int faceIndex = it.key();
    BorderFace &borderFace = it.value();
    const QgsMeshFace &face = mesh.face( faceIndex );
    int faceSize = face.size();

    const QVector<int> neighbors = topology.neighborsOfFace( faceIndex );

    for ( int positionInFace = 0; positionInFace < faceSize; ++positionInFace )
    {
      if ( borderFace.borderFacesNeighbor.at( positionInFace ) )
      {
        int otherIndex = neighbors.at( positionInFace );
        const QgsMeshFace &otherFace = mesh.face( otherIndex );
        int otherFaceSize = otherFace.size();
        int otherPositionInface = ( vertexPositionInFace( face.at( positionInFace ), otherFace ) + otherFaceSize - 1 ) % otherFaceSize;
        const BorderFace &otherBorderFace = borderFaces.value( otherIndex );
        int otherNewFaceIndex = otherBorderFace.edgeFace.at( otherPositionInface );

        int newFaceChangesIndex = borderFace.edgeFace.at( positionInFace ) - startingFaceChangesGlobalIndex;
        const QgsMeshFace &newFace = mFacesToAdd.at( newFaceChangesIndex );
        int newFacePositionInFace = vertexPositionInFace( face.at( positionInFace ), newFace );

        mFacesNeighborhoodToAdd[newFaceChangesIndex][newFacePositionInFace] = otherNewFaceIndex;
      }

      // ... and look for vertexToFace
      for ( int positionInFace = 0; positionInFace < faceSize; ++positionInFace )
      {
        int vertexIndex = face.at( positionInFace );
        if ( topology.firstFaceLinked( vertexIndex ) == faceIndex )
          mVerticesToFaceChanges.append( {vertexIndex, faceIndex, borderFace.edgeFace.at( positionInFace ) } );
      }
    }
  }

  return true;
}

QString QgsMeshEditRefineFaces::text() const
{
  return QObject::tr( "Refine %n face(s)", nullptr, mInputFaces.count() );
}

bool QgsMeshTransformVerticesByExpression::calculate( QgsMeshLayer *layer )
{
  if ( !layer || !layer->meshEditor() || !layer->nativeMesh() )
    return false;

  if ( mInputVertices.isEmpty() )
    return false;

  const QgsMesh mesh = *layer->nativeMesh();
  QSet<int> concernedFaces;
  mChangingVertexMap = QHash<int, int>();

  std::unique_ptr<QgsExpressionContextScope> expScope( QgsExpressionContextUtils::meshExpressionScope( QgsMesh::Vertex ) );
  QgsExpressionContext context;
  context.appendScope( expScope.release() );
  context.lastScope()->setVariable( QStringLiteral( "_mesh_layer" ), QVariant::fromValue( layer ) );

  QVector<QgsMeshVertex> newVertices;
  newVertices.reserve( mInputVertices.count() );

  int inputCount = mInputVertices.count();
  mChangeCoordinateVerticesIndexes = mInputVertices;

  bool calcX = !mExpressionX.isEmpty();
  bool calcY = !mExpressionY.isEmpty();
  bool calcZ = !mExpressionZ.isEmpty();
  QgsExpression expressionX;
  if ( calcX )
  {
    expressionX = QgsExpression( mExpressionX );
    expressionX.prepare( &context );
  }

  QgsExpression expressionY;
  if ( calcY )
  {
    expressionY = QgsExpression( mExpressionY );
    expressionY.prepare( &context );
  }

  if ( calcX || calcY )
  {
    mNewXYValues.reserve( inputCount );
    mOldXYValues.reserve( inputCount );
  }

  QgsExpression expressionZ;
  if ( calcZ )
  {
    expressionZ = QgsExpression( mExpressionZ );
    expressionZ.prepare( &context );
    mNewZValues.reserve( inputCount );
    mOldZValues.reserve( inputCount );
  }

  for ( int i = 0; i < mInputVertices.count(); ++i )
  {
    const int vertexIndex = mInputVertices.at( i );
    context.lastScope()->setVariable( QStringLiteral( "_mesh_vertex_index" ), vertexIndex, false );

    mChangingVertexMap[vertexIndex] = i;
    const QVariant xvar = expressionX.evaluate( &context );
    const QVariant yvar = expressionY.evaluate( &context );
    const QVariant zvar = expressionZ.evaluate( &context );

    const QgsMeshVertex &vert = mesh.vertex( vertexIndex );

    if ( calcX || calcY )
    {
      mOldXYValues.append( QgsPointXY( vert ) );
      mNewXYValues.append( QgsPointXY( vert ) );

      const QList<int> facesAround = layer->meshEditor()->topologicalMesh().facesAroundVertex( vertexIndex );
      concernedFaces.unite( qgis::listToSet( facesAround ) );
    }

    bool ok = false;
    if ( calcX )
    {
      if ( xvar.isValid() )
      {
        double x = xvar.toDouble( &ok );
        if ( ok )
        {
          mNewXYValues.last().setX( x );
        }
        else
          return false;
      }
      else
      {
        return false;
      }
    }

    if ( calcY )
    {
      if ( yvar.isValid() )
      {
        double y = yvar.toDouble( &ok );
        if ( ok )
        {
          mNewXYValues.last().setY( y );
        }
        else
          return false;
      }
      else
        return false;
    }

    if ( calcZ )
    {
      double z = std::numeric_limits<double>::quiet_NaN();
      if ( zvar.isValid() )
      {
        z = zvar.toDouble( &ok );
        if ( !ok )
          z = std::numeric_limits<double>::quiet_NaN();
      }

      mNewZValues.append( z );
      mOldZValues.append( vert.z() );
    }
  }

  auto transformFunction = [this, layer ]( int vi )-> const QgsMeshVertex
  {
    return transformedVertex( layer, vi );
  };

  mNativeFacesIndexesGeometryChanged = qgis::setToList( concernedFaces );
  return ( !calcX && !calcY ) || layer->meshEditor()->canBeTransformed( mNativeFacesIndexesGeometryChanged, transformFunction );
}

QString QgsMeshTransformVerticesByExpression::text() const
{
  return QObject::tr( "Transform %n vertices by expression", nullptr, mInputVertices.count() );
}

void QgsMeshTransformVerticesByExpression::setExpressions( const QString &expressionX, const QString &expressionY, const QString &expressionZ )
{
  mExpressionX = expressionX;
  mExpressionY = expressionY;
  mExpressionZ = expressionZ;

  mChangingVertexMap.clear();
}

QgsTopologicalMesh::Changes QgsMeshTransformVerticesByExpression::apply( QgsMeshEditor *meshEditor )
{
  meshEditor->topologicalMesh().applyChanges( *this );
  mIsFinished = true;
  return *this;
}

QgsMeshVertex QgsMeshTransformVerticesByExpression::transformedVertex( QgsMeshLayer *layer, int vertexIndex ) const
{
  int pos = mChangingVertexMap.value( vertexIndex, -1 );
  if ( pos > -1 )
  {
    QgsPointXY pointXY;
    double z;

    if ( mNewXYValues.isEmpty() )
      pointXY = layer->nativeMesh()->vertex( vertexIndex );
    else
      pointXY = mNewXYValues.at( pos );

    if ( mNewZValues.isEmpty() )
      z = layer->nativeMesh()->vertex( vertexIndex ).z();
    else
      z = mNewZValues.at( pos );

    return QgsMeshVertex( pointXY.x(), pointXY.y(), z );
  }
  else
    return layer->nativeMesh()->vertex( vertexIndex );
}
