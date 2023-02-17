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

#include "qgstest.h"

#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgstracer.h>
#include <qgsvectorlayer.h>
#include "qgsproject.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgssettings.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgsmapsettings.h"
#include "qgssnappingutils.h"
#include "qgssymbol.h"

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
    void testOffset();
    void testInvisible();

  private:

};

namespace QTest
{
  template<>
  char *toString( const QgsPointXY &point )
  {
    QByteArray ba = "QgsPointXY(" + QByteArray::number( point.x() ) +
                    ", " + QByteArray::number( point.y() ) + ")";
    return qstrdup( ba.data() );
  }
}

static QgsFeature make_feature( const QString &wkt )
{
  QgsFeature f;
  const QgsGeometry g = QgsGeometry::fromWkt( wkt ) ;
  f.setGeometry( g );
  return f;
}

static QgsVectorLayer *make_layer( const QStringList &wkts )
{
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:4326" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  Q_ASSERT( vl->isValid() );

  vl->startEditing();
  for ( const QString &wkt : wkts )
  {
    QgsFeature f( make_feature( wkt ) );
    vl->addFeature( f );
  }
  vl->commitChanges();

  return vl;
}

void print_shortest_path( QgsTracer &tracer, const QgsPointXY &p1, const QgsPointXY &p2 )
{
  qDebug( "from (%f,%f) to (%f,%f)", p1.x(), p1.y(), p2.x(), p2.y() );
  const QVector<QgsPointXY> points = tracer.findShortestPath( p1, p2 );

  if ( points.isEmpty() )
    qDebug( "no path!" );

  for ( const QgsPointXY &p : points )
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
  wkts  << QStringLiteral( "LINESTRING(0 0, 0 10)" )
        << QStringLiteral( "LINESTRING(0 0, 10 0)" )
        << QStringLiteral( "LINESTRING(0 10, 20 10)" )
        << QStringLiteral( "LINESTRING(10 0, 20 10)" );

  /* This shape - nearly a square (one side is shifted to have exactly one shortest
   * path between corners):
   * 0,10 +----+  20,10
   *      |   /
   * 0,0  +--+  10,0
   */

  QgsVectorLayer *vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << vl );

  QgsPolylineXY points1 = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 20, 10 ) );
  QCOMPARE( points1.count(), 3 );
  QCOMPARE( points1[0], QgsPointXY( 0, 0 ) );
  QCOMPARE( points1[1], QgsPointXY( 10, 0 ) );
  QCOMPARE( points1[2], QgsPointXY( 20, 10 ) );

  // one joined point
  QgsPolylineXY points2 = tracer.findShortestPath( QgsPointXY( 5, 10 ), QgsPointXY( 0, 0 ) );
  QCOMPARE( points2.count(), 3 );
  QCOMPARE( points2[0], QgsPointXY( 5, 10 ) );
  QCOMPARE( points2[1], QgsPointXY( 0, 10 ) );
  QCOMPARE( points2[2], QgsPointXY( 0, 0 ) );

  // two joined points
  QgsPolylineXY points3 = tracer.findShortestPath( QgsPointXY( 0, 1 ), QgsPointXY( 11, 1 ) );
  QCOMPARE( points3.count(), 4 );
  QCOMPARE( points3[0], QgsPointXY( 0, 1 ) );
  QCOMPARE( points3[1], QgsPointXY( 0, 0 ) );
  QCOMPARE( points3[2], QgsPointXY( 10, 0 ) );
  QCOMPARE( points3[3], QgsPointXY( 11, 1 ) );

  // two joined points on one line
  QgsPolylineXY points4 = tracer.findShortestPath( QgsPointXY( 11, 1 ), QgsPointXY( 19, 9 ) );
  QCOMPARE( points4.count(), 2 );
  QCOMPARE( points4[0], QgsPointXY( 11, 1 ) );
  QCOMPARE( points4[1], QgsPointXY( 19, 9 ) );

  // no path to (1,1)
  const QgsPolylineXY points5 = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 1, 1 ) );
  QCOMPARE( points5.count(), 0 );

  delete vl;
}

