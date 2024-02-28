/***************************************************************************
  qgsmeshforcebypolylines.cpp - QgsMeshForceByPolylines

 ---------------------
 begin                : 5.9.2021
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
#include "qgsmeshforcebypolylines.h"

#include "qgsmesheditor.h"
#include "qgsgeometryutils.h"
#include "poly2tri.h"
#include "qgsmultisurface.h"
#include "qgsmulticurve.h"
#include "qgscurvepolygon.h"
#include "qgsmeshlayerutils.h"
#include "qgscurve.h"

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

bool QgsMeshEditForceByLine::edgeIntersection(
  int vertex1,
  int vertex2,
  int &closestSnappedVertex,
  QgsPoint &intersectionPoint,
  bool outAllowed )
{
  const QgsPointXY pt1( mCurrentPointPosition );
  const QgsPointXY pt2( mPoint2 );

  closestSnappedVertex = -1;

  QgsTriangularMesh *triangularMesh = mEditor->triangularMesh();

  const QgsPointXY v1( triangularMesh->vertices().at( vertex1 ) );
  const QgsPointXY v2( triangularMesh->vertices().at( vertex2 ) );

  double epsilon = std::numeric_limits<double>::epsilon() * mTolerance * mTolerance;

  QgsPointXY minDistPoint;
  bool snapV1 = sqrt( v1.sqrDistToSegment( pt1.x(), pt1.y(), pt2.x(), pt2.y(), minDistPoint, epsilon ) ) < mTolerance;
  bool snapV2 = sqrt( v2.sqrDistToSegment( pt1.x(), pt1.y(), pt2.x(), pt2.y(), minDistPoint, epsilon ) ) < mTolerance;

  bool intersectLine = false;
  bool isIntersect = QgsGeometryUtils::segmentIntersection(
                       mCurrentPointPosition,
                       mPoint2,
                       triangularMesh->vertices().at( vertex1 ),
                       triangularMesh->vertices().at( vertex2 ),
                       intersectionPoint,
                       intersectLine,
                       outAllowed ? mTolerance : 0, true );

  if ( snapV1 == snapV2 ) //both or neither of them are snapped
  {
    double distance1FromIntersection = v1.distance( intersectionPoint );
    double distance2FromIntersection = v2.distance( intersectionPoint );
    if ( distance1FromIntersection <= distance2FromIntersection )
    {
      snapV1 &= true;
      snapV2 = false;
    }
    else
    {
      snapV1 = false;
      snapV2 &= true;
    }
  }

  if ( isIntersect && snapV1 )
  {
    closestSnappedVertex = vertex1;
    intersectionPoint = triangularMesh->vertices().at( vertex1 );
    return true;
  }
  else if ( isIntersect && snapV2 )
  {
    closestSnappedVertex = vertex2;
    intersectionPoint = triangularMesh->vertices().at( vertex2 );
    return true;
  }

  return isIntersect;
}


static void buildHolesWithOneFace( QgsMeshEditor *meshEditor,
                                   int faceIndex,
                                   int firstVertex,
                                   int secondVertex,
                                   QList<int> &holeOnLeft,
                                   QList<int> &neighborsOnLeft,
                                   QList<int> &holeOnRight,
                                   QList<int> &neighborsOnRight )
{
  const QgsMeshFace &face = meshEditor->topologicalMesh().mesh()->face( faceIndex );
  const QVector<int> &neighbors = meshEditor->topologicalMesh().neighborsOfFace( faceIndex );
  const int faceSize = face.size();

  int beginPos = vertexPositionInFace( firstVertex, face );
  int endPos = vertexPositionInFace( secondVertex, face );

  // build hole on the right
  for ( int i = 0; i < faceSize; ++i )
  {
    int currentPos = ( beginPos + i ) % faceSize;
    holeOnRight.append( face.at( currentPos ) );
    if ( currentPos == endPos )
      break;
    neighborsOnRight.append( neighbors.at( currentPos ) );
  }

  // build hole on the left
  for ( int i = 0; i < faceSize; ++i )
  {
    int currentPos = ( beginPos + faceSize - i ) % faceSize;
    holeOnLeft.append( face.at( currentPos ) );
    if ( currentPos == endPos )
      break;
    int neighborPos = ( currentPos + faceSize - 1 ) % faceSize;
    neighborsOnLeft.append( neighbors.at( neighborPos ) );
  }
}


//! Returns the next faces (face on the other side of the cut edge)
static int cutEdgeFromSnappedVertex( QgsMeshEditor *meshEditor,
                                     int faceIndex,
                                     int startingSnappedVertex,
                                     int edgePosition,
                                     QList<int> &newVerticesOnLeftHole,
                                     QList<int> &newNeighborsOnLeftHole,
                                     QList<int> &newVerticesOnRightHole,
                                     QList<int> &newNeighborsOnRightHole )
{
  const QgsMeshFace &face = meshEditor->topologicalMesh().mesh()->face( faceIndex );
  const QVector<int> &neighbors = meshEditor->topologicalMesh().neighborsOfFace( faceIndex );
  const int faceSize = face.size();

  const int beginPos = vertexPositionInFace( startingSnappedVertex, face );

  const int endPosOnLeft = ( edgePosition + 1 ) % faceSize;
  const int endPosOnRight = edgePosition;

  // build hole on the left
  for ( int i = 0; i < faceSize; ++i )
  {
    int currentPos = ( beginPos + faceSize - i ) % faceSize;
    newVerticesOnLeftHole.append( face.at( currentPos ) );
    if ( currentPos == endPosOnLeft )
      break;
    int neighborPos = ( currentPos + faceSize - 1 ) % faceSize;
    newNeighborsOnLeftHole.append( neighbors.at( neighborPos ) );
  }

  // build hole on the right
  for ( int i = 0; i < faceSize; ++i )
  {
    int currentPos = ( beginPos + i ) % faceSize;
    newVerticesOnRightHole.append( face.at( currentPos ) );
    if ( currentPos == endPosOnRight )
      break;
    newNeighborsOnRightHole.append( neighbors.at( currentPos ) );
  }

  return neighbors.at( edgePosition );
}


void QgsMeshEditForceByLine::setInputLine( const QgsPoint &pt1, const QgsPoint &pt2, double tolerance, bool newVertexOnIntersection )
{
  clear();
  mPoint1 = pt1;
  mPoint2 = pt2;
  mTolerance = tolerance;
  mNewVertexOnIntersection = newVertexOnIntersection;
  mFirstPointChecked = false;
  mSecondPointChecked = false;
  mCurrentSnappedVertex = -1;
  mIsFinished = false;
  mCurrentPointPosition = mPoint1;
  mRemovedFaces.clear();
  mHoleOnLeft.clear();
  mNeighborOnLeft.clear();
  mHoleOnRight.clear();
  mNeighborOnRight.clear();
  mNewVerticesIndexesOnLine.clear();
  mEndOnPoint2 = false;
  mPoint2VertexIndex = -1;
}

QgsTopologicalMesh::Changes QgsMeshEditForceByLine::apply( QgsMeshEditor *meshEditor )
{
  clear();
  mEditor = meshEditor;

  if ( ! mFirstPointChecked )
  {
    mFirstPointChecked = true;

    if ( mInterpolateZValueOnMesh )
      interpolateZValueOnMesh( mPoint1 );

    QgsMeshVertex v1 = meshEditor->triangularMesh()->triangularToNativeCoordinates( mPoint1 );
    int closeEdge = -1;
    int faceCloseEdge = -1;
    if ( meshEditor->edgeIsClose( mPoint1, mTolerance, faceCloseEdge, closeEdge ) )
    {
      //if we are too close form a vertex, do nothing
      const QgsMeshFace &face = meshEditor->topologicalMesh().mesh()->face( faceCloseEdge );
      int vertexIndex1 = face.at( closeEdge );
      int vertexIndex2 = face.at( ( closeEdge + 1 ) % face.size() );

      if ( ( meshEditor->triangularMesh()->vertices().at( vertexIndex1 ).distance( mPoint1 ) > mTolerance ) &&
           ( meshEditor->triangularMesh()->vertices().at( vertexIndex2 ).distance( mPoint1 ) > mTolerance ) )
        return meshEditor->topologicalMesh().insertVertexInFacesEdge( faceCloseEdge, closeEdge, v1 );
    }
    else
    {
      int includdingFace = meshEditor->triangularMesh()->nativeFaceIndexForPoint( mPoint1 );
      if ( includdingFace != -1 )
      {
        return meshEditor->topologicalMesh().addVertexInFace( includdingFace, v1 );
      }
    }
  }

  if ( ! mSecondPointChecked )
  {
    mSecondPointChecked = true;
    if ( mInterpolateZValueOnMesh )
      interpolateZValueOnMesh( mPoint2 );

    QgsMeshVertex v2 = meshEditor->triangularMesh()->triangularToNativeCoordinates( mPoint2 );
    int closeEdge = -1;
    int faceCloseEdge = -1;
    if ( meshEditor->edgeIsClose( mPoint2, mTolerance, faceCloseEdge, closeEdge ) )
    {
      //if we are too close form a vertex, do nothing, just records the index of the vertex for point 2
      const QgsMeshFace &face = meshEditor->topologicalMesh().mesh()->face( faceCloseEdge );
      int vertexIndex1 = face.at( closeEdge );
      int vertexIndex2 = face.at( ( closeEdge + 1 ) % face.size() );

      bool snap1 = meshEditor->triangularMesh()->vertices().at( vertexIndex1 ).distance( mPoint2 ) <= mTolerance ;
      bool snap2 = meshEditor->triangularMesh()->vertices().at( vertexIndex2 ).distance( mPoint2 ) <= mTolerance ;

      if ( snap1 )
      {
        mEndOnPoint2 = true;
        mPoint2VertexIndex = vertexIndex1;
        mPoint2 = meshEditor->triangularMesh()->vertices().at( vertexIndex1 );
      }
      else if ( snap2 )
      {
        mEndOnPoint2 = true;
        mPoint2VertexIndex = vertexIndex2;
        mPoint2 = meshEditor->triangularMesh()->vertices().at( vertexIndex2 );
      }
      else
      {
        QgsTopologicalMesh::Changes changes = meshEditor->topologicalMesh().insertVertexInFacesEdge( faceCloseEdge, closeEdge, v2 );
        if ( !changes.addedVertices().isEmpty() )
        {
          mEndOnPoint2 = true;
          mPoint2VertexIndex = meshEditor->topologicalMesh().mesh()->vertexCount() - 1;
        }

        return changes;
      }
    }
    else
    {
      int includdingFace = meshEditor->triangularMesh()->nativeFaceIndexForPoint( mPoint2 );
      if ( includdingFace != -1 )
      {
        QgsTopologicalMesh::Changes changes = meshEditor->topologicalMesh().addVertexInFace( includdingFace, v2 );
        if ( !changes.addedVertices().isEmpty() )
        {
          mEndOnPoint2 = true;
          mPoint2VertexIndex = meshEditor->topologicalMesh().mesh()->vertexCount() - 1;
        }

        return changes;
      }
    }
  }

  if ( buildForcedElements() )
  {
    meshEditor->topologicalMesh().applyChanges( *this );
    return ( *this );
  }

  return QgsTopologicalMesh::Changes();
}

void QgsMeshEditForceByLine::finish()
{
  mIsFinished = true;
}

void QgsMeshEditForceByLine::interpolateZValueOnMesh( QgsPoint &point ) const
{
  QgsTriangularMesh *triangularMesh = mEditor->triangularMesh();

  int includdingFace = triangularMesh->nativeFaceIndexForPoint( point );
  if ( includdingFace != -1 )
  {
    int triangleFaceIndex = triangularMesh->faceIndexForPoint_v2( point );
    if ( triangleFaceIndex != -1 )
    {
      QgsMeshFace triangleFace = triangularMesh->triangles().at( triangleFaceIndex );
      QgsMeshVertex tv1 = triangularMesh->vertices().at( triangleFace.at( 0 ) );
      QgsMeshVertex tv2 = triangularMesh->vertices().at( triangleFace.at( 1 ) );
      QgsMeshVertex tv3 = triangularMesh->vertices().at( triangleFace.at( 2 ) );
      double z = QgsMeshLayerUtils::interpolateFromVerticesData( tv1, tv2, tv3, tv1.z(), tv2.z(), tv3.z(), point );
      point.setZ( z );
    }
  }
}

void QgsMeshEditForceByLine::interpolateZValue( QgsMeshVertex &point, const QgsPoint &otherPoint1, const QgsPoint &otherPoint2 )
{
  double distPoint = point.distance( otherPoint1 );
  double totalDistance = otherPoint1.distance( otherPoint2 );

  point.setZ( otherPoint1.z() + ( otherPoint2.z() - otherPoint1.z() )*distPoint / totalDistance );
}

bool QgsMeshEditForceByLine::buildForcedElements()
{
  QgsTriangularMesh *triangularMesh = mEditor->triangularMesh();
  QgsMesh *mesh = mEditor->topologicalMesh().mesh();
  mAddedFacesFirstIndex = mesh->faceCount();
  QSet<int> treatedFaces;

  int startingVertexIndex = mesh->vertices.count();

  int currentFaceIndex = -1;
  bool searchOutside = false; //if we need to search outside last faces
  QPair<int, int> currentEdge{-1, -1};

  int currentAddedVertex = -1; // Last added point
  int nextCutFace = -1; //face that has to be cut from an intersected edge (not snap on existing vertex)
  int leftFace = -1; //the face that has been just cut in a edge

  while ( true )
  {
    if ( mCurrentSnappedVertex == -1 )
      currentFaceIndex = triangularMesh->nativeFaceIndexForPoint( mCurrentPointPosition );

    if ( currentFaceIndex == leftFace )
      currentFaceIndex = -1;

    if ( mCurrentSnappedVertex != -1 && !searchOutside )
    {
      //the current intersection is snapped on a existing vertex
      currentFaceIndex = -1;
      int previousSnappedVertex = -1;
      int intersectionFaceIndex = -1;
      QgsPoint intersectionPoint( 0, 0, 0 );
      int edgePosition = -1;

      bool result = searchIntersectionEdgeFromSnappedVertex(
                      intersectionFaceIndex,
                      previousSnappedVertex,
                      mCurrentSnappedVertex,
                      intersectionPoint,
                      edgePosition,
                      treatedFaces );

      if ( !result )
      {
        //here maybe mPoint2 is snapped with the current snap vertex, check first this last check and stop if it true
        if ( mCurrentSnappedVertex != -1 &&
             mPoint2.distance( triangularMesh->vertices().at( mCurrentSnappedVertex ) ) < mTolerance )
          break;

        //we have nothing in this part of the mesh, restart from the last interesting point to the point 2
        searchOutside = true;
      }
      else
      {
        if ( mEndOnPoint2 && mCurrentSnappedVertex == mPoint2VertexIndex )
        {
          mIsFinished = true;
          return false;
        }

        if ( mCurrentSnappedVertex != -1 )
        {
          // we have snapped a vertex on the other side of a face, we can build holes

          buildHolesWithOneFace( mEditor,
                                 intersectionFaceIndex,
                                 previousSnappedVertex,
                                 mCurrentSnappedVertex,
                                 mHoleOnLeft,
                                 mNeighborOnLeft,
                                 mHoleOnRight,
                                 mNeighborOnRight );

          mRemovedFaces.append( intersectionFaceIndex );

          if ( finishForcingLine() )
            return true;
          else
            break;
        }
        else
        {
          // we cut an edge and start a new hole (without finishing it)

          nextCutFace = cutEdgeFromSnappedVertex( mEditor,
                                                  intersectionFaceIndex,
                                                  previousSnappedVertex,
                                                  edgePosition,
                                                  mHoleOnLeft,
                                                  mNeighborOnLeft,
                                                  mHoleOnRight,
                                                  mNeighborOnRight );

          mRemovedFaces.append( intersectionFaceIndex );

          int iv1 = mHoleOnLeft.last();
          int iv2 = mHoleOnRight.last();

          if ( mNewVertexOnIntersection )
          {
            currentAddedVertex = startingVertexIndex + mVerticesToAdd.count();
            mNewVerticesIndexesOnLine.append( mVerticesToAdd.count() );
            if ( mInterpolateZValueOnMesh )
              interpolateZValue( intersectionPoint,
                                 triangularMesh->vertices().at( iv1 ),
                                 triangularMesh->vertices().at( iv2 ) );
            else
              interpolateZValue( intersectionPoint, mPoint1, mPoint2 );
            mVerticesToAdd.append( triangularMesh->triangularToNativeCoordinates( intersectionPoint ) );
          }

          if ( nextCutFace != -1 )
          {
            mCurrentSnappedVertex = -1;
            currentFaceIndex = nextCutFace;
            currentEdge = {mHoleOnLeft.last(), mHoleOnRight.last()};
          }
          else
          {
            // we cut a boundary edge, we need to close the hole
            if ( !mNewVertexOnIntersection )
            {
              currentAddedVertex = startingVertexIndex + mVerticesToAdd.count();
              if ( mInterpolateZValueOnMesh )
                interpolateZValue( intersectionPoint,
                                   triangularMesh->vertices().at( iv1 ),
                                   triangularMesh->vertices().at( iv2 ) );
              else
                interpolateZValue( intersectionPoint, mPoint1, mPoint2 );
              mVerticesToAdd.append( triangularMesh->triangularToNativeCoordinates( intersectionPoint ) );
            }
            else
              mNewVerticesIndexesOnLine.removeLast();

            mHoleOnLeft.append( currentAddedVertex );
            mNeighborOnLeft.append( -1 );
            mHoleOnRight.append( currentAddedVertex );
            mNeighborOnRight.append( -1 );

            if ( finishForcingLine() )
              return true;
            else
              break;

            leftFace = intersectionFaceIndex;
          }

          mCurrentPointPosition = intersectionPoint;
        }
      }
    }
    else if ( nextCutFace != -1 )
    {
      const QgsMeshFace &face = mesh->face( nextCutFace );
      const QVector<int> &neighbors = mEditor->topologicalMesh().neighborsOfFace( nextCutFace );
      int faceSize = face.size();

      currentFaceIndex = nextCutFace;

      mRemovedFaces.append( nextCutFace );

      int edgePositionOnLeft =  vertexPositionInFace( currentEdge.first, face );
      int edgePositionOnRight = vertexPositionInFace( currentEdge.second, face );
      int firstEdgeToTest = vertexPositionInFace( currentEdge.second, face );

      bool foundSomething = false;
      //search for another edge or a snapped vertex
      for ( int fi = 0; fi < faceSize; ++fi )
      {
        int iv1 = face.at( ( firstEdgeToTest + fi ) % faceSize );
        int iv2 = face.at( ( firstEdgeToTest + fi + 1 ) % faceSize );

        if ( iv1 == currentEdge.first && iv2 == currentEdge.second )
          continue;

        int snapVertex = -1;
        QgsPoint intersection( 0, 0, 0 );
        if ( edgeIntersection( iv1, iv2, snapVertex, intersection, false ) ||
             snapVertex != -1 )
        {
          foundSomething = true;

          int endPositionOnRight;
          int endPositionOnLeft;

          if ( snapVertex != -1 )
          {
            // closing holes
            endPositionOnRight = vertexPositionInFace( snapVertex, face );
            endPositionOnLeft = vertexPositionInFace( snapVertex, face );

            nextCutFace = -1;
            currentEdge = {-1, -1};
            mCurrentSnappedVertex = snapVertex;
          }
          else
          {
            // we cut another edge
            endPositionOnLeft = vertexPositionInFace( iv2, face );
            endPositionOnRight = vertexPositionInFace( iv1, face );

            nextCutFace = neighbors.at( endPositionOnRight );

            if ( mNewVertexOnIntersection )
            {
              currentAddedVertex = startingVertexIndex + mVerticesToAdd.count();
              mNewVerticesIndexesOnLine.append( mVerticesToAdd.count() );
              if ( mInterpolateZValueOnMesh )
                interpolateZValue( intersection,
                                   triangularMesh->vertices().at( iv1 ),
                                   triangularMesh->vertices().at( iv2 ) );
              else
                interpolateZValue( intersection, mPoint1, mPoint2 );
              mVerticesToAdd.append( triangularMesh->triangularToNativeCoordinates( intersection ) );
            }
          }

          int currentPos = edgePositionOnLeft;
          while ( currentPos != endPositionOnLeft )
          {
            int nextPos = ( currentPos - 1 + faceSize ) % faceSize;
            mHoleOnLeft.append( face.at( nextPos ) );
            int neighborPos = nextPos;
            mNeighborOnLeft.append( neighbors.at( neighborPos ) );
            currentPos = nextPos;
          }

          currentPos = edgePositionOnRight;
          while ( currentPos != endPositionOnRight )
          {
            int nextPos = ( currentPos + 1 ) % faceSize;
            mHoleOnRight.append( face.at( nextPos ) );
            int neighborPos = ( nextPos + faceSize - 1 ) % faceSize;
            mNeighborOnRight.append( neighbors.at( neighborPos ) );
            currentPos = nextPos;
          }

          mCurrentPointPosition = intersection;

          if ( snapVertex != -1 )
          {
            currentEdge = {-1, -1};

            if ( finishForcingLine() )
            {
              mIsFinished = mEndOnPoint2 && mPoint2VertexIndex == snapVertex;
              return true;
            }
            else
            {
              mIsFinished = true;
              return false;
            }
          }
          else if ( nextCutFace == -1 )
          {
            // we had cut a boundary edge, we need to close the hole
            if ( !mNewVertexOnIntersection )
            {
              currentAddedVertex = startingVertexIndex + mVerticesToAdd.count();
              if ( mInterpolateZValueOnMesh )
                interpolateZValue( intersection,
                                   triangularMesh->vertices().at( iv1 ),
                                   triangularMesh->vertices().at( iv2 ) );
              else
                interpolateZValue( intersection, mPoint1, mPoint2 );
              mVerticesToAdd.append( triangularMesh->triangularToNativeCoordinates( intersection ) );
            }
            else
              mNewVerticesIndexesOnLine.removeLast();

            mHoleOnLeft.append( currentAddedVertex );
            mNeighborOnLeft.append( -1 );
            mHoleOnRight.append( currentAddedVertex );
            mNeighborOnRight.append( -1 );

            if ( finishForcingLine() )
              return true;
            else
            {
              mIsFinished = true;
              return false;
            }
          }
          else
            currentEdge = {mHoleOnLeft.last(), mHoleOnRight.last()};

          break;
        }
      }

      if ( ! foundSomething )
      {
        // nothing intersected, something wrong -->leave with false
        break;
      }
    }
    else if ( currentFaceIndex == -1 || searchOutside )
    {
      // point1 is outside the mesh, we need to find the first face intersecting the segment of the line
      const QgsRectangle bbox( mCurrentPointPosition, mPoint2 );
      const QList<int> candidateFacesIndexes = triangularMesh->nativeFaceIndexForRectangle( bbox );
      int closestFaceIndex = -1;
      int closestEdge = -1; //the index of the first vertex of the edge (ccw)
      int closestSnapVertex = -1;
      double minimalDistance = std::numeric_limits<double>::max();
      QgsPoint closestIntersectionPoint( 0, 0, 0 );
      for ( const int candidateFaceIndex : candidateFacesIndexes )
      {
        if ( candidateFaceIndex == leftFace )
          continue; //we have just left this face

        if ( treatedFaces.contains( candidateFaceIndex ) ) //we have already pass through this face
          continue;

        const QgsMeshFace &candidateFace = mesh->face( candidateFaceIndex );
        const int faceSize = candidateFace.size();

        for ( int i = 0; i < faceSize; ++i )
        {
          int iv1 = candidateFace.at( i );
          int iv2 = candidateFace.at( ( i + 1 ) % faceSize );

          if ( iv1 == mCurrentSnappedVertex || iv2 == mCurrentSnappedVertex )
            continue;

          int snapVertex = -1;
          QgsPoint intersectionPoint;
          bool isIntersect = edgeIntersection( iv1, iv2, snapVertex, intersectionPoint, true );

          if ( isIntersect )
          {
            double distance = intersectionPoint.distance( mCurrentPointPosition );
            if ( distance < minimalDistance )
            {
              closestFaceIndex = candidateFaceIndex;
              closestEdge = candidateFace.at( i );
              closestSnapVertex = snapVertex;
              closestIntersectionPoint = intersectionPoint;
              minimalDistance = distance;
            }
          }
        }
      }

      if ( closestFaceIndex < 0 || closestEdge < 0 ) //we don't have an intersecting face
      {
        //nothing to do
        break;
      }
      else //we have an intersecting, face
      {
        mCurrentSnappedVertex = closestSnapVertex;
        treatedFaces.insert( closestFaceIndex );
        searchOutside = false;
        if ( mCurrentSnappedVertex == -1 )
        {
          //we cut an edge when entering the mesh
          const QgsMeshFace &face = mesh->face( closestFaceIndex );
          currentAddedVertex = startingVertexIndex + mVerticesToAdd.count();

          nextCutFace = closestFaceIndex;

          mHoleOnRight.append( currentAddedVertex );
          mHoleOnRight.append( face.at( ( vertexPositionInFace( closestEdge, face ) + 1 ) % face.size() ) );
          mNeighborOnRight.append( -1 );


          mHoleOnLeft.append( currentAddedVertex );
          mHoleOnLeft.append( closestEdge ) ;
          mNeighborOnLeft.append( -1 );

          currentEdge = {mHoleOnLeft.last(), mHoleOnRight.last()};

          if ( mInterpolateZValueOnMesh )
            interpolateZValue( closestIntersectionPoint,
                               triangularMesh->vertices().at( mHoleOnLeft.last() ),
                               triangularMesh->vertices().at( mHoleOnRight.last() ) );
          else
            interpolateZValue( closestIntersectionPoint, mPoint1, mPoint2 );

          mVerticesToAdd.append( triangularMesh->triangularToNativeCoordinates( closestIntersectionPoint ) );

        }
      }
    }
    else if ( mCurrentSnappedVertex == -1 ) // we are in a face without snapped vertex (yet)
    {
      //check in we snap with any of the vertices
      double minimalDistance = std::numeric_limits<double>::max();
      const QgsMeshFace &face = mesh->face( currentFaceIndex );
      for ( int i = 0; i < face.size(); ++i )
      {
        const QgsMeshVertex &vertex = triangularMesh->vertices().at( face.at( i ) );
        const double distance = mCurrentPointPosition.distance( vertex );
        if ( distance < mTolerance && distance < minimalDistance )
        {
          minimalDistance = distance;
          mCurrentSnappedVertex = face.at( i );
        }
      }
      searchOutside = false;

      if ( mCurrentSnappedVertex == -1 )
      {
        //we can't be in a face without snap vertex --> leave
        break;
      }
    }
    if ( mCurrentSnappedVertex != -1 )
    {
      if ( mCurrentSnappedVertex == mPoint2VertexIndex )
      {
        mIsFinished = true;
        return true;
      }
      mCurrentPointPosition = triangularMesh->vertices().at( mCurrentSnappedVertex );
    }
  }

  mIsFinished = true;
  return false;
}


bool QgsMeshEditForceByLine::searchIntersectionEdgeFromSnappedVertex
( int &intersectionFaceIndex,
  int &previousSnappedVertex,
  int &currentSnappedVertexIndex,
  QgsPoint &intersectionPoint,
  int &edgePosition,
  QSet<int> &treatedFaces )
{
  previousSnappedVertex = currentSnappedVertexIndex;
  QSet<int> treatedVertices;

  while ( true )
  {
    treatedVertices.insert( currentSnappedVertexIndex );
    const QList<int> facesAround = mEditor->topologicalMesh().facesAroundVertex( currentSnappedVertexIndex );

    bool foundSomething = false;
    for ( const int faceIndex : std::as_const( facesAround ) )
    {
      const QgsMeshFace &face = mEditor->topologicalMesh().mesh()->face( faceIndex );
      int faceSize = face.size();
      int vertexPos = vertexPositionInFace( currentSnappedVertexIndex, face );
      QgsPoint oppositeIntersectionPoint;
      int newSnapVertex = -1;
      for ( int i = 1; i < faceSize - 1; ++i )
      {
        edgePosition = ( vertexPos + i ) % faceSize ;
        foundSomething = edgeIntersection( face.at( edgePosition ),
                                           face.at( ( edgePosition + 1 ) % faceSize ),
                                           newSnapVertex,
                                           intersectionPoint,
                                           false );

        if ( mEndOnPoint2 && newSnapVertex == mPoint2VertexIndex )
        {
          mCurrentSnappedVertex = newSnapVertex;
          return true;
        }

        if ( newSnapVertex != -1 )
          foundSomething = ( newSnapVertex != previousSnappedVertex &&
                             !treatedVertices.contains( newSnapVertex ) );

        if ( foundSomething )
          break;
      }

      if ( foundSomething )
      {
        treatedFaces.insert( faceIndex );
        if ( newSnapVertex != -1 )
        {
          previousSnappedVertex = currentSnappedVertexIndex;
          currentSnappedVertexIndex = newSnapVertex;
          if ( newSnapVertex == face.at( ( vertexPos + 1 ) % faceSize ) || newSnapVertex == face.at( ( vertexPos - 1 + faceSize ) % faceSize ) )
          {
            // the two vertices already share an edge
            mCurrentPointPosition = mEditor->triangularMesh()->vertices().at( newSnapVertex );
            break;
          }
          else
          {
            intersectionFaceIndex = faceIndex;
            return true;
          }
        }
        else
        {
          intersectionFaceIndex = faceIndex;
          previousSnappedVertex = currentSnappedVertexIndex;
          currentSnappedVertexIndex = -1;
          return true;
        }
      }
    }

    if ( !foundSomething )
      break;
  }

  return false;
}

bool QgsMeshEditForceByLine::triangulateHoles(
  const QList<int> &hole,
  const QList<int> &neighbors,
  bool isLeftHole,
  QList<std::array<int, 2>> &newFacesOnLine,
  std::array<int, 2> &extremeFaces )
{
  QgsMesh *mesh = mEditor->topologicalMesh().mesh();

  // Check if we don't have duplicate vertex, that could lead to a crash
  for ( int i = 0; i < hole.count(); ++i )
  {
    for ( int j = i + 1; j < hole.count(); ++j )
    {
      if ( hole.at( i ) == hole.at( j ) )
        return false;

      if ( mesh->vertex( hole.at( i ) ).distance( mesh->vertex( hole.at( j ) ) ) < mTolerance )
        return false;
    }
  }

  int startingFaceGlobalIndex = mAddedFacesFirstIndex + mFacesToAdd.count();
  int startingFaceLocalIndex = mFacesToAdd.count();

  std::vector<p2t::Point *> holeToFill;
  try
  {
    QHash<p2t::Point *, int> mapPoly2TriPointToVertex;
    holeToFill.resize( hole.count() + mNewVerticesIndexesOnLine.count() );
    for ( int i = 0; i < hole.count(); ++i )
    {
      int vertexIndex = hole.at( i );
      QgsMeshVertex vertex;
      if ( vertexIndex < mesh->vertexCount() )
        vertex = mesh->vertex( vertexIndex );
      else
        vertex = mVerticesToAdd.at( vertexIndex - mesh->vertexCount() );

      holeToFill[i] = new p2t::Point( vertex.x(), vertex.y() );
      mapPoly2TriPointToVertex.insert( holeToFill[i], vertexIndex );
    }

    const int verticesOnLineCount = mNewVerticesIndexesOnLine.count();
    for ( int i = 0; i < verticesOnLineCount; ++i )
    {
      int vertexLocalIndex = mNewVerticesIndexesOnLine.at( verticesOnLineCount - i - 1 );
      const QgsMeshVertex &vertex = mVerticesToAdd.at( vertexLocalIndex );
      holeToFill[i + hole.count()] = new p2t::Point( vertex.x(), vertex.y() );
      mapPoly2TriPointToVertex.insert( holeToFill[i + hole.count()], vertexLocalIndex + mesh->vertexCount() );
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
        face[j] = vertInd;
      }
    }

    QgsMeshEditingError error;
    QgsTopologicalMesh::TopologicalFaces topologicalFaces = QgsTopologicalMesh::createNewTopologicalFaces( newFaces, false, error );
    if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
      throw std::exception();

    const QVector<QgsMeshFace> &facesToAdd = topologicalFaces.meshFaces();
    mFacesToAdd.append( facesToAdd );
    mFacesNeighborhoodToAdd.append( topologicalFaces.facesNeighborhood() );

    for ( const int vtc : hole )
    {
      int firstLinkedFace = mEditor->topologicalMesh().firstFaceLinked( vtc );
      if ( mRemovedFaces.contains( firstLinkedFace ) )
        mVerticesToFaceChanges.append( {vtc, firstLinkedFace, topologicalFaces.vertexToFace( vtc ) + startingFaceGlobalIndex} );
    }

    // reindex neighborhood for new faces
    for ( int i = 0; i < facesToAdd.count(); ++i )
    {
      QgsTopologicalMesh::FaceNeighbors &faceNeighbors = mFacesNeighborhoodToAdd[i + startingFaceLocalIndex];
      faceNeighbors = topologicalFaces.facesNeighborhood().at( i );
      for ( int n = 0; n < faceNeighbors.count(); ++n )
      {
        if ( faceNeighbors.at( n ) != -1 )
          faceNeighbors[n] += startingFaceGlobalIndex; //reindex internal neighborhood
      }
    }

    // link neighborhood for boundaries of each side
    for ( int i = 0 ; i < hole.count() - 1; ++i )
    {
      int vertexHoleIndex = hole.at( i );
      int meshFaceBoundaryIndex = neighbors.at( i );

      QgsMeshVertexCirculator circulator = QgsMeshVertexCirculator( topologicalFaces, vertexHoleIndex );
      if ( !circulator.isValid() )
        throw std::exception();

      if ( isLeftHole )
        circulator.goBoundaryCounterClockwise();
      else
        circulator.goBoundaryClockwise();

      int newFaceBoundaryLocalIndex = circulator.currentFaceIndex();
      int newFaceBoundaryIndexInMesh = circulator.currentFaceIndex() + startingFaceGlobalIndex;
      const QgsMeshFace &newFace = circulator.currentFace();
      int positionInNewFaces = vertexPositionInFace( vertexHoleIndex, newFace );
      if ( isLeftHole )
        positionInNewFaces = ( positionInNewFaces - 1 + newFace.size() ) % newFace.size(); //take the index just before

      if ( meshFaceBoundaryIndex != -1 )
      {
        const QgsMeshFace &meshFace = mesh->face( meshFaceBoundaryIndex );

        int positionInMeshFaceBoundary = vertexPositionInFace( *mesh, vertexHoleIndex, meshFaceBoundaryIndex );
        if ( !isLeftHole )
          positionInMeshFaceBoundary = ( positionInMeshFaceBoundary - 1 + meshFace.size() ) % meshFace.size(); //take the index just before

        int oldNeighbor = mEditor->topologicalMesh().neighborsOfFace( meshFaceBoundaryIndex ).at( positionInMeshFaceBoundary );
        mNeighborhoodChanges.append( {meshFaceBoundaryIndex, positionInMeshFaceBoundary, oldNeighbor, newFaceBoundaryIndexInMesh} );
      }

      mFacesNeighborhoodToAdd[newFaceBoundaryLocalIndex + startingFaceLocalIndex][positionInNewFaces] = meshFaceBoundaryIndex;
    }

    // search for the new face that are on the forcing line
    int vertexOnLine = hole.at( 0 );
    while ( vertexOnLine != hole.last() )
    {
      QgsMeshVertexCirculator circulatorOnLine( topologicalFaces, vertexOnLine );
      int nextVertex;
      if ( isLeftHole )
      {
        circulatorOnLine.goBoundaryClockwise();
        nextVertex = circulatorOnLine.oppositeVertexClockwise();
      }
      else
      {
        circulatorOnLine.goBoundaryCounterClockwise();
        nextVertex = circulatorOnLine.oppositeVertexCounterClockwise();
      }
      const QgsMeshFace &faceOnLine = circulatorOnLine.currentFace();
      int positionOnFaceOnTheLine = vertexPositionInFace( vertexOnLine, faceOnLine );
      if ( !isLeftHole )
        positionOnFaceOnTheLine = ( positionOnFaceOnTheLine - 1 + faceOnLine.size() ) % faceOnLine.size();

      newFacesOnLine.append( {circulatorOnLine.currentFaceIndex() + startingFaceLocalIndex, positionOnFaceOnTheLine} );
      vertexOnLine = nextVertex;
    }

    QgsMeshVertexCirculator circulatorOnStart( topologicalFaces, hole.first() );
    if ( isLeftHole )
      circulatorOnStart.goBoundaryCounterClockwise();
    else
      circulatorOnStart.goBoundaryClockwise();

    QgsMeshVertexCirculator circulatorOnEnd( topologicalFaces, hole.last() );
    if ( isLeftHole )
      circulatorOnEnd.goBoundaryClockwise();
    else
      circulatorOnEnd.goBoundaryCounterClockwise();


    extremeFaces = {circulatorOnStart.currentFaceIndex() + startingFaceLocalIndex,
                    circulatorOnEnd.currentFaceIndex() + startingFaceLocalIndex
                   };
    qDeleteAll( holeToFill );
  }
  catch ( ... )
  {
    qDeleteAll( holeToFill );
    return false;
  }



  return true;
}

bool QgsMeshEditForceByLine::finishForcingLine()
{
  QgsMesh *mesh = mEditor->topologicalMesh().mesh();

  QList<std::array<int, 2>> newLeftFacesOnLine;
  QList<std::array<int, 2>> newRightFacesOnLine;

  std::array<int, 2> extremeFacesOnLeft;
  std::array<int, 2> extremeFacesOnRight;

  if ( !triangulateHoles( mHoleOnLeft, mNeighborOnLeft, true, newLeftFacesOnLine, extremeFacesOnLeft ) )
    return false;
  if ( !triangulateHoles( mHoleOnRight, mNeighborOnRight, false, newRightFacesOnLine, extremeFacesOnRight ) )
    return false;

  //link the vertices that are on the line
  if ( newLeftFacesOnLine.count() != newRightFacesOnLine.count() )
    return false;

  for ( int i = 0; i < newLeftFacesOnLine.count(); ++i )
  {
    int leftFaceLocalIndex = newLeftFacesOnLine.at( i ).at( 0 );
    int leftPositionInFace = newLeftFacesOnLine.at( i ).at( 1 );
    int rightFaceLocalIndex = newRightFacesOnLine.at( i ).at( 0 );
    int rightPositionInFace = newRightFacesOnLine.at( i ).at( 1 );

    mFacesNeighborhoodToAdd[leftFaceLocalIndex][leftPositionInFace] = mAddedFacesFirstIndex + rightFaceLocalIndex;
    mFacesNeighborhoodToAdd[rightFaceLocalIndex][rightPositionInFace] = mAddedFacesFirstIndex + leftFaceLocalIndex;
  }

  // set the vertices to face
  mVertexToFaceToAdd.resize( mVerticesToAdd.count() );
  const int firstVertexIndex = mHoleOnLeft.first();
  if ( firstVertexIndex >= mesh->vertexCount() )
  {
    const int firstVertexLocalIndex = firstVertexIndex - mesh->vertexCount();
    mVertexToFaceToAdd[firstVertexLocalIndex] = newLeftFacesOnLine.first().at( 0 ) + mesh->faceCount();
  }

  const int lastVertexIndex = mHoleOnLeft.last();
  if ( lastVertexIndex >= mesh->vertexCount() )
  {
    const int lastVertexLocalIndex = lastVertexIndex - mesh->vertexCount();
    mVertexToFaceToAdd[lastVertexLocalIndex] = newLeftFacesOnLine.last().at( 0 ) + mesh->faceCount();
  }

  for ( int i = 0; i < mNewVerticesIndexesOnLine.count(); ++i )
  {
    mVertexToFaceToAdd[mNewVerticesIndexesOnLine.at( i )] = newLeftFacesOnLine.at( i ).at( 0 ) + mesh->faceCount();
  }

  for ( const int fi : std::as_const( mRemovedFaces ) )
  {
    mFacesToRemove.append( mesh->face( fi ) );
    mFaceIndexesToRemove.append( fi );
    mFacesNeighborhoodToRemove.append( mEditor->topologicalMesh().neighborsOfFace( fi ) );
  }

  mRemovedFaces.clear();
  mHoleOnLeft.clear();
  mNeighborOnLeft.clear();
  mHoleOnRight.clear();
  mNeighborOnRight.clear();
  mNewVerticesIndexesOnLine.clear();
  return true;
}


QString QgsMeshEditForceByPolylines::text() const
{
  return QObject::tr( "Force mesh by polyline" );
}

bool QgsMeshEditForceByPolylines::isFinished() const
{
  return mCurrentPolyline >= mPolylines.count() && QgsMeshEditForceByLine::isFinished();
}

QgsTopologicalMesh::Changes QgsMeshEditForceByPolylines::apply( QgsMeshEditor *meshEditor )
{
  if ( mPolylines.isEmpty() )
  {
    mIsFinished = true;
    return QgsTopologicalMesh::Changes();
  }

  if ( mCurrentPolyline == 0 && mCurrentSegment == 0 )
  {
    setInputLine( mPolylines.at( 0 ).at( 0 ),
                  mPolylines.at( 0 ).at( 1 ),
                  mTolerance, mNewVertexOnIntersection );

    incrementSegment();
    return QgsMeshEditForceByLine::apply( meshEditor );
  }

  if ( QgsMeshEditForceByLine::isFinished() )
  {
    setInputLine( mPolylines.at( mCurrentPolyline ).at( mCurrentSegment ),
                  mPolylines.at( mCurrentPolyline ).at( mCurrentSegment + 1 ),
                  mTolerance, mNewVertexOnIntersection );

    incrementSegment();
  }
  return QgsMeshEditForceByLine::apply( meshEditor );
}

void QgsMeshEditForceByPolylines::addLineFromGeometry( const QgsGeometry &geom )
{
  std::vector<const QgsCurve *> curves;
  if ( QgsWkbTypes::geometryType( geom.wkbType() ) == Qgis::GeometryType::Polygon )
  {
    std::vector< const QgsCurvePolygon * > polygons;
    if ( geom.isMultipart() )
    {
      const QgsMultiSurface *ms = qgsgeometry_cast< const QgsMultiSurface * >( geom.constGet() );
      for ( int i = 0; i < ms->numGeometries(); ++i )
        polygons.emplace_back( qgsgeometry_cast< const QgsCurvePolygon * >( ms->geometryN( i ) ) );
    }
    else
      polygons.emplace_back( qgsgeometry_cast< const QgsCurvePolygon * >( geom.constGet() ) );

    for ( const QgsCurvePolygon *polygon : polygons )
    {
      if ( !polygon )
        continue;

      if ( polygon->exteriorRing() )
        curves.emplace_back( polygon->exteriorRing() );

      for ( int i = 0; i < polygon->numInteriorRings(); ++i )
        curves.emplace_back( polygon->interiorRing( i ) );
    }
  }
  else
  {
    if ( geom.isMultipart() )
    {
      const QgsMultiCurve *mc = qgsgeometry_cast< const QgsMultiCurve * >( geom.constGet() );
      for ( int i = 0; i < mc->numGeometries(); ++i )
        curves.emplace_back( qgsgeometry_cast< const QgsCurve * >( mc->geometryN( i ) ) );
    }
    else
      curves.emplace_back( qgsgeometry_cast< const QgsCurve * >( geom.constGet() ) );
  }

  for ( const QgsCurve *curve : curves )
  {
    if ( !curve )
      continue;

    QgsPointSequence linePoints;
    curve->points( linePoints );
    if ( linePoints.count() < 2 )
      continue;
    if ( !curve->is3D() )
    {
      for ( int i = 0; i < linePoints.count(); ++i )
      {
        const QgsPoint &point = linePoints.at( i );
        linePoints[i] = QgsPoint( point.x(), point.y(), mDefaultZValue );
      }
    }
    mPolylines.append( linePoints );
  }
}

void QgsMeshEditForceByPolylines::addLinesFromGeometries( const QList<QgsGeometry> geometries )
{
  for ( const QgsGeometry &geom : geometries )
    addLineFromGeometry( geom );
}

void QgsMeshEditForceByLine::setTolerance( double tolerance )
{
  mTolerance = tolerance;
}

void QgsMeshEditForceByLine::setAddVertexOnIntersection( bool addVertex )
{
  mNewVertexOnIntersection = addVertex;
}

void QgsMeshEditForceByLine::setDefaultZValue( double defaultZValue )
{
  mDefaultZValue = defaultZValue;
}

void QgsMeshEditForceByLine::setInterpolateZValueOnMesh( bool interpolateZValueOnMesh )
{
  mInterpolateZValueOnMesh = interpolateZValueOnMesh;
}

void QgsMeshEditForceByPolylines::incrementSegment()
{
  mCurrentSegment++;
  if ( mCurrentSegment >= mPolylines.at( mCurrentPolyline ).count() - 1 )
  {
    mCurrentSegment = 0;
    mCurrentPolyline++;
  }
}
