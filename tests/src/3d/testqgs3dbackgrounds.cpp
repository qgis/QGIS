/***************************************************************************
  testqgs3dbackgrounds.cpp
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
#include "qgsfixedgradientbackgroundsettings.h"
#include "qgsflatterraingenerator.h"
#include "qgsflatterrainsettings.h"
#include "qgsframegraph.h"
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

class TestQgs3DBackgrounds : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3DBackgrounds()
      : QgsTest( u"3D Background Tests"_s, u"3d_backgrounds"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testGradientBackground_data();
    void testGradientBackground();
    void testSkyboxRendering_data();
    void testSkyboxRendering();

  private:
    std::unique_ptr<QgsProject> mProject;
    QgsRasterLayer *mLayerDtm = nullptr;
    QgsRasterLayer *mLayerRgb = nullptr;
    QgsVectorLayer *mLayerBuildings = nullptr;
};

// runs before all tests
void TestQgs3DBackgrounds::initTestCase()
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
}

//runs after all tests
void TestQgs3DBackgrounds::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgs3DBackgrounds::testGradientBackground_data()
{
  QTest::addColumn<QColor>( "topColor" );
  QTest::addColumn<QColor>( "bottomColor" );
  QTest::addColumn<QString>( "reference" );

  QTest::newRow( "gradient 1" ) << QColor( 30, 120, 220 ) << QColor( 0, 0, 0 ) << "gradient_background";
  QTest::newRow( "gradient 2" ) << QColor( 220, 30, 30 ) << QColor( 255, 255, 255 ) << "gradient_background_2";
  QTest::newRow( "no background" ) << QColor() << QColor() << "buildings_no_background";
}

void TestQgs3DBackgrounds::testGradientBackground()
{
  QFETCH( QColor, topColor );
  QFETCH( QColor, bottomColor );
  QFETCH( QString, reference );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings );

  if ( topColor.isValid() )
  {
    auto *gradientBg = new QgsFixedGradientBackgroundSettings();
    gradientBg->setTopColor( topColor );
    gradientBg->setBottomColor( bottomColor );
    map->setBackgroundSettings( gradientBg );
  }
  else
  {
    map->setBackgroundSettings( nullptr );
  }

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 0 );

  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( reference, reference, img, QString(), 40, QSize( 0, 0 ), 2 );

  // passing null, the bg settings should clear any bg entity
  map->setBackgroundSettings( nullptr );
  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage img3 = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "buildings_no_background", "buildings_no_background", img3, QString(), 40, QSize( 0, 0 ), 2 );

  delete scene;
  delete map;
}

void TestQgs3DBackgrounds::testSkyboxRendering_data()
{
  QTest::addColumn<float>( "pitch" );
  QTest::addColumn<float>( "yaw" );
  QTest::addColumn<QString>( "reference" );

  QTest::newRow( "skybox 1" ) << 90.0f << 0.0f << "skybox1";
  QTest::newRow( "skybox 2" ) << 90.0f << 90.0f << "skybox2";
  QTest::newRow( "skybox 3" ) << 90.0f << 180.0f << "skybox3";
  QTest::newRow( "skybox 4" ) << 90.0f << 270.0f << "skybox4";
  QTest::newRow( "skybox 5" ) << 0.0f << 0.0f << "skybox5";
  QTest::newRow( "skybox 6" ) << 180.0f << 0.0f << "skybox6";
}

void TestQgs3DBackgrounds::testSkyboxRendering()
{
  QFETCH( float, pitch );
  QFETCH( float, yaw );
  QFETCH( QString, reference );

  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mProject->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setTerrainRenderingEnabled( false );

  QgsSkyboxSettings skyboxSettings;
  skyboxSettings.setCubeMapping( Qgis::SkyboxCubeMapping::LeftHandedYUpMirrored );
  skyboxSettings.setCubeMapFace( u"posX"_s, testDataPath( "/3d/skybox/skybox_right.jpg" ) );
  skyboxSettings.setCubeMapFace( u"posY"_s, testDataPath( "/3d/skybox/skybox_front.jpg" ) );
  skyboxSettings.setCubeMapFace( u"posZ"_s, testDataPath( "/3d/skybox/skybox_up.jpg" ) );
  skyboxSettings.setCubeMapFace( u"negX"_s, testDataPath( "/3d/skybox/skybox_left.jpg" ) );
  skyboxSettings.setCubeMapFace( u"negY"_s, testDataPath( "/3d/skybox/skybox_back.jpg" ) );
  skyboxSettings.setCubeMapFace( u"negZ"_s, testDataPath( "/3d/skybox/skybox_down.jpg" ) );
  mapSettings->setBackgroundSettings( skyboxSettings.clone() );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), mapSettings->transformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 10, pitch, yaw );

  Qgs3DUtils::captureSceneImage( engine, scene );
  QImage imgModel = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( reference, reference, imgModel, QString(), 80, QSize( 0, 0 ), 2 );

  delete scene;
  delete mapSettings;
}

QGSTEST_MAIN( TestQgs3DBackgrounds )
#include "testqgs3dbackgrounds.moc"
