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
#include <qgstestutils.h>
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
    void testLayerUpdates();
    void testExtent();
    void testReprojection();
    void testCurved();

  private:

};

namespace QTest
{
  template<>
  char* toString( const QgsPoint& point )
  {
    QByteArray ba = "QgsPoint(" + QByteArray::number( point.x() ) +
                    ", " + QByteArray::number( point.y() ) + ")";
    return qstrdup( ba.data() );
  }
}

static QgsFeature make_feature( const QString& wkt )
{
  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( wkt ) );
  return f;
}

static QgsVectorLayer* make_layer( const QStringList& wkts )
{
  QgsVectorLayer* vl = new QgsVectorLayer( "LineString", "x", "memory" );
  Q_ASSERT( vl->isValid() );

  vl->startEditing();
  Q_FOREACH ( const QString& wkt, wkts )
  {
    QgsFeature f( make_feature( wkt ) );
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

  Q_FOREACH ( const QgsPoint& p, points )
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

  QgsVectorLayer* vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer*>() << vl );

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

  QgsVectorLayer* vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer*>() << vl );

  QgsPolyline points = tracer.findShortestPath( QgsPoint( 1, 0 ), QgsPoint( 0, 1 ) );
  QCOMPARE( points.count(), 3 );
  QCOMPARE( points[0], QgsPoint( 1, 0 ) );
  QCOMPARE( points[1], QgsPoint( 0, 0 ) );
  QCOMPARE( points[2], QgsPoint( 0, 1 ) );

  delete vl;
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

  QgsVectorLayer* vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer*>() << vl );

  QgsPolyline points = tracer.findShortestPath( QgsPoint( 0, 0 ), QgsPoint( 10, 0 ) );

  QCOMPARE( points.count(), 3 );
  QCOMPARE( points[0], QgsPoint( 0, 0 ) );
  QCOMPARE( points[1], QgsPoint( 5, 5 ) );
  QCOMPARE( points[2], QgsPoint( 10, 0 ) );

  delete vl;
}

void TestQgsTracer::testLayerUpdates()
{
  // check whether the tracer is updated on added/removed/changed features

  // same shape as in testSimple()
  QStringList wkts;
  wkts  << "LINESTRING(0 0, 0 10)"
  << "LINESTRING(0 0, 10 0)"
  << "LINESTRING(0 10, 20 10)"
  << "LINESTRING(10 0, 20 10)";

  QgsVectorLayer* vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer*>() << vl );
  tracer.init();

  QgsPolyline points1 = tracer.findShortestPath( QgsPoint( 10, 0 ), QgsPoint( 10, 10 ) );
  QCOMPARE( points1.count(), 3 );
  QCOMPARE( points1[0], QgsPoint( 10, 0 ) );
  QCOMPARE( points1[1], QgsPoint( 20, 10 ) );
  QCOMPARE( points1[2], QgsPoint( 10, 10 ) );

  vl->startEditing();

  // add a shortcut
  QgsFeature f( make_feature( "LINESTRING(10 0, 10 10)" ) );
  vl->addFeature( f );

  QgsPolyline points2 = tracer.findShortestPath( QgsPoint( 10, 0 ), QgsPoint( 10, 10 ) );
  QCOMPARE( points2.count(), 2 );
  QCOMPARE( points2[0], QgsPoint( 10, 0 ) );
  QCOMPARE( points2[1], QgsPoint( 10, 10 ) );

  // delete the shortcut
  vl->deleteFeature( f.id() );

  QgsPolyline points3 = tracer.findShortestPath( QgsPoint( 10, 0 ), QgsPoint( 10, 10 ) );
  QCOMPARE( points3.count(), 3 );
  QCOMPARE( points3[0], QgsPoint( 10, 0 ) );
  QCOMPARE( points3[1], QgsPoint( 20, 10 ) );
  QCOMPARE( points3[2], QgsPoint( 10, 10 ) );

  // make the shortcut again from a different feature
  QgsGeometry* g = QgsGeometry::fromWkt( "LINESTRING(10 0, 10 10)" );
  vl->changeGeometry( 2, g );  // change bottom line (second item in wkts)
  delete g;

  QgsPolyline points4 = tracer.findShortestPath( QgsPoint( 10, 0 ), QgsPoint( 10, 10 ) );
  QCOMPARE( points4.count(), 2 );
  QCOMPARE( points4[0], QgsPoint( 10, 0 ) );
  QCOMPARE( points4[1], QgsPoint( 10, 10 ) );

  QgsPolyline points5 = tracer.findShortestPath( QgsPoint( 0, 0 ), QgsPoint( 10, 0 ) );
  QCOMPARE( points5.count(), 4 );
  QCOMPARE( points5[0], QgsPoint( 0, 0 ) );
  QCOMPARE( points5[1], QgsPoint( 0, 10 ) );
  QCOMPARE( points5[2], QgsPoint( 10, 10 ) );
  QCOMPARE( points5[3], QgsPoint( 10, 0 ) );

  vl->rollBack();

  delete vl;
}

