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
#include "qgstransformeffect.h"
#include "qgsmeshforcebypolylines.h"
#include "qgslinestring.h"


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
    void startEditingWithErrors();
    void createTopologicMesh();
    void editTopologicMesh();
    void badTopologicMesh();
    void meshEditorSimpleEdition();
    void faceIntersection();

    void meshEditorFromMeshLayer_quadTriangle();
    void meshEditorFromMeshLayer_quadFlower();

    void refineMesh();

    void transformByExpression();

    void forceByLine();

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

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;

  QVERIFY( meshLayerQuadTriangle->startFrameEditing( transform, error, false ) );
  QVERIFY( !meshLayerQuadTriangle->startFrameEditing( transform, error, false ) ); //mesh editing is already started

  // Edition has started, dataset groups are removed ans replace by a virtual one that represent the Z value of vertices
  QCOMPARE( meshLayerQuadTriangle->datasetGroupCount(), 1 );
  datasetGroupIndex = meshLayerQuadTriangle->datasetGroupsIndexes().at( 0 );
  meta = meshLayerQuadTriangle->datasetGroupMetadata( datasetGroupIndex );
  QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );
  QVERIFY( meta.name() == QLatin1String( "vertices Z value" ) );
  QCOMPARE( meta.isTemporal(), false );
  QCOMPARE( meta.isScalar(), true );
  QCOMPARE( meta.minimum(), 10.0 );
  QCOMPARE( meta.maximum(), 50.0 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );
  const QgsMesh mesh = *meshLayerQuadTriangle->nativeMesh();
  for ( int i = 0; i < mesh.vertexCount(); ++i )
    QCOMPARE( mesh.vertex( i ).z(), meshLayerQuadTriangle->datasetValue( QgsMeshDatasetIndex( 0, 0 ), i ).scalar() );

  QgsMeshEditor *editor = meshLayerQuadTriangle->meshEditor();
  QCOMPARE( editor->addVertices( {QgsMeshVertex( 1500, 2500, 0 )}, 10 ), 1 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 6 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 5 );

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
  QVERIFY( meta.name() == QLatin1String( "vertices Z value" ) );
  QCOMPARE( meta.isTemporal(), false );
  QCOMPARE( meta.isScalar(), true );
  QCOMPARE( meta.minimum(), 10.0 );
  QCOMPARE( meta.maximum(), 50.0 );

  QCOMPARE( editor->addVertices( {QgsMeshVertex( 1500, 2500, 0 )}, 10 ), 1 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 6 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 5 );

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
  QVERIFY( meta.name() == QLatin1String( "Bed Elevation" ) );
  QCOMPARE( meta.isTemporal(), false );
  QCOMPARE( meta.isScalar(), true );
  QCOMPARE( meta.minimum(), 10.0 );
  QCOMPARE( meta.maximum(), 50.0 );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );
}

void TestQgsMeshEditor::startEditingWithErrors()
{
  const QgsCoordinateTransform transform;

  QString uri( mDataDir + QStringLiteral( "/with_flat_face.2dm" ) );
  std::unique_ptr<QgsMeshLayer> mesh = std::make_unique<QgsMeshLayer>( uri, QStringLiteral( "With flat face" ), QStringLiteral( "mdal" ) );
  QVERIFY( mesh->isValid() );
  QCOMPARE( mesh->meshFaceCount(), 3 );
  QCOMPARE( mesh->meshVertexCount(), 5 );

  QgsMeshEditingError error;
  QVERIFY( !mesh->startFrameEditing( transform, error, false ) );
  QCOMPARE( error, QgsMeshEditingError( Qgis::MeshEditingErrorType::FlatFace, 2 ) );

  QVERIFY( mesh->startFrameEditing( transform, error, true ) );
  QCOMPARE( error, QgsMeshEditingError() );

  QCOMPARE( mesh->meshFaceCount(), 2 );
  QCOMPARE( mesh->meshVertexCount(), 5 );

  uri = mDataDir + QStringLiteral( "/with_manifold_face.2dm" );
  mesh = std::make_unique<QgsMeshLayer>( uri, QStringLiteral( "With manifold face" ), QStringLiteral( "mdal" ) );
  QVERIFY( mesh->isValid() );
  QCOMPARE( mesh->meshFaceCount(), 3 );
  QCOMPARE( mesh->meshVertexCount(), 5 );

  QVERIFY( !mesh->startFrameEditing( transform, error, false ) );
  QCOMPARE( error, QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, 1 ) );

  QVERIFY( mesh->startFrameEditing( transform, error, true ) );
  QCOMPARE( error, QgsMeshEditingError() );

  QCOMPARE( mesh->meshFaceCount(), 2 );
  QCOMPARE( mesh->meshVertexCount(), 5 );

  uri = mDataDir + QStringLiteral( "/with_free_vertex_in_mesh.2dm" );
  mesh = std::make_unique<QgsMeshLayer>( uri, QStringLiteral( "With free vertex in mesh" ), QStringLiteral( "mdal" ) );
  QVERIFY( mesh->isValid() );
  QCOMPARE( mesh->meshFaceCount(), 2 );
  QCOMPARE( mesh->meshVertexCount(), 6 );

  QVERIFY( !mesh->startFrameEditing( transform, error, false ) );
  QCOMPARE( error, QgsMeshEditingError( Qgis::MeshEditingErrorType::InvalidVertex, 5 ) );

  QVERIFY( mesh->startFrameEditing( transform, error, true ) );
  QCOMPARE( error, QgsMeshEditingError() );

  QCOMPARE( mesh->meshFaceCount(), 2 );
  QCOMPARE( mesh->meshVertexCount(), 5 );

  uri = mDataDir + QStringLiteral( "/with_unique_shared_vertex.2dm" );
  mesh = std::make_unique<QgsMeshLayer>( uri, QStringLiteral( "With unique shared vertex" ), QStringLiteral( "mdal" ) );
  QVERIFY( mesh->isValid() );
  QCOMPARE( mesh->meshFaceCount(), 3 );
  QCOMPARE( mesh->meshVertexCount(), 7 );

  QVERIFY( !mesh->startFrameEditing( transform, error, false ) );
  QCOMPARE( error, QgsMeshEditingError( Qgis::MeshEditingErrorType::UniqueSharedVertex, 2 ) );

  QVERIFY( mesh->startFrameEditing( transform, error, true ) );
  QCOMPARE( error, QgsMeshEditingError() );

  QCOMPARE( mesh->meshFaceCount(), 1 );
  QCOMPARE( mesh->meshVertexCount(), 6 );
}

static bool checkNeighbors( const QgsTopologicalMesh &mesh, int faceIndex, const QList<int> &expectedNeighbors )
{
  const QVector<int> neighbors = mesh.neighborsOfFace( faceIndex );
  bool ret = true;
  ret &= neighbors.count() == mesh.mesh()->face( faceIndex ).count();
  for ( const int exn : expectedNeighbors )
    ret &= neighbors.contains( exn ) ;

  return ret;
}

