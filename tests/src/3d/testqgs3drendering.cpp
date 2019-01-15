/***************************************************************************
  testqgs3drendering.cpp
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsrenderchecker.h"

#include "qgsmaplayerstylemanager.h"
#include "qgsmapthemecollection.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrastershader.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsmeshlayer.h"

#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgschunknode_p.h"
#include "qgsdemterraingenerator.h"
#include "qgsflatterraingenerator.h"
#include "qgsoffscreen3dengine.h"
#include "qgspolygon3dsymbol.h"
#include "qgsterrainentity_p.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgsmeshlayer3drenderer.h"

class TestQgs3DRendering : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testFlatTerrain();
    void testDemTerrain();
    void testExtrudedPolygons();
    void testMapTheme();
    void testMesh();

  private:
    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0 );

    QString mReport;

    std::unique_ptr<QgsProject> mProject;
    QgsRasterLayer *mLayerDtm;
    QgsRasterLayer *mLayerRgb;
    QgsVectorLayer *mLayerBuildings;
    QgsMeshLayer *mLayerMesh;
};

//runs before all tests
void TestQgs3DRendering::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport = QStringLiteral( "<h1>3D Rendering Tests</h1>\n" );

  mProject.reset( new QgsProject );

  QString dataDir( TEST_DATA_DIR );
  mLayerDtm = new QgsRasterLayer( dataDir + "/3d/dtm.tif", "dtm", "gdal" );
  QVERIFY( mLayerDtm->isValid() );
  mProject->addMapLayer( mLayerDtm );

  mLayerRgb = new QgsRasterLayer( dataDir + "/3d/rgb.tif", "rgb", "gdal" );
  QVERIFY( mLayerRgb->isValid() );
  mProject->addMapLayer( mLayerRgb );

  mLayerBuildings = new QgsVectorLayer( dataDir + "/3d/buildings.shp", "buildings", "ogr" );
  QVERIFY( mLayerBuildings->isValid() );
  mProject->addMapLayer( mLayerBuildings );

  QgsPhongMaterialSettings material;
  material.setAmbient( Qt::lightGray );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterial( material );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  mLayerMesh = new QgsMeshLayer( dataDir + "/mesh/quad_flower.2dm", "mesh", "mdal" );
  QVERIFY( mLayerMesh->isValid() );
  mLayerMesh->setCrs( mLayerDtm->crs() );  // this testing mesh does not have any CRS defined originally
  mProject->addMapLayer( mLayerMesh );

  QgsPhongMaterialSettings meshMaterial;
  QgsMesh3DSymbol *symbolMesh3d = new QgsMesh3DSymbol;
  symbolMesh3d->setMaterial( meshMaterial );
  QgsMeshLayer3DRenderer *meshRenderer3d = new QgsMeshLayer3DRenderer( symbolMesh3d );
  mLayerMesh->setRenderer3D( meshRenderer3d );

  mProject->setCrs( mLayerDtm->crs() );

  //
  // prepare styles for DTM layer
  //

  mLayerDtm->styleManager()->addStyleFromLayer( "grayscale" );

  double vMin = 44, vMax = 198;
  QColor cMin = Qt::red, cMax = Qt::yellow;

  // change renderer for the new style
  std::unique_ptr<QgsColorRampShader> colorRampShader( new QgsColorRampShader( vMin, vMax ) );
  colorRampShader->setColorRampItemList( QList<QgsColorRampShader::ColorRampItem>()
                                         << QgsColorRampShader::ColorRampItem( vMin, cMin )
                                         << QgsColorRampShader::ColorRampItem( vMax, cMax ) );
  std::unique_ptr<QgsRasterShader> shader( new QgsRasterShader( vMin, vMax ) );
  shader->setRasterShaderFunction( colorRampShader.release() );
  QgsSingleBandPseudoColorRenderer *r = new QgsSingleBandPseudoColorRenderer( mLayerDtm->renderer()->input(), 1, shader.release() );
  mLayerDtm->setRenderer( r );
  mLayerDtm->styleManager()->addStyleFromLayer( "my_style" );

  mLayerDtm->styleManager()->setCurrentStyle( "grayscale" );

  //
  // add map theme
  //

  QgsMapThemeCollection::MapThemeLayerRecord layerRecord( mLayerDtm );
  layerRecord.usingCurrentStyle = true;
  layerRecord.currentStyle = "my_style";
  QgsMapThemeCollection::MapThemeRecord record;
  record.addLayerRecord( layerRecord );
  mProject->mapThemeCollection()->insert( "theme_dtm", record );
}

//runs after all tests
void TestQgs3DRendering::cleanupTestCase()
{
  mProject.reset();

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

void TestQgs3DRendering::testFlatTerrain()
{
  QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerRgb );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "flat_terrain_1", img, 40 ) );

  // tilted view (pitch = 60 degrees)
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 60, 0 );
  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "flat_terrain_2", img2, 40 ) );

  // also add horizontal rotation (yaw = 45 degrees)
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 60, 45 );
  QImage img3 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "flat_terrain_3", img3, 40 ) );
}

void TestQgs3DRendering::testDemTerrain()
{
  QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerRgb );

  QgsDemTerrainGenerator *demTerrain = new QgsDemTerrainGenerator;
  demTerrain->setLayer( mLayerDtm );
  map->setTerrainGenerator( demTerrain );
  map->setTerrainVerticalScale( 3 );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2000, 60, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img3 = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "dem_terrain_1", img3, 40 ) );
}

void TestQgs3DRendering::testExtrudedPolygons()
{
  QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );
  QgsPointLightSettings defaultLight;
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setPointLights( QList<QgsPointLightSettings>() << defaultLight );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 250 ), 500, 45, 0 );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "polygon3d_extrusion", img, 40 ) );
}

void TestQgs3DRendering::testMapTheme()
{
  QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerRgb );

  // set theme - this should override what we set in setLayers()
  map->setMapThemeCollection( mProject->mapThemeCollection() );
  map->setTerrainMapTheme( "theme_dtm" );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "terrain_theme", img, 40 ) );
}

void TestQgs3DRendering::testMesh()
{
  QgsRectangle fullExtent = mLayerMesh->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerMesh );
  QgsPointLightSettings defaultLight;
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setPointLights( QList<QgsPointLightSettings>() << defaultLight );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 3000, 25, 45 );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "mesh3d", img, 40 ) );
}

bool TestQgs3DRendering::renderCheck( const QString &testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + '/';
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "3d" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  myChecker.setColorTolerance( 2 );  // color tolerance < 2 was failing polygon3d_extrusion test
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgs3DRendering )
#include "testqgs3drendering.moc"
