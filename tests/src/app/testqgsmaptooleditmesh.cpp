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

#include <QDir>

#include "qgstest.h"
#include "qgisapp.h"
#include "testqgsmaptoolutils.h"
#include "qgsmaptooleditmeshframe.h"
#include "qgsmeshlayer.h"
#include "qgsmesheditor.h"
#include "qgsrasterlayer.h"
#include "qgsprojectelevationproperties.h"
#include "qgsterrainprovider.h"
#include "qgsmeshtransformcoordinatesdockwidget.h"

class TestQgsMapToolEditMesh : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsMapToolEditMesh()
      : QgsTest( QStringLiteral( "Map Tool Edit Mesh Tests" ), QStringLiteral( "app" ) )
    {}

  private slots:
    void initTestCase();       // will be called before the first testfunction is executed.
    void cleanupTestCase() {}; // will be called after the last testfunction was executed.
    void init();               // will be called before each testfunction is executed.
    void cleanup() {}          // will be called after every testfunction.

    void hoverElements();

    void editMesh();

    void selectElements();
    void testAssignVertexZValueFromTerrainOnCreation();
    void testAssignVertexZValueFromTerrainOnButtonClick();
    void testDelaunayRefinement();

  private:
    static QString read2DMFileContent( const QString &filePath );

    QgisApp *mQgisApp = nullptr;
    std::unique_ptr<QgsMeshLayer> meshLayerQuadFlower;
    QString mDataDir;
    std::unique_ptr<QgsMapCanvas> mCanvas;
    QgsMapToolEditMeshFrame *mEditMeshMapTool;

    std::unique_ptr<QgsMeshLayer> meshLayerSimpleBox;
};


//runs before all tests
void TestQgsMapToolEditMesh::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/mesh";
}

void TestQgsMapToolEditMesh::init()
{
  mCanvas.reset( new QgsMapCanvas() );
  mEditMeshMapTool = new QgsMapToolEditMeshFrame( mCanvas.get() );
}