static bool checkFacesAround( const QgsTopologicalMesh &mesh, int vertexIndex, QList<int> expectedFace )
{
  const QList<int> facesAround = mesh.facesAroundVertex( vertexIndex );
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
  const QgsTopologicalMesh topologicMesh = QgsTopologicalMesh::createTopologicalMesh( &nativeMesh, 4, error );
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

  QgsMeshVertexCirculator circulator0( topologicMesh, 0 );
  QVERIFY( circulator0.isValid() );
  QCOMPARE( circulator0.degree(), 2 );

  QgsMeshVertexCirculator circulator1( topologicMesh, 1 );
  QVERIFY( circulator1.isValid() );
  QCOMPARE( circulator1.degree(), 3 );

  QgsMeshVertexCirculator circulator2( topologicMesh, 2 );
  QVERIFY( circulator2.isValid() );
  QCOMPARE( circulator2.degree(), 3 );

  QgsMeshVertexCirculator circulator3( topologicMesh, 3 );
  QVERIFY( circulator3.isValid() );
  QCOMPARE( circulator3.degree(), 4 );

  QgsMeshVertexCirculator circulator4( topologicMesh, 4 );
  QVERIFY( circulator4.isValid() );
  QCOMPARE( circulator4.degree(), 4 );

  QgsMeshVertexCirculator circulator5( topologicMesh, 5 );
  QVERIFY( circulator5.isValid() );
  QCOMPARE( circulator5.degree(), 2 );

  QgsMeshVertexCirculator circulator6( topologicMesh, 6 ); //no vertices with index 6
  QVERIFY( !circulator6.isValid() );
  QCOMPARE( circulator6.degree(), 0 );
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

  QVERIFY( topologicalMesh.canBeSplit( 0 ) );
  QVERIFY( !topologicalMesh.canBeSplit( 1 ) );
  QVERIFY( !topologicalMesh.canBeSplit( 2 ) );
  QVERIFY( !topologicalMesh.canBeSplit( 3 ) );

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
  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.facesCanBeAdded( topologicFaces ) ==
           QgsMeshEditingError( Qgis::MeshEditingErrorType::UniqueSharedVertex, 5 ) );

  faces = {{5, 7, 6, 4}};
  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.facesCanBeAdded( topologicFaces ) == QgsMeshEditingError() );

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

  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, true, error );

  QVERIFY( topologicalMesh.facesCanBeAdded( topologicFaces ) == QgsMeshEditingError() );

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

  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.facesCanBeAdded( topologicFaces ).errorType ==  Qgis::MeshEditingErrorType::UniqueSharedVertex );
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

  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.facesCanBeAdded( topologicFaces ) == QgsMeshEditingError() );

  faces =
  {
    {3, 5, 7, 6}, // 0 share vertices with same clockwise
    {6, 8, 4},    // 1
    {1, 4, 9},    // 2
    {10, 1, 9},   // 3
    {0, 1, 10},   // 4
    {11, 3, 0},   // 5
  };

  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, true, error );
  QVERIFY( error == QgsMeshEditingError() );
  error = topologicalMesh.facesCanBeAdded( topologicFaces );
  QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, 0 ) );

  faces = {{5, 7, 6, 4}};
  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.facesCanBeAdded( topologicFaces ) == QgsMeshEditingError() );
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

  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( faces, true, error );
  QVERIFY( topologicalMesh.facesCanBeAdded( topologicFaces ) == QgsMeshEditingError() );
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
  QVERIFY( topologicalMesh.facesCanBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {0, 1, 2, 3};
  QVERIFY( topologicalMesh.facesCanBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {0, 9};
  QVERIFY( topologicalMesh.facesCanBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {8, 0, 9, 10};
  QVERIFY( topologicalMesh.facesCanBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {1, 2, 3, 4, 5};
  QVERIFY( topologicalMesh.facesCanBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {0, 1, 2, 3, 4, 5};
  QVERIFY( topologicalMesh.facesCanBeRemoved( faceToRemove ).errorType == Qgis::MeshEditingErrorType::UniqueSharedVertex );

  faceToRemove = {9, 0, 1, 2, 3, 4, 5};
  QVERIFY( topologicalMesh.facesCanBeRemoved( faceToRemove ) == QgsMeshEditingError() );

  faceToRemove = {0, 6, 7, 8};
  QVERIFY( topologicalMesh.facesCanBeRemoved( faceToRemove ) == QgsMeshEditingError() );

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

  topologicalChanges.append( topologicalMesh.addVertexInFace( 4, {2.2, 0.5, 0} ) ); // vertex 12

  QVERIFY( checkFacesAround( topologicalMesh, 12, {11, 12, 13, 14} ) );

  topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( {{4, 8, 9}, {0, 3, 2, 1}}, true, error );
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

  QVERIFY( topologicalMesh.canBeSplit( 25 ) );
  topologicalChanges.append( topologicalMesh.splitFace( 25 ) );
  QVERIFY( checkFacesAround( topologicalMesh, 12, {11, 12, 20, 22, 26, 27} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 8, {18, 20, 27} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {16, 17, 18, 27, 26} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {16, 26, 22, 10} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  topologicalChanges.append( topologicalMesh.insertVertexInFacesEdge( 17, 0, QgsMeshVertex( 0.44, 0.94, 0.0 ) ) );
  QVERIFY( checkFacesAround( topologicalMesh, 13, {28, 29, 30, 31, 32} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 0, {8, 30, 31} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 1, {7, 8, 28, 30} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 2, {18, 27, 26, 29, 32} ) );
  QVERIFY( checkFacesAround( topologicalMesh, 3, {10, 22, 26, 32, 31} ) );
  QVERIFY( topologicalMesh.checkConsistency() == QgsMeshEditingError() );

  topologicalChanges.append( topologicalMesh.insertVertexInFacesEdge( 8, 2, QgsMeshVertex( -0.5, 0.25, 0.0 ) ) );
  QVERIFY( checkFacesAround( topologicalMesh, 14, {33, 34} ) );
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

  QCOMPARE( topologicalMesh.mesh()->faceCount(), 16 );
  QCOMPARE( topologicalMesh.mesh()->vertexCount(), 14 );
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
  const QUndoStack undoStack;

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  QVERIFY( meshLayerQuadTriangle->startFrameEditing( transform, error, false ) );

  QgsMesh mesh;
  QgsTriangularMesh triangularMesh;
  triangularMesh.update( &mesh, QgsCoordinateTransform() );

  QgsMeshEditor meshEditor( &mesh, &triangularMesh );
  QCOMPARE( meshEditor.initialize(), QgsMeshEditingError() );

  const QVector<QgsMeshVertex> vertices( {QgsPoint( 0.0, 0.0, 0.0 ), // 0
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
  QVERIFY( meshEditor.checkConsistency( error ) );

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
  QVERIFY( meshEditor.checkConsistency( error ) );

  // redo edition
  meshEditor.mUndoStack->redo();

  QCOMPARE( triangularMesh.faceCentroids().count(), 1 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.4, 0.5 ) ), 0 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 0.6, 0.5 ) ), 1 );
  QCOMPARE( triangularMesh.faceIndexesForRectangle( QgsRectangle( 0.1, 0.1, 0.7, 0.7 ) ).count(), 2 );
  QCOMPARE( triangularMesh.faceIndexForPoint_v2( QgsPointXY( 1, 0.5 ) ), -1 );
  QVERIFY( meshEditor.checkConsistency( error ) );

  QCOMPARE( meshEditor.mTopologicalMesh.neighborsOfFace( 0 ), QVector<int>( {-1, -1, -1, -1} ) );
  for ( int i = 0; i < 4; ++i )
    QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( i ), QList<int>( {0} ) );
  QCOMPARE( meshEditor.mTopologicalMesh.facesAroundVertex( 4 ), QList<int>() );

}

void TestQgsMeshEditor::faceIntersection()
{
  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  QVERIFY( meshLayerQuadFlower->startFrameEditing( transform, error, false ) );

  QgsMeshEditor *editor = meshLayerQuadFlower->meshEditor();
  QVERIFY( editor );

  // add some free vertices
  const QVector<QgsMeshVertex> vertices( {QgsPoint( 2500.0, 3500.0, 0.0 ), // 8
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
    QgsMeshEditingError error;

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

    const QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency( error ) );

    QVERIFY( meshEditor.removeVerticesWithoutFillHoles( {5, 1} ) == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency( error ) );

    meshEditor.mUndoStack->undo();
    meshEditor.mUndoStack->redo();
    meshEditor.mUndoStack->undo();

    QVERIFY( meshEditor.checkConsistency( error ) );

    QVERIFY( meshEditor.removeVerticesWithoutFillHoles( {6} ) == QgsMeshEditingError() );

    meshEditor.stopEditing();

    QCOMPARE( meshEditor.mMesh->vertexCount(), 11 );
    QCOMPARE( meshEditor.mMesh->faceCount(), 6 );
  }

  {
    // remove boundary with fill hole, simple case
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );
    QgsMeshEditingError error;

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

    const QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency( error ) );

    QVERIFY( meshEditor.removeVerticesFillHoles( {5} ) == QList<int>() );
  }

  {
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );
    QgsMeshEditingError error;

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


    const QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency( error ) );

    QVERIFY( meshEditor.isFaceGeometricallyCompatible( {1, 3, 2} ) );

    const QVector<QgsMeshFace> facesToAdd( {{0, 1, 2}, {0, 2, 6}, {1, 3, 2}, {2, 3, 4}} );

    const QgsTopologicalMesh::TopologicalFaces topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( facesToAdd, true, error );
    QVERIFY( error == QgsMeshEditingError() );
    error = meshEditor.mTopologicalMesh.facesCanBeAdded( topologicFaces );
    QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, 2 ) );
  }

  {
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );
    QgsMeshEditingError error;

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

    const QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency( error ) );

    const QVector<QgsMeshFace> facesToAdd( {{1, 6, 0}, {1, 2, 6}, {2, 3, 6}, {3, 4, 6 }, {1, 3, 2} } );

    const QgsTopologicalMesh::TopologicalFaces topologicFaces = QgsTopologicalMesh::createNewTopologicalFaces( facesToAdd, true, error );
    QVERIFY( error == QgsMeshEditingError() );
    error = meshEditor.mTopologicalMesh.facesCanBeAdded( topologicFaces );
    QVERIFY( error == QgsMeshEditingError( Qgis::MeshEditingErrorType::ManifoldFace, 4 ) );

    QVERIFY( !meshEditor.isFaceGeometricallyCompatible( {1, 3, 2} ) );
  }

  {
    // massive remove
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );
    QgsMeshEditingError error;

    const int sideSize = 40;

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

    QList<int> verticesToRemove;
    for ( int i = 0; i < sideSize; ++i )
      for ( int j = 0; j < sideSize; ++j )
        if ( j > 1 && j < sideSize - 2 && i < sideSize - 20 )
          verticesToRemove.append( i * sideSize + j );

    QVERIFY( meshEditor.removeVerticesWithoutFillHoles( verticesToRemove ) == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency( error ) );

    // just create a valid different transform to update the vertices of the triangular mesh
    transform = QgsCoordinateTransform( QgsCoordinateReferenceSystem( "EPSG:32620" ),
                                        QgsCoordinateReferenceSystem( "EPSG:32620" ),
                                        QgsCoordinateTransformContext() );
    triangularMesh.update( &mesh, transform );
    meshEditor.mUndoStack->undo();
    QVERIFY( meshEditor.checkConsistency( error ) );
    meshEditor.mUndoStack->redo();
  }

  {
    // remove vertex filling hole on boundary -- first configuration: all other vertex inside or only one outide
    QgsMesh mesh;
    mesh.vertices.append( QgsMeshVertex( 0, 4, 0 ) ); // 0
    mesh.vertices.append( QgsMeshVertex( 2, 2, 0 ) ); // 1
    mesh.vertices.append( QgsMeshVertex( 4, 2, 0 ) ); // 2
    mesh.vertices.append( QgsMeshVertex( 6, 2, 0 ) ); // 3
    mesh.vertices.append( QgsMeshVertex( 8, 4, 0 ) ); // 4
    mesh.vertices.append( QgsMeshVertex( 4, 10, 0 ) ); // 5
    mesh.vertices.append( QgsMeshVertex( 0, 2, 0 ) ); // 6
    mesh.vertices.append( QgsMeshVertex( 3, 0, 0 ) ); // 7
    mesh.vertices.append( QgsMeshVertex( 5, 0, 0 ) ); // 8
    mesh.vertices.append( QgsMeshVertex( 8, 2, 0 ) ); // 9

    mesh.faces.append( {0, 6, 1} );
    mesh.faces.append( {1, 7, 2} );
    mesh.faces.append( {2, 8, 3} );
    mesh.faces.append( {3, 9, 4} );
    mesh.faces.append( {0, 1, 5} );
    mesh.faces.append( {1, 2, 5} );
    mesh.faces.append( {2, 3, 5} );
    mesh.faces.append( {3, 4, 5} );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    QgsMeshEditingError error;
    QgsTopologicalMesh topologicMesh = QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    Q_ASSERT( error == QgsMeshEditingError() );

    QgsTopologicalMesh::Changes changes = topologicMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 1 );
    QCOMPARE( changes.removedFaces().count(), 4 );
    QCOMPARE( changes.addedFaces().count(), 3 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicMesh.reverseChanges( changes );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    // closing segment intersect one extremity of still existing edge,
    // with this, we can't do the operation because that leads to an unique shared vertex
    mesh.vertices[2] = QgsMeshVertex( 4, 4, 0 );

    changes = topologicMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 0 );
    QCOMPARE( changes.removedFaces().count(), 0 );
    QCOMPARE( changes.addedFaces().count(), 0 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicMesh.reverseChanges( changes );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    // with the intersecting vertex completely out
    mesh.vertices[2] = QgsMeshVertex( 4, 5, 0 );

    changes = topologicMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 0 );
    QCOMPARE( changes.removedFaces().count(), 0 );
    QCOMPARE( changes.addedFaces().count(), 0 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicMesh.reverseChanges( changes );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    // with adding a face to make vertex 2 not an boundary anymore
    QgsTopologicalMesh::Changes addFaceChanges = topologicMesh.addFaces( QgsTopologicalMesh::createNewTopologicalFaces( {{2, 7, 8}}, false, error ) );
    Q_ASSERT( error == QgsMeshEditingError() );

    changes = topologicMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 1 );
    QCOMPARE( changes.removedFaces().count(), 4 );
    QCOMPARE( changes.addedFaces().count(), 2 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicMesh.reverseChanges( changes );
    topologicMesh.reverseChanges( addFaceChanges );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    // try removing a face, no sufficient
    QgsTopologicalMesh::Changes removeFaceChange = topologicMesh.removeFaces( {1} );

    changes = topologicMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 0 );
    QCOMPARE( changes.removedFaces().count(), 0 );
    QCOMPARE( changes.addedFaces().count(), 0 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicMesh.reverseChanges( changes );
    topologicMesh.reverseChanges( removeFaceChange );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    // try removing two faces, no sufficient
    QgsTopologicalMesh::Changes remove2FacesChange = topologicMesh.removeFaces( {1, 2} );

    changes = topologicMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 0 );
    QCOMPARE( changes.removedFaces().count(), 0 );
    QCOMPARE( changes.addedFaces().count(), 0 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicMesh.reverseChanges( changes );
    topologicMesh.reverseChanges( remove2FacesChange );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );
  }

  {
    // remove vertex filling hole -- second configuration: all vertex outside
    QgsMesh mesh;
    mesh.vertices.append( QgsMeshVertex( 0, 2, 0 ) ); // 0
    mesh.vertices.append( QgsMeshVertex( 2, 4, 0 ) ); // 1
    mesh.vertices.append( QgsMeshVertex( 4, 5, 0 ) ); // 2
    mesh.vertices.append( QgsMeshVertex( 6, 4, 0 ) ); // 3
    mesh.vertices.append( QgsMeshVertex( 8, 2, 0 ) ); // 4
    mesh.vertices.append( QgsMeshVertex( 4, 10, 0 ) ); // 5
    mesh.vertices.append( QgsMeshVertex( 0, 0, 0 ) ); // 6
    mesh.vertices.append( QgsMeshVertex( 3, 0, 0 ) ); // 7
    mesh.vertices.append( QgsMeshVertex( 5, 0, 0 ) ); // 8
    mesh.vertices.append( QgsMeshVertex( 8, 0, 0 ) ); // 9

    mesh.faces.append( {0, 6, 1} );
    mesh.faces.append( {1, 7, 2} );
    mesh.faces.append( {2, 8, 3} );
    mesh.faces.append( {3, 9, 4} );
    mesh.faces.append( {0, 1, 5} );
    mesh.faces.append( {1, 2, 5} );
    mesh.faces.append( {2, 3, 5} );
    mesh.faces.append( {3, 4, 5} );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    QgsMeshEditingError error;
    QgsTopologicalMesh topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    Q_ASSERT( error == QgsMeshEditingError() );

    QgsTopologicalMesh::Changes changes = topologicalMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 0 );
    QCOMPARE( changes.removedFaces().count(), 0 );
    QCOMPARE( changes.addedFaces().count(), 0 );

    // try adding a face, not sufficient
    QgsTopologicalMesh::Changes addFaceChanges = topologicalMesh.addFaces( QgsTopologicalMesh::createNewTopologicalFaces( {{1, 6, 7}}, true, error ) );
    Q_ASSERT( error == QgsMeshEditingError() );

    changes = topologicalMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 0 );
    QCOMPARE( changes.removedFaces().count(), 0 );
    QCOMPARE( changes.addedFaces().count(), 0 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicalMesh.reverseChanges( changes );
    topologicalMesh.reverseChanges( addFaceChanges );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    // try adding two faces, not sufficient
    QgsTopologicalMesh::Changes add2FacesChanges =
    topologicalMesh.addFaces( QgsTopologicalMesh::createNewTopologicalFaces( {{1, 6, 7}, {2, 7, 8}}, true, error ) );
    Q_ASSERT( error == QgsMeshEditingError() );

    changes = topologicalMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 0 );
    QCOMPARE( changes.removedFaces().count(), 0 );
    QCOMPARE( changes.addedFaces().count(), 0 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicalMesh.reverseChanges( changes );
    topologicalMesh.reverseChanges( add2FacesChanges );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );

    // try adding three faces, good
    QgsTopologicalMesh::Changes add3FacesChanges =
    topologicalMesh.addFaces( QgsTopologicalMesh::createNewTopologicalFaces( {{1, 6, 7}, {2, 7, 8}, {3, 8, 9}}, true, error ) );
    Q_ASSERT( error == QgsMeshEditingError() );

    changes = topologicalMesh.removeVertexFillHole( 5 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 1 );
    QCOMPARE( changes.removedFaces().count(), 4 );
    QCOMPARE( changes.addedFaces().count(), 0 );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );

    topologicalMesh.reverseChanges( changes );
    topologicalMesh.reverseChanges( add3FacesChanges );

    QCOMPARE( mesh.vertexCount(), 10 );
    QCOMPARE( mesh.faceCount(), 8 );
  }

  {
    // remove vertex filling hole on boundary -- enclosed void
    QgsMesh mesh;
    mesh.vertices.append( QgsMeshVertex( 0, 2, 0 ) ); // 0
    mesh.vertices.append( QgsMeshVertex( 2, 2, 0 ) ); // 1
    mesh.vertices.append( QgsMeshVertex( 4, 5, 0 ) ); // 2
    mesh.vertices.append( QgsMeshVertex( 6, 2, 0 ) ); // 3
    mesh.vertices.append( QgsMeshVertex( 8, 2, 0 ) ); // 4
    mesh.vertices.append( QgsMeshVertex( 4, 10, 0 ) ); // 5
    mesh.vertices.append( QgsMeshVertex( 0, 0, 0 ) ); // 6
    mesh.vertices.append( QgsMeshVertex( 4, 0, 0 ) ); // 7
    mesh.vertices.append( QgsMeshVertex( 8, 0, 0 ) ); // 8

    mesh.faces.append( {6, 1, 0} ); // 0
    mesh.faces.append( {1, 6, 7} ); // 1
    mesh.faces.append( {1, 7, 3} ); // 2
    mesh.faces.append( {3, 7, 8} ); // 3
    mesh.faces.append( {3, 8, 4} ); // 4
    mesh.faces.append( {0, 1, 5} ); // 5
    mesh.faces.append( {1, 2, 5} ); // 6
    mesh.faces.append( {2, 3, 5} ); // 7
    mesh.faces.append( {3, 4, 5} ); // 8

    QCOMPARE( mesh.vertexCount(), 9 );
    QCOMPARE( mesh.faceCount(), 9 );

    QgsMeshEditingError error;
    QgsTopologicalMesh topologicalMesh = QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    Q_ASSERT( error == QgsMeshEditingError() );

    QVERIFY( topologicalMesh.isVertexOnBoundary( 1 ) );
    QVERIFY( topologicalMesh.isVertexOnBoundary( 2 ) );
    QVERIFY( topologicalMesh.isVertexOnBoundary( 3 ) );

    QgsTopologicalMesh::Changes changes = topologicalMesh.removeVertexFillHole( 2 );
    QCOMPARE( changes.verticesToRemoveIndexes().count(), 1 );
    QCOMPARE( changes.removedFaces().count(), 2 );
    QCOMPARE( changes.addedFaces().count(), 1 );

    QList<int> facesAround = topologicalMesh.facesAroundVertex( 3 );
    QCOMPARE( facesAround, QList<int>( {2, 3, 4, 8, 9} ) );
    facesAround = topologicalMesh.facesAroundVertex( 1 );
    QCOMPARE( facesAround, QList<int>( {0, 1, 2, 9, 5} ) );

    QVERIFY( !topologicalMesh.isVertexOnBoundary( 1 ) );
    QVERIFY( !topologicalMesh.isVertexOnBoundary( 3 ) );
    QVERIFY( topologicalMesh.edgeCanBeFlipped( 1, 3 ) );

    QgsTopologicalMesh::createTopologicalMesh( &mesh, 4, error );
    QVERIFY( error == QgsMeshEditingError() );
  }
}