void TestQgsTracer::testInvisible()
{
  QgsVectorLayer *mVL = new QgsVectorLayer( QStringLiteral( "Linestring?field=fld:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature f1, f2, f3, f4;
  const int idx = mVL->fields().indexFromName( QStringLiteral( "fld" ) );
  QVERIFY( idx != -1 );
  f1.initAttributes( 1 );
  f2.initAttributes( 1 );
  f3.initAttributes( 1 );
  f4.initAttributes( 1 );

  /* This shape - nearly a square (one side is shifted to have exactly one shortest
   * path between corners):
   * 0,10 +----+  20,10
   *      |   /
   * 0,0  +--+  10,0
   */
  QgsGeometry geom = QgsGeometry::fromWkt( "LINESTRING(0 0, 0 10)" );
  f1.setGeometry( geom );
  f1.setAttribute( idx, QVariant( 2 ) );
  geom = QgsGeometry::fromWkt( "LINESTRING(0 0, 10 0)" );
  f2.setGeometry( geom );
  f2.setAttribute( idx, QVariant( 1 ) );
  geom = QgsGeometry::fromWkt( "LINESTRING(0 10, 20 10)" );
  f3.setGeometry( geom );
  f3.setAttribute( idx, QVariant( 1 ) );
  geom = QgsGeometry::fromWkt( "LINESTRING(10 0, 20 10)" );
  f4.setGeometry( geom );
  f4.setAttribute( idx, QVariant( 1 ) );
  QgsFeatureList flist;
  flist << f1 << f2 << f3 << f4;

  mVL->dataProvider()->addFeatures( flist );

  QgsProject::instance()->addMapLayer( mVL );

  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( QStringLiteral( "fld" ) );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::LineGeometry ) );
  renderer->addCategory( QgsRendererCategory( "2", QgsSymbol::defaultSymbol( QgsWkbTypes::LineGeometry ), QStringLiteral( "2" ) ) );
  mVL->setRenderer( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTree *root = new QgsLayerTree();
  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( mVL );
  root->addChildNode( n );
  QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
  m->refreshLayerLegend( n );

  const QList<QgsLayerTreeModelLegendNode *> nodes = m->layerLegendNodes( n );
  QCOMPARE( nodes.length(), 1 );
  //uncheck all and test that all nodes are unchecked
  static_cast< QgsSymbolLegendNode * >( nodes.at( 0 ) )->uncheckAllItems();
  for ( QgsLayerTreeModelLegendNode *ln :  nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Unchecked );
  }

  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( QSize( 100, 100 ) );
  mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
  QVERIFY( mapSettings.hasValidSettings() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mVL );

  QgsSnappingUtils u;
  u.setMapSettings( mapSettings );
  u.setEnableSnappingForInvisibleFeature( false );
  u.setCurrentLayer( mVL );

  QgsSnappingConfig snappingConfig = u.config();
  snappingConfig.setEnabled( true );
  snappingConfig.setTolerance( 10 );
  snappingConfig.setUnits( QgsTolerance::Pixels );
  snappingConfig.setMode( Qgis::SnappingMode::ActiveLayer );
  u.setConfig( snappingConfig );
  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << mVL );

  QgsPolylineXY points1 = tracer.findShortestPath( QgsPointXY( 10, 0 ), QgsPointXY( 0, 10 ) );
  QCOMPARE( points1.count(), 3 );
  QCOMPARE( points1[0], QgsPointXY( 10, 0 ) );
  QCOMPARE( points1[1], QgsPointXY( 0, 0 ) );
  QCOMPARE( points1[2], QgsPointXY( 0, 10 ) );

  const QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( mapSettings );
  tracer.setRenderContext( &renderContext );
  points1 = tracer.findShortestPath( QgsPointXY( 10, 0 ), QgsPointXY( 0, 10 ) );
  QCOMPARE( points1.count(), 0 );

}

void TestQgsTracer::testPolygon()
{
  // the same shape as in testSimple() but with just one polygon ring
  // to check extraction from polygons work + routing along one ring works

  QStringList wkts;
  wkts << QStringLiteral( "POLYGON((0 0, 0 10, 20 10, 10 0, 0 0))" );

  QgsVectorLayer *vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << vl );

  QgsPolylineXY points = tracer.findShortestPath( QgsPointXY( 1, 0 ), QgsPointXY( 0, 1 ) );
  QCOMPARE( points.count(), 3 );
  QCOMPARE( points[0], QgsPointXY( 1, 0 ) );
  QCOMPARE( points[1], QgsPointXY( 0, 0 ) );
  QCOMPARE( points[2], QgsPointXY( 0, 1 ) );

  delete vl;
}