void TestQgsMapToolEditMesh::hoverElements()
{
  QString uri = QString( mDataDir + "/simplebox_clm.nc" );
  meshLayerSimpleBox.reset( new QgsMeshLayer( uri, "Simple box", "mdal" ) );
  QVERIFY( meshLayerSimpleBox->isValid() );

  mCanvas->setLayers( QList<QgsMapLayer *>() << meshLayerSimpleBox.get() );

  QgsCoordinateReferenceSystem wgs84Crs;
  wgs84Crs.createFromProj( "+proj=longlat +datum=WGS84 +no_defs" );
  QVERIFY( wgs84Crs.isValid() );
  QVERIFY( meshLayerSimpleBox->crs().isValid() );
  mCanvas->setDestinationCrs( wgs84Crs );

  QgsCoordinateTransform transform( meshLayerSimpleBox->crs(), mCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
  QgsMeshEditingError error;
  QVERIFY( meshLayerSimpleBox->startFrameEditing( transform, error, false ) );
  QVERIFY( error.errorType == Qgis::MeshEditingErrorType::NoError );

  QgsRectangle extent( 3.31351393, 47.97489613, 3.31351792, 47.97508220 );
  mCanvas->setExtent( extent );
  TestQgsMapToolAdvancedDigitizingUtils tool( mEditMeshMapTool );
  mCanvas->setCurrentLayer( meshLayerSimpleBox.get() );
  mEditMeshMapTool->mActionDigitizing->trigger();
  tool.mouseMove( 3.31376427, 47.97500487 );
  QCOMPARE( mEditMeshMapTool->mCurrentFaceIndex, 8 );
  QVERIFY( mEditMeshMapTool->mCurrentEdge == QgsMapToolEditMeshFrame::Edge( { -1, -1 } ) );
  QCOMPARE( mEditMeshMapTool->mCurrentVertexIndex, -1 );

  tool.mouseMove( 3.31368247, 47.97500500 );
  QCOMPARE( mEditMeshMapTool->mCurrentFaceIndex, 8 );
  QVERIFY( mEditMeshMapTool->mCurrentEdge == QgsMapToolEditMeshFrame::Edge( { 8, 5 } ) );
  QCOMPARE( mEditMeshMapTool->mCurrentVertexIndex, -1 );

  tool.mouseMove( 3.31368064, 47.97503705 );
  QCOMPARE( mEditMeshMapTool->mCurrentFaceIndex, 8 );
  QVERIFY( mEditMeshMapTool->mCurrentEdge == QgsMapToolEditMeshFrame::Edge( { 8, 5 } ) );
  QCOMPARE( mEditMeshMapTool->mCurrentVertexIndex, 10 );
}

void TestQgsMapToolEditMesh::editMesh()
{
  QString uri = QString( mDataDir + "/quad_flower.2dm" );
  meshLayerQuadFlower.reset( new QgsMeshLayer( uri, "Quad Flower", "mdal" ) );
  QVERIFY( meshLayerQuadFlower->isValid() );
  QCOMPARE( meshLayerQuadFlower->datasetGroupCount(), 1 );

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  meshLayerQuadFlower->startFrameEditing( transform, error, false );
  QVERIFY( error == QgsMeshEditingError() );

  mCanvas->setLayers( QList<QgsMapLayer *>() << meshLayerQuadFlower.get() );
  const double offsetInMapUnits = 15 * mCanvas->mapSettings().mapUnitsPerPixel();

  QVERIFY( meshLayerQuadFlower->meshEditor() );

  TestQgsMapToolAdvancedDigitizingUtils tool( mEditMeshMapTool );
  mCanvas->setCurrentLayer( meshLayerQuadFlower.get() );
  mEditMeshMapTool->mActionDigitizing->trigger();

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 8 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  tool.mouseDoubleClick( 1500, 2800, Qt::LeftButton ); //add a vertex on the face 0
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 8 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  meshLayerQuadFlower->undoStack()->undo();

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 5 );

  meshLayerQuadFlower->rollBackFrameEditing( transform, false );

  QVERIFY( !meshLayerQuadFlower->meshEditor() );

  meshLayerQuadFlower->startFrameEditing( transform, error, false );
  QVERIFY( error == QgsMeshEditingError() );

  QVERIFY( meshLayerQuadFlower->meshEditor() );

  // redo
  tool.mouseDoubleClick( 1500, 2800, Qt::LeftButton );
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 8 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  // attempt to add a vertex on place of existing one
  tool.mouseDoubleClick( 2500, 2500, Qt::LeftButton );
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 8 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 0 );

  // add a free vertex
  tool.mouseDoubleClick( 2500, 3500, Qt::LeftButton ); // 9
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 8 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 10 );
  QCOMPARE( meshLayerQuadFlower->meshEditor()->freeVerticesIndexes().count(), 1 );

  // add a face
  tool.mouseMove( 1999, 2999 );                                                               //move near a vertex
  tool.mouseMove( 2000 + offsetInMapUnits / sqrt( 2 ), 3000 + offsetInMapUnits / sqrt( 2 ) ); //move on the new face marker
  tool.mouseClick( 2000 + offsetInMapUnits / sqrt( 4 ), 3000 + offsetInMapUnits / sqrt( 2 ), Qt::LeftButton );
  tool.mouseMove( 2499, 3501 );                   //move near the new free vertex
  tool.mouseClick( 2501, 3499, Qt::LeftButton );  // click near the vertex
  tool.mouseMove( 2490, 2600 );                   //move elsewhere
  tool.mouseMove( 2495, 2500 );                   //move near another vertex
  tool.mouseClick( 2495, 2500, Qt::LeftButton );  // click near the vertex
  tool.mouseClick( 5000, 5000, Qt::RightButton ); // valid the face

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );

  // add a face with new vertices
  tool.mouseMove( 2500, 3500 ); //move near a vertex
  tool.mouseMove( 2500, 3501 );
  tool.mouseMove( 2500 + offsetInMapUnits / sqrt( 5 ), 3500 + 2 * offsetInMapUnits / sqrt( 5 ) ); //move on the new face marker
  tool.mouseClick( 2500 + offsetInMapUnits / sqrt( 5 ), 3500 + 2 * offsetInMapUnits / sqrt( 5 ), Qt::LeftButton );
  tool.mouseMove( 3000, 3000 ); //move to a new place outsite the mesh
  tool.mouseClick( 3000, 3000, Qt::LeftButton );
  tool.mouseMove( 3000, 3500 ); //move to a new place outsite the mesh (cross edge of new face: invalid)
  tool.mouseClick( 3000, 3500, Qt::LeftButton );
  tool.mouseMove( 2500, 2500 );
  tool.mouseClick( 2500, 2500, Qt::LeftButton );          // close the face
  tool.mouseClick( 5000, 5000, Qt::RightButton );         // valid the fac
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );    //-> face not added
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 10 ); //-> vertices not added
  tool.keyClick( Qt::Key_Backspace );
  tool.keyClick( Qt::Key_Backspace );
  tool.keyClick( Qt::Key_Backspace );
  tool.mouseMove( 3000, 3500 );
  tool.mouseClick( 3000, 3000, Qt::LeftButton );
  tool.mouseMove( 2500, 2500 );
  tool.mouseClick( 2500, 2500, Qt::LeftButton );
  tool.mouseMove( 5000, 5000 );
  tool.mouseClick( 5000, 5000, Qt::RightButton );
  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 10 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 11 );

  meshLayerQuadFlower->undoStack()->undo();

  QCOMPARE( meshLayerQuadFlower->meshFaceCount(), 9 );
  QCOMPARE( meshLayerQuadFlower->meshVertexCount(), 10 );

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
  mEditMeshMapTool->mZValueWidget->setZValue( 1600 );
  tool.keyClick( Qt::Key_Enter );

  QCOMPARE( meshLayerQuadFlower->nativeMesh()->vertex( 4 ).z(), 1600 );

  //Change Z value dragging rectangle
  tool.mouseMove( 1200, 3600 );
  tool.mousePress( 1200, 3600, Qt::LeftButton );
  tool.mouseMove( 2700, 2250 );
  tool.mouseRelease( 2700, 2250, Qt::LeftButton );
  mEditMeshMapTool->mZValueWidget->setZValue( 1500 );
  tool.keyClick( Qt::Key_Enter );

  QCOMPARE( meshLayerQuadFlower->datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( 2500, 3250 ) ).x(), 1500 );

  //Selection
  // completely included
  tool.mouseMove( 1200, 3600 );
  tool.mousePress( 1200, 3600, Qt::LeftButton );
  tool.mouseMove( 2700, 2250 );
  tool.mouseRelease( 2700, 2250, Qt::LeftButton, Qt::AltModifier );
  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 5 );
  QCOMPARE( mEditMeshMapTool->mSelectedFaces.count(), 1 );

  // touched
  tool.mouseMove( 2700, 2250 );
  tool.mousePress( 2700, 2250, Qt::LeftButton );
  tool.mouseMove( 1200, 3600 );
  tool.mouseRelease( 1200, 3600, Qt::LeftButton );
  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 8 );
  QCOMPARE( mEditMeshMapTool->mSelectedFaces.count(), 7 );

  // add a face to the selection (click on centroid)
  tool.mouseMove( 800, 2500 );
  tool.mouseMove( 833, 2500 );
  tool.mouseClick( 833, 2500, Qt::LeftButton, Qt::ShiftModifier );
  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 9 );
  QCOMPARE( mEditMeshMapTool->mSelectedFaces.count(), 8 );

  // remove a vertex from the selection
  tool.mouseMove( 2500, 2400 );
  tool.mouseMove( 2500, 2500 );
  tool.mouseClick( 2500, 2500, Qt::LeftButton, Qt::ControlModifier );

  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 8 );
  QCOMPARE( mEditMeshMapTool->mSelectedFaces.count(), 6 );

  // move some vertices with invalid resulting faces
  QgsPointXY centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1500, 1600 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 1833.333 ), 1e-2 ) );
  // first, select vertices by using the center of the face (all vertices of face are selected)
  tool.mouseMove( 1501, 1833.33 );
  tool.mouseClick( 1501, 1833.33, Qt::LeftButton ); //select a face
  tool.mouseClick( 1500, 1833.33, Qt::LeftButton ); //start move
  tool.mouseMove( 2500, 3000 );
  tool.mouseClick( 2500, 3000, Qt::LeftButton ); //end move
  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1500, 1600 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1500, 1833.333 ), 1e-2 ) );

  // move some vertices
  QVERIFY( centroid.compare( QgsPointXY( 1500, 1833.333 ), 1e-2 ) );
  tool.mouseMove( 1501, 1833.33 );
  tool.mouseClick( 1502, 1833, Qt::LeftButton ); //start move (already selected)
  tool.mouseMove( 1520, 1850 );
  tool.mouseClick( 1520, 1850, Qt::LeftButton ); //end move
  centroid = meshLayerQuadFlower->snapOnElement( QgsMesh::Face, QgsPointXY( 1500, 1600 ), 10 );
  QVERIFY( centroid.compare( QgsPointXY( 1520, 1850 ), 1e-2 ) );

  // select only one vertex
  tool.mouseMove( 1520, 1516.66 );
  tool.mouseClick( 1520, 1516.66, Qt::LeftButton ); //select vertex
  tool.mouseMove( 1521, 1516.66 );
  tool.mouseClick( 1520, 1516.66, Qt::LeftButton ); //start move
  tool.mouseMove( 1500, 1500 );
  tool.mouseClick( 1500, 1500, Qt::LeftButton ); //end move
  QgsPointXY vertexPosition = meshLayerQuadFlower->snapOnElement( QgsMesh::Vertex, QgsPointXY( 1520, 1480 ), 30 );
  QVERIFY( vertexPosition.compare( QgsPointXY( 1500, 1500 ), 1e-2 ) );

  // select an edge and move it
  tool.mouseMove( 1760, 1758 );
  tool.mouseClick( 1760, 1758, Qt::LeftButton );
  tool.mouseMove( 1760, 1758 );
  tool.mouseClick( 1760, 1760, Qt::LeftButton );
  tool.mouseMove( 1800, 1760 );
  tool.mouseClick( 1800, 1760, Qt::LeftButton );
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

  tool.keyClick( Qt::Key_Escape );

  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 0 );
  QCOMPARE( mEditMeshMapTool->mSelectedFaces.count(), 0 );

  // Selection by polygon
  mEditMeshMapTool->mActionSelectByPolygon->trigger();

  // touched
  tool.mouseClick( 3500, 3250, Qt::LeftButton );
  tool.mouseClick( 2750, 3250, Qt::LeftButton );
  tool.mouseClick( 1750, 2500, Qt::LeftButton );
  tool.mouseClick( 2500, 2000, Qt::LeftButton );
  tool.mouseClick( 3000, 2000, Qt::LeftButton );
  tool.mouseClick( 3000, 2000, Qt::RightButton );

  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 5 );
  QCOMPARE( mEditMeshMapTool->mSelectedFaces.count(), 3 );

  // completely included
  tool.mouseClick( 2750, 3250, Qt::LeftButton );
  tool.mouseClick( 3500, 3250, Qt::LeftButton );
  tool.mouseClick( 3000, 2000, Qt::LeftButton );
  tool.mouseClick( 2500, 2000, Qt::LeftButton );
  tool.mouseClick( 1750, 2500, Qt::LeftButton );
  tool.mouseClick( 1750, 2500, Qt::RightButton, Qt::AltModifier );

  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 1 );
  QCOMPARE( mEditMeshMapTool->mSelectedFaces.count(), 0 );
}

