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

class TestQgsOverlayExpression: public QObject
{
    Q_OBJECT

  public:

    TestQgsOverlayExpression() = default;

  private:
    QgsVectorLayer *mRectanglesLayer = nullptr;
    QgsVectorLayer *mPolyLayer = nullptr;

  private slots:

    void initTestCase();

    void cleanupTestCase();

    void testOverlay();
    void testOverlay_data();

    void testOverlayExpression();
    void testOverlayExpression_data();
};



void TestQgsOverlayExpression::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';

  QString rectanglesFileName = testDataDir + QStringLiteral( "rectangles.shp" );
  QFileInfo rectanglesFileInfo( rectanglesFileName );
  mRectanglesLayer = new QgsVectorLayer( rectanglesFileInfo.filePath(),
                                         QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
  QString polygonsFileName = testDataDir + QStringLiteral( "polys_overlapping_with_id.shp" );
  QFileInfo polygonsFileInfo( polygonsFileName );
  mPolyLayer = new QgsVectorLayer( polygonsFileInfo.filePath(),
                                   QStringLiteral( "polys" ), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( mRectanglesLayer );
  QgsProject::instance()->addMapLayer( mPolyLayer );
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

  QTest::newRow( "intersects" ) << "geometry_overlay_intersects('rectangles')" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << true;
  QTest::newRow( "intersects [cached]" ) << "geometry_overlay_intersects('rectangles',cache:=true)" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << true;

  QTest::newRow( "intersects no match" ) << "geometry_overlay_intersects('rectangles')" << "POLYGON((10 0, 5 0, 5 5, 10 5, 10 0))" << false;
  QTest::newRow( "intersects no match [cached]" ) << "geometry_overlay_intersects('rectangles',cache:=true)" << "POLYGON((10 0, 5 0, 5 5, 10 5, 10 0))" << false;

  QTest::newRow( "touches" ) << "geometry_overlay_touches('rectangles')" << "POLYGON((-86 54, -95 50, -81 50, -86 54))" << true;
  QTest::newRow( "touches [cached]" ) << "geometry_overlay_touches('rectangles',cache:=true)" << "POLYGON((-86 54, -95 50, -81 50, -86 54))" << true;

  QTest::newRow( "touches no intersects no match" ) << "geometry_overlay_touches('rectangles')" << "POLYGON((-86 54, -95 51, -81 51, -86 54))" << false;
  QTest::newRow( "touches no intersects no match [cached]" ) << "geometry_overlay_touches('rectangles',cache:=true)" << "POLYGON((-86 54, -95 51, -81 51, -86 54))" << false;

  QTest::newRow( "touches intersects no match" ) << "geometry_overlay_touches('rectangles')" << "POLYGON((-86 54, -95 49, -81 49, -86 54))" << false;
  QTest::newRow( "touches intersects no match [cached]" ) << "geometry_overlay_touches('rectangles',cache:=true)" << "POLYGON((-86 54, -95 49, -81 49, -86 54))" << false;

  QTest::newRow( "within" ) << "geometry_overlay_within('rectangles')" << "POLYGON((-166 15, -166 58, -107 58, -107 15, -166 15))" << true;
  QTest::newRow( "within [cached]" ) << "geometry_overlay_within('rectangles',cache:=true)" << "POLYGON((-166 15, -166 58, -107 58, -107 15, -166 15))" << true;

  QTest::newRow( "within no match" ) << "geometry_overlay_within('rectangles')" << "POLYGON((-156 46, -149 46, -148 37, -156 46))" << false;
  QTest::newRow( "within no match [cached]" ) << "geometry_overlay_within('rectangles',cache:=true)" << "POLYGON((-156 46, -149 46, -148 37, -156 46))" << false;

  QTest::newRow( "contains" ) << "geometry_overlay_contains('rectangles')" << "POINT(-83 47)" << true;
  QTest::newRow( "contains [cached]" ) << "geometry_overlay_contains('rectangles',cache:=true)" << "POINT(-83 47)" << true;

  QTest::newRow( "contains no match" ) << "geometry_overlay_contains('rectangles')" << "POINT(-122 43)" << false;
  QTest::newRow( "contains no match [cached]" ) << "geometry_overlay_contains('rectangles',cache:=true)" << "POINT(-122 43)" << false;

  QTest::newRow( "equals" ) << "geometry_overlay_equals('rectangles')" << "MULTIPOLYGON(((-160 50, -145 50, -145 35, -160 35, -160 50)))" << true;
  QTest::newRow( "equals [cached]" ) << "geometry_overlay_equals('rectangles',cache:=true)" << "MULTIPOLYGON(((-160 50, -145 50, -145 35, -160 35, -160 50)))" << true;

  QTest::newRow( "equals no match" ) << "geometry_overlay_equals('rectangles')" << "POLYGON((-156 46, -149 46, -148 37, -156 46))" << false;
  QTest::newRow( "equals no match [cached]" ) << "geometry_overlay_equals('rectangles',cache:=true)" << "POLYGON((-156 46, -149 46, -148 37, -156 46))" << false;

  QTest::newRow( "disjoint" ) << "geometry_overlay_disjoint('rectangles')" << "LINESTRING(-155 15, -122 55, -84 4)" << true;
  QTest::newRow( "disjoint [cached]" ) << "geometry_overlay_disjoint('rectangles',cache:=true)" << "LINESTRING(-155 15, -122 55, -84 4)" << true;

  QTest::newRow( "disjoint no match" ) << "geometry_overlay_disjoint('rectangles')" << "LINESTRING(-155 15, -122 32, -84 4)" << false;
  QTest::newRow( "disjoint no match [cached]" ) << "geometry_overlay_disjoint('rectangles',cache:=true)" << "LINESTRING(-155 15, -122 32, -84 4)" << false;
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


  QTest::newRow( "intersects get geometry" ) << "geometry_overlay_intersects('rectangles', geom_to_wkt($geometry))" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << QVariantList { QVariant( QStringLiteral( "MultiPolygon (((-130 40, -115 40, -115 25, -130 25, -130 40)))" ) ) };
  QTest::newRow( "intersects get geometry [cached]" ) << "geometry_overlay_intersects('rectangles', geom_to_wkt($geometry),cache:=true)" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << QVariantList { QVariant( QStringLiteral( "MultiPolygon (((-130 40, -115 40, -115 25, -130 25, -130 40)))" ) ) };

  QTest::newRow( "intersects get ids" ) << "geometry_overlay_intersects('rectangles', id)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 2, 3 };
  QTest::newRow( "intersects get ids [cached]" ) << "geometry_overlay_intersects('rectangles', id,cache:=true)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 2, 3 };

  QTest::newRow( "intersects get ids limit 2" ) << "geometry_overlay_intersects('rectangles', id, limit:=2)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 2 };
  QTest::newRow( "intersects get ids limit 2 [cached]" ) << "geometry_overlay_intersects('rectangles', id, limit:=2,cache:=true)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 2 };

  QTest::newRow( "intersects filtered get ids" ) << "geometry_overlay_intersects('rectangles', id, id!=2)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 3 };
  QTest::newRow( "intersects filtered get ids [cached]" ) << "geometry_overlay_intersects('rectangles', id, id!=2,cache:=true)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 1, 3 };

  QTest::newRow( "intersects filtered get ids limit 1" ) << "geometry_overlay_intersects('rectangles', id, id!=1, 1)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 2 };
  QTest::newRow( "intersects filtered get ids limit 1 [cached]" ) << "geometry_overlay_intersects('rectangles', id, id!=1, 1,cache:=true)" << "LINESTRING(-178 52, -133 33, -64 46)" << QVariantList { 2 };

  QTest::newRow( "touches get ids" ) << "geometry_overlay_touches('rectangles',id)" << "POLYGON((-86 54, -95 50, -81 50, -86 54))" << QVariantList { 3 };
  QTest::newRow( "touches get ids [cached]" ) << "geometry_overlay_touches('rectangles',id,cache:=true)" << "POLYGON((-86 54, -95 50, -81 50, -86 54))" << QVariantList { 3 };

  QTest::newRow( "equals get ids" ) << "geometry_overlay_equals('rectangles',id)" << "MULTIPOLYGON(((-160 50, -145 50, -145 35, -160 35, -160 50)))" << QVariantList { 1 };
  QTest::newRow( "equals get ids [cached]" ) << "geometry_overlay_equals('rectangles',id,cache:=true)" << "MULTIPOLYGON(((-160 50, -145 50, -145 35, -160 35, -160 50)))" << QVariantList { 1 };

  QTest::newRow( "disjoint get ids" ) << "geometry_overlay_disjoint('rectangles',id)" << "LINESTRING(-155 15, -122 55, -84 4)" << QVariantList { 1, 2, 3 };
  QTest::newRow( "disjoint get ids [cached]" ) << "geometry_overlay_disjoint('rectangles',id,cache:=true)" << "LINESTRING(-155 15, -122 55, -84 4)" << QVariantList { 1, 2, 3 };

  QTest::newRow( "disjoint get ids limit 2" ) << "geometry_overlay_disjoint('rectangles',id, limit:=2)" << "LINESTRING(-155 15, -122 55, -84 4)" << QVariantList { 1, 2 };
  QTest::newRow( "disjoint get ids limit 2 [cached]" ) << "geometry_overlay_disjoint('rectangles',id, limit:=2,cache:=true)" << "LINESTRING(-155 15, -122 55, -84 4)" << QVariantList { 1, 2 };

  QTest::newRow( "nearest" ) << "geometry_overlay_nearest('rectangles',id)" << "POINT(-135 38)" << QVariantList { 2 };
  QTest::newRow( "nearest [cached]" ) << "geometry_overlay_nearest('rectangles',id,cache:=true)" << "POINT(-135 38)" << QVariantList { 2 };

  QTest::newRow( "nearest filtered" ) << "geometry_overlay_nearest('rectangles',id,id!=2)" << "POINT(-135 38)" << QVariantList { 1 };
  QTest::newRow( "nearest filtered [cached]" ) << "geometry_overlay_nearest('rectangles',id,id!=2,cache:=true)" << "POINT(-135 38)" << QVariantList { 1 };

  QTest::newRow( "nearest limited" ) << "geometry_overlay_nearest('rectangles',id,limit:=2)" << "POINT(-135 38)" << QVariantList { 2, 1 };
  QTest::newRow( "nearest limited [cached]" ) << "geometry_overlay_nearest('rectangles',id,limit:=2,cache:=true)" << "POINT(-135 38)" << QVariantList { 2, 1 };

  QTest::newRow( "nearest limited filtered" ) << "geometry_overlay_nearest('rectangles',id,id!=2,limit:=2)" << "POINT(-135 38)" << QVariantList { 1, 3 };
  QTest::newRow( "nearest limited filtered [cached]" ) << "geometry_overlay_nearest('rectangles',id,id!=2,limit:=2,cache:=true)" << "POINT(-135 38)" << QVariantList { 1, 3 };
}
QGSTEST_MAIN( TestQgsOverlayExpression )

#include "testqgsoverlayexpression.moc"
