/***************************************************************************
     testqgsmaptoolregularpolygon.cpp
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
#include "qgsmaptoolshaperegularpolygon2points.h"
#include "qgsmaptoolshaperegularpolygoncenterpoint.h"
#include "qgsmaptoolshaperegularpolygoncentercorner.h"


class TestQgsMapToolRegularPolygon : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolRegularPolygon();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testRegularPolygonFrom2Points();
    void testRegularPolygonFrom2PointsWithDeletedVertex();
    void testRegularPolygonFromCenterAndPoint();
    void testRegularPolygonFromCenterAndPointWithDeletedVertex();
    void testRegularPolygonFromCenterAndCroner();
    void testRegularPolygonFromCenterAndCronerWithDeletedVertex();

  private:
    void resetMapTool( QgsMapToolShapeMetadata *metadata );

    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mMapTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

TestQgsMapToolRegularPolygon::TestQgsMapToolRegularPolygon() = default;


//runs before all tests
void TestQgsMapToolRegularPolygon::initTestCase()
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

void TestQgsMapToolRegularPolygon::cleanupTestCase()
{
  QgsApplication::exitQgis();
  delete mMapTool;
}

void TestQgsMapToolRegularPolygon::cleanup()
{
  mMapTool->clean();
}

void TestQgsMapToolRegularPolygon::resetMapTool( QgsMapToolShapeMetadata *metadata )
{
  mMapTool->setCurrentShapeMapTool( metadata ) ;
}

void TestQgsMapToolRegularPolygon::testRegularPolygonFrom2Points()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );
  mLayer->startEditing();

  QgsMapToolShapeRegularPolygon2PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (0 0 333, 2 1 333, 4 0 333, 4 -2 333, 2 -3 333, 0 -2 333, 0 0 333)";
  QCOMPARE( f.geometry().asWkt( 0 ), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}
void TestQgsMapToolRegularPolygon::testRegularPolygonFrom2PointsWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );
  mLayer->startEditing();

  QgsMapToolShapeRegularPolygon2PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (0 0 333, 2 1 333, 4 0 333, 4 -2 333, 2 -3 333, 0 -2 333, 0 0 333)";
  QCOMPARE( f.geometry().asWkt( 0 ), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}


void TestQgsMapToolRegularPolygon::testRegularPolygonFromCenterAndPoint()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 222 );
  mLayer->startEditing();

  QgsMapToolShapeRegularPolygonCenterPointMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (1 2 222, 3 0 222, 1 -2 222, -1 -2 222, -3 0 222, -1 2 222, 1 2 222)";
  QCOMPARE( f.geometry().asWkt( 0 ), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}
void TestQgsMapToolRegularPolygon::testRegularPolygonFromCenterAndPointWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 222 );
  mLayer->startEditing();

  QgsMapToolShapeRegularPolygonCenterPointMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (1 2 222, 3 0 222, 1 -2 222, -1 -2 222, -3 0 222, -1 2 222, 1 2 222)";
  QCOMPARE( f.geometry().asWkt( 0 ), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}


void TestQgsMapToolRegularPolygon::testRegularPolygonFromCenterAndCroner()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolShapeRegularPolygonCenterCornerMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (2 1 111, 2 -1 111, 0 -2 111, -2 -1 111, -2 1 111, 0 2 111, 2 1 111)";
  QCOMPARE( f.geometry().asWkt( 0 ), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}
void TestQgsMapToolRegularPolygon::testRegularPolygonFromCenterAndCronerWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolShapeRegularPolygonCenterCornerMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "LineStringZ (2 1 111, 2 -1 111, 0 -2 111, -2 -1 111, -2 1 111, 0 2 111, 2 1 111)";
  QCOMPARE( f.geometry().asWkt( 0 ), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}


QGSTEST_MAIN( TestQgsMapToolRegularPolygon )
#include "testqgsmaptoolregularpolygon.moc"
