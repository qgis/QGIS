/***************************************************************************
  testqgslayertree.cpp
  --------------------------------------
  Date                 : January 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>

#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgsmaplayerregistry.h>
#include <qgstracer.h>
#include <qgsvectorlayer.h>

class TestQgsTracer : public QObject
{
    Q_OBJECT
  public:
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testSimple();
    void testPolygon();
    void testButterfly();

  private:

};

static QgsVectorLayer* make_layer( const QStringList& wkts )
{
  QgsVectorLayer* vl = new QgsVectorLayer( "LineString", "x", "memory" );
  Q_ASSERT( vl->isValid() );

  vl->startEditing();
  foreach ( const QString& wkt, wkts )
  {
    QgsFeature f;
    f.setGeometry( QgsGeometry::fromWkt( wkt ) );
    vl->addFeature( f, false );
  }
  vl->commitChanges();

  return vl;
}

void print_shortest_path( QgsTracer& tracer, const QgsPoint& p1, const QgsPoint& p2 )
{
  qDebug( "from (%f,%f) to (%f,%f)", p1.x(), p1.y(), p2.x(), p2.y() );
  QVector<QgsPoint> points = tracer.findShortestPath( p1, p2 );

  if ( points.isEmpty() )
    qDebug( "no path!" );

  foreach ( QgsPoint p, points )
    qDebug( "p: %f %f", p.x(), p.y() );
}



void TestQgsTracer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsTracer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTracer::testSimple()
{
  QStringList wkts;
  wkts  << "LINESTRING(0 0, 0 10)"
  << "LINESTRING(0 0, 10 0)"
  << "LINESTRING(0 10, 20 10)"
  << "LINESTRING(10 0, 20 10)";

  /* This shape - nearly a square (one side is shifted to have exactly one shortest
   * path between corners):
   * 0,10 +----+  20,10
   *      |   /
   * 0,0  +--+  10,0
   */

  QList<QgsVectorLayer*> layers;
  QgsVectorLayer* vl = make_layer( wkts );
  layers << vl;

  QgsTracer tracer( layers );

  QgsPolyline points1 = tracer.findShortestPath( QgsPoint( 0, 0 ), QgsPoint( 20, 10 ) );
  QCOMPARE( points1.count(), 3 );
  QCOMPARE( points1[0], QgsPoint( 0, 0 ) );
  QCOMPARE( points1[1], QgsPoint( 10, 0 ) );
  QCOMPARE( points1[2], QgsPoint( 20, 10 ) );

  // one joined point
  QgsPolyline points2 = tracer.findShortestPath( QgsPoint( 5, 10 ), QgsPoint( 0, 0 ) );
  QCOMPARE( points2.count(), 3 );
  QCOMPARE( points2[0], QgsPoint( 5, 10 ) );
  QCOMPARE( points2[1], QgsPoint( 0, 10 ) );
  QCOMPARE( points2[2], QgsPoint( 0, 0 ) );

  // two joined points
  QgsPolyline points3 = tracer.findShortestPath( QgsPoint( 0, 1 ), QgsPoint( 11, 1 ) );
  QCOMPARE( points3.count(), 4 );
  QCOMPARE( points3[0], QgsPoint( 0, 1 ) );
  QCOMPARE( points3[1], QgsPoint( 0, 0 ) );
  QCOMPARE( points3[2], QgsPoint( 10, 0 ) );
  QCOMPARE( points3[3], QgsPoint( 11, 1 ) );

  // two joined points on one line
  QgsPolyline points4 = tracer.findShortestPath( QgsPoint( 11, 1 ), QgsPoint( 19, 9 ) );
  QCOMPARE( points4.count(), 2 );
  QCOMPARE( points4[0], QgsPoint( 11, 1 ) );
  QCOMPARE( points4[1], QgsPoint( 19, 9 ) );

  // no path to (1,1)
  QgsPolyline points5 = tracer.findShortestPath( QgsPoint( 0, 0 ), QgsPoint( 1, 1 ) );
  QCOMPARE( points5.count(), 0 );

  delete vl;
}

void TestQgsTracer::testPolygon()
{
  // the same shape as in testSimple() but with just one polygon ring
  // to check extraction from polygons work + routing along one ring works

  QStringList wkts;
  wkts << "POLYGON((0 0, 0 10, 20 10, 10 0, 0 0))";

  QList<QgsVectorLayer*> layers;
  QgsVectorLayer* vl = make_layer( wkts );
  layers << vl;

  QgsTracer tracer( layers );
  QgsPolyline points = tracer.findShortestPath( QgsPoint( 1, 0 ), QgsPoint( 0, 1 ) );
  QCOMPARE( points.count(), 3 );
  QCOMPARE( points[0], QgsPoint( 1, 0 ) );
  QCOMPARE( points[1], QgsPoint( 0, 0 ) );
  QCOMPARE( points[2], QgsPoint( 0, 1 ) );
}

void TestQgsTracer::testButterfly()
{
  // checks whether tracer internally splits linestrings at intersections

  QStringList wkts;
  wkts << "LINESTRING(0 0, 0 10, 10 0, 10 10, 0 0)";

  /* This shape (without a vertex where the linestring crosses itself):
   *    +  +  10,10
   *    |\/|
   *    |/\|
   *    +  +
   *  0,0
   */

  QList<QgsVectorLayer*> layers;
  QgsVectorLayer* vl = make_layer( wkts );
  layers << vl;

  QgsTracer tracer( layers );
  QgsPolyline points = tracer.findShortestPath( QgsPoint( 0, 0 ), QgsPoint( 10, 0 ) );

  QCOMPARE( points.count(), 3 );
  QCOMPARE( points[0], QgsPoint( 0, 0 ) );
  QCOMPARE( points[1], QgsPoint( 5, 5 ) );
  QCOMPARE( points[2], QgsPoint( 10, 0 ) );

  delete vl;
}


QTEST_MAIN( TestQgsTracer )
#include "testqgstracer.moc"
