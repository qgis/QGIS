/***************************************************************************
     testqgscadutils.cpp
     --------------------------------------
    Date                 : September 2017
    Copyright            : (C) 2017 by Martin Dobias
    Email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>

#include "qgscadutils.h"
#include "qgsproject.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the QgsCadUtils class.
 */
class TestQgsCadUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsCadUtils() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testBasic();
    void testXY();
    void testAngle();
    void testCommonAngle();
    void testDistance();
    void testEdge();

  private:

    QgsCadUtils::AlignMapPointContext baseContext()
    {
      QgsCadUtils::AlignMapPointContext context;
      context.snappingUtils = mSnappingUtils;
      context.mapUnitsPerPixel = mMapSettings.mapUnitsPerPixel();
      context.setCadPoints( QList<QgsPoint>() << QgsPoint() << QgsPoint( 30, 20 ) << QgsPoint( 30, 30 ) );
      return context;
    }

    QString mTestDataDir;
    QgsVectorLayer *mLayerPolygon = nullptr;
    QgsSnappingUtils *mSnappingUtils = nullptr;
    QgsMapSettings mMapSettings;
};


//runs before all tests
void TestQgsCadUtils::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mLayerPolygon = new QgsVectorLayer( "Polygon?crs=EPSG:27700", "layer polygon", "memory" );
  QVERIFY( mLayerPolygon->isValid() );

  QgsPolygonXY polygon1;
  QgsPolylineXY polygon1exterior;
  polygon1exterior << QgsPointXY( 10, 10 ) << QgsPointXY( 30, 10 ) << QgsPointXY( 10, 20 ) << QgsPointXY( 10, 10 );
  polygon1 << polygon1exterior;
  QgsFeature polygonF1;
  polygonF1.setGeometry( QgsGeometry::fromPolygonXY( polygon1 ) );

  mLayerPolygon->startEditing();
  mLayerPolygon->addFeature( polygonF1 );

  QgsProject::instance()->addMapLayer( mLayerPolygon );

  QgsSnappingConfig snapConfig;
  snapConfig.setEnabled( true );
  snapConfig.setMode( Qgis::SnappingMode::AllLayers );
  snapConfig.setTypeFlag( static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ) );
  snapConfig.setTolerance( 1.0 );

  mMapSettings.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  mMapSettings.setOutputSize( QSize( 100, 100 ) );
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mLayerPolygon );

  mSnappingUtils = new QgsSnappingUtils;
  mSnappingUtils->setConfig( snapConfig );
  mSnappingUtils->setMapSettings( mMapSettings );

  mSnappingUtils->locatorForLayer( mLayerPolygon )->init();
}

//runs after all tests
void TestQgsCadUtils::cleanupTestCase()
{
  delete mSnappingUtils;

  QgsApplication::exitQgis();
}

void TestQgsCadUtils::testBasic()
{
  const QgsCadUtils::AlignMapPointContext context( baseContext() );

  // no snap
  const QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 5, 5 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 5, 5 ) );

  // simple snap to vertex
  const QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 9.5, 9.5 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 10, 10 ) );
}

void TestQgsCadUtils::testXY()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );

  // x absolute
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 20 );
  const QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 20, 29 ) );

  // x relative
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, -5 );
  const QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 25, 29 ) );

  context.xConstraint = QgsCadUtils::AlignMapPointConstraint();

  // y absolute
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 20 );
  const QgsCadUtils::AlignMapPointOutput res2 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res2.valid );
  QCOMPARE( res2.finalMapPoint, QgsPointXY( 29, 20 ) );

  // y relative
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, -5 );
  const QgsCadUtils::AlignMapPointOutput res3 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res3.valid );
  QCOMPARE( res3.finalMapPoint, QgsPointXY( 29, 15 ) );

  // x and y (relative)
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 32 );
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 22 );
  const QgsCadUtils::AlignMapPointOutput res4 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res4.valid );
  QCOMPARE( res4.finalMapPoint, QgsPointXY( 32, 22 ) );

  // x and y (relative)
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, -2 );
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, -2 );
  const QgsCadUtils::AlignMapPointOutput res5 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res5.valid );
  QCOMPARE( res5.finalMapPoint, QgsPointXY( 28, 18 ) );
}

void TestQgsCadUtils::testAngle()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );

  // angle abs
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  const QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 35, 25 ) );

  // angle rel
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, 45 );
  const QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 30, 30 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 25, 25 ) );

  // angle + x abs
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 38 );
  const QgsCadUtils::AlignMapPointOutput res2 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20 ), context );
  QVERIFY( res2.valid );
  QCOMPARE( res2.finalMapPoint, QgsPointXY( 38, 28 ) );

  // angle + y rel
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 17 );
  const QgsCadUtils::AlignMapPointOutput res3 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20 ), context );
  QVERIFY( res3.valid );
  QCOMPARE( res3.finalMapPoint, QgsPointXY( 27, 17 ) );
}

