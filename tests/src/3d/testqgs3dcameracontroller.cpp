/***************************************************************************
  testqgs3dcameracontroller.cpp
  --------------------------------------
  Date                 : November 2023
  Copyright            : (C) 2023 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgs3d.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsflatterraingenerator.h"
#include "qgsoffscreen3dengine.h"
#include "qgsphongmaterialsettings.h"
#include "qgspolygon3dsymbol.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"


class TestQgs3DCameraController : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3DCameraController()
      : QgsTest( QStringLiteral( "3D Camera Controller Tests" ), QStringLiteral( "3d" ) )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testTranslate();
    void testZoom();
    void testZoomWheel();
    void testRotationCenter();
    void testRotationCamera();
    void testRotationCenterZoomWheelRotationCenter();
    void testTranslateRotationCenterTranslate();
    void testTranslateZoomWheelTranslate();
    void testTranslateRotationCameraTranslate();
    void testRotationCenterRotationCameraRotationCenter();

  private:
    QgsRasterLayer *mLayerRgb = nullptr;
    QgsVectorLayer *mLayerBuildings = nullptr;
};

// runs before all tests
void TestQgs3DCameraController::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  mLayerRgb = new QgsRasterLayer( testDataPath( "/3d/rgb.tif" ), "rgb", "gdal" );
  QVERIFY( mLayerRgb->isValid() );

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
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );
}

//runs after all tests
void TestQgs3DCameraController::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgs3DCameraController::testTranslate()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( -944.6, 180.3, 0.0 ), 4.0 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -944.6, 180.3, 0.0 ), 4.0 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testZoom()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Zoom );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Zoom );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Zoom );

  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 0.0, 0.0, 3.4 ), 1.5 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( 0.0, 1.5, -850.4 ), 1.5 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testZoomWheel()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  QWheelEvent wheelEvent( midPos, midPos, QPoint(), QPoint( 0, 200 ), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false, Qt::MouseEventSynthesizedByApplication );
  scene->cameraController()->onWheel( new Qt3DInput::QWheelEvent( wheelEvent ) );
  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::ZoomWheel );
  QCOMPARE( scene->cameraController()->mCumulatedWheelY, wheelEvent.angleDelta().y() * 5 );
  QCOMPARE( scene->cameraController()->mCameraBefore->viewCenter(), scene->cameraController()->cameraPose().centerPoint().toVector3D() );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  QGSCOMPARENEARVECTOR3D( scene->cameraController()->mZoomPoint, QVector3D( -1381.3, 1036.7, 1.0 ), 5.0 );
  QGSCOMPARENEARVECTOR3D( scene->cameraController()->cameraPose().centerPoint(), QVector3D( -479.2, 359.4, 1.6 ), 5.0 );
  QGSCOMPARENEAR( scene->cameraController()->cameraPose().distanceFromCenterPoint(), 1631.9, 5.0 );
  QCOMPARE( scene->cameraController()->mCumulatedWheelY, 0 );
  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testRotationCenter()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 2.1, 1.1, 0.3 ), 1.0 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -465.0, -241.2, -56.1 ), 4.0 );
  float diffPitch = scene->cameraController()->pitch() - initialPitch;
  float diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 12.1, 0.1 );
  QGSCOMPARENEAR( diffYaw, -62.2, 0.1 );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testRotationCamera()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCamera );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCamera );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCamera );

  // TODO: ???
  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 263.6, 266.5, 28.7 ), 4.0 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( 0.0, 0.0, 0.0 ), 1.0 );
  float diffPitch = scene->cameraController()->pitch() - initialPitch;
  float diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 8.6, 0.1 );
  QGSCOMPARENEAR( diffYaw, -44.2, 0.1 );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testRotationCenterZoomWheelRotationCenter()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  //
  // 1. Rotation around the clicked point
  //

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 2.1, 1.1, 0.3 ), 1.0 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -465.0, -241.2, -56.1 ), 4.0 );
  float diffPitch = scene->cameraController()->pitch() - initialPitch;
  float diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 12.1, 0.1 );
  QGSCOMPARENEAR( diffYaw, -62.2, 0.1 );

  //
  // 2. Zoom Wheel
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();
  initialPitch = scene->cameraController()->pitch();
  initialYaw = scene->cameraController()->yaw();

  QWheelEvent wheelEvent( midPos, midPos, QPoint(), QPoint( 0, 200 ), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false, Qt::MouseEventSynthesizedByApplication );
  scene->cameraController()->onWheel( new Qt3DInput::QWheelEvent( wheelEvent ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::ZoomWheel );
  QCOMPARE( scene->cameraController()->mCumulatedWheelY, wheelEvent.angleDelta().y() * 5 );
  QCOMPARE( scene->cameraController()->mCameraBefore->viewCenter(), scene->cameraController()->cameraPose().centerPoint().toVector3D() );


  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  QGSCOMPARENEARVECTOR3D( scene->cameraController()->mZoomPoint, QVector3D( 283.2, -923.1, -27.0 ), 1.5 );
  QGSCOMPARENEARVECTOR3D( scene->cameraController()->cameraPose().centerPoint(), QVector3D( 99.4, -319.9, -8.8 ), 2.0 );
  QGSCOMPARENEAR( scene->cameraController()->cameraPose().distanceFromCenterPoint(), 1631.9, 2.0 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );
  QCOMPARE( scene->cameraController()->mCumulatedWheelY, 0 );
  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  //
  // 3. Rotate around the clicked point
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();
  initialPitch = scene->cameraController()->pitch();
  initialYaw = scene->cameraController()->yaw();

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QMouseEvent mouseMoveEvent3( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent3 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  QMouseEvent mouseMoveEvent4( QEvent::MouseMove, midPos + movement1 + 10 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent4 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 25.9, 7.1, 5.2 ), 1.0 );
  diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -44.3, -9.1, -11.7 ), 1.0 );
  diffPitch = scene->cameraController()->pitch() - initialPitch;
  diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 2.5, 0.1 );
  QGSCOMPARENEAR( diffYaw, -2.5, 0.1 );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testTranslateRotationCenterTranslate()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  //
  // 1. Translate
  //

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( -944.6, 180.3, 0.0 ), 4.0 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -944.6, 180.3, 0.0 ), 4.0 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );

  //
  // 2. Rotate around the clicked point
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();
  initialPitch = scene->cameraController()->pitch();
  initialYaw = scene->cameraController()->yaw();

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QMouseEvent mouseMoveEvent3( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent3 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  QMouseEvent mouseMoveEvent4( QEvent::MouseMove, midPos + movement1 + 10 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent4 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 9.1, 42.0, 8.2 ), 4.0 );
  diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( 3.8, -68.3, 5.3 ), 4.0 );
  float diffPitch = scene->cameraController()->pitch() - initialPitch;
  float diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 2.5, 0.1 );
  QGSCOMPARENEAR( diffYaw, -2.5, 0.1 );

  //
  // 3. Translate
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();
  initialPitch = scene->cameraController()->pitch();
  initialYaw = scene->cameraController()->yaw();

  // switch to translation
  QMouseEvent mouseMoveEvent5( QEvent::MouseMove, midPos + movement1 + 10 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent5 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // this updates the mouse position
  QMouseEvent mouseMoveEvent6( QEvent::MouseMove, midPos + movement1 + 15 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent6 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 15 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );


  // this updates the camera
  QMouseEvent mouseMoveEvent7( QEvent::MouseMove, midPos + movement1 + 15 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent7 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 15 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( -20.2, 22.4, 0.0 ), 4.0 );
  diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -20.2, 22.4, 0.0 ), 4.0 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testTranslateZoomWheelTranslate()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  //
  // 1. Translate
  //

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( -944.6, 180.3, 0.0 ), 4.0 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -944.6, 180.3, 0.0 ), 4.0 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );


  //
  // 2. Zoom Wheel
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();

  QWheelEvent wheelEvent( midPos + movement1 + movement2, midPos + movement1 + movement2, QPoint(), QPoint( 0, 200 ), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false, Qt::MouseEventSynthesizedByApplication );
  scene->cameraController()->onWheel( new Qt3DInput::QWheelEvent( wheelEvent ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::ZoomWheel );
  QCOMPARE( scene->cameraController()->mCumulatedWheelY, wheelEvent.angleDelta().y() * 5 );
  QCOMPARE( scene->cameraController()->mCameraBefore->viewCenter(), scene->cameraController()->cameraPose().centerPoint().toVector3D() );


  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  QGSCOMPARENEARVECTOR3D( scene->cameraController()->mZoomPoint, QVector3D( 4.8, -4.4, 9.9 ), 1.0 );
  QGSCOMPARENEARVECTOR3D( scene->cameraController()->cameraPose().centerPoint(), QVector3D( -615.5, 116.2, 3.4 ), 4.0 );
  QGSCOMPARENEAR( scene->cameraController()->cameraPose().distanceFromCenterPoint(), 1631.9, 2.0 );
  QCOMPARE( scene->cameraController()->mCumulatedWheelY, 0 );
  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );

  //
  // 3. Translate
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();

  // switch to translation
  QMouseEvent mouseMoveEvent3( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent3 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // this updates the mouse position
  QMouseEvent mouseMoveEvent4( QEvent::MouseMove, midPos + movement1 + 5 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent4 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 5 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );


  // this updates the camera
  QMouseEvent mouseMoveEvent5( QEvent::MouseMove, midPos + movement1 + 5 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent5 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 5 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( -11.3, 11.3, 0.0 ), 1.0 );
  diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -11.3, 11.3, 0.0 ), 1.0 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testTranslateRotationCameraTranslate()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  //
  // 1. Translate
  //

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( -944.6, 180.3, 0.0 ), 4.0 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -944.6, 180.3, 0.0 ), 4.0 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );

  //
  // 2. Rotate around the camera
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QMouseEvent mouseMoveEvent3( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent3 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCamera );

  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  QMouseEvent mouseMoveEvent4( QEvent::MouseMove, midPos + movement1 + 10 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent4 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCamera );

  diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 2.7, 78.0, 1.5 ), 2.0 );
  diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( 0.0, 0.0, 0.0 ), 2.0 );
  float diffPitch = scene->cameraController()->pitch() - initialPitch;
  float diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 1.8, 0.1 );
  QGSCOMPARENEAR( diffYaw, -1.8, 0.1 );

  //
  // 3. Translate
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();
  initialPitch = scene->cameraController()->pitch();
  initialYaw = scene->cameraController()->yaw();

  // switch to translation
  QMouseEvent mouseMoveEvent5( QEvent::MouseMove, midPos + movement1 + 10 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent5 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  // this updates the mouse position
  QMouseEvent mouseMoveEvent6( QEvent::MouseMove, midPos + movement1 + 15 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent6 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 15 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );


  // this updates the camera
  QMouseEvent mouseMoveEvent7( QEvent::MouseMove, midPos + movement1 + 15 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent7 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 15 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::Translation );

  diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( -20.2, 22.4, 0.0 ), 2.0 );
  diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -20.2, 22.4, 0.0 ), 2.0 );
  QCOMPARE( scene->cameraController()->pitch(), initialPitch );
  QCOMPARE( scene->cameraController()->yaw(), initialYaw );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

void TestQgs3DCameraController::testRotationCenterRotationCameraRotationCenter()
{
  const QgsRectangle fullExtent = mLayerRgb->extent();

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setCrs( mLayerRgb->crs() );
  mapSettings->setExtent( fullExtent );
  mapSettings->setLayers( QList<QgsMapLayer *>() << mLayerRgb << mLayerBuildings );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );
  mapSettings->setTerrainGenerator( flatTerrain );

  QPoint winSize = QPoint( 640, 480 ); // default window size
  QPoint midPos = winSize / 2;
  QgsOffscreen3DEngine engine;
  engine.setSize( QSize( winSize.x(), winSize.y() ) );
  Qgs3DMapScene *scene = new Qgs3DMapScene( *mapSettings, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );
  QVector3D initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  QVector3D initialCamPosition = scene->cameraController()->camera()->position();
  float initialPitch = scene->cameraController()->pitch();
  float initialYaw = scene->cameraController()->yaw();

  // this call is not used but ensures to synchronize the scene
  Qgs3DUtils::captureSceneImage( engine, scene );

  QMouseEvent mousePressEvent( QEvent::MouseButtonPress, midPos, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onMousePressed( new Qt3DInput::QMouseEvent( mousePressEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QImage depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  //
  // 1. Rotation around the clicked point
  //

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QPoint movement1( 220, 42 );
  QMouseEvent mouseMoveEvent1( QEvent::MouseMove, midPos + movement1, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent1 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QPoint movement2( 1, 1 );
  QMouseEvent mouseMoveEvent2( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent2 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  QVector3D diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 2.1, 1.1, 0.3 ), 1.0 );
  QVector3D diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -465.0, -241.2, -56.1 ), 4.0 );
  float diffPitch = scene->cameraController()->pitch() - initialPitch;
  float diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 12.1, 0.1 );
  QGSCOMPARENEAR( diffYaw, -62.2, 0.1 );

  //
  // 2. Rotate around the camera
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();
  initialPitch = scene->cameraController()->pitch();
  initialYaw = scene->cameraController()->yaw();

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QMouseEvent mouseMoveEvent3( QEvent::MouseMove, midPos + movement1 + movement2, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent3 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCamera );

  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  QMouseEvent mouseMoveEvent4( QEvent::MouseMove, midPos + movement1 + 10 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent4 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCamera );

  diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 76.3, 18.7, 17.9 ), 1.0 );
  diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( 0.0, 0.0, 0.0 ), 1.0 );
  diffPitch = scene->cameraController()->pitch() - initialPitch;
  diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 1.8, 0.1 );
  QGSCOMPARENEAR( diffYaw, -1.8, 0.1 );

  //
  // 3. Rotate around the clicked point
  //

  initialCamViewCenter = scene->cameraController()->camera()->viewCenter();
  initialCamPosition = scene->cameraController()->camera()->position();
  initialPitch = scene->cameraController()->pitch();
  initialYaw = scene->cameraController()->yaw();

  // the first mouse event only updates the mouse position
  // the second one will update the camera
  QMouseEvent mouseMoveEvent5( QEvent::MouseMove, midPos + movement1 + 10 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent5 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  depthImage = Qgs3DUtils::captureSceneDepthBuffer( engine, scene );
  scene->cameraController()->depthBufferCaptured( depthImage );

  QMouseEvent mouseMoveEvent6( QEvent::MouseMove, midPos + movement1 + 12 * movement2, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  scene->cameraController()->onPositionChanged( new Qt3DInput::QMouseEvent( mouseMoveEvent6 ) );
  QCOMPARE( scene->cameraController()->mClickPoint, midPos + movement1 + 10 * movement2 );
  QCOMPARE( scene->cameraController()->mMousePos, midPos + movement1 + 12 * movement2 );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::RotationCenter );

  diffViewCenter = scene->cameraController()->camera()->viewCenter() - initialCamViewCenter;
  QGSCOMPARENEARVECTOR3D( diffViewCenter, QVector3D( 9.3, 2.1, 2.2 ), 1.0 );
  diffPosition = scene->cameraController()->camera()->position() - initialCamPosition;
  QGSCOMPARENEARVECTOR3D( diffPosition, QVector3D( -14.8, -2.8, -3.9 ), 1.0 );
  diffPitch = scene->cameraController()->pitch() - initialPitch;
  diffYaw = scene->cameraController()->yaw() - initialYaw;
  QGSCOMPARENEAR( diffPitch, 0.6, 0.1 );
  QGSCOMPARENEAR( diffYaw, -0.6, 0.1 );

  QMouseEvent mouseReleaseEvent( QEvent::MouseButtonRelease, midPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
  scene->cameraController()->onMouseReleased( new Qt3DInput::QMouseEvent( mouseReleaseEvent ) );

  QCOMPARE( scene->cameraController()->mClickPoint, QPoint() );
  QCOMPARE( scene->cameraController()->mCurrentOperation, QgsCameraController::MouseOperation::None );

  delete scene;
  mapSettings->setLayers( {} );
}

QGSTEST_MAIN( TestQgs3DCameraController )
#include "testqgs3dcameracontroller.moc"
