/***************************************************************************
  testqgsinterpolator.cpp
  -----------------------
Date                 : November 2017
Copyright            : (C) 2017 by Nyall Dawson
Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include "qgsapplication.h"
#include "DualEdgeTriangulation.h"

class TestQgsInterpolator : public QObject
{
    Q_OBJECT

  public:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction.
    void dualEdge();

  private:
};

void  TestQgsInterpolator::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsInterpolator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsInterpolator::init()
{}

void TestQgsInterpolator::cleanup()
{}

void TestQgsInterpolator::dualEdge()
{
  DualEdgeTriangulation tri;
  QVERIFY( !tri.getPoint( 0 ) );
  QVERIFY( !tri.getPoint( 1 ) );
  QCOMPARE( tri.getNumberOfPoints(), 0 );

  tri.addPoint( QgsPoint( 1, 2, 3 ) );
  QCOMPARE( *tri.getPoint( 0 ), QgsPoint( 1, 2, 3 ) );
  QCOMPARE( tri.getNumberOfPoints(), 1 );

  tri.addPoint( QgsPoint( 3, 0, 4 ) );
  QCOMPARE( *tri.getPoint( 1 ), QgsPoint( 3, 0, 4 ) );
  QCOMPARE( tri.getNumberOfPoints(), 2 );

  tri.addPoint( QgsPoint( 4, 4, 5 ) );
  QCOMPARE( *tri.getPoint( 2 ), QgsPoint( 4, 4, 5 ) );
  QCOMPARE( tri.getNumberOfPoints(), 3 );

  QgsPoint p1( 0, 0, 0 );
  QgsPoint p2( 0, 0, 0 );
  QgsPoint p3( 0, 0, 0 );
  int n1 = 0;
  int n2 = 0;
  int n3 = 0;
  QVERIFY( !tri.pointInside( 0, 1 ) );
  QVERIFY( !tri.getTriangle( 0, 1, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 0, 1, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 1, 1 ) );
  QVERIFY( !tri.getTriangle( 1, 1, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 1, 1, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 4, 1 ) );
  QVERIFY( !tri.getTriangle( 4, 1, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 4, 1, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 2, 4 ) );
  QVERIFY( !tri.getTriangle( 2, 4, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 2, 4, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 3, -1 ) );
  QVERIFY( !tri.getTriangle( 3, -1, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 3, -1, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( tri.pointInside( 2, 2 ) );
  QVERIFY( tri.getTriangle( 2, 2, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QVERIFY( tri.getTriangle( 2, 2, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );
  QVERIFY( tri.pointInside( 3, 1 ) );
  QVERIFY( tri.getTriangle( 3, 1, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QVERIFY( tri.getTriangle( 3, 1, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );
  QVERIFY( tri.pointInside( 3.5, 3.5 ) );
  QVERIFY( tri.getTriangle( 3.5, 3.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QVERIFY( tri.getTriangle( 3.5, 3.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );

  QCOMPARE( tri.getOppositePoint( 0, 1 ), -1 );
  QCOMPARE( tri.getOppositePoint( 0, 2 ), 1 );
  QCOMPARE( tri.getOppositePoint( 1, 0 ), 2 );
  QCOMPARE( tri.getOppositePoint( 1, 2 ), -1 );
  QCOMPARE( tri.getOppositePoint( 2, 0 ), -1 );
  QCOMPARE( tri.getOppositePoint( 2, 1 ), 0 );

  // add another point
  tri.addPoint( QgsPoint( 2, 4, 6 ) );
  QCOMPARE( *tri.getPoint( 3 ), QgsPoint( 2, 4, 6 ) );
  QCOMPARE( tri.getNumberOfPoints(), 4 );
  QVERIFY( !tri.pointInside( 2, 4.5 ) );
  QVERIFY( !tri.getTriangle( 2, 4.5, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 2, 4.5, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 1, 4 ) );
  QVERIFY( !tri.getTriangle( 1, 4, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 1, 4, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( tri.pointInside( 2, 3.5 ) );
  QVERIFY( tri.getTriangle( 2, 3.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( p3, QgsPoint( 2, 4, 6 ) );
  QVERIFY( tri.getTriangle( 2, 3.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( p3, QgsPoint( 2, 4, 6 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 2 );
  QCOMPARE( n3, 3 );
  QVERIFY( tri.getTriangle( 2, 2, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );

  QCOMPARE( tri.getOppositePoint( 0, 1 ), -1 );
  QCOMPARE( tri.getOppositePoint( 0, 2 ), 1 );
  QCOMPARE( tri.getOppositePoint( 0, 3 ), 2 );
  QCOMPARE( tri.getOppositePoint( 1, 0 ), 2 );
  QCOMPARE( tri.getOppositePoint( 1, 2 ), -1 );
  QCOMPARE( tri.getOppositePoint( 1, 3 ), -10 );
  QCOMPARE( tri.getOppositePoint( 2, 0 ), 3 );
  QCOMPARE( tri.getOppositePoint( 2, 1 ), 0 );
  QCOMPARE( tri.getOppositePoint( 2, 3 ), -1 );
  QCOMPARE( tri.getOppositePoint( 3, 0 ), -1 );
  QCOMPARE( tri.getOppositePoint( 3, 1 ), -10 );
  QCOMPARE( tri.getOppositePoint( 3, 2 ), 0 );


  // add another point
  tri.addPoint( QgsPoint( 2, 2, 7 ) );
  QCOMPARE( *tri.getPoint( 4 ), QgsPoint( 2, 2, 7 ) );
  QCOMPARE( tri.getNumberOfPoints(), 5 );
  QVERIFY( !tri.pointInside( 2, 4.5 ) );
  QVERIFY( !tri.getTriangle( 2, 4.5, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 2, 4.5, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 1, 4 ) );
  QVERIFY( !tri.getTriangle( 1, 4, p1, p2, p3 ) );
  QVERIFY( !tri.getTriangle( 1, 4, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( tri.pointInside( 2, 3.5 ) );
  QVERIFY( tri.getTriangle( 2, 3.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 2, 4, 6 ) );
  QCOMPARE( p2, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p3, QgsPoint( 2, 2, 7 ) );
  QVERIFY( tri.getTriangle( 2, 3.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 2, 4, 6 ) );
  QCOMPARE( p2, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p3, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( n1, 3 );
  QCOMPARE( n2, 0 );
  QCOMPARE( n3, 4 );
  QVERIFY( tri.pointInside( 2, 1.5 ) );
  QVERIFY( tri.getTriangle( 2, 1.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 2, 2, 7 ) );
  QVERIFY( tri.getTriangle( 2, 1.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 4 );
  QVERIFY( tri.pointInside( 3.1, 1 ) );
  QVERIFY( tri.getTriangle( 3.1, 1, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QVERIFY( tri.getTriangle( 3.1, 1, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 4 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );
  QVERIFY( tri.pointInside( 2.5, 3.5 ) );
  QVERIFY( tri.getTriangle( 2.5, 3.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( p2, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( p3, QgsPoint( 2, 4, 6 ) );
  QVERIFY( tri.getTriangle( 2.5, 3.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( p2, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( p3, QgsPoint( 2, 4, 6 ) );
  QCOMPARE( n1, 4 );
  QCOMPARE( n2, 2 );
  QCOMPARE( n3, 3 );

  QCOMPARE( tri.getOppositePoint( 0, 1 ), -1 );
  QCOMPARE( tri.getOppositePoint( 0, 2 ), -10 );
  QCOMPARE( tri.getOppositePoint( 0, 3 ), 4 );
  QCOMPARE( tri.getOppositePoint( 0, 4 ), 1 );
  QCOMPARE( tri.getOppositePoint( 1, 0 ), 4 );
  QCOMPARE( tri.getOppositePoint( 1, 2 ), -1 );
  QCOMPARE( tri.getOppositePoint( 1, 3 ), -10 );
  QCOMPARE( tri.getOppositePoint( 1, 4 ), 2 );
  QCOMPARE( tri.getOppositePoint( 2, 0 ), -10 );
  QCOMPARE( tri.getOppositePoint( 2, 1 ), 4 );
  QCOMPARE( tri.getOppositePoint( 2, 3 ), -1 );
  QCOMPARE( tri.getOppositePoint( 2, 4 ), 3 );
  QCOMPARE( tri.getOppositePoint( 3, 0 ), -1 );
  QCOMPARE( tri.getOppositePoint( 3, 1 ), -10 );
  QCOMPARE( tri.getOppositePoint( 3, 2 ), 4 );
  QCOMPARE( tri.getOppositePoint( 3, 4 ), 0 );
  QCOMPARE( tri.getOppositePoint( 4, 0 ), 3 );
  QCOMPARE( tri.getOppositePoint( 4, 1 ), 0 );
  QCOMPARE( tri.getOppositePoint( 4, 2 ), 1 );
  QCOMPARE( tri.getOppositePoint( 4, 3 ), 2 );

//  QVERIFY( tri.getSurroundingTriangles( 0 ).empty() );
}

QGSTEST_MAIN( TestQgsInterpolator )
#include "testqgsinterpolator.moc"
