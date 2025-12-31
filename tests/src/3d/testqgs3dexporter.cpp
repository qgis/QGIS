/***************************************************************************
  testqgs3dexporter.cpp
  --------------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3d.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3drendercontext.h"
#include "qgs3dsceneexporter.h"
#include "qgs3dutils.h"
#include "qgsdemterrainsettings.h"
#include "qgsflatterraingenerator.h"
#include "qgsflatterrainsettings.h"
#include "qgsoffscreen3dengine.h"
#include "qgspointlightsettings.h"
#include "qgspolygon3dsymbol.h"
#include "qgsrasterlayer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"

class TestQgs3DExporter : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3DExporter()
      : QgsTest( u"3D Exporter Tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void test3DSceneExporter();
    void test3DSceneExporterBig();
    void test3DSceneExporterFlatTerrain();

  private:
    void do3DSceneExport( const QString &testName, int zoomLevelsCount, int expectedObjectCount, int expectedFeatureCount, int maxFaceCount, Qgs3DMapScene *scene, QgsVectorLayer *layerPoly, QgsOffscreen3DEngine *engine, QgsTerrainEntity *terrainEntity = nullptr );

    QgsVectorLayer *mLayerBuildings = nullptr;
};

// runs before all tests
void TestQgs3DExporter::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  mLayerBuildings = new QgsVectorLayer( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( mLayerBuildings->isValid() );

  // best to keep buildings without 2D renderer so it is not painted on the terrain
  // so we do not get some possible artifacts
  mLayerBuildings->setRenderer( nullptr );

  QgsPhongMaterialSettings materialSettings;
  materialSettings.setAmbient( Qt::lightGray );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  symbol3d->setAltitudeClamping( Qgis::AltitudeClamping::Relative );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );
}

//runs after all tests
void TestQgs3DExporter::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgs3DExporter::do3DSceneExport( const QString &testName, int zoomLevelsCount, int expectedObjectCount, int expectedFeatureCount, int maxFaceCount, Qgs3DMapScene *scene, QgsVectorLayer *layerPoly, QgsOffscreen3DEngine *engine, QgsTerrainEntity *terrainEntity )
{
  // 3d renderer must be replaced to have the tiling updated
  QgsVectorLayer3DRenderer *renderer3d = dynamic_cast<QgsVectorLayer3DRenderer *>( layerPoly->renderer3D() );
  QgsVectorLayer3DRenderer *newRenderer3d = new QgsVectorLayer3DRenderer( renderer3d->symbol()->clone() );
  QgsVectorLayer3DTilingSettings tilingSettings;
  tilingSettings.setZoomLevelsCount( zoomLevelsCount );
  tilingSettings.setShowBoundingBoxes( true );
  newRenderer3d->setTilingSettings( tilingSettings );
  layerPoly->setRenderer3D( newRenderer3d );

  Qgs3DUtils::captureSceneImage( *engine, scene );

  Qgs3DSceneExporter exporter;
  exporter.setTerrainResolution( 128 );
  exporter.setSmoothEdges( false );
  exporter.setExportNormals( true );
  exporter.setExportTextures( false );
  exporter.setTerrainTextureResolution( 512 );
  exporter.setScale( 1.0 );

  QVERIFY( exporter.parseVectorLayerEntity( scene->layerEntity( layerPoly ), layerPoly ) );
  if ( terrainEntity )
    exporter.parseTerrain( terrainEntity, "DEM_Tile" );

  QString objFileName = QString( "%1-%2" ).arg( testName ).arg( zoomLevelsCount );
  const bool saved = exporter.save( objFileName, QDir::tempPath(), 3 );
  QVERIFY( saved );

  size_t sum = 0;
  for ( auto o : qAsConst( exporter.mObjects ) )
  {
    if ( !terrainEntity ) // not compatible with terrain entity
      QVERIFY( o->indexes().size() * 3 <= o->vertexPosition().size() );
    sum += o->indexes().size();
  }

  QCOMPARE( sum, maxFaceCount );
  QCOMPARE( exporter.mExportedFeatureIds.size(), expectedFeatureCount );
  QCOMPARE( exporter.mObjects.size(), expectedObjectCount );

  QFile file( QString( "%1/%2.obj" ).arg( QDir::tempPath(), objFileName ) );
  QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
  QTextStream fileStream( &file );

  // check the generated obj file
  QGSCOMPARELONGSTR( testName.toStdString().c_str(), QString( "%1.obj" ).arg( objFileName ), fileStream.readAll().toUtf8() );
}

void TestQgs3DExporter::test3DSceneExporter()
{
  QgsVectorLayer *layerPoly = new QgsVectorLayer( testDataPath( "/3d/polygons.gpkg.gz" ), "polygons", "ogr" );
  QVERIFY( layerPoly->isValid() );

  const QgsRectangle fullExtent = layerPoly->extent();

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol();
  symbol3d->setExtrusionHeight( 10.f );
  QgsPhongMaterialSettings materialSettings;
  materialSettings.setAmbient( Qt::lightGray );
  symbol3d->setMaterialSettings( materialSettings.clone() );

  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  layerPoly->setRenderer3D( renderer3d );

  QgsProject project;
  project.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 3857 ) );
  project.addMapLayer( layerPoly );

  Qgs3DMapSettings mapSettings;
  mapSettings.setCrs( project.crs() );
  mapSettings.setExtent( fullExtent );
  mapSettings.setLayers( { layerPoly } );

  mapSettings.setTransformContext( project.transformContext() );
  mapSettings.setPathResolver( project.pathResolver() );
  mapSettings.setMapThemeCollection( project.mapThemeCollection() );
  mapSettings.setOutputDpi( 92 );

  QPoint winSize = QPoint( 640, 480 ); // default window size

  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( mapSettings, &engine );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 7000, 20.0, -10.0 );
  engine.setRootEntity( scene );

  const int nbFaces = 165;
  const int nbFeat = 3;

  // =========== check with 1 big tile ==> 1 exported object
  do3DSceneExport( "scene_export", 1, 1, nbFeat, nbFaces, scene, layerPoly, &engine );
  // =========== check with 4 tiles ==> 1 exported objects
  do3DSceneExport( "scene_export", 2, 1, nbFeat, nbFaces, scene, layerPoly, &engine );
  // =========== check with 9 tiles ==> 3 exported objects
  do3DSceneExport( "scene_export", 3, 3, nbFeat, nbFaces, scene, layerPoly, &engine );
  // =========== check with 16 tiles ==> 3 exported objects
  do3DSceneExport( "scene_export", 4, 3, nbFeat, nbFaces, scene, layerPoly, &engine );
  // =========== check with 25 tiles ==> 3 exported objects
  do3DSceneExport( "scene_export", 5, 3, nbFeat, nbFaces, scene, layerPoly, &engine );

  delete scene;
  mapSettings.setLayers( {} );
}

void TestQgs3DExporter::test3DSceneExporterBig()
{
  QgsRasterLayer *layerDtm = new QgsRasterLayer( testDataPath( "/3d/dtm.tif" ), "dtm", "gdal" );
  QVERIFY( layerDtm->isValid() );

  const QgsRectangle fullExtent = layerDtm->extent();

  QgsProject project;
  project.setCrs( layerDtm->crs() );
  project.addMapLayer( layerDtm );

  Qgs3DMapSettings mapSettings;
  mapSettings.setCrs( project.crs() );
  mapSettings.setExtent( fullExtent );
  mapSettings.setLayers( { layerDtm, mLayerBuildings } );

  mapSettings.setTransformContext( project.transformContext() );
  mapSettings.setPathResolver( project.pathResolver() );
  mapSettings.setMapThemeCollection( project.mapThemeCollection() );

  QgsDemTerrainSettings *demTerrainSettings = new QgsDemTerrainSettings;
  demTerrainSettings->setLayer( layerDtm );
  demTerrainSettings->setVerticalScale( 3 );
  mapSettings.setTerrainSettings( demTerrainSettings );

  QgsPointLightSettings defaultPointLight;
  defaultPointLight.setPosition( mapSettings.origin() + QgsVector3D( 0, 400, 0 ) );
  defaultPointLight.setConstantAttenuation( 0 );
  mapSettings.setLightSources( { defaultPointLight.clone() } );
  mapSettings.setOutputDpi( 92 );

  QPoint winSize = QPoint( 640, 480 ); // default window size

  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( mapSettings, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QVector3D( 0, 0, 0 ), 1500, 40.0, -10.0 );

  const int nbFaces = 19869;
  const int nbFeat = 401;

  // =========== check with 1 big tile ==> 1 exported object
  do3DSceneExport( "big_scene_export", 1, 1, nbFeat, nbFaces, scene, mLayerBuildings, &engine );
  // =========== check with 4 tiles ==> 4 exported objects
  do3DSceneExport( "big_scene_export", 2, 4, nbFeat, nbFaces, scene, mLayerBuildings, &engine );
  // =========== check with 9 tiles ==> 14 exported objects
  do3DSceneExport( "big_scene_export", 3, 14, nbFeat, nbFaces, scene, mLayerBuildings, &engine );
  // =========== check with 16 tiles ==> 32 exported objects
  do3DSceneExport( "big_scene_export", 4, 32, nbFeat, nbFaces, scene, mLayerBuildings, &engine );
  // =========== check with 25 tiles ==> 70 exported objects
  do3DSceneExport( "big_scene_export", 5, 70, nbFeat, nbFaces, scene, mLayerBuildings, &engine );

  delete scene;
  mapSettings.setLayers( {} );
}

void TestQgs3DExporter::test3DSceneExporterFlatTerrain()
{
  QgsRasterLayer *layerRgb = new QgsRasterLayer( testDataPath( "/3d/rgb.tif" ), "rgb", "gdal" );
  QVERIFY( layerRgb->isValid() );

  const QgsRectangle fullExtent = layerRgb->extent();

  QgsProject project;
  project.setCrs( layerRgb->crs() );
  project.addMapLayer( layerRgb );

  Qgs3DMapSettings mapSettings;
  mapSettings.setCrs( project.crs() );
  mapSettings.setExtent( fullExtent );
  mapSettings.setLayers( { layerRgb, mLayerBuildings } );

  mapSettings.setTransformContext( project.transformContext() );
  mapSettings.setPathResolver( project.pathResolver() );
  mapSettings.setMapThemeCollection( project.mapThemeCollection() );

  QgsFlatTerrainSettings *flatTerrainSettings = new QgsFlatTerrainSettings;
  mapSettings.setTerrainSettings( flatTerrainSettings );

  std::unique_ptr<QgsTerrainGenerator> generator = flatTerrainSettings->createTerrainGenerator( Qgs3DRenderContext::fromMapSettings( &mapSettings ) );
  QVERIFY( dynamic_cast<QgsFlatTerrainGenerator *>( generator.get() )->isValid() );
  QCOMPARE( dynamic_cast<QgsFlatTerrainGenerator *>( generator.get() )->crs(), mapSettings.crs() );

  QgsPointLightSettings defaultPointLight;
  defaultPointLight.setPosition( mapSettings.origin() + QgsVector3D( 0, 400, 0 ) );
  defaultPointLight.setConstantAttenuation( 0 );
  mapSettings.setLightSources( { defaultPointLight.clone() } );
  mapSettings.setOutputDpi( 92 );

  QPoint winSize = QPoint( 640, 480 ); // default window size

  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( mapSettings, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QVector3D( 0, 0, 0 ), 1500, 40.0, -10.0 );

  do3DSceneExport( "flat_terrain_scene_export", 5, 70, 401, 19875, scene, mLayerBuildings, &engine, scene->terrainEntity() );

  delete scene;
  mapSettings.setLayers( {} );
}

QGSTEST_MAIN( TestQgs3DExporter )
#include "testqgs3dexporter.moc"
