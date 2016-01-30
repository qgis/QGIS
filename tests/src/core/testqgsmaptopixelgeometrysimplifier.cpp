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

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testDefaultGeometry();
    void testLine1();
    void testIsGeneralizableByMapBoundingBox();
    void testMismatchWKB();

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
  QgsGeometry *g = new QgsGeometry();
  int fl = QgsMapToPixelSimplifier::SimplifyGeometry;
  bool ret = QgsMapToPixelSimplifier::simplifyGeometry( g, fl, 10.0 );
  QVERIFY( ! ret ); // not simplifiable
}

void TestQgsMapToPixelGeometrySimplifier::testLine1()
{
  // NOTE: we need more than 4 vertices, or the line will not be
  //       reduced at all by the algorithm
  QgsGeometry *g = QgsGeometry::fromWkt( "LINESTRING(0 0,1 1,2 0,3 1,4 0,20 1,20 0,10 0,5 0)" );
  int fl;
  bool ret;
  QString wkt;

  fl = QgsMapToPixelSimplifier::SimplifyGeometry;
  ret = QgsMapToPixelSimplifier::simplifyGeometry( g, fl, 10.0 );
  QVERIFY( ret );
  wkt = g->exportToWkt();
  // NOTE: I don't think vertex 1,1 should be in this result,
  //       but that's what we get now
  QCOMPARE( wkt, QString( "LineString (0 0, 1 1, 20 1, 10 0, 5 0)" ) );

  fl = QgsMapToPixelSimplifier::SimplifyEnvelope;
  ret = QgsMapToPixelSimplifier::simplifyGeometry( g, fl, 20.0 );
  QVERIFY( ! ret );

  ret = QgsMapToPixelSimplifier::simplifyGeometry( g, fl, 30.0 );
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

void TestQgsMapToPixelGeometrySimplifier::testMismatchWKB()
{
  // TODO see http://hub.qgis.org/issues/12416
}

QTEST_MAIN( TestQgsMapToPixelGeometrySimplifier )
#include "testqgsmaptopixelgeometrysimplifier.moc"
