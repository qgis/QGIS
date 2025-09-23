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
#include "qgs3dexportobject.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3drendercontext.h"
#include "qgs3dsceneexporter.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgsdemterrainsettings.h"
#include "qgsflatterraingenerator.h"
#include "qgsflatterrainsettings.h"
#include "qgsoffscreen3dengine.h"
#include "qgspoint3dsymbol.h"
#include "qgspointlightsettings.h"
#include "qgspolygon3dsymbol.h"
#include "qgsrasterlayer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"

#include <QString>

using namespace Qt::StringLiterals;

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
    void testExportObjectToObj();
    void test3DSceneExporter();
    void test3DSceneExporterBig();
    void test3DSceneExporterFlatTerrain();
    void test3DSceneExporterInstanced();
    void test3DSceneExporterInstancedTrs();
    void test3DSceneExporterInstancedDataDefinedTrs();

  private:
    void do3DSceneExport(
      const QString &testName,
      int expectedObjectCount,
      int expectedFeatureCount,
      int maxFaceCount,
      Qgs3DMapScene *scene,
      QgsVectorLayer *layerPoly,
      QgsOffscreen3DEngine *engine,
      QgsTerrainEntity *terrainEntity = nullptr
    );

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

void TestQgs3DExporter::testExportObjectToObj()
{
  // all vertice positions
  // clang-format off
  QVector<float> positionData = {
    -0.456616, 0.00187836, -0.413774,
    -0.4718, 0.00187836, -0.0764642,
    -0.25705, 0.00187836, -0.230477,
    -0.25705, 0.00187836, -0.230477,
    -0.4718, 0.00187836, -0.0764642,
    0.0184382, 0.00187836, 0.177332,
    -0.25705, 0.00187836, -0.230477,
    0.0184382, 0.00187836, 0.177332,
    -0.25705, -0.00187836, -0.230477,
    -0.25705, -0.00187836, -0.230477,
    0.0184382, 0.00187836, 0.177332,
    0.0184382, -0.00187836, 0.177332,
    0.0184382, 0.00187836, 0.177332,
    -0.4718, 0.00187836, -0.0764642,
    0.0184382, -0.00187836, 0.177332,
    0.0184382, -0.00187836, 0.177332,
    -0.4718, 0.00187836, -0.0764642,
    -0.4718, -0.00187836, -0.0764642,
    -0.4718, 0.00187836, -0.0764642,
    -0.456616, 0.00187836, -0.413774,
    -0.4718, -0.00187836, -0.0764642,
    -0.4718, -0.00187836, -0.0764642,
    -0.456616, 0.00187836, -0.413774,
    -0.456616, -0.00187836, -0.413774,
    -0.456616, 0.00187836, -0.413774,
    -0.25705, 0.00187836, -0.230477,
    -0.456616, -0.00187836, -0.413774,
    -0.456616, -0.00187836, -0.413774,
    -0.25705, 0.00187836, -0.230477,
    -0.25705, -0.00187836, -0.230477
  };

  // all vertice normals
  QVector<float> normalsData = {
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489
  };
  // clang-format on

  const QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  // case where all vertices are used
  {
    Qgs3DExportObject object( "all_faces" );

    // exported vertice indexes
    QVector<uint> indexData = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    };

    object.setupTriangle( positionData, indexData, QMatrix4x4() );
    QCOMPARE( object.vertexPosition().size(), positionData.size() );

    QCOMPARE( object.indexes().size(), indexData.size() );

    object.setupNormalCoordinates( normalsData, QMatrix4x4() );
    QCOMPARE( object.normals().size(), normalsData.size() );


    QFile file( tempDir.path() + "all_faces.obj" );
    QVERIFY( file.open( QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate ) );
    QTextStream out( &file );

    out << "o " << object.name() << "\n";
    object.saveTo( out, 1.0, QVector3D( 0, 0, 0 ), 3 );

    out.flush();
    out.seek( 0 );

    QString actual = out.readAll();
    QGSCOMPARELONGSTR( "export_obj", "all_faces.obj", actual.toUtf8() );
  }

  // case where only a subset of vertices are used
  {
    // exported vertice indexes
    // clang-format off
    QVector<uint> indexData = {
      // 0, 1, 2,
      // 3, 4, 5,
      6, 7, 8,
      9, 10, 11,
      12, 13, 14,
      15, 16, 17,
      // 18, 19, 20,
      21, 22, 23,
      // 24, 25, 26,
      // 27, 28, 29,
    };
    // clang-format on

    Qgs3DExportObject object( "sparse_faces" );
    object.setupTriangle( positionData, indexData, QMatrix4x4() );
    QCOMPARE( object.vertexPosition().size(), positionData.size() );

    QCOMPARE( object.indexes().size(), indexData.size() );

    object.setupNormalCoordinates( normalsData, QMatrix4x4() );
    QCOMPARE( object.normals().size(), normalsData.size() );

    QFile file( tempDir.path() + "sparse_faces.obj" );
    QVERIFY( file.open( QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate ) );
    QTextStream out( &file );
    out << "o " << object.name() << "\n";
    object.saveTo( out, 1.0, QVector3D( 0, 0, 0 ), 3 );

    out.flush();
    out.seek( 0 );

    QString actual = out.readAll();
    QGSCOMPARELONGSTR( "export_obj", "sparse_faces.obj", actual.toUtf8() );
  }
}

