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
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmaptooladdfeature.h"
#include "qgsgeometryutils.h"

#include "testqgsmaptoolutils.h"
#include "qgsmaptoolrectanglecenter.h"
#include "qgsmaptoolrectangleextent.h"
#include "qgsmaptoolrectangle3points.h"


class TestQgsMapToolRectangle : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolRectangle();

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testRectangleFromCenter();
    void testRectangleFromExtent();
    void testRectangleFrom3PointsDistance();
    void testRectangleFrom3PointsProjected();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mParentTool = nullptr;
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

  mParentTool = new QgsMapToolAddFeature( mCanvas, QgsMapToolCapture::CaptureLine );
}

void TestQgsMapToolRectangle::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolRectangle::testRectangleFromCenter()
{
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 333 );
  mLayer->startEditing();

  QgsMapToolRectangleCenter mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "LineStringZ (-2 -1 333, -2 1 333, 2 1 333, 2 -1 333, -2 -1 333)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 0 );
}

void TestQgsMapToolRectangle::testRectangleFromExtent()
{
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 222 );
  mLayer->startEditing();

  QgsMapToolRectangleExtent mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "LineStringZ (0 0 222, 0 1 222, 2 1 222, 2 0 222, 0 0 222)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 0 );
}

void TestQgsMapToolRectangle::testRectangleFrom3PointsDistance()
{
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 111 );
  mLayer->startEditing();

  QgsMapToolRectangle3Points mapTool( mParentTool, mCanvas, QgsMapToolRectangle3Points::DistanceMode );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 0 );
  utils.mouseClick( 2, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "LineStringZ (0 0 111, 2 0 111, 2 1 111, 0 1 111, 0 0 111)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 0 );
}

void TestQgsMapToolRectangle::testRectangleFrom3PointsProjected()
{
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 111 );
  mLayer->startEditing();

  QgsMapToolRectangle3Points mapTool( mParentTool, mCanvas, QgsMapToolRectangle3Points::ProjectedMode );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 0 );
  utils.mouseClick( 2, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  // QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "LineStringZ (0 0 111, 2 0 111, 2 1 111, 0 1 111, 0 0 111)";
  QgsLineString line;
  line.fromWkt( wkt );
  QVERIFY( static_cast<QgsLineString *>( f.geometry().get() )->equals( line ) );

  mLayer->rollBack();
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 0 );
}
QGSTEST_MAIN( TestQgsMapToolRectangle )
#include "testqgsmaptoolrectangle.moc"
