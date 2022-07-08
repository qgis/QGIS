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
#include "qgsmultirenderchecker.h"

#include "qgslinestring.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmapthemecollection.h"
#include "qgsmeshlayer.h"
#include "qgsmeshrenderersettings.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrastershader.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgs3d.h"

#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgschunknode_p.h"
#include "qgsdemterraingenerator.h"
#include "qgsflatterraingenerator.h"
#include "qgsmeshterraingenerator.h"
#include "qgsline3dsymbol.h"
#include "qgsoffscreen3dengine.h"
#include "qgspolygon3dsymbol.h"
#include "qgsrulebased3drenderer.h"
#include "qgsterrainentity_p.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgspoint3dsymbol.h"
#include "qgssymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgssimplelinematerialsettings.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"

#include <QFileInfo>
#include <QDir>
#include <QDesktopWidget>

class TestQgs3DRendering : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testFlatTerrain();
    void testDemTerrain();
    void testTerrainShading();
    void testMeshTerrain();
    void testEpsg4978LineRendering();
    void testExtrudedPolygons();
    void testExtrudedPolygonsDataDefined();
    void testPolygonsEdges();
    void testLineRendering();
    void testLineRenderingCurved();
    void testBufferedLineRendering();
    void testBufferedLineRenderingWidth();
    void testMapTheme();
    void testMesh();
    void testMesh_datasetOnFaces();
    void testMeshSimplified();
    void testRuleBasedRenderer();
    void testAnimationExport();
    void testBillboardRendering();

  private:
    // color tolerance < 2 was failing polygon3d_extrusion test
    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0, int colorTolerance = 2 );

    QString mReport;

    std::unique_ptr<QgsProject> mProject;
    QgsRasterLayer *mLayerDtm;
    QgsRasterLayer *mLayerRgb;
    QgsVectorLayer *mLayerBuildings;
    QgsMeshLayer *mLayerMeshTerrain;
    QgsMeshLayer *mLayerMeshDataset;
    QgsMeshLayer *mLayerMeshSimplified;

};

//runs before all tests
void TestQgs3DRendering::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  mReport = QStringLiteral( "<h1>3D Rendering Tests</h1>\n" );

  mProject.reset( new QgsProject );

  const QString dataDir( TEST_DATA_DIR );
  mLayerDtm = new QgsRasterLayer( dataDir + "/3d/dtm.tif", "dtm", "gdal" );
  QVERIFY( mLayerDtm->isValid() );
  mProject->addMapLayer( mLayerDtm );

  mLayerRgb = new QgsRasterLayer( dataDir + "/3d/rgb.tif", "rgb", "gdal" );
  QVERIFY( mLayerRgb->isValid() );
  mProject->addMapLayer( mLayerRgb );

  mLayerBuildings = new QgsVectorLayer( dataDir + "/3d/buildings.shp", "buildings", "ogr" );
  QVERIFY( mLayerBuildings->isValid() );
  mProject->addMapLayer( mLayerBuildings );

  // best to keep buildings without 2D renderer so it is not painted on the terrain
  // so we do not get some possible artifacts
  mLayerBuildings->setRenderer( nullptr );

  QgsPhongMaterialSettings material;
  material.setAmbient( Qt::lightGray );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterial( material.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  mLayerMeshTerrain = new QgsMeshLayer( dataDir + "/mesh/quad_flower.2dm", "mesh", "mdal" );
  QVERIFY( mLayerMeshTerrain->isValid() );
  mLayerMeshTerrain->setCrs( mLayerDtm->crs() );  // this testing mesh does not have any CRS defined originally
  mProject->addMapLayer( mLayerMeshTerrain );

  mLayerMeshDataset = new QgsMeshLayer( dataDir + "/mesh/quad_and_triangle.2dm", "mesh", "mdal" );
  mLayerMeshDataset->dataProvider()->addDataset( dataDir + "/mesh/quad_and_triangle_vertex_scalar.dat" );
  mLayerMeshDataset->dataProvider()->addDataset( dataDir + "/mesh/quad_and_triangle_vertex_vector.dat" );
  mLayerMeshDataset->dataProvider()->addDataset( dataDir + "/mesh/quad_and_triangle_els_face_scalar.dat" );
  QVERIFY( mLayerMeshDataset->isValid() );
  mLayerMeshDataset->setCrs( mLayerDtm->crs() ); // this testing mesh does not have any CRS defined originally
  mLayerMeshDataset->temporalProperties()->setIsActive( false );
  mLayerMeshDataset->setStaticScalarDatasetIndex( QgsMeshDatasetIndex( 1, 0 ) );
  mLayerMeshDataset->setStaticVectorDatasetIndex( QgsMeshDatasetIndex( 2, 0 ) );
  mProject->addMapLayer( mLayerMeshDataset );

  mLayerMeshSimplified = new QgsMeshLayer( dataDir + "/mesh/trap_steady_05_3D.nc", "mesh", "mdal" );
  mLayerMeshSimplified->setCrs( mProject->crs() );
  QVERIFY( mLayerMeshSimplified->isValid() );
  mProject->addMapLayer( mLayerMeshSimplified );

  QgsMesh3DSymbol *symbolMesh3d = new QgsMesh3DSymbol;
  symbolMesh3d->setVerticalDatasetGroupIndex( 0 );
  symbolMesh3d->setVerticalScale( 10 );
  symbolMesh3d->setRenderingStyle( QgsMesh3DSymbol::ColorRamp2DRendering );
  symbolMesh3d->setArrowsEnabled( true );
  symbolMesh3d->setArrowsSpacing( 300 );
  QgsMeshLayer3DRenderer *meshDatasetRenderer3d = new QgsMeshLayer3DRenderer( symbolMesh3d );
  mLayerMeshDataset->setRenderer3D( meshDatasetRenderer3d );

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

  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  const QgsRectangle fullExtent = mLayerDtm->extent();

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

  // change camera lens field of view
  map->setFieldOfView( 85.0f );
  QImage img4 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "flat_terrain_4", img4, 40 ) );
}

