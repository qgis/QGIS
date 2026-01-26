/***************************************************************************
  testqgs3dmapscene.cpp
  --------------------------------------
  Date                 : March 2025
  Copyright            : (C) 2025 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgs3d.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsflatterraingenerator.h"
#include "qgsoffscreen3dengine.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgsprojectviewsettings.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

class QgsSingleColorPointCloud3DSymbol;
class TestQgs3DMapScene : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3DMapScene()
      : QgsTest( u"3D Map Scene Tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testOriginShift();
    void testElevationRange();

  private:
    std::unique_ptr<QgsProject> mProject;
    QgsVectorLayer *mLayerCountries = nullptr;
    QgsPointCloudLayer *mPointCloudLayer = nullptr;
};

// runs before all tests
void TestQgs3DMapScene::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  mProject = std::make_unique<QgsProject>();

  const QString fileName = QgsApplication::pkgDataPath() + u"/resources/data/world_map.gpkg|layername=countries"_s;

  mLayerCountries = new QgsVectorLayer( fileName, u"world"_s, u"ogr"_s );
  QVERIFY( mLayerCountries->isValid() );
  const QString dataDir( TEST_DATA_DIR );
  mPointCloudLayer = new QgsPointCloudLayer( dataDir + "/point_clouds/ept/sunshine-coast-laz/ept.json", "test", "ept" );
  QVERIFY( mPointCloudLayer->isValid() );

  mProject->addMapLayer( mLayerCountries );
  mProject->addMapLayer( mPointCloudLayer );

  mProject->setCrs( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );
}

//runs after all tests
void TestQgs3DMapScene::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}


void TestQgs3DMapScene::testOriginShift()
{
  Qgs3DMapSettings map;
  map.setCrs( mProject->crs() );
  map.setExtent( mProject->viewSettings()->fullExtent() );
  map.setLayers( QList<QgsMapLayer *>() << mLayerCountries );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map.crs(), map.transformContext() );
  map.setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( map, &engine );
  engine.setRootEntity( scene );

  QgsVector3D webMercatorRome( 1'390'000, 5'146'000, 0 );

  map.setOrigin( QgsVector3D( 0, 0, 0 ) );
  scene->setSceneOriginShiftEnabled( false );

  // start with the initial view far from Rome...
  // origin should be unchanged because shifting is disabled
  scene->cameraController()->setLookingAtMapPoint( webMercatorRome, 250'000, 0, 0 );
  Qgs3DUtils::waitForFrame( engine, scene );

  QCOMPARE( scene->cameraController()->lookingAtMapPoint(), webMercatorRome );
  QCOMPARE( scene->mapSettings()->origin(), QgsVector3D( 0, 0, 0 ) );
  QCOMPARE( scene->cameraController()->camera()->position(), QVector3D( 1'390'000, 5'146'000, 250'000 ) );

  // now let's get automatic origin shifts enabled
  scene->setSceneOriginShiftEnabled( true );

  // we're moving closer to Rome, but still quite far - origin should be moved
  // and camera will be at the placed at the origin
  scene->cameraController()->setLookingAtMapPoint( webMercatorRome, 100'000, 0, 0 );
  Qgs3DUtils::waitForFrame( engine, scene );

  QCOMPARE( scene->cameraController()->lookingAtMapPoint(), webMercatorRome );
  QCOMPARE( scene->mapSettings()->origin(), QgsVector3D( 1'390'000, 5'146'000, 100'000 ) );
  QCOMPARE( scene->cameraController()->camera()->position(), QVector3D( 0, 0, 0 ) );

  // we're moving even closer to Rome, but the move is still quite big
  // so we're expecting another origin shift
  scene->cameraController()->setLookingAtMapPoint( webMercatorRome, 1'000, 0, 0 );
  Qgs3DUtils::waitForFrame( engine, scene );

  QCOMPARE( scene->cameraController()->lookingAtMapPoint(), webMercatorRome );
  QCOMPARE( scene->mapSettings()->origin(), QgsVector3D( 1'390'000, 5'146'000, 1'000 ) );
  QCOMPARE( scene->cameraController()->camera()->position(), QVector3D( 0, 0, 0 ) );

  // move even closer to the map. The move is relatively small,
  // so we are not experiencing any shift of the origin
  scene->cameraController()->setLookingAtMapPoint( webMercatorRome, 500, 0, 0 );
  Qgs3DUtils::waitForFrame( engine, scene );

  QCOMPARE( scene->cameraController()->lookingAtMapPoint(), webMercatorRome );
  QCOMPARE( scene->mapSettings()->origin(), QgsVector3D( 1'390'000, 5'146'000, 1'000 ) );
  QCOMPARE( scene->cameraController()->camera()->position(), QVector3D( 0, 0, -500 ) );

  delete scene;
}

void TestQgs3DMapScene::testElevationRange()
{
  Qgs3DMapSettings map;
  map.setCrs( mPointCloudLayer->crs() );
  map.setExtent( mPointCloudLayer->extent() );
  map.setLayers( QList<QgsMapLayer *>() << mPointCloudLayer );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map.crs(), map.transformContext() );
  map.setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( map, &engine );
  engine.setRootEntity( scene );

  QgsSingleColorPointCloud3DSymbol *symbol = new QgsSingleColorPointCloud3DSymbol();
  symbol->setSingleColor( QColor( 255, 0, 0 ) );
  symbol->setPointSize( 10 );

  QgsPointCloudLayer3DRenderer *renderer = new QgsPointCloudLayer3DRenderer();
  renderer->setSymbol( symbol );
  mPointCloudLayer->setRenderer3D( renderer );

  QgsDoubleRange elevationRange = scene->elevationRange( false );
  QGSCOMPARENEAR( elevationRange.lower(), 0, 0.1 );
  QGSCOMPARENEAR( elevationRange.upper(), 80, 0.1 );
  elevationRange = scene->elevationRange( true );
  QGSCOMPARENEAR( elevationRange.lower(), 74.3, 0.1 );
  QGSCOMPARENEAR( elevationRange.upper(), 80, 0.1 );
}


QGSTEST_MAIN( TestQgs3DMapScene )
#include "testqgs3dmapscene.moc"
