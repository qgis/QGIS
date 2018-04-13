/***************************************************************************
     testqgsinternalgeometryengine.cpp
     --------------------------------------
    Date                 : April 2018
    Copyright            : (C) 2018 by Nyall Dawson
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

//qgis includes...
#include "qgsinternalgeometryengine.h"
#include "qgslinesegment.h"

class TestQgsInternalGeometryEngine : public QObject
{
    Q_OBJECT

  public:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void ray();
    void lineSegmentDistanceComparer();

};

void TestQgsInternalGeometryEngine::initTestCase()
{
  // Runs once before any tests are run
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}


void TestQgsInternalGeometryEngine::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsInternalGeometryEngine::init()
{
}

void TestQgsInternalGeometryEngine::cleanup()
{
}

void TestQgsInternalGeometryEngine::ray()
{
  QgsRay2D ray( QgsPointXY( 3, 5 ), QgsVector( -1, 1 ) );
  QgsPointXY intersect;
  QVERIFY( !ray.intersects( QgsLineSegment2D( QgsPointXY( 4, -1 ), QgsPointXY( 5, 15 ) ), intersect ) );
  QVERIFY( !ray.intersects( QgsLineSegment2D( QgsPointXY( 5, 15 ), QgsPointXY( 4, -1 ) ), intersect ) );

  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 5, 15 ), QgsPointXY( 3, 5 ) ), intersect ) );
  QCOMPARE( intersect.x(), 3.0 );
  QCOMPARE( intersect.y(), 5.0 );
  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 3, 5 ), QgsPointXY( 5, 15 ) ), intersect ) );
  QCOMPARE( intersect.x(), 3.0 );
  QCOMPARE( intersect.y(), 5.0 );
  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 3, 1 ), QgsPointXY( 3, 6 ) ), intersect ) );
  QCOMPARE( intersect.x(), 3.0 );
  QCOMPARE( intersect.y(), 5.0 );
  QVERIFY( !ray.intersects( QgsLineSegment2D( QgsPointXY( 4, 1 ), QgsPointXY( 4, 6 ) ), intersect ) );
  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 2, 1 ), QgsPointXY( 2, 6 ) ), intersect ) );
  QCOMPARE( intersect.x(), 2.0 );
  QCOMPARE( intersect.y(), 6.0 );
  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 2, 1 ), QgsPointXY( 2, 16 ) ), intersect ) );
  QCOMPARE( intersect.x(), 2.0 );
  QCOMPARE( intersect.y(), 6.0 );
  QVERIFY( !ray.intersects( QgsLineSegment2D( QgsPointXY( 1, 1 ), QgsPointXY( 1, 6 ) ), intersect ) );
  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 1, 1 ), QgsPointXY( 1, 26 ) ), intersect ) );
  QCOMPARE( intersect.x(), 1.0 );
  QCOMPARE( intersect.y(), 7.0 );
  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 4, 4 ), QgsPointXY( 2, 6 ) ), intersect ) );
  QCOMPARE( intersect.x(), 3.0 );
  QCOMPARE( intersect.y(), 5.0 );
  QVERIFY( !ray.intersects( QgsLineSegment2D( QgsPointXY( 14, 14 ), QgsPointXY( 12, 16 ) ), intersect ) );
  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 2, 6 ), QgsPointXY( 1, 7 ) ), intersect ) );
  QCOMPARE( intersect.x(), 2.0 );
  QCOMPARE( intersect.y(), 6.0 );
  QVERIFY( ray.intersects( QgsLineSegment2D( QgsPointXY( 1, 7 ), QgsPointXY( 2, 6 ) ), intersect ) );
  QCOMPARE( intersect.x(), 2.0 );
  QCOMPARE( intersect.y(), 6.0 );

}

void TestQgsInternalGeometryEngine::lineSegmentDistanceComparer()
{
  QgsLineSegmentDistanceComparer comp( QgsPointXY( 3, 5 ) );

  QVERIFY( comp( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ),
                 QgsLineSegment2D( QgsPointXY( 11, 2 ), QgsPointXY( 13, 4 ) ) ) );
  QVERIFY( comp( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ),
                 QgsLineSegment2D( QgsPointXY( 13, 4 ), QgsPointXY( 11, 2 ) ) ) );
  QVERIFY( comp( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ),
                 QgsLineSegment2D( QgsPointXY( -13, 4 ), QgsPointXY( -11, 2 ) ) ) );
  QVERIFY( comp( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ),
                 QgsLineSegment2D( QgsPointXY( -13, -4 ), QgsPointXY( -11, 2 ) ) ) );
  QVERIFY( comp( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ),
                 QgsLineSegment2D( QgsPointXY( 11, 2 ), QgsPointXY( 13, 4 ) ) ) );
  QVERIFY( !comp( QgsLineSegment2D( QgsPointXY( 11, 2 ), QgsPointXY( 13, 4 ) ),
                  QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ) ) );
  QVERIFY( !comp( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ),
                  QgsLineSegment2D( QgsPointXY( 5, 6 ), QgsPointXY( 8, 9 ) ) ) );
  QVERIFY( comp( QgsLineSegment2D( QgsPointXY( 5, 6 ), QgsPointXY( 8, 9 ) ),
                 QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ) ) );
  QVERIFY( !comp( QgsLineSegment2D( QgsPointXY( 5, 6 ), QgsPointXY( 8, 9 ) ),
                  QgsLineSegment2D( QgsPointXY( 8, 9 ), QgsPointXY( 13, 14 ) ) ) );
  QVERIFY( !comp( QgsLineSegment2D( QgsPointXY( 8, 9 ), QgsPointXY( 13, 14 ) ),
                  QgsLineSegment2D( QgsPointXY( 5, 6 ), QgsPointXY( 8, 9 ) ) ) );
  QVERIFY( comp( QgsLineSegment2D( QgsPointXY( 1, 4 ), QgsPointXY( 8, 9 ) ),
                 QgsLineSegment2D( QgsPointXY( 8, 9 ), QgsPointXY( 15, 16 ) ) ) );
}

QGSTEST_MAIN( TestQgsInternalGeometryEngine )
#include "testqgsinternalgeometryengine.moc"