void TestQgs3DRendering::testDemTerrain()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

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

void TestQgs3DRendering::testTerrainShading()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  // no terrain layers set!

  QgsDemTerrainGenerator *demTerrain = new QgsDemTerrainGenerator;
  demTerrain->setLayer( mLayerDtm );
  map->setTerrainGenerator( demTerrain );
  map->setTerrainVerticalScale( 3 );

  QgsPhongMaterialSettings terrainMaterial;
  terrainMaterial.setAmbient( QColor( 0, 0, 0 ) );
  terrainMaterial.setDiffuse( QColor( 255, 255, 0 ) );
  terrainMaterial.setSpecular( QColor( 255, 255, 255 ) );
  map->setTerrainShadingMaterial( terrainMaterial );
  map->setTerrainShadingEnabled( true );

  QgsPointLightSettings defaultLight;
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  defaultLight.setIntensity( 0.5 );
  map->setLightSources( {defaultLight.clone() } );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2000, 60, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img3 = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "shaded_terrain_no_layers", img3, 40 ) );
}

void TestQgs3DRendering::testMeshTerrain()
{
  if ( !QgsTest::runFlakyTests() )
    QSKIP( "This test is flaky and disabled on CI" );

  const QgsRectangle fullExtent = mLayerMeshTerrain->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );

  QgsMeshTerrainGenerator *meshTerrain = new QgsMeshTerrainGenerator;
  meshTerrain->setCrs( mProject->crs(), mProject->transformContext() );
  meshTerrain->setLayer( mLayerMeshTerrain );
  map->setTerrainGenerator( meshTerrain );

  QCOMPARE( meshTerrain->heightAt( 750, 2500, *map ), 500.0 );

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
  QVERIFY( renderCheck( "mesh_terrain_1", img, 40 ) );
}

void TestQgs3DRendering::testExtrudedPolygons()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings << mLayerRgb );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( {defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 250 ), 500, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "polygon3d_extrusion", img, 40 ) );
}

void TestQgs3DRendering::testExtrudedPolygonsDataDefined()
{
  QgsPropertyCollection propertyColection;
  QgsProperty diffuseColor;
  QgsProperty ambientColor;
  QgsProperty specularColor;
  diffuseColor.setExpressionString( QStringLiteral( "color_rgb( 120*(\"ogc_fid\"%3),125,0)" ) );
  ambientColor.setExpressionString( QStringLiteral( "color_rgb( 120,(\"ogc_fid\"%2)*255,0)" ) );
  specularColor.setExpressionString( QStringLiteral( "'yellow'" ) );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Diffuse, diffuseColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Ambient, ambientColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Specular, specularColor );
  QgsPhongMaterialSettings material;
  material.setDataDefinedProperties( propertyColection );
  material.setAmbient( Qt::red );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterial( material.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings << mLayerRgb );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 250 ), 500, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "polygon3d_extrusion_data_defined", img, 40 ) );
}

