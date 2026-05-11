/***************************************************************************
  testqgs3dshadows.cpp
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

class TestQgs3DShadows : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3DShadows()
      : QgsTest( u"3D Shadow Tests"_s, u"3d_shadows"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testFlatTerrain_data();
    void testFlatTerrain();

  private:
    std::unique_ptr<QgsProject> mProject;
    QgsRasterLayer *mLayerDtm = nullptr;
    QgsRasterLayer *mLayerRgb = nullptr;
    QgsVectorLayer *mLayerBuildings = nullptr;
};

// runs before all tests
void TestQgs3DShadows::initTestCase()
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

  QgsMetalRoughMaterialSettings materialSettings;
  materialSettings.setBaseColor( QColor( 200, 200, 255 ) );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  mProject->setCrs( mLayerDtm->crs() );
}

//runs after all tests
void TestQgs3DShadows::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgs3DShadows::testFlatTerrain_data()
{
  QTest::addColumn<float>( "lightAzimuth" );
  QTest::addColumn<float>( "lightAltitude" );
  QTest::addColumn<QgsVector3D>( "point" );
  QTest::addColumn<float>( "distance" );
  QTest::addColumn<float>( "pitch" );
  QTest::addColumn<float>( "yaw" );
  QTest::addColumn<QString>( "reference" );

  // keep directional light low, for prominent shadows
  QTest::newRow( "basic" ) << 210.0f << 30.0f << QgsVector3D( 0, -250, 0 ) << 100.0f << 75.0f << 130.0f << "shadows1";
  QTest::newRow( "lower light" ) << 170.0f << 10.0f << QgsVector3D( 0, -250, 0 ) << 100.0f << 75.0f << 130.0f << "shadows2";
  QTest::newRow( "very low light" ) << 225.0f << 40.0f << QgsVector3D( 0, -250, 0 ) << 100.0f << 85.0f << 130.0f << "shadows3";
  QTest::newRow( "very high light" ) << 225.0f << 70.0f << QgsVector3D( 0, -250, 0 ) << 100.0f << 85.0f << 130.0f << "shadows4";
  QTest::newRow( "zoomed out" ) << 210.0f << 30.0f << QgsVector3D( 0, -250, 0 ) << 300.0f << 85.0f << 130.0f << "shadows5";
  QTest::newRow( "zoomed out looking down" ) << 210.0f << 30.0f << QgsVector3D( 0, -250, 0 ) << 300.0f << 35.0f << 130.0f << "shadows6";
  QTest::newRow( "very zoomed out" ) << 210.0f << 30.0f << QgsVector3D( 0, -250, 0 ) << 900.0f << 75.0f << 130.0f << "shadows7";
  QTest::newRow( "very zoomed out other side" ) << 210.0f << 20.0f << QgsVector3D( 0, -250, 0 ) << 900.0f << 55.0f << 310.0f << "shadows8";
}

void TestQgs3DShadows::testFlatTerrain()
{
  QFETCH( float, lightAzimuth );
  QFETCH( float, lightAltitude );
  QFETCH( QgsVector3D, point );
  QFETCH( float, distance );
  QFETCH( float, pitch );
  QFETCH( float, yaw );
  QFETCH( QString, reference );

  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings );
  map->setBackgroundColor( Qt::white );
  QgsDirectionalLightSettings defaultLight;

  const double horizontalVectorMagnitude = cos( lightAltitude / 180 * M_PI );
  QgsVector3D lightDirection( -horizontalVectorMagnitude * sin( lightAzimuth / 180 * M_PI ), -horizontalVectorMagnitude * cos( lightAzimuth / 180 * M_PI ), -sin( lightAltitude / 180 * M_PI ) );
  defaultLight.setDirection( lightDirection );
  map->setLightSources( { defaultLight.clone() } );
  QgsShadowSettings shadow = map->shadowSettings();

  // render with the cascade splits visible so that we test these
  shadow.setShowCascadeSplits( true );
  shadow.setRenderShadows( true );
  shadow.setSelectedDirectionalLight( 0 );
  shadow.setShadowQuality( Qgis::ShadowQuality::High );
  map->setShadowSettings( shadow );

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

QGSTEST_MAIN( TestQgs3DShadows )
#include "testqgs3dshadows.moc"
