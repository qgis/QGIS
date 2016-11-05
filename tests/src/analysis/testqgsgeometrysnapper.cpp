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
#include <QtTest/QtTest>

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
    void snapPointToPolygon();
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
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature ff( 0 );
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 10 0, 10 10, 0 10, 0 0))" ) );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry polygonGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1))" ) );
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( polygonGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );

  QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom2 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 0 10, 0 0))" ) );

  // insert new vertex
  QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom3 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0))" ) );

  // remove vertex
  QgsGeometry polygonGeom4 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom4 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );
}

void TestQgsGeometrySnapper::snapLineToLine()
{
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature ff( 0 );

  // closed linestrings
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 10 0, 10 10, 0 10, 0 0)" ) );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry lineGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( lineGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10, 0 0)" ) );

  QgsGeometry lineGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom2 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10, 0 0)" ) );

  // insert new vertex
  QgsGeometry lineGeom3 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom3 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0)" ) );

  // remove vertex
  QgsGeometry lineGeom4 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom4 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10, 0 0)" ) );


  // unclosed linestrings
  QgsGeometry lineGeom5 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom5 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );

  QgsGeometry lineGeom6 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom6 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10)" ) );

  // insert new vertex
  QgsGeometry lineGeom7 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9)" ) );
  result = snapper.snapGeometry( lineGeom7 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10)" ) );

  // remove vertex
  QgsGeometry lineGeom8 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom8 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );
}

void TestQgsGeometrySnapper::snapLineToPolygon()
{
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsFeature ff( 0 );

  // closed linestrings
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 10 0, 10 10, 0 10, 0 0))" ) );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry lineGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10, 0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( lineGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10, 0 0)" ) );

  QgsGeometry lineGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom2 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10, 0 0)" ) );

  // insert new vertex
  QgsGeometry lineGeom3 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom3 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0)" ) );

  // remove vertex
  QgsGeometry lineGeom4 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1)" ) );
  result = snapper.snapGeometry( lineGeom4 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10, 0 0)" ) );


  // unclosed linestrings
  QgsGeometry lineGeom5 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom5 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );

  QgsGeometry lineGeom6 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom6 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10)" ) );

  // insert new vertex
  QgsGeometry lineGeom7 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.5 0.5, 20 10, 0 9.9)" ) );
  result = snapper.snapGeometry( lineGeom7 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10)" ) );

  // remove vertex
  QgsGeometry lineGeom8 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom8 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );
}

void TestQgsGeometrySnapper::snapLineToPoint()
{
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

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
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( lineGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 10 10, 0 10)" ) );

  QgsGeometry lineGeom2 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 10.1 0, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom2 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 0 10)" ) );

  // insert new vertex
  QgsGeometry lineGeom3 = QgsGeometry::fromWkt( QStringLiteral( "LineString(0.1 -0.1, 20.0 0.0, 20 10, 0 10)" ) );
  result = snapper.snapGeometry( lineGeom3 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "LineString (0 0, 10 0, 20 0, 20 10, 0 10)" ) );
}

void TestQgsGeometrySnapper::snapPolygonToLine()
{
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

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
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( polygonGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );

  QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom2 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 0 10, 0 0))" ) );

  // insert new vertex
  QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 20.5 0.5, 20 10, 0 9.9, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom3 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 20.5 0.5, 20 10, 10 10, 0 10, 0 0))" ) );

  // remove vertex
  QgsGeometry polygonGeom4 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 9.9 10.1, 5 10, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom4 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );


  // snapping to unclosed linestring
  QgsGeometry polygonGeom5 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((100.1 -0.1, 110.1 0, 109.9 10.1, 100 10, 100.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom5 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((100 0, 110 0, 110 10, 100 10, 100 0))" ) );

  QgsGeometry polygonGeom6 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((100.1 -0.1, 110.1 0, 100 10, 100.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom6 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((100 0, 110 0, 100 10, 100 0))" ) );

  // insert new vertex
  QgsGeometry polygonGeom7 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((100.1 -0.1, 120.5 0.5, 120 10, 100 9.9, 100.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom7 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((100 0, 110 0, 120.5 0.5, 120 10, 110 10, 100 10, 100 0))" ) );

  // remove vertex
  QgsGeometry polygonGeom8 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((100.1 -0.1, 110.1 0, 109.9 10.1, 105 10, 100 10, 100.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom8 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((100 0, 110 0, 110 10, 100 10, 100 0))" ) );
}

void TestQgsGeometrySnapper::snapPolygonToPoint()
{
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

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
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( polygonGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" ) );

  QgsGeometry polygonGeom2 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 10.1 0, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom2 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 0 10, 0 0))" ) );

  // insert new vertex
  QgsGeometry polygonGeom3 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0.1 -0.1, 20.0 0.0, 20 10, 0 10, 0.1 -0.1))" ) );
  result = snapper.snapGeometry( polygonGeom3 );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Polygon ((0 0, 10 0, 20 0, 20 10, 0 10, 0 0))" ) );
}

void TestQgsGeometrySnapper::snapPointToPoint()
{
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

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
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( pointGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Point (0 0)" ) );

  pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.6 -0.1)" ) );
  result = snapper.snapGeometry( pointGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Point (1 0)" ) );
}

void TestQgsGeometrySnapper::snapPointToLine()
{
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Linestring" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  // closed linestring
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 10 0, 10 10, 0 10, 0 0)" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( pointGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Point (0 0)" ) );

  pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(10.6 -0.1)" ) );
  result = snapper.snapGeometry( pointGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Point (10 0)" ) );
}

void TestQgsGeometrySnapper::snapPointToPolygon()
{
  QgsVectorLayer* rl = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );

  // closed linestring
  QgsGeometry refGeom = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 10 0, 10 10, 0 10, 0 0))" ) );
  QgsFeature ff( 0 );
  ff.setGeometry( refGeom );
  QgsFeatureList flist;
  flist << ff;
  rl->dataProvider()->addFeatures( flist );

  QgsGeometry pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(0.1 -0.1)" ) );
  QgsGeometrySnapper snapper( rl, 1 );
  QgsGeometry result = snapper.snapGeometry( pointGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Point (0 0)" ) );

  pointGeom = QgsGeometry::fromWkt( QStringLiteral( "Point(10.6 -0.1)" ) );
  result = snapper.snapGeometry( pointGeom );
  QCOMPARE( result.exportToWkt(), QStringLiteral( "Point (10 0)" ) );
}


QTEST_MAIN( TestQgsGeometrySnapper )
#include "testqgsgeometrysnapper.moc"