void TestQgsMeshEditor::meshEditorFromMeshLayer_quadTriangle()
{
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 5 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  QVERIFY( meshLayerQuadTriangle->startFrameEditing( transform, error, false ) );

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

  // Add vertices under the tolerance from the edge inside the face
  editor->addVertices( {{2500, 2002, 0}}, 10 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 8 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 3 );
  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 2 );
  meshLayerQuadTriangle->undoStack()->undo();

  // Add vertices under the tolerance from the edge outside the face
  editor->addVertices( {{2500, 1998, 0}}, 10 );
  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 8 );
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 3 );
  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 2 );
  meshLayerQuadTriangle->undoStack()->undo();

  // try to add a face that shares only one vertex
  error = editor->addFaces( {{2, 5, 6}} );
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

  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 2 );
  QVERIFY( meshLayerQuadTriangle->nativeMesh()->face( 2 ).isEmpty() ); //removed faces are still present but empty
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
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 7 ); // vertex on a quad face : 4 faces created, 1 removed, removed are still present but void and not counted
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

  QVERIFY( editor->canBeMerged( 2, 3 ) );

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
  QCOMPARE( meshLayerQuadTriangle->meshFaceCount(), 7 );

  QCOMPARE( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadTriangle->meshEditor()->freeVerticesIndexes().contains( 8 ) );

  editor->removeVerticesWithoutFillHoles( {7} );

  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2050, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1150, 2340, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1400, 2950, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );
  centroid = meshLayerQuadTriangle->snapOnElement( QgsMesh::Face, QgsPoint( 1950, 2700, 0 ), 10 );
  QVERIFY( centroid.isEmpty() );

  QCOMPARE( meshLayerQuadTriangle->meshVertexCount(), 8 ); //empty vertex still presents but not counted

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

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  QVERIFY( meshLayerQuadFlower->startFrameEditing( transform, error, false ) );

  QgsMeshEditor *editor = meshLayerQuadFlower->meshEditor();
  QVERIFY( editor );
  editor->mMaximumVerticesPerFace = 5; //for testing

  meshLayerQuadFlower->startFrameEditing( transform, error, false );
  editor = meshLayerQuadFlower->meshEditor();
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1500, 2800, -10 )}, 10 ), 1 ); // 8
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 8 );
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1800, 2700, -10 )}, 10 ), 1 ); // 9
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 10 );
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1400, 2300, -10 ), QgsPoint( 1500, 2200, -10 )}, 10 ), 2 ); // 10 & 11

  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 14 );
  QVERIFY( editor->checkConsistency( error ) );

  // attempt to add a vertex under tolerance next existing one
  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1499, 2801, -10 )}, 10 ), 0 );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 14 );

  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 700, 1750, 0 )}, 10 ), 1 ); // 12

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 14 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 13 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 1 );
  QVERIFY( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().contains( 12 ) );
  QVERIFY( editor->checkConsistency( error ) );

  QVERIFY( editor->addFace( {0, 6, 12} ) == QgsMeshEditingError() );

  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );
  QVERIFY( editor->checkConsistency( error ) );

  QCOMPARE( editor->addPointsAsVertices( {QgsPoint( 1400, 2200, -10 )}, 10 ), 1 ); // 13

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 17 );
  QVERIFY( editor->checkConsistency( error ) );

  QCOMPARE( meshLayerQuadFlower->datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( 1420, 2220 ), 10 ).x(), -10 );

  QVERIFY( editor->removeVerticesFillHoles( {0} ) == QList<int>() );

  meshLayerQuadFlower->undoStack()->undo();

  QVERIFY( editor->removeVerticesWithoutFillHoles( {0} ) == QgsMeshEditingError() );
  QVERIFY( editor->checkConsistency( error ) );

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

  QVERIFY( editor->removeVerticesFillHoles( {10} ) == QList<int>() );
  QVERIFY( editor->checkConsistency( error ) );

  editor->mUndoStack->undo();

  QVERIFY( editor->removeVerticesFillHoles( {10} ) == QList<int>() );

  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPoint( 1330, 2500, 0 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1400, 2500 ), 1e-2 ) );

  QVERIFY( editor->edgeCanBeFlipped( 14, 13 ) );
  editor->flipEdge( 14, 13 );

  editor->mUndoStack->undo();

  QVERIFY( editor->removeVerticesFillHoles( {13} ) == QList<int>() );

  QVERIFY( editor->removeVerticesFillHoles( {11} ) == QList<int>() );

  QVERIFY( editor->removeVerticesFillHoles( {9} ) == QList<int>() );

  QVERIFY( editor->removeVerticesFillHoles( {8} ) == QList<int>() );

  QVERIFY( editor->removeVerticesFillHoles( {15} ) == QList<int>() );

  QVERIFY( editor->removeVerticesFillHoles( {14} ) == QList<int>() );

  QVERIFY( editor->removeVerticesFillHoles( {7} ) == QList<int>() );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 7 );
  QVERIFY( editor->checkConsistency( error ) );

  meshLayerQuadFlower->commitFrameEditing( transform, true );
  QVERIFY( editor->checkConsistency( error ) );

  QVERIFY( meshLayerQuadFlower->meshEditor() == editor );

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 7 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  QVERIFY( editor->removeVerticesWithoutFillHoles( {3} ).errorType != Qgis::MeshEditingErrorType::NoError ); // leads to a topological error
  QCOMPARE( meshLayerQuadFlower->meshEditor()->addVertices( {{4000, 4000, 0}, {4000, 4100, 0}, {4100, 4000, 0}, {4100, 4100, 0}}, 10 ), 4 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 4 );

  //QVERIFY( editor->removeVertices( {3}, true ).errorType != Qgis::MeshEditingErrorType::NoError ); // filling after removing boundary not supported, so not fill and leads to a topological error

  QVERIFY( editor->removeVerticesFillHoles( {4} ) == QList<int>() );
  QVERIFY( editor->checkConsistency( error ) );

  meshLayerQuadFlower->commitFrameEditing( transform, true );

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

  QVERIFY( meshLayerQuadFlower->reindex( transform, true ) );
  meshLayerQuadFlower->commitFrameEditing( transform, false );
  QVERIFY( meshLayerQuadFlower->meshEditor() == nullptr );

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
    QgsMeshEditingError error;

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

    const QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency( error ) );

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

    const int sideSize = 20;

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

    const QgsCoordinateTransform transform;
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
  {
    // Attempt to refine a face with 5 vertices (not allowed)
    QgsMesh mesh;
    QgsTriangularMesh triangularMesh;
    QgsMeshEditor meshEditor( &mesh, &triangularMesh );
    QgsMeshEditingError error;

    mesh.vertices.append( QgsMeshVertex( 0, 200, 0 ) ); // 0
    mesh.vertices.append( QgsMeshVertex( 200, 200, 0 ) ); // 1
    mesh.vertices.append( QgsMeshVertex( 0, 0, 0 ) ); // 2
    mesh.vertices.append( QgsMeshVertex( 200, 0, 0 ) ); //
    mesh.vertices.append( QgsMeshVertex( 100, 175, 0 ) ); // 4
    mesh.vertices.append( QgsMeshVertex( 25, 150, 0 ) ); // 5
    mesh.vertices.append( QgsMeshVertex( 25, 25, 0 ) ); // 6
    mesh.vertices.append( QgsMeshVertex( 175, 25, 0 ) ); // 7
    mesh.vertices.append( QgsMeshVertex( 175, 150, 0 ) ); // 8

    mesh.faces.append( {2, 5, 0 } ); // 0
    mesh.faces.append( {0, 5, 4 } ); // 1
    mesh.faces.append( {0, 4, 1 } ); // 2
    mesh.faces.append( {1, 4, 8 } ); // 3
    mesh.faces.append( {4, 5, 6, 7, 8 } ); // 4
    mesh.faces.append( {2, 6, 5 } ); // 5
    mesh.faces.append( {6, 2, 3, 7 } ); // 6
    mesh.faces.append( {3, 8, 7} ); // 7

    const QgsCoordinateTransform transform;
    triangularMesh.update( &mesh, transform );
    QVERIFY( meshEditor.initialize() == QgsMeshEditingError() );
    QVERIFY( meshEditor.checkConsistency( error ) );

    QCOMPARE( meshEditor.mMesh->faceCount(), 8 );
    QCOMPARE( meshEditor.mMesh->vertexCount(), 9 );

    QList<int> facesList;
    facesList << 4;

    QgsMeshEditRefineFaces refineEditing;
    refineEditing.setInputFaces( facesList );
    QHash<int, QgsMeshEditRefineFaces::FaceRefinement > facesRefinement;
    QHash<int, QgsMeshEditRefineFaces::BorderFace> borderFaces;
    QSet<int> facesToRefine;
    facesToRefine = qgis::listToSet( facesList );

    refineEditing.createNewVerticesAndRefinedFaces( &meshEditor, facesToRefine, facesRefinement );
    refineEditing.createNewBorderFaces( &meshEditor, facesToRefine, facesRefinement, borderFaces );

    // refinement not done
    QVERIFY( facesRefinement.isEmpty() );
    QVERIFY( borderFaces.isEmpty() );

    facesList.clear();
    facesList << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7;
    refineEditing = QgsMeshEditRefineFaces();
    refineEditing.setInputFaces( facesList );
    facesRefinement.clear();
    borderFaces.clear();
    facesToRefine.clear();
    facesToRefine = qgis::listToSet( facesList );

    refineEditing.createNewVerticesAndRefinedFaces( &meshEditor, facesToRefine, facesRefinement );
    refineEditing.createNewBorderFaces( &meshEditor, facesToRefine, facesRefinement, borderFaces );

    QVERIFY( !facesRefinement.isEmpty() );
    QVERIFY( !borderFaces.isEmpty() );

  }
}

