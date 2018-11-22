/***************************************************************************
     testqgsmaptoolcircularstring.cpp
     --------------------------------
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

#include "testqgsmaptoolutils.h"
#include "qgsmaptoolcircularstringcurvepoint.h"
#include "qgsmaptoolcircularstringradius.h"


class TestQgsMapToolCircularString : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolCircularString();

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testAddCircularStringCurvePoint();
    void testAddCircularStringRadius();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mParentTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

TestQgsMapToolCircularString::TestQgsMapToolCircularString() = default;


//runs before all tests
void TestQgsMapToolCircularString::initTestCase()
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

void TestQgsMapToolCircularString::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolCircularString::testAddCircularStringCurvePoint()
{
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 333 );
  mLayer->startEditing();

  QgsMapToolCircularStringCurvePoint mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "CompoundCurveZ (CircularStringZ (0 0 333, 1 1 333, 0 2 333))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 0 );
}

void TestQgsMapToolCircularString::testAddCircularStringRadius()
{
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 111 );
  mLayer->startEditing();

  QgsMapToolCircularStringRadius mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  QgsFeature f = mLayer->getFeature( newFid );

  QString wkt = "CompoundCurveZ (CircularStringZ (0 0 111, 0.17912878474779187 0.82087121525220819 111, 1 1 111))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 0 );
}

QGSTEST_MAIN( TestQgsMapToolCircularString )
#include "testqgsmaptoolcircularstring.moc"