void TestQgs3DRendering::testPolygonsEdges()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );

  QgsPhongMaterialSettings material;
  material.setAmbient( Qt::lightGray );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterial( material.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  symbol3d->setHeight( 20.f );
  symbol3d->setEdgesEnabled( true );
  symbol3d->setEdgeWidth( 8 );
  symbol3d->setEdgeColor( QColor( 255, 0, 0 ) );

  std::unique_ptr< QgsVectorLayer > layer( mLayerBuildings->clone() );

  std::unique_ptr< QgsSimpleFillSymbolLayer > simpleFill = std::make_unique< QgsSimpleFillSymbolLayer >( QColor( 0, 0, 0 ), Qt::SolidPattern, QColor( 0, 0, 0 ), Qt::NoPen );
  std::unique_ptr< QgsFillSymbol > fillSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList() << simpleFill.release() );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol.release() ) );

  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  layer->setRenderer3D( renderer3d );

  map->setLayers( QList<QgsMapLayer *>() << layer.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 250 ), 100, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "polygon_edges_height", img, 40 ) );
}

void TestQgs3DRendering::testLineRendering()
{
  const QgsRectangle fullExtent( 0, 0, 1000, 1000 );

  QgsVectorLayer *layerLines = new QgsVectorLayer( "LineString?crs=EPSG:27700", "lines", "memory" );

  QgsLine3DSymbol *lineSymbol = new QgsLine3DSymbol;
  lineSymbol->setRenderAsSimpleLines( true );
  lineSymbol->setWidth( 10 );
  QgsSimpleLineMaterialSettings mat;
  mat.setAmbient( Qt::red );
  lineSymbol->setMaterial( mat.clone() );
  layerLines->setRenderer3D( new QgsVectorLayer3DRenderer( lineSymbol ) );

  QVector<QgsPoint> pts;
  pts << QgsPoint( 0, 0, 10 ) << QgsPoint( 0, 1000, 10 ) << QgsPoint( 1000, 1000, 10 ) << QgsPoint( 1000, 0, 10 );
  pts << QgsPoint( 1000, 0, 500 ) << QgsPoint( 1000, 1000, 500 ) << QgsPoint( 0, 1000, 500 ) << QgsPoint( 0, 0, 500 );
  QgsFeature f1( layerLines->fields() );
  f1.setGeometry( QgsGeometry( new QgsLineString( pts ) ) );
  QgsFeatureList flist;
  flist << f1;
  layerLines->dataProvider()->addFeatures( flist );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << layerLines );

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
  QVERIFY( renderCheck( "line_rendering_1", img, 40 ) );

  // more perspective look
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 45 );

  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "line_rendering_2", img2, 40 ) );

  delete layerLines;
}

void TestQgs3DRendering::testLineRenderingCurved()
{
  // test rendering of compound curve lines works
  const QgsRectangle fullExtent( 0, 0, 1000, 1000 );

  QgsVectorLayer *layerLines = new QgsVectorLayer( "CompoundCurve?crs=EPSG:27700", "lines", "memory" );

  QgsLine3DSymbol *lineSymbol = new QgsLine3DSymbol;
  lineSymbol->setRenderAsSimpleLines( true );
  lineSymbol->setWidth( 10 );
  QgsSimpleLineMaterialSettings mat;
  mat.setAmbient( Qt::red );
  lineSymbol->setMaterial( mat.clone() );
  layerLines->setRenderer3D( new QgsVectorLayer3DRenderer( lineSymbol ) );

  QVector<QgsPoint> pts;
  pts << QgsPoint( 0, 0, 10 ) << QgsPoint( 0, 1000, 10 ) << QgsPoint( 1000, 1000, 10 ) << QgsPoint( 1000, 0, 10 );
  std::unique_ptr< QgsCompoundCurve > curve = std::make_unique< QgsCompoundCurve >();
  curve->addCurve( new QgsLineString( pts ) );
  pts.clear();
  pts << QgsPoint( 1000, 0, 10 ) << QgsPoint( 1000, 0, 500 ) << QgsPoint( 1000, 1000, 500 ) << QgsPoint( 0, 1000, 500 ) << QgsPoint( 0, 0, 500 );
  curve->addCurve( new QgsLineString( pts ) );

  QgsFeature f1( layerLines->fields() );
  f1.setGeometry( QgsGeometry( std::move( curve ) ) );
  QgsFeatureList flist;
  flist << f1;
  layerLines->dataProvider()->addFeatures( flist );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << layerLines );

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
  QVERIFY( renderCheck( "line_rendering_1", img, 40 ) );

  delete layerLines;
}

