/***************************************************************************
     testqgsmaptoolcirclezm.cpp
     ------------------------
    Date                 : April 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
    Email                : loic dot bartoletti @oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgisapp.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgsmaptooladdfeature.h"

#include "testqgsmaptoolutils.h"
#include "qgsmaptoolcircle2points.h"
#include "qgsmaptoolcircle3points.h"
#include "qgsmaptoolcirclecenterpoint.h"


class TestQgsMapToolCircleZM : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolCircleZM();

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testCircleFrom2Points();
    void testCircleFrom2PointsWithDeletedVertex();
    void testCircleFrom3Points();
    void testCircleFrom3PointsWithDeletedVertex();
    void testCircleFromCenterPoint();
    void testCircleFromCenterPointWithDeletedVertex();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mParentTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

TestQgsMapToolCircleZM::TestQgsMapToolCircleZM() = default;


//runs before all tests
void TestQgsMapToolCircleZM::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  // make testing layers
  mLayer = new QgsVectorLayer( QStringLiteral( "LineStringZM?crs=EPSG:27700" ), QStringLiteral( "layer line ZM" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayer->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayer );

  // set layers in canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayer );
  mCanvas->setCurrentLayer( mLayer );

  mParentTool = new QgsMapToolAddFeature( mCanvas, QgsMapToolCapture::CaptureLine );
}

void TestQgsMapToolCircleZM::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolCircleZM::testCircleFrom2Points()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 444 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 333 );
  mLayer->startEditing();

  QgsMapToolCircle2Points mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "CompoundCurveZM (CircularStringZM (0 2 444 333, 1 1 444 333, 0 0 444 333, -1 1 444 333, 0 2 444 333))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );
}

void TestQgsMapToolCircleZM::testCircleFrom2PointsWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 444 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 333 );
  mLayer->startEditing();

  QgsMapToolCircle2Points mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );

  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "CompoundCurveZM (CircularStringZM (0 2 444 333, 1 1 444 333, 0 0 444 333, -1 1 444 333, 0 2 444 333))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );
}

void TestQgsMapToolCircleZM::testCircleFrom3Points()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 444 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolCircle3Points mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "CompoundCurveZM (CircularStringZM (0 2 444 111, 1 1 444 111, 0 0 444 111, -1 1 444 111, 0 2 444 111))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );
}

void TestQgsMapToolCircleZM::testCircleFrom3PointsWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 444 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolCircle3Points mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "CompoundCurveZM (CircularStringZM (0 2 444 111, 1 1 444 111, 0 0 444 111, -1 1 444 111, 0 2 444 111))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );
}

void TestQgsMapToolCircleZM::testCircleFromCenterPoint()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 444 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 222 );
  mLayer->startEditing();

  QgsMapToolCircleCenterPoint mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "CompoundCurveZM (CircularStringZM (0 2 444 222, 2 0 444 222, 0 -2 444 222, -2 0 444 222, 0 2 444 222))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );
}

void TestQgsMapToolCircleZM::testCircleFromCenterPointWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 444 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 222 );
  mLayer->startEditing();

  QgsMapToolCircleCenterPoint mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );

  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "CompoundCurveZM (CircularStringZM (0 2 444 222, 2 0 444 222, 0 -2 444 222, -2 0 444 222, 0 2 444 222))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );
}

QGSTEST_MAIN( TestQgsMapToolCircleZM )
#include "testqgsmaptoolcirclezm.moc"
