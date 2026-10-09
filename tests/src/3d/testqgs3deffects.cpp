/***************************************************************************
  testqgs3deffects.cpp
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
#include "qgsmetalroughmaterialsettings.h"
#include "qgsmetalroughtexturedmaterialsettings.h"
#include "qgsoffscreen3dengine.h"
#include "qgspolygon3dsymbol.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgssettingsentryenumflag.h"
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

class TestQgs3DEffects : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3DEffects()
      : QgsTest( u"3D Effects Tests"_s, u"3d_effects"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testBloom_data();
    void testBloom();
    void testColorCorrection_data();
    void testColorCorrection();

  private:
    std::unique_ptr<QgsProject> mProject;
    QgsRasterLayer *mLayerDtm = nullptr;
    QgsRasterLayer *mLayerRgb = nullptr;
    QgsVectorLayer *mLayerBuildings = nullptr;
};

// runs before all tests
void TestQgs3DEffects::initTestCase()
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

  mProject->setCrs( mLayerDtm->crs() );
}

//runs after all tests
void TestQgs3DEffects::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgs3DEffects::testBloom_data()
{
  QTest::addColumn<float>( "emissionStrength" );
  QTest::addColumn<double>( "intensity" );
  QTest::addColumn<double>( "radius" );
  QTest::addColumn<QString>( "reference" );

  QTest::newRow( "emission 30" ) << 30.0f << 0.05 << 0.005 << "bloom1";
  QTest::newRow( "emission 50" ) << 50.0f << 0.05 << 0.005 << "bloom2";
  QTest::newRow( "intensity 0.1" ) << 30.0f << 0.1 << 0.005 << "bloom3";
  QTest::newRow( "radius 0.002" ) << 30.0f << 0.05 << 0.002 << "bloom4";
}

void TestQgs3DEffects::testBloom()
{
  const float lightAzimuth = 210.f;
  const float lightAltitude = 30.f;
  const QgsVector3D point( 0, -250, 0 );
  const float distance( 100.0f );
  const float pitch( 75.0f );
  const float yaw( 130.0f );
  QFETCH( float, emissionStrength );
  QFETCH( double, intensity );
  QFETCH( double, radius );

  QFETCH( QString, reference );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  QgsMetalRoughTexturedMaterialSettings materialSettings;
  materialSettings.setEmissionTexturePath( testDataPath( "/3d/materials/Metal005_Emission.jpg" ) );
  materialSettings.setEmissionFactor( emissionStrength );
  materialSettings.setTextureScale( 0.02 );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings );
  map->setBackgroundColor( Qt::black );
  QgsDirectionalLightSettings defaultLight;

  const double horizontalVectorMagnitude = cos( lightAltitude / 180 * M_PI );
  QgsVector3D lightDirection( -horizontalVectorMagnitude * sin( lightAzimuth / 180 * M_PI ), -horizontalVectorMagnitude * cos( lightAzimuth / 180 * M_PI ), -sin( lightAltitude / 180 * M_PI ) );
  defaultLight.setDirection( lightDirection );
  map->setLightSources( { defaultLight.clone() } );

  QgsBloomSettings bloom = map->bloomSettings();
  bloom.setEnabled( true );
  bloom.setIntensity( intensity );
  bloom.setRadius( radius );
  map->setBloomSettings( bloom );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( point, distance, pitch, yaw );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( reference, reference, img, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DEffects::testColorCorrection_data()
{
  QTest::addColumn<double>( "exposure" );
  QTest::addColumn<Qgis::ToneMappingMethod>( "toneMapping" );
  QTest::addColumn<QString>( "reference" );

  QTest::newRow( "increase exposure" ) << 2.0 << Qgis::ToneMappingMethod::Clamp << "exposure_increase";
  QTest::newRow( "decrease exposure" ) << -2.0 << Qgis::ToneMappingMethod::Clamp << "exposure_decrease";
  QTest::newRow( "aces mapping" ) << 0.0 << Qgis::ToneMappingMethod::Aces << "aces";
}

void TestQgs3DEffects::testColorCorrection()
{
  const float lightAzimuth = 210.f;
  const float lightAltitude = 30.f;
  const QgsVector3D point( 0, -250, 0 );
  const float distance( 100.0f );
  const float pitch( 75.0f );
  const float yaw( -50.0f );
  QFETCH( double, exposure );
  QFETCH( Qgis::ToneMappingMethod, toneMapping );

  QFETCH( QString, reference );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  QgsMetalRoughMaterialSettings materialSettings;
  materialSettings.setBaseColor( QColor( 255, 200, 0 ) );
  materialSettings.setRoughness( 1.0 );
  materialSettings.setMetalness( 0 );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings );
  map->setBackgroundColor( Qt::black );
  QgsDirectionalLightSettings defaultLight;
  defaultLight.setIntensity( 5 );

  QgsColorGradingSettings colorSettings = map->colorGradingSettings();
  colorSettings.setExposureAdjustment( exposure );
  colorSettings.setToneMapping( toneMapping );
  map->setColorGradingSettings( colorSettings );

  const double horizontalVectorMagnitude = cos( lightAltitude / 180 * M_PI );
  QgsVector3D lightDirection( -horizontalVectorMagnitude * sin( lightAzimuth / 180 * M_PI ), -horizontalVectorMagnitude * cos( lightAzimuth / 180 * M_PI ), -sin( lightAltitude / 180 * M_PI ) );
  defaultLight.setDirection( lightDirection );
  map->setLightSources( { defaultLight.clone() } );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( point, distance, pitch, yaw );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( reference, reference, img, QString(), 40, QSize( 0, 0 ), 2 );
}

QGSTEST_MAIN( TestQgs3DEffects )
#include "testqgs3deffects.moc"
