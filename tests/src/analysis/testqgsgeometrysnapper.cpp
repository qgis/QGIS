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


class TestQgsGeometrySnapper : public QObject
{
    Q_OBJECT

  public:


  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction.
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
};

void  TestQgsGeometrySnapper::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}
void  TestQgsGeometrySnapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}
void  TestQgsGeometrySnapper::init()
{

}
void  TestQgsGeometrySnapper::cleanup()
{

}

void TestQgsGeometrySnapper::snapPolygonToPolygon()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature ff( 0 );
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 10 0, 10 10, 0 10, 0 0))" ) );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry polygonGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1))" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( polygonGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );

  QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom2, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 0 10, 0 0))" ) );

  // insert new vertex
  QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom3, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0))" ) );

  // remove vertex
  QgsGeometry polygonGeom4 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom4, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );
}

void TestQgsGeometrySnapper::snapLineToLine()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature ff( 0 );

  // closed linestrings
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 10 0, 10 10, 0 10, 0 0)" ) );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry lineGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10, 0 0)" ) );

  QgsGeometry lineGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom2, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10, 0 0)" ) );

  // insert new vertex
  QgsGeometry lineGeom3 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom3, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0)" ) );

  // remove vertex
  QgsGeometry lineGeom4 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom4, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10, 0 0)" ) );


  // unclosed linestrings
  QgsGeometry lineGeom5 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom5, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );

  QgsGeometry lineGeom6 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom6, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10)" ) );

  // insert new vertex
  QgsGeometry lineGeom7 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9)" ) );
  result = snapper.snapGeometry( lineGeom7, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10)" ) );

  // remove vertex
  QgsGeometry lineGeom8 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom8, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );
}

void TestQgsGeometrySnapper::snapLineToPolygon()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature ff( 0 );

  // closed linestrings
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 10 0, 10 10, 0 10, 0 0))" ) );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry lineGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10, 0 0)" ) );

  QgsGeometry lineGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom2, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10, 0 0)" ) );

  // insert new vertex
  QgsGeometry lineGeom3 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom3, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0)" ) );

  // remove vertex
  QgsGeometry lineGeom4 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom4, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10, 0 0)" ) );


  // unclosed linestrings
  QgsGeometry lineGeom5 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom5, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );

  QgsGeometry lineGeom6 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom6, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10)" ) );

  // insert new vertex
  QgsGeometry lineGeom7 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9)" ) );
  result = snapper.snapGeometry( lineGeom7, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10)" ) );

  // remove vertex
  QgsGeometry lineGeom8 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom8, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );
}

void TestQgsGeometrySnapper::snapLineToPoint()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0 0)" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsGeometry refGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Point(10 0)" ) );
  QgsFeature ff2( 2 );
  ff2.setGeometry( refGeom2 );
  QgsFeatureList flist;
  flist << ff << ff2;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry lineGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 10 10, 0 10)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );

  QgsGeometry lineGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom2, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10)" ) );

  // insert new vertex
  QgsGeometry lineGeom3 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.0 0.0, 20 10, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom3, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 10 0, 20 0, 20 10, 0 10)" ) );
}

void TestQgsGeometrySnapper::snapPolygonToLine()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  // closed linestring
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 10 0, 10 10, 0 10, 0 0)" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  // unclosed linestring
  QgsGeometry refGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(100 0, 110 0, 110 10, 100 10)" ) );
  QgsFeature ff2( 2 );
  ff2.setGeometry( refGeom2 );
  QgsFeatureList flist;
  flist << ff << ff2;
  rl->dataProvider()->addFeatures( flist );


  // snapping to closed linestring
  QgsGeometry polygonGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1))" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( polygonGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );

  QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom2, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 0 10, 0 0))" ) );

  // insert new vertex
  QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom3, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0))" ) );

  // remove vertex
  QgsGeometry polygonGeom4 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom4, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );


  // snapping to unclosed linestring
  QgsGeometry polygonGeom5 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((100.1 -0.1, 110.1 0, 109.9 10.1, 100 10, 100.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom5, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((100 0, 110 0, 110 10, 100 10, 100 0))" ) );

  QgsGeometry polygonGeom6 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((100.1 -0.1, 110.1 0, 100 10, 100.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom6, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((100 0, 110 0, 100 10, 100 0))" ) );

  // insert new vertex
  QgsGeometry polygonGeom7 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((100.1 -0.1, 120.5 0.5, 120 10, 100 9.9, 100.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom7, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((100 0, 110 0, 120.5 0.5, 120 10, 110 10, 100 10, 100 0))" ) );

  // remove vertex
  QgsGeometry polygonGeom8 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((100.1 -0.1, 110.1 0, 109.9 10.1, 105 10, 100 10, 100.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom8, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((100 0, 110 0, 110 10, 100 10, 100 0))" ) );
}

void TestQgsGeometrySnapper::snapPolygonToPoint()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0 0)" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsGeometry refGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Point(10 0)" ) );
  QgsFeature ff2( 2 );
  ff2.setGeometry( refGeom2 );
  QgsFeatureList flist;
  flist << ff << ff2;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry polygonGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 10 10, 0 10, 0.1 -0.1))" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( polygonGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );

  QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom2, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 0 10, 0 0))" ) );

  // insert new vertex
  QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 20.0 0.0, 20 10, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom3, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 20 0, 20 10, 0 10, 0 0))" ) );
}