void TestQgs3DRendering::testBufferedLineRendering()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  QgsVectorLayer *layerLines = new QgsVectorLayer( QString( TEST_DATA_DIR ) + "/3d/lines.shp", "lines", "ogr" );
  QVERIFY( layerLines->isValid() );

  QgsLine3DSymbol *lineSymbol = new QgsLine3DSymbol;
  lineSymbol->setWidth( 10 );
  lineSymbol->setExtrusionHeight( 30 );
  QgsPhongMaterialSettings mat;
  mat.setAmbient( Qt::red );
  lineSymbol->setMaterial( mat.clone() );
  layerLines->setRenderer3D( new QgsVectorLayer3DRenderer( lineSymbol ) );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << layerLines );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 300, 0, 250 ), 500, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "buffered_lines", img, 40 ) );

  delete layerLines;
}

void TestQgs3DRendering::testBufferedLineRenderingWidth()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  QgsVectorLayer *layerLines = new QgsVectorLayer( QString( TEST_DATA_DIR ) + "/3d/lines.shp", "lines", "ogr" );
  QVERIFY( layerLines->isValid() );

  QgsLine3DSymbol *lineSymbol = new QgsLine3DSymbol;
  lineSymbol->setWidth( 20 );
  lineSymbol->setExtrusionHeight( 30 );
  lineSymbol->setHeight( 10 );
  QgsPhongMaterialSettings mat;
  mat.setAmbient( Qt::red );
  lineSymbol->setMaterial( mat.clone() );
  layerLines->setRenderer3D( new QgsVectorLayer3DRenderer( lineSymbol ) );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << layerLines );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 300, 0, 250 ), 500, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "buffered_lines_width", img, 40 ) );

  delete layerLines;
}

void TestQgs3DRendering::testMapTheme()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerDtm );

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
  const QgsRectangle fullExtent = mLayerMeshDataset->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerMeshDataset );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 3000, 25, 45 );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "mesh3d", img, 40 ) );
}

void TestQgs3DRendering::testMesh_datasetOnFaces()
{
  const QgsRectangle fullExtent = mLayerMeshDataset->extent();

  QgsMesh3DSymbol *symbolMesh3d = new QgsMesh3DSymbol;
  symbolMesh3d->setVerticalDatasetGroupIndex( 3 );
  symbolMesh3d->setVerticalScale( 10 );
  symbolMesh3d->setRenderingStyle( QgsMesh3DSymbol::ColorRamp2DRendering );
  symbolMesh3d->setArrowsEnabled( true );
  symbolMesh3d->setArrowsSpacing( 300 );
  QgsMeshLayer3DRenderer *meshDatasetRenderer3d = new QgsMeshLayer3DRenderer( symbolMesh3d );
  mLayerMeshDataset->setRenderer3D( meshDatasetRenderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerMeshDataset );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 3000, 25, 45 );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "mesh3dOnFace", img, 40 ) );
}

void TestQgs3DRendering::testMeshSimplified()
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
    map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );

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

    QVERIFY( renderCheck( QString( "mesh_simplified_%1" ).arg( i ), img, 40 ) );
  }

}

void TestQgs3DRendering::testRuleBasedRenderer()
{
  QgsPhongMaterialSettings material;
  material.setAmbient( Qt::lightGray );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterial( material.clone() );
  symbol3d->setExtrusionHeight( 10.f );

  QgsPhongMaterialSettings material2;
  material2.setAmbient( Qt::red );
  QgsPolygon3DSymbol *symbol3d2 = new QgsPolygon3DSymbol;
  symbol3d2->setMaterial( material2.clone() );
  symbol3d2->setExtrusionHeight( 10.f );

  QgsRuleBased3DRenderer::Rule *root = new QgsRuleBased3DRenderer::Rule( nullptr );
  QgsRuleBased3DRenderer::Rule *rule1 = new QgsRuleBased3DRenderer::Rule( symbol3d, "ogc_fid < 29069", "rule 1" );
  QgsRuleBased3DRenderer::Rule *rule2 = new QgsRuleBased3DRenderer::Rule( symbol3d2, "ogc_fid >= 29069", "rule 2" );
  root->appendChild( rule1 );
  root->appendChild( rule2 );
  QgsRuleBased3DRenderer *renderer3d = new QgsRuleBased3DRenderer( root );
  mLayerBuildings->setRenderer3D( renderer3d );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings );

  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 250 ), 500, 45, 0 );

  // When running the test, it would sometimes return partially rendered image.
  // It is probably based on how fast qt3d manages to upload the data to GPU...
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "rulebased", img, 40 ) );
}

