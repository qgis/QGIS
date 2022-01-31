/***************************************************************************
  qgsmesheditor.cpp - QgsMeshEditor

 ---------------------
 begin                : 8.6.2021
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
#include "qgsmesheditor.h"
#include "qgsmeshdataprovider.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgslogger.h"
#include "qgsgeometryengine.h"
#include "qgsmeshadvancedediting.h"
#include "qgsgeometryutils.h"

#include <poly2tri.h>

#include <QSet>


QgsMeshEditor::QgsMeshEditor( QgsMeshLayer *meshLayer )
  : QObject( meshLayer )
  , mMesh( meshLayer ? meshLayer->nativeMesh() : nullptr )
  , mTriangularMesh( meshLayer ? meshLayer->triangularMeshByLodIndex( 0 ) : nullptr )
  , mUndoStack( meshLayer ? meshLayer->undoStack() : nullptr )
{
  if ( meshLayer && meshLayer->dataProvider() )
    mMaximumVerticesPerFace = meshLayer->dataProvider()->maximumVerticesCountPerFace();

  if ( meshLayer )
    connect( mUndoStack, &QUndoStack::indexChanged, this, &QgsMeshEditor::meshEdited );
}

QgsMeshEditor::QgsMeshEditor( QgsMesh *nativeMesh, QgsTriangularMesh *triangularMesh, QObject *parent )
  : QObject( parent )
  , mMesh( nativeMesh )
  , mTriangularMesh( triangularMesh )
{
  mUndoStack = new QUndoStack( this );
  connect( mUndoStack, &QUndoStack::indexChanged, this, &QgsMeshEditor::meshEdited );
}

QgsMeshDatasetGroup *QgsMeshEditor::createZValueDatasetGroup()
{
  std::unique_ptr<QgsMeshDatasetGroup> zValueDatasetGroup = std::make_unique<QgsMeshVerticesElevationDatasetGroup>( tr( "vertices Z value" ), mMesh );
  mZValueDatasetGroup = zValueDatasetGroup.get();
  return zValueDatasetGroup.release();
}

QgsMeshEditor::~QgsMeshEditor() = default;

QgsMeshEditingError QgsMeshEditor::initialize()
{
  QgsMeshEditingError error;
  mTopologicalMesh = QgsTopologicalMesh::createTopologicalMesh( mMesh, mMaximumVerticesPerFace, error );
  mValidFacesCount = mMesh->faceCount();
  mValidVerticesCount = mMesh->vertexCount();
  return error;
}

void QgsMeshEditor::resetTriangularMesh( QgsTriangularMesh *triangularMesh )
{
  mTriangularMesh = triangularMesh;
}

bool QgsMeshEditor::isFaceGeometricallyCompatible( const QgsMeshFace &face )
{
  const QgsGeometry newFaceGeom = QgsMeshUtils::toGeometry( face, mTriangularMesh->vertices() );
  std::unique_ptr<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( newFaceGeom.constGet() ) );
  geomEngine->prepareGeometry();

  QgsRectangle boundingBox = newFaceGeom.boundingBox();
  QList<int> newFaceVerticesIndexes( face.toList() );
  int newFaceSize = face.count();
  QList<int> concernedFaceIndex = mTriangularMesh->nativeFaceIndexForRectangle( boundingBox );
  if ( !concernedFaceIndex.isEmpty() )
  {
    // for each concerned face, we take edges and, if no common vertex with the new face,
    // check is the edge intersects or is contained in the new face
    for ( const int faceIndex : concernedFaceIndex )
    {
      const QgsMeshFace &existingFace = mMesh->faces.at( faceIndex );
      int existingFaceSize = existingFace.count();
      bool shareVertex = false;
      for ( int i = 0; i < existingFaceSize; ++i )
      {
        if ( newFaceVerticesIndexes.contains( existingFace.at( i ) ) )
        {
          shareVertex = true;
          break;
        }
      }

      if ( shareVertex )
      {
        for ( int i = 0; i < existingFaceSize; ++i )
        {
          int index1 = existingFace.at( i );
          int index2 = existingFace.at( ( i + 1 ) % existingFaceSize );
          const QgsMeshVertex &v1 = mTriangularMesh->vertices().at( index1 );
          const QgsMeshVertex &v2 = mTriangularMesh->vertices().at( index2 );
          QgsGeometry edgeGeom = QgsGeometry( new QgsLineString( v1, v2 ) );

          if ( ! newFaceVerticesIndexes.contains( index1 )  && !newFaceVerticesIndexes.contains( index2 ) )
          {
            // test if the edge that not contains a shared vertex intersect the entire new face
            if ( geomEngine->intersects( edgeGeom.constGet() ) )
              return false;
          }
          else
          {
            for ( int vi = 0; vi < newFaceVerticesIndexes.count(); ++vi )
            {
              int vertInNewFace1 = newFaceVerticesIndexes.at( vi );
              int vertInNewFace2 = newFaceVerticesIndexes.at( ( vi + 1 ) % newFaceSize );
              if ( vertInNewFace1 != index1 && vertInNewFace2 != index2 && vertInNewFace1 != index2 && vertInNewFace2 != index1 )
              {
                const QgsMeshVertex &nv1 = mTriangularMesh->vertices().at( vertInNewFace1 );
                const QgsMeshVertex &nv2 = mTriangularMesh->vertices().at( vertInNewFace2 );
                QgsGeometry newEdgeGeom = QgsGeometry( new QgsLineString( nv1, nv2 ) );

                if ( newEdgeGeom.intersects( edgeGeom ) )
                  return false;
              }
            }
          }
        }
      }
      else
      {
        const QgsGeometry existingFaceGeom = QgsMeshUtils::toGeometry( existingFace, mTriangularMesh->vertices() );
        if ( geomEngine->intersects( existingFaceGeom.constGet() ) )
          return false;
      }
    }
  }

  // Then search for free vertices included in the new face
  const QList<int> &freeVertices = freeVerticesIndexes();

  for ( const int freeVertexIndex : freeVertices )
  {
    if ( newFaceVerticesIndexes.contains( freeVertexIndex ) )
      continue;

    const QgsMeshVertex &vertex = mTriangularMesh->vertices().at( freeVertexIndex );
    if ( geomEngine->contains( &vertex ) )
      return false;
  }

  return true;
}


bool QgsMeshEditor::faceCanBeAdded( const QgsMeshFace &face )
{
  QgsMeshEditingError error;

  // Prepare and check the face
  QVector<QgsMeshFace> facesToAdd = prepareFaces( {face}, error );

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return false;

  // Check if there is topological error with the mesh
  QgsTopologicalMesh::TopologicalFaces topologicalFaces = QgsTopologicalMesh::createNewTopologicalFaces( facesToAdd, true, error );
  error = mTopologicalMesh.facesCanBeAdded( topologicalFaces );

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return false;

  // Check geometry compatibility
  // With the topological check, we know that the new face is not included in an existing one
  // But maybe, the new face includes or intersects existing faces or free vertices, we need to check
  // First search for faces intersecting the bounding box of the new face.

  return isFaceGeometricallyCompatible( face );
}

void QgsMeshEditor::applyEdit( QgsMeshEditor::Edit &edit )
{
  mTopologicalMesh.applyChanges( edit.topologicalChanges );
  mTriangularMesh->applyChanges( edit.triangularMeshChanges );

  if ( mZValueDatasetGroup &&
       ( !edit.topologicalChanges.newVerticesZValues().isEmpty() ||
         !edit.topologicalChanges.verticesToRemoveIndexes().isEmpty() ||
         !edit.topologicalChanges.addedVertices().isEmpty() ) )
    mZValueDatasetGroup->setStatisticObsolete();

  updateElementsCount( edit.topologicalChanges );
}

void QgsMeshEditor::reverseEdit( QgsMeshEditor::Edit &edit )
{
  mTopologicalMesh.reverseChanges( edit.topologicalChanges );
  mTriangularMesh->reverseChanges( edit.triangularMeshChanges, *mMesh );

  if ( mZValueDatasetGroup &&
       ( !edit.topologicalChanges.newVerticesZValues().isEmpty() ||
         !edit.topologicalChanges.verticesToRemoveIndexes().isEmpty() ||
         !edit.topologicalChanges.addedVertices().isEmpty() ) )
    mZValueDatasetGroup->setStatisticObsolete();

  updateElementsCount( edit.topologicalChanges, false );
}

void QgsMeshEditor::applyAddVertex( QgsMeshEditor::Edit &edit, const QgsMeshVertex &vertex, double tolerance )
{
  QgsMeshVertex vertexInTriangularCoordinate =  mTriangularMesh->nativeToTriangularCoordinates( vertex );

  //check if edges is closest than the tolerance from the vertex
  int faceEdgeIntersect = -1;
  int edgePosition = -1;

  QgsTopologicalMesh::Changes topologicChanges;

  if ( edgeIsClose( vertexInTriangularCoordinate, tolerance, faceEdgeIntersect, edgePosition ) )
  {
    topologicChanges = mTopologicalMesh.insertVertexInFacesEdge( faceEdgeIntersect, edgePosition, vertex );
  }
  else
  {
    int includingFaceIndex = mTriangularMesh->nativeFaceIndexForPoint( vertexInTriangularCoordinate );

    if ( includingFaceIndex != -1 )
      topologicChanges = mTopologicalMesh.addVertexInFace( includingFaceIndex, vertex );
    else
      topologicChanges = mTopologicalMesh.addFreeVertex( vertex );
  }

  applyEditOnTriangularMesh( edit, topologicChanges );

  if ( mZValueDatasetGroup )
    mZValueDatasetGroup->setStatisticObsolete();

  updateElementsCount( edit.topologicalChanges );
}

bool QgsMeshEditor::applyRemoveVertexFillHole( QgsMeshEditor::Edit &edit, int vertexIndex )
{
  QgsTopologicalMesh::Changes changes = mTopologicalMesh.removeVertexFillHole( vertexIndex );

  if ( !changes.isEmpty() )
  {
    applyEditOnTriangularMesh( edit, changes );

    if ( mZValueDatasetGroup )
      mZValueDatasetGroup->setStatisticObsolete();

    updateElementsCount( edit.topologicalChanges );
    return true;
  }
  else
    return false;
}

void QgsMeshEditor::applyRemoveVerticesWithoutFillHole( QgsMeshEditor::Edit &edit, const QList<int> &verticesIndexes )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.removeVertices( verticesIndexes ) );

  if ( mZValueDatasetGroup )
    mZValueDatasetGroup->setStatisticObsolete();

  updateElementsCount( edit.topologicalChanges );
}

void QgsMeshEditor::applyAddFaces( QgsMeshEditor::Edit &edit, const QgsTopologicalMesh::TopologicalFaces &faces )
{
  applyEditOnTriangularMesh( edit,  mTopologicalMesh.addFaces( faces ) );

  updateElementsCount( edit.topologicalChanges );
}

void QgsMeshEditor::applyRemoveFaces( QgsMeshEditor::Edit &edit, const QList<int> &faceToRemoveIndex )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.removeFaces( faceToRemoveIndex ) );

  updateElementsCount( edit.topologicalChanges );
}

void QgsMeshEditor::applyChangeZValue( QgsMeshEditor::Edit &edit, const QList<int> &verticesIndexes, const QList<double> &newValues )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.changeZValue( verticesIndexes, newValues ) );

  if ( mZValueDatasetGroup )
    mZValueDatasetGroup->setStatisticObsolete();
}

void QgsMeshEditor::applyChangeXYValue( QgsMeshEditor::Edit &edit, const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.changeXYValue( verticesIndexes, newValues ) );
}

void QgsMeshEditor::applyFlipEdge( QgsMeshEditor::Edit &edit, int vertexIndex1, int vertexIndex2 )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.flipEdge( vertexIndex1, vertexIndex2 ) );

  updateElementsCount( edit.topologicalChanges );
}

void QgsMeshEditor::applyMerge( QgsMeshEditor::Edit &edit, int vertexIndex1, int vertexIndex2 )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.merge( vertexIndex1, vertexIndex2 ) );

  updateElementsCount( edit.topologicalChanges );
}

void QgsMeshEditor::applySplit( QgsMeshEditor::Edit &edit, int faceIndex )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.splitFace( faceIndex ) );

  updateElementsCount( edit.topologicalChanges );
}

void QgsMeshEditor::applyAdvancedEdit( QgsMeshEditor::Edit &edit, QgsMeshAdvancedEditing *editing )
{
  applyEditOnTriangularMesh( edit, editing->apply( this ) );

  updateElementsCount( edit.topologicalChanges );

  if ( mZValueDatasetGroup )
    mZValueDatasetGroup->setStatisticObsolete();
}

void QgsMeshEditor::applyEditOnTriangularMesh( QgsMeshEditor::Edit &edit, const QgsTopologicalMesh::Changes &topologicChanges )
{
  QgsTriangularMesh::Changes triangularChanges( topologicChanges, *mMesh );
  mTriangularMesh->applyChanges( triangularChanges );

  edit.topologicalChanges = topologicChanges;
  edit.triangularMeshChanges = triangularChanges;
}

void QgsMeshEditor::updateElementsCount( const QgsTopologicalMesh::Changes &changes, bool apply )
{
  if ( apply )
  {
    mValidFacesCount += changes.addedFaces().count() - changes.removedFaces().count();
    mValidVerticesCount += changes.addedVertices().count() - changes.verticesToRemoveIndexes().count();
  }
  else
  {
    //reverse
    mValidFacesCount -= changes.addedFaces().count() - changes.removedFaces().count();
    mValidVerticesCount -= changes.addedVertices().count() - changes.verticesToRemoveIndexes().count();
  }
}

bool QgsMeshEditor::checkConsistency( QgsMeshEditingError &error ) const
{
  error = mTopologicalMesh.checkConsistency();
  switch ( error.errorType )
  {
    case Qgis::MeshEditingErrorType::NoError:
      break;
    case Qgis::MeshEditingErrorType::InvalidFace:
    case Qgis::MeshEditingErrorType::TooManyVerticesInFace:
    case Qgis::MeshEditingErrorType::FlatFace:
    case Qgis::MeshEditingErrorType::UniqueSharedVertex:
    case Qgis::MeshEditingErrorType::InvalidVertex:
    case Qgis::MeshEditingErrorType::ManifoldFace:
      return false;
  }

  if ( mTriangularMesh->vertices().count() != mMesh->vertexCount() )
    return false;

  if ( mTriangularMesh->faceCentroids().count() != mMesh->faceCount() )
    return false;

  return true;
}

bool QgsMeshEditor::edgeIsClose( QgsPointXY point, double tolerance, int &faceIndex, int &edgePosition )
{
  QgsRectangle toleranceZone( point.x() - tolerance,
                              point.y() - tolerance,
                              point.x() + tolerance,
                              point.y() + tolerance );

  edgePosition = -1;
  double minDist = std::numeric_limits<double>::max();
  const QList<int> &nativeFaces = mTriangularMesh->nativeFaceIndexForRectangle( toleranceZone );
  double epsilon = std::numeric_limits<double>::epsilon() * tolerance;
  for ( const int nativeFaceIndex : nativeFaces )
  {
    const QgsMeshFace &face = mMesh->face( nativeFaceIndex );
    const int faceSize = face.size();
    for ( int i = 0; i < faceSize; ++i )
    {
      const QgsMeshVertex &v1 = mTriangularMesh->vertices().at( face.at( i ) );
      const QgsMeshVertex &v2 = mTriangularMesh->vertices().at( face.at( ( i + 1 ) % faceSize ) );

      double mx, my;
      double dist = sqrt( QgsGeometryUtils::sqrDistToLine( point.x(),
                          point.y(),
                          v1.x(),
                          v1.y(),
                          v2.x(),
                          v2.y(),
                          mx,
                          my,
                          epsilon ) );

      if ( dist < tolerance && dist < minDist )
      {
        faceIndex = nativeFaceIndex;
        edgePosition = i;
        minDist = dist;
      }
    }
  }

  if ( edgePosition != -1 )
    return true;

  return false;

}

int QgsMeshEditor::validFacesCount() const
{
  return mValidFacesCount;
}

int QgsMeshEditor::validVerticesCount() const
{
  return mValidVerticesCount;
}

int QgsMeshEditor::maximumVerticesPerFace() const
{
  return mMaximumVerticesPerFace;
}

QgsMeshEditingError QgsMeshEditor::removeFaces( const QList<int> &facesToRemove )
{
  QgsMeshEditingError error = mTopologicalMesh.facesCanBeRemoved( facesToRemove );
  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return error;

  mUndoStack->push( new QgsMeshLayerUndoCommandRemoveFaces( this, facesToRemove ) );

  return error;
}

bool QgsMeshEditor::edgeCanBeFlipped( int vertexIndex1, int vertexIndex2 ) const
{
  return mTopologicalMesh.edgeCanBeFlipped( vertexIndex1, vertexIndex2 );
}

void QgsMeshEditor::flipEdge( int vertexIndex1, int vertexIndex2 )
{
  if ( !edgeCanBeFlipped( vertexIndex1, vertexIndex2 ) )
    return;

  mUndoStack->push( new QgsMeshLayerUndoCommandFlipEdge( this, vertexIndex1, vertexIndex2 ) );
}

bool QgsMeshEditor::canBeMerged( int vertexIndex1, int vertexIndex2 ) const
{
  return mTopologicalMesh.canBeMerged( vertexIndex1, vertexIndex2 );
}

void QgsMeshEditor::merge( int vertexIndex1, int vertexIndex2 )
{
  if ( !canBeMerged( vertexIndex1, vertexIndex2 ) )
    return;

  mUndoStack->push( new QgsMeshLayerUndoCommandMerge( this, vertexIndex1, vertexIndex2 ) );
}

bool QgsMeshEditor::faceCanBeSplit( int faceIndex ) const
{
  return mTopologicalMesh.canBeSplit( faceIndex );
}

int QgsMeshEditor::splitFaces( const QList<int> &faceIndexes )
{
  QList<int> faceIndexesSplittable;

  for ( const int faceIndex : faceIndexes )
    if ( faceCanBeSplit( faceIndex ) )
      faceIndexesSplittable.append( faceIndex );

  if ( faceIndexesSplittable.isEmpty() )
    return 0;

  mUndoStack->push( new QgsMeshLayerUndoCommandSplitFaces( this, faceIndexesSplittable ) );

  return faceIndexesSplittable.count();
}

QVector<QgsMeshFace> QgsMeshEditor::prepareFaces( const QVector<QgsMeshFace> &faces, QgsMeshEditingError &error )
{
  QVector<QgsMeshFace> treatedFaces = faces;

  // here we could add later some filters, for example, removing faces intersecting with existing one

  for ( int i = 0; i < treatedFaces.count(); ++i )
  {
    QgsMeshFace &face = treatedFaces[i];
    if ( mMaximumVerticesPerFace != 0 && face.count() > mMaximumVerticesPerFace )
    {
      error = QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, i );
      break;
    }

    error = QgsTopologicalMesh::counterClockwiseFaces( face, mMesh );
    if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
      break;
  }

  return treatedFaces;
}

QgsMeshEditingError QgsMeshEditor::addFaces( const QVector<QVector<int> > &faces )
{
  QgsMeshEditingError error;
  QVector<QgsMeshFace> facesToAdd = prepareFaces( faces, error );

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return error;

  QgsTopologicalMesh::TopologicalFaces topologicalFaces = QgsTopologicalMesh::createNewTopologicalFaces( facesToAdd, true, error );

  error = mTopologicalMesh.facesCanBeAdded( topologicalFaces );

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return error;

  mUndoStack->push( new QgsMeshLayerUndoCommandAddFaces( this, topologicalFaces ) );

  return error;
}

QgsMeshEditingError QgsMeshEditor::addFace( const QVector<int> &vertexIndexes )
{
  return addFaces( {vertexIndexes} );
}

int QgsMeshEditor::addVertices( const QVector<QgsMeshVertex> &vertices, double tolerance )
{
  QVector<QgsMeshVertex> verticesInLayerCoordinate( vertices.count() );
  int ignoredVertex = 0;
  for ( int i = 0; i < vertices.count(); ++i )
  {
    const QgsPointXY &pointInTriangularMesh = vertices.at( i );
    bool isTooClose = false;
    int triangleIndex = mTriangularMesh->faceIndexForPoint_v2( pointInTriangularMesh );
    if ( triangleIndex != -1 )
    {
      const QgsMeshFace face = mTriangularMesh->triangles().at( triangleIndex );
      for ( int j = 0; j < 3; ++j )
      {
        const QgsPointXY &facePoint = mTriangularMesh->vertices().at( face.at( j ) );
        double dist = pointInTriangularMesh.distance( facePoint );
        if ( dist < tolerance )
        {
          isTooClose = true;
          break;
        }
      }
    }

    if ( !isTooClose )
      verticesInLayerCoordinate[i] = mTriangularMesh->triangularToNativeCoordinates( vertices.at( i ) );
    else
      verticesInLayerCoordinate[i] = QgsMeshVertex();

    if ( verticesInLayerCoordinate.at( i ).isEmpty() )
      ignoredVertex++;
  }

  if ( ignoredVertex < vertices.count() )
  {
    mUndoStack->push( new QgsMeshLayerUndoCommandAddVertices( this, verticesInLayerCoordinate, tolerance ) );
  }

  int effectivlyAddedVertex = vertices.count() - ignoredVertex;

  return effectivlyAddedVertex;
}

int QgsMeshEditor::addPointsAsVertices( const QVector<QgsPoint> &point, double tolerance )
{
  return addVertices( point, tolerance );
}

QgsMeshEditingError QgsMeshEditor::removeVerticesWithoutFillHoles( const QList<int> &verticesToRemoveIndexes )
{
  QgsMeshEditingError error;

  QList<int> verticesIndexes = verticesToRemoveIndexes;

  QSet<int> concernedNativeFaces;
  for ( const int vi : std::as_const( verticesIndexes ) )
    concernedNativeFaces.unite( qgis::listToSet( mTopologicalMesh.facesAroundVertex( vi ) ) );

  error = mTopologicalMesh.facesCanBeRemoved( concernedNativeFaces.values() );

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return error;

  mUndoStack->push( new QgsMeshLayerUndoCommandRemoveVerticesWithoutFillHoles( this, verticesIndexes ) );
  return error;
}

QList<int> QgsMeshEditor::removeVerticesFillHoles( const QList<int> &verticesToRemoveIndexes )
{
  QList<int> remainingVertices;
  mUndoStack->push( new QgsMeshLayerUndoCommandRemoveVerticesFillHoles( this, verticesToRemoveIndexes, &remainingVertices ) );

  return remainingVertices;
}


void QgsMeshEditor::changeZValues( const QList<int> &verticesIndexes, const QList<double> &newZValues )
{
  mUndoStack->push( new QgsMeshLayerUndoCommandChangeZValue( this, verticesIndexes, newZValues ) );
}

bool QgsMeshEditor::canBeTransformed( const QList<int> &facesToCheck, const std::function<const QgsMeshVertex( int )> &transformFunction ) const
{
  for ( const int faceIndex : facesToCheck )
  {
    const QgsMeshFace &face = mMesh->face( faceIndex );
    int faceSize = face.count();
    QVector<QgsPointXY> pointsInTriangularMeshCoordinate( faceSize );
    QVector<QgsPointXY> points( faceSize );
    for ( int i = 0; i < faceSize; ++i )
    {
      int ip0 =  face[i];
      int ip1 = face[( i + 1 ) % faceSize];
      int ip2 = face[( i + 2 ) % faceSize];

      QgsMeshVertex p0 = transformFunction( ip0 );
      QgsMeshVertex p1 = transformFunction( ip1 );
      QgsMeshVertex p2 = transformFunction( ip2 );

      double ux = p0.x() - p1.x();
      double uy = p0.y() - p1.y();
      double vx = p2.x() - p1.x();
      double vy = p2.y() - p1.y();

      double crossProduct = ux * vy - uy * vx;
      if ( crossProduct >= 0 ) //if cross product>0, we have two edges clockwise
        return false;
      pointsInTriangularMeshCoordinate[i] = mTriangularMesh->nativeToTriangularCoordinates( p0 );
      points[i] = p0;
    }

    const QgsGeometry &deformedFace = QgsGeometry::fromPolygonXY( {points} );

    // now test if the deformed face contain something else
    QList<int> otherFaceIndexes =
      mTriangularMesh->nativeFaceIndexForRectangle( QgsGeometry::fromPolygonXY( {pointsInTriangularMeshCoordinate} ).boundingBox() );

    for ( const int otherFaceIndex : otherFaceIndexes )
    {
      const QgsMeshFace &otherFace = mMesh->face( otherFaceIndex );
      int existingFaceSize = otherFace.count();
      bool shareVertex = false;
      for ( int i = 0; i < existingFaceSize; ++i )
      {
        if ( face.contains( otherFace.at( i ) ) )
        {
          shareVertex = true;
          break;
        }
      }
      if ( shareVertex )
      {
        //only test the edge that not contains a shared vertex
        for ( int i = 0; i < existingFaceSize; ++i )
        {
          int index1 = otherFace.at( i );
          int index2 = otherFace.at( ( i + 1 ) % existingFaceSize );
          if ( ! face.contains( index1 )  && !face.contains( index2 ) )
          {
            const QgsPointXY &v1 = transformFunction( index1 );
            const QgsPointXY &v2 =  transformFunction( index2 );
            QgsGeometry edgeGeom = QgsGeometry::fromPolylineXY( { v1, v2} );
            if ( deformedFace.intersects( edgeGeom ) )
              return false;
          }
        }
      }
      else
      {
        QVector<QgsPointXY> otherPoints( existingFaceSize );
        for ( int i = 0; i < existingFaceSize; ++i )
          otherPoints[i] = transformFunction( otherFace.at( i ) );
        const QgsGeometry existingFaceGeom = QgsGeometry::fromPolygonXY( {otherPoints } );
        if ( deformedFace.intersects( existingFaceGeom ) )
          return false;
      }
    }

    const QList<int> freeVerticesIndex = freeVerticesIndexes();
    for ( const int vertexIndex : freeVerticesIndex )
    {
      const QgsPointXY &mapPoint = transformFunction( vertexIndex ); //free vertices can be transformed
      if ( deformedFace.contains( &mapPoint ) )
        return false;
    }
  }

  // free vertices
  const QList<int> freeVerticesIndex = freeVerticesIndexes();
  for ( const int vertexIndex : freeVerticesIndex )
  {
    const QgsMeshVertex &newFreeVertexPosition = transformFunction( vertexIndex ); // transformed free vertex
    const QgsMeshVertex pointInTriangularCoord = mTriangularMesh->nativeToTriangularCoordinates( newFreeVertexPosition );
    const int originalIncludingFace = mTriangularMesh->nativeFaceIndexForPoint( pointInTriangularCoord );

    if ( originalIncludingFace != -1 )
    {
      // That means two things: the free vertex is moved AND is included in a face before transform
      // Before returning false, we need to check if the vertex is still in the face after transform
      const QgsMeshFace &face = mMesh->face( originalIncludingFace );
      int faceSize = face.count();
      QVector<QgsPointXY> points( faceSize );
      for ( int i = 0; i < faceSize; ++i )
        points[i] = transformFunction( face.at( i ) );

      const QgsGeometry &deformedFace = QgsGeometry::fromPolygonXY( {points} );
      const QgsPointXY ptXY( newFreeVertexPosition );
      if ( deformedFace.contains( &ptXY ) )
        return false;
    }
  }

  return true;
}

void QgsMeshEditor::changeXYValues( const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues )
{
  // TODO : implement a check if it is possible to change the (x,y) values. For now, this check is made in the APP part
  mUndoStack->push( new QgsMeshLayerUndoCommandChangeXYValue( this, verticesIndexes, newValues ) );
}

void QgsMeshEditor::changeCoordinates( const QList<int> &verticesIndexes, const QList<QgsPoint> &newCoordinates )
{
  mUndoStack->push( new QgsMeshLayerUndoCommandChangeCoordinates( this, verticesIndexes, newCoordinates ) );
}

void QgsMeshEditor::advancedEdit( QgsMeshAdvancedEditing *editing )
{
  mUndoStack->push( new QgsMeshLayerUndoCommandAdvancedEditing( this, editing ) );
}

void QgsMeshEditor::stopEditing()
{
  mTopologicalMesh.reindex();
  mUndoStack->clear();
}

QgsMeshLayerUndoCommandMeshEdit::QgsMeshLayerUndoCommandMeshEdit( QgsMeshEditor *meshEditor )
  : mMeshEditor( meshEditor )
{
}

void QgsMeshLayerUndoCommandMeshEdit::undo()
{
  if ( mMeshEditor.isNull() )
    return;

  for ( int i = mEdits.count() - 1; i >= 0; --i )
    mMeshEditor->reverseEdit( mEdits[i] );
}

void QgsMeshLayerUndoCommandMeshEdit::redo()
{
  if ( mMeshEditor.isNull() )
    return;

  for ( QgsMeshEditor::Edit &edit : mEdits )
    mMeshEditor->applyEdit( edit );
}

QgsMeshLayerUndoCommandAddVertices::QgsMeshLayerUndoCommandAddVertices( QgsMeshEditor *meshEditor, const QVector<QgsMeshVertex> &vertices, double tolerance )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVertices( vertices )
  , mTolerance( tolerance )
{
  setText( QObject::tr( "Add %n vertices", nullptr, mVertices.count() ) );
}

void QgsMeshLayerUndoCommandAddVertices::redo()
{
  if ( !mVertices.isEmpty() )
  {
    for ( int i = 0; i < mVertices.count(); ++i )
    {
      const QgsMeshVertex &vertex = mVertices.at( i );
      if ( vertex.isEmpty() )
        continue;
      QgsMeshEditor::Edit edit;
      mMeshEditor->applyAddVertex( edit, vertex, mTolerance );
      mEdits.append( edit );
    }
    mVertices.clear(); //not needed anymore, changes are store in mEdits
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}

QgsMeshLayerUndoCommandRemoveVerticesFillHoles::QgsMeshLayerUndoCommandRemoveVerticesFillHoles(
  QgsMeshEditor *meshEditor,
  const QList<int> &verticesToRemoveIndexes,
  QList<int> *remainingVerticesPointer )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVerticesToRemoveIndexes( verticesToRemoveIndexes )
  , mRemainingVerticesPointer( remainingVerticesPointer )
{
  setText( QObject::tr( "Remove %n vertices filling holes", nullptr, verticesToRemoveIndexes.count() ) );
}

void QgsMeshLayerUndoCommandRemoveVerticesFillHoles::redo()
{
  int initialVertexCount = mVerticesToRemoveIndexes.count();
  if ( !mVerticesToRemoveIndexes.isEmpty() )
  {
    QgsMeshEditor::Edit edit;
    QList<int> vertexToRetry;
    while ( !mVerticesToRemoveIndexes.isEmpty() )
    {
      // try again and again until there is no vertices to remove anymore or nothing is removed.
      for ( const int &vertex : std::as_const( mVerticesToRemoveIndexes ) )
      {
        if ( mMeshEditor->applyRemoveVertexFillHole( edit, vertex ) )
          mEdits.append( edit );
        else
          vertexToRetry.append( vertex );
      }

      if ( vertexToRetry.count() == mVerticesToRemoveIndexes.count() )
        break;
      else
        mVerticesToRemoveIndexes = vertexToRetry;
    }

    if ( initialVertexCount == mVerticesToRemoveIndexes.count() )
      setObsolete( true );

    if ( mRemainingVerticesPointer != nullptr )
      *mRemainingVerticesPointer = mVerticesToRemoveIndexes;

    mRemainingVerticesPointer = nullptr;

    mVerticesToRemoveIndexes.clear(); //not needed anymore, changes are store in mEdits
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}


QgsMeshLayerUndoCommandRemoveVerticesWithoutFillHoles::QgsMeshLayerUndoCommandRemoveVerticesWithoutFillHoles(
  QgsMeshEditor *meshEditor,
  const QList<int> &verticesToRemoveIndexes )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVerticesToRemoveIndexes( verticesToRemoveIndexes )
{
  setText( QObject::tr( "Remove %n vertices without filling holes", nullptr, verticesToRemoveIndexes.count() ) ) ;
}

void QgsMeshLayerUndoCommandRemoveVerticesWithoutFillHoles::redo()
{
  if ( !mVerticesToRemoveIndexes.isEmpty() )
  {
    QgsMeshEditor::Edit edit;

    mMeshEditor->applyRemoveVerticesWithoutFillHole( edit, mVerticesToRemoveIndexes );
    mEdits.append( edit );

    mVerticesToRemoveIndexes.clear(); //not needed anymore, changes are store in mEdits
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}

QgsMeshLayerUndoCommandAddFaces::QgsMeshLayerUndoCommandAddFaces( QgsMeshEditor *meshEditor, QgsTopologicalMesh::TopologicalFaces &faces )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mFaces( faces )
{
  setText( QObject::tr( "Add %n face(s)", nullptr, faces.meshFaces().count() ) );
}

void QgsMeshLayerUndoCommandAddFaces::redo()
{
  if ( !mFaces.meshFaces().isEmpty() )
  {
    QgsMeshEditor::Edit edit;
    mMeshEditor->applyAddFaces( edit, mFaces );
    mEdits.append( edit );

    mFaces.clear(); //not needed anymore, now changes are store in edit
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}

QgsMeshLayerUndoCommandRemoveFaces::QgsMeshLayerUndoCommandRemoveFaces( QgsMeshEditor *meshEditor, const QList<int> &facesToRemoveIndexes )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mfacesToRemoveIndexes( facesToRemoveIndexes )
{
  setText( QObject::tr( "Remove %n face(s)", nullptr, facesToRemoveIndexes.count() ) );
}

void QgsMeshLayerUndoCommandRemoveFaces::redo()
{
  if ( !mfacesToRemoveIndexes.isEmpty() )
  {
    QgsMeshEditor::Edit edit;
    mMeshEditor->applyRemoveFaces( edit, mfacesToRemoveIndexes );
    mEdits.append( edit );

    mfacesToRemoveIndexes.clear(); //not needed anymore, now changes are store in edit
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}

QgsMeshEditingError::QgsMeshEditingError(): errorType( Qgis::MeshEditingErrorType::NoError ), elementIndex( -1 ) {}

QgsMeshEditingError::QgsMeshEditingError( Qgis::MeshEditingErrorType type, int elementIndex ): errorType( type ), elementIndex( elementIndex ) {}

QgsRectangle QgsMeshEditor::extent() const
{
  return mTriangularMesh->nativeExtent();
}

bool QgsMeshEditor::isModified() const
{
  if ( mUndoStack )
    return !mUndoStack->isClean();

  return false;
}

bool QgsMeshEditor::reindex( bool renumbering )
{
  mTopologicalMesh.reindex();
  mUndoStack->clear();
  QgsMeshEditingError error = initialize();
  mValidFacesCount = mMesh->faceCount();
  mValidVerticesCount = mMesh->vertexCount();

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return false;

  if ( renumbering )
  {
    if ( !mTopologicalMesh.renumber() )
      return false;

    QgsMeshEditingError error = initialize();
    return error.errorType == Qgis::MeshEditingErrorType::NoError;
  }

  else
    return true;
}

QList<int> QgsMeshEditor::freeVerticesIndexes() const
{
  return mTopologicalMesh.freeVerticesIndexes();
}

bool QgsMeshEditor::isVertexOnBoundary( int vertexIndex ) const
{
  return mTopologicalMesh.isVertexOnBoundary( vertexIndex );
}

bool QgsMeshEditor::isVertexFree( int vertexIndex ) const
{
  return mTopologicalMesh.isVertexFree( vertexIndex );
}

QgsMeshVertexCirculator QgsMeshEditor::vertexCirculator( int vertexIndex ) const
{
  return mTopologicalMesh.vertexCirculator( vertexIndex );
}

QgsTopologicalMesh &QgsMeshEditor::topologicalMesh()
{
  return mTopologicalMesh;
}

QgsTriangularMesh *QgsMeshEditor::triangularMesh()
{
  return mTriangularMesh;
}

QgsMeshLayerUndoCommandChangeZValue::QgsMeshLayerUndoCommandChangeZValue( QgsMeshEditor *meshEditor, const QList<int> &verticesIndexes, const QList<double> &newValues )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVerticesIndexes( verticesIndexes )
  , mNewValues( newValues )
{
  setText( QObject::tr( "Change %n vertices Z Value", nullptr, verticesIndexes.count() ) );
}

void QgsMeshLayerUndoCommandChangeZValue::redo()
{
  if ( !mVerticesIndexes.isEmpty() )
  {
    QgsMeshEditor::Edit edit;
    mMeshEditor->applyChangeZValue( edit, mVerticesIndexes, mNewValues );
    mEdits.append( edit );
    mVerticesIndexes.clear();
    mNewValues.clear();
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}

QgsMeshLayerUndoCommandChangeXYValue::QgsMeshLayerUndoCommandChangeXYValue( QgsMeshEditor *meshEditor, const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVerticesIndexes( verticesIndexes )
  , mNewValues( newValues )
{
  setText( QObject::tr( "Move %n vertices", nullptr, verticesIndexes.count() ) );
}

void QgsMeshLayerUndoCommandChangeXYValue::redo()
{
  if ( !mVerticesIndexes.isEmpty() )
  {
    QgsMeshEditor::Edit edit;
    mMeshEditor->applyChangeXYValue( edit, mVerticesIndexes, mNewValues );
    mEdits.append( edit );
    mVerticesIndexes.clear();
    mNewValues.clear();
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}


QgsMeshLayerUndoCommandChangeCoordinates::QgsMeshLayerUndoCommandChangeCoordinates( QgsMeshEditor *meshEditor, const QList<int> &verticesIndexes, const QList<QgsPoint> &newCoordinates )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVerticesIndexes( verticesIndexes )
  , mNewCoordinates( newCoordinates )
{
  setText( QObject::tr( "Transform %n vertices coordinates", nullptr, verticesIndexes.count() ) );
}

void QgsMeshLayerUndoCommandChangeCoordinates::redo()
{
  if ( !mVerticesIndexes.isEmpty() )
  {
    QgsMeshEditor::Edit editXY;
    QList<QgsPointXY> newXY;
    newXY.reserve( mNewCoordinates.count() );
    QgsMeshEditor::Edit editZ;
    QList<double> newZ;
    newZ.reserve( mNewCoordinates.count() );

    for ( const QgsPoint &pt : std::as_const( mNewCoordinates ) )
    {
      newXY.append( pt );
      newZ.append( pt.z() );
    }

    mMeshEditor->applyChangeXYValue( editXY, mVerticesIndexes, newXY );
    mEdits.append( editXY );
    mMeshEditor->applyChangeZValue( editZ, mVerticesIndexes, newZ );
    mEdits.append( editZ );
    mVerticesIndexes.clear();
    mNewCoordinates.clear();
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}



QgsMeshLayerUndoCommandFlipEdge::QgsMeshLayerUndoCommandFlipEdge( QgsMeshEditor *meshEditor, int vertexIndex1, int vertexIndex2 )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVertexIndex1( vertexIndex1 )
  , mVertexIndex2( vertexIndex2 )
{
  setText( QObject::tr( "Flip edge" ) );
}

void QgsMeshLayerUndoCommandFlipEdge::redo()
{
  if ( mVertexIndex1 >= 0 && mVertexIndex2 >= 0 )
  {
    QgsMeshEditor::Edit edit;
    mMeshEditor->applyFlipEdge( edit, mVertexIndex1, mVertexIndex2 );
    mEdits.append( edit );
    mVertexIndex1 = -1;
    mVertexIndex2 = -1;
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}

QgsMeshLayerUndoCommandMerge::QgsMeshLayerUndoCommandMerge( QgsMeshEditor *meshEditor, int vertexIndex1, int vertexIndex2 )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVertexIndex1( vertexIndex1 )
  , mVertexIndex2( vertexIndex2 )
{
  setText( QObject::tr( "Merge faces" ) );
}

void QgsMeshLayerUndoCommandMerge::redo()
{
  if ( mVertexIndex1 >= 0 && mVertexIndex2 >= 0 )
  {
    QgsMeshEditor::Edit edit;
    mMeshEditor->applyMerge( edit, mVertexIndex1, mVertexIndex2 );
    mEdits.append( edit );
    mVertexIndex1 = -1;
    mVertexIndex2 = -1;
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}

QgsMeshLayerUndoCommandSplitFaces::QgsMeshLayerUndoCommandSplitFaces( QgsMeshEditor *meshEditor, const QList<int> &faceIndexes )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mFaceIndexes( faceIndexes )
{
  setText( QObject::tr( "Split %n face(s)", nullptr, faceIndexes.count() ) );
}

void QgsMeshLayerUndoCommandSplitFaces::redo()
{
  if ( !mFaceIndexes.isEmpty() )
  {
    for ( int faceIndex : std::as_const( mFaceIndexes ) )
    {
      QgsMeshEditor::Edit edit;
      mMeshEditor->applySplit( edit, faceIndex );
      mEdits.append( edit );
    }
    mFaceIndexes.clear();
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}

QgsMeshLayerUndoCommandAdvancedEditing::QgsMeshLayerUndoCommandAdvancedEditing( QgsMeshEditor *meshEditor, QgsMeshAdvancedEditing *advancdEdit )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mAdvancedEditing( advancdEdit )
{
  setText( advancdEdit->text() );
}

void QgsMeshLayerUndoCommandAdvancedEditing::redo()
{
  if ( mAdvancedEditing )
  {
    QgsMeshEditor::Edit edit;
    while ( !mAdvancedEditing->isFinished() )
    {
      mMeshEditor->applyAdvancedEdit( edit, mAdvancedEditing );
      mEdits.append( edit );
    }

    mAdvancedEditing = nullptr;
  }
  else
  {
    for ( QgsMeshEditor::Edit &edit : mEdits )
      mMeshEditor->applyEdit( edit );
  }
}
