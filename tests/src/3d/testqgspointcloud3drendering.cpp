/***************************************************************************
  testqgspointcloud3drendering.cpp
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Stefanos Natsis
  Email                : uclaros at gmail dot com
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

#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgs3d.h"

#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgsoffscreen3dengine.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerrenderer.h"
#include "qgspointcloudextentrenderer.h"
#include "qgspointcloudattributebyramprenderer.h"
#include "qgspointcloudrgbrenderer.h"
#include "qgspointcloudclassifiedrenderer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspointcloud3dsymbol.h"
#include "qgsstyle.h"

#include <QFileInfo>
#include <QDir>

class TestQgsPointCloud3DRendering : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsPointCloud3DRendering() : QgsTest( QStringLiteral( "Point Cloud 3D Rendering Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testSync3DRendererTo2DRenderer();
    void testDisableSync3DRendererTo2DRenderer();

//    void testPointCloudSingleColor();
//    void testPointCloudAttributeByRamp();
//    void testPointCloudRgb();
//    void testPointCloudClassification();
//    void testPointCloudSyncedTo2D();


  private:
    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0 );

    std::unique_ptr<QgsProject> mProject;
    QgsPointCloudLayer *mLayer;

};

bool TestQgsPointCloud3DRendering::renderCheck( const QString &testName, QImage &image, int mismatchCount )
{
  const QString myTmpDir = QDir::tempPath() + '/';
  const QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "3d" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  myChecker.setColorTolerance( 2 );  // color tolerance < 2 was failing polygon3d_extrusion test
  const bool myResultFlag = myChecker.runTest( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

//runs before all tests
void TestQgsPointCloud3DRendering::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  mProject.reset( new QgsProject );

  const QString dataDir( TEST_DATA_DIR );

  mLayer = new QgsPointCloudLayer( dataDir + "/point_clouds/ept/rgb/ept.json", "test", "ept" );
  QVERIFY( mLayer->isValid() );
  mProject->addMapLayer( mLayer );
  mProject->setCrs( mLayer->crs() );

  // set a default 3D renderer
  QgsSingleColorPointCloud3DSymbol *symbol = new QgsSingleColorPointCloud3DSymbol();
  symbol->setSingleColor( QColor( 255, 0, 0 ) );
  QgsPointCloudLayer3DRenderer *renderer = new QgsPointCloudLayer3DRenderer();
  renderer->setSymbol( symbol );
  mLayer->setRenderer3D( renderer );
}

//runs after all tests
void TestQgsPointCloud3DRendering::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgsPointCloud3DRendering::testSync3DRendererTo2DRenderer()
{
  // gather different 2D renderers
  QgsPointCloudExtentRenderer *extent2DRenderer = new QgsPointCloudExtentRenderer();
  QgsPointCloudAttributeByRampRenderer *colorramp2DRenderer = new QgsPointCloudAttributeByRampRenderer();
  colorramp2DRenderer->setAttribute( QStringLiteral( "Z" ) );
  QgsColorRampShader shader = QgsColorRampShader( 0.98, 1.25 );
  shader.setSourceColorRamp( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  shader.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
  colorramp2DRenderer->setColorRampShader( shader );
  QgsPointCloudRgbRenderer *rgb2DRenderer = dynamic_cast<QgsPointCloudRgbRenderer *>( mLayer->renderer()->clone() );
  QgsPointCloudClassifiedRenderer *classification2DRenderer = new QgsPointCloudClassifiedRenderer();
  classification2DRenderer->setAttribute( QStringLiteral( "Classification" ) );
  auto categories = QgsPointCloudClassifiedRenderer::defaultCategories();
  // change a couple of categories
  categories[3].setRenderState( false );
  categories[3].setColor( QColor( 255, 0, 0 ) );
  categories[5].setRenderState( false );
  categories[5].setColor( QColor( 0, 255, 0 ) );
  classification2DRenderer->setCategories( categories );

  // enable syncing, the 3D renderer should change when the 2D renderer changes
  mLayer->setSync3DRendererTo2DRenderer( true );

  {
    // for the extent 2D renderer we should have a 3D renderer but no symbol
    mLayer->setRenderer( extent2DRenderer );
    QgsPointCloudLayer3DRenderer *r3D = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    QVERIFY( !r3D->symbol() );
  }
  {
    mLayer->setRenderer( colorramp2DRenderer );
    QgsPointCloudAttributeByRampRenderer *r2D = static_cast<QgsPointCloudAttributeByRampRenderer *>( mLayer->renderer() );
    QgsPointCloudLayer3DRenderer *r3D = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    const QgsColorRampPointCloud3DSymbol *s = dynamic_cast<const QgsColorRampPointCloud3DSymbol *>( r3D->symbol() );
    QVERIFY( s );
    QVERIFY( s->attribute() == r2D->attribute() );
    QVERIFY( s->colorRampShader() == r2D->colorRampShader() );
    QVERIFY( s->colorRampShaderMin() == r2D->minimum() );
    QVERIFY( s->colorRampShaderMax() == r2D->maximum() );
  }
  {
    mLayer->setRenderer( rgb2DRenderer );
    QgsPointCloudRgbRenderer *r2D = static_cast<QgsPointCloudRgbRenderer *>( mLayer->renderer() );
    QgsPointCloudLayer3DRenderer *r3D = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    QgsRgbPointCloud3DSymbol *s = const_cast<QgsRgbPointCloud3DSymbol *>( dynamic_cast<const QgsRgbPointCloud3DSymbol *>( r3D->symbol() ) );

    QVERIFY( s );
    QVERIFY( s->redAttribute() == r2D->redAttribute() );
    QVERIFY( s->blueAttribute() == r2D->blueAttribute() );
    QVERIFY( s->greenAttribute() == r2D->greenAttribute() );
    QVERIFY( s->redContrastEnhancement() == r2D->redContrastEnhancement() );
    QVERIFY( s->greenContrastEnhancement() == r2D->greenContrastEnhancement() );
    QVERIFY( s->blueContrastEnhancement() == r2D->blueContrastEnhancement() );
  }
  {
    mLayer->setRenderer( classification2DRenderer );
    QgsPointCloudClassifiedRenderer *r2D = static_cast<QgsPointCloudClassifiedRenderer *>( mLayer->renderer() );
    QgsPointCloudLayer3DRenderer *r3D = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    const QgsClassificationPointCloud3DSymbol *s = dynamic_cast<const QgsClassificationPointCloud3DSymbol *>( r3D->symbol() );
    QVERIFY( s );
    QVERIFY( s->attribute() == r2D->attribute() );
    QVERIFY( s->categoriesList() == r2D->categories() );
  }
}

void TestQgsPointCloud3DRendering::testDisableSync3DRendererTo2DRenderer()
{
  // gather different 2D renderers
  QgsPointCloudExtentRenderer *extent2DRenderer = new QgsPointCloudExtentRenderer();
  QgsPointCloudAttributeByRampRenderer *colorramp2DRenderer = new QgsPointCloudAttributeByRampRenderer();
  colorramp2DRenderer->setAttribute( QStringLiteral( "Z" ) );
  QgsColorRampShader shader = QgsColorRampShader( 0.98, 1.25 );
  shader.setSourceColorRamp( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  shader.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
  colorramp2DRenderer->setColorRampShader( shader );
  QgsPointCloudRgbRenderer *rgb2DRenderer = dynamic_cast<QgsPointCloudRgbRenderer *>( mLayer->renderer()->clone() );
  QgsPointCloudClassifiedRenderer *classification2DRenderer = new QgsPointCloudClassifiedRenderer();
  classification2DRenderer->setAttribute( QStringLiteral( "Classification" ) );
  auto categories = QgsPointCloudClassifiedRenderer::defaultCategories();
  // change a couple of categories
  categories[3].setRenderState( false );
  categories[3].setColor( QColor( 255, 0, 0 ) );
  categories[5].setRenderState( false );
  categories[5].setColor( QColor( 0, 255, 0 ) );
  classification2DRenderer->setCategories( categories );


  // enable syncing, the 3D renderer should change when the 2D renderer changes
  mLayer->setSync3DRendererTo2DRenderer( true );

  {
    // for the extent 2D renderer we should have a 3D renderer but no symbol
    mLayer->setRenderer( extent2DRenderer );
    QgsPointCloudLayer3DRenderer *r3D = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    QVERIFY( !r3D->symbol() );
  }
  {
    mLayer->setRenderer( colorramp2DRenderer );
    QgsPointCloudAttributeByRampRenderer *r2D = static_cast<QgsPointCloudAttributeByRampRenderer *>( mLayer->renderer() );
    QgsPointCloudLayer3DRenderer *r3D = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    const QgsColorRampPointCloud3DSymbol *s = dynamic_cast<const QgsColorRampPointCloud3DSymbol *>( r3D->symbol() );
    QVERIFY( s );
    QVERIFY( s->attribute() == r2D->attribute() );
    QVERIFY( s->colorRampShader() == r2D->colorRampShader() );
    QVERIFY( s->colorRampShaderMin() == r2D->minimum() );
    QVERIFY( s->colorRampShaderMax() == r2D->maximum() );
  }

  // now disable syncing and check that the 3D  renderer symbol does not change
  mLayer->setSync3DRendererTo2DRenderer( false );

  {
    mLayer->setRenderer( colorramp2DRenderer );
    QgsPointCloudLayer3DRenderer *r3D  = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    const QgsColorRampPointCloud3DSymbol *s = dynamic_cast<const QgsColorRampPointCloud3DSymbol *>( r3D ->symbol() );
    QVERIFY( s );
  }
  {
    mLayer->setRenderer( rgb2DRenderer );
    QgsPointCloudLayer3DRenderer *r3D  = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    const QgsColorRampPointCloud3DSymbol *s = dynamic_cast<const QgsColorRampPointCloud3DSymbol *>( r3D ->symbol() );
    QVERIFY( s );
  }
  {
    mLayer->setRenderer( classification2DRenderer );
    QgsPointCloudLayer3DRenderer *r3D  = static_cast<QgsPointCloudLayer3DRenderer *>( mLayer->renderer3D() );
    QVERIFY( r3D );
    const QgsColorRampPointCloud3DSymbol *s = dynamic_cast<const QgsColorRampPointCloud3DSymbol *>( r3D ->symbol() );
    QVERIFY( s );
  }
}
/*

void TestQgsPointCloud3DRendering::testPointCloudSingleColor()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setPointLights( QList<QgsPointLightSettings>() << defaultLight );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsSingleColorPointCloud3DSymbol *symbol = new QgsSingleColorPointCloud3DSymbol();
  symbol->setSingleColor( QColor( 255, 0, 0 ) );

  QgsPointCloudLayer3DRenderer *renderer = new QgsPointCloudLayer3DRenderer();
  renderer->setSymbol( symbol );
  mLayer->setRenderer3D( renderer );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  scene->cameraController()->resetView( 2.5 );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "pointcloud_3d_singlecolor", img, 40 ) );
}

void TestQgsPointCloud3DRendering::testPointCloudAttributeByRamp()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setPointLights( QList<QgsPointLightSettings>() << defaultLight );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsColorRampPointCloud3DSymbol *symbol = new QgsColorRampPointCloud3DSymbol();
  symbol->setAttribute( QStringLiteral( "Z" ) );
  QgsColorRampShader shader = QgsColorRampShader( 0.98, 1.25 );
  shader.setSourceColorRamp( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  shader.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
  symbol->setColorRampShader( shader );

  QgsPointCloudLayer3DRenderer *renderer = new QgsPointCloudLayer3DRenderer();
  renderer->setSymbol( symbol );
  mLayer->setRenderer3D( renderer );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  scene->cameraController()->resetView( 2.5 );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "pointcloud_3d_colorramp", img, 40 ) );
}

void TestQgsPointCloud3DRendering::testPointCloudRgb()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setPointLights( QList<QgsPointLightSettings>() << defaultLight );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsRgbPointCloud3DSymbol *symbol = new QgsRgbPointCloud3DSymbol();
  symbol->setRedAttribute( QStringLiteral( "Red" ) );
  symbol->setGreenAttribute( QStringLiteral( "Green" ) );
  symbol->setBlueAttribute( QStringLiteral( "Blue" ) );

  QgsContrastEnhancement *redContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
  redContrastEnhancement->setMinimumValue( 0 );
  redContrastEnhancement->setMaximumValue( 255 );
  QgsContrastEnhancement *greenContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
  greenContrastEnhancement->setMinimumValue( 0 );
  greenContrastEnhancement->setMaximumValue( 255 );
  QgsContrastEnhancement *blueContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
  blueContrastEnhancement->setMinimumValue( 0 );
  blueContrastEnhancement->setMaximumValue( 255 );
  blueContrastEnhancement->setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm::StretchAndClipToMinimumMaximum );
  greenContrastEnhancement->setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm::StretchAndClipToMinimumMaximum );
  blueContrastEnhancement->setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm::StretchAndClipToMinimumMaximum );
  symbol->setRedContrastEnhancement( redContrastEnhancement );
  symbol->setGreenContrastEnhancement( greenContrastEnhancement );
  symbol->setBlueContrastEnhancement( blueContrastEnhancement );

  QgsPointCloudLayer3DRenderer *renderer = new QgsPointCloudLayer3DRenderer();
  renderer->setSymbol( symbol );
  mLayer->setRenderer3D( renderer );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  scene->cameraController()->resetView( 2.5 );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "pointcloud_3d_rgb", img, 40 ) );
}

void TestQgsPointCloud3DRendering::testPointCloudClassification()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setPointLights( QList<QgsPointLightSettings>() << defaultLight );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsClassificationPointCloud3DSymbol *symbol = new QgsClassificationPointCloud3DSymbol();
  symbol->setAttribute( QStringLiteral( "Classification" ) );
  symbol->setCategoriesList( QgsPointCloudClassifiedRenderer::defaultCategories() );

  QgsPointCloudLayer3DRenderer *renderer = new QgsPointCloudLayer3DRenderer();
  renderer->setSymbol( symbol );
  mLayer->setRenderer3D( renderer );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  scene->cameraController()->resetView( 2.5 );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QVERIFY( renderCheck( "pointcloud_3d_classification", img, 40 ) );
}

void TestQgsPointCloud3DRendering::testPointCloudSyncedTo2D()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setPointLights( QList<QgsPointLightSettings>() << defaultLight );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsPointCloudLayer3DRenderer *renderer = new QgsPointCloudLayer3DRenderer();
  mLayer->setRenderer3D( renderer );

  // gather different 2D renderers
  QgsPointCloudExtentRenderer *extent2DRenderer = new QgsPointCloudExtentRenderer();
  QgsPointCloudAttributeByRampRenderer *colorramp2DRenderer = new QgsPointCloudAttributeByRampRenderer();
  colorramp2DRenderer->setAttribute( QStringLiteral( "Z" ) );
  QgsColorRampShader shader = QgsColorRampShader( 0.98, 1.25 );
  shader.setSourceColorRamp( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  shader.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
  colorramp2DRenderer->setColorRampShader( shader );
  QgsPointCloudRgbRenderer *rgb2DRenderer = dynamic_cast<QgsPointCloudRgbRenderer *>( mLayer->renderer()->clone() );
  QgsPointCloudClassifiedRenderer *classification2DRenderer = new QgsPointCloudClassifiedRenderer();
  classification2DRenderer->setAttribute( QStringLiteral( "Classification" ) );
  classification2DRenderer->setCategories( QgsPointCloudClassifiedRenderer::defaultCategories() );

  QgsPointCloudExtentRenderer *extent2DRenderer_2 = new QgsPointCloudExtentRenderer();
  QgsPointCloudAttributeByRampRenderer *colorramp2DRenderer_2 = new QgsPointCloudAttributeByRampRenderer();
  colorramp2DRenderer_2->setAttribute( QStringLiteral( "Z" ) );
  QgsColorRampShader shader_2 = QgsColorRampShader( 0.98, 1.25 );
  shader_2.setSourceColorRamp( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  shader_2.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
  colorramp2DRenderer_2->setColorRampShader( shader_2 );
  QgsPointCloudRgbRenderer *rgb2DRenderer_2 = dynamic_cast<QgsPointCloudRgbRenderer *>( mLayer->renderer()->clone() );
  QgsPointCloudClassifiedRenderer *classification2DRenderer_2 = new QgsPointCloudClassifiedRenderer();
  classification2DRenderer_2->setAttribute( QStringLiteral( "Classification" ) );
  classification2DRenderer_2->setCategories( QgsPointCloudClassifiedRenderer::defaultCategories() );

  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  scene->cameraController()->resetView( 2.5 );

  // 3D symbols should now automatically change to match the 2D renderer
  mLayer->setSync3DRendererTo2DRenderer( true );

  // default 2D renderer is rgb for this pointcloud
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_rgb", img, 40 ) );

  // extent renderer should not render anythin in 3D
  mLayer->setRenderer( extent2DRenderer );
  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_norenderer", img2, 40 ) );

  mLayer->setRenderer( colorramp2DRenderer );
  QImage img3 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_colorramp", img3, 40 ) );

  mLayer->setRenderer( rgb2DRenderer );
  QImage img4 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_rgb", img4, 40 ) );

  mLayer->setRenderer( classification2DRenderer );
  QImage img5 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img5, 40 ) );

  // Now let's stop syncing, 3D symbol should stay at the last one used
  mLayer->setSync3DRendererTo2DRenderer( false );

  mLayer->setRenderer( extent2DRenderer_2 );
  QImage img6 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img6, 40 ) );

  mLayer->setRenderer( colorramp2DRenderer_2 );
  QImage img7 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img7, 40 ) );

  mLayer->setRenderer( rgb2DRenderer_2 );
  QImage img8 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img8, 40 ) );

  mLayer->setRenderer( classification2DRenderer_2 );
  QImage img9 = Qgs3DUtils::captureSceneImage( engine, scene );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img9, 40 ) );
}
*/
QGSTEST_MAIN( TestQgsPointCloud3DRendering )
#include "testqgspointcloud3drendering.moc"
