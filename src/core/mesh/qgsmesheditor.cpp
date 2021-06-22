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
#include "qgslogger.h"

#include <poly2tri.h>

#include <QSet>


QgsMeshEditor::QgsMeshEditor( QgsMeshLayer *meshLayer ):
  QObject( meshLayer )
  , mMesh( meshLayer ? meshLayer->nativeMesh() : nullptr )
  , mTriangularMesh( meshLayer ? meshLayer->triangularMeshByLodIndex( 0 ) : nullptr )
  , mUndoStack( meshLayer ? meshLayer->undoStack() : nullptr )
{}

QgsMeshEditor::QgsMeshEditor( QgsMesh *nativeMesh, QgsTriangularMesh *triangularMesh, QObject *parent ):
  QObject( parent )
  , mMesh( nativeMesh )
  , mTriangularMesh( triangularMesh )
{
  mUndoStack = new QUndoStack( this );
}

QgsMeshEditor::~QgsMeshEditor() = default;

QgsMeshEditingError QgsMeshEditor::initialize()
{
  QgsMeshEditingError error;
  mTopologicalMesh = QgsTopologicalMesh::createTopologicalMesh( mMesh, error );
  return error;
}

void QgsMeshEditor::applyEdit( QgsMeshEditor::Edit &edit )
{
  mTopologicalMesh.applyChanges( edit.topologicalChanges );
  mTriangularMesh->applyChanges( edit.triangularMeshChanges );

  emit meshEdited();
}

void QgsMeshEditor::reverseEdit( QgsMeshEditor::Edit &edit )
{
  mTopologicalMesh.reverseChanges( edit.topologicalChanges );
  mTriangularMesh->reverseChanges( edit.triangularMeshChanges );

  emit meshEdited();
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

void QgsMeshEditor::applyRemoveVertex( QgsMeshEditor::Edit &edit, int vertexIndex, bool fillHole )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.removeVertex( vertexIndex, fillHole ) );

  emit meshEdited();
}

void QgsMeshEditor::applyAddFaces( QgsMeshEditor::Edit &edit, const QgsTopologicalMesh::TopologicalFaces &faces )
{
  applyEditOnTriangularMesh( edit,  mTopologicalMesh.addFaces( faces ) );

  emit meshEdited();
}

void QgsMeshEditor::applyRemoveFaces( QgsMeshEditor::Edit &edit, const QList<int> faceToRemoveIndex )
{
  applyEditOnTriangularMesh( edit, mTopologicalMesh.removeFaces( faceToRemoveIndex ) );

  emit meshEdited();
}

void QgsMeshEditor::applyEditOnTriangularMesh( QgsMeshEditor::Edit &edit, const QgsTopologicalMesh::Changes &topologicChanges )
{
  QgsTriangularMesh::Changes triangularChanges( topologicChanges );
  mTriangularMesh->applyChanges( triangularChanges );

  edit.topologicalChanges = topologicChanges;
  edit.triangularMeshChanges = triangularChanges;
}

QgsMeshEditingError QgsMeshEditor::removeFaces( const QList<int> &facesToRemove )
{
  QgsMeshEditingError error = mTopologicalMesh.canFacesBeRemoved( facesToRemove );
  if ( error.errorType != QgsMeshEditingError::NoError )
    return error;

  mUndoStack->push( new QgsMeshLayerUndoCommandRemoveFaces( this, facesToRemove ) );

  return error;
}

QVector<QgsMeshFace> QgsMeshEditor::prepareFaces( const QVector<QgsMeshFace> &faces, QgsMeshEditingError &error )
{
  QVector<QgsMeshFace> treatedFaces = faces;

  // here we could add later some filters, for example, removing faces intersecting with existing one

  for ( QgsMeshFace &face : treatedFaces )
  {
    error = mTopologicalMesh.counterClockWiseFaces( face, mMesh );
    if ( error.errorType != QgsMeshEditingError::NoError )
      break;
  }

  return treatedFaces;
}

QgsMeshEditingError QgsMeshEditor::addFaces( const QVector<QVector<int> > &faces )
{
  QgsMeshEditingError error;
  QVector<QgsMeshFace> facesToAdd = prepareFaces( faces, error );

  if ( error.errorType != QgsMeshEditingError::NoError )
    return error;

  QgsTopologicalMesh::TopologicalFaces topologicalFaces = mTopologicalMesh.createNewTopologicalFaces( facesToAdd, error );

  error = mTopologicalMesh.canFacesBeAdded( topologicalFaces );

  if ( error.errorType != QgsMeshEditingError::NoError )
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
    emit meshEdited();
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
  if ( !fillHoles )
  {
    QSet<int> concernedNativeFaces;
    for ( const int vi : verticesToRemoveIndexes )
      concernedNativeFaces.unite( mTopologicalMesh.facesAroundVertex( vi ).toSet() );

    error = mTopologicalMesh.canFacesBeRemoved( concernedNativeFaces.values() );
    if ( error.errorType != QgsMeshEditingError::NoError )
      return error;
  }

  mUndoStack->push( new QgsMeshLayerUndoCommandRemoveVertices( this, verticesToRemoveIndexes, fillHoles ) );

  return error;
}

void QgsMeshEditor::stopEditing()
{
  mTopologicalMesh.reindex();
}



QgsMeshLayerUndoCommandMeshEdit::QgsMeshLayerUndoCommandMeshEdit( QgsMeshEditor *meshEditor ):
  mMeshEditor( meshEditor )
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

QgsMeshLayerUndoCommandAddVertices::QgsMeshLayerUndoCommandAddVertices( QgsMeshEditor *meshEditor, const QVector<QgsMeshVertex> &vertices ):
  QgsMeshLayerUndoCommandMeshEdit( meshEditor )
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

QgsMeshLayerUndoCommandRemoveVertices::QgsMeshLayerUndoCommandRemoveVertices( QgsMeshEditor *meshEditor, const QList<int> &verticesToRemoveIndexes, bool fillHole ):
  QgsMeshLayerUndoCommandMeshEdit( meshEditor )
  , mVerticesToRemoveIndexes( verticesToRemoveIndexes )
  , mFillHole( fillHole )
{}

void QgsMeshLayerUndoCommandRemoveVertices::redo()
{
  if ( !mVerticesToRemoveIndexes.isEmpty() )
  {
    for ( const int &vertex : std::as_const( mVerticesToRemoveIndexes ) )
    {
      QgsMeshEditor::Edit edit;
      mMeshEditor->applyRemoveVertex( edit, vertex, mFillHole );
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

QgsMeshLayerUndoCommandAddFaces::QgsMeshLayerUndoCommandAddFaces( QgsMeshEditor *meshEditor, QgsTopologicalMesh::TopologicalFaces &faces ):
  QgsMeshLayerUndoCommandMeshEdit( meshEditor )
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

QgsMeshLayerUndoCommandRemoveFaces::QgsMeshLayerUndoCommandRemoveFaces( QgsMeshEditor *meshEditor, const QList<int> &facesToRemoveIndexes ):
  QgsMeshLayerUndoCommandMeshEdit( meshEditor )
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
