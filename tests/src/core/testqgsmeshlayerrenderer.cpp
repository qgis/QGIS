/***************************************************************************
                         testqgsmeshlayer.cpp
                         --------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QLabel>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

//qgis includes...
#include "qgsmaplayer.h"
#include "qgsmeshlayer.h"
#include "qgsapplication.h"
#include "qgsmaplayerlegend.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmeshmemorydataprovider.h"

//qgis test includes
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the different renderers for mesh layers.
 */
class TestQgsMeshRenderer : public QObject
{
    Q_OBJECT

  public:
    TestQgsMeshRenderer() = default;

  private:
    QString mDataDir;
    QgsMeshLayer *mMemoryLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;
    QString mReport;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    bool imageCheck( const QString &testType );
    QString readFile( const QString &fname ) const;


    void test_native_mesh_rendering();
    void test_triangular_mesh_rendering();

    void test_vertex_scalar_dataset_rendering();
    void test_vertex_vector_dataset_rendering();
    void test_face_scalar_dataset_rendering();
    void test_face_vector_dataset_rendering();

    void test_signals();
};

void TestQgsMeshRenderer::init()
{
  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset();
  rendererSettings.setActiveVectorDataset();
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  mMemoryLayer->setRendererSettings( rendererSettings );
}

void TestQgsMeshRenderer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/mesh";

  mReport = QStringLiteral( "<h1>Mesh Layer Rendering Tests</h1>\n" );

  mMapSettings = new QgsMapSettings();

  // Memory layer
  mMemoryLayer = new QgsMeshLayer( readFile( "/quad_and_triangle.txt" ), "Triangle and Quad Memory", "mesh_memory" );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_vector.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_vector.txt" ) );
  QVERIFY( mMemoryLayer->isValid() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemoryLayer );
  mMapSettings->setLayers(
    QList<QgsMapLayer *>() << mMemoryLayer );

  // here we check that datasets automatically get some default style applied
  QgsMeshDatasetIndex ds( 0, 0 );
  QgsMeshRendererScalarSettings scalarSettings = mMemoryLayer->rendererSettings().scalarSettings( ds.group() );
  QgsColorRampShader shader = scalarSettings.colorRampShader();
  QList<QgsColorRampShader::ColorRampItem> lst = shader.colorRampItemList();
  QCOMPARE( lst.count(), 5 );
  QCOMPARE( lst.at( 0 ).value, 1. );  // min group value
  QCOMPARE( lst.at( 4 ).value, 4. );  // max group value
}

void TestQgsMeshRenderer::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

QString TestQgsMeshRenderer::readFile( const QString &fname ) const
{
  QString uri;
  QFile f( mDataDir + fname );
  if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    uri = f.readAll();
  return uri;
}

bool TestQgsMeshRenderer::imageCheck( const QString &testType )
{
  mReport += "<h2>" + testType + "</h2>\n";
  mMapSettings->setExtent( mMemoryLayer->extent() );
  mMapSettings->setDestinationCrs( mMemoryLayer->crs() );
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "mesh" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 15 );
  bool myResultFlag = myChecker.runTest( testType, 0 );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsMeshRenderer::test_native_mesh_rendering()
{
  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererMeshSettings settings = rendererSettings.nativeMeshSettings();
  settings.setEnabled( true );
  settings.setLineWidth( 1. );
  rendererSettings.setNativeMeshSettings( settings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  QVERIFY( imageCheck( "quad_and_triangle_native_mesh" ) );
}

void TestQgsMeshRenderer::test_triangular_mesh_rendering()
{
  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererMeshSettings settings = rendererSettings.triangularMeshSettings();
  settings.setEnabled( true );
  settings.setColor( Qt::red );
  settings.setLineWidth( 0.26 );
  rendererSettings.setTriangularMeshSettings( settings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  QVERIFY( imageCheck( "quad_and_triangle_triangular_mesh" ) );
}

void TestQgsMeshRenderer::test_vertex_scalar_dataset_rendering()
{
  QgsMeshDatasetIndex ds( 0, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexScalarDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_scalar_dataset" ) );
}

void TestQgsMeshRenderer::test_vertex_vector_dataset_rendering()
{
  QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setMinShaftLength( 15 );
  rendererSettings.setVectorSettings( ds.group(), settings );
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_dataset" ) );
}

void TestQgsMeshRenderer::test_face_scalar_dataset_rendering()
{
  QgsMeshDatasetIndex ds( 2, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceScalarDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_face_scalar_dataset" ) );
}

void TestQgsMeshRenderer::test_face_vector_dataset_rendering()
{
  QgsMeshDatasetIndex ds( 3, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_face_vector_dataset" ) );
}

void TestQgsMeshRenderer::test_signals()
{
  QSignalSpy spy1( mMemoryLayer, &QgsMapLayer::rendererChanged );
  QSignalSpy spy2( mMemoryLayer->legend(), &QgsMapLayerLegend::itemsChanged );
  QSignalSpy spy3( mMemoryLayer, &QgsMapLayer::legendChanged );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset( QgsMeshDatasetIndex( 1, 0 ) );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QCOMPARE( spy1.count(), 1 );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( spy3.count(), 1 );
}

QGSTEST_MAIN( TestQgsMeshRenderer )
#include "testqgsmeshlayerrenderer.moc"
