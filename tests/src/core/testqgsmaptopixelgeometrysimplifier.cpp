/***************************************************************************
     testqgsmaptopixelgeometrysimplifier.cpp
     --------------------------------------
    Date                 : 20 Jan 2008
    Copyright            : (C) 2016 by Sandro Santilli
    Email                : strk @ keybit.net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QVector>
#include <QPointF>
#include <QImage>
#include <QPainter>

//qgis includes...
#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgsmaptopixelgeometrysimplifier.h>
#if 0
#include <qgspoint.h>
#include "qgsgeometryutils.h"
#include "qgspointv2.h"
#include "qgslinestringv2.h"
#include "qgspolygonv2.h"
#include "qgscircularstringv2.h"
#endif

//qgs unit test utility class
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for the QgsMapToPixelGeometrySimplifier class
 */
class TestQgsMapToPixelGeometrySimplifier : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToPixelGeometrySimplifier();

  private:
    // Release return with delete []
    unsigned char *
    hex2bytes( const char *hex, int *size )
    {
      QByteArray ba = QByteArray::fromHex( hex );
      unsigned char *out = new unsigned char[ba.size()];
      memcpy( out, ba.data(), ba.size() );
      *size = ba.size();
      return out;
    }

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testDefaultGeometry();
    void testLine1();
    void testIsGeneralizableByMapBoundingBox();
    void testWkbDimensionMismatch();

};

TestQgsMapToPixelGeometrySimplifier::TestQgsMapToPixelGeometrySimplifier()
{
}

void TestQgsMapToPixelGeometrySimplifier::initTestCase()
{
  // Runs once before any tests are run
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}


void TestQgsMapToPixelGeometrySimplifier::cleanupTestCase()
{
  // Runs once after all tests are run
  QgsApplication::exitQgis();
}

void TestQgsMapToPixelGeometrySimplifier::init()
{
  // will be called before every testfunction.
}

void TestQgsMapToPixelGeometrySimplifier::cleanup()
{
  // will be called after every testfunction.
}

void TestQgsMapToPixelGeometrySimplifier::testDefaultGeometry()
{
  QScopedPointer< QgsGeometry > g( new QgsGeometry() );
  int fl = QgsMapToPixelSimplifier::SimplifyGeometry;
  bool ret = QgsMapToPixelSimplifier::simplifyGeometry( g.data(), fl, 10.0 );
  QVERIFY( ! ret ); // not simplifiable
}

void TestQgsMapToPixelGeometrySimplifier::testLine1()
{
  // NOTE: we need more than 4 vertices, or the line will not be
  //       reduced at all by the algorithm
  QScopedPointer< QgsGeometry > g( QgsGeometry::fromWkt( "LINESTRING(0 0,1 1,2 0,3 1,4 0,20 1,20 0,10 0,5 0)" ) );
  int fl;
  bool ret;
  QString wkt;

  fl = QgsMapToPixelSimplifier::SimplifyGeometry;
  ret = QgsMapToPixelSimplifier::simplifyGeometry( g.data(), fl, 10.0 );
  QVERIFY( ret );
  wkt = g->exportToWkt();
  // NOTE: I don't think vertex 1,1 should be in this result,
  //       but that's what we get now
  QCOMPARE( wkt, QString( "LineString (0 0, 1 1, 20 1, 10 0, 5 0)" ) );

  fl = QgsMapToPixelSimplifier::SimplifyEnvelope;
  ret = QgsMapToPixelSimplifier::simplifyGeometry( g.data(), fl, 20.0 );
  QVERIFY( ret );
  wkt = g->exportToWkt();
  QCOMPARE( wkt, QString( "LineString (0 0, 1 1, 20 1, 10 0, 5 0)" ) );

  ret = QgsMapToPixelSimplifier::simplifyGeometry( g.data(), fl, 30.0 );
  QVERIFY( ret );
  wkt = g->exportToWkt();
  // NOTE: I don't understand this result either
  QCOMPARE( wkt, QString( "LineString (0 0, 20 1)" ) );
}

void
TestQgsMapToPixelGeometrySimplifier::testIsGeneralizableByMapBoundingBox()
{
  QgsRectangle r1( 0, 0, 10, 1 );
  bool ret;

  ret = QgsMapToPixelSimplifier::isGeneralizableByMapBoundingBox( r1, 15 );
  QVERIFY( ret );

  // NOTE: boundary case
  ret = QgsMapToPixelSimplifier::isGeneralizableByMapBoundingBox( r1, 10 );
  QVERIFY( ! ret );

  ret = QgsMapToPixelSimplifier::isGeneralizableByMapBoundingBox( r1, 5 );
  QVERIFY( ! ret );
}

void TestQgsMapToPixelGeometrySimplifier::testWkbDimensionMismatch()
{
  // 2D multilinestring containing 2 3DZ linestrings
  // See http://hub.qgis.org/issues/12416
  // NOTE: the first line needs to be 5 vertices or more, or
  // simplification won't even be attempted
  const char *hexwkb = "010500000002000000010200008005000000000000000000000000000000000000000000000000000000000000000000F03F000000000000F03F00000000000000000000000000000040000000000000000000000000000000000000000000000840000000000000F03F0000000000000000000000000000244000000000000000008DEDB5A0F7C6B0BE010200008002000000000000000000000000000000000000000000000000000000000000000000000000000000000000008DEDB5A0F7C6B03E";
  int size;
  unsigned char *wkb = hex2bytes( hexwkb, &size );
  QgsGeometry g12416;
  // NOTE: wkb onwership transferred to QgsGeometry
  g12416.fromWkb( wkb, size );
  QString wkt = g12416.exportToWkt();
  QCOMPARE( wkt, QString( "MultiLineStringZ ((0 0 0, 1 1 0, 2 0 0, 3 1 0, 10 0 -0.000001),(0 0 0, 0 0 0.000001))" ) );

  int fl = QgsMapToPixelSimplifier::SimplifyGeometry;
  int ret = QgsMapToPixelSimplifier::simplifyGeometry( &g12416, fl, 20.0 );
  QVERIFY( ret );
}

QTEST_MAIN( TestQgsMapToPixelGeometrySimplifier )
#include "testqgsmaptopixelgeometrysimplifier.moc"
