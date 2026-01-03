/***************************************************************************
  testqgsgeometrysnapper.cpp
  --------------------------
Date                 : November 2016
Copyright            : (C) 2016 by Nyall Dawson
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

//header for class being tested
#include "qgsgeometrysnapper.h"
#include "qgsgeometry.h"
#include <qgsapplication.h>
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsgeos.h"
#include <geos_c.h>


class TestQgsGeometrySnapper : public QgsTest
{
    Q_OBJECT

  public:
  public:
    TestQgsGeometrySnapper()
      : QgsTest( u"Geometry Snapper Tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    //! Our tests proper begin here
    void snapPolygonToPolygon();
    void snapPolygonToLine();
    void snapPolygonToPoint();
    void snapLineToLine();
    void snapLineToPolygon();
    void snapLineToPoint();
    void snapPointToPoint();
    void snapPointToLine();
    void snapPointToLinePreferNearest();
    void snapPointToPolygon();
    void endPointSnap();
    void endPointToEndPoint();
    void internalSnapper();
    void insertExtra();
    void duplicateNodes();
    void snapMultiPolygonToPolygon();
};

void TestQgsGeometrySnapper::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}
void TestQgsGeometrySnapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}
void TestQgsGeometrySnapper::init()
{
}
void TestQgsGeometrySnapper::cleanup()
{
}

void TestQgsGeometrySnapper::snapPolygonToPolygon()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Polygon"_s, u"x"_s, u"memory"_s );
  QgsFeature ff( 0 );
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"Polygon((0 0, 10 0, 10 10, 0 10, 0 0))"_s );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  const QgsGeometry polygonGeom = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1))"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( polygonGeom, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))"_s );

  const QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom2, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 0 10, 0 0))"_s );

  // insert new vertex
  const QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom3, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0))"_s );

  // remove vertex
  const QgsGeometry polygonGeom4 = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom4, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))"_s );
}

void TestQgsGeometrySnapper::snapLineToLine()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Linestring"_s, u"x"_s, u"memory"_s );
  QgsFeature ff( 0 );

  // closed linestrings
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 10 0, 10 10, 0 10, 0 0)"_s );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  const QgsGeometry lineGeom = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10, 0 0)"_s );

  const QgsGeometry lineGeom2 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1)"_s );
  result = snapper.snapGeometry( lineGeom2, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 0 10, 0 0)"_s );

  // insert new vertex
  const QgsGeometry lineGeom3 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1)"_s );
  result = snapper.snapGeometry( lineGeom3, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0)"_s );

  // remove vertex
  const QgsGeometry lineGeom4 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1)"_s );
  result = snapper.snapGeometry( lineGeom4, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10, 0 0)"_s );


  // unclosed linestrings
  const QgsGeometry lineGeom5 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10)"_s );
  result = snapper.snapGeometry( lineGeom5, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10)"_s );

  const QgsGeometry lineGeom6 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 0 10)"_s );
  result = snapper.snapGeometry( lineGeom6, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 0 10)"_s );

  // insert new vertex
  const QgsGeometry lineGeom7 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9)"_s );
  result = snapper.snapGeometry( lineGeom7, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10)"_s );

  // remove vertex
  const QgsGeometry lineGeom8 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10)"_s );
  result = snapper.snapGeometry( lineGeom8, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10)"_s );
}

void TestQgsGeometrySnapper::snapLineToPolygon()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Polygon"_s, u"x"_s, u"memory"_s );
  QgsFeature ff( 0 );

  // closed linestrings
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"Polygon((0 0, 10 0, 10 10, 0 10, 0 0))"_s );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  const QgsGeometry lineGeom = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10, 0 0)"_s );

  const QgsGeometry lineGeom2 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1)"_s );
  result = snapper.snapGeometry( lineGeom2, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 0 10, 0 0)"_s );

  // insert new vertex
  const QgsGeometry lineGeom3 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1)"_s );
  result = snapper.snapGeometry( lineGeom3, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0)"_s );

  // remove vertex
  const QgsGeometry lineGeom4 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1)"_s );
  result = snapper.snapGeometry( lineGeom4, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10, 0 0)"_s );


  // unclosed linestrings
  const QgsGeometry lineGeom5 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10)"_s );
  result = snapper.snapGeometry( lineGeom5, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10)"_s );

  const QgsGeometry lineGeom6 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 0 10)"_s );
  result = snapper.snapGeometry( lineGeom6, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 0 10)"_s );

  // insert new vertex
  const QgsGeometry lineGeom7 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9)"_s );
  result = snapper.snapGeometry( lineGeom7, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10)"_s );

  // remove vertex
  const QgsGeometry lineGeom8 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10)"_s );
  result = snapper.snapGeometry( lineGeom8, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10)"_s );
}

void TestQgsGeometrySnapper::snapLineToPoint()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Point"_s, u"x"_s, u"memory"_s );

  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"Point(0 0)"_s );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  const QgsGeometry refGeom2 = QgsGeometry::fromWkt( u"Point(10 0)"_s );
  QgsFeature ff2( 2 );
  ff2.setGeometry( refGeom2 );
  QgsFeatureList flist;
  flist << ff << ff2;
  rl->dataProvider()->addFeatures( flist );

  const QgsGeometry lineGeom = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 10 10, 0 10)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 10 10, 0 10)"_s );

  const QgsGeometry lineGeom2 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 10.1 0, 0 10)"_s );
  result = snapper.snapGeometry( lineGeom2, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 0 10)"_s );

  // insert new vertex
  const QgsGeometry lineGeom3 = QgsGeometry::fromWkt( u"LineString(0.1 -0.1, 20.0 0.0, 20 10, 0 10)"_s );
  result = snapper.snapGeometry( lineGeom3, 1 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 10 0, 20 0, 20 10, 0 10)"_s );
}

void TestQgsGeometrySnapper::snapPolygonToLine()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Linestring"_s, u"x"_s, u"memory"_s );

  // closed linestring
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 10 0, 10 10, 0 10, 0 0)"_s );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  // unclosed linestring
  const QgsGeometry refGeom2 = QgsGeometry::fromWkt( u"LineString(100 0, 110 0, 110 10, 100 10)"_s );
  QgsFeature ff2( 2 );
  ff2.setGeometry( refGeom2 );
  QgsFeatureList flist;
  flist << ff << ff2;
  rl->dataProvider()->addFeatures( flist );


  // snapping to closed linestring
  const QgsGeometry polygonGeom = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1))"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( polygonGeom, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))"_s );

  const QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom2, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 0 10, 0 0))"_s );

  // insert new vertex
  const QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom3, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0))"_s );

  // remove vertex
  const QgsGeometry polygonGeom4 = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom4, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))"_s );


  // snapping to unclosed linestring
  const QgsGeometry polygonGeom5 = QgsGeometry::fromWkt( u"Polygon((100.1 -0.1, 110.1 0, 109.9 10.1, 100 10, 100.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom5, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((100 0, 110 0, 110 10, 100 10, 100 0))"_s );

  const QgsGeometry polygonGeom6 = QgsGeometry::fromWkt( u"Polygon((100.1 -0.1, 110.1 0, 100 10, 100.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom6, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((100 0, 110 0, 100 10, 100 0))"_s );

  // insert new vertex
  const QgsGeometry polygonGeom7 = QgsGeometry::fromWkt( u"Polygon((100.1 -0.1, 120.5 0.5, 120 10, 100 9.9, 100.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom7, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((100 0, 110 0, 120.5 0.5, 120 10, 110 10, 100 10, 100 0))"_s );

  // remove vertex
  const QgsGeometry polygonGeom8 = QgsGeometry::fromWkt( u"Polygon((100.1 -0.1, 110.1 0, 109.9 10.1, 105 10, 100 10, 100.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom8, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((100 0, 110 0, 110 10, 100 10, 100 0))"_s );
}

void TestQgsGeometrySnapper::snapPolygonToPoint()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Point"_s, u"x"_s, u"memory"_s );

  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"Point(0 0)"_s );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  const QgsGeometry refGeom2 = QgsGeometry::fromWkt( u"Point(10 0)"_s );
  QgsFeature ff2( 2 );
  ff2.setGeometry( refGeom2 );
  QgsFeatureList flist;
  flist << ff << ff2;
  rl->dataProvider()->addFeatures( flist );

  const QgsGeometry polygonGeom = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 10.1 0, 10 10, 0 10, 0.1 -0.1))"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( polygonGeom, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))"_s );

  const QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom2, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 0 10, 0 0))"_s );

  // insert new vertex
  const QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( u"Polygon((0.1 -0.1, 20.0 0.0, 20 10, 0 10, 0.1 -0.1))"_s );
  result = snapper.snapGeometry( polygonGeom3, 1 );
  QCOMPARE( result.asWkt(), u"Polygon ((0 0, 10 0, 20 0, 20 10, 0 10, 0 0))"_s );
}

void TestQgsGeometrySnapper::snapPointToPoint()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Point"_s, u"x"_s, u"memory"_s );

  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"Point(0 0)"_s );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  const QgsGeometry refGeom2 = QgsGeometry::fromWkt( u"Point(1 0)"_s );
  QgsFeature ff2( 2 );
  ff2.setGeometry( refGeom2 );
  QgsFeatureList flist;
  flist << ff << ff2;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( u"Point(0.1 -0.1)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), u"Point (0 0)"_s );

  pointGeom = QgsGeometry::fromWkt( u"Point(0.6 -0.1)"_s );
  result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), u"Point (1 0)"_s );
}

void TestQgsGeometrySnapper::snapPointToLine()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Linestring"_s, u"x"_s, u"memory"_s );

  // closed linestring
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 10 0, 10 10, 0 10, 0 0)"_s );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( u"Point(0.1 -0.1)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), u"Point (0 0)"_s );

  pointGeom = QgsGeometry::fromWkt( u"Point(10.6 -0.1)"_s );
  result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), u"Point (10 0)"_s );

  pointGeom = QgsGeometry::fromWkt( u"Point(0.5 0.5)"_s );
  result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), u"Point (0 0)"_s );

  pointGeom = QgsGeometry::fromWkt( u"Point(3 3)"_s );
  result = snapper.snapGeometry( pointGeom, 4 );
  QCOMPARE( result.asWkt(), u"Point (3 0)"_s );
}

void TestQgsGeometrySnapper::snapPointToLinePreferNearest()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Linestring"_s, u"x"_s, u"memory"_s );

  // closed linestring
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 10 0, 10 10, 0 10, 0 0)"_s );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  const QgsGeometry pointGeom = QgsGeometry::fromWkt( u"Point(0.5 0.5)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  const QgsGeometry result = snapper.snapGeometry( pointGeom, 1, QgsGeometrySnapper::PreferClosest );
  QCOMPARE( result.asWkt(), u"Point (0.5 0)"_s );
}

void TestQgsGeometrySnapper::snapPointToPolygon()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Polygon"_s, u"x"_s, u"memory"_s );

  // closed linestring
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"Polygon((0 0, 10 0, 10 10, 0 10, 0 0))"_s );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( u"Point(0.1 -0.1)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), u"Point (0 0)"_s );

  pointGeom = QgsGeometry::fromWkt( u"Point(10.6 -0.1)"_s );
  result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), u"Point (10 0)"_s );
}

void TestQgsGeometrySnapper::endPointSnap()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Linestring"_s, u"x"_s, u"memory"_s );
  QgsFeature ff( 0 );

  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 100 0, 100 100, 0 100)"_s );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  const QgsGeometry lineGeom = QgsGeometry::fromWkt( u"LineString(1 -1, 102 0, 98 102, 0 101)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 10, QgsGeometrySnapper::EndPointPreferNodes );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 102 0, 98 102, 0 100)"_s );

  const QgsGeometry lineGeom2 = QgsGeometry::fromWkt( u"LineString(50 0, 102 0, 98 102, 0 50)"_s );
  result = snapper.snapGeometry( lineGeom2, 1, QgsGeometrySnapper::EndPointPreferNodes );
  QCOMPARE( result.asWkt(), u"LineString (50 0, 102 0, 98 102, 0 50)"_s );

  const QgsGeometry lineGeom3 = QgsGeometry::fromWkt( u"LineString(50 -10, 50 -1)"_s );
  result = snapper.snapGeometry( lineGeom3, 2, QgsGeometrySnapper::EndPointPreferNodes );
  QCOMPARE( result.asWkt(), u"LineString (50 -10, 50 0)"_s );
}

void TestQgsGeometrySnapper::endPointToEndPoint()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Linestring"_s, u"x"_s, u"memory"_s );
  QgsFeature ff( 0 );

  // closed linestrings
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 100 0, 100 100, 0 100)"_s );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  const QgsGeometry lineGeom = QgsGeometry::fromWkt( u"LineString(1 -1, 102 0, 98 102, 0 101)"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 10, QgsGeometrySnapper::EndPointToEndPoint );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 102 0, 98 102, 0 100)"_s );

  const QgsGeometry lineGeom2 = QgsGeometry::fromWkt( u"LineString(50 0, 102 0, 98 102)"_s );
  result = snapper.snapGeometry( lineGeom2, 1, QgsGeometrySnapper::EndPointToEndPoint );
  QCOMPARE( result.asWkt(), u"LineString (50 0, 102 0, 98 102)"_s );

  const QgsGeometry lineGeom3 = QgsGeometry::fromWkt( u"LineString(50 -10, 50 -1)"_s );
  result = snapper.snapGeometry( lineGeom3, 2, QgsGeometrySnapper::EndPointToEndPoint );
  QCOMPARE( result.asWkt(), u"LineString (50 -10, 50 -1)"_s );
}

void TestQgsGeometrySnapper::internalSnapper()
{
  QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 10 0, 10 10)"_s );
  QgsFeature f1( 1 );
  f1.setGeometry( refGeom );

  QgsInternalGeometrySnapper snapper( 2 );
  QgsGeometry result = snapper.snapFeature( f1 );
  QCOMPARE( result.asWkt(), f1.geometry().asWkt() );

  refGeom = QgsGeometry::fromWkt( u"LineString(5 5, 10 11, 15 15)"_s );
  QgsFeature f2( 2 );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt(), u"LineString (5 5, 10 10, 15 15)"_s );

  refGeom = QgsGeometry::fromWkt( u"LineString(20 20, 30 20)"_s );
  QgsFeature f3( 3 );
  f3.setGeometry( refGeom );
  result = snapper.snapFeature( f3 );
  QCOMPARE( result.asWkt(), f3.geometry().asWkt() );

  refGeom = QgsGeometry::fromWkt( u"LineString(0 -1, 5.5 5, 9.8 10, 14.5 14.8)"_s );
  QgsFeature f4( 4 );
  f4.setGeometry( refGeom );
  result = snapper.snapFeature( f4 );
  QCOMPARE( result.asWkt(), u"LineString (0 0, 5 5, 10 10, 15 15)"_s );

  const QgsGeometryMap res = snapper.snappedGeometries();
  QCOMPARE( res.count(), 4 );
  QCOMPARE( res.value( 1 ).asWkt(), f1.geometry().asWkt() );
  QCOMPARE( res.value( 2 ).asWkt(), u"LineString (5 5, 10 10, 15 15)"_s );
  QCOMPARE( res.value( 3 ).asWkt(), f3.geometry().asWkt() );
  QCOMPARE( res.value( 4 ).asWkt(), u"LineString (0 0, 5 5, 10 10, 15 15)"_s );
}

void TestQgsGeometrySnapper::insertExtra()
{
  // test extra node insertion behavior
  QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 0.1 0, 0.2 0, 9.8 0, 9.9 0, 10 0, 10.1 0, 10.2 0, 20 0)"_s );
  QgsFeature f1( 1 );
  f1.setGeometry( refGeom );

  // inserting extra nodes
  QgsInternalGeometrySnapper snapper( 2, QgsGeometrySnapper::PreferNodes );
  QgsGeometry result = snapper.snapFeature( f1 );
  QCOMPARE( result.asWkt(), f1.geometry().asWkt() );

  refGeom = QgsGeometry::fromWkt( u"LineString(8 -5, 9 0, 10 5)"_s );
  QgsFeature f2( 2 );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (8 -5, 9.8 0, 9.9 0, 10 0, 10.1 0, 10 5)"_s );

  // reset snapper
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferNodes );
  result = snapper.snapFeature( f1 );

  refGeom = QgsGeometry::fromWkt( u"LineString(7 -2, 10 0)"_s );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  // should 'follow' line for a bit
  QCOMPARE( result.asWkt( 1 ), u"LineString (7 -2, 9.8 0, 9.9 0, 10 0)"_s );

  // using PreferNodesNoExtraVertices mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferNodesNoExtraVertices );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( u"LineString(8 -5, 9 0.1, 10 5)"_s );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (8 -5, 9.8 0, 10 5)"_s );

  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferNodesNoExtraVertices );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( u"LineString(7 -2, 10.1 0.1)"_s );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (7 -2, 10.1 0)"_s );

  // using PreferClosestNoExtraVertices mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferClosestNoExtraVertices );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( u"LineString(8 -5, 9 0.1, 10 5)"_s );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (8 -5, 9 0, 10 5)"_s );

  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferClosestNoExtraVertices );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( u"LineString(7 -2, 10.1 0.1)"_s );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (7 -2, 10.1 0)"_s );

  // using EndPointPreferNodes mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::EndPointPreferNodes );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( u"LineString(7 -2, 10.02 0)"_s );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (7 -2, 10 0)"_s );

  // using EndPointPreferClosest mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::EndPointPreferClosest );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( u"LineString(7 -2, 10.02 0)"_s );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (7 -2, 10 0)"_s );

  // using EndPointToEndPoint mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::EndPointToEndPoint );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( u"LineString(-7 -2, 0.12 0)"_s );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (-7 -2, 0 0)"_s );
}

void TestQgsGeometrySnapper::duplicateNodes()
{
  // test that snapper does not result in duplicate nodes

  QgsGeometry refGeom = QgsGeometry::fromWkt( u"LineString(0 0, 20 0)"_s );
  QgsFeature f1( 1 );
  f1.setGeometry( refGeom );

  QgsInternalGeometrySnapper snapper( 2, QgsGeometrySnapper::PreferNodes );
  QgsGeometry result = snapper.snapFeature( f1 );
  QCOMPARE( result.asWkt(), f1.geometry().asWkt() );

  refGeom = QgsGeometry::fromWkt( u"LineString(10 10, 19 0, 19.5 1, 20 0.1)"_s );
  QgsFeature f2( 2 );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (10 10, 20 0)"_s );

  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferNodesNoExtraVertices );
  result = snapper.snapFeature( f1 );
  QCOMPARE( result.asWkt(), f1.geometry().asWkt() );

  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), u"LineString (10 10, 20 0)"_s );
}

void TestQgsGeometrySnapper::snapMultiPolygonToPolygon()
{
  auto rl = std::make_unique<QgsVectorLayer>( u"Polygon"_s, u"x"_s, u"memory"_s );
  QgsFeature ff( 0 );
  const QgsGeometry refGeom = QgsGeometry::fromWkt( u"Polygon((0 0, 10 0, 10 10, 0 10, 0 0))"_s );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  // test MultiPolygon that could be removed in the process https://github.com/qgis/QGIS/issues/26385
  const QgsGeometry polygonGeom = QgsGeometry::fromWkt( u"MultiPolygon(((0.1 -0.1, 5 0.1, 9.9 0.1, 0.1 -0.1)))"_s );
  const QgsGeometrySnapper snapper( rl.get() );
  const QgsGeometry result = snapper.snapGeometry( polygonGeom, 1 );
  QCOMPARE( result.asWkt(), u"MultiPolygon (((0 0, 5 0, 10 0, 0 0)))"_s );
}


QGSTEST_MAIN( TestQgsGeometrySnapper )
#include "testqgsgeometrysnapper.moc"