void TestQgsGeometrySnapper::snapPointToPoint()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0 0)" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsGeometry refGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Point(1 0)" ) );
  QgsFeature ff2( 2 );
  ff2.setGeometry( refGeom2 );
  QgsFeatureList flist;
  flist << ff << ff2;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Point (0 0)" ) );

  pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.6 -0.1)" ) );
  result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Point (1 0)" ) );
}

void TestQgsGeometrySnapper::snapPointToLine()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  // closed linestring
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 10 0, 10 10, 0 10, 0 0)" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Point (0 0)" ) );

  pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(10.6 -0.1)" ) );
  result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Point (10 0)" ) );

  pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.5 0.5)" ) );
  result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Point (0 0)" ) );
}

void TestQgsGeometrySnapper::snapPointToLinePreferNearest()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  // closed linestring
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 10 0, 10 10, 0 10, 0 0)" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.5 0.5)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( pointGeom, 1, QgsGeometrySnapper::PreferClosest );
  QCOMPARE( result.asWkt(), QStringLiteral( "Point (0.5 0)" ) );
}

void TestQgsGeometrySnapper::snapPointToPolygon()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  // closed linestring
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 10 0, 10 10, 0 10, 0 0))" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Point (0 0)" ) );

  pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(10.6 -0.1)" ) );
  result = snapper.snapGeometry( pointGeom, 1 );
  QCOMPARE( result.asWkt(), QStringLiteral( "Point (10 0)" ) );
}

void TestQgsGeometrySnapper::endPointSnap()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature ff( 0 );

  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 100 0, 100 100, 0 100)" ) );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry lineGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 -1, 102 0, 98 102, 0 101)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 10, QgsGeometrySnapper::EndPointPreferNodes );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 102 0, 98 102, 0 100)" ) );

  QgsGeometry lineGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(50 0, 102 0, 98 102, 0 50)" ) );
  result = snapper.snapGeometry( lineGeom2, 1, QgsGeometrySnapper::EndPointPreferNodes );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (50 0, 102 0, 98 102, 0 50)" ) );

  QgsGeometry lineGeom3 = QgsGeometry::fromWkt( QStringLiteral( "LineString(50 -10, 50 -1)" ) );
  result = snapper.snapGeometry( lineGeom3, 2, QgsGeometrySnapper::EndPointPreferNodes );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (50 -10, 50 0)" ) );
}

void TestQgsGeometrySnapper::endPointToEndPoint()
{
  QgsVectorLayer *rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature ff( 0 );

  // closed linestrings
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 100 0, 100 100, 0 100)" ) );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry lineGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 -1, 102 0, 98 102, 0 101)" ) );
  QgsGeometrySnapper snapper( rl );
  QgsGeometry result = snapper.snapGeometry( lineGeom, 10, QgsGeometrySnapper::EndPointToEndPoint );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 102 0, 98 102, 0 100)" ) );

  QgsGeometry lineGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(50 0, 102 0, 98 102)" ) );
  result = snapper.snapGeometry( lineGeom2, 1, QgsGeometrySnapper::EndPointToEndPoint );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (50 0, 102 0, 98 102)" ) );

  QgsGeometry lineGeom3 = QgsGeometry::fromWkt( QStringLiteral( "LineString(50 -10, 50 -1)" ) );
  result = snapper.snapGeometry( lineGeom3, 2, QgsGeometrySnapper::EndPointToEndPoint );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (50 -10, 50 -1)" ) );
}

