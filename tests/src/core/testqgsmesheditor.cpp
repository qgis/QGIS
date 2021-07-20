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

#include "qgis.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgsmesheditor.h"
#include "qgsmeshadvancedediting.h"


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
    void faceIntersection();

    void meshEditorFromMeshLayer_quadTriangle();
    void meshEditorFromMeshLayer_quadFlower();

    void refineMesh();

    void particularCases();
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
  QVERIFY( meta.name() == QStringLiteral( "vertices Z value" ) );
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
  QVERIFY( meshLayerQuadTriangle->isModified() );
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
  QVERIFY( meta.name() == QStringLiteral( "vertices Z value" ) );
  QCOMPARE( meta.isTemporal(), false );
  QCOMPARE( meta.isScalar(), true );
  QCOMPARE( meta.minimum(), 10.0 );
  QCOMPARE( meta.maximum(), 50.0 );

  QCOMPARE( editor->addVertices( {QgsMeshVertex( 1500, 2500, 0 )}, 10 ), 1 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 6 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 6 );

  // roll back editing and stop editing
  QVERIFY( meshLayerQuadTriangle->isModified() );
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
  QVector<int> neighbors = mesh.neighborsOfFace( faceIndex );
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
  QgsTopologicalMesh topologicMesh = QgsTopologicalMesh::createTopologicalMesh( &nativeMesh, 4, error );
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

  QVERIFY( topologicMesh.checkConsistency() == QgsMeshEditingError() );
}

