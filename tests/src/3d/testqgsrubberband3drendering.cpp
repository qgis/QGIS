/***************************************************************************
  testqgsrubberband3drendering.cpp
  --------------------------------------
  Date                 : February 2025
  Copyright            : (C) 2025 by Matej Bagar
  Email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoffscreen3dengine.h"
#include "qgstest.h"

#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgs3d.h"
#include "qgspointcloudlayer.h"
#include "qgspointlightsettings.h"
#include "qgsstyle.h"
#include "qgs3dutils.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmapscene.h"
#include "qgsframegraph.h"
#include "qgsrubberband3d.h"

class TestQgsRubberBand3DRendering : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsRubberBand3DRendering()
      : QgsTest( QStringLiteral( "Rubberband 3D Rendering Tests" ), QStringLiteral( "3d" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testRubberBandPoint();
    void testRubberBandLine();
    void testRubberBandPolygon();
    void testRubberBandHiddenMarker();
    void testRubberBandHiddenLastMarker();
    void testRubberBandHiddenEdges();
    void testRubberBandHiddenPolygonFill();

  private:
    std::unique_ptr<QgsProject> mProject;
    QgsPointCloudLayer *mLayer;
    QgsLineString *mPoints;
};

//runs before all tests
void TestQgsRubberBand3DRendering::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  mProject.reset( new QgsProject );

  const QString dataDir( TEST_DATA_DIR );

  mLayer = new QgsPointCloudLayer( dataDir + "/point_clouds/copc/rgb.copc.laz", "test", "copc" );
  QVERIFY( mLayer->isValid() );
  mProject->addMapLayer( mLayer );
  mProject->setCrs( mLayer->crs() );

  const QVector<QgsPoint> *points = new QVector<QgsPoint>( { QgsPoint( mLayer->extent().center().x() - 25, mLayer->extent().center().y() - 25, 0 ), QgsPoint( mLayer->extent().center().x() + 25, mLayer->extent().center().y() - 25, 0 ), QgsPoint( mLayer->extent().center().x() + 25, mLayer->extent().center().y() + 25, 0 ), QgsPoint( mLayer->extent().center().x() - 25, mLayer->extent().center().y() + 25, 0 ) } );
  mPoints = new QgsLineString( *points );
}

//runs after all tests
void TestQgsRubberBand3DRendering::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgsRubberBand3DRendering::testRubberBandPoint()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsRubberBand3D pointRubberBand( *map, &engine, engine.frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Point );
  pointRubberBand.addPoint( QgsPoint( fullExtent.center().x() - 25, fullExtent.center().y() - 25, 0 ) );
  pointRubberBand.addPoint( QgsPoint( fullExtent.center().x() + 25, fullExtent.center().y() - 25, 0 ) );
  pointRubberBand.addPoint( QgsPoint( fullExtent.center().x() - 25, fullExtent.center().y() + 25, 0 ) );
  pointRubberBand.addPoint( QgsPoint( fullExtent.center().x() + 25, fullExtent.center().y() + 25, 0 ) );

  scene->cameraController()->resetView( 90 );
  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "rubberband_3d_point", "rubberband_3d_point", img, QString(), 80, QSize( 0, 0 ), 15 );
}

void TestQgsRubberBand3DRendering::testRubberBandLine()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsRubberBand3D pointRubberBand( *map, &engine, engine.frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Line );
  pointRubberBand.setGeometry( QgsGeometry( mPoints->clone() ) );

  scene->cameraController()->resetView( 90 );
  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "rubberband_3d_line", "rubberband_3d_line", img, QString(), 80, QSize( 0, 0 ), 15 );
}

void TestQgsRubberBand3DRendering::testRubberBandPolygon()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsRubberBand3D pointRubberBand( *map, &engine, engine.frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
  pointRubberBand.setGeometry( QgsGeometry( new QgsPolygon( mPoints->clone() ) ) );

  scene->cameraController()->resetView( 90 );
  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "rubberband_3d_polygon", "rubberband_3d_polygon", img, QString(), 80, QSize( 0, 0 ), 15 );
}

void TestQgsRubberBand3DRendering::testRubberBandHiddenMarker()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsRubberBand3D pointRubberBand( *map, &engine, engine.frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
  pointRubberBand.setMarkerEnabled( false );
  pointRubberBand.setGeometry( QgsGeometry( new QgsPolygon( mPoints->clone() ) ) );

  scene->cameraController()->resetView( 90 );
  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "rubberband_3d_hidden_marker", "rubberband_3d_hidden_marker", img, QString(), 80, QSize( 0, 0 ), 15 );
}

void TestQgsRubberBand3DRendering::testRubberBandHiddenLastMarker()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsRubberBand3D pointRubberBand( *map, &engine, engine.frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Line );
  pointRubberBand.setHideLastMarker( true );
  pointRubberBand.setGeometry( QgsGeometry( mPoints->clone() ) );

  scene->cameraController()->resetView( 90 );
  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "rubberband_3d_hidden_last_marker", "rubberband_3d_hidden_last_marker", img, QString(), 80, QSize( 0, 0 ), 15 );
}

void TestQgsRubberBand3DRendering::testRubberBandHiddenEdges()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsRubberBand3D pointRubberBand( *map, &engine, engine.frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
  pointRubberBand.setEdgesEnabled( false );
  pointRubberBand.setGeometry( QgsGeometry( new QgsPolygon( mPoints->clone() ) ) );

  scene->cameraController()->resetView( 90 );
  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "rubberband_3d_hidden_edges", "rubberband_3d_hidden_edges", img, QString(), 80, QSize( 0, 0 ), 15 );
}

void TestQgsRubberBand3DRendering::testRubberBandHiddenPolygonFill()
{
  const QgsRectangle fullExtent = mLayer->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << mLayer );
  QgsPointLightSettings defaultLight;
  defaultLight.setIntensity( 0.5 );
  defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
  map->setLightSources( { defaultLight.clone() } );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  QgsRubberBand3D pointRubberBand( *map, &engine, engine.frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
  pointRubberBand.setPolygonFillEnabled( false );
  pointRubberBand.setGeometry( QgsGeometry( new QgsPolygon( mPoints->clone() ) ) );

  scene->cameraController()->resetView( 90 );
  Qgs3DUtils::captureSceneImage( engine, scene );
  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QGSVERIFYIMAGECHECK( "rubberband_3d_hidden_fill", "rubberband_3d_hidden_fill", img, QString(), 80, QSize( 0, 0 ), 15 );
}

QGSTEST_MAIN( TestQgsRubberBand3DRendering )
#include "testqgsrubberband3drendering.moc"