void TestQgsGeometrySnapper::internalSnapper()
{
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 10 0, 10 10)" ) );
  QgsFeature f1( 1 );
  f1.setGeometry( refGeom );

  QgsInternalGeometrySnapper snapper( 2 );
  QgsGeometry result = snapper.snapFeature( f1 );
  QCOMPARE( result.asWkt(), f1.geometry().asWkt() );

  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(5 5, 10 11, 15 15)" ) );
  QgsFeature f2( 2 );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (5 5, 10 10, 15 15)" ) );

  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(20 20, 30 20)" ) );
  QgsFeature f3( 3 );
  f3.setGeometry( refGeom );
  result = snapper.snapFeature( f3 );
  QCOMPARE( result.asWkt(), f3.geometry().asWkt() );

  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 -1, 5.5 5, 9.8 10, 14.5 14.8)" ) );
  QgsFeature f4( 4 );
  f4.setGeometry( refGeom );
  result = snapper.snapFeature( f4 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineString (0 0, 5 5, 10 10, 15 15)" ) );

  QgsGeometryMap res = snapper.snappedGeometries();
  QCOMPARE( res.count(), 4 );
  QCOMPARE( res.value( 1 ).asWkt(), f1.geometry().asWkt() );
  QCOMPARE( res.value( 2 ).asWkt(), QStringLiteral( "LineString (5 5, 10 10, 15 15)" ) );
  QCOMPARE( res.value( 3 ).asWkt(), f3.geometry().asWkt() );
  QCOMPARE( res.value( 4 ).asWkt(), QStringLiteral( "LineString (0 0, 5 5, 10 10, 15 15)" ) );
}

void TestQgsGeometrySnapper::insertExtra()
{
  // test extra node insertion behavior
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 0.1 0, 0.2 0, 9.8 0, 9.9 0, 10 0, 10.1 0, 10.2 0, 20 0)" ) );
  QgsFeature f1( 1 );
  f1.setGeometry( refGeom );

  // inserting extra nodes
  QgsInternalGeometrySnapper snapper( 2, QgsGeometrySnapper::PreferNodes );
  QgsGeometry result = snapper.snapFeature( f1 );
  QCOMPARE( result.asWkt(), f1.geometry().asWkt() );

  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(8 -5, 9 0, 10 5)" ) );
  QgsFeature f2( 2 );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (8 -5, 9.8 0, 9.9 0, 10 0, 10.1 0, 10 5)" ) );

  // reset snapper
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferNodes );
  result = snapper.snapFeature( f1 );

  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(7 -2, 10 0)" ) );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  // should 'follow' line for a bit
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (7 -2, 9.8 0, 9.9 0, 10 0)" ) );

  // using PreferNodesNoExtraVertices mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferNodesNoExtraVertices );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(8 -5, 9 0.1, 10 5)" ) );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (8 -5, 9.8 0, 10 5)" ) );

  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferNodesNoExtraVertices );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(7 -2, 10.1 0.1)" ) );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (7 -2, 10.1 0)" ) );

  // using PreferClosestNoExtraVertices mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferClosestNoExtraVertices );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(8 -5, 9 0.1, 10 5)" ) );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (8 -5, 9 0, 10 5)" ) );

  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferClosestNoExtraVertices );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(7 -2, 10.1 0.1)" ) );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (7 -2, 10.1 0)" ) );

  // using EndPointPreferNodes mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::EndPointPreferNodes );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(7 -2, 10.02 0)" ) );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (7 -2, 10 0)" ) );

  // using EndPointPreferClosest mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::EndPointPreferClosest );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(7 -2, 10.02 0)" ) );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (7 -2, 10 0)" ) );

  // using EndPointToEndPoint mode, no extra vertices should be inserted
  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::EndPointToEndPoint );
  result = snapper.snapFeature( f1 );
  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(-7 -2, 0.12 0)" ) );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (-7 -2, 0 0)" ) );

}

void TestQgsGeometrySnapper::duplicateNodes()
{
  // test that snapper does not result in duplicate nodes

  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 20 0)" ) );
  QgsFeature f1( 1 );
  f1.setGeometry( refGeom );

  QgsInternalGeometrySnapper snapper( 2, QgsGeometrySnapper::PreferNodes );
  QgsGeometry result = snapper.snapFeature( f1 );
  QCOMPARE( result.asWkt(), f1.geometry().asWkt() );

  refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(10 10, 19 0, 19.5 1, 20 0.1)" ) );
  QgsFeature f2( 2 );
  f2.setGeometry( refGeom );
  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (10 10, 20 0)" ) );

  snapper = QgsInternalGeometrySnapper( 2, QgsGeometrySnapper::PreferNodesNoExtraVertices );
  result = snapper.snapFeature( f1 );
  QCOMPARE( result.asWkt(), f1.geometry().asWkt() );

  result = snapper.snapFeature( f2 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "LineString (10 10, 20 0)" ) );

}


QGSTEST_MAIN( TestQgsGeometrySnapper )
#include "testqgsgeometrysnapper.moc"
