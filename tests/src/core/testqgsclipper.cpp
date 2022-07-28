/***************************************************************************
     testqgsclipper.cpp
     --------------------------------------
    Date                 : Tue 14 Aug 2012
    Copyright            : (C) 2012 by Magnus Homann
    Email                : magnus at homann dot se
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qgsapplication.h>
//header for class being tested
#include <qgsclipper.h>
#include <qgspoint.h>
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgslinesymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmultirenderchecker.h"

class TestQgsClipper: public QObject
{

    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void basic();
    void basicWithZ();
    void basicWithZInf();
    void epsg4978LineRendering();

  private:
    QString mReport;

    bool checkBoundingBox( const QPolygonF &polygon, const QgsRectangle &clipRect );
    bool checkBoundingBox( const QgsLineString &polygon, const QgsBox3d &clipRect );
    bool render2dCheck( const QString &testName, QgsVectorLayer *layer, QgsRectangle extent );
};

void TestQgsClipper::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsClipper::initTestCase()
{
  mReport = QStringLiteral( "<h1>Clipper Rendering Tests</h1>\n" );

}

void TestQgsClipper::basicWithZ()
{
  // QgsClipper is static only
  QgsBox3d clipRect( 10, 10, 11, 25, 30, 19 );

  QVector< double > x;
  QVector< double > y;
  QVector< double > z;
  // check that clipping an empty polygon doesn't crash
  QgsClipper::trimPolygon( x, y, z, clipRect );

  x = { 10.4, 20.2 };
  y = { 20.5, 30.2 };
  z = { 10.0, 20.0 };
  QgsClipper::trimPolygon( x, y, z, clipRect );

  // Check nothing sticks out.
  QVERIFY( checkBoundingBox( QgsLineString( x, y, z ), clipRect ) );
  // Check that it didn't clip too much
  QgsBox3d clipRectInner( clipRect );
  clipRectInner.scale( 0.999 );
  QVERIFY( ! checkBoundingBox( QgsLineString( x, y, z ), clipRectInner ) );

  // A more complex example
  x = { 1.0, 11.0, 9.0 };
  y = { 9.0, 11.0, 1.0 };
  z = { 1.0, 11.0, 9.0 };
  clipRect = QgsBox3d( 0.0, 0.0, 0.0, 10.0, 10.0, 10.0 );

  QgsClipper::trimPolygon( x, y, z, clipRect );

  // We should have 5 vertices now?
  QCOMPARE( x.size(), 5 );
  QCOMPARE( y.size(), 5 );
  QCOMPARE( z.size(), 5 );
  // Check nothing sticks out.
  QVERIFY( checkBoundingBox( QgsLineString( x, y, z ), clipRect ) );
  // Check that it didn't clip too much
  clipRectInner = clipRect;
  clipRectInner.scale( 0.999 );
  QVERIFY( ! checkBoundingBox( QgsLineString( x, y, z ), clipRectInner ) );
}

void TestQgsClipper::basicWithZInf()
{
  // QgsClipper is static only

  QVector< double > x { 10.4, 20.2 };
  QVector< double > y { 20.5, 30.2 };
  QVector< double > z { 10.0, 20.0 };

  QgsBox3d clipRect( 10, 10, -HUGE_VAL, 25, 30, HUGE_VAL );

  QgsClipper::trimPolygon( x, y, z, clipRect );

  // Check nothing sticks out.
  QVERIFY( checkBoundingBox( QgsLineString( x, y, z ), clipRect ) );
  // Check that it didn't clip too much
  QgsBox3d clipRectInner( clipRect );
  clipRectInner.scale( 0.999 );
  QVERIFY( ! checkBoundingBox( QgsLineString( x, y, z ), clipRectInner ) );

  // A more complex example
  x = { 1.0, 11.0, 9.0 };
  y = { 9.0, 11.0, 1.0 };
  z = { 1.0, 11.0, 9.0 };
  clipRect = QgsBox3d( 0.0, 0.0, 0.0, 10.0, 10.0, 10.0 );

  QgsClipper::trimPolygon( x, y, z, clipRect );

  // We should have 5 vertices now?
  QCOMPARE( x.size(), 5 );
  QCOMPARE( y.size(), 5 );
  QCOMPARE( z.size(), 5 );
  // Check nothing sticks out.
  QVERIFY( checkBoundingBox( QgsLineString( x, y, z ), clipRect ) );
  // Check that it didn't clip too much
  clipRectInner = clipRect;
  clipRectInner.scale( 0.999 );
  QVERIFY( ! checkBoundingBox( QgsLineString( x, y, z ), clipRectInner ) );
}

void TestQgsClipper::basic()
{
  // QgsClipper is static only

  QPolygonF polygon;
  polygon << QPointF( 10.4, 20.5 ) << QPointF( 20.2, 30.2 );

  QgsRectangle clipRect( 10, 10, 25, 30 );

  QgsClipper::trimPolygon( polygon, clipRect );

  // Check nothing sticks out.
  QVERIFY( checkBoundingBox( polygon, clipRect ) );
  // Check that it didn't clip too much
  QgsRectangle clipRectInner( clipRect );
  clipRectInner.scale( 0.999 );
  QVERIFY( ! checkBoundingBox( polygon, clipRectInner ) );

  // A more complex example
  polygon.clear();
  polygon << QPointF( 1.0, 9.0 ) << QPointF( 11.0, 11.0 ) << QPointF( 9.0, 1.0 );
  clipRect.set( 0.0, 0.0, 10.0, 10.0 );

  QgsClipper::trimPolygon( polygon, clipRect );

  // We should have 5 vertices now?
  QCOMPARE( polygon.size(), 5 );
  // Check nothing sticks out.
  QVERIFY( checkBoundingBox( polygon, clipRect ) );
  // Check that it didn't clip too much
  clipRectInner = clipRect;
  clipRectInner.scale( 0.999 );
  QVERIFY( ! checkBoundingBox( polygon, clipRectInner ) );
}

bool TestQgsClipper::checkBoundingBox( const QgsLineString &polygon, const QgsBox3d &clipRect )
{
  return clipRect.contains( polygon.calculateBoundingBox3d() );
}

bool TestQgsClipper::checkBoundingBox( const QPolygonF &polygon, const QgsRectangle &clipRect )
{
  const QgsRectangle bBox( polygon.boundingRect() );

  return clipRect.contains( bBox );
}

void TestQgsClipper::epsg4978LineRendering()
{
  QgsVectorLayer *layerLines = new QgsVectorLayer( QString( TEST_DATA_DIR ) + "/3d/earth_size_sphere_4978.gpkg", "lines", "ogr" );

  QgsLineSymbol *fillSymbol = new QgsLineSymbol();
  fillSymbol->setWidth( 0.5 );
  fillSymbol->setColor( QColor( 255, 0, 0 ) );
  layerLines->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVERIFY( render2dCheck( "4978_2D_line_rendering", layerLines, QgsRectangle( -2.5e7, -2.5e7, 2.5e7, 2.5e7 ) ) );

  delete layerLines;
}

bool TestQgsClipper::render2dCheck( const QString &testName, QgsVectorLayer *layer, QgsRectangle extent )
{
  mReport += "<h2>" + testName + "</h2>\n";

  const QString myTmpDir = QDir::tempPath() + '/';
  const QString myFileName = myTmpDir + testName + ".png";

  auto mMapSettings = new QgsMapSettings();
  mMapSettings->setLayers( QList<QgsMapLayer *>() << layer );
  mMapSettings->setExtent( extent );
  mMapSettings->setOutputDpi( 96 );
  QgsCoordinateReferenceSystem newCrs;
  newCrs.createFromString( "EPSG:3857" );
  mMapSettings->setDestinationCrs( newCrs );

  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "3d" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setRenderedImage( myFileName );
  myChecker.setColorTolerance( 50 );
  const bool myResultFlag = myChecker.runTest( testName, 0 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsClipper )
#include "testqgsclipper.moc"
