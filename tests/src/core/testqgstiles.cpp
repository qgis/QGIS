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
#include "qgscoordinatetransform.h"

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

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_matrixFromWebMercator();
    void test_matrixFromCustomDef();
    void test_matrixFromTileMatrix();
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
  QCOMPARE( tm.crs().authid(), "EPSG:3857" );
  QCOMPARE( tm.zoomLevel(), 0 );
  QCOMPARE( tm.matrixWidth(), 1 );
  QCOMPARE( tm.matrixHeight(), 1 );
  QVERIFY( qgsDoubleNear( tm.extent().xMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().xMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.scale(), 559082264.0287178, 0.00001 ) );

  tm = QgsTileMatrix::fromWebMercator( 3 );
  QCOMPARE( tm.crs().authid(), "EPSG:3857" );
  QCOMPARE( tm.zoomLevel(), 3 );
  QCOMPARE( tm.matrixWidth(), 8 );
  QCOMPARE( tm.matrixHeight(), 8 );
  QVERIFY( qgsDoubleNear( tm.extent().xMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().xMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.scale(), 559082264.0287178 / 8.0, 0.00001 ) );

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

void TestQgsTiles::test_matrixFromCustomDef()
{
  // Try to build the Web Mercator tile matrix
  QgsTileMatrix tm = QgsTileMatrix::fromCustomDef( 0, QgsCoordinateReferenceSystem( "EPSG:3857" ), QgsPointXY( -20037508.3427892, 20037508.3427892 ), 40075016.6855784 );
  QCOMPARE( tm.crs().authid(), "EPSG:3857" );
  QCOMPARE( tm.zoomLevel(), 0 );
  QCOMPARE( tm.matrixWidth(), 1 );
  QCOMPARE( tm.matrixHeight(), 1 );
  QVERIFY( qgsDoubleNear( tm.extent().xMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().xMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.scale(), 559082264.0287178, 0.00001 ) );

  tm = QgsTileMatrix::fromCustomDef( 3, QgsCoordinateReferenceSystem( "EPSG:3857" ), QgsPointXY( -20037508.3427892, 20037508.3427892 ), 40075016.6855784 );
  QCOMPARE( tm.crs().authid(), "EPSG:3857" );
  QCOMPARE( tm.zoomLevel(), 3 );
  QCOMPARE( tm.matrixWidth(), 8 );
  QCOMPARE( tm.matrixHeight(), 8 );
  QVERIFY( qgsDoubleNear( tm.extent().xMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().xMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm.scale(), 559082264.0287178 / 8.0, 0.00001 ) );

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

  // Try to build the World Global tile matrix
  tm = QgsTileMatrix::fromCustomDef( 0, QgsCoordinateReferenceSystem( "EPSG:4326" ), QgsPointXY( -180.0, 90.0 ), 180.0, 2, 1 );
  QCOMPARE( tm.crs().authid(), "EPSG:4326" );
  QCOMPARE( tm.zoomLevel(), 0 );
  QCOMPARE( tm.matrixWidth(), 2 );
  QCOMPARE( tm.matrixHeight(), 1 );
  QVERIFY( qgsDoubleNear( tm.extent().xMinimum(), -180.0, 0.1 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMinimum(), -90.0, 0.1 ) );
  QVERIFY( qgsDoubleNear( tm.extent().xMaximum(), 180.0, 0.1 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMaximum(), 90.0, 0.1 ) );
  QVERIFY( qgsDoubleNear( tm.scale(), 279541132.014, 0.001 ) );

  tm = QgsTileMatrix::fromCustomDef( 3, QgsCoordinateReferenceSystem( "EPSG:4326" ), QgsPointXY( -180.0, 90.0 ), 180.0, 2, 1 );
  QCOMPARE( tm.crs().authid(), "EPSG:4326" );
  QCOMPARE( tm.zoomLevel(), 3 );
  QCOMPARE( tm.matrixWidth(), 16 );
  QCOMPARE( tm.matrixHeight(), 8 );
  QVERIFY( qgsDoubleNear( tm.extent().xMinimum(), -180.0, 0.1 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMinimum(), -90.0, 0.1 ) );
  QVERIFY( qgsDoubleNear( tm.extent().xMaximum(), 180.0, 0.1 ) );
  QVERIFY( qgsDoubleNear( tm.extent().yMaximum(), 90.0, 0.1 ) );
  QVERIFY( qgsDoubleNear( tm.scale(), 279541132.014 / 8.0, 0.001 ) );

  te = tm.tileExtent( QgsTileXYZ( 3, 3, 3 ) );
  QVERIFY( qgsDoubleNear( te.xMinimum(), -112.5, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.yMinimum(), 0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.xMaximum(), -90, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.yMaximum(), 22.5, 0.0000001 ) );

  tc = tm.tileCenter( QgsTileXYZ( 3, 3, 3 ) );
  QVERIFY( qgsDoubleNear( tc.x(), te.xMinimum() + te.width() / 2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.x(), -101.25, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.y(), te.yMinimum() + te.height() / 2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.y(), 11.25, 0.0000001 ) );

  destCrs = QgsCoordinateReferenceSystem( "EPSG:4326" );
  ct = QgsCoordinateTransform( vl->crs(), destCrs, QgsCoordinateTransformContext() );
  r = ct.transformBoundingBox( vl->extent() );
  tr = tm.tileRangeFromExtent( r );
  QVERIFY( tr.isValid() );
  QCOMPARE( tr.startColumn(), 2 );
  QCOMPARE( tr.endColumn(), 4 );
  QCOMPARE( tr.startRow(), 1 );
  QCOMPARE( tr.endRow(), 2 );
}

