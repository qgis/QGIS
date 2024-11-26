/***************************************************************************
  testqgsmesh3drendering.cpp
  --------------------------------------
  Date                 : October 2023
  Copyright            : (C) 2023 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>

#include "qgstest.h"

#include "qgs3d.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsflatterraingenerator.h"
#include "qgsmeshlayer.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmeshterraingenerator.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsoffscreen3dengine.h"
#include "qgspointlightsettings.h"
#include "qgsproject.h"
#include "qgs3drendercontext.h"

class TestQgsMesh3DRendering : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMesh3DRendering()
      : QgsTest( QStringLiteral( "Mesh 3D Rendering Tests" ), QStringLiteral( "3d" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testMeshTerrain();
    void testMesh();
    void testMesh_datasetOnFaces();
    void testMeshSimplified();
    void testFilteredMesh();
    void testMeshClipping();

  private:
    std::unique_ptr<QgsProject> mProject;
    QgsMeshLayer *mLayerMeshTerrain = nullptr;
    QgsMeshLayer *mLayerMeshDataset = nullptr;
    QgsMeshLayer *mLayerMeshSimplified = nullptr;
};

//runs before all tests
void TestQgsMesh3DRendering::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  mProject.reset( new QgsProject );

  mLayerMeshTerrain = new QgsMeshLayer( testDataPath( "/mesh/quad_flower.2dm" ), "mesh", "mdal" );
  QVERIFY( mLayerMeshTerrain->isValid() );
  mLayerMeshTerrain->setCrs( QgsCoordinateReferenceSystem( "EPSG:27700" ) ); // this testing mesh does not have any CRS defined originally
  mProject->addMapLayer( mLayerMeshTerrain );

  mLayerMeshDataset = new QgsMeshLayer( testDataPath( "/mesh/quad_and_triangle.2dm" ), "mesh", "mdal" );
  mLayerMeshDataset->dataProvider()->addDataset( testDataPath( "/mesh/quad_and_triangle_vertex_scalar.dat" ) );
  mLayerMeshDataset->dataProvider()->addDataset( testDataPath( "/mesh/quad_and_triangle_vertex_vector.dat" ) );
  mLayerMeshDataset->dataProvider()->addDataset( testDataPath( "/mesh/quad_and_triangle_els_face_scalar.dat" ) );
  QVERIFY( mLayerMeshDataset->isValid() );
  mLayerMeshDataset->setCrs( mLayerMeshTerrain->crs() ); // this testing mesh does not have any CRS defined originally
  mLayerMeshDataset->temporalProperties()->setIsActive( false );
  mLayerMeshDataset->setStaticScalarDatasetIndex( QgsMeshDatasetIndex( 1, 0 ) );
  mLayerMeshDataset->setStaticVectorDatasetIndex( QgsMeshDatasetIndex( 2, 0 ) );
  mProject->addMapLayer( mLayerMeshDataset );

  mLayerMeshSimplified = new QgsMeshLayer( testDataPath( "/mesh/trap_steady_05_3D.nc" ), "mesh", "mdal" );
  mLayerMeshSimplified->setCrs( mProject->crs() );
  QVERIFY( mLayerMeshSimplified->isValid() );
  mProject->addMapLayer( mLayerMeshSimplified );

  QgsMesh3DSymbol *symbolMesh3d = new QgsMesh3DSymbol;
  symbolMesh3d->setVerticalDatasetGroupIndex( 0 );
  symbolMesh3d->setVerticalScale( 10 );
  symbolMesh3d->setRenderingStyle( QgsMesh3DSymbol::RenderingStyle::ColorRamp2DRendering );
  symbolMesh3d->setArrowsEnabled( true );
  symbolMesh3d->setArrowsSpacing( 300 );
  QgsMeshLayer3DRenderer *meshDatasetRenderer3d = new QgsMeshLayer3DRenderer( symbolMesh3d );
  mLayerMeshDataset->setRenderer3D( meshDatasetRenderer3d );

  mProject->setCrs( mLayerMeshTerrain->crs() );
}

//runs after all tests
void TestQgsMesh3DRendering::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgsMesh3DRendering::testMeshTerrain()
{
  const QgsRectangle fullExtent = mLayerMeshTerrain->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );

  QgsMeshTerrainGenerator *meshTerrain = new QgsMeshTerrainGenerator;
  meshTerrain->setCrs( mProject->crs(), mProject->transformContext() );
  meshTerrain->setLayer( mLayerMeshTerrain );
  map->setTerrainGenerator( meshTerrain );

  QCOMPARE( meshTerrain->heightAt( 750, 2500, Qgs3DRenderContext::fromMapSettings( map ) ), 500.0 );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 3000, 25, 45 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "mesh_terrain_1", "mesh_terrain_1", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgsMesh3DRendering::testMesh()
{
  const QgsRectangle fullExtent = mLayerMeshDataset->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerMeshDataset );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 3000, 25, 45 );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "mesh3d", "mesh3d", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgsMesh3DRendering::testMesh_datasetOnFaces()
{
  const QgsRectangle fullExtent = mLayerMeshDataset->extent();

  QgsMesh3DSymbol *symbolMesh3d = new QgsMesh3DSymbol;
  symbolMesh3d->setVerticalDatasetGroupIndex( 3 );
  symbolMesh3d->setVerticalScale( 10 );
  symbolMesh3d->setRenderingStyle( QgsMesh3DSymbol::RenderingStyle::ColorRamp2DRendering );
  symbolMesh3d->setArrowsEnabled( true );
  symbolMesh3d->setArrowsSpacing( 300 );
  QgsMeshLayer3DRenderer *meshDatasetRenderer3d = new QgsMeshLayer3DRenderer( symbolMesh3d );
  mLayerMeshDataset->setRenderer3D( meshDatasetRenderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerMeshDataset );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 3000, 25, 45 );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "mesh3dOnFace", "mesh3dOnFace", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgsMesh3DRendering::testMeshSimplified()
{
  if ( QgsTest::isCIRun() )
  {
    QSKIP( "Intermittently fails on CI" );
  }

  QgsMeshSimplificationSettings simplificationSettings;
  simplificationSettings.setEnabled( true );
  simplificationSettings.setReductionFactor( 3 );

  QgsMeshRendererSettings settings;
  settings.setActiveScalarDatasetGroup( 16 );
  settings.setActiveVectorDatasetGroup( 6 );
  mLayerMeshSimplified->setRendererSettings( settings );

  const QgsRectangle fullExtent = mLayerMeshSimplified->extent();
  mLayerMeshSimplified->setMeshSimplificationSettings( simplificationSettings );
  mLayerMeshSimplified->temporalProperties()->setIsActive( false );
  mLayerMeshSimplified->setStaticScalarDatasetIndex( QgsMeshDatasetIndex( 16, 5 ) );
  mLayerMeshSimplified->setStaticVectorDatasetIndex( QgsMeshDatasetIndex( 6, 5 ) );

  for ( int i = 0; i < 4; ++i )
  {
    Qgs3DMapSettings *map = new Qgs3DMapSettings;
    map->setCrs( mProject->crs() );
    map->setLayers( QList<QgsMapLayer *>() << mLayerMeshSimplified );
    map->setExtent( fullExtent );

    QgsMesh3DSymbol *symbolDataset = new QgsMesh3DSymbol;
    symbolDataset->setVerticalDatasetGroupIndex( 11 );
    symbolDataset->setVerticalScale( 1 );
    symbolDataset->setWireframeEnabled( true );
    symbolDataset->setWireframeLineWidth( 0.1 );
    symbolDataset->setArrowsEnabled( true );
    symbolDataset->setArrowsSpacing( 20 );
    symbolDataset->setSingleMeshColor( Qt::yellow );
    symbolDataset->setLevelOfDetailIndex( i );
    QgsMeshLayer3DRenderer *meshDatasetRenderer3d = new QgsMeshLayer3DRenderer( symbolDataset );
    mLayerMeshSimplified->setRenderer3D( meshDatasetRenderer3d );

    QgsMeshTerrainGenerator *meshTerrain = new QgsMeshTerrainGenerator;
    meshTerrain->setLayer( mLayerMeshSimplified );
    QgsMesh3DSymbol *symbol = new QgsMesh3DSymbol;
    symbol->setWireframeEnabled( true );
    symbol->setWireframeLineWidth( 0.1 );
    symbol->setLevelOfDetailIndex( i );
    meshTerrain->setSymbol( symbol );
    map->setTerrainGenerator( meshTerrain );

    QgsOffscreen3DEngine engine;
    Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
    engine.setRootEntity( scene );

    // look from the top
    scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 1000, 25, 45 );

    // When running the test on Travis, it would initially return empty rendered image.
    // Capturing the initial image and throwing it away fixes that. Hopefully we will
    // find a better fix in the future.
    Qgs3DUtils::captureSceneImage( engine, scene );
    QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
    delete scene;
    delete map;

    QGSVERIFYIMAGECHECK( QString( "mesh_simplified_%1" ).arg( i ), QString( "mesh_simplified_%1" ).arg( i ), img, QString(), 40, QSize( 0, 0 ), 2 );
  }
}

void TestQgsMesh3DRendering::testFilteredMesh()
{
  const QgsRectangle fullExtent = mLayerMeshDataset->extent();
  const QgsRectangle filteredExtent = QgsRectangle( fullExtent.xMinimum(), fullExtent.yMinimum(), fullExtent.xMinimum() + fullExtent.width() / 3.0, fullExtent.yMinimum() + fullExtent.height() / 4.0 );

  QgsMesh3DSymbol *symbolMesh3d = new QgsMesh3DSymbol;
  symbolMesh3d->setVerticalDatasetGroupIndex( 0 );
  symbolMesh3d->setVerticalScale( 10 );
  symbolMesh3d->setRenderingStyle( QgsMesh3DSymbol::RenderingStyle::ColorRamp2DRendering );
  symbolMesh3d->setArrowsEnabled( true );
  symbolMesh3d->setArrowsSpacing( 300 );
  QgsMeshLayer3DRenderer *meshDatasetRenderer3d = new QgsMeshLayer3DRenderer( symbolMesh3d );
  mLayerMeshDataset->setRenderer3D( meshDatasetRenderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( filteredExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerMeshDataset );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 3000, 25, 45 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "mesh3d_filtered", "mesh3d_filtered", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgsMesh3DRendering::testMeshClipping()
{
  QgsMesh3DSymbol *symbolMesh3d = new QgsMesh3DSymbol;
  symbolMesh3d->setVerticalDatasetGroupIndex( 3 );
  symbolMesh3d->setVerticalScale( 10 );
  symbolMesh3d->setRenderingStyle( QgsMesh3DSymbol::RenderingStyle::ColorRamp2DRendering );
  symbolMesh3d->setArrowsEnabled( true );
  symbolMesh3d->setArrowsSpacing( 300 );
  QgsMeshLayer3DRenderer *meshDatasetRenderer3d = new QgsMeshLayer3DRenderer( symbolMesh3d );
  mLayerMeshDataset->setRenderer3D( meshDatasetRenderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( mLayerMeshDataset->extent() );
  map->setLayers( QList<QgsMapLayer *>() << mLayerMeshDataset );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 3000, 25, 45 );

  QList<QVector4D> clipPlanesEquations = QList<QVector4D>()
                                         << QVector4D( 1.0, 0, 0.0, 1.0 );
  scene->enableClipping( clipPlanesEquations );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "mesh3d_clipping", "mesh3d_clipping", img, QString(), 40, QSize( 0, 0 ), 2 );

  scene->disableClipping();

  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;
  QGSVERIFYIMAGECHECK( "mesh3dOnFace", "mesh3dOnFace", img2, QString(), 40, QSize( 0, 0 ), 2 );
}

QGSTEST_MAIN( TestQgsMesh3DRendering )
#include "testqgsmesh3drendering.moc"