void TestQgsTracer::testButterfly()
{
  // checks whether tracer internally splits linestrings at intersections

  QStringList wkts;
  wkts << QStringLiteral( "LINESTRING(0 0, 0 10, 10 0, 10 10, 0 0)" );

  /* This shape (without a vertex where the linestring crosses itself):
   *    +  +  10,10
   *    |\/|
   *    |/\|
   *    +  +
   *  0,0
   */

  QgsVectorLayer *vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << vl );

  QgsPolylineXY points = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 10, 0 ) );

  QCOMPARE( points.count(), 3 );
  QCOMPARE( points[0], QgsPointXY( 0, 0 ) );
  QCOMPARE( points[1], QgsPointXY( 5, 5 ) );
  QCOMPARE( points[2], QgsPointXY( 10, 0 ) );

  delete vl;
}

void TestQgsTracer::testLayerUpdates()
{
  // check whether the tracer is updated on added/removed/changed features

  // same shape as in testSimple()
  QStringList wkts;
  wkts  << QStringLiteral( "LINESTRING(0 0, 0 10)" )
        << QStringLiteral( "LINESTRING(0 0, 10 0)" )
        << QStringLiteral( "LINESTRING(0 10, 20 10)" )
        << QStringLiteral( "LINESTRING(10 0, 20 10)" );

  QgsVectorLayer *vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << vl );
  tracer.init();

  QgsPolylineXY points1 = tracer.findShortestPath( QgsPointXY( 10, 0 ), QgsPointXY( 10, 10 ) );
  QCOMPARE( points1.count(), 3 );
  QCOMPARE( points1[0], QgsPointXY( 10, 0 ) );
  QCOMPARE( points1[1], QgsPointXY( 20, 10 ) );
  QCOMPARE( points1[2], QgsPointXY( 10, 10 ) );

  vl->startEditing();

  // add a shortcut
  QgsFeature f( make_feature( QStringLiteral( "LINESTRING(10 0, 10 10)" ) ) );
  vl->addFeature( f );

  QgsPolylineXY points2 = tracer.findShortestPath( QgsPointXY( 10, 0 ), QgsPointXY( 10, 10 ) );
  QCOMPARE( points2.count(), 2 );
  QCOMPARE( points2[0], QgsPointXY( 10, 0 ) );
  QCOMPARE( points2[1], QgsPointXY( 10, 10 ) );

  // delete the shortcut
  vl->deleteFeature( f.id() );

  QgsPolylineXY points3 = tracer.findShortestPath( QgsPointXY( 10, 0 ), QgsPointXY( 10, 10 ) );
  QCOMPARE( points3.count(), 3 );
  QCOMPARE( points3[0], QgsPointXY( 10, 0 ) );
  QCOMPARE( points3[1], QgsPointXY( 20, 10 ) );
  QCOMPARE( points3[2], QgsPointXY( 10, 10 ) );

  // make the shortcut again from a different feature
  QgsGeometry g = QgsGeometry::fromWkt( QStringLiteral( "LINESTRING(10 0, 10 10)" ) );
  vl->changeGeometry( 2, g );  // change bottom line (second item in wkts)

  QgsPolylineXY points4 = tracer.findShortestPath( QgsPointXY( 10, 0 ), QgsPointXY( 10, 10 ) );
  QCOMPARE( points4.count(), 2 );
  QCOMPARE( points4[0], QgsPointXY( 10, 0 ) );
  QCOMPARE( points4[1], QgsPointXY( 10, 10 ) );

  QgsPolylineXY points5 = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 10, 0 ) );
  QCOMPARE( points5.count(), 4 );
  QCOMPARE( points5[0], QgsPointXY( 0, 0 ) );
  QCOMPARE( points5[1], QgsPointXY( 0, 10 ) );
  QCOMPARE( points5[2], QgsPointXY( 10, 10 ) );
  QCOMPARE( points5[3], QgsPointXY( 10, 0 ) );

  vl->rollBack();

  delete vl;
}