void TestQgsCadUtils::testCommonAngle()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );

  // without common angle
  const QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20.1 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.softLockCommonAngle, -1.0 );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 40, 20.1 ) );

  // common angle
  context.commonAngleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 90 );
  const QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20.1 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.softLockCommonAngle, 0.0 );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 40, 20 ) );

  // common angle + angle  (make sure that angle constraint has priority)
  context.commonAngleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 90 );
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  const QgsCadUtils::AlignMapPointOutput res2 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20.1 ), context );
  QVERIFY( res2.valid );
  QCOMPARE( res2.softLockCommonAngle, -1.0 );
  QCOMPARE( res2.finalMapPoint, QgsPointXY( 35.05, 25.05 ) );

  // common angle rel
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.commonAngleConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, 90 );
  context.setCadPoint( 1, QgsPoint( 40, 20 ) );
  QgsCadUtils::AlignMapPointOutput res3 = QgsCadUtils::alignMapPoint( QgsPointXY( 50.1, 29.9 ), context );
  QVERIFY( res3.valid );
  QCOMPARE( res3.softLockCommonAngle, 90.0 );
  QCOMPARE( res3.finalMapPoint, QgsPointXY( 50, 30 ) );
}

void TestQgsCadUtils::testDistance()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );
  const double distance = 5.0;
  // without distance constraint
  const QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 45, 20 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.softLockCommonAngle, -1.0 );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 45, 20 ) );

  // dist
  context.distanceConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, distance );
  const QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 45, 20 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 35, 20 ) );

  // dist+x
  const double d = distance * sqrt( 2 ) / 2.;   // sine/cosine of 45 times radius of our distance constraint
  const double expectedX1 = 30 + d;
  const double expectedY1 = 20 + d;
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, expectedX1 );
  const QgsCadUtils::AlignMapPointOutput res2 = QgsCadUtils::alignMapPoint( QgsPointXY( 45, 25 ), context );
  QVERIFY( res2.valid );
  QCOMPARE( res2.finalMapPoint, QgsPointXY( expectedX1, expectedY1 ) );

  // dist+x invalid
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 1000 );
  const QgsCadUtils::AlignMapPointOutput res2x = QgsCadUtils::alignMapPoint( QgsPointXY( 45, 25 ), context );
  QVERIFY( !res2x.valid );

  // dist+rel x
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, 0 );
  const QgsCadUtils::AlignMapPointOutput res2r = QgsCadUtils::alignMapPoint( QgsPointXY( 45, 25 ), context );
  QVERIFY( res2r.valid );
  QCOMPARE( res2r.finalMapPoint, QgsPointXY( 30, 20 + distance ) );

  // dist+y
  const double expectedX2 = 30 + d;
  const double expectedY2 = 20 - d;
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, expectedY2 );
  const QgsCadUtils::AlignMapPointOutput res3 = QgsCadUtils::alignMapPoint( QgsPointXY( 45, 15 ), context );
  QVERIFY( res3.valid );
  QCOMPARE( res3.finalMapPoint, QgsPointXY( expectedX2, expectedY2 ) );

  // dist+y invalid
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 1000 );
  const QgsCadUtils::AlignMapPointOutput res3x = QgsCadUtils::alignMapPoint( QgsPointXY( 45, 15 ), context );
  QVERIFY( !res3x.valid );

  // dist+rel y
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, 0 );
  const QgsCadUtils::AlignMapPointOutput res3r = QgsCadUtils::alignMapPoint( QgsPointXY( 45, 15 ), context );
  QVERIFY( res3r.valid );
  QCOMPARE( res3r.finalMapPoint, QgsPointXY( 30 + distance, 20 ) );

  // dist+angle
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  const QgsCadUtils::AlignMapPointOutput res4 = QgsCadUtils::alignMapPoint( QgsPointXY( 25, 15 ), context );
  QVERIFY( res4.valid );
  QCOMPARE( res4.finalMapPoint, QgsPointXY( 30 - d, 20 - d ) );
}

void TestQgsCadUtils::testEdge()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );
  context.setCadPoints( QList<QgsPoint>() << QgsPoint() << QgsPoint( 40, 30 ) << QgsPoint( 40, 40 ) );

  const QgsPointXY edgePt( 20, 15 );  // in the middle of the triangle polygon's edge

  // x+edge
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 40 );
  const QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( edgePt, context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 40, 5 ) );

  // y+edge
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 30 );
  const QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( edgePt, context );
  QVERIFY( res1.valid );
  //qDebug() << res1.finalMapPoint.toString();
  QCOMPARE( res1.finalMapPoint, QgsPointXY( -10, 30 ) );

  // angle+edge
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 90 );
  const QgsCadUtils::AlignMapPointOutput res2 = QgsCadUtils::alignMapPoint( edgePt, context );
  QVERIFY( res2.valid );
  QCOMPARE( res2.finalMapPoint, QgsPointXY( 40, 5 ) );

  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 0 );
  const QgsCadUtils::AlignMapPointOutput res3 = QgsCadUtils::alignMapPoint( edgePt, context );
  QVERIFY( res3.valid );
  QCOMPARE( res3.finalMapPoint, QgsPointXY( -10, 30 ) );

  // distance+edge
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.distanceConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 50 );
  const QgsCadUtils::AlignMapPointOutput res4 = QgsCadUtils::alignMapPoint( edgePt, context );
  QVERIFY( res4.valid );
  qDebug() << res4.finalMapPoint.toString();
  // there is a tiny numerical error, so exact test with QgsPointXY does not work here
  QCOMPARE( res4.finalMapPoint.x(), -10. );
  QCOMPARE( res4.finalMapPoint.y(), 30. );
}

QGSTEST_MAIN( TestQgsCadUtils )

#include "testqgscadutils.moc"
