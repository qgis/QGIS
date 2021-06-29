/***************************************************************************
  testqgsmesheditor.cpp

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

#include "qgstest.h"
#include <QObject>

#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgsmesheditor.h"


class TestQgsMeshEditor : public QObject
{
    Q_OBJECT

  public:
    TestQgsMeshEditor() = default;
    QString readFile( const QString &frame ) const;

  private:
    std::unique_ptr<QgsMeshLayer> meshLayerQuadTriangle;
    std::unique_ptr<QgsMeshLayer> meshLayerQuadFlower;
    QgsMesh nativeMesh;
    QString mDataDir;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void startStopEditing();
    void createTopologicMesh();
    void editTopologicMesh();
    void badTopologicMesh();

    void meshEditorSimpleEdition();

    void meshEditorFromMeshLayer_quadTriangle();
    void meshEditorFromMeshLayer_quadFlower();
};


void TestQgsMeshEditor::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  nativeMesh.clear();
  nativeMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) ); // 0
  nativeMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) ); // 1
  nativeMesh.vertices.append( QgsMeshVertex( 0.9, 0.9, 0.0 ) ); // 2
  nativeMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) ); // 3
  nativeMesh.vertices.append( QgsMeshVertex( 1.5, 1.2, 0.0 ) ); // 4
  nativeMesh.vertices.append( QgsMeshVertex( 2.0, -0.2, 0.0 ) ); // 5
  nativeMesh.faces.append( QgsMeshFace( {0, 1, 2, 3} ) ); //clock wise face
  nativeMesh.faces.append( QgsMeshFace( {1, 4, 2} ) ); //clock wise face
  nativeMesh.faces.append( QgsMeshFace( {3, 4, 2} ) ); //counter clock wise face
  nativeMesh.faces.append( QgsMeshFace( {3, 5, 4} ) ); //counter clock wise face
}

void TestQgsMeshEditor::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMeshEditor::init()
{
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/mesh";
  QString uri( mDataDir + "/quad_and_triangle.2dm" );
  meshLayerQuadTriangle.reset( new QgsMeshLayer( uri, "Triangle and Quad", "mdal" ) );
  QVERIFY( meshLayerQuadTriangle );
  QCOMPARE( meshLayerQuadTriangle->datasetGroupCount(), 1 );

  uri = QString( mDataDir + "/quad_flower_to_edit.2dm" );
  meshLayerQuadFlower.reset( new QgsMeshLayer( uri, "Quad Flower", "mdal" ) );
  QVERIFY( meshLayerQuadFlower );
  QCOMPARE( meshLayerQuadFlower->datasetGroupCount(), 1 );
}

void TestQgsMeshEditor::startStopEditing()
{
  meshLayerQuadTriangle->addDatasets( mDataDir + "/quad_and_triangle_vertex_scalar.dat" );
  QCOMPARE( meshLayerQuadTriangle->datasetGroupCount(), 2 );
  int datasetGroupIndex = meshLayerQuadTriangle->datasetGroupsIndexes().at( 0 );
  QgsMeshDatasetGroupMetadata meta = meshLayerQuadTriangle->datasetGroupMetadata( datasetGroupIndex );
  QCOMPARE( meta.name(), QStringLiteral( "Bed Elevation" ) );

  QgsCoordinateTransform transform;

  QVERIFY( meshLayerQuadTriangle->startFrameEditing( transform ) );
  QVERIFY( !meshLayerQuadTriangle->startFrameEditing( transform ) ); //mesh editing is already started

  // Edition has started, dataset groups are removed ans replace by a virtual one that represent the Z value of vertices
  QCOMPARE( meshLayerQuadTriangle->datasetGroupCount(), 1 );
  datasetGroupIndex = meshLayerQuadTriangle->datasetGroupsIndexes().at( 0 );
  meta = meshLayerQuadTriangle->datasetGroupMetadata( datasetGroupIndex );
  QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );
  QVERIFY( meta.name() == QStringLiteral( "vertices elevation" ) );
  QCOMPARE( meta.isTemporal(), false );
  QCOMPARE( meta.isScalar(), true );
  QCOMPARE( meta.minimum(), 10.0 );
  QCOMPARE( meta.maximum(), 50.0 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );
  QgsMesh mesh = *meshLayerQuadTriangle->nativeMesh();
  for ( int i = 0; i < mesh.vertexCount(); ++i )
    QCOMPARE( mesh.vertex( i ).z(), meshLayerQuadTriangle->datasetValue( QgsMeshDatasetIndex( 0, 0 ), i ).scalar() );

  QgsMeshEditor *editor = meshLayerQuadTriangle->meshEditor();
  QCOMPARE( editor->addVertices( {QgsMeshVertex( 1500, 2500, 0 )}, 10 ), 1 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 6 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 6 );

  // roll back editing and continue editing
  QVERIFY( meshLayerQuadTriangle->isFrameModified() );
  QVERIFY( meshLayerQuadTriangle->rollBackFrameEditing( transform, true ) );
  QVERIFY( editor->mTriangularMesh->faceCentroids().count() == editor->mMesh->faceCount() );
  QVERIFY( editor->mTriangularMesh->vertices().count() == editor->mMesh->vertexCount() );

  QVERIFY( meshLayerQuadTriangle->meshEditor() );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );
  QCOMPARE( meshLayerQuadTriangle->datasetGroupCount(), 1 );
  datasetGroupIndex = meshLayerQuadTriangle->datasetGroupsIndexes().at( 0 );
  meta = meshLayerQuadTriangle->datasetGroupMetadata( datasetGroupIndex );
  QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );
  QVERIFY( meta.name() == QStringLiteral( "vertices elevation" ) );
  QCOMPARE( meta.isTemporal(), false );
  QCOMPARE( meta.isScalar(), true );
  QCOMPARE( meta.minimum(), 10.0 );
  QCOMPARE( meta.maximum(), 50.0 );

  QCOMPARE( editor->addVertices( {QgsMeshVertex( 1500, 2500, 0 )}, 10 ), 1 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 6 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 6 );

  // roll back editing and stop editing
  QVERIFY( meshLayerQuadTriangle->isFrameModified() );
  QVERIFY( meshLayerQuadTriangle->rollBackFrameEditing( transform, false ) );

  QVERIFY( !meshLayerQuadTriangle->meshEditor() );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );
  QCOMPARE( meshLayerQuadTriangle->datasetGroupCount(), 1 );
  datasetGroupIndex = meshLayerQuadTriangle->datasetGroupsIndexes().at( 0 );
  meta = meshLayerQuadTriangle->datasetGroupMetadata( datasetGroupIndex );
  QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );
  QVERIFY( meta.name() == QStringLiteral( "Bed Elevation" ) );
  QCOMPARE( meta.isTemporal(), false );
  QCOMPARE( meta.isScalar(), true );
  QCOMPARE( meta.minimum(), 10.0 );
  QCOMPARE( meta.maximum(), 50.0 );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );
}

static bool checkNeighbors( const QgsTopologicalMesh &mesh, int faceIndex, const QList<int> &expectedNeighbors )
{
  QList<int> neighbors = mesh.neighborsOfFace( faceIndex );
  bool ret = true;
  ret &= neighbors.count() == mesh.mesh()->face( faceIndex ).count();
  for ( const int exn : expectedNeighbors )
    ret &= neighbors.contains( exn ) ;

  return ret;
}

static bool checkFacesAround( const QgsTopologicalMesh &mesh, int vertexIndex, QList<int> expectedFace )
{
  QList<int> facesAround = mesh.facesAroundVertex( vertexIndex );
  bool ret = true;
  ret &= expectedFace.count() == facesAround.count();
  for ( const int exf : expectedFace )
    ret &= facesAround.contains( exf ) ;

  return ret;
}

void TestQgsMeshEditor::createTopologicMesh()
{
  // Test of the creation of the topologic mesh from the native mesh, then test access to the elements from other elements

  QgsMeshEditingError error;
  QgsTopologicalMesh topologicMesh = QgsTopologicalMesh::createTopologicalMesh( &nativeMesh, error );
  QVERIFY( error.errorType == Qgis::MeshEditingErrorType::NoError );

  // Check if face are counter clock wise
  QVERIFY( !QgsMesh::compareFaces( nativeMesh.face( 0 ), QgsMeshFace( {0, 1, 2, 3} ) ) );
  QVERIFY( QgsMesh::compareFaces( nativeMesh.face( 0 ), QgsMeshFace( {3, 2, 1, 0} ) ) );
  QVERIFY( QgsMesh::compareFaces( nativeMesh.face( 1 ), QgsMeshFace( {2, 4, 1} ) ) );
  QVERIFY( QgsMesh::compareFaces( nativeMesh.face( 2 ), QgsMeshFace( {3, 4, 2} ) ) );
  QVERIFY( QgsMesh::compareFaces( nativeMesh.face( 3 ), QgsMeshFace( {5, 4, 3} ) ) );

  QVERIFY( checkNeighbors( topologicMesh, 0, {-1, 1, 2} ) );
  QVERIFY( checkNeighbors( topologicMesh, 1, {-1, 0, 2} ) );
  QVERIFY( checkNeighbors( topologicMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 3, {-1, 2} ) );

  QVERIFY( checkFacesAround( topologicMesh, 0, {0} ) );
  QVERIFY( checkFacesAround( topologicMesh, 1, {0, 1} ) );
  QVERIFY( checkFacesAround( topologicMesh, 2, {0, 1, 2} ) );
  QVERIFY( checkFacesAround( topologicMesh, 3, {0, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicMesh, 4, {1, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicMesh, 5, {3} ) );
}

void TestQgsMeshEditor::editTopologicMesh()
{
  QgsMeshEditingError error;
  QgsTopologicalMesh topologicMesh = QgsTopologicalMesh::createTopologicalMesh( &nativeMesh, error );
  QVERIFY( error.errorType == Qgis::MeshEditingErrorType::NoError );

  QCOMPARE( topologicMesh.mesh()->faceCount(), 4 );
  QCOMPARE( topologicMesh.mesh()->vertexCount(), 6 );

  QVector<QgsTopologicalMesh::Changes> topologicChanges;

  const QVector<QgsMeshVertex> vertices(
  {
    {2.5, 1.0, 0},    // 6
    {2.5, 0.0, 0},   // 7
    {2.0, 1.7, 0},   // 8
    { 0.9, 1.8, 0 }, // 9
    {-1, 0.5, 0},    // 10
    {0.9, -0.8, 0}   // 11
  } );

  for ( const QgsMeshVertex &vertex : vertices )
    topologicChanges.append( topologicMesh.addFreeVertex( vertex ) );

  QCOMPARE( topologicMesh.mesh()->faceCount(), 4 );
  QCOMPARE( topologicMesh.mesh()->vertexCount(), 12 );

  QgsTopologicalMesh::TopologicalFaces topologicFaces;

  QVector<QgsMeshFace> faces;
  faces = {{5, 7, 6}};
  topologicFaces = topologicMesh.createNewTopologicalFaces( faces, error );
  QVERIFY( topologicMesh.canFacesBeAdded( topologicFaces ) ==
           QgsMeshEditingError( Qgis::MeshEditingErrorType::UniqueSharedVertex, 5 ) );

  faces = {{5, 7, 6, 4}};
  topologicFaces = topologicMesh.createNewTopologicalFaces( faces, error );
  QVERIFY( topologicMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );

  faces =
  {
    {5, 7, 6, 4}, // 0
    {6, 8, 4},    // 1
    {1, 4, 9},    // 2
    {10, 1, 9},   // 3
    {0, 1, 10},   // 4
    {11, 3, 0},   // 5
    {11, 5, 3},   // 6
  };

  topologicFaces = topologicMesh.createNewTopologicalFaces( faces, error );

  QVERIFY( topologicMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );

  faces =
  {
    {5, 7, 6},    // 0 face that share only one vertex
    {6, 8, 4},    // 1
    {1, 4, 9},    // 2
    {10, 1, 9},   // 3
    {0, 1, 10},   // 4
    {11, 3, 0},   // 5
    {11, 5, 3},   // 6
  };

  topologicFaces = topologicMesh.createNewTopologicalFaces( faces, error );
  QVERIFY( topologicMesh.canFacesBeAdded( topologicFaces ).errorType ==  Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faces =
  {
    {5, 7, 6},    // 0 face that share only one vertex
    {6, 8, 4},    // 1
    {1, 4, 9},    // 2
    {10, 1, 9},   // 3
    {0, 1, 10},   // 4
    {11, 3, 0},   // 5
    {11, 5, 3},   // 6
    {5, 6, 4}     // face added to fixe the first one
  };

  topologicFaces = topologicMesh.createNewTopologicalFaces( faces, error );
  QVERIFY( topologicMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );

  faces = {{5, 7, 6, 4}};
  topologicFaces = topologicMesh.createNewTopologicalFaces( faces, error );
  QVERIFY( topologicMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );
  topologicChanges.append( topologicMesh.addFaces( topologicFaces ) ) ;

  QVERIFY( checkNeighbors( topologicMesh, 0, {-1, 2, 1} ) );
  QVERIFY( checkNeighbors( topologicMesh, 1, {-1, 0, 2} ) );
  QVERIFY( checkNeighbors( topologicMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 3, {-1, 2, 4} ) );
  QVERIFY( checkNeighbors( topologicMesh, 4, {-1, 3} ) );
  QVERIFY( checkFacesAround( topologicMesh, 0, {0} ) );
  QVERIFY( checkFacesAround( topologicMesh, 1, {0, 1} ) );
  QVERIFY( checkFacesAround( topologicMesh, 2, {0, 1, 2} ) );
  QVERIFY( checkFacesAround( topologicMesh, 3, {0, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicMesh, 4, {1, 2, 3, 4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 5, {3, 4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 6, {4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 7, {4} ) );

  faces =
  {
    {6, 8, 4},    // 1
    {1, 4, 9},    // 2
    {10, 1, 9},   // 3
    {0, 1, 10},   // 4
    {11, 3, 0},   // 5
    {11, 5, 3},   // 6
  };

  topologicFaces = topologicMesh.createNewTopologicalFaces( faces, error );
  QVERIFY( topologicMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );
  topologicChanges.append( topologicMesh.addFaces( topologicFaces ) ) ;

  QVERIFY( checkNeighbors( topologicMesh, 0, {1, 2, 9, 8} ) );
  QVERIFY( checkNeighbors( topologicMesh, 1, {0, 2, 6} ) );
  QVERIFY( checkNeighbors( topologicMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 3, {2, 4, 10} ) );
  QVERIFY( checkNeighbors( topologicMesh, 4, {-1, 3, 5} ) );
  QVERIFY( checkNeighbors( topologicMesh, 5, {-1, 4} ) );
  QVERIFY( checkNeighbors( topologicMesh, 6, {-1, 1, 7} ) );
  QVERIFY( checkNeighbors( topologicMesh, 7, {-1, 6, 8} ) );
  QVERIFY( checkNeighbors( topologicMesh, 8, {-1, 0, 7} ) );
  QVERIFY( checkNeighbors( topologicMesh, 9, {-1, 0, 10} ) );
  QVERIFY( checkNeighbors( topologicMesh, 10, {-1, 3, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 0, {0, 8, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 1, {0, 1, 6, 7, 8} ) );
  QVERIFY( checkFacesAround( topologicMesh, 2, {1, 2, 0} ) );
  QVERIFY( checkFacesAround( topologicMesh, 3, {0, 2, 3, 10, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 4, {1, 2, 3, 4, 5, 6} ) );
  QVERIFY( checkFacesAround( topologicMesh, 5, {4, 3, 10} ) );
  QVERIFY( checkFacesAround( topologicMesh, 6, {4, 5} ) );
  QVERIFY( checkFacesAround( topologicMesh, 7, {4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 8, {5} ) );
  QVERIFY( checkFacesAround( topologicMesh, 9, {6, 7} ) );
  QVERIFY( checkFacesAround( topologicMesh, 10, {7, 8} ) );
  QVERIFY( checkFacesAround( topologicMesh, 11, {9, 10} ) );

  topologicMesh.reverseChanges( topologicChanges.last() );

  QVERIFY( checkNeighbors( topologicMesh, 0, {-1, 2, 1} ) );
  QVERIFY( checkNeighbors( topologicMesh, 1, {-1, 0, 2} ) );
  QVERIFY( checkNeighbors( topologicMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 3, {-1, 2, 4} ) );
  QVERIFY( checkNeighbors( topologicMesh, 4, {-1, 3} ) );
  QVERIFY( checkFacesAround( topologicMesh, 0, {0} ) );
  QVERIFY( checkFacesAround( topologicMesh, 1, {0, 1} ) );
  QVERIFY( checkFacesAround( topologicMesh, 2, {0, 1, 2} ) );
  QVERIFY( checkFacesAround( topologicMesh, 3, {0, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicMesh, 4, {1, 2, 3, 4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 5, {3, 4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 6, {4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 7, {4} ) );

  topologicMesh.applyChanges( topologicChanges.last() );

  QVERIFY( checkNeighbors( topologicMesh, 0, {1, 2, 9, 8} ) );
  QVERIFY( checkNeighbors( topologicMesh, 1, {0, 2, 6} ) );
  QVERIFY( checkNeighbors( topologicMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 3, {2, 4, 10} ) );
  QVERIFY( checkNeighbors( topologicMesh, 4, {-1, 3, 5} ) );
  QVERIFY( checkNeighbors( topologicMesh, 5, {-1, 4} ) );
  QVERIFY( checkNeighbors( topologicMesh, 6, {-1, 1, 7} ) );
  QVERIFY( checkNeighbors( topologicMesh, 7, {-1, 6, 8} ) );
  QVERIFY( checkNeighbors( topologicMesh, 8, {-1, 0, 7} ) );
  QVERIFY( checkNeighbors( topologicMesh, 9, {-1, 0, 10} ) );
  QVERIFY( checkNeighbors( topologicMesh, 10, {-1, 3, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 0, {0, 8, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 1, {0, 1, 6, 7, 8} ) );
  QVERIFY( checkFacesAround( topologicMesh, 2, {1, 2, 0} ) );
  QVERIFY( checkFacesAround( topologicMesh, 3, {0, 2, 3, 10, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 4, {1, 2, 3, 4, 5, 6} ) );
  QVERIFY( checkFacesAround( topologicMesh, 5, {4, 3, 10} ) );
  QVERIFY( checkFacesAround( topologicMesh, 6, {4, 5} ) );
  QVERIFY( checkFacesAround( topologicMesh, 7, {4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 8, {5} ) );
  QVERIFY( checkFacesAround( topologicMesh, 9, {6, 7} ) );
  QVERIFY( checkFacesAround( topologicMesh, 10, {7, 8} ) );
  QVERIFY( checkFacesAround( topologicMesh, 11, {9, 10} ) );


  QList<int> faceToRemove;
  faceToRemove = {2, 3};
  QVERIFY( topologicMesh.canFacesBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {0, 1, 2, 3};
  QVERIFY( topologicMesh.canFacesBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {0, 9};
  QVERIFY( topologicMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {8, 0, 9, 10};
  QVERIFY( topologicMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {1, 2, 3, 4, 5};
  QVERIFY( topologicMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {0, 1, 2, 3, 4, 5};
  QVERIFY( topologicMesh.canFacesBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {9, 0, 1, 2, 3, 4, 5};
  QVERIFY( topologicMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {0, 6, 7, 8};
  QVERIFY( topologicMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  topologicChanges.append( topologicMesh.removeFaces( {0, 9} ) );

  QVERIFY( checkNeighbors( topologicMesh, 8, {-1, 7} ) );
  QVERIFY( checkNeighbors( topologicMesh, 10, {-1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 2, {-1, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 1, {-1, 2, 6} ) );
  QVERIFY( checkFacesAround( topologicMesh, 0, {8} ) );
  QVERIFY( checkFacesAround( topologicMesh, 1, {1, 6, 7, 8} ) );
  QVERIFY( checkFacesAround( topologicMesh, 2, { 1, 2} ) );
  QVERIFY( checkFacesAround( topologicMesh, 3, { 2, 3, 10} ) );
  QVERIFY( checkFacesAround( topologicMesh, 11, {10} ) );

  topologicMesh.reverseChanges( topologicChanges.last() );

  QVERIFY( checkNeighbors( topologicMesh, 0, {1, 2, 9, 8} ) );
  QVERIFY( checkNeighbors( topologicMesh, 1, {0, 2, 6} ) );
  QVERIFY( checkNeighbors( topologicMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 3, {2, 4, 10} ) );
  QVERIFY( checkNeighbors( topologicMesh, 4, {-1, 3, 5} ) );
  QVERIFY( checkNeighbors( topologicMesh, 5, {-1, 4} ) );
  QVERIFY( checkNeighbors( topologicMesh, 6, {-1, 1, 7} ) );
  QVERIFY( checkNeighbors( topologicMesh, 7, {-1, 6, 8} ) );
  QVERIFY( checkNeighbors( topologicMesh, 8, {-1, 0, 7} ) );
  QVERIFY( checkNeighbors( topologicMesh, 9, {-1, 0, 10} ) );
  QVERIFY( checkNeighbors( topologicMesh, 10, {-1, 3, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 0, {0, 8, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 1, {0, 1, 6, 7, 8} ) );
  QVERIFY( checkFacesAround( topologicMesh, 2, {1, 2, 0} ) );
  QVERIFY( checkFacesAround( topologicMesh, 3, {0, 2, 3, 10, 9} ) );
  QVERIFY( checkFacesAround( topologicMesh, 4, {1, 2, 3, 4, 5, 6} ) );
  QVERIFY( checkFacesAround( topologicMesh, 5, {4, 3, 10} ) );
  QVERIFY( checkFacesAround( topologicMesh, 6, {4, 5} ) );
  QVERIFY( checkFacesAround( topologicMesh, 7, {4} ) );
  QVERIFY( checkFacesAround( topologicMesh, 8, {5} ) );
  QVERIFY( checkFacesAround( topologicMesh, 9, {6, 7} ) );
  QVERIFY( checkFacesAround( topologicMesh, 10, {7, 8} ) );
  QVERIFY( checkFacesAround( topologicMesh, 11, {9, 10} ) );

  topologicMesh.applyChanges( topologicChanges.last() );

  topologicChanges.append( topologicMesh.addVertexInface( 4, {2.2, 0.5, 0} ) );

  QVERIFY( checkFacesAround( topologicMesh, 12, {11, 12, 13, 14} ) );

  topologicFaces = topologicMesh.createNewTopologicalFaces( {{4, 8, 9}, {0, 3, 2, 1}}, error );
  QVERIFY( error == QgsMeshEditingError() );
  topologicChanges.append( topologicMesh.addFaces( topologicFaces ) );
  QVERIFY( checkFacesAround( topologicMesh, 9, {7, 6, 15} ) );

  topologicChanges.append( topologicMesh.removeVertex( 4, true ) );

  // reverse all!!!
  for ( int i = 0; i < topologicChanges.count(); ++i )
    topologicMesh.reverseChanges( topologicChanges.at( topologicChanges.count() - i - 1 ) );

  QVERIFY( checkNeighbors( topologicMesh, 0, {-1, 1, 2} ) );
  QVERIFY( checkNeighbors( topologicMesh, 1, {-1, 0, 2} ) );
  QVERIFY( checkNeighbors( topologicMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicMesh, 3, {-1, 2} ) );
  QVERIFY( checkFacesAround( topologicMesh, 0, {0} ) );
  QVERIFY( checkFacesAround( topologicMesh, 1, {0, 1} ) );
  QVERIFY( checkFacesAround( topologicMesh, 2, {0, 2, 1} ) );
  QVERIFY( checkFacesAround( topologicMesh, 3, {0, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicMesh, 4, {1, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicMesh, 5, {3} ) );

  QCOMPARE( topologicMesh.mesh()->faceCount(), 4 );
  QCOMPARE( topologicMesh.mesh()->vertexCount(), 6 );

  // reapply all!!!
  for ( int i = 0; i < topologicChanges.count(); ++i )
    topologicMesh.applyChanges( topologicChanges.at( i ) );

  topologicMesh.reindex();

  QCOMPARE( topologicMesh.mesh()->faceCount(), 12 );
  QCOMPARE( topologicMesh.mesh()->vertexCount(), 12 );
}

void TestQgsMeshEditor::badTopologicMesh()
{
  // here we test if the creation of the topologic mesh fails if there are some forbitten faces :
  // concave face,
  // unorder vertex index in the face
  // completely or partially flat face
  // faces sharing only one vertex

  QgsMesh badMesh;
  //Invalid vertex (empty one)
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex() );
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.2, 0.2, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 1, 2, 3} ) );
  QgsMeshEditingError error;
  QgsTopologicalMesh topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, 0 ) );

  //Invalid vertex (index out of range)
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.2, 0.2, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 1, 2, 5} ) );
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, 5 ) );


  // concave face
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.2, 0.2, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 1, 2, 3} ) ); //concave face
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, 0 ) );

  // bad ordering of vertex index in face
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.9, 0.9, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 2, 1, 3} ) ); //bad ordering of faces
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, 0 ) );

  // Flat face
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( -1.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.9, 0.9, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 2, 1, 3} ) ); //bad ordering of faces
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::FlatFace, 0 ) );

  // Sharing only one vertex
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) ); // 0
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) ); // 1
  badMesh.vertices.append( QgsMeshVertex( 0.9, 0.9, 0.0 ) ); // 2
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) ); // 3
  badMesh.vertices.append( QgsMeshVertex( 1.5, 1.2, 0.0 ) ); // 4
  badMesh.vertices.append( QgsMeshVertex( 2.0, -0.2, 0.0 ) ); // 5
  badMesh.vertices.append( QgsMeshVertex( 2.5, 1.3, 0.0 ) ); // 6
  badMesh.faces.append( QgsMeshFace( {0, 1, 2, 3} ) ); //clock wise face
  badMesh.faces.append( QgsMeshFace( {1, 4, 2} ) ); //clock wise face
  badMesh.faces.append( QgsMeshFace( {3, 4, 2} ) ); //counter clock wise face
  badMesh.faces.append( QgsMeshFace( {4, 5, 6} ) ); // isolated face linked by ony one vertices
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::UniqueSharedVertex, 4 ) );
}

void TestQgsMeshEditor::meshEditorSimpleEdition()
{
  QUndoStack undoStack;

  QgsCoordinateTransform transform;
  QVERIFY( meshLayerQuadTriangle->startFrameEditing( transform ) );

  QgsMesh mesh;
  QgsTriangularMesh triangularMesh;
  triangularMesh.update( &mesh, QgsCoordinateTransform() );

  QgsMeshEditor meshEditor( &mesh, &triangularMesh );
  QCOMPARE( meshEditor.initialize(), QgsMeshEditingError() );

  QVector<QgsMeshVertex> vertices( {QgsPoint( 0.0, 0.0, 0.0 ), // 0
                                    QgsPoint( 0.0, 1.0, 0.0 ), // 1
                                    QgsPoint( 0.9, 0.9, 0.0 ), // 2
                                    QgsPoint( 1.0, 0.0, 0.0 ), // 3
                                    QgsPoint( )} );            // 4

  meshEditor.addVertices( vertices, 0.01 );

  for ( int i = 0; i < 5; ++i )
    QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( i ), QList<int>() );

  // add bad face
  QVERIFY( meshEditor.addFaces( {{0, 1, 3, 2}} ).errorType == Qgis::MeshEditingErrorType::InvalidFace ); //unordered vertex index
  QVERIFY( meshEditor.addFaces( {{0, 1, 2, 100}} ).errorType == Qgis::MeshEditingErrorType::InvalidVertex ); //out of range vertex index
  QVERIFY( meshEditor.addFaces( {{0, 1, 2, 4}} ).errorType == Qgis::MeshEditingErrorType::InvalidVertex ); // empty vertex

// add good face
  QVERIFY( meshEditor.addFaces( {{0, 1, 2, 3}} ).errorType == Qgis::MeshEditingErrorType::NoError );

  QCOMPARE( triangularMesh.vertices().count(), 4 );
  QCOMPARE( triangularMesh.faceCentroids().count(), 1 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.4, 0.5 ) ), 0 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.6, 0.5 ) ), 1 );
  QCOMPARE( triangularMesh.faceIndexesForRectangle( QgsRectangle( 0.1, 0.1, 0.7, 0.7 ) ).count(), 2 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 1, 0.5 ) ), -1 );

  QCOMPARE( meshEditor.mTopologicalMesh.neighborsOfFace( 0 ), QList<int>( {-1, -1, -1, -1} ) );
  for ( int i = 0; i < 4; ++i )
    QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( i ), QList<int>( {0} ) );
  QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( 4 ), QList<int>() );

  // undo edition
  meshEditor.mUndoStack->undo();

  QCOMPARE( triangularMesh.faceCentroids().count(), 0 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.4, 0.5 ) ), -1 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.6, 0.5 ) ), -1 );
  QCOMPARE( triangularMesh.faceIndexesForRectangle( QgsRectangle( 0.1, 0.1, 0.7, 0.7 ) ).count(), 0 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 1, 0.5 ) ), -1 );

  // redo edition
  meshEditor.mUndoStack->redo();

  QCOMPARE( triangularMesh.faceCentroids().count(), 1 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.4, 0.5 ) ), 0 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.6, 0.5 ) ), 1 );
  QCOMPARE( triangularMesh.faceIndexesForRectangle( QgsRectangle( 0.1, 0.1, 0.7, 0.7 ) ).count(), 2 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 1, 0.5 ) ), -1 );

  QCOMPARE( meshEditor.mTopologicalMesh.neighborsOfFace( 0 ), QList<int>( {-1, -1, -1, -1} ) );
  for ( int i = 0; i < 4; ++i )
    QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( i ), QList<int>( {0} ) );
  QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( 4 ), QList<int>() );

}

void TestQgsMeshEditor::meshEditorFromMeshLayer_quadTriangle()
{
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );

  QgsCoordinateTransform transform;
  QVERIFY( meshLayerQuadTriangle->startFrameEditing( transform ) );

  QgsMeshEditor *editor = meshLayerQuadTriangle->meshEditor();

  editor->addVertices( {{4000, 2000, 0}}, 10 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 6 );

  meshLayerQuadTriangle->undoStack()->undo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );

  editor->addVertices(
  {
    {4000, 2000, 0}, // 5
    {4000, 3000, 0}  // 6
  }, 10 );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );

  meshLayerQuadTriangle->undoStack()->undo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );

  meshLayerQuadTriangle->undoStack()->redo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );

  // try to add a face that shares only one vertex
  QgsMeshEditingError error = editor->addFaces( {{2, 5, 6}} );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::UniqueSharedVertex, 2 ) );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );

  // Add a face that shares two vertices
  error = editor->addFaces( {{2, 3, 6}} );
  QVERIFY( error == QgsMeshEditingError() );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 3 );

  QgsPointXY centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3000, 2666.666666666 ), 1e-6 ) );

  meshLayerQuadTriangle->undoStack()->undo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  meshLayerQuadTriangle->undoStack()->undo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );

  meshLayerQuadTriangle->undoStack()->redo();
  meshLayerQuadTriangle->undoStack()->redo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 3 );

  // Add another face
  error = editor->addFaces( {{2, 5, 6}} );
  QVERIFY( error == QgsMeshEditingError() );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 4 );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3000, 2666.666666666 ), 1e-6 ) );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3500, 2100, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3666.6666666, 2333.33333333 ), 1e-6 ) );

  meshLayerQuadTriangle->undoStack()->undo();
  meshLayerQuadTriangle->undoStack()->undo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3500, 2100, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  meshLayerQuadTriangle->undoStack()->redo();
  meshLayerQuadTriangle->undoStack()->redo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3000, 2666.666666666 ), 1e-6 ) );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3500, 2100, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3666.6666666, 2333.33333333 ), 1e-6 ) );

  editor->removeFaces( {2, 3} );

  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 4 ); //removed faces are still present but empty
  QVERIFY( meshLayerQuadTriangle->nativeMesh()->face( 2 ).isEmpty() );
  QVERIFY( meshLayerQuadTriangle->nativeMesh()->face( 3 ).isEmpty() );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3500, 2100, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  meshLayerQuadTriangle->undoStack()->undo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3000, 2666.666666666 ), 1e-6 ) );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3500, 2100, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3666.6666666, 2333.33333333 ), 1e-6 ) );

  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 4 ); //removed faces are still present but empty
  QVERIFY( QgsMesh::compareFaces( meshLayerQuadTriangle->nativeMesh()->face( 2 ), {2, 6, 3} ) );
  QVERIFY( QgsMesh::compareFaces( meshLayerQuadTriangle->nativeMesh()->face( 3 ), {2, 5, 6} ) );

  //Add a vertex on a face and one external
  editor->addVertices(
  {
    {1500, 2800, 0},
    {3000, 3500, 0}
  }, 10 );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 8 ); // vertex on a quad face : 4 faces created, 1 removed, removed are still present but void
  QVERIFY( meshLayerQuadTriangle->nativeMesh()->face( 0 ).isEmpty() );
  QVERIFY( QgsMesh::compareFaces( meshLayerQuadTriangle->nativeMesh()->face( 4 ), {0, 1, 7} ) );
  QVERIFY( QgsMesh::compareFaces( meshLayerQuadTriangle->nativeMesh()->face( 5 ), {1, 3, 7} ) );
  QVERIFY( QgsMesh::compareFaces( meshLayerQuadTriangle->nativeMesh()->face( 6 ), {3, 4, 7} ) );
  QVERIFY( QgsMesh::compareFaces( meshLayerQuadTriangle->nativeMesh()->face( 7 ), {4, 0, 7} ) );

  QgsPointXY snappedPoint = meshLayerQuadTriangle->snapOnElement( QgsMesh::Vertex, QgsPoint( 1498, 2805, 0 ), 10 );
  QVERIFY( snappedPoint.compare( QgsPointXY( 1500, 2800 ), 1e-6 ) );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2050, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2266.66666666 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1150, 2340, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1166.6666666, 2600 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2950, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2933.33333333 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2700, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1833.33333333, 2600 ), 1e-6 ) );

  meshLayerQuadTriangle->undoStack()->undo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3000, 2666.666666666 ), 1e-6 ) );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3500, 2100, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3666.6666666, 2333.33333333 ), 1e-6 ) );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 4 );
  QVERIFY( QgsMesh::compareFaces( meshLayerQuadTriangle->nativeMesh()->face( 2 ), {2, 6, 3} ) );
  QVERIFY( QgsMesh::compareFaces( meshLayerQuadTriangle->nativeMesh()->face( 3 ), {2, 5, 6} ) );

  meshLayerQuadTriangle->undoStack()->redo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2050, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2266.66666666 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1150, 2340, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1166.6666666, 2600 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2950, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2933.33333333 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2700, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1833.33333333, 2600 ), 1e-6 ) );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 8 );

  editor->removeVertices( {7} );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2050, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1150, 2340, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2950, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2700, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 9 ); //empty vertex still presents

  snappedPoint = meshLayerQuadTriangle->snapOnElement( QgsMesh::Vertex, QgsPoint( 1498, 3505, 0 ), 10 );
  QVERIFY( snappedPoint.isEmpty() );

  meshLayerQuadTriangle->undoStack()->undo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2050, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2266.66666666 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1150, 2340, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1166.6666666, 2600 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2950, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2933.33333333 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2700, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1833.33333333, 2600 ), 1e-6 ) );

  snappedPoint = meshLayerQuadTriangle->snapOnElement( QgsMesh::Vertex, QgsPoint( 1498, 2805, 0 ), 10 );
  QVERIFY( snappedPoint.compare( QgsPointXY( 1500, 2800 ), 1e-6 ) );

  meshLayerQuadTriangle->stopFrameEditing( transform );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 7 );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2050, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2266.66666666 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1150, 2340, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1166.6666666, 2600 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2950, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2933.33333333 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2700, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1833.33333333, 2600 ), 1e-6 ) );

  snappedPoint = meshLayerQuadTriangle->snapOnElement( QgsMesh::Vertex, QgsPoint( 1498, 2805, 0 ), 10 );
  QVERIFY( snappedPoint.compare( QgsPointXY( 1500, 2800 ), 1e-6 ) );
}

void TestQgsMeshEditor::meshEditorFromMeshLayer_quadFlower()
{
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 8 );
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );

  QgsCoordinateTransform transform;
  QVERIFY( meshLayerQuadFlower->startFrameEditing( transform ) );

  QgsMeshEditor *editor = meshLayerQuadFlower->meshEditor();
  QVERIFY( editor );

  meshLayerQuadFlower->startFrameEditing( transform );
  editor = meshLayerQuadFlower->meshEditor();
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1500, 2800, -10 )}, 10 ), 1 ); // 8
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1800, 2700, -10 )}, 10 ), 1 ); // 9
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 12 );
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1400, 2300, -10 ), QgsPoint( 1500, 2200, -10 )}, 10 ), 2 ); // 10 & 11
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 18 );

  // attempt to add a vertex under tolerance next existing one
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1499, 2801, -10 )}, 10 ), 0 );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 18 );

  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 700, 1750, 0 )}, 10 ), 1 ); // 12

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 18 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 13 );

  QVERIFY( editor->addFace( {0, 6, 12} ) == QgsMeshEditingError() );


  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1400, 2200, -10 )}, 10 ), 1 ); // 13

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 22 );
  QCOMPARE( meshLayerQuadFlower->datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( 1420, 2220 ), 10 ).x(), -10 );

  QVERIFY( editor->removeVertices( {0}, true ) == QgsMeshEditingError() ); //for now filling after removing boundary vertices is not supported, --> behavior as it is false

  meshLayerQuadFlower->undoStack()->undo();

  QVERIFY( editor->removeVertices( {0} ) == QgsMeshEditingError() );

  QgsPointXY centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1200, 2500, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  QVERIFY( editor->addFace( {12, 10, 8, 4, 5} ) == QgsMeshEditingError() );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1200, 2500, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 984.879, 2436.712 ), 1e-2 ) );

  QVERIFY( editor->addFace( {6, 1, 11, 10, 12} ).errorType == Qgis::MeshEditingErrorType::InvalidFace ); //concave face

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1600, 1750, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  QVERIFY( editor->addFace( {6, 1, 11, 13} ) == QgsMeshEditingError() );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1600, 1750, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1633.3333, 1911.11111 ), 1e-2 ) );

  QVERIFY( editor->addFace( {13, 10, 12} ) == QgsMeshEditingError() );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1330, 2200, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1166.66666, 2083.33333 ), 1e-2 ) );

  QVERIFY( editor->addFace( {6, 13, 12} ) == QgsMeshEditingError() );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1300, 1800, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1200.0, 1816.6666 ), 1e-2 ) );

  QCOMPARE( editor->addVertices( {QgsMeshVertex( 750, 2500, 550 )}, 10 ), 1 );
  QCOMPARE( editor->addVertices( {QgsMeshVertex( 1200, 2500, 700 )}, 10 ), 1 );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1330, 2500, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1366.6666, 2533.3333 ), 1e-2 ) );

  QVERIFY( editor->removeVertices( {10}, true ) == QgsMeshEditingError() );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1330, 2500, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1400, 2500 ), 1e-2 ) );

  QVERIFY( editor->removeVertices( {13}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {11}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {9}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {8}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {15}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {14}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {7}, true ) == QgsMeshEditingError() );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 57 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 16 );

  meshLayerQuadFlower->commitFrameEditing( transform, true );

  QVERIFY( meshLayerQuadFlower->meshEditor() == editor );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 7 );

  QVERIFY( editor->removeVertices( {3}, false ).errorType != Qgis::MeshEditingErrorType::NoError ); // leads to a topological error

  QVERIFY( editor->removeVertices( {3}, true ).errorType != Qgis::MeshEditingErrorType::NoError ); // filling after removing boundary not supported, so not fill and leads to a topological error

  QVERIFY( editor->removeVertices( {4}, true ) == QgsMeshEditingError() );

  meshLayerQuadFlower->commitFrameEditing( transform, false );

  QVERIFY( meshLayerQuadFlower->meshEditor() == nullptr );
  ;
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 4 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 6 );

  QFile alteredFile( QString( mDataDir + "/quad_flower_to_edit.2dm" ) );
  QFile expectedFile( QString( mDataDir + "/quad_flower_to_edit_expected.2dm" ) );
  QFile original( QString( mDataDir + "/quad_flower.2dm" ) );

  alteredFile.open( QIODevice::ReadOnly );
  original.open( QIODevice::ReadOnly );
  expectedFile.open( QIODevice::ReadOnly );

  QTextStream streamAltered( &alteredFile );
  QTextStream streamOriginal( &original );
  QTextStream streamExpected( &expectedFile );

  QCOMPARE( streamAltered.readAll(), streamExpected.readAll() );

  alteredFile.close();
  alteredFile.open( QIODevice::WriteOnly );
  streamAltered << streamOriginal.readAll();
}

QGSTEST_MAIN( TestQgsMeshEditor )
#include "testqgsmesheditor.moc"
