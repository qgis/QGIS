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
#include <QSignalSpy>
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
#include "qgsmeshlayertemporalproperties.h"

//qgis test includes
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the different renderers for mesh layers.
 */
class TestQgsMeshRenderer : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMeshRenderer() : QgsTest( QStringLiteral( "Mesh Layer Rendering Tests" ) ) {}

  private:
    QString mDataDir;
    QgsMeshLayer *mMemory1DLayer = nullptr;
    QgsMeshLayer *mMemoryLayer = nullptr;
    QgsMeshLayer *mMdalLayer = nullptr;
    QgsMeshLayer *mMdal3DLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    bool imageCheck( const QString &testType, QgsMeshLayer *layer, double rotation = 0.0 );
    QString readFile( const QString &fname ) const;


    void test_native_mesh_rendering();
    void test_native_mesh_renderingWithClipping();
    void test_triangular_mesh_rendering();
    void test_edge_mesh_rendering();

    void test_1d_vertex_scalar_dataset_rendering();
    void test_1d_vertex_vector_dataset_rendering();
    void test_1d_edge_scalar_dataset_rendering();
    void test_1d_edge_vector_dataset_rendering();

    void test_vertex_scalar_dataset_rendering();
    void test_vertex_vector_dataset_rendering();
    void test_vertex_vector_dataset_colorRamp_rendering();
    void test_face_scalar_dataset_rendering();
    void test_face_scalar_dataset_interpolated_neighbour_average_rendering();
    void test_face_vector_dataset_rendering();
    void test_vertex_scalar_dataset_with_inactive_face_rendering();
    void test_face_vector_on_user_grid();
    void test_face_vector_on_user_grid_streamlines();
    void test_vertex_vector_on_user_grid();
    void test_vertex_vector_on_user_grid_streamlines();
    void test_vertex_vector_on_user_grid_streamlines_colorRamp();
    void test_vertex_vector_traces();
    void test_vertex_vector_traces_colorRamp();
    void test_stacked_3d_mesh_single_level_averaging();
    void test_simplified_triangular_mesh_rendering();
    void test_classified_values();

    void test_signals();
};

void TestQgsMeshRenderer::init()
{
  QgsMeshRendererSettings rendererSettings = mMemory1DLayer->rendererSettings();
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setEdgeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setAveragingMethod( nullptr );
  mMemory1DLayer->setRendererSettings( rendererSettings );
  mMemory1DLayer->temporalProperties()->setIsActive( false );
  mMemory1DLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex() );
  mMemory1DLayer->setStaticVectorDatasetIndex( QgsMeshDatasetIndex() );

  rendererSettings = mMemoryLayer->rendererSettings();
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setEdgeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setAveragingMethod( nullptr );
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->temporalProperties()->setIsActive( false );
  mMemoryLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex() );
  mMemoryLayer->setStaticVectorDatasetIndex( QgsMeshDatasetIndex() );

  rendererSettings = mMdalLayer->rendererSettings();
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setEdgeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setAveragingMethod( nullptr );
  mMdalLayer->setRendererSettings( rendererSettings );
  mMdalLayer->temporalProperties()->setIsActive( false );
  mMdalLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex() );
  mMdalLayer->setStaticVectorDatasetIndex( QgsMeshDatasetIndex() );

  rendererSettings = mMdal3DLayer->rendererSettings();
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setEdgeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setAveragingMethod( nullptr );
  mMdal3DLayer->setRendererSettings( rendererSettings );
  mMdal3DLayer->temporalProperties()->setIsActive( false );
  mMdal3DLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex() );
  mMdal3DLayer->setStaticVectorDatasetIndex( QgsMeshDatasetIndex() );
}