void TestQgsMeshEditor::transformByExpression()
{
  std::unique_ptr<QgsMeshLayer> layer = std::make_unique<QgsMeshLayer>( mDataDir + "/quad_flower_to_edit.2dm", "mesh", "mdal" );

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  layer->startFrameEditing( transform, error, false );

  QgsMeshTransformVerticesByExpression transformVertex;

  transformVertex.setExpressions( QStringLiteral( "$vertex_x + 50" ), QStringLiteral( "$vertex_y - 50" ), QStringLiteral( "$vertex_z + 100" ) );

  // no input set
  QVERIFY( !transformVertex.calculate( layer.get() ) );

  transformVertex.setInputVertices( {0, 1, 3, 4} );

  QVERIFY( transformVertex.calculate( layer.get() ) );

  QCOMPARE( transformVertex.mChangeCoordinateVerticesIndexes, QList<int>( {0, 1, 3, 4} ) );
  QVERIFY( transformVertex.mOldXYValues.at( 0 ).compare( QgsPointXY( 1000, 2000 ), 0.1 ) );
  QVERIFY( transformVertex.mOldXYValues.at( 1 ).compare( QgsPointXY( 2000, 2000 ), 0.1 ) );
  QVERIFY( transformVertex.mOldXYValues.at( 2 ).compare( QgsPointXY( 2000, 3000 ), 0.1 ) );
  QVERIFY( transformVertex.mOldXYValues.at( 3 ).compare( QgsPointXY( 1000, 3000 ), 0.1 ) );
  QVERIFY( transformVertex.mNewXYValues.at( 0 ).compare( QgsPointXY( 1050, 1950 ), 0.1 ) );
  QVERIFY( transformVertex.mNewXYValues.at( 1 ).compare( QgsPointXY( 2050, 1950 ), 0.1 ) );
  QVERIFY( transformVertex.mNewXYValues.at( 2 ).compare( QgsPointXY( 2050, 2950 ), 0.1 ) );
  QVERIFY( transformVertex.mNewXYValues.at( 3 ).compare( QgsPointXY( 1050, 2950 ), 0.1 ) );
  QCOMPARE( transformVertex.mNewZValues.at( 0 ), 300 );
  QCOMPARE( transformVertex.mNewZValues.at( 1 ), 300 );
  QCOMPARE( transformVertex.mNewZValues.at( 2 ), 300 );
  QCOMPARE( transformVertex.mNewZValues.at( 3 ), 300 );

  layer->meshEditor()->advancedEdit( &transformVertex );

  QgsMesh &mesh = *layer->nativeMesh();

  QVERIFY( QgsPoint( 1050, 1950, 300 ).compareTo( &mesh.vertices.at( 0 ) ) == 0 );
  QVERIFY( QgsPoint( 2050, 1950, 300 ).compareTo( &mesh.vertices.at( 1 ) )  == 0 );
  QVERIFY( QgsPoint( 2500, 2500, 800 ).compareTo( &mesh.vertices.at( 2 ) )  == 0 );
  QVERIFY( QgsPoint( 2050, 2950, 300 ).compareTo( &mesh.vertices.at( 3 ) )  == 0 );
  QVERIFY( QgsPoint( 1050, 2950, 300 ).compareTo( &mesh.vertices.at( 4 ) )  == 0 );
  QVERIFY( QgsPoint( 500, 2500, 800 ).compareTo( &mesh.vertices.at( 5 ) )  == 0 );
  QVERIFY( QgsPoint( 1500, 1500, 800 ).compareTo( &mesh.vertices.at( 6 ) )  == 0 );
  QVERIFY( QgsPoint( 1500, 3500, 800 ).compareTo( &mesh.vertices.at( 7 ) )  == 0 );

  layer->undoStack()->undo();
  mesh = *layer->nativeMesh();

  QVERIFY( QgsPoint( 1000, 2000, 200 ).compareTo( &mesh.vertices.at( 0 ) ) == 0 );
  QVERIFY( QgsPoint( 2000, 2000, 200 ).compareTo( &mesh.vertices.at( 1 ) )  == 0 );
  QVERIFY( QgsPoint( 2500, 2500, 800 ).compareTo( &mesh.vertices.at( 2 ) )  == 0 );
  QVERIFY( QgsPoint( 2000, 3000, 200 ).compareTo( &mesh.vertices.at( 3 ) )  == 0 );
  QVERIFY( QgsPoint( 1000, 3000, 200 ).compareTo( &mesh.vertices.at( 4 ) )  == 0 );
  QVERIFY( QgsPoint( 500, 2500, 800 ).compareTo( &mesh.vertices.at( 5 ) )  == 0 );
  QVERIFY( QgsPoint( 1500, 1500, 800 ).compareTo( &mesh.vertices.at( 6 ) )  == 0 );
  QVERIFY( QgsPoint( 1500, 3500, 800 ).compareTo( &mesh.vertices.at( 7 ) )  == 0 );

  layer->undoStack()->redo();
  mesh = *layer->nativeMesh();

  QVERIFY( QgsPoint( 1050, 1950, 300 ).compareTo( &mesh.vertices.at( 0 ) ) == 0 );
  QVERIFY( QgsPoint( 2050, 1950, 300 ).compareTo( &mesh.vertices.at( 1 ) )  == 0 );
  QVERIFY( QgsPoint( 2500, 2500, 800 ).compareTo( &mesh.vertices.at( 2 ) )  == 0 );
  QVERIFY( QgsPoint( 2050, 2950, 300 ).compareTo( &mesh.vertices.at( 3 ) )  == 0 );
  QVERIFY( QgsPoint( 1050, 2950, 300 ).compareTo( &mesh.vertices.at( 4 ) )  == 0 );
  QVERIFY( QgsPoint( 500, 2500, 800 ).compareTo( &mesh.vertices.at( 5 ) )  == 0 );
  QVERIFY( QgsPoint( 1500, 1500, 800 ).compareTo( &mesh.vertices.at( 6 ) )  == 0 );
  QVERIFY( QgsPoint( 1500, 3500, 800 ).compareTo( &mesh.vertices.at( 7 ) )  == 0 );

  layer->undoStack()->undo();

  // leads to an invalid mesh
  transformVertex.clear();
  transformVertex.setInputVertices( {1, 3} );
  transformVertex.setExpressions( QStringLiteral( "$vertex_x -1500" ), QStringLiteral( "$vertex_y - 1500" ), QString() );

  QVERIFY( !transformVertex.calculate( layer.get() ) );

  // transforme with intersecting existing faces
  transformVertex.clear();
  transformVertex.setInputVertices( {2, 3, 7} );
  transformVertex.setExpressions( QStringLiteral( "$vertex_x+700" ), QStringLiteral( "$vertex_y + 700" ), QString() );

  QVERIFY( transformVertex.calculate( layer.get() ) );

  // add a other face that will intersects transformed ones
  layer->meshEditor()->addVertices(
  {
    {2000, 3500, 0},  // 8
    {2500, 3500, 10}, // 9
    {2500, 4000, 20}} // 10
  , 1 );

  QVERIFY( !transformVertex.calculate( layer.get() ) );

  layer->meshEditor()->addFace( {8, 9, 10} );

  QVERIFY( !transformVertex.calculate( layer.get() ) );

  // undo adding vertices and face
  layer->undoStack()->undo();
  layer->undoStack()->undo();

  QVERIFY( transformVertex.calculate( layer.get() ) );

  // composed expression
  transformVertex.clear();
  transformVertex.setInputVertices( {0, 1, 2, 3, 4, 5, 6, 7} );
  transformVertex.setExpressions( QStringLiteral( "$vertex_y + 50" ),
                                  QStringLiteral( "-$vertex_x" ),
                                  QStringLiteral( "if( $vertex_x <= 1500 , $vertex_z + 80 , $vertex_z - 150)" ) );

  QVERIFY( transformVertex.calculate( layer.get() ) );
  layer->meshEditor()->advancedEdit( &transformVertex );

  mesh = *layer->nativeMesh();

  QVERIFY( QgsPoint( 2050, -1000, 280 ).compareTo( &mesh.vertices.at( 0 ) ) == 0 );
  QVERIFY( QgsPoint( 2050, -2000, 50 ).compareTo( &mesh.vertices.at( 1 ) )  == 0 );
  QVERIFY( QgsPoint( 2550, -2500, 650 ).compareTo( &mesh.vertices.at( 2 ) )  == 0 );
  QVERIFY( QgsPoint( 3050, -2000, 50 ).compareTo( &mesh.vertices.at( 3 ) )  == 0 );
  QVERIFY( QgsPoint( 3050, -1000, 280 ).compareTo( &mesh.vertices.at( 4 ) )  == 0 );
  QVERIFY( QgsPoint( 2550, -500, 880 ).compareTo( &mesh.vertices.at( 5 ) )  == 0 );
  QVERIFY( QgsPoint( 1550, -1500, 880 ).compareTo( &mesh.vertices.at( 6 ) )  == 0 );
  QVERIFY( QgsPoint( 3550, -1500, 880 ).compareTo( &mesh.vertices.at( 7 ) )  == 0 );

  layer->undoStack()->undo();


  // move only a free vertex in an existing face
  layer->meshEditor()->addVertices( {QgsMeshVertex( 2500, 3500, 0 )}, 10 );
  QVERIFY( layer->meshVertexCount() == 9 );

  transformVertex.clear();
  transformVertex.setInputVertices( {8} );
  transformVertex.setExpressions( QStringLiteral( "$vertex_x - 1000" ),
                                  QStringLiteral( "$vertex_y - 1000" ),
                                  QLatin1String( "" ) );

  QVERIFY( !transformVertex.calculate( layer.get() ) );

  transformVertex.clear();
  transformVertex.setInputVertices( {8} );

  transformVertex.setExpressions( QStringLiteral( "$vertex_x + 1000" ),
                                  QStringLiteral( "$vertex_y + 1000" ),
                                  QLatin1String( "" ) );

  QVERIFY( transformVertex.calculate( layer.get() ) );
}