void TestQgsMeshEditor::editTopologicMesh()
{
  QgsMeshEditingError error;
  QgsTopologicalMesh topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &nativeMesh, 4, error );
  QVERIFY( error.errorType == Qgis::MeshEditingErrorType::NoError );

  QCOMPARE( topologicalMesh.mesh()->faceCount(), 4 );
  QCOMPARE( topologicalMesh.mesh()->vertexCount(), 6 );

  QVERIFY( !topologicalMesh.edgeCanBeFlipped( 2, 3 ) );
  QVERIFY( topologicalMesh.edgeCanBeFlipped( 3, 4 ) );
  QVERIFY( !topologicalMesh.edgeCanBeFlipped( 2, 4 ) );
  QVERIFY( !topologicalMesh.edgeCanBeFlipped( 1, 2 ) );
  QVERIFY( !topologicalMesh.edgeCanBeFlipped( 1, 4 ) );

  QVERIFY( topologicalMesh.faceCanBeSplit( 0 ) );
  QVERIFY( !topologicalMesh.faceCanBeSplit( 1 ) );
  QVERIFY( !topologicalMesh.faceCanBeSplit( 2 ) );
  QVERIFY( !topologicalMesh.faceCanBeSplit( 3 ) );

  QVector<QgsTopologicalMesh::Changes> topologicalChanges;

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
    topologicalChanges.append( topologicalMesh.addFreeVertex( vertex ) );

  QCOMPARE( topologicalMesh.mesh()->faceCount(), 4 );
  QCOMPARE( topologicalMesh.mesh()->vertexCount(), 12 );
  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 6 );

  QgsTopologicalMesh::TopologicalFaces topologicFaces;

  QVector<QgsMeshFace> faces;
  faces = {{5, 7, 6}};
  topologicFaces = topologicalMesh.createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.canFacesBeAdded( topologicFaces ) ==
           QgsMeshEditingError( Qgis::MeshEditingErrorType::UniqueSharedVertex, 5 ) );

  faces = {{5, 7, 6, 4}};
  topologicFaces = topologicalMesh.createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );

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

  topologicFaces = topologicalMesh.createNewTopologicalFaces( faces, true, error );

  QVERIFY( topologicalMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );

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

  topologicFaces = topologicalMesh.createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.canFacesBeAdded( topologicFaces ).errorType ==  Qgis::MeshEditingErrorType::UniqueSharedVertex );
  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 6 );

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

  topologicFaces = topologicalMesh.createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );

  faces =
  {
    {3, 5, 7, 6}, // 0 share vertices with same clockwise
    {6, 8, 4},    // 1
    {1, 4, 9},    // 2
    {10, 1, 9},   // 3
    {0, 1, 10},   // 4
    {11, 3, 0},   // 5
  };

  topologicFaces = topologicalMesh.createNewTopologicalFaces( faces, true, error );
  QVERIFY( error == QgsMeshEditingError() );
  error = topologicalMesh.canFacesBeAdded( topologicFaces );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, 0 ) );

  faces = {{5, 7, 6, 4}};
  topologicFaces = topologicalMesh.createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );
  topologicalChanges.append( topologicalMesh.addFaces( topologicFaces ) ) ;

  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 4 );

  QVERIFY( checkNeighbors( topologicalMesh, 0, {-1, 2, 1} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 1, {-1, 0, 2} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 3, {-1, 2, 4} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 4, {-1, 3} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 0, {0} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 1, {0, 1} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {0, 1, 2} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {0, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 4, {1, 2, 3, 4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 5, {3, 4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 6, {4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 7, {4} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  QVERIFY( !topologicalMesh.canBeMerged( 4, 5 ) );
  QVERIFY( topologicalMesh.canBeMerged( 3, 4 ) );
  QVERIFY( !topologicalMesh.canBeMerged( 1, 4 ) );
  QVERIFY( !topologicalMesh.canBeMerged( 0, 1 ) );
  QVERIFY( !topologicalMesh.canBeMerged( 0, 3 ) );
  QVERIFY( !topologicalMesh.canBeMerged( 2, 3 ) );
  QVERIFY( !topologicalMesh.canBeMerged( 4, 2 ) );
  QVERIFY( !topologicalMesh.canBeMerged( 2, 1 ) );
  QVERIFY( !topologicalMesh.canBeMerged( 3, 5 ) );
  QVERIFY( !topologicalMesh.canBeMerged( 6, 7 ) );

  faces =
  {
    {6, 8, 4},    // 1
    {1, 4, 9},    // 2
    {10, 1, 9},   // 3
    {0, 1, 10},   // 4
    {11, 3, 0},   // 5
    {11, 5, 3},   // 6
  };

  topologicFaces = topologicalMesh.createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.canFacesBeAdded( topologicFaces ) == QgsMeshEditingError() );
  topologicalChanges.append( topologicalMesh.addFaces( topologicFaces ) ) ;

  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 0 );

  QVERIFY( checkNeighbors( topologicalMesh, 0, {1, 2, 9, 8} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 1, {0, 2, 6} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 3, {2, 4, 10} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 4, {-1, 3, 5} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 5, {-1, 4} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 6, {-1, 1, 7} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 7, {-1, 6, 8} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 8, {-1, 0, 7} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 9, {-1, 0, 10} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 10, {-1, 3, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 0, {0, 8, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 1, {0, 1, 6, 7, 8} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {1, 2, 0} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {0, 2, 3, 10, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 4, {1, 2, 3, 4, 5, 6} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 5, {4, 3, 10} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 6, {4, 5} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 7, {4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 8, {5} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 9, {6, 7} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 10, {7, 8} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 11, {9, 10} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  topologicalMesh.reverseChanges( topologicalChanges.last() );

  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 4 );
  QVERIFY( checkNeighbors( topologicalMesh, 0, {-1, 2, 1} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 1, {-1, 0, 2} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 3, {-1, 2, 4} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 4, {-1, 3} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 0, {0} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 1, {0, 1} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {0, 1, 2} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {0, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 4, {1, 2, 3, 4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 5, {3, 4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 6, {4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 7, {4} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  topologicalMesh.applyChanges( topologicalChanges.last() );

  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 0 );
  QVERIFY( checkNeighbors( topologicalMesh, 0, {1, 2, 9, 8} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 1, {0, 2, 6} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 3, {2, 4, 10} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 4, {-1, 3, 5} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 5, {-1, 4} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 6, {-1, 1, 7} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 7, {-1, 6, 8} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 8, {-1, 0, 7} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 9, {-1, 0, 10} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 10, {-1, 3, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 0, {0, 8, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 1, {0, 1, 6, 7, 8} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {1, 2, 0} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {0, 2, 3, 10, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 4, {1, 2, 3, 4, 5, 6} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 5, {4, 3, 10} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 6, {4, 5} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 7, {4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 8, {5} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 9, {6, 7} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 10, {7, 8} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 11, {9, 10} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  QList<int> faceToRemove;
  faceToRemove = {2, 3};
  QVERIFY( topologicalMesh.canFacesBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {0, 1, 2, 3};
  QVERIFY( topologicalMesh.canFacesBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {0, 9};
  QVERIFY( topologicalMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {8, 0, 9, 10};
  QVERIFY( topologicalMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {1, 2, 3, 4, 5};
  QVERIFY( topologicalMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {0, 1, 2, 3, 4, 5};
  QVERIFY( topologicalMesh.canFacesBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {9, 0, 1, 2, 3, 4, 5};
  QVERIFY( topologicalMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {0, 6, 7, 8};
  QVERIFY( topologicalMesh.canFacesBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  topologicalChanges.append( topologicalMesh.removeFaces( {0, 9} ) );

  QVERIFY( checkNeighbors( topologicalMesh, 8, {-1, 7} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 10, {-1, 3} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 2, {-1, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 1, {-1, 2, 6} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 0, {8} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 1, {1, 6, 7, 8} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, { 1, 2} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, { 2, 3, 10} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 11, {10} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  topologicalMesh.reverseChanges( topologicalChanges.last() );

  QVERIFY( checkNeighbors( topologicalMesh, 0, {1, 2, 9, 8} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 1, {0, 2, 6} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 3, {2, 4, 10} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 4, {-1, 3, 5} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 5, {-1, 4} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 6, {-1, 1, 7} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 7, {-1, 6, 8} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 8, {-1, 0, 7} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 9, {-1, 0, 10} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 10, {-1, 3, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 0, {0, 8, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 1, {0, 1, 6, 7, 8} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {1, 2, 0} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {0, 2, 3, 10, 9} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 4, {1, 2, 3, 4, 5, 6} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 5, {4, 3, 10} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 6, {4, 5} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 7, {4} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 8, {5} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 9, {6, 7} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 10, {7, 8} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 11, {9, 10} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  topologicalMesh.applyChanges( topologicalChanges.last() );

  topologicalChanges.append( topologicalMesh.addVertexInface( 4, {2.2, 0.5, 0} ) ); // vertex 12

  QVERIFY( checkFacesAround( topologicalMesh, 12, {11, 12, 13, 14} ) );

  topologicFaces = topologicalMesh.createNewTopologicalFaces( {{4, 8, 9}, {0, 3, 2, 1}}, true, error );
  QVERIFY( error == QgsMeshEditingError() );
  topologicalChanges.append( topologicalMesh.addFaces( topologicFaces ) );
  QVERIFY( checkFacesAround( topologicalMesh, 9, {7, 6, 15} ) );

  topologicalChanges.append( topologicalMesh.removeVertexFillHole( 4 ) );

  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 0 );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  QVERIFY( topologicalMesh.edgeCanBeFlipped( 2, 12 ) );
  topologicalChanges.append( topologicalMesh.flipEdge( 2, 12 ) );
  QVERIFY( checkFacesAround( topologicalMesh, 12, {11, 12, 20, 22, 24} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  QVERIFY( topologicalMesh.canBeMerged( 3, 8 ) );
  topologicalChanges.append( topologicalMesh.merge( 3, 8 ) );
  QVERIFY( checkFacesAround( topologicalMesh, 12, {11, 12, 20, 22, 25} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 8, {18, 20, 25} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {16, 17, 18, 25} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {16, 25, 22, 10} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  QVERIFY( topologicalMesh.faceCanBeSplit( 25 ) );
  topologicalChanges.append( topologicalMesh.splitFace( 25 ) );
  QVERIFY( checkFacesAround( topologicalMesh, 12, {11, 12, 20, 22, 26, 27} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 8, {18, 20, 27} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {16, 17, 18, 27, 26} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {16, 26, 22, 10} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  // reverse all!!!
  for ( int i = 0; i < topologicalChanges.count(); ++i )
    topologicalMesh.reverseChanges( topologicalChanges.at( topologicalChanges.count() - i - 1 ) );

  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 0 );
  QVERIFY( checkNeighbors( topologicalMesh, 0, {-1, 1, 2} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 1, {-1, 0, 2} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 2, {0, 1, 3} ) );
  QVERIFY( checkNeighbors( topologicalMesh, 3, {-1, 2} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 0, {0} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 1, {0, 1} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {0, 2, 1} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {0, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 4, {1, 2, 3} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 5, {3} ) );

  QCOMPARE( topologicalMesh.mesh()->faceCount(), 4 );
  QCOMPARE( topologicalMesh.mesh()->vertexCount(), 6 );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  // reapply all!!!
  for ( int i = 0; i < topologicalChanges.count(); ++i )
    topologicalMesh.applyChanges( topologicalChanges.at( i ) );

  QCOMPARE( topologicalMesh.freeVerticesIndexes().count(), 0 );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  topologicalMesh.reindex();

  QCOMPARE( topologicalMesh.mesh()->faceCount(), 12 );
  QCOMPARE( topologicalMesh.mesh()->vertexCount(), 12 );
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
  QgsTopologicalMesh topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, 4, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, 0 ) );

  //Invalid vertex (index out of range)
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.2, 0.2, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 1, 2, 5} ) );
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, 4, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, 5 ) );


  // concave face
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.2, 0.2, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 1, 2, 3} ) ); //concave face
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, 4, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, 0 ) );

  // bad ordering of vertex index in face
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.9, 0.9, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 2, 1, 3} ) ); //bad ordering of faces
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, 4, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, 0 ) );

  // Flat face
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( -1.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.9, 0.9, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 2, 1, 3} ) );
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, 4, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::FlatFace, 0 ) );

  // Too much vertices
  badMesh.clear();
  badMesh.vertices.append( QgsMeshVertex( 0.0, 0.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.0, 1.0, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 0.9, 0.9, 0.0 ) );
  badMesh.vertices.append( QgsMeshVertex( 1.0, 0.0, 0.0 ) );
  badMesh.faces.append( QgsMeshFace( {0, 1, 2, 3} ) );
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, 3, error );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidFace, 0 ) );

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
  topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &badMesh, 4, error );
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
  QVERIFY( meshEditor.checkConsistency() );

  QCOMPARE( meshEditor.mTopologicalMesh.neighborsOfFace( 0 ), QVector<int>( {-1, -1, -1, -1} ) );
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
  QVERIFY( meshEditor.checkConsistency() );

  // redo edition
  meshEditor.mUndoStack->redo();

  QCOMPARE( triangularMesh.faceCentroids().count(), 1 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.4, 0.5 ) ), 0 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.6, 0.5 ) ), 1 );
  QCOMPARE( triangularMesh.faceIndexesForRectangle( QgsRectangle( 0.1, 0.1, 0.7, 0.7 ) ).count(), 2 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 1, 0.5 ) ), -1 );
  QVERIFY( meshEditor.checkConsistency() );

  QCOMPARE( meshEditor.mTopologicalMesh.neighborsOfFace( 0 ), QVector<int>( {-1, -1, -1, -1} ) );
  for ( int i = 0; i < 4; ++i )
    QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( i ), QList<int>( {0} ) );
  QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( 4 ), QList<int>() );

}

void TestQgsMeshEditor::faceIntersection()
{
  QgsCoordinateTransform transform;
  QVERIFY( meshLayerQuadFlower->startFrameEditing( transform ) );

  QgsMeshEditor *editor = meshLayerQuadFlower->meshEditor();
  QVERIFY( editor );

  // add some free vertices
  QVector<QgsMeshVertex> vertices( {QgsPoint( 2500.0, 3500.0, 0.0 ), // 8
                                    QgsPoint( 1500.0, 4000.0, 0.0 ), // 9
                                    QgsPoint( 2750.0, 3000.0, 0.0 ), // 10
                                    QgsPoint( 1750.0, 3750.0, 0.0 ), // 11
                                    QgsPoint( 500.0, 1500.0, 0.0 ), // 12
                                    QgsPoint( 0.0, 0.0, 0.0 ), // 13
                                    QgsPoint( 0.0, 5000.0, 0.0 ), // 14
                                    QgsPoint( 5000.0, 5000.0, 0.0 ), // 15
                                    QgsPoint( 5000.0, 0.0, 0.0 ), // 16
                                   } );
  editor->addVertices( vertices, 10 );

  QCOMPARE( editor->freeVerticesIndexes().count(), 9 );

  QVERIFY( editor->faceCanBeAdded( {3, 8, 7} ) );
  editor->addFace( {3, 8, 7} );

  QCOMPARE( editor->freeVerticesIndexes().count(), 8 );
  QCOMPARE( editor->mMesh->faceCount(), 6 );

  QVERIFY( !editor->faceCanBeAdded( {2, 3, 11} ) );
  QVERIFY( !editor->faceCanBeAdded( {7, 8, 9} ) );
  QVERIFY( !editor->faceCanBeAdded( {7, 8, 9} ) );
  QVERIFY( !editor->faceCanBeAdded( {10, 12, 9} ) );
  QVERIFY( !editor->faceCanBeAdded( {13, 14, 15, 16} ) );
  QVERIFY( !editor->faceCanBeAdded( {0, 9, 10} ) );

  QVERIFY( editor->faceCanBeAdded( {2, 3, 8} ) );
  QVERIFY( editor->faceCanBeAdded( {2, 3, 10} ) );
  QVERIFY( editor->faceCanBeAdded( {7, 11, 8} ) );
}

void TestQgsMeshEditor::particularCases()
{
  {
    // unique shared vertex when removing without filling hole
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );

    mesh.vertices.append( QgsMeshVertex( 0, 0, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 100, 0, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 200, 0, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 300, 0, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 0, 100, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 100, 100, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 200, 100, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 300, 100, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 0, 200, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 100, 200, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 200, 200, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 300, 200, 0 ) );

    mesh.faces.append( {0, 1, 4} );
    mesh.faces.append( {1, 2, 6} );
    mesh.faces.append( {2, 3, 7} );
    mesh.faces.append( {1, 5, 4} );
    mesh.faces.append( {1, 6, 5} );
    mesh.faces.append( {2, 7, 6} );
    mesh.faces.append( {4, 5, 8} );
    mesh.faces.append( {6, 7, 11, 10} );
    mesh.faces.append( {5, 9, 8} );
    mesh.faces.append( {5, 10, 9} );
    mesh.faces.append( {5, 6, 10} );

    QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency() );

    QVERIFY( meshEditor.removeVertices( {5, 1}, false ) == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency() );

    meshEditor.mUndoStack->undo();
    meshEditor.mUndoStack->redo();
    meshEditor.mUndoStack->undo();

    QVERIFY( meshEditor.checkConsistency() );

    QVERIFY( meshEditor.removeVertices( {6}, false ) == QgsMeshEditingError() );

    meshEditor.stopEditing();

    QCOMPARE( meshEditor.mMesh->vertexCount(), 11 );
    QCOMPARE( meshEditor.mMesh->faceCount(), 6 );
  }

  {
    // remove boundary with fill hole, simple case
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );

    mesh.vertices.append( QgsMeshVertex( 0, 0, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 100, 0, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 200, 0, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 0, 100, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 200, 100, 0 ) );
    mesh.vertices.append( QgsMeshVertex( 100, 200, 0 ) );

    mesh.faces.append( {0, 1, 3} );
    mesh.faces.append( {1, 2, 4} );
    mesh.faces.append( {1, 5, 3} );
    mesh.faces.append( {1, 4, 5} );

    QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency() );

    QVERIFY( meshEditor.removeVertices( {5}, true ) == QgsMeshEditingError() );
  }

  {
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );

    mesh.vertices.append( QgsMeshVertex( 100, 100, 0 ) ); // 0
    mesh.vertices.append( QgsMeshVertex( 200, 110, 0 ) ); // 1
    mesh.vertices.append( QgsMeshVertex( 250, 130, 0 ) ); // 2
    mesh.vertices.append( QgsMeshVertex( 300, 110, 0 ) ); // 3
    mesh.vertices.append( QgsMeshVertex( 400, 120, 0 ) ); // 4
    mesh.vertices.append( QgsMeshVertex( 130, 000, 0 ) ); // 5
    mesh.vertices.append( QgsMeshVertex( 130, 400, 0 ) ); // 6

    mesh.faces.append( {5, 1, 0} );
    mesh.faces.append( {5, 3, 2, 1} );
    mesh.faces.append( {5, 4, 3} );


    QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency() );

    QVERIFY( meshEditor.isFaceGeometricallyCompatible( {1, 3, 2} ) );

    QgsMeshEditingError error;
    QVector<QgsMeshFace> facesToAdd( {{0, 1, 2}, {0, 2, 6}, {1, 3, 2}, {2, 3, 4}} );

    QgsTopologicalMesh::TopologicalFaces topologicFaces = meshEditor.mTopologicalMesh.createNewTopologicalFaces( facesToAdd, true, error );
    QVERIFY( error == QgsMeshEditingError() );
    error = meshEditor.mTopologicalMesh.canFacesBeAdded( topologicFaces );
    QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, 2 ) );
  }

  {
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );

    mesh.vertices.append( QgsMeshVertex( 100, 100, 0 ) ); // 0
    mesh.vertices.append( QgsMeshVertex( 200, 110, 0 ) ); // 1
    mesh.vertices.append( QgsMeshVertex( 250, 130, 0 ) ); // 2
    mesh.vertices.append( QgsMeshVertex( 300, 110, 0 ) ); // 3
    mesh.vertices.append( QgsMeshVertex( 400, 120, 0 ) ); // 4
    mesh.vertices.append( QgsMeshVertex( 130, 000, 0 ) ); // 5
    mesh.vertices.append( QgsMeshVertex( 130, 400, 0 ) ); // 6

    mesh.faces.append( {5, 1, 0} );
    mesh.faces.append( {5, 2, 1} );
    mesh.faces.append( {5, 3, 2} );
    mesh.faces.append( {5, 4, 3} );

    QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency() );

    QgsMeshEditingError error;
    QVector<QgsMeshFace> facesToAdd( {{1, 6, 0}, {1, 2, 6}, {2, 3, 6}, {3, 4, 6 }, {1, 3, 2} } );

    QgsTopologicalMesh::TopologicalFaces topologicFaces = meshEditor.mTopologicalMesh.createNewTopologicalFaces( facesToAdd, true, error );
    QVERIFY( error == QgsMeshEditingError() );
    error = meshEditor.mTopologicalMesh.canFacesBeAdded( topologicFaces );
    QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, 4 ) );

    QVERIFY( !meshEditor.isFaceGeometricallyCompatible( {1, 3, 2} ) );
  }

  {
    // massive remove
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );

    int sideSize = 100;

    for ( int i = 0; i < sideSize; ++i )
      for ( int j = 0; j < sideSize; ++j )
        mesh.vertices.append( QgsMeshVertex( i, j, 0 ) );

    for ( int i = 0; i < sideSize - 1; ++i )
      for ( int j = 0; j < sideSize - 1; ++j )
      {
        if ( j % 3 == 0 )
        {
          // add a quad
          mesh.faces.append( QgsMeshFace(
          {
            i * sideSize + j,
            ( i + 1 ) * sideSize + j,
            ( i + 1 ) * sideSize + j + 1,
            ( i ) * sideSize + j + 1} ) );
        }
        else
        {
          // add two triangles
          mesh.faces.append( QgsMeshFace( {i * sideSize + j,
                                           ( i + 1 ) * sideSize + j,
                                           ( i + 1 ) * sideSize + j + 1} ) );
          mesh.faces.append( QgsMeshFace( {i * sideSize + j,
                                           i  * sideSize + j + 1,
                                           ( i + 1 ) * sideSize + j + 1} ) );
        }
      }

    QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );

    for ( int i = 0; i < sideSize * sideSize; ++i )
    {
      if ( i < sideSize )
        QVERIFY( meshEditor.isVertexOnBoundary( i ) );
      if ( i > sideSize && i % sideSize == 0 )
      {
        QVERIFY( meshEditor.isVertexOnBoundary( i ) );
        QVERIFY( meshEditor.isVertexOnBoundary( i ) );
      }
      if ( i > sideSize * sideSize - sideSize )
        QVERIFY( meshEditor.isVertexOnBoundary( i ) );
    }

    QVERIFY( meshEditor.checkConsistency() );

    QList<int> verticesToRemove;
    for ( int i = 0; i < sideSize; ++i )
      for ( int j = 0; j < sideSize; ++j )
        if ( j > 1 && j < sideSize - 2 && i < sideSize - 20 )
          verticesToRemove.append( i * sideSize + j );

    QVERIFY( meshEditor.removeVertices( verticesToRemove, false ) == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency() );
    meshEditor.mUndoStack->undo();
    QVERIFY( meshEditor.checkConsistency() );
    QVERIFY( meshEditor.removeVertices( verticesToRemove, true ) == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency() );
  }
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
  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 5 ) );

  meshLayerQuadTriangle->undoStack()->undo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 0 );

  editor->addVertices(
  {
    {4000, 2000, 0}, // 5
    {4000, 3000, 0}  // 6
  }, 10 );

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 2 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 5 ) );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 6 ) );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );

  meshLayerQuadTriangle->undoStack()->undo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 0 );

  meshLayerQuadTriangle->undoStack()->redo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 2 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 5 ) );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 6 ) );

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
  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 5 ) );

  meshLayerQuadTriangle->undoStack()->undo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );
  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 2 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 5 ) );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 6 ) );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  meshLayerQuadTriangle->undoStack()->undo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 0 );

  meshLayerQuadTriangle->undoStack()->redo();
  meshLayerQuadTriangle->undoStack()->redo();
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 3 );

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 5 ) );

  // Add another face
  error = editor->addFaces( {{2, 5, 6}} );
  QVERIFY( error == QgsMeshEditingError() );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 4 );

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 0 );

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

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 2 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 5 ) );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 6 ) );

  meshLayerQuadTriangle->undoStack()->redo();
  meshLayerQuadTriangle->undoStack()->redo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3100, 2600, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3000, 2666.666666666 ), 1e-6 ) );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 3500, 2100, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 3666.6666666, 2333.33333333 ), 1e-6 ) );

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 0 );

  editor->removeFaces( {2, 3} );

  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 4 ); //removed faces are still present but empty
  QVERIFY( meshLayerQuadTriangle->nativeMesh()->face( 2 ).isEmpty() );
  QVERIFY( meshLayerQuadTriangle->nativeMesh()->face( 3 ).isEmpty() );

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 2 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 5 ) );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 6 ) );

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

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 0 );

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

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 8 ) );

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

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 0 );

  meshLayerQuadTriangle->undoStack()->redo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2050, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2266.66666666 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1150, 2340, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1166.6666666, 2600 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2950, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2933.33333333 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2700, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1833.33333333, 2600 ), 1e-6 ) );

  //flip edge
  QVERIFY( editor->edgeCanBeFlipped( 1, 3 ) );
  QVERIFY( !editor->edgeCanBeFlipped( 4, 3 ) );
  editor->flipEdge( 3, 1 );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 2100, 2050, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 2166.6666666, 2266.66666666 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 2050, 2800, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 2166.6666666, 2600 ), 1e-6 ) );

  meshLayerQuadTriangle->undoStack()->undo();

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2050, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2266.66666666 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1150, 2340, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1166.6666666, 2600 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2950, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 2933.33333333 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2700, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1833.33333333, 2600 ), 1e-6 ) );

  //merge
  QVERIFY( editor->canBeMerged( 1, 3 ) );
  QVERIFY( !editor->canBeMerged( 4, 3 ) );
  editor->merge( 3, 1 );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 2100, 2050, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 2166.6666666, 2422.22222222 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 2050, 2800, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 2166.6666666, 2422.22222222 ), 1e-6 ) ); //same face

  QVERIFY( !editor->canBeMerged( 2, 3 ) ); //leads to 5 vertices per face, limited to 4

  //split
  QVERIFY( editor->faceCanBeSplit( 8 ) );
  QCOMPARE( editor->splitFaces( {8} ), 1 );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 2100, 2500, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 2333.3333333, 2333.33333333 ), 1e-6 ) );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2500, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1833.3333333, 2600.0 ), 1e-6 ) );

  meshLayerQuadTriangle->undoStack()->undo();
  meshLayerQuadTriangle->undoStack()->undo();

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

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 8 ) );

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

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 3 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 8 ) );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 4 ) );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 0 ) );

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

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 8 ) );

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
  editor->mMaximumVerticesPerFace = 5; //for testing

  meshLayerQuadFlower->startFrameEditing( transform );
  editor = meshLayerQuadFlower->meshEditor();
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1500, 2800, -10 )}, 10 ), 1 ); // 8
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1800, 2700, -10 )}, 10 ), 1 ); // 9
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 12 );
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1400, 2300, -10 ), QgsPoint( 1500, 2200, -10 )}, 10 ), 2 ); // 10 & 11

  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 18 );
  QVERIFY( editor->checkConsistency() );

  // attempt to add a vertex under tolerance next existing one
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1499, 2801, -10 )}, 10 ), 0 );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 18 );

  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 700, 1750, 0 )}, 10 ), 1 ); // 12

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 18 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 13 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().contains( 12 ) );
  QVERIFY( editor->checkConsistency() );

  QVERIFY( editor->addFace( {0, 6, 12} ) == QgsMeshEditingError() );

  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );
  QVERIFY( editor->checkConsistency() );

  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1400, 2200, -10 )}, 10 ), 1 ); // 13

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 22 );
  QVERIFY( editor->checkConsistency() );

  QCOMPARE( meshLayerQuadFlower->datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( 1420, 2220 ), 10 ).x(), -10 );

  QVERIFY( editor->removeVertices( {0}, true ) == QgsMeshEditingError() );

  meshLayerQuadFlower->undoStack()->undo();

  QVERIFY( editor->removeVertices( {0} ) == QgsMeshEditingError() );
  QVERIFY( editor->checkConsistency() );

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

  editor->changeZValues( {6, 13, 12}, {-200.0, -200.0, -200.0} );
  QCOMPARE( meshLayerQuadFlower->datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPoint( 1300, 1800 ), 10 ).x(), -200 );

  QCOMPARE( editor->addVertices( {QgsMeshVertex( 750, 2500, 550 )}, 10 ), 1 );
  QCOMPARE( editor->addVertices( {QgsMeshVertex( 1200, 2500, 700 )}, 10 ), 1 );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1330, 2500, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1366.6666, 2533.3333 ), 1e-2 ) );

  QVERIFY( editor->edgeCanBeFlipped( 4, 3 ) );
  QVERIFY( editor->edgeCanBeFlipped( 1, 3 ) );
  QVERIFY( editor->edgeCanBeFlipped( 1, 10 ) );
  QVERIFY( editor->edgeCanBeFlipped( 14, 8 ) );
  QVERIFY( !editor->edgeCanBeFlipped( 4, 15 ) );
  QVERIFY( editor->edgeCanBeFlipped( 12, 10 ) );
  QVERIFY( !editor->edgeCanBeFlipped( 10, 13 ) );
  QVERIFY( !editor->edgeCanBeFlipped( 8, 9 ) );
  QVERIFY( !editor->edgeCanBeFlipped( 10, 17 ) );

  QVERIFY( editor->removeVertices( {10}, true ) == QgsMeshEditingError() );
  QVERIFY( editor->checkConsistency() );

  editor->mUndoStack->undo();

  QVERIFY( editor->removeVertices( {10}, true ) == QgsMeshEditingError() );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1330, 2500, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1400, 2500 ), 1e-2 ) );

  QVERIFY( editor->edgeCanBeFlipped( 14, 13 ) );
  editor->flipEdge( 14, 13 );

  editor->mUndoStack->undo();

  QVERIFY( editor->removeVertices( {13}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {11}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {9}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {8}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {15}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {14}, true ) == QgsMeshEditingError() );

  QVERIFY( editor->removeVertices( {7}, true ) == QgsMeshEditingError() );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 57 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 16 );
  QVERIFY( editor->checkConsistency() );

  meshLayerQuadFlower->commitFrameEditing( transform, true );
  QVERIFY( editor->checkConsistency() );

  QVERIFY( meshLayerQuadFlower->meshEditor() == editor );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  QVERIFY( editor->removeVertices( {3}, false ).errorType != Qgis::MeshEditingErrorType::NoError ); // leads to a topological error
  QCOMPARE( meshLayerQuadFlower->meshEditor()->addVertices( {{4000, 4000, 0}, {4000, 4100, 0}, {4100, 4000, 0}, {4100, 4100, 0}}, 10 ), 4 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 4 );

  //QVERIFY( editor->removeVertices( {3}, true ).errorType != Qgis::MeshEditingErrorType::NoError ); // filling after removing boundary not supported, so not fill and leads to a topological error

  QVERIFY( editor->removeVertices( {4}, true ) == QgsMeshEditingError() );
  QVERIFY( editor->checkConsistency() );

  meshLayerQuadFlower->commitFrameEditing( transform, false );

  QVERIFY( meshLayerQuadFlower->meshEditor() == nullptr );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 4 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 10 );

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