void TestQgs3DExporter::do3DSceneExport(
  const QString &testName, int expectedObjectCount, int expectedFeatureCount, int maxFaceCount, Qgs3DMapScene *scene, QgsVectorLayer *layerPoly, QgsOffscreen3DEngine *engine, QgsTerrainEntity *terrainEntity
)
{
  // 3d renderer must be replaced to have the tiling updated
  QgsVectorLayer3DRenderer *renderer3d = dynamic_cast<QgsVectorLayer3DRenderer *>( layerPoly->renderer3D() );
  QgsVectorLayer3DRenderer *newRenderer3d = new QgsVectorLayer3DRenderer( renderer3d->symbol()->clone() );
  QgsVectorLayer3DTilingSettings tilingSettings;
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

  const QString objFileName = testName;
  const bool saved = exporter.save( objFileName, QDir::tempPath(), 3 );
  QVERIFY( saved );

  size_t sum = 0;
  for ( Qgs3DExportObject *o : std::as_const( exporter.mObjects ) )
  {
    if ( !terrainEntity ) // not compatible with terrain entity
      QVERIFY( o->indexes().size() <= o->vertexPosition().size() );
    sum += o->indexes().size();
  }

  QCOMPARE( sum, maxFaceCount );
  QCOMPARE( exporter.mExportedFeatureIds.size(), expectedFeatureCount );
  QCOMPARE( exporter.mObjects.size(), expectedObjectCount );

  QFile file( QString( "%1/%2.obj" ).arg( QDir::tempPath(), objFileName ) );
  QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
  QTextStream fileStream( &file );

  // check the generated obj file
  QGSCOMPARELONGSTR( testName.toStdString().c_str(), u"%1.obj"_s.arg( testName ), fileStream.readAll().toUtf8() );
}

void TestQgs3DExporter::test3DSceneExporter()
{
  QgsVectorLayer *layerPoly = new QgsVectorLayer( testDataPath( u"/3d/polygons.gpkg.gz"_s ), u"polygons"_s, u"ogr"_s );
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

  do3DSceneExport( u"scene_export"_s, 1, nbFeat, nbFaces, scene, layerPoly, &engine );

  delete scene;
  mapSettings.setLayers( {} );
}

void TestQgs3DExporter::test3DSceneExporterBig()
{
  QgsRasterLayer *layerDtm = new QgsRasterLayer( testDataPath( u"/3d/dtm.tif"_s ), u"dtm"_s, u"gdal"_s );
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

  do3DSceneExport( u"big_scene_export"_s, 1, nbFeat, nbFaces, scene, mLayerBuildings, &engine );

  delete scene;
  mapSettings.setLayers( {} );
}

void TestQgs3DExporter::test3DSceneExporterFlatTerrain()
{
  QgsRasterLayer *layerRgb = new QgsRasterLayer( testDataPath( u"/3d/rgb.tif"_s ), u"rgb"_s, u"gdal"_s );
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

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 1500, 40.0, -10.0 );

  do3DSceneExport( u"flat_terrain_scene_export"_s, 2, 401, 19875, scene, mLayerBuildings, &engine, scene->terrainEntity() );

  delete scene;
  mapSettings.setLayers( {} );
}