void TestQgsMapToolEditMesh::testAssignVertexZValueFromTerrainOnCreation()
{
  QgsCoordinateReferenceSystem crs3857;
  crs3857.createFromString( "EPSG:3857" );

  QString uri = QString( mDataDir + "/quad_and_triangle_with_free_vertices.2dm" );
  std::unique_ptr<QgsMeshLayer> layer = std::make_unique<QgsMeshLayer>( uri, "quad and triangle", "mdal" );
  layer->setCrs( crs3857 );
  QVERIFY( layer->isValid() );

  QString rasterUri = QString( mDataDir + "/terrain_under_mesh.tif" );
  std::unique_ptr<QgsRasterLayer> terrainLayer = std::make_unique<QgsRasterLayer>( rasterUri, "terrain", "gdal" );
  terrainLayer->setCrs( crs3857 );
  QVERIFY( terrainLayer->isValid() );

  std::unique_ptr<QgsRasterDemTerrainProvider> terrain = std::make_unique<QgsRasterDemTerrainProvider>();
  terrain->setLayer( terrainLayer.get() );

  QgsProject::instance()->elevationProperties()->setTerrainProvider( terrain.release() );
  mCanvas->setLayers( QList<QgsMapLayer *>() << layer.get() << terrainLayer.get() );
  mCanvas->setDestinationCrs( layer->crs() );

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  layer->startFrameEditing( transform, error, false );
  QVERIFY( error == QgsMeshEditingError() );
  QVERIFY( layer->meshEditor() );

  TestQgsMapToolAdvancedDigitizingUtils tool( mEditMeshMapTool );
  mCanvas->setCurrentLayer( layer.get() );
  mEditMeshMapTool->mActionDigitizing->trigger();

  // setup Z value widget
  double defaultZ = -10.0;
  mEditMeshMapTool->mZValueWidget->setDefaultValue( defaultZ );

  QgsPointXY point;
  QgsMeshVertex vertex;

  // default settings in mesh interpolate mesh otherwise default value of z widget
  mEditMeshMapTool->setZValueSourceType( QgsMeshEditDigitizingAction::ZValueSource::PreferMeshThenZWidget );

  // test points outside of faces, should get defaultZ Z value
  point = QgsPointXY( 1100, 3100 );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 5 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 6 );

  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QCOMPARE( vertex.z(), defaultZ );

  point = QgsPointXY( 2500, 2700 );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 6 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 7 );

  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QCOMPARE( vertex.z(), defaultZ );

  // points inside faces are not affected - still interpolated from the mesh values
  point = QgsPointXY( 1700, 2200 );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 2 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 5 );

  tool.mouseMove( point.x() - 1, point.y() - 1 );
  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), 31, 0.000001 );

  // remove edits
  layer->rollBackFrameEditing( transform, false );

  // start editing again
  layer->startFrameEditing( transform, error, false );
  mEditMeshMapTool->mZValueWidget->setDefaultValue( defaultZ );

  // set get Z from project elevation to true - Z values will always be obtained from elevation provider
  mEditMeshMapTool->setZValueSourceType( QgsMeshEditDigitizingAction::ZValueSource::Terrain );

  // test points outside of faces
  point = QgsPointXY( 1100, 3100 );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 5 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 6 );

  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), 66.00578, 0.00001 );

  // points inside faces
  point = QgsPointXY( 1700, 2200 );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 2 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 5 );

  tool.mouseMove( point.x() - 1, point.y() - 1 );
  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), 66.091468, 0.000001 );

  // remove edits
  layer->rollBackFrameEditing( transform, false );

  // start editing again
  layer->startFrameEditing( transform, error, false );
  mEditMeshMapTool->mZValueWidget->setDefaultValue( defaultZ );

  // set get Z from project elevation to true - Z values will be obtained from elevation provider outside of mesh
  mEditMeshMapTool->setZValueSourceType( QgsMeshEditDigitizingAction::ZValueSource::PreferMeshThenTerrain );

  // test points outside of faces
  point = QgsPointXY( 1100, 3100 );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 5 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 6 );

  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), 66.00578, 0.00001 );

  point = QgsPointXY( 2500, 2700 );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 6 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 7 );

  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), 4.100819, 0.000001 );

  // points inside faces are not affected - still interpolated from the mesh values
  point = QgsPointXY( 1700, 2200 );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 2 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 5 );

  tool.mouseMove( point.x() - 1, point.y() - 1 );
  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), 31, 0.000001 );

  // test points outside of terrain provider - should get default Z value from the widget
  point = QgsPointXY( 3000, 4000 );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 7 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 8 );

  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), defaultZ, 0.0000001 );

  // remove edits
  layer->rollBackFrameEditing( transform, false );

  // start editing again
  layer->startFrameEditing( transform, error, false );
  mEditMeshMapTool->mZValueWidget->setDefaultValue( defaultZ );

  // set get Z from project elevation to false - Z values will be obtained Z widget outside of mesh
  mEditMeshMapTool->setZValueSourceType( QgsMeshEditDigitizingAction::ZValueSource::PreferMeshThenZWidget );

  // test points outside of faces
  point = QgsPointXY( 2700, 1800 );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 5 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 6 );

  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), defaultZ, 0.0000001 );

  // points inside faces are not affected - still interpolated from the mesh values
  point = QgsPointXY( 1700, 2200 );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 2 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 5 );

  tool.mouseMove( point.x() - 1, point.y() - 1 );
  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), 31, 0.000001 );

  // remove edits
  layer->rollBackFrameEditing( transform, false );

  // start editing again
  layer->startFrameEditing( transform, error, false );
  mEditMeshMapTool->mZValueWidget->setDefaultValue( defaultZ );

  // set get Z from project elevation to false - Z values will be obtained Z widget
  mEditMeshMapTool->setZValueSourceType( QgsMeshEditDigitizingAction::ZValueSource::ZWidget );

  // point inside faces
  point = QgsPointXY( 1700, 2200 );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 2 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 5 );

  tool.mouseMove( point.x() - 1, point.y() - 1 );
  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), defaultZ, 0.000001 );

  // test points outside of faces
  point = QgsPointXY( 2700, 1800 );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 5 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->freeVerticesIndexes().count(), 6 );

  vertex = mEditMeshMapTool->mapVertex( mEditMeshMapTool->closeVertex( point ) );
  QGSCOMPARENEAR( vertex.z(), defaultZ, 0.0000001 );

  // remove edits
  layer->rollBackFrameEditing( transform, false );
}