void TestQgsMeshEditor::refineMesh()
{
  auto checkRefinedFace = []( const QgsMesh & mesh,
                              const QHash<int, QgsMeshEditRefineFaces::FaceRefinement> &facesRefinement,
                              int faceIndex,
                              int refinedNeighborCount,
                              int centerVertexIndex,
                              int newBorderVertexCount,
                              int newFaceCount )
  {
    const QgsMeshEditRefineFaces::FaceRefinement &refinement = facesRefinement.value( faceIndex );
    int refinedNeighbor = 0;
    for ( int j = 0; j < mesh.face( faceIndex ).count(); ++j )
    {
      if ( refinement.refinedFaceNeighbor.at( j ) )
        refinedNeighbor++;
    }
    QCOMPARE( refinedNeighbor, refinedNeighborCount );
    QCOMPARE( refinement.newCenterVertexIndex, centerVertexIndex );
    QCOMPARE( refinement.newVerticesLocalIndex.count(), newBorderVertexCount );
    QCOMPARE( refinement.newFacesChangesIndex.count(), newFaceCount );
  };

  {
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );

    mesh.vertices.append( QgsMeshVertex( 100, 300, 0 ) ); // 0
    mesh.vertices.append( QgsMeshVertex( 100, 250, 0 ) ); // 1
    mesh.vertices.append( QgsMeshVertex( 200, 250, 0 ) ); // 2
    mesh.vertices.append( QgsMeshVertex( 200, 300, 0 ) ); // 3
    mesh.vertices.append( QgsMeshVertex( 100, 200, 0 ) ); // 4
    mesh.vertices.append( QgsMeshVertex( 200, 200, 0 ) ); // 5
    mesh.vertices.append( QgsMeshVertex( 300, 200, 0 ) ); // 6
    mesh.vertices.append( QgsMeshVertex( 200, 100, 0 ) ); // 7
    mesh.vertices.append( QgsMeshVertex( 100, 100, 0 ) ); // 8
    mesh.vertices.append( QgsMeshVertex( 0, 230, 0 ) ); // 9
    mesh.vertices.append( QgsMeshVertex( 0, 120, 0 ) ); // 10
    mesh.vertices.append( QgsMeshVertex( 0, 0, 0 ) ); // 11
    mesh.vertices.append( QgsMeshVertex( 100, 0, 0 ) ); // 12

    mesh.faces.append( {0, 1, 2, 3} ); // 0
    mesh.faces.append( {4, 5, 2, 1} ); // 1
    mesh.faces.append( {8, 7, 5, 4} ); // 2
    mesh.faces.append( {9, 8, 4} ); // 3
    mesh.faces.append( {10, 8, 9} ); // 4
    mesh.faces.append( {5, 7, 6} ); // 5
    mesh.faces.append( {10, 11, 12, 8} ); // 6
    mesh.faces.append( {8, 12, 7,} ); // 7

    QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency() );

    QList<int> facesList;
    facesList << 2 << 3 << 4;

    QgsMeshEditRefineFaces refineEditing;
    refineEditing.setInputFaces( facesList );
    QHash<int, QgsMeshEditRefineFaces::FaceRefinement > facesRefinement;
    QHash<int, QgsMeshEditRefineFaces::BorderFace> borderFaces;
    QSet<int> facesToRefine;
    facesToRefine = qgis::listToSet( facesList );

    refineEditing.createNewVerticesAndRefinedFaces( &meshEditor, facesToRefine, facesRefinement );
    refineEditing.createNewBorderFaces( &meshEditor, facesToRefine, facesRefinement, borderFaces );

    QCOMPARE( facesRefinement.count(), 3 );
    QCOMPARE( refineEditing.mVerticesToAdd.count(), 9 );
    QCOMPARE( refineEditing.mFacesToAdd.count(), 22 );
    QCOMPARE( facesRefinement.count(), 3 );
    QCOMPARE( borderFaces.count(), 4 );

    checkRefinedFace( mesh, facesRefinement, 2, 1, 4, 4, 4 );
    checkRefinedFace( mesh, facesRefinement, 3, 2, -1, 3, 4 );
    checkRefinedFace( mesh, facesRefinement, 4, 1, -1, 3, 4 );
  }


  {
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );

    int sideSize = 20;

    for ( int i = 0; i < sideSize; ++i )
      for ( int j = 0; j < sideSize; ++j )
        mesh.vertices.append( QgsMeshVertex( i, j, 0 ) );

    for ( int i = 0; i < sideSize - 1; ++i )
      for ( int j = 0; j < sideSize - 1; ++j )
      {
        if ( j % 3 == 0 )
        {
          // add a quad
          mesh.faces.append( QgsMeshFace(
          {
            i * sideSize + j,
            ( i + 1 ) * sideSize + j,
            ( i + 1 ) * sideSize + j + 1,
            ( i ) * sideSize + j + 1} ) );
        }
        else
        {
          // add two triangles
          mesh.faces.append( QgsMeshFace( {i * sideSize + j,
                                           ( i + 1 ) * sideSize + j,
                                           ( i + 1 ) * sideSize + j + 1} ) );
          mesh.faces.append( QgsMeshFace( {i * sideSize + j,
                                           i  * sideSize + j + 1,
                                           ( i + 1 ) * sideSize + j + 1} ) );
        }
      }

    QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );

    QgsMeshEditRefineFaces refineEditing;
    QList<int> facesList;
    for ( int i = 10; i < 20; ++i )
      facesList.append( i );

    refineEditing.setInputFaces( facesList );

    QSet<int> facesToRefine;
    QHash<int, QgsMeshEditRefineFaces::FaceRefinement> facesRefinement;
    QHash<int, QgsMeshEditRefineFaces::BorderFace> borderFaces;

    facesToRefine = qgis::listToSet( facesList );

    refineEditing.createNewVerticesAndRefinedFaces( &meshEditor, facesToRefine, facesRefinement );
    refineEditing.createNewBorderFaces( &meshEditor, facesToRefine, facesRefinement, borderFaces );

    QCOMPARE( facesRefinement.count(), 10 );
    QCOMPARE( refineEditing.mVerticesToAdd.count(), 25 );
    QCOMPARE( refineEditing.mFacesToAdd.count(), 59 );
    QCOMPARE( facesRefinement.count(), 10 );
    QCOMPARE( borderFaces.count(), 8 );

    for ( int i = 10; i < 20; ++i )
    {
      if ( i == 10 ) // first quad face
        checkRefinedFace( mesh, facesRefinement, i, 1, 4, 4, 4 );
      else if ( i == 15 ) //middle quad face
        checkRefinedFace( mesh, facesRefinement, i, 2, 16, 4, 4 );
      else if ( i == 19 ) //last triangle face
        checkRefinedFace( mesh, facesRefinement, i, 1, -1, 3, 4 );
      else
        checkRefinedFace( mesh, facesRefinement, i, 2, -1, 3, 4 );
    }
  }
}

QGSTEST_MAIN( TestQgsMeshEditor )
#include "testqgsmesheditor.moc"