void TestQgs3DExporter::test3DSceneExporterInstanced()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  auto layerPointsZ = std::make_unique<QgsVectorLayer>( "PointZ?crs=EPSG:27700", "points Z", "memory" );

  QgsPoint *p1 = new QgsPoint( 1000, 1000, 50 );
  QgsPoint *p2 = new QgsPoint( 1000, 2000, 100 );

  QgsFeature f1( layerPointsZ->fields() );
  QgsFeature f2( layerPointsZ->fields() );

  f1.setGeometry( QgsGeometry( p1 ) );
  f2.setGeometry( QgsGeometry( p2 ) );

  QgsFeatureList featureList;
  featureList << f1 << f2;
  layerPointsZ->dataProvider()->addFeatures( featureList );

  QgsPoint3DSymbol *plane3DSymbol = new QgsPoint3DSymbol();
  plane3DSymbol->setShape( Qgis::Point3DShape::Plane );
  QVariantMap vmPlane;
  vmPlane[u"size"_s] = 100.0f;
  plane3DSymbol->setShapeProperties( vmPlane );
  QgsPhongMaterialSettings materialSettings;
  materialSettings.setAmbient( Qt::blue );
  plane3DSymbol->setMaterialSettings( materialSettings.clone() );

  layerPointsZ->setRenderer3D( new QgsVectorLayer3DRenderer( plane3DSymbol ) );

  Qgs3DMapSettings mapSettings;
  mapSettings.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 27700 ) );
  mapSettings.setExtent( fullExtent );
  mapSettings.setLayers( QList<QgsMapLayer *>() << layerPointsZ.get() );
  mapSettings.setOutputDpi( 92 );

  QPoint winSize = QPoint( 640, 480 ); // default window size

  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( mapSettings, &engine );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 7000, 20.0, -10.0 );
  engine.setRootEntity( scene );

  do3DSceneExport( u"instanced_export"_s, 2, 0, 12, scene, layerPointsZ.get(), &engine );

  delete scene;
  mapSettings.setLayers( {} );
}

void TestQgs3DExporter::test3DSceneExporterInstancedTrs()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  auto layerPointsZ = std::make_unique<QgsVectorLayer>( "PointZ?crs=EPSG:27700", "points Z", "memory" );

  QgsPoint *p1 = new QgsPoint( 1000, 1000, 50 );
  QgsPoint *p2 = new QgsPoint( 1000, 2000, 100 );

  QgsFeature f1( layerPointsZ->fields() );
  QgsFeature f2( layerPointsZ->fields() );

  f1.setGeometry( QgsGeometry( p1 ) );
  f2.setGeometry( QgsGeometry( p2 ) );

  QgsFeatureList featureList;
  featureList << f1 << f2;
  layerPointsZ->dataProvider()->addFeatures( featureList );

  QgsPoint3DSymbol *cube3DSymbol = new QgsPoint3DSymbol();
  cube3DSymbol->setShape( Qgis::Point3DShape::Cube );
  QVariantMap vmCube;
  vmCube[u"size"_s] = 100.0f;
  cube3DSymbol->setShapeProperties( vmCube );

  QMatrix4x4 trsTransform;
  trsTransform.translate( 550, 150, 300 );
  trsTransform.scale( 1.5, 0.5, 2.3 );
  trsTransform.rotate( QQuaternion::fromEulerAngles( 20, 40, 15 ) );
  cube3DSymbol->setTransform( trsTransform );

  QgsPhongMaterialSettings materialSettings;
  materialSettings.setAmbient( Qt::blue );
  cube3DSymbol->setMaterialSettings( materialSettings.clone() );

  layerPointsZ->setRenderer3D( new QgsVectorLayer3DRenderer( cube3DSymbol ) );

  Qgs3DMapSettings mapSettings;
  mapSettings.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 27700 ) );
  mapSettings.setExtent( fullExtent );
  mapSettings.setLayers( QList<QgsMapLayer *>() << layerPointsZ.get() );
  mapSettings.setOutputDpi( 92 );

  QPoint winSize = QPoint( 640, 480 ); // default window size

  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( mapSettings, &engine );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 7000, 20.0, -10.0 );
  engine.setRootEntity( scene );

  do3DSceneExport( u"instanced_trs_export"_s, 2, 0, 72, scene, layerPointsZ.get(), &engine );

  delete scene;
  mapSettings.setLayers( {} );
}