void TestQgsMapToolEditMesh::testAssignVertexZValueFromTerrainOnButtonClick()
{
  QgsCoordinateReferenceSystem crs3857;
  crs3857.createFromString( "EPSG:3857" );

  QString uri = QString( mDataDir + "/quad_and_triangle_with_free_vertices.2dm" );
  std::unique_ptr<QgsMeshLayer> layer = std::make_unique<QgsMeshLayer>( uri, "quad and triangle", "mdal" );
  layer->setCrs( crs3857 );
  QVERIFY( layer->isValid() );

  QString rasterUri = QString( mDataDir + "/terrain_under_mesh.tif" );
  std::unique_ptr<QgsRasterLayer> terrainLayer = std::make_unique<QgsRasterLayer>( rasterUri, "terrain", "gdal" );
  terrainLayer->setCrs( crs3857 );
  QVERIFY( terrainLayer->isValid() );

  std::unique_ptr<QgsRasterDemTerrainProvider> terrain = std::make_unique<QgsRasterDemTerrainProvider>();
  terrain->setLayer( terrainLayer.get() );

  QgsProject::instance()->elevationProperties()->setTerrainProvider( terrain.release() );
  mCanvas->setLayers( QList<QgsMapLayer *>() << layer.get() << terrainLayer.get() );
  mCanvas->setDestinationCrs( layer->crs() );

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  layer->startFrameEditing( transform, error, false );
  QVERIFY( error == QgsMeshEditingError() );
  QVERIFY( layer->meshEditor() );

  TestQgsMapToolAdvancedDigitizingUtils tool( mEditMeshMapTool );
  mCanvas->setCurrentLayer( layer.get() );
  mEditMeshMapTool->mActionDigitizing->trigger();

  QList<int> selectedVertices;
  selectedVertices << 1 << 2 << 3;

  QgsPoint vertex;

  // test vertices prior to assignment from elevation provider
  vertex = mEditMeshMapTool->mapVertex( 1 );
  QGSCOMPARENEAR( vertex.z(), 30, 0.01 );

  vertex = mEditMeshMapTool->mapVertex( 2 );
  QGSCOMPARENEAR( vertex.z(), 40, 0.01 );

  vertex = mEditMeshMapTool->mapVertex( 3 );
  QGSCOMPARENEAR( vertex.z(), 50, 0.01 );

  mEditMeshMapTool->triggerTransformCoordinatesDockWidget( true );

  // set selected vertices and press the button
  mEditMeshMapTool->setSelectedVertices( selectedVertices, Qgis::SelectBehavior::SetSelection );
  mEditMeshMapTool->mTransformDockWidget->updateZValuesFromTerrain();

  // test vertices after the assignment from elevation provider
  vertex = mEditMeshMapTool->mapVertex( 1 );
  QGSCOMPARENEAR( vertex.z(), 18.244469, 0.000001 );

  vertex = mEditMeshMapTool->mapVertex( 2 );
  QGSCOMPARENEAR( vertex.z(), 14.353244, 0.000001 );

  vertex = mEditMeshMapTool->mapVertex( 3 );
  QGSCOMPARENEAR( vertex.z(), 54.627747, 0.000001 );

  // remove edits
  layer->rollBackFrameEditing( transform, false );
}

