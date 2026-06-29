/***************************************************************************
  testqgs3dmaterialrendering.cpp
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Martin Dobias
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
#include "qgs3dhighlightfeaturehandler.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3drendercontext.h"
#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgscameracontroller.h"
#include "qgsdemterrainsettings.h"
#include "qgsdirectionallightsettings.h"
#include "qgsflatterraingenerator.h"
#include "qgsflatterrainsettings.h"
#include "qgsframegraph.h"
#include "qgsgoochmaterialsettings.h"
#include "qgsline3dsymbol.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmapthemecollection.h"
#include "qgsmetalroughmaterialsettings.h"
#include "qgsmetalroughtexturedmaterialsettings.h"
#include "qgsoffscreen3dengine.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgspoint3dsymbol.h"
#include "qgspointlightsettings.h"
#include "qgspolygon3dsymbol.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrastershader.h"
#include "qgssettingsentryenumflag.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgssymbollayer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"

#include <QDir>
#include <QFileInfo>
#include <QSignalSpy>
#include <QString>
#include <Qt3DRender/QGeometryRenderer>

using namespace Qt::StringLiterals;

class TestQgs3DMaterialRendering : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3DMaterialRendering()
      : QgsTest( u"3D Material Rendering Tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testPhongShading();
    void testExtrudedPolygonsTexturedPhong();
    void testPhongTexturedDataDefined_data();
    void testPhongTexturedDataDefined();
    void testExtrudedPolygonsDataDefinedPhong();
    void testExtrudedPolygonsDataDefinedPhongClipping();
    void testExtrudedPolygonsDataDefinedGooch();
    void testExtrudedPolygonsDataDefinedGoochClipping();
    void testExtrudedPolygonsGoochShading();
    void testExtrudedPolygonsMetalRoughShading();
    void testExtrudedPolygonsMetalRoughShadingOpacity();
    void testExtrudedPolygonsMetalRoughShadingEmission();
    void testExtrudedPolygonsMetalRoughTexturedShading();
    void testExtrudedPolygonsMetalRoughTexturedShadingNormals();
    void testExtrudedPolygonsMetalRoughTexturedShadingEmission();
    void testExtrudedPolygonsMetalRoughTexturedShadingDisplacement();
    void testExtrudedPolygonsMetalRoughTexturedShadingOpacity();
    void testMetalRoughTexturedDataDefined_data();
    void testMetalRoughTexturedDataDefined();
    void testExtrudedPolygonsDataDefinedMetalRoughBase();
    void testExtrudedPolygonsDataDefinedMetalRoughEmission();
    void testMetalRoughEnvironmentLight_data();
    void testMetalRoughEnvironmentLight();

  private:
    QImage convertDepthImageToGrayscaleImage( const QImage &depthImage );

    std::unique_ptr<QgsProject> mProject;
    QgsRasterLayer *mLayerDtm = nullptr;
    QgsRasterLayer *mLayerRgb = nullptr;
    QgsVectorLayer *mLayerBuildings = nullptr;
};

QImage TestQgs3DMaterialRendering::convertDepthImageToGrayscaleImage( const QImage &depthImage )
{
  const int width = depthImage.width();
  const int height = depthImage.height();
  QImage grayImage( width, height, QImage::Format_ARGB32 );

  // compute image min/max depth values
  double minV = 9999999.0;
  double maxV = -9999999.0;
  for ( int y = 0; y < height; ++y )
  {
    const QRgb *depthImageScanline = reinterpret_cast<const QRgb *>( depthImage.scanLine( y ) );
    for ( int x = 0; x < width; ++x )
    {
      const double d = Qgs3DUtils::decodeDepth( depthImageScanline[x] );
      if ( d > maxV )
        maxV = d;
      if ( d < minV )
        minV = d;
    }
  }

  // transform depth value to gray value
  const double factor = ( maxV > minV ) ? 255.0 / ( maxV - minV ) : 1.0;
  for ( int y = 0; y < height; ++y )
  {
    const QRgb *depthImageScanline = reinterpret_cast<const QRgb *>( depthImage.scanLine( y ) );
    QRgb *grayImageScanline = reinterpret_cast<QRgb *>( grayImage.scanLine( y ) );
    for ( int x = 0; x < width; ++x )
    {
      const double d = Qgs3DUtils::decodeDepth( depthImageScanline[x] );
      unsigned short v = static_cast<unsigned short>( factor * ( d - minV ) );
      grayImageScanline[x] = qRgb( v, v, v );
    }
  }

  return grayImage;
}

// runs before all tests
void TestQgs3DMaterialRendering::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QSurfaceFormat format;
  format.setRenderableType( QSurfaceFormat::OpenGL );
#ifdef Q_OS_MACOS
  format.setVersion( 4, 1 ); //OpenGL is deprecated on MacOS, use last supported version
  format.setProfile( QSurfaceFormat::CoreProfile );
#else
  format.setVersion( 4, 3 );
  format.setProfile( QSurfaceFormat::CompatibilityProfile );
#endif
  format.setDepthBufferSize( 24 );
  format.setSamples( 4 );
  format.setStencilBufferSize( 8 );
  QSurfaceFormat::setDefaultFormat( format );

  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();
  Qgs3D::settingTextureFilterQuality->setValue( Qgis::TextureFilterQuality::Trilinear );

  mProject = std::make_unique<QgsProject>();

  mLayerDtm = new QgsRasterLayer( testDataPath( "/3d/dtm.tif" ), "dtm", "gdal" );
  QVERIFY( mLayerDtm->isValid() );
  mProject->addMapLayer( mLayerDtm );

  mLayerRgb = new QgsRasterLayer( testDataPath( "/3d/rgb.tif" ), "rgb", "gdal" );
  QVERIFY( mLayerRgb->isValid() );
  mProject->addMapLayer( mLayerRgb );

  mLayerBuildings = new QgsVectorLayer( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( mLayerBuildings->isValid() );
  mProject->addMapLayer( mLayerBuildings );

  // best to keep buildings without 2D renderer so it is not painted on the terrain
  // so we do not get some possible artifacts
  mLayerBuildings->setRenderer( nullptr );

  QgsPhongMaterialSettings materialSettings;
  materialSettings.setAmbient( Qt::lightGray );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  mProject->setCrs( mLayerDtm->crs() );

  //
  // prepare styles for DTM layer
  //

  mLayerDtm->styleManager()->addStyleFromLayer( "grayscale" );

  double vMin = 44, vMax = 198;
  QColor cMin = Qt::red, cMax = Qt::yellow;

  // change renderer for the new style
  auto colorRampShader = std::make_unique<QgsColorRampShader>( vMin, vMax );
  colorRampShader->setColorRampItemList( QList<QgsColorRampShader::ColorRampItem>() << QgsColorRampShader::ColorRampItem( vMin, cMin ) << QgsColorRampShader::ColorRampItem( vMax, cMax ) );
  auto shader = std::make_unique<QgsRasterShader>( vMin, vMax );
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
void TestQgs3DMaterialRendering::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgs3DMaterialRendering::testPhongShading()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsPhongMaterialSettings materialSettings;
  materialSettings.setAmbient( QColor( 0, 100, 0 ) );
  materialSettings.setDiffuse( QColor( 255, 0, 255 ) );
  materialSettings.setSpecular( QColor( 0, 255, 255 ) );
  materialSettings.setShininess( 2 );
  materialSettings.setAmbientCoefficient( 0.3 );
  materialSettings.setDiffuseCoefficient( 0.8 );
  materialSettings.setSpecularCoefficient( 0.7 );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 300, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "phong_shading", "phong_shading", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsTexturedPhong()
{
  QgsPhongTexturedMaterialSettings materialSettings;
  materialSettings.setAmbient( QColor( 26, 26, 26 ) );
  materialSettings.setSpecular( QColor( 10, 10, 10 ) );
  materialSettings.setShininess( 1.0 );
  materialSettings.setDiffuseTexturePath( testDataPath( "/sample_image.png" ) );
  materialSettings.setTextureScale( 0.05 );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings << mLayerRgb );
  QgsDirectionalLightSettings directionalLight;
  directionalLight.setDirection( QgsVector3D( 0.32, 0.27, -0.91 ) );
  map->setLightSources( { directionalLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 60, 60, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  delete scene;
  delete map;
  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_textured_phong", "polygon3d_extrusion_textured_phong", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testPhongTexturedDataDefined_data()
{
  QTest::addColumn<QgsProperty>( "scale" );
  QTest::addColumn<QgsProperty>( "rotation" );
  QTest::addColumn<QgsProperty>( "offset" );
  QTest::addColumn<QString>( "reference" );

  QTest::newRow( "dd scale" ) << QgsProperty::fromExpression( "case when ogc_fid =29204 then 400 else 800 end" ) << QgsProperty() << QgsProperty() << "phong_dd_texture_scale";
  QTest::newRow( "dd rotation" ) << QgsProperty() << QgsProperty::fromExpression( "case when ogc_fid =29204 then 45 else 90 end" ) << QgsProperty() << "phong_dd_texture_rotation";
  QTest::newRow( "dd offset" ) << QgsProperty() << QgsProperty() << QgsProperty::fromExpression( "case when ogc_fid =29204 then '10,20' else '2,-3' end" ) << "phong_dd_texture_offset";
  QTest::newRow( "dd scale, rotation, offset" )
    << QgsProperty::fromExpression( "case when ogc_fid =29204 then 400 else 800 end" )
    << QgsProperty::fromExpression( "case when ogc_fid =29204 then 45 else 90 end" )
    << QgsProperty::fromExpression( "case when ogc_fid =29204 then '10,20' else '15,10' end" )
    << "phong_dd_texture_scale_rotation_offset";
}

void TestQgs3DMaterialRendering::testPhongTexturedDataDefined()
{
  QFETCH( QgsProperty, scale );
  QFETCH( QgsProperty, rotation );
  QFETCH( QgsProperty, offset );
  QFETCH( QString, reference );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsPhongTexturedMaterialSettings materialSettings;
  materialSettings.setAmbient( QColor( 26, 26, 26 ) );
  materialSettings.setSpecular( QColor( 10, 10, 10 ) );
  materialSettings.setShininess( 1.0 );
  materialSettings.setDiffuseTexturePath( testDataPath( "/sample_image.png" ) );
  materialSettings.setTextureScale( 0.02 );
  QgsPropertyCollection props = materialSettings.dataDefinedProperties();
  if ( scale.isActive() )
    props.setProperty( QgsAbstractMaterialSettings::Property::TextureScale, scale );
  if ( rotation.isActive() )
    props.setProperty( QgsAbstractMaterialSettings::Property::TextureRotation, rotation );
  if ( offset.isActive() )
    props.setProperty( QgsAbstractMaterialSettings::Property::TextureOffset, offset );
  materialSettings.setDataDefinedProperties( props );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 30, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( reference, reference, img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsDataDefinedPhong()
{
  QgsPropertyCollection propertyColection;
  QgsProperty diffuseColor;
  QgsProperty ambientColor;
  QgsProperty specularColor;
  diffuseColor.setExpressionString( u"color_rgb( 120*(\"ogc_fid\"%3),125,0)"_s );
  ambientColor.setExpressionString( u"color_rgb( 120,(\"ogc_fid\"%2)*255,0)"_s );
  specularColor.setExpressionString( u"'yellow'"_s );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Diffuse, diffuseColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Ambient, ambientColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Specular, specularColor );
  QgsPhongMaterialSettings materialSettings;
  materialSettings.setDataDefinedProperties( propertyColection );
  materialSettings.setAmbient( Qt::red );
  materialSettings.setShininess( 1 );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings << mLayerRgb );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 500, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  delete scene;
  delete map;
  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_phong", "polygon3d_extrusion_data_defined_phong", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsDataDefinedPhongClipping()
{
  QgsPropertyCollection propertyColection;
  QgsProperty diffuseColor;
  QgsProperty ambientColor;
  QgsProperty specularColor;
  diffuseColor.setExpressionString( u"color_rgb( 120*(\"ogc_fid\"%3),125,0)"_s );
  ambientColor.setExpressionString( u"color_rgb( 120,(\"ogc_fid\"%2)*255,0)"_s );
  specularColor.setExpressionString( u"'yellow'"_s );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Diffuse, diffuseColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Ambient, ambientColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Specular, specularColor );
  QgsPhongMaterialSettings materialSettings;
  materialSettings.setDataDefinedProperties( propertyColection );
  materialSettings.setAmbient( Qt::red );
  materialSettings.setShininess( 1 );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings << mLayerRgb );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 500, 45, 0 );

  // First, without clipping
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img_no_clipping = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_phong", "polygon3d_extrusion_data_defined_phong", img_no_clipping, QString(), 40, QSize( 0, 0 ), 2 );

  // Enable clipping
  const QList<QVector4D> clipPlanesEquations
    = QList<QVector4D>() << QVector4D( 0.866025, -0.5, 0, 150.0 ) << QVector4D( -0.866025, 0.5, 0, 150.0 ) << QVector4D( 0.5, 0.866025, 0, 305.0 ) << QVector4D( -0.5, -0.866025, 0, 205.0 );
  scene->enableClipping( clipPlanesEquations );
  QImage img_clipping = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_phong_clipping", "polygon3d_extrusion_data_defined_phong_clipping", img_clipping, QString(), 40, QSize( 0, 0 ), 2 );

  // Disable clipping
  scene->disableClipping();

  QImage img_no_clipping_again = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_phong", "polygon3d_extrusion_data_defined_phong", img_no_clipping_again, QString(), 40, QSize( 0, 0 ), 2 );

  // Enable clipping a second time
  scene->enableClipping( clipPlanesEquations );

  QImage img_clipping_again = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_phong_clipping", "polygon3d_extrusion_data_defined_phong_clipping", img_clipping_again, QString(), 40, QSize( 0, 0 ), 2 );

  // Disable clipping again
  scene->disableClipping();

  QImage img_no_clipping_final = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;
  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_phong", "polygon3d_extrusion_data_defined_phong", img_no_clipping_final, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsDataDefinedGooch()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  QgsPropertyCollection propertyColection;
  QgsProperty diffuseColor;
  QgsProperty warmColor;
  QgsProperty coolColor;
  diffuseColor.setExpressionString( u"color_rgb( 120*(\"ogc_fid\"%3),125,0)"_s );
  warmColor.setExpressionString( u"color_rgb( 120,(\"ogc_fid\"%2)*255,0)"_s );
  coolColor.setExpressionString( u"'yellow'"_s );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Diffuse, diffuseColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Warm, warmColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Cool, coolColor );
  QgsGoochMaterialSettings materialSettings;
  materialSettings.setDataDefinedProperties( propertyColection );
  materialSettings.setAlpha( 0.2f );
  materialSettings.setBeta( 0.6f );
  materialSettings.setShininess( 2 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mProject->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerBuildings << mLayerRgb );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( mapSettings->origin() + QgsVector3D( 0, 0, 1000 ) );
  mapSettings->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), mProject->transformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 500, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  delete scene;
  delete mapSettings;
  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_gooch", "polygon3d_extrusion_data_defined_gooch", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsDataDefinedGoochClipping()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  QgsPropertyCollection propertyColection;
  QgsProperty diffuseColor;
  QgsProperty warmColor;
  QgsProperty coolColor;
  diffuseColor.setExpressionString( u"color_rgb( 120*(\"ogc_fid\"%3),125,0)"_s );
  warmColor.setExpressionString( u"color_rgb( 120,(\"ogc_fid\"%2)*255,0)"_s );
  coolColor.setExpressionString( u"'yellow'"_s );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Diffuse, diffuseColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Warm, warmColor );
  propertyColection.setProperty( QgsAbstractMaterialSettings::Property::Cool, coolColor );
  QgsGoochMaterialSettings materialSettings;
  materialSettings.setDataDefinedProperties( propertyColection );
  materialSettings.setAlpha( 0.2f );
  materialSettings.setBeta( 0.6f );
  materialSettings.setShininess( 2 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mProject->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerBuildings << mLayerRgb );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( mapSettings->origin() + QgsVector3D( 0, 0, 1000 ) );
  mapSettings->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), mProject->transformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 500, 45, 0 );

  // First, without clipping
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img_no_clipping = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_gooch", "polygon3d_extrusion_data_defined_gooch", img_no_clipping, QString(), 40, QSize( 0, 0 ), 2 );

  // Enable clipping
  const QList<QVector4D> clipPlanesEquations
    = QList<QVector4D>() << QVector4D( 0.866025, -0.5, 0, 150.0 ) << QVector4D( -0.866025, 0.5, 0, 150.0 ) << QVector4D( 0.5, 0.866025, 0, 305.0 ) << QVector4D( -0.5, -0.866025, 0, 205.0 );
  scene->enableClipping( clipPlanesEquations );

  QImage img_clipping = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_gooch_clipping", "polygon3d_extrusion_data_defined_gooch_clipping", img_clipping, QString(), 40, QSize( 0, 0 ), 2 );

  // Disable clipping
  scene->disableClipping();
  QImage img_no_clipping_again = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_gooch", "polygon3d_extrusion_data_defined_gooch", img_no_clipping_again, QString(), 40, QSize( 0, 0 ), 2 );

  // Enable clipping a second time
  scene->enableClipping( clipPlanesEquations );

  QImage img_clipping_again = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_gooch_clipping", "polygon3d_extrusion_data_defined_gooch_clipping", img_clipping_again, QString(), 40, QSize( 0, 0 ), 2 );

  // Disable clipping again
  scene->disableClipping();
  QImage img_no_clipping_final = Qgs3DUtils::captureSceneImage( engine, scene );

  delete scene;
  delete mapSettings;
  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_data_defined_gooch", "polygon3d_extrusion_data_defined_gooch", img_no_clipping_final, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsGoochShading()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );

  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings << mLayerRgb );

  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsGoochMaterialSettings materialSettings;
  materialSettings.setWarm( QColor( 224, 224, 17 ) );
  materialSettings.setCool( QColor( 21, 187, 235 ) );
  materialSettings.setAlpha( 0.2f );
  materialSettings.setBeta( 0.6f );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3dOpacity = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3dOpacity );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 500, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_gooch_shading", "polygon3d_extrusion_gooch_shading", img, QString(), 50, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsMetalRoughShading()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughMaterialSettings materialSettings;
  materialSettings.setBaseColor( QColor( 255, 0, 255 ) );
  materialSettings.setMetalness( 0.5 );
  materialSettings.setRoughness( 0.3 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 300, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "metal_rough", "metal_rough", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsMetalRoughShadingOpacity()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughMaterialSettings materialSettings;
  materialSettings.setBaseColor( QColor( 255, 0, 255 ) );
  materialSettings.setMetalness( 0 );
  materialSettings.setRoughness( 0.6 );
  materialSettings.setOpacity( 0.7 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() << mLayerRgb );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 100, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "metal_rough_opacity", "metal_rough_opacity", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsMetalRoughShadingEmission()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughMaterialSettings materialSettings;
  materialSettings.setBaseColor( QColor( 255, 0, 255 ) );
  materialSettings.setMetalness( 0.5 );
  materialSettings.setRoughness( 0.3 );
  materialSettings.setEmissionColor( QColor( 0, 255, 0 ) );
  materialSettings.setEmissionFactor( 2.2 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.1 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 300, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "metal_rough_emission", "metal_rough_emission", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsMetalRoughTexturedShading()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughTexturedMaterialSettings materialSettings;
  materialSettings.setBaseColorTexturePath( testDataPath( "/3d/materials/Metal005_Color.jpg" ) );
  materialSettings.setMetalnessTexturePath( testDataPath( "/3d/materials/Metal005_Metalness.jpg" ) );
  materialSettings.setRoughnessTexturePath( testDataPath( "/3d/materials/Metal005_Roughness.jpg" ) );
  materialSettings.setTextureScale( 0.02 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 30, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_textured_metalrough", "polygon3d_extrusion_textured_metalrough", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsMetalRoughTexturedShadingEmission()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughTexturedMaterialSettings materialSettings;
  materialSettings.setBaseColorTexturePath( testDataPath( "/3d/materials/Metal005_Color.jpg" ) );
  materialSettings.setMetalnessTexturePath( testDataPath( "/3d/materials/Metal005_Metalness.jpg" ) );
  materialSettings.setRoughnessTexturePath( testDataPath( "/3d/materials/Metal005_Roughness.jpg" ) );
  materialSettings.setEmissionTexturePath( testDataPath( "/3d/materials/Metal005_Emission.jpg" ) );
  materialSettings.setTextureScale( 0.02 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  // pull the light down low
  defaultLight.setIntensity( 0.1 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 30, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_textured_metalrough_emission", "polygon3d_extrusion_textured_metalrough_emission", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsMetalRoughTexturedShadingDisplacement()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughTexturedMaterialSettings materialSettings;
  materialSettings.setBaseColorTexturePath( testDataPath( "/3d/materials/Metal005_Gradient.jpg" ) );
  materialSettings.setHeightTexturePath( testDataPath( "/3d/materials/Metal005_Displacement.jpg" ) );
  materialSettings.setParallaxScale( 0.3 );
  materialSettings.setTextureScale( 0.05 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 1.5 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1500 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 20, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_textured_metalrough_displacement1", "polygon3d_extrusion_textured_metalrough_displacement1", img, QString(), 40, QSize( 0, 0 ), 2 );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 20, 45, 45 );

  Qgs3DUtils::captureSceneImage( engine, scene );
  img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_textured_metalrough_displacement2", "polygon3d_extrusion_textured_metalrough_displacement2", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsMetalRoughTexturedShadingOpacity()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughTexturedMaterialSettings materialSettings;
  materialSettings.setBaseColorTexturePath( testDataPath( "/3d/materials/Metal005_Color.jpg" ) );
  materialSettings.setMetalnessTexturePath( testDataPath( "/3d/materials/Metal005_Metalness.jpg" ) );
  materialSettings.setRoughnessTexturePath( testDataPath( "/3d/materials/Metal005_Roughness.jpg" ) );
  materialSettings.setEmissionTexturePath( testDataPath( "/3d/materials/Metal005_Emission.jpg" ) );
  materialSettings.setTextureScale( 0.02 );
  materialSettings.setOpacity( 0.95 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() << mLayerRgb );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 30, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_textured_metalrough_opacity", "polygon3d_extrusion_textured_metalrough_opacity", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testMetalRoughTexturedDataDefined_data()
{
  QTest::addColumn<QgsProperty>( "scale" );
  QTest::addColumn<QgsProperty>( "rotation" );
  QTest::addColumn<QgsProperty>( "offset" );
  QTest::addColumn<QString>( "reference" );

  QTest::newRow( "dd scale" ) << QgsProperty::fromExpression( "case when ogc_fid =29204 then 400 else 800 end" ) << QgsProperty() << QgsProperty() << "metalrough_dd_texture_scale";
  QTest::newRow( "dd rotation" ) << QgsProperty() << QgsProperty::fromExpression( "case when ogc_fid =29204 then 45 else 90 end" ) << QgsProperty() << "metalrough_dd_texture_rotation";
  QTest::newRow( "dd offset" ) << QgsProperty() << QgsProperty() << QgsProperty::fromExpression( "case when ogc_fid =29204 then '10,20' else '2,-3' end" ) << "metalrough_dd_texture_offset";
  QTest::newRow( "dd scale, rotation, offset" )
    << QgsProperty::fromExpression( "case when ogc_fid =29204 then 400 else 800 end" )
    << QgsProperty::fromExpression( "case when ogc_fid =29204 then 45 else 90 end" )
    << QgsProperty::fromExpression( "case when ogc_fid =29204 then '10,20' else '15,10' end" )
    << "metalrough_dd_texture_scale_rotation_offset";
}

void TestQgs3DMaterialRendering::testMetalRoughTexturedDataDefined()
{
  QFETCH( QgsProperty, scale );
  QFETCH( QgsProperty, rotation );
  QFETCH( QgsProperty, offset );
  QFETCH( QString, reference );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughTexturedMaterialSettings materialSettings;
  materialSettings.setBaseColorTexturePath( testDataPath( "/sample_image.png" ) );
  materialSettings.setTextureScale( 0.02 );
  QgsPropertyCollection props = materialSettings.dataDefinedProperties();
  if ( scale.isActive() )
    props.setProperty( QgsAbstractMaterialSettings::Property::TextureScale, scale );
  if ( rotation.isActive() )
    props.setProperty( QgsAbstractMaterialSettings::Property::TextureRotation, rotation );
  if ( offset.isActive() )
    props.setProperty( QgsAbstractMaterialSettings::Property::TextureOffset, offset );
  materialSettings.setDataDefinedProperties( props );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 30, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( reference, reference, img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsDataDefinedMetalRoughBase()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughMaterialSettings materialSettings;
  materialSettings.setBaseColor( QColor( 255, 0, 255 ) );
  materialSettings.setMetalness( 0.5 );
  materialSettings.setRoughness( 0.3 );

  QgsPropertyCollection propertyCollection;
  QgsProperty baseColor;
  baseColor.setExpressionString( u"color_rgb( 120*(\"ogc_fid\"%3),125,0)"_s );
  propertyCollection.setProperty( QgsAbstractMaterialSettings::Property::BaseColor, baseColor );
  materialSettings.setDataDefinedProperties( propertyCollection );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 300, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "metal_rough_dd_base", "metal_rough_dd_base", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsDataDefinedMetalRoughEmission()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughMaterialSettings materialSettings;
  materialSettings.setBaseColor( QColor( 255, 0, 255 ) );
  materialSettings.setMetalness( 0.0 );
  materialSettings.setRoughness( 1.0 );
  materialSettings.setEmissionFactor( 150 );

  QgsPropertyCollection propertyCollection;
  QgsProperty emissionColor;
  emissionColor.setExpressionString( u"color_rgb( 120*(\"ogc_fid\"%3),125,0)"_s );
  propertyCollection.setProperty( QgsAbstractMaterialSettings::Property::EmissionColor, emissionColor );
  materialSettings.setDataDefinedProperties( propertyCollection );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, -250, 0 ), 300, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "metal_rough_dd_emission", "metal_rough_dd_emission", img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testMetalRoughEnvironmentLight_data()
{
  QTest::addColumn<double>( "metalness" );
  QTest::addColumn<double>( "roughness" );
  QTest::addColumn<QColor>( "baseColor" );
  QTest::addColumn<double>( "reflectance" );
  QTest::addColumn<double>( "anisotropy" );
  QTest::addColumn<double>( "anisotropyRotation" );
  QTest::addColumn<double>( "clearCoatFactor" );
  QTest::addColumn<double>( "clearCoatRoughness" );
  QTest::addColumn<double>( "strength" );
  QTest::addColumn<QString>( "reference" );

  QTest::newRow( "dielectric smooth" ) << 0.0 << 0.0 << QColor( 200, 255, 200 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_dielectric_smooth";
  QTest::newRow( "dielectric 30% rough" ) << 0.0 << 0.3 << QColor( 200, 255, 200 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_dielectric_30_rough";
  QTest::newRow( "dielectric 60% rough" ) << 0.0 << 0.6 << QColor( 200, 255, 200 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_dielectric_60_rough";
  QTest::newRow( "dielectric 100% rough" ) << 0.0 << 1.0 << QColor( 200, 255, 200 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_dielectric_100_rough";
  QTest::newRow( "metal smooth" ) << 1.0 << 0.0 << QColor( 200, 255, 200 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_metal_smooth";
  QTest::newRow( "metal 30% rough" ) << 1.0 << 0.3 << QColor( 200, 255, 200 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_metal_30_rough";
  QTest::newRow( "metal 60% rough" ) << 1.0 << 0.6 << QColor( 200, 255, 200 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_metal_60_rough";
  QTest::newRow( "metal 100% rough" ) << 1.0 << 1.0 << QColor( 200, 255, 200 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_metal_100_rough";
  QTest::newRow( "dielectric smooth dark" ) << 0.0 << 0.0 << QColor( 30, 30, 30 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_dielectric_smooth_dark";
  QTest::newRow( "dielectric smooth white" ) << 0.0 << 0.0 << QColor( 230, 230, 230 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_dielectric_smooth_light";
  QTest::newRow( "metal smooth dark" ) << 1.0 << 0.0 << QColor( 30, 30, 30 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_metal_smooth_dark";
  QTest::newRow( "metal smooth white" ) << 1.0 << 0.0 << QColor( 230, 230, 230 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_metal_smooth_light";
  QTest::newRow( "50% strength" ) << 1.0 << 0.0 << QColor( 230, 230, 230 ) << 0.5 << 0.0 << 0.0 << 0.0 << 0.0 << 0.5 << "env_light_mid_strength";

  QTest::newRow( "dielectric reflectance 0.3 (low reflectance))" ) << 0.0 << 0.0 << QColor( 200, 0, 0 ) << 0.3 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_low_reflectance";
  QTest::newRow( "dielectric reflectance 0.7 (gemstone)" ) << 0.0 << 0.0 << QColor( 200, 0, 0 ) << 0.7 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_mid_reflectance";
  QTest::newRow( "dielectric reflectance 1.0 (gemstone)" ) << 0.0 << 0.0 << QColor( 200, 0, 0 ) << 1.0 << 0.0 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_high_reflectance";

  QTest::newRow( "smooth metal anisotropy 0.4" ) << 1.0 << 0.0 << QColor( 100, 100, 100 ) << 0.5 << 0.4 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_smooth_metal_anisotropy_mid";
  QTest::newRow( "smooth metal anisotropy 0.8" ) << 1.0 << 0.0 << QColor( 100, 100, 100 ) << 0.5 << 0.8 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_smooth_metal_anisotropy_high";
  QTest::newRow( "60% rough metal anisotropy 0.8" ) << 1.0 << 0.6 << QColor( 100, 100, 100 ) << 0.5 << 0.8 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_rough_metal_anisotropy_high";
  QTest::newRow( "smooth dielectric anisotropy 0.4" ) << 0.0 << 0.0 << QColor( 100, 100, 100 ) << 0.5 << 0.4 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_smooth_dielectric_anisotropy_mid";
  QTest::newRow( "smooth dielectric anisotropy 0.8" ) << 0.0 << 0.0 << QColor( 100, 100, 100 ) << 0.5 << 0.8 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_smooth_dielectric_anisotropy_high";
  QTest::newRow( "60% rough dielectric anisotropy 0.8" ) << 0.0 << 0.6 << QColor( 100, 100, 100 ) << 0.5 << 0.8 << 0.0 << 0.0 << 0.0 << 1.0 << "env_light_rough_dielectric_anisotropy_high";
  QTest::newRow( "60% rough metal anisotropy 0.8 rotated 45" ) << 1.0 << 0.6 << QColor( 100, 100, 100 ) << 0.5 << 0.8 << 45.0 << 0.0 << 0.0 << 1.0 << "env_light_anisotropy_rotated_45";
  QTest::newRow( "60% rough metal anisotropy 0.8 rotated 90" ) << 1.0 << 0.6 << QColor( 100, 100, 100 ) << 0.5 << 0.8 << 90.0 << 0.0 << 0.0 << 1.0 << "env_light_anisotropy_rotated_90";

  QTest::newRow( "metal clear coat 100" ) << 1.0 << 0.8 << QColor( 0, 0, 220 ) << 0.5 << 0.0 << 0.0 << 1.0 << 0.0 << 1.0 << "env_light_metal_clear_coat_100";
  QTest::newRow( "metal rough clear coat 100 " ) << 1.0 << 0.8 << QColor( 0, 0, 220 ) << 0.5 << 0.0 << 0.0 << 1.0 << 0.5 << 1.0 << "env_light_metal_rough_clear_coat_100";
  QTest::newRow( "dielectric clear coat 100" ) << 0.0 << 0.8 << QColor( 200, 0, 0 ) << 0.5 << 0.0 << 0.0 << 1.0 << 0.0 << 1.0 << "env_light_dielectric_clear_coat_100";
  QTest::newRow( "dielectric rough clear coat 100" ) << 0.0 << 0.8 << QColor( 200, 0, 0 ) << 0.5 << 0.0 << 0.0 << 1.0 << 0.5 << 1.0 << "env_light_dielectric_rough_clear_coat_100";
}

void TestQgs3DMaterialRendering::testMetalRoughEnvironmentLight()
{
  QFETCH( double, metalness );
  QFETCH( double, roughness );
  QFETCH( QColor, baseColor );
  QFETCH( double, reflectance );
  QFETCH( double, anisotropy );
  QFETCH( double, anisotropyRotation );
  QFETCH( double, clearCoatFactor );
  QFETCH( double, clearCoatRoughness );
  QFETCH( double, strength );
  QFETCH( QString, reference );

  const QgsRectangle fullExtent( -100, -100, 100, 100 );

  auto layerPointsZ = std::make_unique<QgsVectorLayer>( "PointZ?crs=EPSG:3857&field=field1:int&field=field2:int&field=field3:int", "points Z", "memory" );

  QgsPoint *p1 = new QgsPoint( 0, 0, 0 );

  QgsFeature f1( layerPointsZ->fields() );

  f1.setAttributes( QgsAttributes() << 1 << 2 << 3 );
  f1.setGeometry( QgsGeometry( p1 ) );

  QgsFeatureList featureList;
  featureList << f1;
  layerPointsZ->dataProvider()->addFeatures( featureList );

  QgsPoint3DSymbol *symbol = new QgsPoint3DSymbol();
  symbol->setShape( Qgis::Point3DShape::Sphere );

  QVariantMap props;
  props[u"slices"_s] = 64;
  props[u"rings"_s] = 64;

  symbol->setShapeProperties( props );
  QgsMetalRoughMaterialSettings materialSettings;
  materialSettings.setMetalness( metalness );
  materialSettings.setRoughness( roughness );
  materialSettings.setBaseColor( baseColor );
  materialSettings.setReflectance( reflectance );
  materialSettings.setAnisotropy( anisotropy );
  materialSettings.setAnisotropyRotation( anisotropyRotation );
  materialSettings.setClearCoatFactor( clearCoatFactor );
  materialSettings.setClearCoatRoughness( clearCoatRoughness );
  symbol->setMaterialSettings( materialSettings.clone() );

  layerPointsZ->setRenderer3D( new QgsVectorLayer3DRenderer( symbol ) );

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << layerPointsZ.get() );

  QgsDirectionalLightSettings directionalLight;
  directionalLight.setDirection( QgsVector3D( 0.32, 0.27, -0.91 ) );
  mapSettings->setLightSources( { directionalLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), mProject->transformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );
  mapSettings->setTerrainRenderingEnabled( false );

  QgsSkyboxSettings skyboxSettings;
  skyboxSettings.setCubeMapping( Qgis::SkyboxCubeMapping::LeftHandedYUpMirrored );
  skyboxSettings.setCubeMapFace( u"posX"_s, testDataPath( "/3d/skybox/skybox_right.jpg" ) );
  skyboxSettings.setCubeMapFace( u"posY"_s, testDataPath( "/3d/skybox/skybox_front.jpg" ) );
  skyboxSettings.setCubeMapFace( u"posZ"_s, testDataPath( "/3d/skybox/skybox_up.jpg" ) );
  skyboxSettings.setCubeMapFace( u"negX"_s, testDataPath( "/3d/skybox/skybox_left.jpg" ) );
  skyboxSettings.setCubeMapFace( u"negY"_s, testDataPath( "/3d/skybox/skybox_back.jpg" ) );
  skyboxSettings.setCubeMapFace( u"negZ"_s, testDataPath( "/3d/skybox/skybox_down.jpg" ) );
  skyboxSettings.setEnvironmentalLightingEnabled( true );
  skyboxSettings.setEnvironmentalLightStrength( strength );
  mapSettings->setBackgroundSettings( skyboxSettings.clone() );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 30, 90, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( reference, reference, img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DMaterialRendering::testExtrudedPolygonsMetalRoughTexturedShadingNormals()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  auto buildings = std::make_unique<QgsVectorLayer>( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( buildings->isValid() );

  QgsMetalRoughTexturedMaterialSettings materialSettings;
  materialSettings.setBaseColorTexturePath( testDataPath( "/3d/materials/Metal005_Color.jpg" ) );
  materialSettings.setMetalnessTexturePath( testDataPath( "/3d/materials/Metal005_Metalness.jpg" ) );
  materialSettings.setRoughnessTexturePath( testDataPath( "/3d/materials/Metal005_Roughness.jpg" ) );
  materialSettings.setNormalTexturePath( testDataPath( "/3d/materials/Metal005_Normal.jpg" ) );
  materialSettings.setTextureScale( 0.02 );

  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  buildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << buildings.get() );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.9 );
  defaultLight.setPosition( map->origin() + QgsVector3D( 0, 0, 1000 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( -60, -360, 10 ), 30, 45, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "polygon3d_extrusion_textured_metalrough_normals", "polygon3d_extrusion_textured_metalrough_normals", img, QString(), 40, QSize( 0, 0 ), 2 );
}

QGSTEST_MAIN( TestQgs3DMaterialRendering )
#include "testqgs3dmaterialrendering.moc"
