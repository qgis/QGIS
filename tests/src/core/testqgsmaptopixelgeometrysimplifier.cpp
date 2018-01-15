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
#include "qgstest.h"
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
#include "qgspoint.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgscircularstring.h"
#endif

//qgs unit test utility class
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
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
    void testCircularString();
    void testVisvalingam();

};

TestQgsMapToPixelGeometrySimplifier::TestQgsMapToPixelGeometrySimplifier() = default;

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
  QgsGeometry g;
  int fl = QgsMapToPixelSimplifier::SimplifyGeometry;
  QgsMapToPixelSimplifier simplifier( fl, 10.0 );
  QgsGeometry ret = simplifier.simplify( g );
  QVERIFY( ret.isNull() ); // not simplifiable
}

void TestQgsMapToPixelGeometrySimplifier::testLine1()
{
  // NOTE: we need more than 4 vertices, or the line will not be
  //       reduced at all by the algorithm
  QgsGeometry g( QgsGeometry::fromWkt( QStringLiteral( "LINESTRING(0 0,1 1,2 0,3 1,4 0,20 1,20 0,10 0,5 0)" ) ) );
  int fl;
  QString wkt;

  fl = QgsMapToPixelSimplifier::SimplifyGeometry;
  QgsMapToPixelSimplifier simplifier( fl, 10.0 );
  QgsGeometry ret = simplifier.simplify( g );
  wkt = ret.asWkt();
  // NOTE: vertex 1,1 is in this result, because we keep the first two or last two vertices in a line
  //       TO ensure that the angles at the line start and end are the same after simplification
  //       compared to what we have before
  QCOMPARE( wkt, QString( "LineString (0 0, 1 1, 20 1, 10 0, 5 0)" ) );

  simplifier.setSimplifyFlags( QgsMapToPixelSimplifier::SimplifyEnvelope );
  simplifier.setTolerance( 20.0 );
  ret = simplifier.simplify( g );
  wkt = ret.asWkt();
  // Cannot be simplified to its envelope since it's just at the setTollerance
  QCOMPARE( wkt, QString( "LineString (0 0, 1 1, 2 0, 3 1, 4 0, 20 1, 20 0, 10 0, 5 0)" ) );

  simplifier.setTolerance( 30.0 );
  ret = simplifier.simplify( g );
  wkt = ret.asWkt();
  // Got simplified into a line going from one corner of the envelope to the other
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
  // See https://issues.qgis.org/issues/12416
  // NOTE: the first line needs to be 5 vertices or more, or
  // simplification won't even be attempted
  const char *hexwkb = "010500000002000000010200008005000000000000000000000000000000000000000000000000000000000000000000F03F000000000000F03F00000000000000000000000000000040000000000000000000000000000000000000000000000840000000000000F03F0000000000000000000000000000244000000000000000008DEDB5A0F7C6B0BE010200008002000000000000000000000000000000000000000000000000000000000000000000000000000000000000008DEDB5A0F7C6B03E";
  int size;
  unsigned char *wkb = hex2bytes( hexwkb, &size );
  QgsGeometry g12416;
  // NOTE: wkb onwership transferred to QgsGeometry
  g12416.fromWkb( wkb, size );
  QString wkt = g12416.asWkt();
  QCOMPARE( wkt, QString( "MultiLineStringZ ((0 0 0, 1 1 0, 2 0 0, 3 1 0, 10 0 -0.000001),(0 0 0, 0 0 0.000001))" ) );

  int fl = QgsMapToPixelSimplifier::SimplifyGeometry;
  QgsMapToPixelSimplifier simplifier( fl, 20.0 );
  QgsGeometry ret = simplifier.simplify( g12416 );
  QVERIFY( !ret.equals( g12416 ) );
}

void TestQgsMapToPixelGeometrySimplifier::testCircularString()
{
  static const QString WKT( QStringLiteral( "MultiCurve (LineString (5 5, 3 5, 3 3, 0 3),CircularString (0 0, 2 1, 2 2))" ) );
  const QgsGeometry g( QgsGeometry::fromWkt( WKT ) );

  const QgsMapToPixelSimplifier simplifier( QgsMapToPixelSimplifier::SimplifyGeometry, 0.1 );
  QCOMPARE( simplifier.simplify( g ).asWkt(), WKT );
}

void TestQgsMapToPixelGeometrySimplifier::testVisvalingam()
{
  QString wkt( QStringLiteral( "LineString (0 0, 30 0, 31 30, 32 0, 40 0, 41 100, 42 0, 50 0)" ) );
  QgsGeometry g = QgsGeometry::fromWkt( wkt );

  const QgsMapToPixelSimplifier simplifier( QgsMapToPixelSimplifier::SimplifyGeometry, 7, QgsMapToPixelSimplifier::Visvalingam );
  QString expectedWkt( QStringLiteral( "LineString (0 0, 40 0, 41 100, 42 0, 50 0)" ) );

  QCOMPARE( simplifier.simplify( g ).asWkt(), expectedWkt );
}

QGSTEST_MAIN( TestQgsMapToPixelGeometrySimplifier )
#include "testqgsmaptopixelgeometrysimplifier.moc"