void TestQgsMapToolEditMesh::selectElements()
{
  QString uri = QString( mDataDir + "/quad_and_triangle_with_free_vertices.2dm" );
  std::unique_ptr<QgsMeshLayer> layer = std::make_unique<QgsMeshLayer>( uri, "quad and triangle", "mdal" );
  QVERIFY( layer->isValid() );

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  layer->startFrameEditing( transform, error, false );
  QVERIFY( error == QgsMeshEditingError() );

  mCanvas->setLayers( QList<QgsMapLayer *>() << layer.get() );

  QVERIFY( layer->meshEditor() );

  TestQgsMapToolAdvancedDigitizingUtils tool( mEditMeshMapTool );
  mCanvas->setCurrentLayer( layer.get() );
  mEditMeshMapTool->mActionDigitizing->trigger();

  // select all vertices
  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 0 );
  mEditMeshMapTool->mActionSelectAllVertices->trigger();
  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 10 );

  // reset selection
  tool.mouseClick( 0, 0, Qt::LeftButton );

  // select isolated vertices
  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 0 );
  mEditMeshMapTool->mActionSelectIsolatedVertices->trigger();
  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 5 );

  // reset selection
  tool.mouseClick( 0, 0, Qt::LeftButton );

  // select by polygon
  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 0 );
  mEditMeshMapTool->mActionSelectByPolygon->trigger();

  // polygon definition
  tool.mouseClick( 2100, 3000, Qt::LeftButton );
  tool.mouseClick( 2900, 2300, Qt::LeftButton );
  tool.mouseClick( 3100, 3000, Qt::LeftButton );
  tool.mouseClick( 2500, 3000, Qt::RightButton );

  QCOMPARE( mEditMeshMapTool->mSelectedVertices.count(), 3 );
}