void TestQgsTracer::testExtent()
{
  // check whether the tracer correctly handles the extent limitation

  // same shape as in testSimple()
  QStringList wkts;
  wkts  << QStringLiteral( "LINESTRING(0 0, 0 10)" )
        << QStringLiteral( "LINESTRING(0 0, 10 0)" )
        << QStringLiteral( "LINESTRING(0 10, 20 10)" )
        << QStringLiteral( "LINESTRING(10 0, 20 10)" );

  QgsVectorLayer *vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << vl );
  tracer.setExtent( QgsRectangle( 0, 0, 5, 5 ) );
  tracer.init();

  QgsPolylineXY points1 = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 10, 0 ) );
  QCOMPARE( points1.count(), 2 );
  QCOMPARE( points1[0], QgsPointXY( 0, 0 ) );
  QCOMPARE( points1[1], QgsPointXY( 10, 0 ) );

  const QgsPolylineXY points2 = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 20, 10 ) );
  QCOMPARE( points2.count(), 0 );
}

void TestQgsTracer::testReprojection()
{
  QStringList wkts;
  wkts  << QStringLiteral( "LINESTRING(1 0, 2 0)" );

  QgsVectorLayer *vl = make_layer( wkts );

  const QgsCoordinateReferenceSystem dstCrs( QStringLiteral( "EPSG:3857" ) );
  const QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), dstCrs, QgsProject::instance() );
  const QgsPointXY p1 = ct.transform( QgsPointXY( 1, 0 ) );
  const QgsPointXY p2 = ct.transform( QgsPointXY( 2, 0 ) );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << vl );
  const QgsCoordinateTransformContext context;
  tracer.setDestinationCrs( dstCrs, context );
  tracer.init();

  const QgsPolylineXY points1 = tracer.findShortestPath( p1, p2 );
  QCOMPARE( points1.count(), 2 );
}

void TestQgsTracer::testCurved()
{
  QStringList wkts;
  wkts  << QStringLiteral( "CIRCULARSTRING(0 0, 10 10, 20 0)" );

  /* This shape - half of a circle (r = 10)
   * 10,10  _
   *       / \
   * 0,0  |   |  20,0
   */

  QgsVectorLayer *vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << vl );

  QgsPolylineXY points1 = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 10, 10 ) );

  QVERIFY( !points1.isEmpty() );

  const QgsGeometry tmpG1 = QgsGeometry::fromPolylineXY( points1 );
  const double l = tmpG1.length();

  // fuzzy comparison as QCOMPARE is too strict for this case
  const double full_circle_length = 2 * M_PI * 10;
  QGSCOMPARENEAR( l, full_circle_length / 4, 0.01 );

  QCOMPARE( points1[0], QgsPointXY( 0, 0 ) );
  QCOMPARE( points1[points1.count() - 1], QgsPointXY( 10, 10 ) );

  delete vl;
}

void TestQgsTracer::testOffset()
{
  QStringList wkts;
  wkts  << QStringLiteral( "LINESTRING(0 0, 0 10)" )
        << QStringLiteral( "LINESTRING(0 0, 10 0)" )
        << QStringLiteral( "LINESTRING(0 10, 20 10)" )
        << QStringLiteral( "LINESTRING(10 0, 20 10)" );

  /* This shape - nearly a square (one side is shifted to have exactly one shortest
   * path between corners):
   * 0,10 +----+  20,10
   *      |   /
   * 0,0  +--+  10,0
   */

  QgsVectorLayer *vl = make_layer( wkts );

  QgsTracer tracer;
  tracer.setLayers( QList<QgsVectorLayer *>() << vl );

  // curve on the right side
  tracer.setOffset( -1 );
  QgsPolylineXY points1 = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 20, 10 ) );
  QCOMPARE( points1.count(), 3 );
  QCOMPARE( points1[0], QgsPointXY( 0, -1 ) );
  QCOMPARE( points1[1], QgsPointXY( 10 + sqrt( 2 ) - 1, -1 ) );
  QCOMPARE( points1[2], QgsPointXY( 20 + sqrt( 2 ) / 2, 10 - sqrt( 2 ) / 2 ) );

  // curve on the left side
  tracer.setOffset( 1 );
  QgsPolylineXY points2 = tracer.findShortestPath( QgsPointXY( 0, 0 ), QgsPointXY( 20, 10 ) );
  QCOMPARE( points2.count(), 3 );
  QCOMPARE( points2[0], QgsPointXY( 0, 1 ) );
  QCOMPARE( points2[1], QgsPointXY( 10 - sqrt( 2 ) + 1, 1 ) );
  QCOMPARE( points2[2], QgsPointXY( 20 - sqrt( 2 ) / 2, 10 + sqrt( 2 ) / 2 ) );

  delete vl;
}


QGSTEST_MAIN( TestQgsTracer )
#include "testqgstracer.moc"
