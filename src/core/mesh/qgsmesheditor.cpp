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

#include "qgsmesheditor.h"
#include "qgsmeshdataprovider.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgslogger.h"
#include "qgsgeometryengine.h"

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

QgsMeshEditor::~QgsMeshEditor() = default;

QgsMeshEditingError QgsMeshEditor::initialize()
{
  QgsMeshEditingError error;
  mTopologicalMesh = QgsTopologicalMesh::createTopologicalMesh( mMesh, mMaximumVerticesPerFace, error );
  return error;
}


bool QgsMeshEditor::faceCanBeAdded( const QgsMeshFace &face )
{
  QgsMeshEditingError error;

  // Prepare and check the face
  QVector<QgsMeshFace> facesToAdd = prepareFaces( {face}, error );

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return false;

  // Check if there is toological error with the mesh
  QgsTopologicalMesh::TopologicalFaces topologicalFaces = mTopologicalMesh.createNewTopologicalFaces( facesToAdd, error );
  error = mTopologicalMesh.canFacesBeAdded( topologicalFaces );

  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return false;

  // Check geometry compatibility
  // With the topological check, we know that the new face is not included in an existing one
  // But maybe, the new face includes or intersects existing faces or free vertices, we need to check
  // First search for faces intersecting the bounding box of the new face.

  const QgsGeometry newFaceGeom = QgsMeshUtils::toGeometry( face, mTriangularMesh->vertices() );
  std::unique_ptr<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( newFaceGeom.constGet() ) );
  geomEngine->prepareGeometry();

  QgsRectangle boundingBox = newFaceGeom.boundingBox();
  QList<int> newFaceVerticesIndexes( face.toList() );
  QList<int> concernedFaceIndex = mTriangularMesh->nativeFaceIndexForRectangle( boundingBox );
  if ( !concernedFaceIndex.isEmpty() )
  {
    //for each concerned face, we take edges and, if no common vertex with the new face,
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
        //only test the edge that not contain a shared vertex
        for ( int i = 0; i < existingFaceSize; ++i )
        {
          int index1 = existingFace.at( i );
          int index2 = existingFace.at( ( i + 1 ) % existingFaceSize );
          if ( ! newFaceVerticesIndexes.contains( index1 )  && !newFaceVerticesIndexes.contains( index2 ) )
          {
            const QgsMeshVertex &v1 = mTriangularMesh->vertices().at( index1 );
            const QgsMeshVertex &v2 = mTriangularMesh->vertices().at( index2 );
            QgsGeometry edgeGeom = QgsGeometry( new QgsLineString( v1, v2 ) );
            if ( geomEngine->intersects( edgeGeom.constGet() ) )
              return false;
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

void QgsMeshEditor::applyEdit( QgsMeshEditor::Edit &edit )
{
  mTopologicalMesh.applyChanges( edit.topologicalChanges );
  mTriangularMesh->applyChanges( edit.triangularMeshChanges );
}

void QgsMeshEditor::reverseEdit( QgsMeshEditor::Edit &edit )
{
  mTopologicalMesh.reverseChanges( edit.topologicalChanges );
  mTriangularMesh->reverseChanges( edit.triangularMeshChanges );
}

void QgsMeshEditor::applyAddVertex( QgsMeshEditor::Edit &edit, const QgsMeshVertex &vertex )
{
  int includingFaceIndex = mTriangularMesh->nativeFaceIndexForPoint( mTriangularMesh->nativeToTriangularCoordinates( vertex ) );
  QgsTopologicalMesh::Changes topologicChanges;

  if ( includingFaceIndex != -1 )
    topologicChanges = mTopologicalMesh.addVertexInface( includingFaceIndex, vertex );
  else
    topologicChanges = mTopologicalMesh.addFreeVertex( vertex );

  applyEditOnTriangularMesh( edit, topologicChanges );
}

void QgsMeshEditor::applyRemoveVertexFillHole( QgsMeshEditor::Edit &edit, int vertexIndex )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.removeVertexFillHole( vertexIndex ) );
}

void QgsMeshEditor::applyRemoveVerticesWithoutFillHole( QgsMeshEditor::Edit &edit, const QList<int> &verticesIndexes )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.removeVertices( verticesIndexes ) );
}

void QgsMeshEditor::applyAddFaces( QgsMeshEditor::Edit &edit, const QgsTopologicalMesh::TopologicalFaces &faces )
{
  applyEditOnTriangularMesh( edit,  mTopologicalMesh.addFaces( faces ) );
}

void QgsMeshEditor::applyRemoveFaces( QgsMeshEditor::Edit &edit, const QList<int> &faceToRemoveIndex )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.removeFaces( faceToRemoveIndex ) );
}

void QgsMeshEditor::applyChangeZValue( QgsMeshEditor::Edit &edit, const QList<int> &verticesIndexes, const QList<double> &newValues )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.changeZValue( verticesIndexes, newValues ) );
}

