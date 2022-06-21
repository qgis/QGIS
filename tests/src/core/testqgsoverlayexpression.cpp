/***************************************************************************
     test_qgsoverlayexpression.cpp
     --------------------------------------
    Date                 : 20.6.2019
    Copyright            : (C) 2019 by Matthias Kuhn
    Email                : matthias@opengis.ch
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
#include <QtConcurrentMap>

#include <qgsapplication.h>
//header for class being tested
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsgeometry.h"
#include "qgsrenderchecker.h"
#include "qgsexpressioncontext.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsvectorlayerutils.h"
#include "qgsexpressioncontextutils.h"

#include "geos_c.h"

class TestQgsOverlayExpression: public QObject
{
    Q_OBJECT

  public:

    TestQgsOverlayExpression() = default;
    void testOverlayExpression();
    void testOverlayExpression_data();

  private:
    QgsVectorLayer *mRectanglesLayer = nullptr;
    QgsVectorLayer *mPolyLayer = nullptr;

  private slots:

    void initTestCase();

    void cleanupTestCase();
    void testOverlaySelf();

    void testOverlay();
    void testOverlay_data();

    void testOverlayMeasure();
    void testOverlayMeasure_data();

};



void TestQgsOverlayExpression::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';

  const QString rectanglesFileName = testDataDir + QStringLiteral( "rectangles.shp" );
  const QFileInfo rectanglesFileInfo( rectanglesFileName );
  mRectanglesLayer = new QgsVectorLayer( rectanglesFileInfo.filePath(),
                                         QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
  const QString polygonsFileName = testDataDir + QStringLiteral( "polys_overlapping_with_id.shp" );
  const QFileInfo polygonsFileInfo( polygonsFileName );
  mPolyLayer = new QgsVectorLayer( polygonsFileInfo.filePath(),
                                   QStringLiteral( "polys" ), QStringLiteral( "ogr" ) );

  // Create linestrings target layer for test
  QgsVectorLayer *linestringsLayer = new  QgsVectorLayer{ QStringLiteral( "LineString?crs=epsg:4326" ), QStringLiteral( "linestrings" ), QStringLiteral( "memory" ) };

  QgsFeature f1 { linestringsLayer->fields() };
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LINESTRING(1 0, 2 0)" ) ) );
  QgsFeature f2 { linestringsLayer->fields() };
  f2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LINESTRING(3 0, 5 0)" ) ) );

  linestringsLayer->dataProvider()->addFeature( f1 );
  linestringsLayer->dataProvider()->addFeature( f2 );

  // Points layer for tests
  QgsVectorLayer *pointsLayer = new  QgsVectorLayer{ QStringLiteral( "Point?crs=epsg:4326" ), QStringLiteral( "points" ), QStringLiteral( "memory" ) };

  QgsFeature f3 { pointsLayer->fields() };
  f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT(1 0)" ) ) );
  QgsFeature f4 { pointsLayer->fields() };
  f4.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT(3 0)" ) ) );

  pointsLayer->dataProvider()->addFeature( f3 );
  pointsLayer->dataProvider()->addFeature( f4 );

  QgsVectorLayer *polygonsLayer = new QgsVectorLayer{ R"layer_definition(Polygon?crs=EPSG:2051&uid={d6f1eaf3-ec08-4e6c-9e39-6dab242e73f4}&field=fid:integer)layer_definition", QStringLiteral( "polygons2" ), QStringLiteral( "memory" ) };
  QgsFeature f { polygonsLayer->fields() };
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((2604689.23400000017136335 1231339.72999999998137355, 2604696.20599999977275729 1231363.55400000000372529, 2604720.66000000014901161 1231358.27099999994970858, 2604713.89399999985471368 1231333.42900000000372529, 2604704.85499999998137355 1231335.10299999988637865, 2604695.41300000017508864 1231337.88999999989755452, 2604689.23400000017136335 1231339.72999999998137355))" ) ) );
  f.setAttribute( 0, 997 );
  polygonsLayer->dataProvider()->addFeature( f );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((2604689.01899999985471368 1231313.05799999996088445, 2604695.41300000017508864 1231337.88999999989755452, 2604704.85499999998137355 1231335.10299999988637865, 2604713.89399999985471368 1231333.42900000000372529, 2604719.80599999986588955 1231332.34700000006705523, 2604713.325999999884516 1231305.375, 2604697.20899999979883432 1231310.25600000005215406, 2604689.01899999985471368 1231313.05799999996088445))" ) ) );
  f.setAttribute( 0, 1002 );
  polygonsLayer->dataProvider()->addFeature( f );
  QVERIFY( polygonsLayer->isValid() );
  QCOMPARE( polygonsLayer->featureCount(), 2 );

  QgsProject::instance()->addMapLayer( pointsLayer );
  QgsProject::instance()->addMapLayer( linestringsLayer );
  QgsProject::instance()->addMapLayer( mRectanglesLayer );
  QgsProject::instance()->addMapLayer( mPolyLayer );
  QgsProject::instance()->addMapLayer( polygonsLayer );


}

void TestQgsOverlayExpression::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsOverlayExpression::testOverlay()
{
  QFETCH( QString, expression );
  QFETCH( QString, geometry );
  QFETCH( bool, expectedResult );

  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  QgsFeature feat;
  feat.setGeometry( QgsGeometry::fromWkt( geometry ) );
  context.setFeature( feat );

  QgsExpression exp( expression );
  QVERIFY2( exp.prepare( &context ), exp.parserErrorString().toUtf8().constData() );
  const QVariant result = exp.evaluate( &context );

  QCOMPARE( result.toBool(), expectedResult );

}

void TestQgsOverlayExpression::testOverlay_data()
{
  //test passing named parameters to functions
  QTest::addColumn<QString>( "expression" );
  QTest::addColumn<QString>( "geometry" );
  QTest::addColumn<bool>( "expectedResult" );

  QTest::newRow( "intersects" ) << "overlay_intersects('rectangles')" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << true;
  QTest::newRow( "intersects [cached]" ) << "overlay_intersects('rectangles',cache:=true)" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << true;

  QTest::newRow( "intersects no match" ) << "overlay_intersects('rectangles')" << "POLYGON((10 0, 5 0, 5 5, 10 5, 10 0))" << false;
  QTest::newRow( "intersects no match [cached]" ) << "overlay_intersects('rectangles',cache:=true)" << "POLYGON((10 0, 5 0, 5 5, 10 5, 10 0))" << false;

  QTest::newRow( "touches" ) << "overlay_touches('rectangles')" << "POLYGON((-86 54, -95 50, -81 50, -86 54))" << true;
  QTest::newRow( "touches [cached]" ) << "overlay_touches('rectangles',cache:=true)" << "POLYGON((-86 54, -95 50, -81 50, -86 54))" << true;

  QTest::newRow( "touches no intersects no match" ) << "overlay_touches('rectangles')" << "POLYGON((-86 54, -95 51, -81 51, -86 54))" << false;
  QTest::newRow( "touches no intersects no match [cached]" ) << "overlay_touches('rectangles',cache:=true)" << "POLYGON((-86 54, -95 51, -81 51, -86 54))" << false;

  QTest::newRow( "touches intersects no match" ) << "overlay_touches('rectangles')" << "POLYGON((-86 54, -95 49, -81 49, -86 54))" << false;
  QTest::newRow( "touches intersects no match [cached]" ) << "overlay_touches('rectangles',cache:=true)" << "POLYGON((-86 54, -95 49, -81 49, -86 54))" << false;

  QTest::newRow( "within" ) << "overlay_within('rectangles')" << "POINT(-83 47)" << true;
  QTest::newRow( "within [cached]" ) << "overlay_within('rectangles',cache:=true)" << "POINT(-83 47)" << true;

  QTest::newRow( "within no match" ) << "overlay_within('rectangles')" << "POINT(-122 43)" << false;
  QTest::newRow( "within no match [cached]" ) << "overlay_within('rectangles',cache:=true)" << "POINT(-122 43)" << false;

  QTest::newRow( "contains" ) << "overlay_contains('rectangles')" << "POLYGON((-166 15, -166 58, -107 58, -107 15, -166 15))" << true;
  QTest::newRow( "contains [cached]" ) << "overlay_contains('rectangles',cache:=true)" << "POLYGON((-166 15, -166 58, -107 58, -107 15, -166 15))" << true;

  QTest::newRow( "contains no match" ) << "overlay_contains('rectangles')" << "POLYGON((-156 46, -149 46, -148 37, -156 46))" << false;
  QTest::newRow( "contains no match [cached]" ) << "overlay_contains('rectangles',cache:=true)" << "POLYGON((-156 46, -149 46, -148 37, -156 46))" << false;

  QTest::newRow( "equals" ) << "overlay_equals('rectangles')" << "MULTIPOLYGON(((-160 50, -145 50, -145 35, -160 35, -160 50)))" << true;
  QTest::newRow( "equals [cached]" ) << "overlay_equals('rectangles',cache:=true)" << "MULTIPOLYGON(((-160 50, -145 50, -145 35, -160 35, -160 50)))" << true;

  QTest::newRow( "equals no match" ) << "overlay_equals('rectangles')" << "POLYGON((-156 46, -149 46, -148 37, -156 46))" << false;
  QTest::newRow( "equals no match [cached]" ) << "overlay_equals('rectangles',cache:=true)" << "POLYGON((-156 46, -149 46, -148 37, -156 46))" << false;

  QTest::newRow( "disjoint" ) << "overlay_disjoint('rectangles')" << "LINESTRING(-155 15, -122 55, -84 4)" << true;
  QTest::newRow( "disjoint [cached]" ) << "overlay_disjoint('rectangles',cache:=true)" << "LINESTRING(-155 15, -122 55, -84 4)" << true;

  QTest::newRow( "disjoint no match" ) << "overlay_disjoint('rectangles')" << "LINESTRING(-155 15, -122 32, -84 4)" << false;
  QTest::newRow( "disjoint no match [cached]" ) << "overlay_disjoint('rectangles',cache:=true)" << "LINESTRING(-155 15, -122 32, -84 4)" << false;

  // Multi part intersection
  QTest::newRow( "intersects min_overlap multi no match" ) << "overlay_intersects('polys', min_overlap:=1.5)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << false;
  QTest::newRow( "intersects min_overlap multi match" ) << "overlay_intersects('polys', min_overlap:=1.34)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << true;

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=9 )
  QTest::newRow( "intersects min_inscribed_circle_radius multi no match" ) << "overlay_intersects('polys', min_inscribed_circle_radius:=1.0)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << false;
  QTest::newRow( "intersects min_inscribed_circle_radius multi match" ) << "overlay_intersects('polys', min_inscribed_circle_radius:=0.5)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << true;
#endif

  // Single part intersection
  QTest::newRow( "intersects min_overlap no match" ) << "overlay_intersects('polys', min_overlap:=1.5)" << "POLYGON((-105 33.75, -102.76 33.75, -102.76 35.2, -105 35.2, -105 33.75))" << false;
  QTest::newRow( "intersects min_overlap match" ) << "overlay_intersects('polys', min_overlap:=1.34)" << "POLYGON((-105 33.75, -102.76 33.75, -102.76 35.2, -105 35.2, -105 33.75))" << true;

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=9 )
  QTest::newRow( "intersects min_inscribed_circle_radius no match" ) << "overlay_intersects('polys', min_inscribed_circle_radius:=1.0)" << "POLYGON((-105 33.75, -102.76 33.75, -102.76 35.2, -105 35.2, -105 33.75))" << false;
  QTest::newRow( "intersects min_inscribed_circle_radius match" ) << "overlay_intersects('polys', min_inscribed_circle_radius:=0.5)" << "POLYGON((-105 33.75, -102.76 33.75, -102.76 35.2, -105 35.2, -105 33.75))" << true;

  // Test both checks combined: they must all pass
  // Multi part intersection
  QTest::newRow( "intersects multi combined no match 1" ) << "overlay_intersects('polys', min_overlap:=1.5, min_inscribed_circle_radius:=1.0)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << false;
  QTest::newRow( "intersects multi combined no match 2" ) << "overlay_intersects('polys', min_overlap:=1.5, min_inscribed_circle_radius:=0.5)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << false;

  QTest::newRow( "intersects multi combined no match 3" ) << "overlay_intersects('polys', min_overlap:=1.34, min_inscribed_circle_radius:=1.0)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << false;
  QTest::newRow( "intersects multi combined match" ) << "overlay_intersects('polys', min_overlap:=1.34, min_inscribed_circle_radius:=0.5)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << true;

  // Single part intersection
  QTest::newRow( "intersects combined no match 1" ) << "overlay_intersects('polys', min_overlap:=1.5, min_inscribed_circle_radius:=1.0)" << "POLYGON((-105 33.75, -102.76 33.75, -102.76 35.2, -105 35.2, -105 33.75))" << false;
  QTest::newRow( "intersects combined no match 2" ) << "overlay_intersects('polys', min_overlap:=1.34, min_inscribed_circle_radius:=1.0)" << "POLYGON((-105 33.75, -102.76 33.75, -102.76 35.2, -105 35.2, -105 33.75))" << false;
  QTest::newRow( "intersects combined no match 3" ) << "overlay_intersects('polys', min_overlap:=1.5, min_inscribed_circle_radius:=0.5)" << "POLYGON((-105 33.75, -102.76 33.75, -102.76 35.2, -105 35.2, -105 33.75))" << false;

  QTest::newRow( "intersects combined match" ) << "overlay_intersects('polys', min_overlap:=1.34, min_inscribed_circle_radius:=0.5)" << "POLYGON((-105 33.75, -102.76 33.75, -102.76 35.2, -105 35.2, -105 33.75))" << true;

#endif

  // Check linestrings
  QTest::newRow( "intersects linestring match" ) << "overlay_intersects('polys', min_overlap:=1.76)" << "LINESTRING(-105 33.75, -102.76 33.75)" << true;
  QTest::newRow( "intersects linestring no match" ) << "overlay_intersects('polys', min_overlap:=2.0)" << "LINESTRING(-105 33.75, -102.76 33.75)" << false;

  QTest::newRow( "intersects linestring multi match" ) << "overlay_intersects('polys', min_overlap:=1.76)" << "LINESTRING(-102.76 33.74, -106.12 33.74)" << true;
  QTest::newRow( "intersects linestring multi no match" ) << "overlay_intersects('polys', min_overlap:=2.0)" << "LINESTRING(-102.76 33.74, -106.12 33.74)" << false;

}


void TestQgsOverlayExpression::testOverlayMeasure()
{
  QFETCH( QString, expression );
  QFETCH( QString, geometry );
  QFETCH( QVariantList, expectedResult );

  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  QgsFeature feat;
  feat.setGeometry( QgsGeometry::fromWkt( geometry ) );
  context.setFeature( feat );

  QgsExpression exp( expression );
  QVERIFY2( exp.prepare( &context ), exp.parserErrorString().toUtf8().constData() );
  const QVariant result = exp.evaluate( &context );
  QCOMPARE( result, expectedResult );

}

void TestQgsOverlayExpression::testOverlayMeasure_data()
{

  QTest::addColumn<QString>( "expression" );
  QTest::addColumn<QString>( "geometry" );
  QTest::addColumn<QVariantList>( "expectedResult" );

  QVariantMap expected3;
  expected3.insert( QStringLiteral( "id" ), 3LL );
  expected3.insert( QStringLiteral( "result" ), 3LL );
  expected3.insert( QStringLiteral( "overlap" ), 1.4033836999701634 );
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=9 )
  expected3 .insert( QStringLiteral( "radius" ), 0.5344336346973622 );
#endif
  QVariantMap expected1;
  expected1.insert( QStringLiteral( "id" ), 1LL );
  expected1.insert( QStringLiteral( "result" ), 1LL );
  expected1.insert( QStringLiteral( "overlap" ), 1.2281139270096446 );
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=9 )
  expected1.insert( QStringLiteral( "radius" ),  0.46454276882989376 );
#endif

  QTest::newRow( "intersects min_overlap multi match return measure" ) << "overlay_intersects('polys', expression:=$id, min_overlap:=1.34, return_details:=true)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << ( QVariantList() << expected3 ) ;

  QTest::newRow( "intersects multi match return measure" ) << "overlay_intersects('polys', expression:=$id, return_details:=true)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << ( QVariantList() << expected1 << expected3 ) ;

  QTest::newRow( "intersects multi match return sorted measure" ) << "overlay_intersects('polys', expression:=$id, sort_by_intersection_size:='des', return_details:=true)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << ( QVariantList() << expected3 << expected1 ) ;

  QTest::newRow( "intersects multi match return sorted" ) << "overlay_intersects('polys', expression:=$id, sort_by_intersection_size:='des')" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << ( QVariantList() << 3LL << 1LL ) ;

  QTest::newRow( "intersects multi match return unsorted" ) << "overlay_intersects('polys', expression:=$id)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << ( QVariantList() << 1LL << 3LL ) ;

  QTest::newRow( "intersects multi match return unsorted limit " ) << "overlay_intersects('polys', limit:=1, expression:=$id)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << ( QVariantList() << 1LL ) ;

  QTest::newRow( "intersects multi match return sorted limit " ) << "overlay_intersects('polys', sort_by_intersection_size:='des', limit:=1, expression:=$id)" << "POLYGON((-107.37 33.75, -102.76 33.75, -102.76 36.97, -107.37 36.97, -107.37 33.75))" << ( QVariantList() << 3LL ) ;

  QTest::newRow( "intersects multi match points" ) << "overlay_intersects('polys', expression:=$id)" << "MULTIPOINT((-107.37 33.75), (-102.8 36.97))" << ( QVariantList() << 1LL << 3LL ) ;

  QTest::newRow( "intersects multi match points sorted" ) << "overlay_intersects('polys', sort_by_intersection_size:='des', expression:=$id)" << "MULTIPOINT((-107.37 33.75), (-102.8 36.97))" << ( QVariantList() << 3LL << 1LL );

  {
    // Check returned values from point intersection
    QVariantMap expected3;
    expected3.insert( QStringLiteral( "id" ), 3LL );
    expected3.insert( QStringLiteral( "result" ), 3 );
    expected3.insert( QStringLiteral( "overlap" ), 19.049688572712284 );
    QVariantMap expected1;
    expected1.insert( QStringLiteral( "id" ), 1LL );
    expected1.insert( QStringLiteral( "result" ), 1 );
    expected1.insert( QStringLiteral( "overlap" ), 18.569843123334977 );

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=9 )
    expected3.insert( QStringLiteral( "radius" ), 1.3414663642343596 );
    expected1.insert( QStringLiteral( "radius" ), 1.8924012738149243 );
#endif

    QTest::newRow( "intersects multi match points return sorted" ) << "overlay_intersects('polys', sort_by_intersection_size:='des', return_details:=true, expression:=$id)" << "MULTIPOINT((-107.37 33.75), (-102.8 36.97))" << ( QVariantList() << expected3 << expected1 ) ;

  }

  // Linestring tests!

  QVariantMap expectedLine3;
  expectedLine3.insert( QStringLiteral( "id" ), 3LL );
  expectedLine3.insert( QStringLiteral( "result" ), 3LL );
  expectedLine3.insert( QStringLiteral( "overlap" ), 0.9114785997128507 );
  QVariantMap expectedLine1;
  expectedLine1.insert( QStringLiteral( "id" ), 1LL );
  expectedLine1.insert( QStringLiteral( "result" ), 1LL );
  expectedLine1.insert( QStringLiteral( "overlap" ), 0.4447782690201052 );

  QTest::newRow( "intersects linestring multi match" ) << "overlay_intersects('polys', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << 1LL << 3LL );

  QTest::newRow( "intersects linestring multi match sorted" ) << "overlay_intersects('polys', sort_by_intersection_size:='des', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << 3LL << 1LL );

  QTest::newRow( "intersects linestring multi match sorted limit" ) << "overlay_intersects('polys', limit:=1, sort_by_intersection_size:='des', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << 3LL );

  QTest::newRow( "intersects linestring multi match measure sorted" ) << "overlay_intersects('polys',  sort_by_intersection_size:='des', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << 3LL << 1LL );

  QTest::newRow( "intersects linestring multi match measure sorted asc" ) << "overlay_intersects('polys',  sort_by_intersection_size:='asc', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << 1LL << 3LL );

  // Return measure
  QTest::newRow( "intersects linestring multi match" ) << "overlay_intersects('polys', return_details:=true, expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << expectedLine1 << expectedLine3 );

  QTest::newRow( "intersects linestring multi match sorted" ) << "overlay_intersects('polys', return_details:=true, sort_by_intersection_size:='des', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << expectedLine3 << expectedLine1 );

  QTest::newRow( "intersects linestring multi match sorted asc" ) << "overlay_intersects('polys', return_details:=true, sort_by_intersection_size:='asc', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << expectedLine1 << expectedLine3 );

  QTest::newRow( "intersects linestring multi match sorted limit" ) << "overlay_intersects('polys', return_details:=true, limit:=1, sort_by_intersection_size:='des', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << expectedLine3 );

  QTest::newRow( "intersects linestring multi match measure sorted" ) << "overlay_intersects('polys', return_details:=true, sort_by_intersection_size:='des', expression:=$id)" << "LINESTRING(-102.76 33.74, -102.76 36.44)" << ( QVariantList() << expectedLine3 << expectedLine1 );

  // Test linestring intersections
  {
    QVariantMap expectedLine1;
    expectedLine1.insert( QStringLiteral( "id" ), 1LL );
    expectedLine1.insert( QStringLiteral( "result" ), 1 );
    expectedLine1.insert( QStringLiteral( "overlap" ), 0 );
    QVariantMap expectedLine2;
    expectedLine2.insert( QStringLiteral( "id" ), 2LL );
    expectedLine2.insert( QStringLiteral( "result" ), 2 );
    expectedLine2.insert( QStringLiteral( "overlap" ), 0 );

    QTest::newRow( "intersects linestring single match" ) << "overlay_intersects('linestrings', return_details:=true, expression:=$id)" << "LINESTRING(1.5 1, 1.5 -1)" << ( QVariantList() << expectedLine1 );


    QTest::newRow( "intersects linestring single match fail overlap" ) << "overlay_intersects('linestrings', min_overlap:=0.5, return_details:=true, expression:=$id)" << "LINESTRING(1.5 1, 1.5 -1)" << ( QVariantList() );
    QTest::newRow( "intersects linestring no match" ) << "overlay_intersects('linestrings', return_details:=true, expression:=$id)" << "LINESTRING(1.5 2, 1.5 1)" << ( QVariantList() );
    QTest::newRow( "intersects linestring multi match" ) << "overlay_intersects('linestrings', return_details:=true, expression:=$id)" << "LINESTRING(1.5 1, 1.5 -1, 4 -1, 4 1)" << ( QVariantList() << expectedLine1 << expectedLine2 );
    expectedLine2[QStringLiteral( "overlap" )] = 0.5;
    QTest::newRow( "intersects linestring multi match sorted" ) << "overlay_intersects('linestrings', return_details:=true, sort_by_intersection_size:='des', expression:=$id)" << "LINESTRING(1.5 1, 1.5 -1, 4 -1, 4 0, 4.5 0)" << ( QVariantList() << expectedLine2 << expectedLine1 );

    // Full length of targets is expected
    expectedLine2[QStringLiteral( "overlap" )] = 2.0;
    expectedLine1[QStringLiteral( "overlap" )] = 1.0;

    QTest::newRow( "intersects linestring multi match points" ) << "overlay_intersects('linestrings', return_details:=true, expression:=$id)" << "MULTIPOINT((1.5 0), (3.5 0))" << ( QVariantList() << expectedLine1 << expectedLine2 );
    QTest::newRow( "intersects linestring multi match points sorted" ) << "overlay_intersects('linestrings', sort_by_intersection_size:='des', return_details:=true, expression:=$id)" << "MULTIPOINT((1.5 0), (3.5 0))" << ( QVariantList() << expectedLine2 << expectedLine1 );

  }

  // Test point target layer (no sorting supported)
  {
    QVariantMap expectedPoint1;
    expectedPoint1.insert( QStringLiteral( "id" ), 1LL );
    expectedPoint1.insert( QStringLiteral( "result" ), 1 );
    expectedPoint1.insert( QStringLiteral( "overlap" ), 0 );
    QVariantMap expectedPoint2;
    expectedPoint2.insert( QStringLiteral( "id" ), 2LL );
    expectedPoint2.insert( QStringLiteral( "result" ), 2 );
    expectedPoint2.insert( QStringLiteral( "overlap" ), 0 );

    QTest::newRow( "intersects points single match" ) << "overlay_intersects('points', return_details:=true, expression:=$id)" << "POLYGON((0 -1, 1.5 -1, 1.5 1, 0 1, 0 -1))" << ( QVariantList() << expectedPoint1 );
    QTest::newRow( "intersects points multi match" ) << "overlay_intersects('points', return_details:=true, expression:=$id)" << "POLYGON((0 -1, 3.5 -1, 3.5 1, 0 1, 0 -1))" << ( QVariantList() << expectedPoint1 << expectedPoint2 );
  }

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=9 )
  // Test polygon intersection resulting in a point with min_inscribed_circle_radius and expression
  QTest::newRow( "intersects point expression no match" ) << "overlay_intersects('polys', expression:=$id, min_inscribed_circle_radius:=0.5, sort_by_intersection_size:='desc')" << "POLYGON((-90.825 34.486, -89.981 35.059, -90.009 33.992, -90.825 34.486))" << ( QVariantList() );
  // Test polygon intersection resulting in a line with min_inscribed_circle_radius and expression
  QVariantMap expectedPoly;
  expectedPoly.insert( QStringLiteral( "id" ), 2LL );
  expectedPoly.insert( QStringLiteral( "overlap" ), 667.992431640625 );
  expectedPoly.insert( QStringLiteral( "radius" ), 12.576424447201404 );
  expectedPoly.insert( QStringLiteral( "result" ), 1002 );
  QTest::newRow( "intersects line expression no match" ) << "overlay_intersects('polygons2', expression:=fid, return_details:=true, min_inscribed_circle_radius:=3, sort_by_intersection_size:='desc')" << "Polygon ((2604689.01899999985471368 1231313.05799999996088445, 2604695.41300000017508864 1231337.88999999989755452, 2604704.85499999998137355 1231335.10299999988637865, 2604713.89399999985471368 1231333.42900000000372529, 2604719.80599999986588955 1231332.34700000006705523, 2604713.325999999884516 1231305.375, 2604697.20899999979883432 1231310.25600000005215406, 2604689.01899999985471368 1231313.05799999996088445))" << ( QVariantList() << expectedPoly );
#endif
}

void TestQgsOverlayExpression::testOverlayExpression()
{
  QFETCH( QString, expression );
  QFETCH( QString, geometry );
  QFETCH( QVariantList, expectedResult );

  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  QgsFeature feat;
  feat.setGeometry( QgsGeometry::fromWkt( geometry ) );
  context.setFeature( feat );

  QgsExpression exp( expression );
  QVERIFY2( exp.prepare( &context ), exp.parserErrorString().toUtf8().constData() );
  const QVariantList result = exp.evaluate( &context ).value<QVariantList>();

  QCOMPARE( result.count(), expectedResult.count() );
  QCOMPARE( result, expectedResult );
}

void TestQgsOverlayExpression::testOverlayExpression_data()
{
  //test passing named parameters to functions
  QTest::addColumn<QString>( "expression" );
  QTest::addColumn<QString>( "geometry" );
  QTest::addColumn<QVariantList>( "expectedResult" );

  QTest::newRow( "intersects get geometry" ) << "overlay_intersects('rectangles', geom_to_wkt($geometry))" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << QVariantList { QVariant( QStringLiteral( "MultiPolygon (((-130 40, -115 40, -115 25, -130 25, -130 40)))" ) ) };
  QTest::newRow( "intersects get geometry [cached]" ) << "overlay_intersects('rectangles', geom_to_wkt($geometry),cache:=true)" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << QVariantList { QVariant( QStringLiteral( "MultiPolygon (((-130 40, -115 40, -115 25, -130 25, -130 40)))" ) ) };

  QTest::newRow( "intersects get ids" ) << "overlay_intersects('rectangles', id)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 2, 3 };
  QTest::newRow( "intersects get ids [cached]" ) << "overlay_intersects('rectangles', id,cache:=true)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 2, 3 };

  QTest::newRow( "intersects get ids limit 2" ) << "overlay_intersects('rectangles', id, limit:=2)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 2 };
  QTest::newRow( "intersects get ids limit 2 [cached]" ) << "overlay_intersects('rectangles', id, limit:=2,cache:=true)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 2 };

  QTest::newRow( "intersects filtered get ids" ) << "overlay_intersects('rectangles', id, id!=2)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 3 };
  QTest::newRow( "intersects filtered get ids [cached]" ) << "overlay_intersects('rectangles', id, id!=2,cache:=true)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 3 };

  QTest::newRow( "intersects filtered get ids limit 1" ) << "overlay_intersects('rectangles', id, id!=1, 1)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 2 };
  QTest::newRow( "intersects filtered get ids limit 1 [cached]" ) << "overlay_intersects('rectangles', id, id!=1, 1,cache:=true)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 2 };

  QTest::newRow( "touches get ids" ) << "overlay_touches('rectangles',id)" << "POLYGON((-86 54, -95 50, -81 50, -86 54))" << QVariantList { 3 };
  QTest::newRow( "touches get ids [cached]" ) << "overlay_touches('rectangles',id,cache:=true)" << "POLYGON((-86 54, -95 50, -81 50, -86 54))" << QVariantList { 3 };

  QTest::newRow( "equals get ids" ) << "overlay_equals('rectangles',id)" << "MULTIPOLYGON(((-160 50, -145 50, -145 35, -160 35, -160 50)))" << QVariantList { 1 };
  QTest::newRow( "equals get ids [cached]" ) << "overlay_equals('rectangles',id,cache:=true)" << "MULTIPOLYGON(((-160 50, -145 50, -145 35, -160 35, -160 50)))" << QVariantList { 1 };

  QTest::newRow( "disjoint get ids" ) << "overlay_disjoint('rectangles',id)" << "LINESTRING(-155 15, -122 55, -84 4)" << QVariantList { 1, 2, 3 };
  QTest::newRow( "disjoint get ids [cached]" ) << "overlay_disjoint('rectangles',id,cache:=true)" << "LINESTRING(-155 15, -122 55, -84 4)" << QVariantList { 1, 2, 3 };

  QTest::newRow( "disjoint get ids limit 2" ) << "overlay_disjoint('rectangles',id, limit:=2)" << "LINESTRING(-155 15, -122 55, -84 4)" << QVariantList { 1, 2 };
  QTest::newRow( "disjoint get ids limit 2 [cached]" ) << "overlay_disjoint('rectangles',id, limit:=2,cache:=true)" << "LINESTRING(-155 15, -122 55, -84 4)" << QVariantList { 1, 2 };

  QTest::newRow( "nearest" ) << "overlay_nearest('rectangles',id)" << "POINT(-135 38)" << QVariantList { 2 };
  QTest::newRow( "nearest [cached]" ) << "overlay_nearest('rectangles',id,cache:=true)" << "POINT(-135 38)" << QVariantList { 2 };

  QTest::newRow( "nearest filtered" ) << "overlay_nearest('rectangles',id,id!=2)" << "POINT(-135 38)" << QVariantList { 1 };
  QTest::newRow( "nearest filtered [cached]" ) << "overlay_nearest('rectangles',id,id!=2,cache:=true)" << "POINT(-135 38)" << QVariantList { 1 };

  QTest::newRow( "nearest limited" ) << "overlay_nearest('rectangles',id,limit:=2)" << "POINT(-135 38)" << QVariantList { 2, 1 };
  QTest::newRow( "nearest limited [cached]" ) << "overlay_nearest('rectangles',id,limit:=2,cache:=true)" << "POINT(-135 38)" << QVariantList { 2, 1 };

  QTest::newRow( "nearest limited filtered" ) << "overlay_nearest('rectangles',id,id!=2,limit:=2)" << "POINT(-135 38)" << QVariantList { 1, 3 };
  QTest::newRow( "nearest limited filtered [cached]" ) << "overlay_nearest('rectangles',id,id!=2,limit:=2,cache:=true)" << "POINT(-135 38)" << QVariantList { 1, 3 };

  // specific test for issue #39068
  QTest::newRow( "intersects get ids limit 2 #39068" ) << "overlay_intersects('rectangles', id, limit:=2)" << "LINESTRING(-178 52, -133 33, -132 42, -64 46)" << QVariantList { 1, 3 };
  QTest::newRow( "intersects get ids limit 2 #39068 [cached]" ) << "overlay_intersects('rectangles', id, limit:=2,cache:=true)" << "LINESTRING(-178 52, -133 33, -132 42, -64 46)" << QVariantList { 1, 3 };
}


void TestQgsOverlayExpression::testOverlaySelf()
{
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  context.appendScope( QgsExpressionContextUtils::layerScope( mPolyLayer ) );

  QgsExpression exp( "overlay_intersects('polys')" );
  QVERIFY2( exp.prepare( &context ), exp.parserErrorString().toUtf8().constData() );

  QgsFeature feat;
  QVariant result;

  // Feature 0 does not self-intersect
  feat = mPolyLayer->getFeature( 0 );
  context.setFeature( feat );
  result = exp.evaluate( &context );
  QCOMPARE( result.toBool(), false );

  // Feature 10 self-intersects
  feat = mPolyLayer->getFeature( 10 );
  context.setFeature( feat );
  result = exp.evaluate( &context );
  QCOMPARE( result.toBool(), true );

}



QGSTEST_MAIN( TestQgsOverlayExpression )

#include "testqgsoverlayexpression.moc"