void TestQgsTiles::test_matrixFromTileMatrix()
{
  QgsTileMatrix tm = QgsTileMatrix::fromWebMercator( 0 );

  // Build level 3 from 0
  QgsTileMatrix tm3 = QgsTileMatrix::fromTileMatrix( 3, tm );
  QCOMPARE( tm3.crs().authid(), "EPSG:3857" );
  QCOMPARE( tm3.zoomLevel(), 3 );
  QCOMPARE( tm3.matrixWidth(), 8 );
  QCOMPARE( tm3.matrixHeight(), 8 );
  QVERIFY( qgsDoubleNear( tm3.extent().xMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm3.extent().yMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm3.extent().xMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm3.extent().yMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm3.scale(), 559082264.0287178 / 8.0, 0.00001 ) );

  QgsRectangle te = tm3.tileExtent( QgsTileXYZ( 3, 3, 3 ) );
  QVERIFY( qgsDoubleNear( te.xMinimum(), -5009377.0856973, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.yMinimum(), 0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.xMaximum(), 0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( te.yMaximum(), 5009377.0856973, 0.0000001 ) );

  QgsPointXY tc = tm3.tileCenter( QgsTileXYZ( 3, 3, 3 ) );
  QVERIFY( qgsDoubleNear( tc.x(), te.xMinimum() + te.width() / 2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.x(), -2504688.54284865, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.y(), te.yMinimum() + te.height() / 2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tc.y(), 2504688.54284865, 0.0000001 ) );

  // Build level 0 from 3
  QgsTileMatrix tm0 = QgsTileMatrix::fromTileMatrix( 0, tm3 );
  QCOMPARE( tm0.crs().authid(), "EPSG:3857" );
  QCOMPARE( tm0.zoomLevel(), 0 );
  QCOMPARE( tm0.matrixWidth(), 1 );
  QCOMPARE( tm0.matrixHeight(), 1 );
  QVERIFY( qgsDoubleNear( tm0.extent().xMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm0.extent().yMinimum(), -20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm0.extent().xMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm0.extent().yMaximum(), 20037508.3427892, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( tm0.scale(), 559082264.0287178, 0.00001 ) );
}


QGSTEST_MAIN( TestQgsTiles )
#include "testqgstiles.moc"
