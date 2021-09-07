/***************************************************************************
  testqgstiles.cpp
  --------------------------------------
  Date                 : August 2021
  Copyright            : (C) 2021 by René-Luc Dhont
  Email                : rldhont at gmail dot com
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

//qgis includes...
#include "qgsapplication.h"
#include "qgstiles.h"
#include "qgsvectorlayer.h"

/**
 * \ingroup UnitTests
 * This is a unit test for vector tiles
 */
class TestQgsTiles : public QObject
{
    Q_OBJECT

  public:
    TestQgsTiles() = default;

  private:
    QString mDataDir;
    QString mReport;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_matrixFromWebMercator();
};


void TestQgsTiles::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
}

void TestQgsTiles::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


void TestQgsTiles::test_matrixFromWebMercator()
{
  QgsTileMatrix tm = QgsTileMatrix::fromWebMercator( 0 );
  QCOMPARE( tm.zoomLevel(), 0 );
  QCOMPARE( tm.matrixWidth(), 1 );
  QCOMPARE( tm.matrixHeight(), 1 );
  QVERIFY( qgsDoubleNear( tm.extent().xMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().xMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.scale(), 559082264.0287178, 0.0000001 ) );

  tm = QgsTileMatrix::fromWebMercator( 3 );
  QCOMPARE( tm.zoomLevel(), 3 );
  QCOMPARE( tm.matrixWidth(), 8 );
  QCOMPARE( tm.matrixHeight(), 8 );
  QVERIFY( qgsDoubleNear( tm.extent().xMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().xMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.scale(), 559082264.0287178 / 8.0, 0.0000001 ) );

  QgsRectangle te = tm.tileExtent( QgsTileXYZ( 3, 3, 3 ) );
  QVERIFY( qgsDoubleNear( te.xMinimum(), -5009377.0856973, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.yMinimum(), 0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.xMaximum(), 0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.yMaximum(), 5009377.0856973, 0.0000001 ) );

  QgsPointXY tc = tm.tileCenter( QgsTileXYZ( 3, 3, 3 ) );
  QVERIFY( qgsDoubleNear( tc.x(), te.xMinimum() + te.width() / 2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.x(), -2504688.54284865, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.y(), te.yMinimum() + te.height() / 2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.y(), 2504688.54284865, 0.0000001 ) );

  QgsVectorLayer *vl = new QgsVectorLayer( mDataDir + "/points.shp", "points", "ogr" );
  QgsCoordinateReferenceSystem destCrs( "EPSG:3857" );
  QgsCoordinateTransform ct( vl->crs(), destCrs, QgsCoordinateTransformContext() );
  QgsRectangle r = ct.transformBoundingBox( vl->extent() );
  QgsTileRange tr = tm.tileRangeFromExtent( r );
  QVERIFY( tr.isValid() );
  QCOMPARE( tr.startColumn(), 1 );
  QCOMPARE( tr.endColumn(), 2 );
  QCOMPARE( tr.startRow(), 2 );
  QCOMPARE( tr.endRow(), 3 );
}


QGSTEST_MAIN( TestQgsTiles )
#include "testqgstiles.moc"