void QgsMeshEditor::applyChangeXYValue( QgsMeshEditor::Edit &edit, const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.changeXYValue( verticesIndexes, newValues ) );
}

void QgsMeshEditor::applyEditOnTriangularMesh( QgsMeshEditor::Edit &edit, const QgsTopologicalMesh::Changes &topologicChanges )
{
  QgsTriangularMesh::Changes triangularChanges( topologicChanges, *mMesh );
  mTriangularMesh->applyChanges( triangularChanges );

  edit.topologicalChanges = topologicChanges;
  edit.triangularMeshChanges = triangularChanges;
}

bool QgsMeshEditor::checkConsistency() const
{
  if ( !mTopologicalMesh.checkConsistency() )
    return false;

  if ( mTriangularMesh->vertices().count() != mMesh->vertexCount() )
    return false;

  if ( mTriangularMesh->faceCentroids().count() != mMesh->faceCount() )
    return false;

  return true;
}

QgsMeshEditingError QgsMeshEditor::removeFaces( const QList<int> &facesToRemove )
{
  QgsMeshEditingError error = mTopologicalMesh.canFacesBeRemoved( facesToRemove );
  if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
    return error;

  mUndoStack->push( new QgsMeshLayerUndoCommandRemoveFaces( this, facesToRemove ) );

  return error;
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

    error = mTopologicalMesh.counterClockWiseFaces( face, mMesh );
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

  QgsTopologicalMesh::TopologicalFaces topologicalFaces = mTopologicalMesh.createNewTopologicalFaces( facesToAdd, error );

  error = mTopologicalMesh.canFacesBeAdded( topologicalFaces );

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
    mUndoStack->push( new QgsMeshLayerUndoCommandAddVertices( this, verticesInLayerCoordinate ) );
  }

  return vertices.count() - ignoredVertex;
}

int QgsMeshEditor::addPointsAsVertices( const QVector<QgsPoint> &point, double tolerance )
{
  return addVertices( point, tolerance );
}

QgsMeshEditingError QgsMeshEditor::removeVertices( const QList<int> &verticesToRemoveIndexes, bool fillHoles )
{
  QgsMeshEditingError error;

  QList<int> verticesIndexes = verticesToRemoveIndexes;

  if ( !fillHoles )
  {
    QSet<int> concernedNativeFaces;
    for ( const int vi : std::as_const( verticesIndexes ) )
      concernedNativeFaces.unite( mTopologicalMesh.facesAroundVertex( vi ).toSet() );

    error = mTopologicalMesh.canFacesBeRemoved( concernedNativeFaces.values() );
    if ( error.errorType != Qgis::MeshEditingErrorType::NoError )
      return error;
  }

  mUndoStack->push( new QgsMeshLayerUndoCommandRemoveVertices( this, verticesIndexes, fillHoles ) );

  return error;
}

void QgsMeshEditor::changeZValues( const QList<int> &verticesIndexes, const QList<double> &newZValues )
{
  mUndoStack->push( new QgsMeshLayerUndoCommandChangeZValue( this, verticesIndexes, newZValues ) );
}

void QgsMeshEditor::changeXYValues( const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues )
{
  // TODO : implement a check if it is possible to change the (x,y) values. For now, this check is made in the APP part
  mUndoStack->push( new QgsMeshLayerUndoCommandChangeXYValue( this, verticesIndexes, newValues ) );
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

QgsMeshLayerUndoCommandAddVertices::QgsMeshLayerUndoCommandAddVertices( QgsMeshEditor *meshEditor, const QVector<QgsMeshVertex> &vertices )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVertices( vertices )
{}

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
      mMeshEditor->applyAddVertex( edit, vertex );
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

QgsMeshLayerUndoCommandRemoveVertices::QgsMeshLayerUndoCommandRemoveVertices( QgsMeshEditor *meshEditor, const QList<int> &verticesToRemoveIndexes, bool fillHole )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVerticesToRemoveIndexes( verticesToRemoveIndexes )
  , mFillHole( fillHole )
{}

void QgsMeshLayerUndoCommandRemoveVertices::redo()
{
  if ( !mVerticesToRemoveIndexes.isEmpty() )
  {
    QgsMeshEditor::Edit edit;
    if ( mFillHole )
    {
      for ( const int &vertex : std::as_const( mVerticesToRemoveIndexes ) )
      {
        mMeshEditor->applyRemoveVertexFillHole( edit, vertex );
        mEdits.append( edit );
      }
    }
    else
    {
      mMeshEditor->applyRemoveVerticesWithoutFillHole( edit, mVerticesToRemoveIndexes );
      mEdits.append( edit );
    }

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
{}

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
{}

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

QList<int> QgsMeshEditor::freeVerticesIndexes()
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

QgsMeshLayerUndoCommandChangeZValue::QgsMeshLayerUndoCommandChangeZValue( QgsMeshEditor *meshEditor, const QList<int> &verticesIndexes, const QList<double> &newValues )
  : QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVerticesIndexes( verticesIndexes )
  , mNewValues( newValues )
{}

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
{}

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

