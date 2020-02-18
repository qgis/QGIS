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
#include "qgsmesh3daveraging.h"

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
    QgsMeshLayer *mMdalLayer = nullptr;
    QgsMeshLayer *mMdal3DLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;
    QString mReport;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    bool imageCheck( const QString &testType, QgsMeshLayer *layer );
    QString readFile( const QString &fname ) const;


    void test_native_mesh_rendering();
    void test_triangular_mesh_rendering();

    void test_vertex_scalar_dataset_rendering();
    void test_vertex_vector_dataset_rendering();
    void test_face_scalar_dataset_rendering();
    void test_face_scalar_dataset_interpolated_neighbour_average_rendering();
    void test_face_vector_dataset_rendering();
    void test_vertex_scalar_dataset_with_inactive_face_rendering();
    void test_face_vector_on_user_grid();
    void test_face_vector_on_user_grid_streamlines();
    void test_vertex_vector_on_user_grid();
    void test_vertex_vector_on_user_grid_streamlines();
    void test_vertex_vector_traces();
    void test_stacked_3d_mesh_single_level_averaging();
    void test_simplified_triangular_mesh_rendering();

    void test_signals();
};

void TestQgsMeshRenderer::init()
{
  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset();
  rendererSettings.setActiveVectorDataset();
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setAveragingMethod( nullptr );
  mMemoryLayer->setRendererSettings( rendererSettings );

  rendererSettings = mMdalLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset();
  rendererSettings.setActiveVectorDataset();
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setAveragingMethod( nullptr );
  mMdalLayer->setRendererSettings( rendererSettings );

  rendererSettings = mMdal3DLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset();
  rendererSettings.setActiveVectorDataset();
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setAveragingMethod( nullptr );
  mMdal3DLayer->setRendererSettings( rendererSettings );
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

  // Mdal layer
  mMdalLayer = new QgsMeshLayer( mDataDir + "/quad_and_triangle.2dm", "Triangle and Quad Mdal", "mdal" );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QVERIFY( mMdalLayer->isValid() );

  // Memory layer
  mMemoryLayer = new QgsMeshLayer( readFile( "/quad_and_triangle.txt" ), "Triangle and Quad Memory", "mesh_memory" );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_vector.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_vector.txt" ) );
  QVERIFY( mMemoryLayer->isValid() );

  // Mdal 3D layer
  mMdal3DLayer = new QgsMeshLayer( mDataDir + "/trap_steady_05_3D.nc", "Stacked 3D Mdal", "mdal" );
  QVERIFY( mMdal3DLayer->isValid() );

  // Add layers
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemoryLayer << mMdalLayer << mMdal3DLayer );
  mMapSettings->setLayers(
    QList<QgsMapLayer *>() << mMemoryLayer << mMdalLayer << mMdal3DLayer );

  // here we check that datasets automatically get our default color ramp applied ("Plasma")
  QgsMeshDatasetIndex ds( 0, 0 );
  QgsMeshRendererScalarSettings scalarSettings = mMemoryLayer->rendererSettings().scalarSettings( ds.group() );
  QgsColorRampShader shader = scalarSettings.colorRampShader();
  QList<QgsColorRampShader::ColorRampItem> lst = shader.colorRampItemList();
  QCOMPARE( lst.count(), 52 );
  QCOMPARE( lst.at( 0 ).value, 1. );  // min group value
  QCOMPARE( lst.at( lst.count() - 1 ).value, 4. );  // max group value
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