void TestQgsMeshEditor::forceByLine()
{
  QString uri( mDataDir + "/refined_quad_flower.2dm" );
  std::unique_ptr<QgsMeshLayer> meshLayer = std::make_unique<QgsMeshLayer>( uri, "mesh layer", "mdal" );
  QgsMeshEditingError error;

  QVERIFY( meshLayer->isValid() );
  QgsCoordinateTransform transform;
  QVERIFY( meshLayer->startFrameEditing( transform, error, false ) );

  QVERIFY( meshLayer->meshEditor() );


  int initialFaceCount = meshLayer->nativeMesh()->faceCount();
  int initialVertexCount = meshLayer->nativeMesh()->vertexCount();

  QgsMeshEditForceByLine forceByLine;

  // begin exterior, end exterior, snap on vertex for all the line without splitting faces
  forceByLine.setInputLine( QgsPoint( 750, 2999, 0 ), QgsPoint( 2250, 2999, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount );

  meshLayer->undoStack()->undo();

  // begin interior, end exterior,  snap on vertex for all the line without splitting faces
  forceByLine.setInputLine( QgsPoint( 1001, 2999, 0 ), QgsPoint( 2250, 3000, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount );

  meshLayer->undoStack()->undo();

  // begin exterior, end exterior, snap on vertex for all the line, splitting one quad
  forceByLine.setInputLine( QgsPoint( 750, 2251, 0 ), QgsPoint( 2250, 2249, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 2 );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount );

  meshLayer->undoStack()->undo();

  //begin exterior, end exterior, snap on vertex when entering the mesh and cut edge
  forceByLine.setInputLine( QgsPoint( 500, 3001, 0 ), QgsPoint( 2000, 1499, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 4 );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount );

  meshLayer->undoStack()->undo();

  // begin exterior, end exterior, snap on vertex when entering the mesh and cut edge AND insert vertex when intersection with edge
  forceByLine.setInputLine( QgsPoint( 500, 3001, 0 ), QgsPoint( 2000, 1499, 0 ), 2, true );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 8 );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 2 );

  meshLayer->undoStack()->undo();

  // begin exterior, end exterior, cut edge when entering the mesh
  forceByLine.setInputLine( QgsPoint( 1000, 1751, 0 ), QgsPoint( 2250, 2999, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 8 );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 2 );

  meshLayer->undoStack()->undo();

  // begin interior snapping vertex, end exterior
  forceByLine.setInputLine( QgsPoint( 999, 2251, 0 ), QgsPoint( 2250, 3000, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 1 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 13 );

  meshLayer->undoStack()->undo();

  // begin interior No snapping vertex and snap vertex just after
  forceByLine.setInputLine( QgsPoint( 1010, 2251, 0 ), QgsPoint( 2500, 2249, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 1 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 4 );

  meshLayer->undoStack()->undo();

  // begin interior No snapping vertex and cut edge just after
  forceByLine.setInputLine( QgsPoint( 1250, 2250, 0 ), QgsPoint( 2250, 3250, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 1 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 10 );

  meshLayer->undoStack()->undo();

  // begin interior No snapping vertex and cut edge just after with insertion of point
  forceByLine.setInputLine( QgsPoint( 1250, 2250, 0 ), QgsPoint( 2250, 3250, 0 ), 2, true );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 2 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 12 );

  meshLayer->undoStack()->undo();

  // begin interior No snapping vertex and finish interior without snapping vertex just after a snapped vertex
  forceByLine.setInputLine( QgsPoint( 1250, 2250, 0 ), QgsPoint( 1950, 2950, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 2 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 12 );

  meshLayer->undoStack()->undo();

  // begin interior No snapping vertex and finish interior without snapping vertex just after cutting an edge, without intersection on edges
  forceByLine.setInputLine( QgsPoint( 1250, 2250, 0 ), QgsPoint( 1850, 3050, 0 ), 2, false );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 2 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 18 );

  meshLayer->undoStack()->undo();

  // begin interior No snapping vertex and finish interior without snapping vertex just after cutting an edge, without intersection on edges
  forceByLine.setInputLine( QgsPoint( 1250, 2250, 0 ), QgsPoint( 1850, 3050, 0 ), 2, true );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 8 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 30 );

  meshLayer->undoStack()->undo();

  // a short line in only one face
  forceByLine.setInputLine( QgsPoint( 1200, 2350, 0 ), QgsPoint( 1350, 2150, 0 ), 2, true );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 2 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 7 );

  meshLayer->undoStack()->undo();

  forceByLine.setInputLine( QgsPoint( 3170, 2210, 0 ), QgsPoint( 4775, 2680, 0 ), 10, true );
  meshLayer->meshEditor()->advancedEdit( &forceByLine );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 3 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 12 );

  meshLayer->undoStack()->undo();

  QgsMeshEditForceByPolylines forceByPolyline1;
  forceByPolyline1.setTolerance( 5 );

  std::unique_ptr<QgsLineString> lineString = std::make_unique<QgsLineString>();
  lineString->addVertex( {1250, 2250, 5} );
  lineString->addVertex( {1850, 2850, 20} );
  lineString->addVertex( {1850, 0, 150} );
  forceByPolyline1.addLineFromGeometry( QgsGeometry( lineString.release() ) );

  meshLayer->meshEditor()->advancedEdit( &forceByPolyline1 );

  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 3 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 21 );

  meshLayer->undoStack()->undo();

  QgsMeshEditForceByPolylines forceByPolyline2;
  lineString = std::make_unique<QgsLineString>();
  lineString->addVertex( {1250, 2250, 5} );
  lineString->addVertex( {1850, 2850, 20} );
  lineString->addVertex( {1850, 0, 150} );
  forceByPolyline2.addLineFromGeometry( QgsGeometry( lineString.release() ) );
  forceByPolyline2.clear();
  forceByPolyline2.setAddVertexOnIntersection( true );

  meshLayer->meshEditor()->advancedEdit( &forceByPolyline2 );
  QVERIFY( meshLayer->meshEditor()->checkConsistency( error ) );
  QCOMPARE( meshLayer->nativeMesh()->vertexCount(), initialVertexCount + 8 );
  QCOMPARE( meshLayer->nativeMesh()->faceCount(), initialFaceCount + 31 );

}

QGSTEST_MAIN( TestQgsMeshEditor )
#include "testqgsmesheditor.moc"