void TestQgsTracer::testExtent()
{
  // check whether the tracer correctly handles the extent limitation

  // same shape as in testSimple()
  QStringList wkts;
  wkts  << "LINESTRING(0 0, 0 10)"
  << "LINESTRING(0 0, 10 0)"
  << "LINESTRING(0 10, 20 10)"
  << "LINESTRING(10 0, 20 10)";

  QgsVectorLayer* vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer*>() << vl );
  tracer.setExtent( QgsRectangle( 0, 0, 5, 5 ) );
  tracer.init();

  QgsPolyline points1 = tracer.findShortestPath( QgsPoint( 0, 0 ), QgsPoint( 10, 0 ) );
  QCOMPARE( points1.count(), 2 );
  QCOMPARE( points1[0], QgsPoint( 0, 0 ) );
  QCOMPARE( points1[1], QgsPoint( 10, 0 ) );

  QgsPolyline points2 = tracer.findShortestPath( QgsPoint( 0, 0 ), QgsPoint( 20, 10 ) );
  QCOMPARE( points2.count(), 0 );
}

void TestQgsTracer::testReprojection()
{
  QStringList wkts;
  wkts  << "LINESTRING(1 0, 2 0)";

  QgsVectorLayer* vl = make_layer( wkts );

  QgsCoordinateReferenceSystem dstCrs( "EPSG:3857" );
  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( "EPSG:4326" ), dstCrs );
  QgsPoint p1 = ct.transform( QgsPoint( 1, 0 ) );
  QgsPoint p2 = ct.transform( QgsPoint( 2, 0 ) );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer*>() << vl );
  tracer.setDestinationCrs( dstCrs );
  tracer.setCrsTransformEnabled( true );
  tracer.init();

  QgsPolyline points1 = tracer.findShortestPath( p1, p2 );
  QCOMPARE( points1.count(), 2 );
}

void TestQgsTracer::testCurved()
{
  QStringList wkts;
  wkts  << "CIRCULARSTRING(0 0, 10 10, 20 0)";

  /* This shape - half of a circle (r = 10)
   * 10,10  _
   *       / \
   * 0,0  |   |  20,0
   */

  QgsVectorLayer* vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer*>() << vl );

  QgsPolyline points1 = tracer.findShortestPath( QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) );

  QVERIFY( points1.count() != 0 );

  QgsGeometry* tmpG1 = QgsGeometry::fromPolyline( points1 );
  double l = tmpG1->length();
  delete tmpG1;

  // fuzzy comparison as QCOMPARE is too strict for this case
  double full_circle_length = 2 * M_PI * 10;
  QGSCOMPARENEAR( l, full_circle_length / 4, 0.01 );

  QCOMPARE( points1[0], QgsPoint( 0, 0 ) );
  QCOMPARE( points1[points1.count()-1], QgsPoint( 10, 10 ) );

  delete vl;
}


QTEST_MAIN( TestQgsTracer )
#include "testqgstracer.moc"
