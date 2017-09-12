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

/** \ingroup UnitTests
 * This is a unit test for the QgsCadUtils class.
 */
class TestQgsCadUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsCadUtils()
    {}
    ~TestQgsCadUtils()
    {
    }

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
      context.cadPointList << QgsPointXY() << QgsPointXY( 30, 20 ) << QgsPointXY( 30, 30 );
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

  QgsPolygon polygon1;
  QgsPolyline polygon1exterior;
  polygon1exterior << QgsPointXY( 10, 10 ) << QgsPointXY( 30, 10 ) << QgsPointXY( 10, 20 ) << QgsPointXY( 10, 10 );
  polygon1 << polygon1exterior;
  QgsFeature polygonF1;
  polygonF1.setGeometry( QgsGeometry::fromPolygon( polygon1 ) );

  mLayerPolygon->startEditing();
  mLayerPolygon->addFeature( polygonF1 );

  QgsProject::instance()->addMapLayer( mLayerPolygon );

  QgsSnappingConfig snapConfig;
  snapConfig.setEnabled( true );
  snapConfig.setMode( QgsSnappingConfig::AllLayers );
  snapConfig.setType( QgsSnappingConfig::VertexAndSegment );
  snapConfig.setTolerance( 1.0 );

  mMapSettings.setExtent( QgsRectangle( 0, 0, 100, 100 ) );
  mMapSettings.setOutputSize( QSize( 100, 100 ) );
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mLayerPolygon );

  mSnappingUtils = new QgsSnappingUtils;
  mSnappingUtils->setConfig( snapConfig );
  mSnappingUtils->setMapSettings( mMapSettings );
}

//runs after all tests
void TestQgsCadUtils::cleanupTestCase()
{
  delete mSnappingUtils;

  QgsApplication::exitQgis();
}

void TestQgsCadUtils::testBasic()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );

  // no snap
  QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 5, 5 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 5, 5 ) );

  // simple snap to vertex
  QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 9.5, 9.5 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 10, 10 ) );
}

void TestQgsCadUtils::testXY()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );

  // x absolute
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 20 );
  QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 20, 29 ) );

  // x relative
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, -5 );
  QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 25, 29 ) );

  context.xConstraint = QgsCadUtils::AlignMapPointConstraint();

  // y absolute
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 20 );
  QgsCadUtils::AlignMapPointOutput res2 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res2.valid );
  QCOMPARE( res2.finalMapPoint, QgsPointXY( 29, 20 ) );

  // y relative
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, -5 );
  QgsCadUtils::AlignMapPointOutput res3 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res3.valid );
  QCOMPARE( res3.finalMapPoint, QgsPointXY( 29, 15 ) );

  // x and y (relative)
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 32 );
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 22 );
  QgsCadUtils::AlignMapPointOutput res4 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res4.valid );
  QCOMPARE( res4.finalMapPoint, QgsPointXY( 32, 22 ) );

  // x and y (relative)
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, -2 );
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, -2 );
  QgsCadUtils::AlignMapPointOutput res5 = QgsCadUtils::alignMapPoint( QgsPointXY( 29, 29 ), context );
  QVERIFY( res5.valid );
  QCOMPARE( res5.finalMapPoint, QgsPointXY( 28, 18 ) );
}

void TestQgsCadUtils::testAngle()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );

  // angle abs
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 35, 25 ) );

  // angle rel
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, 45 );
  QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 30, 30 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 25, 25 ) );

  // angle + x abs
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 38 );
  QgsCadUtils::AlignMapPointOutput res2 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20 ), context );
  QVERIFY( res2.valid );
  QCOMPARE( res2.finalMapPoint, QgsPointXY( 38, 28 ) );

  // angle + y rel
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  context.xConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.yConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 17 );
  QgsCadUtils::AlignMapPointOutput res3 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20 ), context );
  QVERIFY( res3.valid );
  QCOMPARE( res3.finalMapPoint, QgsPointXY( 27, 17 ) );
}

void TestQgsCadUtils::testCommonAngle()
{
  QgsCadUtils::AlignMapPointContext context( baseContext() );

  // without common angle
  QgsCadUtils::AlignMapPointOutput res0 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20.1 ), context );
  QVERIFY( res0.valid );
  QCOMPARE( res0.softLockCommonAngle, -1 );
  QCOMPARE( res0.finalMapPoint, QgsPointXY( 40, 20.1 ) );

  // common angle
  context.commonAngleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 90 );
  QgsCadUtils::AlignMapPointOutput res1 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20.1 ), context );
  QVERIFY( res1.valid );
  QCOMPARE( res1.softLockCommonAngle, 0 );
  QCOMPARE( res1.finalMapPoint, QgsPointXY( 40, 20 ) );

  // common angle + angle  (make sure that angle constraint has priority)
  context.commonAngleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 90 );
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint( true, false, 45 );
  QgsCadUtils::AlignMapPointOutput res2 = QgsCadUtils::alignMapPoint( QgsPointXY( 40, 20.1 ), context );
  QVERIFY( res2.valid );
  QCOMPARE( res2.softLockCommonAngle, -1 );
  QCOMPARE( res2.finalMapPoint, QgsPointXY( 35.05, 25.05 ) );

  // common angle rel
  context.angleConstraint = QgsCadUtils::AlignMapPointConstraint();
  context.commonAngleConstraint = QgsCadUtils::AlignMapPointConstraint( true, true, 90 );
  context.cadPointList[1] = QgsPointXY( 40, 20 );
  QgsCadUtils::AlignMapPointOutput res3 = QgsCadUtils::alignMapPoint( QgsPointXY( 50.1, 29.9 ), context );
  QVERIFY( res3.valid );
  QCOMPARE( res3.softLockCommonAngle, 90 );
  QCOMPARE( res3.finalMapPoint, QgsPointXY( 50, 30 ) );
}

void TestQgsCadUtils::testDistance()
{
  // TODO:
  // dist
  // dist+x / dist+y
  // dist+angle
}

void TestQgsCadUtils::testEdge()
{
  // TODO:
  // x+edge / y+edge
  // angle+edge
  // distance+edge
}

QGSTEST_MAIN( TestQgsCadUtils )

#include "testqgscadutils.moc"