void TestQgs3DExporter::test3DSceneExporterInstancedDataDefinedTrs()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  auto layerPointsZ = std::make_unique<QgsVectorLayer>( "PointZ?crs=EPSG:27700&field=field1:int&field=field2:int", "points Z", "memory" );

  QgsPoint *p1 = new QgsPoint( 1000, 1000, 50 );
  QgsPoint *p2 = new QgsPoint( 1000, 2000, 100 );
  QgsPoint *p3 = new QgsPoint( 2000, 1000, 75 );

  QgsFeature f1( layerPointsZ->fields() );
  QgsFeature f2( layerPointsZ->fields() );
  QgsFeature f3( layerPointsZ->fields() );

  f1.setGeometry( QgsGeometry( p1 ) );
  f1.setAttributes( QgsAttributes() << 1 << 2 );
  f2.setGeometry( QgsGeometry( p2 ) );
  f2.setAttributes( QgsAttributes() << 10 << 2 );
  f3.setGeometry( QgsGeometry( p3 ) );
  f3.setAttributes( QgsAttributes() << 1 << 20 );

  QgsFeatureList featureList;
  featureList << f1 << f2 << f3;
  layerPointsZ->dataProvider()->addFeatures( featureList );

  QgsPoint3DSymbol *cube3DSymbol = new QgsPoint3DSymbol();
  cube3DSymbol->setShape( Qgis::Point3DShape::Cube );
  QVariantMap vmCube;
  vmCube[u"size"_s] = 100.0f;
  cube3DSymbol->setShapeProperties( vmCube );

  QgsPropertyCollection ddProps;
  ddProps.setProperty( QgsAbstract3DSymbol::Property::ScaleX, QgsProperty::fromExpression( u"case when \"field1\" = 1 then .75 end"_s ) );
  ddProps.setProperty( QgsAbstract3DSymbol::Property::ScaleY, QgsProperty::fromExpression( u"case when \"field2\" = 2 then .5 end"_s ) );
  ddProps.setProperty( QgsAbstract3DSymbol::Property::ScaleZ, QgsProperty::fromExpression( u"case when \"field1\" = 10 then .3 end"_s ) );
  ddProps.setProperty( QgsAbstract3DSymbol::Property::TranslationX, QgsProperty::fromExpression( u"case when \"field1\" = 1 then -200 end"_s ) );
  ddProps.setProperty( QgsAbstract3DSymbol::Property::TranslationY, QgsProperty::fromExpression( u"case when \"field2\" = 2 then -100 end"_s ) );
  ddProps.setProperty( QgsAbstract3DSymbol::Property::TranslationZ, QgsProperty::fromExpression( u"case when \"field2\" = 20 then 50 end"_s ) );
  ddProps.setProperty( QgsAbstract3DSymbol::Property::RotationX, QgsProperty::fromExpression( u"case when \"field1\" = 1 then 5 end"_s ) );
  ddProps.setProperty( QgsAbstract3DSymbol::Property::RotationY, QgsProperty::fromExpression( u"case when \"field2\" = 2 then -20 end"_s ) );
  ddProps.setProperty( QgsAbstract3DSymbol::Property::RotationZ, QgsProperty::fromExpression( u"case when \"field2\" = 20 then 45 end"_s ) );
  cube3DSymbol->setDataDefinedProperties( ddProps );

  QMatrix4x4 trsTransform;
  trsTransform.translate( 550, 150, 300 );
  trsTransform.scale( 1.5, 0.5, 2.3 );
  trsTransform.rotate( QQuaternion::fromEulerAngles( 20, 40, 15 ) );
  cube3DSymbol->setTransform( trsTransform );

  QgsPhongMaterialSettings materialSettings;
  materialSettings.setAmbient( Qt::blue );
  cube3DSymbol->setMaterialSettings( materialSettings.clone() );

  layerPointsZ->setRenderer3D( new QgsVectorLayer3DRenderer( cube3DSymbol ) );

  Qgs3DMapSettings mapSettings;
  mapSettings.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 27700 ) );
  mapSettings.setExtent( fullExtent );
  mapSettings.setLayers( QList<QgsMapLayer *>() << layerPointsZ.get() );
  mapSettings.setOutputDpi( 92 );

  QPoint winSize = QPoint( 640, 480 ); // default window size

  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( mapSettings, &engine );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 7000, 20.0, -10.0 );
  engine.setRootEntity( scene );

  do3DSceneExport( u"instanced_trs_dd_export"_s, 3, 0, 108, scene, layerPointsZ.get(), &engine );

  delete scene;
  mapSettings.setLayers( {} );
}

QGSTEST_MAIN( TestQgs3DExporter )
#include "testqgs3dexporter.moc"