void TestQgsMeshRenderer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/mesh";

  mMapSettings = new QgsMapSettings();

  // Memory 1D layer
  mMemory1DLayer = new QgsMeshLayer( readFile( "/lines.txt" ), "Lines Memory", "mesh_memory" );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_vertex_scalar.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_vertex_vector.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_els_scalar.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_els_vector.txt" ) );
  QVERIFY( mMemory1DLayer->isValid() );

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
    QList<QgsMapLayer *>() << mMemory1DLayer << mMemoryLayer << mMdalLayer << mMdal3DLayer );
  mMapSettings->setLayers(
    QList<QgsMapLayer *>() << mMemory1DLayer << mMemoryLayer << mMdalLayer << mMdal3DLayer );

  // here we check that datasets automatically get our default color ramp applied ("Plasma")
  QgsMeshDatasetIndex ds( 0, 0 );
  const QgsMeshRendererScalarSettings scalarSettings = mMemoryLayer->rendererSettings().scalarSettings( ds.group() );
  QgsColorRampShader shader = scalarSettings.colorRampShader();
  QList<QgsColorRampShader::ColorRampItem> lst = shader.colorRampItemList();
  QCOMPARE( lst.count(), 52 );
  QCOMPARE( lst.at( 0 ).value, 1. );  // min group value
  QCOMPARE( lst.at( lst.count() - 1 ).value, 4. );  // max group value

  ds = QgsMeshDatasetIndex( 1, 0 );
  const QgsMeshRendererVectorSettings vectorSettings = mMemoryLayer->rendererSettings().vectorSettings( ds.group() );
  shader = vectorSettings.colorRampShader();
  lst = shader.colorRampItemList();
  QCOMPARE( lst.count(), 52 );
  QVERIFY( fabs( lst.at( 0 ).value - 1.41421356237 ) < 0.000001 ); // min group value
  QCOMPARE( lst.at( lst.count() - 1 ).value, 5. ); // max group value
}

