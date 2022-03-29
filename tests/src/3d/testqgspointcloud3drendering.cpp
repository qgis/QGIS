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
#include <QDesktopWidget>

class TestQgsPointCloud3DRendering : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testPointCloudSingleColor();
    void testPointCloudAttributeByRamp();
    void testPointCloudRgb();
    void testPointCloudClassification();
    void testPointCloudSyncedTo2D();


  private:
    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0 );

    QString mReport;

    std::unique_ptr<QgsProject> mProject;
    QgsPointCloudLayer *mLayer;

};

bool TestQgsPointCloud3DRendering::renderCheck( const QString &testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
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

  mReport = QStringLiteral( "<h1>3D Rendering Tests</h1>\n" );

  mProject.reset( new QgsProject );

  const QString dataDir( TEST_DATA_DIR );

  mLayer = new QgsPointCloudLayer( dataDir + "/point_clouds/ept/rgb/ept.json", "test", "ept" );
  QVERIFY( mLayer->isValid() );
  mProject->addMapLayer( mLayer );

  mProject->setCrs( mLayer->crs() );

}

//runs after all tests
void TestQgsPointCloud3DRendering::cleanupTestCase()
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
  int i = 0;
  const QgsRectangle fullExtent = mLayer->extent();
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  map->setCrs( mProject->crs() );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsPointLightSettings defaultLight;
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  defaultLight.setIntensity( 0.5 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  map->setPointLights( QList<QgsPointLightSettings>() << defaultLight );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  QgsOffscreen3DEngine engine;
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  engine.setRootEntity( scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  QgsPointCloudLayer3DRenderer *renderer = new QgsPointCloudLayer3DRenderer();
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  mLayer->setRenderer3D( renderer );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  // gather different 2d renderers
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsPointCloudExtentRenderer *extent2DRenderer = new QgsPointCloudExtentRenderer();
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsPointCloudAttributeByRampRenderer *colorramp2DRenderer = new QgsPointCloudAttributeByRampRenderer();
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  colorramp2DRenderer->setAttribute( QStringLiteral( "Z" ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsColorRampShader shader = QgsColorRampShader( 0.98, 1.25 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  shader.setSourceColorRamp( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  shader.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  colorramp2DRenderer->setColorRampShader( shader );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsPointCloudRgbRenderer *rgb2DRenderer = dynamic_cast<QgsPointCloudRgbRenderer *>( mLayer->renderer()->clone() );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsPointCloudClassifiedRenderer *classification2DRenderer = new QgsPointCloudClassifiedRenderer();
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  classification2DRenderer->setAttribute( QStringLiteral( "Classification" ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  classification2DRenderer->setCategories( QgsPointCloudClassifiedRenderer::defaultCategories() );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  QgsPointCloudExtentRenderer *extent2DRenderer_2 = new QgsPointCloudExtentRenderer();
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsPointCloudAttributeByRampRenderer *colorramp2DRenderer_2 = new QgsPointCloudAttributeByRampRenderer();
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  colorramp2DRenderer_2->setAttribute( QStringLiteral( "Z" ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsColorRampShader shader_2 = QgsColorRampShader( 0.98, 1.25 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  shader_2.setSourceColorRamp( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  shader_2.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  colorramp2DRenderer_2->setColorRampShader( shader_2 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsPointCloudRgbRenderer *rgb2DRenderer_2 = dynamic_cast<QgsPointCloudRgbRenderer *>( mLayer->renderer()->clone() );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QgsPointCloudClassifiedRenderer *classification2DRenderer_2 = new QgsPointCloudClassifiedRenderer();
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  classification2DRenderer_2->setAttribute( QStringLiteral( "Classification" ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  classification2DRenderer_2->setCategories( QgsPointCloudClassifiedRenderer::defaultCategories() );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  // When running the test on Travis, it would initially return empty rendered image.
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  // find a better fix in the future.
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  scene->cameraController()->resetView( 2.5 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  // 3D symbols should now automatically change to match the 2d renderer
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  mLayer->setSync3DRendererTo2DRenderer( true );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  // default 2d renderer is rgb for this pointcloud
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_rgb", img, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  // extent renderer should not render anythin in 3d
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  mLayer->setRenderer( extent2DRenderer );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_norenderer", img2, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  mLayer->setRenderer( colorramp2DRenderer );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img3 = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_colorramp", img3, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  mLayer->setRenderer( rgb2DRenderer );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img4 = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_rgb", img4, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  mLayer->setRenderer( classification2DRenderer );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img5 = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img5, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  // Now let's stop syncing, 3d symbol should stay at the last one used
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  mLayer->setSync3DRendererTo2DRenderer( false );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  mLayer->setRenderer( extent2DRenderer_2 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img6 = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img6, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  mLayer->setRenderer( colorramp2DRenderer_2 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img7 = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img7, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  mLayer->setRenderer( rgb2DRenderer_2 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img8 = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img8, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );

  mLayer->setRenderer( classification2DRenderer_2 );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QImage img9 = Qgs3DUtils::captureSceneImage( engine, scene );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
  QVERIFY( renderCheck( "pointcloud_3d_classification", img9, 40 ) );
  QWARN( ( QString( "%1" ).arg( i++ ) ).toLocal8Bit().data() );
}

QGSTEST_MAIN( TestQgsPointCloud3DRendering )
#include "testqgspointcloud3drendering.moc"