bool TestQgsMeshRenderer::imageCheck( const QString &testType, QgsMeshLayer *layer )
{
  mReport += "<h2>" + testType + "</h2>\n";
  mMapSettings->setExtent( layer->extent() );
  mMapSettings->setDestinationCrs( layer->crs() );
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
  QVERIFY( imageCheck( "quad_and_triangle_native_mesh", mMemoryLayer ) );
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
  QVERIFY( imageCheck( "quad_and_triangle_triangular_mesh", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_vertex_scalar_dataset_rendering()
{
  QgsMeshDatasetIndex ds( 0, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexScalarDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_scalar_dataset", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_vertex_vector_dataset_rendering()
{
  QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  QgsMeshRendererVectorArrowSettings arrowSettings = settings.arrowSettings();
  arrowSettings.setMinShaftLength( 15 );
  settings.setArrowsSettings( arrowSettings );
  rendererSettings.setVectorSettings( ds.group(), settings );
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_dataset", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_face_scalar_dataset_rendering()
{
  QgsMeshDatasetIndex ds( 2, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceScalarDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_face_scalar_dataset", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_face_scalar_dataset_interpolated_neighbour_average_rendering()
{
  QgsMeshDatasetIndex ds( 2, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceScalarDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset( ds );
  auto scalarRendererSettings = rendererSettings.scalarSettings( 2 );
  scalarRendererSettings.setDataInterpolationMethod( QgsMeshRendererScalarSettings::NeighbourAverage );
  rendererSettings.setScalarSettings( 2, scalarRendererSettings );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_face_scalar_interpolated_neighbour_average_dataset", mMemoryLayer ) );
}


void TestQgsMeshRenderer::test_face_vector_dataset_rendering()
{
  QgsMeshDatasetIndex ds( 3, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_face_vector_dataset", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_vertex_scalar_dataset_with_inactive_face_rendering()
{
  QgsMeshDatasetIndex ds( 1, 1 );
  const QgsMeshDatasetGroupMetadata metadata = mMdalLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexScalarDatasetWithInactiveFace1" );

  QgsMeshRendererSettings rendererSettings = mMdalLayer->rendererSettings();
  rendererSettings.setActiveScalarDataset( ds );
  mMdalLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_scalar_dataset_with_inactive_face", mMdalLayer ) );
}

void TestQgsMeshRenderer::test_face_vector_on_user_grid()
{
  QgsMeshDatasetIndex ds( 3, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 30 );
  settings.setUserGridCellHeight( 20 );
  settings.setLineWidth( 0.8 );
  settings.setSymbology( QgsMeshRendererVectorSettings::Arrows );
  rendererSettings.setVectorSettings( ds.group(), settings );
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_face_vector_user_grid_dataset", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_face_vector_on_user_grid_streamlines()
{
  QgsMeshDatasetIndex ds( 3, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 30 );
  settings.setUserGridCellHeight( 20 );
  settings.setLineWidth( 0.8 );
  settings.setSymbology( QgsMeshRendererVectorSettings::Streamlines );
  rendererSettings.setVectorSettings( ds.group(), settings );
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_face_vector_user_grid_dataset_streamlines", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_vertex_vector_on_user_grid()
{
  QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 60 );
  settings.setUserGridCellHeight( 40 );
  settings.setLineWidth( 0.9 );
  settings.setSymbology( QgsMeshRendererVectorSettings::Arrows );
  rendererSettings.setVectorSettings( ds.group(), settings );
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_user_grid_dataset", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_vertex_vector_on_user_grid_streamlines()
{
  QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 60 );
  settings.setUserGridCellHeight( 40 );
  settings.setLineWidth( 0.9 );
  settings.setSymbology( QgsMeshRendererVectorSettings::Streamlines );
  rendererSettings.setVectorSettings( ds.group(), settings );
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_user_grid_dataset_streamlines", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_vertex_vector_traces()
{
  QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 60 );
  settings.setUserGridCellHeight( 40 );
  settings.setLineWidth( 1 );

  settings.setSymbology( QgsMeshRendererVectorSettings::Traces );
  QgsMeshRendererVectorTracesSettings tracesSetting = settings.tracesSettings();
  tracesSetting.setParticlesCount( -1 );
  tracesSetting.setMaximumTailLength( 40 );
  tracesSetting.setMaximumTailLengthUnit( QgsUnitTypes::RenderPixels );
  settings.setTracesSettings( tracesSetting );
  rendererSettings.setVectorSettings( ds.group(), settings );
  rendererSettings.setActiveVectorDataset( ds );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_traces", mMemoryLayer ) );
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

void TestQgsMeshRenderer::test_stacked_3d_mesh_single_level_averaging()
{
  QgsMeshRendererSettings rendererSettings = mMdal3DLayer->rendererSettings();
  // we want to set active scalar dataset one defined on 3d mesh
  QgsMeshDatasetIndex ds( 1, 3 );
  QgsMeshDatasetGroupMetadata metadata = mMdal3DLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "temperature" );
  QVERIFY( metadata.maximumVerticalLevelsCount() == 10 );
  rendererSettings.setActiveScalarDataset( ds );
  // want to set active vector dataset one defined on 3d mesh
  ds = QgsMeshDatasetIndex( 6, 3 );
  metadata = mMdal3DLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "velocity" );
  QVERIFY( metadata.maximumVerticalLevelsCount() == 10 );
  QgsMeshRendererVectorSettings vectorSettings = rendererSettings.vectorSettings( ds.group() );
  QgsMeshRendererVectorArrowSettings arrowSettings = vectorSettings.arrowSettings();
  arrowSettings.setShaftLengthMethod( QgsMeshRendererVectorArrowSettings::ArrowScalingMethod::Scaled );
  vectorSettings.setOnUserDefinedGrid( true );
  vectorSettings.setUserGridCellWidth( 60 );
  vectorSettings.setUserGridCellHeight( 10 );
  arrowSettings.setScaleFactor( 35 );
  vectorSettings.setLineWidth( 1 );
  vectorSettings.setArrowsSettings( arrowSettings );
  rendererSettings.setVectorSettings( ds.group(), vectorSettings );
  rendererSettings.setActiveVectorDataset( ds );
  // switch off mesh renderings
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  std::unique_ptr<QgsMeshMultiLevelsAveragingMethod> method( new QgsMeshMultiLevelsAveragingMethod( 1, true ) );
  rendererSettings.setAveragingMethod( method.get() );
  mMdal3DLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "stacked_3d_mesh_single_level_averaging", mMdal3DLayer ) );
}

void TestQgsMeshRenderer::test_simplified_triangular_mesh_rendering()
{
  QgsMeshSimplifySettings simplificatationSettings;
  simplificatationSettings.setEnabled( true );
  simplificatationSettings.setMeshResolution( 10 );
  simplificatationSettings.setReductionFactor( 2 );

  QgsMeshRendererSettings rendererSettings = mMdal3DLayer->rendererSettings();
  QgsMeshRendererMeshSettings meshSettings = rendererSettings.triangularMeshSettings();
  meshSettings.setEnabled( true );
  rendererSettings.setTriangularMeshSettings( meshSettings );
  mMdal3DLayer->setRendererSettings( rendererSettings );

  mMdal3DLayer->setMeshSimplificationSettings( simplificatationSettings );
  QVERIFY( imageCheck( "simplified_triangular_mesh", mMdal3DLayer ) );
}

QGSTEST_MAIN( TestQgsMeshRenderer )
#include "testqgsmeshlayerrenderer.moc"
