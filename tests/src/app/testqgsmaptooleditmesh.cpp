/***************************************************************************
  testqgsmaptooleditmesh.cpp

 ---------------------
 begin                : 25.6.2021
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
#include "qgisapp.h"
#include "testqgsmaptoolutils.h"
#include "qgsmaptooleditmeshframe.h"
#include "qgsmeshlayer.h"
#include "qgsmesheditor.h"

class TestQgsMapToolEditMesh : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolEditMesh() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase() {};// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void editMesh();

  private:
    QgisApp *mQgisApp = nullptr;
    std::unique_ptr<QgsMeshLayer> meshLayerQuadFlower;
    QString mDataDir;
    std::unique_ptr<QgsMapCanvas> canvas;
    QgsMapToolEditMeshFrame *editMeshMapTool;
};


//runs before all tests
void TestQgsMapToolEditMesh::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/mesh";
  canvas.reset( new QgsMapCanvas() );
  editMeshMapTool = new QgsMapToolEditMeshFrame( canvas.get() );
}

void TestQgsMapToolEditMesh::init()
{
  QString uri = QString( mDataDir + "/quad_flower.2dm" );
  meshLayerQuadFlower.reset( new QgsMeshLayer( uri, "Quad Flower", "mdal" ) );
  QVERIFY( meshLayerQuadFlower );
  QCOMPARE( meshLayerQuadFlower->datasetGroupCount(), 1 );

  QgsCoordinateTransform transform;
  meshLayerQuadFlower->startFrameEditing( transform );

  canvas->setLayers( QList<QgsMapLayer *>() << meshLayerQuadFlower.get() );
}

void TestQgsMapToolEditMesh::editMesh()
{
  double offsetInMapUnits = 15 * canvas->mapSettings().mapUnitsPerPixel();

  QgsCoordinateTransform transform;
  QVERIFY( meshLayerQuadFlower->meshEditor() );

  TestQgsMapToolAdvancedDigitizingUtils tool( editMeshMapTool );
  canvas->setCurrentLayer( meshLayerQuadFlower.get() );
  editMeshMapTool->mActionDigitizing->trigger();

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 8 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  tool.mouseDoubleClick( 1500, 2800, Qt::LeftButton ); //add a vertex on the face 0
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  meshLayerQuadFlower->undoStack()->undo();

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );

  meshLayerQuadFlower->rollBackFrameEditing( transform, false );

  QVERIFY( !meshLayerQuadFlower->meshEditor() );

  meshLayerQuadFlower->startFrameEditing( transform );

  QVERIFY( meshLayerQuadFlower->meshEditor() );

  // redo
  tool.mouseDoubleClick( 1500, 2800, Qt::LeftButton );
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  // attempt to add a vertex on place of existing one
  tool.mouseDoubleClick( 2500, 2500, Qt::LeftButton );
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  // add a free vertex
  tool.mouseDoubleClick( 2500, 3500, Qt::LeftButton );  // 9
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 10 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 1 );

  // add a face
  tool.mouseMove( 1999, 2999 ); //move near a vertex
  tool.mouseMove( 2000 + offsetInMapUnits / sqrt( 2 ), 3000 + offsetInMapUnits / sqrt( 2 ) ); //move on the new face marker
  tool.mouseClick( 2000 + offsetInMapUnits / sqrt( 2 ), 3000 + offsetInMapUnits / sqrt( 2 ), Qt::LeftButton );
  tool.mouseMove( 2499, 3501 ); //move near the new free vertex
  tool.mouseClick( 2501, 3499, Qt::LeftButton ); // click near the vertex
  tool.mouseMove( 2490, 2600 ); //move elsewhere
  tool.mouseMove( 2495, 2500 ); //move near another vertex
  tool.mouseClick( 2495, 2500, Qt::LeftButton ); // click near the vertex
  tool.mouseClick( 5000, 5000, Qt::RightButton ); // valid the face

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 10 );

  // Remove vertex 7 and 9
  tool.mouseMove( 1500, 3250 );
  tool.mouseMove( 1500, 3498 );
  tool.mouseClick( 1501, 3501, Qt::LeftButton );
  tool.mouseMove( 1500, 3250 );
  tool.mouseMove( 2500, 3500 );
  tool.mouseClick( 2500, 3500, Qt::LeftButton, Qt::ShiftModifier );
  tool.keyClick( Qt::Key_Delete, Qt::ControlModifier | Qt::ShiftModifier ); //remove without filling hole

  QCOMPARE( meshLayerQuadFlower->nativeMesh()->vertex( 7 ), QgsMeshVertex() );
  QCOMPARE( meshLayerQuadFlower->nativeMesh()->vertex( 9 ), QgsMeshVertex() );
  QCOMPARE( meshLayerQuadFlower->nativeMesh()->face( 4 ), QgsMeshFace() );
  QCOMPARE( meshLayerQuadFlower->nativeMesh()->face( 9 ), QgsMeshFace() );

  meshLayerQuadFlower->undoStack()->undo();

  QVERIFY( meshLayerQuadFlower->nativeMesh()->vertex( 7 ) != QgsMeshVertex() );
  QVERIFY( meshLayerQuadFlower->nativeMesh()->vertex( 9 ) != QgsMeshVertex() );
  QVERIFY( meshLayerQuadFlower->nativeMesh()->face( 4 ) != QgsMeshFace() );
  QVERIFY( meshLayerQuadFlower->nativeMesh()->face( 9 ) != QgsMeshFace() );

  //Same but with dragging and fill with hole (even there is no hole)
  tool.mouseMove( 1000, 3600 );
  tool.mousePress( 1000, 3600, Qt::LeftButton );
  tool.mouseMove( 2600, 3250 );
  tool.mouseRelease( 2600, 3250, Qt::LeftButton );
  tool.keyClick( Qt::Key_Delete, Qt::ControlModifier ); //remove without filling hole

  QCOMPARE( meshLayerQuadFlower->nativeMesh()->vertex( 7 ), QgsMeshVertex() );
  QCOMPARE( meshLayerQuadFlower->nativeMesh()->vertex( 9 ), QgsMeshVertex() );
  QCOMPARE( meshLayerQuadFlower->nativeMesh()->face( 4 ), QgsMeshFace() );
  QCOMPARE( meshLayerQuadFlower->nativeMesh()->face( 9 ), QgsMeshFace() );

  meshLayerQuadFlower->undoStack()->undo();

  QVERIFY( meshLayerQuadFlower->nativeMesh()->vertex( 7 ) != QgsMeshVertex() );
  QVERIFY( meshLayerQuadFlower->nativeMesh()->vertex( 9 ) != QgsMeshVertex() );
  QVERIFY( meshLayerQuadFlower->nativeMesh()->face( 4 ) != QgsMeshFace() );
  QVERIFY( meshLayerQuadFlower->nativeMesh()->face( 9 ) != QgsMeshFace() );

  //Change Z value selecting one vertex
  tool.mouseMove( 1000, 3000 );
  tool.mouseClick( 1000, 3000, Qt::LeftButton );
  editMeshMapTool->mZValueWidget->setZValue( 1600 );
  tool.keyClick( Qt::Key_Enter );

  QCOMPARE( meshLayerQuadFlower->nativeMesh()->vertex( 4 ).z(), 1600 );

  //Change Z value dragging rectangle
  tool.mouseMove( 1200, 3600 );
  tool.mousePress( 1200, 3600, Qt::LeftButton );
  tool.mouseMove( 2700, 2250 );
  tool.mouseRelease( 2700, 2250, Qt::LeftButton );
  editMeshMapTool->mZValueWidget->setZValue( 1500 );
  tool.keyClick( Qt::Key_Enter );

  QCOMPARE( meshLayerQuadFlower->datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( 2500, 3250 ) ).x(), 1500 );

  //Selection
  // from left to right
  tool.mouseMove( 1200, 3600 );
  tool.mousePress( 1200, 3600, Qt::LeftButton );
  tool.mouseMove( 2700, 2250 );
  tool.mouseRelease( 2700, 2250, Qt::LeftButton );
  QCOMPARE( editMeshMapTool->mSelectedVertices.count(), 5 );
  QCOMPARE( editMeshMapTool->mSelectedFaces.count(), 1 );

  // from left to right
  tool.mouseMove( 2700, 2250 );
  tool.mousePress( 2700, 2250, Qt::LeftButton );
  tool.mouseMove( 1200, 3600 );
  tool.mouseRelease( 1200, 3600, Qt::LeftButton );
  QCOMPARE( editMeshMapTool->mSelectedVertices.count(), 8 );
  QCOMPARE( editMeshMapTool->mSelectedFaces.count(), 7 );

  // add a face to the selection (click on centroid)
  tool.mouseMove( 800, 2500 );
  tool.mouseMove( 833, 2500 );
  tool.mouseClick( 833, 2500, Qt::LeftButton, Qt::ShiftModifier );
  QCOMPARE( editMeshMapTool->mSelectedVertices.count(), 9 );
  QCOMPARE( editMeshMapTool->mSelectedFaces.count(), 8 );

  // remove a vertex from the selection
  tool.mouseMove( 2500, 2400 );
  tool.mouseMove( 2500, 2500 );
  tool.mouseClick( 2500, 2500, Qt::LeftButton, Qt::ControlModifier );

  QCOMPARE( editMeshMapTool->mSelectedVertices.count(), 8 );
  QCOMPARE( editMeshMapTool->mSelectedFaces.count(), 6 );

  // move some vertices with invalid resulting faces
  QgsPointXY centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1500, 1600 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 1833.333 ), 1e-2 ) );
  // first, select vertices by using the center of the face (all vertices of face are selected)
  tool.mouseMove( 1501, 1833.33 );
  tool.mouseClick( 1501, 1833.33, Qt::LeftButton );
  tool.mousePress( 1500, 1833.33, Qt::LeftButton );
  tool.mouseMove( 2500, 3000 );
  tool.mouseRelease( 2500, 3000, Qt::LeftButton );
  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1500, 1600 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 1833.333 ), 1e-2 ) );

  // move some vertices
  QVERIFY( centroid.compare( QgsPointXY( 1500, 1833.333 ), 1e-2 ) );
  tool.mouseMove( 1501, 1833.33 );
  tool.mouseClick( 1501, 1833.33, Qt::LeftButton );
  tool.mousePress( 1502, 1833, Qt::LeftButton );
  tool.mouseMove( 1520, 1850 );
  tool.mouseRelease( 1520, 1850, Qt::LeftButton );
  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1500, 1600 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1520, 1850 ), 1e-2 ) );

  // select only one vertex
  tool.mouseMove( 1520, 1516.66 );
  tool.mouseClick( 1520,  1516.66, Qt::LeftButton );
  tool.mouseMove( 1521, 1516.66 );
  tool.mousePress( 1520, 1516.66, Qt::LeftButton );
  tool.mouseMove( 1500, 1500 );
  tool.mouseRelease( 1500, 1500, Qt::LeftButton );
  QgsPointXY vertexPosition = meshLayerQuadFlower->snapOnElement( QgsMesh::Vertex, QgsPointXY( 1520, 1480 ), 30 );
  QVERIFY( vertexPosition.compare( QgsPointXY( 1500, 1500 ), 1e-2 ) );

  // select an edge and move it
  tool.mouseMove( 1760, 1758 );
  tool.mouseClick( 1760,  1758, Qt::LeftButton );
  tool.mouseMove( 1760, 1758 );
  tool.mousePress( 1760, 1760, Qt::LeftButton );
  tool.mouseMove( 1800, 1760 );
  tool.mouseRelease( 1800, 1760, Qt::LeftButton );
  vertexPosition = meshLayerQuadFlower->snapOnElement( QgsMesh::Vertex, QgsPointXY( 1543, 1501 ), 10 );
  QVERIFY( vertexPosition.compare( QgsPointXY( 1540, 1501.666 ), 1e-2 ) );
  vertexPosition = meshLayerQuadFlower->snapOnElement( QgsMesh::Vertex, QgsPointXY( 2059, 2014 ), 10 );
  QVERIFY( vertexPosition.compare( QgsPointXY( 2060, 2018.3333 ), 1e-2 ) );

  // flip an edge
  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1100, 3050 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 3166.666 ), 1e-2 ) );
  tool.mouseMove( 1500, 3000 );
  tool.mouseMove( 1250, 3000 );
  tool.mouseClick( 1250, 3000, Qt::LeftButton );
  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1100, 3050 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1333.33, 3100 ), 1e-2 ) );
  // merge faces on flipped edge
  tool.mouseMove( 1500, 2900 );
  tool.mouseMove( 1500, 3325 );
  tool.mouseClick( 1500, 3324, Qt::LeftButton );
  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1100, 3050 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 3100 ), 1e-2 ) );
}



QGSTEST_MAIN( TestQgsMapToolEditMesh )
#include "testqgsmaptooleditmesh.moc"