void TestQgsMapToolEditMesh::testDelaunayRefinement()
{
  QgsCoordinateReferenceSystem crs3857;
  crs3857.createFromString( "EPSG:3857" );

  const QgsCoordinateTransform transform;
  QgsMeshEditingError error;
  QgsPointXY point;

  QString originalDataPath = QString( "/mesh/not_delaunay.2dm" );

  // editing with normal setting - without delaunay refinement
  mEditMeshMapTool->mWidgetActionDigitizing->mCheckBoxRefineNeighboringFaces->setChecked( false );

  const QString copyDataPath1 = copyTestData( originalDataPath ); // copy of data to be edited

  std::unique_ptr<QgsMeshLayer> layer = std::make_unique<QgsMeshLayer>( copyDataPath1, "not delaunay", "mdal" );
  layer->setCrs( crs3857 );
  QVERIFY( layer->isValid() );

  layer->startFrameEditing( transform, error, false );
  QVERIFY( error == QgsMeshEditingError() );

  mCanvas->setLayers( QList<QgsMapLayer *>() << layer.get() );
  mCanvas->setDestinationCrs( crs3857 );
  mCanvas->setExtent( layer->extent() );

  QVERIFY( layer->meshEditor() );

  TestQgsMapToolAdvancedDigitizingUtils tool( mEditMeshMapTool );
  mCanvas->setCurrentLayer( layer.get() );
  mEditMeshMapTool->mActionDigitizing->trigger();

  point = QgsPointXY( 4.5, 3.5 );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 8 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 10 );
  QCOMPARE( layer->undoStack()->command( 0 )->text(), "Add 1 vertices" );

  QVERIFY( layer->commitFrameEditing( transform, false ) );

  QGSCOMPARELONGSTR( "edit_no_delaunay_refinement", "not_delaunay.2dm", TestQgsMapToolEditMesh::read2DMFileContent( copyDataPath1 ).toUtf8() );

  // editing with delaunay refinement
  mEditMeshMapTool->mWidgetActionDigitizing->mCheckBoxRefineNeighboringFaces->setChecked( true );

  const QString copyDataPath2 = copyTestData( originalDataPath ); // copy of data to be edited

  layer = std::make_unique<QgsMeshLayer>( copyDataPath2, "not delaunay", "mdal" );
  layer->setCrs( crs3857 );
  QVERIFY( layer->isValid() );

  layer->startFrameEditing( transform, error, false );
  QVERIFY( error == QgsMeshEditingError() );

  mCanvas->setLayers( QList<QgsMapLayer *>() << layer.get() );
  mCanvas->setDestinationCrs( crs3857 );
  mCanvas->setExtent( layer->extent() );

  QVERIFY( layer->meshEditor() );

  mCanvas->setCurrentLayer( layer.get() );
  mEditMeshMapTool->mActionDigitizing->trigger();

  point = QgsPointXY( 4.5, 3.5 );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 8 );
  tool.mouseMove( point.x(), point.y() );
  tool.mouseDoubleClick( point.x(), point.y(), Qt::LeftButton );
  QCOMPARE( layer->meshEditor()->validFacesCount(), 10 );
  QCOMPARE( layer->undoStack()->command( 0 )->text(), "Add vertex inside face with Delaunay refinement" );

  QVERIFY( layer->commitFrameEditing( transform, false ) );

  QGSCOMPARELONGSTR( "edit_delaunay_refinement", "delaunay.2dm", TestQgsMapToolEditMesh::read2DMFileContent( copyDataPath2 ).toUtf8() );
}

QString TestQgsMapToolEditMesh::read2DMFileContent( const QString &filePath )
{
  QFile file( filePath );
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    return QString(); // Return empty string if file can't be opened
  }

  QTextStream in( &file );

  QString content = in.readAll();
  file.close();
  return content;
}

QGSTEST_MAIN( TestQgsMapToolEditMesh )
#include "testqgsmaptooleditmesh.moc"