bool TestQgs3DRendering::renderCheck( const QString &testName, QImage &image, int mismatchCount, int colorTolerance )
{
  mReport += "<h2>" + testName + "</h2>\n";
  const QString myTmpDir = QDir::tempPath() + '/';
  const QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "3d" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  myChecker.setColorTolerance( colorTolerance );
  const bool myResultFlag = myChecker.runTest( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgs3DRendering::testAnimationExport()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings map;
  map.setCrs( mProject->crs() );
  map.setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map.crs() );
  flatTerrain->setExtent( fullExtent );
  map.setTerrainGenerator( flatTerrain );

  Qgs3DAnimationSettings animSettings;
  Qgs3DAnimationSettings::Keyframes keyframes;
  Qgs3DAnimationSettings::Keyframe kf1;
  kf1.dist = 2500;
  Qgs3DAnimationSettings::Keyframe kf2;
  kf2.time = 2;
  kf2.dist = 3000;
  keyframes << kf1;
  keyframes << kf2;
  animSettings.setKeyframes( keyframes );

  const QString dir = QDir::temp().path();
  QString error;

  const bool success = Qgs3DUtils::exportAnimation(
                         animSettings,
                         map,
                         1,
                         dir,
                         "test3danimation###.png",
                         QSize( 600, 400 ),
                         error,
                         nullptr );

  QVERIFY( success );
  QVERIFY( QFileInfo::exists( ( QDir( dir ).filePath( QStringLiteral( "test3danimation001.png" ) ) ) ) );
}

void TestQgs3DRendering::testBillboardRendering()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  std::unique_ptr<QgsVectorLayer> layerPointsZ( new QgsVectorLayer( "PointZ?crs=EPSG:27700", "points Z", "memory" ) );

  QgsPoint *p1 = new QgsPoint( 1000, 1000, 50 );
  QgsPoint *p2 = new QgsPoint( 1000, 2000, 100 );
  QgsPoint *p3 = new QgsPoint( 2000, 2000, 200 );

  QgsFeature f1( layerPointsZ->fields() );
  QgsFeature f2( layerPointsZ->fields() );
  QgsFeature f3( layerPointsZ->fields() );

  f1.setGeometry( QgsGeometry( p1 ) );
  f2.setGeometry( QgsGeometry( p2 ) );
  f3.setGeometry( QgsGeometry( p3 ) );

  QgsFeatureList featureList;
  featureList << f1 << f2 << f3;
  layerPointsZ->dataProvider()->addFeatures( featureList );

  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  markerSymbol->setColor( QColor( 255, 0, 0 ) );
  markerSymbol->setSize( 4 );
  QgsSimpleMarkerSymbolLayer *sl = static_cast<QgsSimpleMarkerSymbolLayer *>( markerSymbol->symbolLayer( 0 ) ) ;
  sl->setStrokeColor( QColor( 0, 0, 255 ) );
  sl->setStrokeWidth( 2 );
  QgsPoint3DSymbol *point3DSymbol = new QgsPoint3DSymbol();
  point3DSymbol->setBillboardSymbol( markerSymbol );
  point3DSymbol->setShape( QgsPoint3DSymbol::Billboard );

  layerPointsZ->setRenderer3D( new QgsVectorLayer3DRenderer( point3DSymbol ) );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << layerPointsZ.get() );

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
  QVERIFY( renderCheck( "billboard_rendering_1", img, 40 ) );

  // more perspective look
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 45 );

  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "billboard_rendering_2", img2, 40 ) );
}

void TestQgs3DRendering::testEpsg4978LineRendering()
{
  const QgsRectangle fullExtent( 0, 0, 1000, 1000 );

  QgsCoordinateReferenceSystem oldCrs = mProject->crs();
  QgsCoordinateReferenceSystem newCrs;
  newCrs.createFromString( "EPSG:4978" );
  mProject->setCrs( newCrs );

  QgsVectorLayer *layerLines = new QgsVectorLayer( QString( TEST_DATA_DIR ) + "/3d/earth_size_sphere_4978.gpkg", "lines", "ogr" );

  QgsLine3DSymbol *lineSymbol = new QgsLine3DSymbol;
  lineSymbol->setRenderAsSimpleLines( true );
  lineSymbol->setWidth( 2 );
  QgsSimpleLineMaterialSettings mat;
  mat.setAmbient( Qt::red );
  lineSymbol->setMaterial( mat.clone() );
  layerLines->setRenderer3D( new QgsVectorLayer3DRenderer( lineSymbol ) );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << layerLines );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 1.5e7, 0, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "4978_line_rendering_1", img, 40, 15 ) );

  // more perspective look
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 1.5e7, 45, 45 );

  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "4978_line_rendering_2", img2, 40, 15 ) );

  delete layerLines;
  mProject->setCrs( oldCrs );
}

QGSTEST_MAIN( TestQgs3DRendering )
#include "testqgs3drendering.moc"