void TestQgsMeshRenderer::cleanupTestCase()
{
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

bool TestQgsMeshRenderer::imageCheck( const QString &testType, QgsMeshLayer *layer, double rotation )
{
  mMapSettings->setDestinationCrs( layer->crs() );
  mMapSettings->setExtent( layer->extent() );
  mMapSettings->setRotation( rotation );
  mMapSettings->setOutputDpi( 96 );

  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "mesh" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 15 );
  const bool myResultFlag = myChecker.runTest( testType, 0 );
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
  QVERIFY( imageCheck( "quad_and_triangle_native_mesh_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_native_mesh_renderingWithClipping()
{
  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererMeshSettings settings = rendererSettings.nativeMeshSettings();
  settings.setEnabled( true );
  settings.setLineWidth( 1. );
  rendererSettings.setNativeMeshSettings( settings );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QgsMapClippingRegion region( QgsGeometry::fromWkt( "Polygon ((1706.47279549718587077 2907.78611632270212795, 1306.56660412757969425 2480.30018761726114462, 1665.10318949343354689 2123.73358348968167775, 2360.5065666041273289 2060.6941838649163401, 2640.24390243902416842 2669.41838649155761232, 2228.51782363977508794 2874.29643527204552811, 1706.47279549718587077 2907.78611632270212795))" ) );
  region.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipPainterOnly );
  QgsMapClippingRegion region2( QgsGeometry::fromWkt( "Polygon ((1966.51031894934340016 2925.51594746716773443, 1801.03189493433410462 2452.7204502814265652, 2057.12945590994377199 2027.20450281425951289, 2457.03564727954972113 2033.11444652908130593, 2380.20637898686709377 2957.03564727955017588, 1966.51031894934340016 2925.51594746716773443))" ) );
  region2.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipToIntersection );
  mMapSettings->addClippingRegion( region );
  mMapSettings->addClippingRegion( region2 );

  const bool res = imageCheck( "painterclip_region", mMemoryLayer );

  mMapSettings->setClippingRegions( QList< QgsMapClippingRegion >() );
  QVERIFY( res );
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
  QVERIFY( imageCheck( "quad_and_triangle_triangular_mesh_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_edge_mesh_rendering()
{
  QgsMeshRendererSettings rendererSettings = mMemory1DLayer->rendererSettings();
  QgsMeshRendererMeshSettings settings = rendererSettings.edgeMeshSettings();
  settings.setEnabled( true );
  settings.setColor( Qt::red );
  settings.setLineWidth( 0.26 );
  rendererSettings.setEdgeMeshSettings( settings );
  mMemory1DLayer->setRendererSettings( rendererSettings );
  QVERIFY( imageCheck( "lines_edge_mesh", mMemory1DLayer ) );
}

void TestQgsMeshRenderer::test_1d_vertex_scalar_dataset_rendering()
{
  const QgsMeshDatasetIndex ds( 0, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemory1DLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexScalarDataset" );

  QgsMeshRendererSettings rendererSettings = mMemory1DLayer->rendererSettings();
  QgsMeshRendererScalarSettings scalarSettings = rendererSettings.scalarSettings( 0 );
  QgsInterpolatedLineWidth strokeWidth = scalarSettings.edgeStrokeWidth();
  strokeWidth.setMaximumValue( 3 );
  strokeWidth.setMaximumWidth( 2 );
  strokeWidth.setIsVariableWidth( true );
  scalarSettings.setEdgeStrokeWidth( strokeWidth );
  rendererSettings.setScalarSettings( 0, scalarSettings );
  mMemory1DLayer->setRendererSettings( rendererSettings );
  mMemory1DLayer->setStaticScalarDatasetIndex( ds );

  QVERIFY( imageCheck( "lines_vertex_scalar_dataset", mMemory1DLayer ) );
  QVERIFY( imageCheck( "lines_vertex_scalar_dataset_rotated_45", mMemory1DLayer, 45 ) );
}

void TestQgsMeshRenderer::test_1d_vertex_vector_dataset_rendering()
{
  const QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemory1DLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemory1DLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  QgsMeshRendererVectorArrowSettings arrowSettings = settings.arrowSettings();
  arrowSettings.setMinShaftLength( 15 );
  settings.setArrowsSettings( arrowSettings );
  rendererSettings.setVectorSettings( ds.group(), settings );
  mMemory1DLayer->setRendererSettings( rendererSettings );
  mMemory1DLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "lines_vertex_vector_dataset", mMemory1DLayer ) );
  QVERIFY( imageCheck( "lines_vertex_vector_dataset_rotated_45", mMemory1DLayer, 45 ) );
}

void TestQgsMeshRenderer::test_1d_edge_scalar_dataset_rendering()
{
  const QgsMeshDatasetIndex ds( 2, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemory1DLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "EdgeScalarDataset" );

  QgsMeshRendererSettings rendererSettings = mMemory1DLayer->rendererSettings();
  QgsMeshRendererScalarSettings scalarSettings = rendererSettings.scalarSettings( 2 );
  QgsInterpolatedLineWidth strokeWidth = scalarSettings.edgeStrokeWidth();
  strokeWidth.setMaximumValue( 3 );
  strokeWidth.setMaximumWidth( 2 );
  strokeWidth.setIsVariableWidth( true );
  scalarSettings.setEdgeStrokeWidth( strokeWidth );
  rendererSettings.setScalarSettings( 2, scalarSettings );
  mMemory1DLayer->setRendererSettings( rendererSettings );
  mMemory1DLayer->setStaticScalarDatasetIndex( ds );

  QVERIFY( imageCheck( "lines_edge_scalar_dataset", mMemory1DLayer ) );
  QVERIFY( imageCheck( "lines_edge_scalar_dataset_rotated_45", mMemory1DLayer, 45 ) );
}

void TestQgsMeshRenderer::test_1d_edge_vector_dataset_rendering()
{
  const QgsMeshDatasetIndex ds( 3, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemory1DLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "EdgeVectorDataset" );

  const QgsMeshRendererSettings rendererSettings = mMemory1DLayer->rendererSettings();
  mMemory1DLayer->setRendererSettings( rendererSettings );
  mMemory1DLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "lines_edge_vector_dataset", mMemory1DLayer ) );
  QVERIFY( imageCheck( "lines_edge_vector_dataset_rotated_45", mMemory1DLayer, 45 ) );
}

void TestQgsMeshRenderer::test_vertex_scalar_dataset_rendering()
{
  const QgsMeshDatasetIndex ds( 0, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexScalarDataset" );

  const QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticScalarDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_scalar_dataset", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_vertex_scalar_dataset_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_vertex_vector_dataset_rendering()
{
  const QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  QgsMeshRendererVectorArrowSettings arrowSettings = settings.arrowSettings();
  arrowSettings.setMinShaftLength( 15 );
  settings.setArrowsSettings( arrowSettings );
  rendererSettings.setVectorSettings( ds.group(), settings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_dataset", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_dataset_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_vertex_vector_dataset_colorRamp_rendering()
{
  const QgsMeshDatasetIndex ds( 1, 0 );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  QgsMeshRendererVectorArrowSettings arrowSettings = settings.arrowSettings();
  arrowSettings.setMinShaftLength( 15 );
  settings.setColoringMethod( QgsInterpolatedLineColor::ColorRamp );
  settings.setArrowsSettings( arrowSettings );
  rendererSettings.setVectorSettings( ds.group(), settings );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_dataset_colorRamp", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_face_scalar_dataset_rendering()
{
  const QgsMeshDatasetIndex ds( 2, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceScalarDataset" );

  const QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticScalarDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_face_scalar_dataset", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_face_scalar_dataset_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_face_scalar_dataset_interpolated_neighbour_average_rendering()
{
  const QgsMeshDatasetIndex ds( 2, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceScalarDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  auto scalarRendererSettings = rendererSettings.scalarSettings( 2 );
  scalarRendererSettings.setDataResamplingMethod( QgsMeshRendererScalarSettings::NeighbourAverage );
  rendererSettings.setScalarSettings( 2, scalarRendererSettings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticScalarDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_face_scalar_interpolated_neighbour_average_dataset", mMemoryLayer ) );
}


void TestQgsMeshRenderer::test_face_vector_dataset_rendering()
{
  const QgsMeshDatasetIndex ds( 3, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "FaceVectorDataset" );

  const QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_face_vector_dataset", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_face_vector_dataset_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_vertex_scalar_dataset_with_inactive_face_rendering()
{
  const QgsMeshDatasetIndex ds( 1, 1 );
  const QgsMeshDatasetGroupMetadata metadata = mMdalLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexScalarDatasetWithInactiveFace1" );

  const QgsMeshRendererSettings rendererSettings = mMdalLayer->rendererSettings();
  mMdalLayer->setRendererSettings( rendererSettings );
  mMdalLayer->setStaticScalarDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_scalar_dataset_with_inactive_face", mMdalLayer ) );
}

void TestQgsMeshRenderer::test_face_vector_on_user_grid()
{
  const QgsMeshDatasetIndex ds( 3, 0 );
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
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_face_vector_user_grid_dataset", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_face_vector_user_grid_dataset_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_face_vector_on_user_grid_streamlines()
{
  const QgsMeshDatasetIndex ds( 3, 0 );
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
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_face_vector_user_grid_dataset_streamlines", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_face_vector_user_grid_dataset_streamlines_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_vertex_vector_on_user_grid()
{
  const QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 60 );
  settings.setUserGridCellHeight( 40 );
  settings.setLineWidth( 0.9 );
  settings.setSymbology( QgsMeshRendererVectorSettings::Arrows );
  settings.setColoringMethod( QgsInterpolatedLineColor::SingleColor );
  rendererSettings.setVectorSettings( ds.group(), settings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_user_grid_dataset", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_user_grid_dataset_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_vertex_vector_on_user_grid_streamlines()
{
  const QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 60 );
  settings.setUserGridCellHeight( 40 );
  settings.setLineWidth( 0.9 );
  settings.setColoringMethod( QgsInterpolatedLineColor::SingleColor );
  settings.setSymbology( QgsMeshRendererVectorSettings::Streamlines );
  rendererSettings.setVectorSettings( ds.group(), settings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_user_grid_dataset_streamlines", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_user_grid_dataset_streamlines_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_vertex_vector_on_user_grid_streamlines_colorRamp()
{
  const QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 60 );
  settings.setUserGridCellHeight( 40 );
  settings.setLineWidth( 0.9 );
  settings.setColoringMethod( QgsInterpolatedLineColor::ColorRamp );
  settings.setSymbology( QgsMeshRendererVectorSettings::Streamlines );
  rendererSettings.setVectorSettings( ds.group(), settings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_user_grid_dataset_streamlines_colorRamp", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_vertex_vector_traces()
{
  const QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 60 );
  settings.setUserGridCellHeight( 40 );
  settings.setLineWidth( 1 );
  settings.setColoringMethod( QgsInterpolatedLineColor::SingleColor );

  settings.setSymbology( QgsMeshRendererVectorSettings::Traces );
  QgsMeshRendererVectorTracesSettings tracesSetting = settings.tracesSettings();
  tracesSetting.setParticlesCount( -1 );
  tracesSetting.setMaximumTailLength( 40 );
  tracesSetting.setMaximumTailLengthUnit( QgsUnitTypes::RenderPixels );
  settings.setTracesSettings( tracesSetting );
  rendererSettings.setVectorSettings( ds.group(), settings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_traces", mMemoryLayer ) );
  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_traces_rotated_45", mMemoryLayer, 45.0 ) );
}

void TestQgsMeshRenderer::test_vertex_vector_traces_colorRamp()
{
  const QgsMeshDatasetIndex ds( 1, 0 );
  const QgsMeshDatasetGroupMetadata metadata = mMemoryLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "VertexVectorDataset" );

  QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( ds.group() );
  settings.setOnUserDefinedGrid( true );
  settings.setUserGridCellWidth( 60 );
  settings.setUserGridCellHeight( 40 );
  settings.setLineWidth( 1 );
  settings.setColoringMethod( QgsInterpolatedLineColor::ColorRamp );

  settings.setSymbology( QgsMeshRendererVectorSettings::Traces );
  QgsMeshRendererVectorTracesSettings tracesSetting = settings.tracesSettings();
  tracesSetting.setParticlesCount( -1 );
  tracesSetting.setMaximumTailLength( 40 );
  tracesSetting.setMaximumTailLengthUnit( QgsUnitTypes::RenderPixels );
  settings.setTracesSettings( tracesSetting );
  rendererSettings.setVectorSettings( ds.group(), settings );
  mMemoryLayer->setRendererSettings( rendererSettings );
  mMemoryLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "quad_and_triangle_vertex_vector_traces_colorRamp", mMemoryLayer ) );
}

void TestQgsMeshRenderer::test_signals()
{
  const QSignalSpy spy1( mMemoryLayer, &QgsMapLayer::rendererChanged );
  const QSignalSpy spy2( mMemoryLayer->legend(), &QgsMapLayerLegend::itemsChanged );
  const QSignalSpy spy3( mMemoryLayer, &QgsMapLayer::legendChanged );

  const QgsMeshRendererSettings rendererSettings = mMemoryLayer->rendererSettings();
  mMemoryLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex( 1, 0 ) );
  mMemoryLayer->setRendererSettings( rendererSettings );

  QCOMPARE( spy1.count(), 1 );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( spy3.count(), 1 );
}

void TestQgsMeshRenderer::test_stacked_3d_mesh_single_level_averaging()
{
  QgsMeshDatasetIndex ds( 1, 3 );
  mMdal3DLayer->setStaticScalarDatasetIndex( ds );
  QgsMeshRendererSettings rendererSettings = mMdal3DLayer->rendererSettings();
  // we want to set active scalar dataset one defined on 3d mesh
  QgsMeshDatasetGroupMetadata metadata = mMdal3DLayer->dataProvider()->datasetGroupMetadata( ds );
  QVERIFY( metadata.name() == "temperature" );
  QVERIFY( metadata.maximumVerticalLevelsCount() == 10 );
  QgsMeshRendererScalarSettings scalarSettings = rendererSettings.scalarSettings( ds.group() );
  scalarSettings.setDataResamplingMethod( QgsMeshRendererScalarSettings::None );
  rendererSettings.setScalarSettings( ds.group(), scalarSettings );
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
  // switch off mesh renderings
  rendererSettings.setNativeMeshSettings( QgsMeshRendererMeshSettings() );
  rendererSettings.setTriangularMeshSettings( QgsMeshRendererMeshSettings() );
  const std::unique_ptr<QgsMeshMultiLevelsAveragingMethod> method( new QgsMeshMultiLevelsAveragingMethod( 1, true ) );
  rendererSettings.setAveragingMethod( method.get() );
  mMdal3DLayer->setRendererSettings( rendererSettings );
  mMdal3DLayer->setStaticVectorDatasetIndex( ds );

  QVERIFY( imageCheck( "stacked_3d_mesh_single_level_averaging", mMdal3DLayer ) );
}

void TestQgsMeshRenderer::test_simplified_triangular_mesh_rendering()
{
  QgsMeshSimplificationSettings simplificatationSettings;
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

void TestQgsMeshRenderer::test_classified_values()
{
  QgsMeshLayer classifiedMesh( mDataDir + "/simplebox_clm.nc", "Mesh with classified values", "mdal" );
  QVERIFY( classifiedMesh.isValid() );

  QgsProject::instance()->addMapLayer( &classifiedMesh );
  mMapSettings->setLayers( QList<QgsMapLayer *>() << &classifiedMesh );

  classifiedMesh.temporalProperties()->setIsActive( false );
  classifiedMesh.setStaticScalarDatasetIndex( QgsMeshDatasetIndex( 3, 4 ) );

  QVERIFY( imageCheck( "classified_values", &classifiedMesh ) );
}

QGSTEST_MAIN( TestQgsMeshRenderer )
#include "testqgsmeshlayerrenderer.moc"
