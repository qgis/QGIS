/***************************************************************************
     testqgsmaptoolcirclez.cpp
     ------------------------
    Date                 : January 2018
    Copyright            : (C) 2018 by Paul Blottiere
    Email                : paul.blottiere@oslandia.com
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


class TestQgsMapToolCircleZ : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolCircleZ();

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

TestQgsMapToolCircleZ::TestQgsMapToolCircleZ() = default;


//runs before all tests
void TestQgsMapToolCircleZ::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  // make testing layers
  mLayer = new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=EPSG:27700" ), QStringLiteral( "layer line Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayer->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayer );

  // set layers in canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayer );
  mCanvas->setCurrentLayer( mLayer );

  mParentTool = new QgsMapToolAddFeature( mCanvas, QgsMapToolCapture::CaptureLine );
}

void TestQgsMapToolCircleZ::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolCircleZ::testCircleFrom2Points()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );
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

  QString wkt = "CompoundCurveZ (CircularStringZ (0 2 333, 1 1 333, 0 0 333, -1 1 333, 0 2 333))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolCircleZ::testCircleFrom2PointsWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );
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

  QString wkt = "CompoundCurveZ (CircularStringZ (0 2 333, 1 1 333, 0 0 333, -1 1 333, 0 2 333))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolCircleZ::testCircleFrom3Points()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
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

  QString wkt = "CompoundCurveZ (CircularStringZ (0 2 111, 1 1 111, 0 0 111, -1 1 111, 0 2 111))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolCircleZ::testCircleFrom3PointsWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
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

  QString wkt = "CompoundCurveZ (CircularStringZ (0 2 111, 1 1 111, 0 0 111, -1 1 111, 0 2 111))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolCircleZ::testCircleFromCenterPoint()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 222 );
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

  QString wkt = "CompoundCurveZ (CircularStringZ (0 2 222, 2 0 222, 0 -2 222, -2 0 222, 0 2 222))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolCircleZ::testCircleFromCenterPointWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 222 );
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

  QString wkt = "CompoundCurveZ (CircularStringZ (0 2 222, 2 0 222, 0 -2 222, -2 0 222, 0 2 222))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

QGSTEST_MAIN( TestQgsMapToolCircleZ )
#include "testqgsmaptoolcirclez.moc"
