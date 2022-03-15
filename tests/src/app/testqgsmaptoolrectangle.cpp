/***************************************************************************
    testqgsmaptoolrectangle.cpp
    ---------------------------
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
#include "qgsgeometryutils.h"

#include "testqgsmaptoolutils.h"
#include "qgsmaptoolshaperectanglecenter.h"
#include "qgsmaptoolshaperectangleextent.h"
#include "qgsmaptoolshaperectangle3points.h"


class TestQgsMapToolRectangle : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolRectangle();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testRectangleFromCenter();
    void testRectangleFromCenterWithDeletedVertex();
    void testRectangleFromExtent();
    void testRectangleFromExtentWithDeletedVertex();
    void testRectangleFrom3PointsDistance();
    void testRectangleFrom3PointsDistanceWithDeletedVertex();
    void testRectangleFrom3PointsProjected();
    void testRectangleFrom3PointsProjectedWithDeletedVertex();

  private:
    void resetMapTool( QgsMapToolShapeMetadata *metadata );

    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mMapTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

TestQgsMapToolRectangle::TestQgsMapToolRectangle() = default;


//runs before all tests
void TestQgsMapToolRectangle::initTestCase()
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

  mMapTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );
  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Shape );
  mCanvas->setMapTool( mMapTool );
}

void TestQgsMapToolRectangle::cleanupTestCase()
{
  QgsApplication::exitQgis();
  delete mMapTool;
}

void TestQgsMapToolRectangle::cleanup()
{
  mMapTool->clean();
}

void TestQgsMapToolRectangle::resetMapTool( QgsMapToolShapeMetadata *metadata )
{
  mMapTool->setCurrentShapeMapTool( metadata ) ;
}

void TestQgsMapToolRectangle::testRectangleFromCenter()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );
  mLayer->startEditing();

  QgsMapToolShapeRectangleCenterMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (-2 -1 333, -2 1 333, 2 1 333, 2 -1 333, -2 -1 333)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolRectangle::testRectangleFromCenterWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );
  mLayer->startEditing();

  QgsMapToolShapeRectangleCenterMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (-2 -1 333, -2 1 333, 2 1 333, 2 -1 333, -2 -1 333)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolRectangle::testRectangleFromExtent()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 222 );
  mLayer->startEditing();

  QgsMapToolShapeRectangleExtentMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (0 0 222, 0 1 222, 2 1 222, 2 0 222, 0 0 222)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}
void TestQgsMapToolRectangle::testRectangleFromExtentWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 222 );
  mLayer->startEditing();

  QgsMapToolShapeRectangleExtentMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (0 0 222, 0 1 222, 2 1 222, 2 0 222, 0 0 222)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}


void TestQgsMapToolRectangle::testRectangleFrom3PointsDistance()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolShapeRectangle3PointsMetadata md( QgsMapToolShapeRectangle3PointsMetadata::CreateMode::Distance );
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 0 );
  utils.mouseClick( 2, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (0 0 111, 2 0 111, 2 1 111, 0 1 111, 0 0 111)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}
void TestQgsMapToolRectangle::testRectangleFrom3PointsDistanceWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolShapeRectangle3PointsMetadata md( QgsMapToolShapeRectangle3PointsMetadata::CreateMode::Distance );
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 0 );
  utils.mouseClick( 3, 0, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 2, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (0 0 111, 2 0 111, 2 1 111, 0 1 111, 0 0 111)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolRectangle::testRectangleFrom3PointsProjected()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolShapeRectangle3PointsMetadata md( QgsMapToolShapeRectangle3PointsMetadata::CreateMode::Projected );
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 0 );
  utils.mouseClick( 2, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (0 0 111, 2 0 111, 2 1 111, 0 1 111, 0 0 111)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}
void TestQgsMapToolRectangle::testRectangleFrom3PointsProjectedWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolShapeRectangle3PointsMetadata md( QgsMapToolShapeRectangle3PointsMetadata::CreateMode::Projected );
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 0 );
  utils.mouseClick( 3, 0, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 2, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (0 0 111, 2 0 111, 2 1 111, 0 1 111, 0 0 111)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}
QGSTEST_MAIN( TestQgsMapToolRectangle )
#include "testqgsmaptoolrectangle.moc"
