/***************************************************************************
     testqgscompoundcurve.cpp
     --------------------------------------
    Date                 : August 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
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

#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgscoordinatetransform.h"
#include "testgeometryutils.h"
#include "testtransformer.h"

class TestQgsCompoundCurve: public QObject
{
    Q_OBJECT
  private slots:
    void constructor();
    void addCurve();
    void addCurveWithZM();
    void addCurveWithMissingDimInCompoundCurve();
    void addCurveWithMissingDimInAddedCurve();
    void addCurveExtend();
    void removeCurve();
    void assignment();
    void clone();
    void gettersSetters();
    void clear();
    void equality();
    void equalityZM();
    void pointAt();
    void insertVertex();
    void insertVertexZM();
    void addVertex();
    void nextVertex();
    void nextVertexZM();
    void vertexAtPointAt();
    void vertexAtPointAtZM();
    void moveVertex();
    void deleteVertex();
    void filterVertices();
    void removeDuplicateNodes();
    void addZValue();
    void addMValue();
    void dropZValue();
    void dropMValue();
    void isRing();
    void startPointEndPoint();
    void orientation();
    void length();
    void centroid();
    void closestSegment();
    void sumUpArea();
    void segmentLength();
    void angle();
    void boundary();
    void boundingBox();
    void interpolate();
    void swapXy();
    void reversed();
    void isClosed();
    void close();
    void transformVertices();
    void transformWithClass();
    void crsTransform();
    void crs3dTransformAndReverse();
    void QTransformation();
    void coordinateSequence();
    void points();
    void segmentize();
    void substring();
    void convertTo();
    void curveToLine();
    void toCurveType();
    void asQPolygonF();
    void toFromWKB();
    void toFromWKT();
    void exportImport();
    void addToPainterPath();
    void compoundCurveCondense_data();
    void compoundCurveCondense();
};

void TestQgsCompoundCurve::constructor()
{
  QgsCompoundCurve cc;

  QVERIFY( cc.isEmpty() );
  QCOMPARE( cc.numPoints(), 0 );
  QCOMPARE( cc.vertexCount(), 0 );
  QCOMPARE( cc.nCoordinates(), 0 );
  QCOMPARE( cc.ringCount(), 0 );
  QCOMPARE( cc.partCount(), 0 );
  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( cc.wktTypeStr(), QString( "CompoundCurve" ) );
  QCOMPARE( cc.geometryType(), QString( "CompoundCurve" ) );
  QCOMPARE( cc.dimension(), 1 );
  QVERIFY( !cc.hasCurvedSegments() );
  QCOMPARE( cc.area(), 0.0 );
  QCOMPARE( cc.perimeter(), 0.0 );

  QgsPointSequence pts;
  cc.points( pts );
  QVERIFY( pts.empty() );

  // empty, test some methods to make sure they don't crash
  QCOMPARE( cc.nCurves(), 0 );
  QVERIFY( !cc.curveAt( -1 ) );
  QVERIFY( !cc.curveAt( 0 ) );
  QVERIFY( !cc.curveAt( 100 ) );

  cc.removeCurve( -1 );
  cc.removeCurve( 0 );
  cc.removeCurve( 100 );
}

void TestQgsCompoundCurve::addCurve()
{
  QgsCompoundCurve cc;

  //try to add null curve
  cc.addCurve( nullptr );
  QCOMPARE( cc.nCurves(), 0 );
  QVERIFY( !cc.curveAt( 0 ) );

  QgsCircularString cs1;
  cs1.setPoints( QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );
  cc.addCurve( cs1.clone() );

  QVERIFY( !cc.isEmpty() );
  QCOMPARE( cc.numPoints(), 1 );
  QCOMPARE( cc.vertexCount(), 1 );
  QCOMPARE( cc.nCoordinates(), 1 );
  QCOMPARE( cc.ringCount(), 1 );
  QCOMPARE( cc.partCount(), 1 );
  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( cc.hasCurvedSegments() );
  QCOMPARE( cc.area(), 0.0 );
  QCOMPARE( cc.perimeter(), 0.0 );

  QgsPointSequence pts;
  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );

  cc.clear();
  cs1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  cc.addCurve( cs1.clone() );

  QVERIFY( !cc.isEmpty() );
  QCOMPARE( cc.numPoints(), 3 );
  QCOMPARE( cc.vertexCount(), 3 );
  QCOMPARE( cc.nCoordinates(), 3 );
  QCOMPARE( cc.ringCount(), 1 );
  QCOMPARE( cc.partCount(), 1 );
  QCOMPARE( cc.nCurves(), 1 );
  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( cc.hasCurvedSegments() );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 )
            << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( cc.curveAt( 0 ) ), cs1 );
  QVERIFY( !cc.curveAt( -1 ) );
  QVERIFY( !cc.curveAt( 1 ) );

  QgsCircularString cs2;
  cs2.setPoints( QgsPointSequence() << QgsPoint( 3, 4 )
                 << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 ) );
  cc.addCurve( cs2.clone() );

  QCOMPARE( cc.numPoints(), 5 );
  QCOMPARE( cc.vertexCount(), 5 );
  QCOMPARE( cc.nCoordinates(), 5 );
  QCOMPARE( cc.ringCount(), 1 );
  QCOMPARE( cc.partCount(), 1 );
  QCOMPARE( cc.nCurves(), 2 );

  pts.clear();
  cc.points( pts );

  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 )
            << QgsPoint( 3, 4 ) << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( cc.curveAt( 0 ) ), cs1 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( cc.curveAt( 1 ) ), cs2 );
  QVERIFY( !cc.curveAt( -1 ) );
  QVERIFY( !cc.curveAt( 2 ) );

  QgsLineString cs3;
  cs3.setPoints( QgsPointSequence() << QgsPoint( 3, 6 ) << QgsPoint( 4, 6 ) );
  cc.addCurve( cs3.clone() );

  QCOMPARE( cc.numPoints(), 6 );
  QCOMPARE( cc.vertexCount(), 6 );
  QCOMPARE( cc.nCoordinates(), 6 );
  QCOMPARE( cc.ringCount(), 1 );
  QCOMPARE( cc.partCount(), 1 );
  QCOMPARE( cc.nCurves(), 3 );

  pts.clear();
  cc.points( pts );

  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 )
            << QgsPoint( 3, 4 ) << QgsPoint( 4, 5 )
            << QgsPoint( 3, 6 ) << QgsPoint( 4, 6 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( cc.curveAt( 0 ) ), cs1 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( cc.curveAt( 1 ) ), cs2 );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( cc.curveAt( 2 ) ), cs3 );
  QVERIFY( !cc.curveAt( -1 ) );
  QVERIFY( !cc.curveAt( 3 ) );
}

void TestQgsCompoundCurve::addCurveWithZM()
{
  QgsCompoundCurve cc;

  //adding first curve should set linestring z/m type
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.isEmpty() );
  QVERIFY( cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QCOMPARE( cc.wktTypeStr(), QString( "CompoundCurveZ" ) );

  QgsPointSequence pts;
  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.isEmpty() );
  QVERIFY( !cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );
  QCOMPARE( cc.wktTypeStr(), QString( "CompoundCurveM" ) );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.isEmpty() );
  QVERIFY( cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QCOMPARE( cc.wktTypeStr(), QString( "CompoundCurveZM" ) );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  cc.clear();

  //addCurve with z
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.numPoints(), 2 );
  QVERIFY( cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  cc.points( pts );

  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
            << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //addCurve with m
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.numPoints(), 2 );
  QVERIFY( !cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
            << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //addCurve with zm
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.numPoints(), 2 );
  QVERIFY( cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
            << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
}

void TestQgsCompoundCurve::addCurveWithMissingDimInCompoundCurve()
{
  QgsCompoundCurve cc;

  //addCurve with z to non z compound curve
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 )
                << QgsPoint( QgsWkbTypes::Point, 2, 3 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZ, 3, 3, 5 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  QgsPointSequence pts;
  cc.points( pts );

  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 )
            << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );

  cc.removeCurve( 1 );

  //addCurve with m to non m compound curve
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 3, 3, 0, 5 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 )
            << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );

  cc.removeCurve( 1 );

  //addCurve with zm to non m compound curve
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 3, 3, 1, 5 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 )
            << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );

  cc.removeCurve( 1 );
}

void TestQgsCompoundCurve::addCurveWithMissingDimInAddedCurve()
{
  QgsCompoundCurve cc;

  //addCurve with no z to z compound curve
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 )
                << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2, 3 )
                << QgsPoint( QgsWkbTypes::Point, 3, 4 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  QgsPointSequence pts;
  cc.points( pts );

  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 )
            << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 0 ) );

  cc.removeCurve( 1 );

  //add curve with m, no z to z compound curve
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 8 )
                << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 9 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 )
            << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 0 ) );

  cc.removeCurve( 1 );

  //add curve with zm to z compound curve
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 8 )
                << QgsPoint( QgsWkbTypes::PointZM, 3, 4, 7, 9 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  cc.points( pts );

  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 )
            << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 7 ) );

  cc.removeCurve( 1 );

  //addCurve with no m to m compound curve
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2, 3 )
                << QgsPoint( QgsWkbTypes::Point, 3, 4 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
            << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 0 ) );

  cc.removeCurve( 1 );

  //add curve with z, no m to m compound curve
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 8 )
                << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 9 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
            << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 0 ) );

  cc.removeCurve( 1 );

  //add curve with zm to m compound curve
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 8 )
                << QgsPoint( QgsWkbTypes::PointZM, 3, 4, 7, 9 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );

  cc.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
            << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 9 ) );

  cc.removeCurve( 1 );
}

void TestQgsCompoundCurve::addCurveExtend()
{
  // add curve and extend existing
  QgsCompoundCurve cc;

  // try to extend empty compound curve
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  cc.addCurve( cs.clone(), true );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4))" ) );

  // try to add another curve with extend existing as true
  // should be ignored.
  cs.setPoints( QgsPointSequence() << QgsPoint( 6, 6 ) << QgsPoint( 7, 8 ) );
  cc.addCurve( cs.clone(), true );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8))" ) );

  // try to add a linestring with extend existing as true
  //should be ignored because the last curve isn't a linestring
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 10, 8 ) << QgsPoint( 10, 12 ) );
  cc.addCurve( ls.clone(), true );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8),(10 8, 10 12))" ) );

  // try to extend with another linestring
  //should add to final part
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 13 ) << QgsPoint( 12, 12 ) );
  cc.addCurve( ls.clone(), true );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8),(10 8, 10 12, 11 13, 12 12))" ) );

  // try to extend with another linestring
  //should add to final part, with no duplicate points
  ls.setPoints( QgsPointSequence() << QgsPoint( 12, 12 )
                << QgsPoint( 13, 12 ) << QgsPoint( 14, 15 ) );
  cc.addCurve( ls.clone(), true );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8),(10 8, 10 12, 11 13, 12 12, 13 12, 14 15))" ) );

  // not extending, should be added as new curve
  ls.setPoints( QgsPointSequence() << QgsPoint( 15, 16 ) << QgsPoint( 17, 12 ) );
  cc.addCurve( ls.clone(), false );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8),(10 8, 10 12, 11 13, 12 12, 13 12, 14 15),(15 16, 17 12))" ) );

  cc.clear();

  // adding a linestring as first part, with extend as true
  cc.addCurve( ls.clone(), true );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve ((15 16, 17 12))" ) );
}

void TestQgsCompoundCurve::removeCurve()
{
  QgsCompoundCurve cc;

  QgsCircularString cs1;
  cs1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  cc.addCurve( cs1.clone() );

  QgsCircularString cs2;
  cs2.setPoints( QgsPointSequence() << QgsPoint( 3, 4 )
                 << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 ) );
  cc.addCurve( cs2.clone() );

  QgsLineString cs3;
  cs3.setPoints( QgsPointSequence() << QgsPoint( 3, 6 ) << QgsPoint( 4, 6 ) );
  cc.addCurve( cs3.clone() );

  cc.removeCurve( -1 );
  cc.removeCurve( 3 );

  QCOMPARE( cc.nCurves(), 3 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( cc.curveAt( 0 ) ), cs1 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( cc.curveAt( 1 ) ), cs2 );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( cc.curveAt( 2 ) ), cs3 );

  cc.removeCurve( 1 );

  QCOMPARE( cc.nCurves(), 2 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( cc.curveAt( 0 ) ), cs1 );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( cc.curveAt( 1 ) ), cs3 );

  cc.removeCurve( 0 );

  QCOMPARE( cc.nCurves(), 1 );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( cc.curveAt( 0 ) ), cs3 );

  cc.removeCurve( 0 );

  QCOMPARE( cc.nCurves(), 0 );
  QVERIFY( cc.isEmpty() );
}

void TestQgsCompoundCurve::assignment()
{
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  QgsCompoundCurve cc1;
  cc1.addCurve( ls.clone() );

  QgsCompoundCurve cc2;

  QVERIFY( cc1 != cc2 );

  cc2 = cc1;
  QCOMPARE( cc1, cc2 );
}

void TestQgsCompoundCurve::clone()
{
  QgsCompoundCurve cc;

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 2 )
                << QgsPoint( 11, 22 ) << QgsPoint( 1, 22 ) );
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 23, 22 ) );
  cc.addCurve( ls.clone() );
  std::unique_ptr<QgsCompoundCurve> cloned( cc.clone() );

  QCOMPARE( *cloned, cc );

  //clone with Z/M
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  cc.addCurve( cs.clone() );
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 22, 31, 34 ) << QgsPoint( 23, 22, 42, 43 ) );
  cc.addCurve( ls.clone() );

  cloned.reset( cc.clone() );

  QCOMPARE( *cloned, cc );

  //clone an empty line
  cc.clear();
  cloned.reset( cc.clone() );

  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CompoundCurve );
}

void TestQgsCompoundCurve::gettersSetters()
{
  QgsCompoundCurve cc;

  // no crash!
  ( void )cc.xAt( -1 );
  ( void )cc.xAt( 1 );
  ( void )cc.yAt( -1 );
  ( void )cc.yAt( 1 );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.xAt( 0 ), 1.0 );
  QCOMPARE( cc.xAt( 1 ), 11.0 );
  QCOMPARE( cc.xAt( 2 ), 21.0 );
  ( void ) cc.xAt( -1 ); //out of range
  ( void ) cc.xAt( 11 ); //out of range
  QCOMPARE( cc.yAt( 0 ), 2.0 );
  QCOMPARE( cc.yAt( 1 ), 12.0 );
  QCOMPARE( cc.yAt( 2 ), 22.0 );
  ( void ) cc.yAt( -1 ); //out of range
  ( void ) cc.yAt( 11 ); //out of range

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 31, 22, 13, 14 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.xAt( 0 ), 1.0 );
  QCOMPARE( cc.xAt( 1 ), 11.0 );
  QCOMPARE( cc.xAt( 2 ), 21.0 );
  QCOMPARE( cc.xAt( 3 ), 31.0 );
  QCOMPARE( cc.xAt( 4 ), 0.0 );
  ( void ) cc.xAt( -1 ); //out of range
  ( void ) cc.xAt( 11 ); //out of range
  QCOMPARE( cc.yAt( 0 ), 2.0 );
  QCOMPARE( cc.yAt( 1 ), 12.0 );
  QCOMPARE( cc.yAt( 2 ), 22.0 );
  QCOMPARE( cc.yAt( 3 ), 22.0 );
  QCOMPARE( cc.yAt( 4 ), 0.0 );
  ( void ) cc.yAt( -1 ); //out of range
  ( void ) cc.yAt( 11 ); //out of range

  cc.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 52.0 ) );
  QCOMPARE( cc.xAt( 0 ), 51.0 );
  QCOMPARE( cc.yAt( 0 ), 52.0 );

  cc.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 62 ) );
  QCOMPARE( cc.xAt( 1 ), 61.0 );
  QCOMPARE( cc.yAt( 1 ), 62.0 );

  cc.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 71.0, 2 ) ); //out of range
  cc.moveVertex( QgsVertexId( 0, 0, 11 ), QgsPoint( 71.0, 2 ) ); //out of range
}

void TestQgsCompoundCurve::clear()
{
  QgsCompoundCurve cc;

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  cc.addCurve( cs.clone() );

  cc.clear();
  QVERIFY( cc.isEmpty() );
  QCOMPARE( cc.nCurves(), 0 );
  QCOMPARE( cc.numPoints(), 0 );
  QCOMPARE( cc.vertexCount(), 0 );
  QCOMPARE( cc.nCoordinates(), 0 );
  QCOMPARE( cc.ringCount(), 0 );
  QCOMPARE( cc.partCount(), 0 );
  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
}

void TestQgsCompoundCurve::equality()
{
  QgsCompoundCurve cc1;
  QgsCompoundCurve cc2;

  QVERIFY( cc1 == cc2 );
  QVERIFY( !( cc1 != cc2 ) );

  QgsLineString ls1;
  ls1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  cc1.addCurve( ls1.clone() );

  QVERIFY( !( cc1 == cc2 ) ); //different number of curves
  QVERIFY( cc1 != cc2 );

  cc2.addCurve( ls1.clone() );

  QVERIFY( cc1 == cc2 );
  QVERIFY( !( cc1 != cc2 ) );

  ls1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  cc1.addCurve( ls1.clone() );

  QVERIFY( !( cc1 == cc2 ) ); //different number of curves
  QVERIFY( cc1 != cc2 );

  QgsLineString ls2;
  ls2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  cc2.addCurve( ls2.clone() );

  QVERIFY( cc1 == cc2 ); //check non-integer equality
  QVERIFY( !( cc1 != cc2 ) );

  ls1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 1 / 3.0, 4 / 3.0 ) << QgsPoint( 7, 8 ) );
  cc1.addCurve( ls1.clone() );
  ls2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 2 / 6.0, 8 / 6.0 ) << QgsPoint( 6, 9 ) );
  cc2.addCurve( ls2.clone() );

  QVERIFY( !( cc1 == cc2 ) ); //different coordinates
  QVERIFY( cc1 != cc2 );
}

void TestQgsCompoundCurve::equalityZM()
{
  QgsCompoundCurve cc1;
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  cc1.addCurve( ls.clone() );

  QgsCompoundCurve cc2;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );
  cc2.addCurve( ls.clone() );

  QVERIFY( !( cc1 == cc2 ) ); //different dimension
  QVERIFY( cc1 != cc2 );

  QgsCompoundCurve cc3;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );
  cc3.addCurve( ls.clone() );

  QVERIFY( !( cc2 == cc3 ) ); //different z coordinates
  QVERIFY( cc2 != cc3 );

  QgsCompoundCurve cc4;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  cc4.addCurve( ls.clone() );

  QgsCompoundCurve cc5;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );
  cc5.addCurve( ls.clone() );

  QVERIFY( !( cc4 == cc5 ) ); //different m values
  QVERIFY( cc4 != cc5 );

  QVERIFY( cc5 != QgsLineString() );
}

void TestQgsCompoundCurve::pointAt()
{
  QgsCompoundCurve cc;

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 31, 22, 13, 14 ) );
  cc.addCurve( ls.clone() );

  QgsPoint p;
  Qgis::VertexType type;
  QVERIFY( !cc.pointAt( -1, p, type ) );
  QVERIFY( !cc.pointAt( 11, p, type ) );

  QVERIFY( cc.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  QVERIFY( cc.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );

  QVERIFY( cc.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  QVERIFY( cc.pointAt( 3, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 31, 22, 13, 14 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsCompoundCurve::insertVertex()
{
  //cannot insert vertex in empty line
  QgsCompoundCurve cc;
  QVERIFY( !cc.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( cc.numPoints(), 0 );

  //2d line
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 4.0, 7.0 ) ) );

  QCOMPARE( cc.numPoints(), 5 );
  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  QgsPoint pt;
  Qgis::VertexType v;
  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 2.0 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 4.0, 7.0 ) );

  cc.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( pt.y(), 9.930870, 0.01 );

  cc.pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( 11.0, 12.0 ) );

  cc.pointAt( 4, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 22.0 ) );

  QVERIFY( cc.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( cc.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 16.0 ) ) );
  QCOMPARE( cc.numPoints(), 9 );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 2.0 ) );

  cc.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 4.363083, 0.01 );
  QGSCOMPARENEAR( pt.y(), 5.636917, 0.01 );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 8.0, 9.0 ) );

  cc.pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( 18.0, 16.0 ) );

  cc.pointAt( 4, pt, v );
  QGSCOMPARENEAR( pt.x(), 5.876894, 0.01 );
  QGSCOMPARENEAR( pt.y(), 8.246211, 0.01 );

  cc.pointAt( 5, pt, v );
  QCOMPARE( pt, QgsPoint( 4.0, 7.0 ) );

  cc.pointAt( 6, pt, v );
  QGSCOMPARENEAR( pt.x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( pt.y(), 9.930870, 0.01 );

  cc.pointAt( 7, pt, v );
  QCOMPARE( pt, QgsPoint( 11.0, 12.0 ) );

  cc.pointAt( 8, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 22.0 ) );

  //insert vertex at end
  QVERIFY( !cc.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 31.0, 32.0 ) ) );

  //insert vertex past end
  QVERIFY( !cc.insertVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( cc.numPoints(), 9 );

  //insert vertex before start
  QVERIFY( !cc.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( cc.numPoints(), 9 );
}

void TestQgsCompoundCurve::insertVertexZM()
{
  //insert 4d vertex in 4d line
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QgsCompoundCurve cc;
  cc.addCurve( cs.clone( ) );

  QVERIFY( cc.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( cc.numPoints(), 5 );

  QgsPoint pt;
  Qgis::VertexType v;
  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( cc.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );
  QCOMPARE( cc.numPoints(), 7 );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 4, 103, 104 ) ) );
  QCOMPARE( cc.numPoints(), 5 );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 2, 4 ) );

  // invalid
  QVERIFY( !cc.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 1, 2 ) ) );
}

void TestQgsCompoundCurve::addVertex()
{
  QgsCompoundCurve cc;
  cc.addVertex( QgsPoint( 1.0, 2.0 ) );

  QVERIFY( !cc.isEmpty() );
  QCOMPARE( cc.numPoints(), 1 );
  QCOMPARE( cc.vertexCount(), 1 );
  QCOMPARE( cc.nCoordinates(), 1 );
  QCOMPARE( cc.ringCount(), 1 );
  QCOMPARE( cc.partCount(), 1 );
  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( !cc.hasCurvedSegments() );
  QCOMPARE( cc.area(), 0.0 );
  QCOMPARE( cc.perimeter(), 0.0 );

  //adding first vertex should set linestring z/m type
  cc.clear();
  cc.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  QVERIFY( !cc.isEmpty() );
  QVERIFY( cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QCOMPARE( cc.wktTypeStr(), QString( "CompoundCurveZ" ) );

  cc.clear();
  cc.addVertex( QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );

  QVERIFY( !cc.isEmpty() );
  QVERIFY( !cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );
  QCOMPARE( cc.wktTypeStr(), QString( "CompoundCurveM" ) );

  cc.clear();
  cc.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  QVERIFY( !cc.isEmpty() );
  QVERIFY( cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QCOMPARE( cc.wktTypeStr(), QString( "CompoundCurveZM" ) );

  //adding subsequent vertices should not alter z/m type, regardless of points type
  cc.clear();
  cc.addVertex( QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) ); //2d type

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  cc.addVertex( QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0, 13.0 ) ); // add 3d point

  QCOMPARE( cc.numPoints(), 2 );
  QCOMPARE( cc.vertexCount(), 2 );
  QCOMPARE( cc.nCoordinates(), 2 );
  QCOMPARE( cc.ringCount(), 1 );
  QCOMPARE( cc.partCount(), 1 );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve ); //should still be 2d
  QVERIFY( !cc.is3D() );
  QCOMPARE( cc.area(), 0.0 );
  QCOMPARE( cc.perimeter(), 0.0 );

  cc.clear();
  cc.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) ); //3d type

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  cc.addVertex( QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) ); //add 2d point

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ ); //should still be 3d

  QgsPoint pt;
  Qgis::VertexType v;
  cc.pointAt( 1, pt, v );

  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0 ) );
  QVERIFY( cc.is3D() );
  QCOMPARE( cc.numPoints(), 2 );
  QCOMPARE( cc.vertexCount(), 2 );
  QCOMPARE( cc.nCoordinates(), 2 );
  QCOMPARE( cc.ringCount(), 1 );
  QCOMPARE( cc.partCount(), 1 );
}

void TestQgsCompoundCurve::nextVertex()
{
  QgsCompoundCurve cc;
  QgsPoint pt;
  QgsVertexId vId;

  QVERIFY( !cc.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, -2 );
  QVERIFY( !cc.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, 10 );
  QVERIFY( !cc.nextVertex( vId, pt ) );

  //CircularString
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  cc.addCurve( cs.clone() );

  vId = QgsVertexId( 0, 0, 2 ); //out of range
  QVERIFY( !cc.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, -5 );
  QVERIFY( cc.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );

  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( !cc.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 1, 0 );
  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 1, 1 ) ); //test that ring number is maintained
  QCOMPARE( pt, QgsPoint( 11, 12 ) );

  vId = QgsVertexId( 1, 0, 0 );
  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 11, 12 ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 13, 14 ) );
  cc.addCurve( ls.clone() );

  vId = QgsVertexId( 0, 0, 1 );
  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 13, 14 ) );
  QVERIFY( !cc.nextVertex( vId, pt ) );
}

void TestQgsCompoundCurve::nextVertexZM()
{
  QgsCompoundCurve cc;
  QgsPoint pt;

  //CircularStringZ
  cc.clear();
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  cc.addCurve( cs.clone() );

  QgsVertexId vId( 0, 0, -1 );
  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );

  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !cc.nextVertex( vId, pt ) );

  //CircularStringM
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  cc.clear();
  cc.addCurve( cs.clone() );

  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );

  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !cc.nextVertex( vId, pt ) );

  //CircularStringZM
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  cc.clear();
  cc.addCurve( cs.clone() );

  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );

  QVERIFY( cc.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !cc.nextVertex( vId, pt ) );
}

void TestQgsCompoundCurve::vertexAtPointAt()
{
  QgsCompoundCurve cc;
  QgsPoint p;
  Qgis::VertexType type;

  cc.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  cc.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash

  QVERIFY( !cc.pointAt( -10, p, type ) );
  QVERIFY( !cc.pointAt( 10, p, type ) );

  //CircularString
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 1, 22 ) );
  cc.addCurve( cs.clone() );

  cc.vertexAt( QgsVertexId( 0, 0, -10 ) );
  cc.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash

  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QVERIFY( !cc.pointAt( -10, p, type ) );
  QVERIFY( !cc.pointAt( 10, p, type ) );

  QVERIFY( cc.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  QVERIFY( cc.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );

  QVERIFY( cc.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 3, 34 ) );
  cc.addCurve( ls.clone() );

  QVERIFY( cc.pointAt( 3, p, type ) );
  QCOMPARE( p, QgsPoint( 3, 34 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsCompoundCurve::vertexAtPointAtZM()
{
  QgsCompoundCurve cc;
  QgsPoint p;
  Qgis::VertexType type;

  //CircularStringZ
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );

  QVERIFY( cc.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  QVERIFY( cc.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );

  QVERIFY( cc.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22, 23 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  //CircularStringM
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 )
                << QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );

  QVERIFY( cc.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  QVERIFY( cc.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );

  QVERIFY( cc.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  //CircularStringZM
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( cc.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );

  QVERIFY( cc.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  QVERIFY( cc.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );

  QVERIFY( cc.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsCompoundCurve::moveVertex()
{
  //empty line
  QgsCompoundCurve cc;

  QVERIFY( !cc.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( cc.isEmpty() );

  //valid line
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( cc.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( cc.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );

  QgsPoint pt;
  Qgis::VertexType v;
  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 6.0, 7.0 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 16.0, 17.0 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 26.0, 27.0 ) );

  //out of range
  QVERIFY( !cc.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !cc.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !cc.moveVertex( QgsVertexId( 0, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 6.0, 7.0 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 16.0, 17.0 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 26.0, 27.0 ) );

  //move 4d point in 4d line
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  cc.clear();
  cc.addCurve( cs.clone() );

  QVERIFY( cc.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) );

  //move 2d point in 4d line, existing z/m should be maintained
  QVERIFY( cc.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 34, 35 ) ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 34, 35, 12, 13 ) );

  //move 4d point in 2d line
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  cc.clear();
  cc.addCurve( cs.clone() );

  QVERIFY( cc.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 3, 4, 2, 3 ) ) );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 3, 4 ) );
}

void TestQgsCompoundCurve::deleteVertex()
{
  //empty line
  QgsCompoundCurve cc;
  QVERIFY( !cc.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( cc.isEmpty() );

  //valid line
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );
  cc.addCurve( cs.clone() );

  //out of range vertices
  QVERIFY( !cc.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !cc.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( cc.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( cc.numPoints(), 2 );

  QgsPoint pt;
  Qgis::VertexType v;
  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );

  //removing the next vertex removes all remaining vertices
  QVERIFY( cc.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( cc.numPoints(), 0 );
  QVERIFY( cc.isEmpty() );

  //removing a vertex from a 3 point comound curveshould remove the whole line - Duplicated ?
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.numPoints(), 3 );

  cc.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( cc.numPoints(), 0 );

  // two lines
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  cc.clear();
  cc.addCurve( ls.clone() );

  ls.setPoints( QgsPointSequence()
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );
  cc.addCurve( ls.clone() );

  QVERIFY( cc.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( cc.nCurves(), 1 );

  const QgsLineString *lsPtr = dynamic_cast< const QgsLineString * >( cc.curveAt( 0 ) );

  QCOMPARE( lsPtr->numPoints(), 2 );
  QCOMPARE( lsPtr->startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( lsPtr->endPoint(), QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );

  //add vertex at the end of linestring
  QVERIFY( cc.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( QgsWkbTypes::PointZM, 35, 43, 4, 5 ) ) );

  lsPtr = dynamic_cast< const QgsLineString * >( cc.curveAt( 0 ) );

  QCOMPARE( lsPtr->numPoints(), 3 );
  QCOMPARE( lsPtr->startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( lsPtr->endPoint(), QgsPoint( QgsWkbTypes::PointZM, 35, 43, 4, 5 ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 ) );
  cc.clear();
  cc.addCurve( ls.clone() );

  ls.setPoints( QgsPointSequence()
                << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );
  cc.addCurve( ls.clone() );

  QVERIFY( cc.deleteVertex( QgsVertexId( 0, 0, 2 ) ) );
  QCOMPARE( cc.nCurves(), 1 );

  lsPtr = dynamic_cast< const QgsLineString * >( cc.curveAt( 0 ) );

  QCOMPARE( lsPtr->numPoints(), 2 );
  QCOMPARE( lsPtr->startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( lsPtr->endPoint(), QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );
}

void TestQgsCompoundCurve::filterVertices()
{
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() > 5;
  };

  QgsCompoundCurve cc;
  cc.filterVertices( filter ); //no crash

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence()   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) );
  cc.addCurve( cs.clone() );
  cc.filterVertices( filter );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24))" ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 111, 23, 24, QgsWkbTypes::PointZM ) );
  cc.addCurve( ls.clone() );
  cc.filterVertices( filter );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24),(12 111 23 24, 22 122 33 34))" ) );
}

void TestQgsCompoundCurve::removeDuplicateNodes()
{
  QgsCompoundCurve cc;

  QVERIFY( !cc.removeDuplicateNodes() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.removeDuplicateNodes() );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12))" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) );
  cc.clear();
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.removeDuplicateNodes() );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 11 2))" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 10, 3 ) << QgsPoint( 11.01, 1.99 )
                << QgsPoint( 9, 3 ) << QgsPoint( 11, 2 ) );
  cc.clear();
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cc.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 10 3, 11.01 1.99, 9 3, 11 2))" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 )
                << QgsPoint( 111.01, 11.99 ) );
  cc.clear();
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.removeDuplicateNodes() );
  QCOMPARE( cc.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99))" ) );
  QVERIFY( cc.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !cc.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cc.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99))" ) );

  // with tiny segment
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 111.01, 11.99 )
                << QgsPoint( 111, 12 ) );
  cc.addCurve( ls.clone() );

  QVERIFY( !cc.removeDuplicateNodes() );
  QCOMPARE( cc.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99),(111.01 11.99, 111 12))" ) );
  QVERIFY( cc.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cc.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99))" ) );

  // with multiple duplicate nodes
  cc.fromWkt( QStringLiteral( "CompoundCurve ((11 1, 11 2, 11 2),CircularString(11 2, 10 3, 10 2),(10 2, 10 2, 11 1))" ) );

  QVERIFY( cc.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cc.asWkt( 0 ), QStringLiteral( "CompoundCurve ((11 1, 11 2),CircularString (11 2, 10 3, 10 2),(10 2, 11 1))" ) );

  // ensure continuity
  cc.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                << QgsPoint( 111.01, 11.99 ) << QgsPoint( 111, 12 ) );
  cc.addCurve( ls.clone() );
  ls.setPoints( QgsPointSequence() << QgsPoint( 111, 12 ) << QgsPoint( 31, 33 ) );
  cc.addCurve( ls.clone() );

  QVERIFY( cc.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cc.asWkt( 2 ), QStringLiteral( "CompoundCurve ((1 1, 111.01 11.99),(111.01 11.99, 31 33))" ) );
}

void TestQgsCompoundCurve::addZValue()
{
  QgsCompoundCurve cc;

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( cc.addZValue() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  cc.clear();

  //2d line
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  cc.addCurve( cs.clone() );
  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 3, 4 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.addZValue( 2 ) );

  QVERIFY( cc.is3D() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  QgsPoint pt;
  Qgis::VertexType v;

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 3, 4, 2 ) );

  QVERIFY( !cc.addZValue( 4 ) ); //already has z value, test that existing z is unchanged

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 3, 4, 2 ) );

  //linestring with m
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  cc.addCurve( cs.clone() );
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 21, 32, 0, 4 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.addZValue( 5 ) );

  QVERIFY( cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 32, 5, 4 ) );
}

void TestQgsCompoundCurve::addMValue()
{
  //2d line
  QgsCompoundCurve cc;
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  cc.addCurve( cs.clone() );
  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 3, 4 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.addMValue( 2 ) );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );

  QgsPoint pt;
  Qgis::VertexType v;

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 2 ) );


  QVERIFY( !cc.addMValue( 4 ) ); //already has z value, test that existing z is unchanged

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 2 ) );

  //linestring with z
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) );
  cc.addCurve( cs.clone() );
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 )
                << QgsPoint( QgsWkbTypes::PointZ, 21, 32, 4 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.addMValue( 5 ) );
  QVERIFY( cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 ) );
}

void TestQgsCompoundCurve::dropZValue()
{
  QgsCompoundCurve cc;

  QVERIFY( !cc.dropZValue() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  cc.addCurve( cs.clone() );
  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.dropZValue() );

  cc.addZValue( 1.0 );

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QVERIFY( cc.is3D() );
  QVERIFY( cc.dropZValue() );
  QVERIFY( !cc.is3D() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  QgsPoint pt;
  Qgis::VertexType v;

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 11, 12 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 21, 22 ) );

  QVERIFY( !cc.dropZValue() ); //already dropped

  //linestring with m
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  cc.addCurve( cs.clone() );
  cs.setPoints( QgsPointSequence() <<  QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 3, 4 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.dropZValue() );
  QVERIFY( !cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 4 ) );
}

void TestQgsCompoundCurve::dropMValue()
{
  QgsCompoundCurve cc;

  QVERIFY( !cc.dropMValue() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  cc.addCurve( cs.clone() );
  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.dropMValue() );

  cc.addMValue( 1.0 );

  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );
  QVERIFY( cc.isMeasure() );
  QVERIFY( cc.dropMValue() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  QgsPoint pt;
  Qgis::VertexType v;

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 11, 12 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 21, 22 ) );

  QVERIFY( !cc.dropMValue() ); //already dropped

  //linestring with z
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  cc.clear();
  cc.addCurve( cs.clone() );
  cs.setPoints( QgsPointSequence() <<  QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 3, 4 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.dropMValue() );
  QVERIFY( !cc.isMeasure() );
  QVERIFY( cc.is3D() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 21, 22, 3 ) );
}

void TestQgsCompoundCurve::isRing()
{
  QgsCompoundCurve cc;

  QVERIFY( !cc.isRing() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.isRing() ); //<4 points

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.isRing() ); //not closed

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.isRing() );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.isRing() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 12 )  << QgsPoint( 21, 22 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.isRing() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 21, 22 )  << QgsPoint( 1, 2 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.isRing() );
}

void TestQgsCompoundCurve::startPointEndPoint()
{
  QgsCompoundCurve cc;
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  cc.addCurve( ls.clone() );

  QCOMPARE( cc.startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( cc.endPoint(), QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );

  //bad start/end points. Test that this doesn't crash.
  cc.clear();
  QVERIFY( cc.startPoint().isEmpty() );
  QVERIFY( cc.endPoint().isEmpty() );
}

void TestQgsCompoundCurve::orientation()
{
  QgsCompoundCurve cc;

  ( void )cc.orientation(); // no crash

  cc.fromWkt( QStringLiteral( "CompoundCurve( ( 0 0, 0 1), CircularString (0 1, 1 1, 1 0), (1 0, 0 0))" ) );
  QCOMPARE( cc.orientation(), Qgis::AngularDirection::Clockwise );

  cc.fromWkt( QStringLiteral( "CompoundCurve( ( 0 0, 1 0), CircularString (1 0, 1 1, 0 1), (0 1, 0 0))" ) );
  QCOMPARE( cc.orientation(), Qgis::AngularDirection::CounterClockwise );
}

void TestQgsCompoundCurve::length()
{
  QgsCompoundCurve cc;
  QCOMPARE( cc.length(), 0.0 );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  cc.addCurve( ls.clone() );

  QGSCOMPARENEAR( cc.length(), 36.1433, 0.001 );
}

void TestQgsCompoundCurve::centroid()
{
  QgsCompoundCurve cc;
  QCOMPARE( cc.centroid(), QgsPoint() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.centroid(), QgsPoint( 5, 10 ) );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 20, 10 ) << QgsPoint( 2, 9 ) );
  cc.addCurve( cs.clone() );
  QgsPoint centroid = cc.centroid();

  QGSCOMPARENEAR( centroid.x(), 7.333, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 6.333, 0.001 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 2, 9 )
                << QgsPoint( 12, 9 ) << QgsPoint( 15, 19 ) );
  cc.addCurve( cs.clone() );
  centroid = cc.centroid();

  QGSCOMPARENEAR( centroid.x(), 9.756646, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 8.229039, 0.001 );
}

void TestQgsCompoundCurve::closestSegment()
{
  QgsCompoundCurve cc;
  QgsVertexId vId;

  int leftOf = 0;
  QgsPoint p( 0, 0 ); // reset all coords to zero
  ( void )cc.closestSegment( QgsPoint( 1, 2 ), p, vId ); //empty line, just want no crash

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.closestSegment( QgsPoint( 5, 10 ), p, vId ) < 0 );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) );
  cc.addCurve( cs.clone() );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 4, 11 ), p, vId, &leftOf ), 2.0, 0.0001 );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 8, 11 ), p, vId, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 5.5, 11.5 ), p, vId, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 7, 16 ), p, vId, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 5.5, 13.5 ), p, vId, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  // point directly on segment
  QCOMPARE( cc.closestSegment( QgsPoint( 5, 15 ), p, vId, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 15 ) << QgsPoint( 5, 20 ) );
  cc.addCurve( ls.clone() );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 5.5, 16.5 ), p, vId, &leftOf ), 0.25, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 16.5, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 4.5, 16.5 ), p, vId, &leftOf ), 0.25, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 16.5, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 4.5, 21.5 ), p, vId, &leftOf ), 2.500000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 5.5, 21.5 ), p, vId, &leftOf ), 2.500000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( cc.closestSegment( QgsPoint( 5, 20 ), p, vId, &leftOf ), 0.0000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 0 );
}

void TestQgsCompoundCurve::sumUpArea()
{
  QgsCompoundCurve cc;
  QgsCircularString cs;

  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  cc.sumUpArea( area );

  QCOMPARE( area, 1.0 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  cc.addCurve( cs.clone() );
  cc.sumUpArea( area );

  QCOMPARE( area, 1.0 );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  cc.addCurve( cs.clone() );
  cc.sumUpArea( area );

  QCOMPARE( area, 1.0 );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 )
                << QgsPoint( 2, 2 ) );
  cc.addCurve( cs.clone() );
  cc.sumUpArea( area );

  QGSCOMPARENEAR( area, 4.141593, 0.0001 );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 )
                << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  cc.addCurve( cs.clone() );
  cc.sumUpArea( area );

  QGSCOMPARENEAR( area, 7.283185, 0.0001 );

  // full circle
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 4, 0 ) << QgsPoint( 0, 0 ) );
  cc.addCurve( cs.clone() );
  area = 0.0;
  cc.sumUpArea( area );

  QGSCOMPARENEAR( area, 12.566370614359172, 0.0001 );

  //test that area of a compound curve ring is equal to a closed linestring with the same vertices
  cc.clear();
  QgsLineString *ll1 = new QgsLineString();
  ll1->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  cc.addCurve( ll1 );

  QgsLineString *ll2 = new QgsLineString();
  ll2->setPoints( QgsPointSequence() << QgsPoint( 0, 2 )
                  << QgsPoint( -1, 0 ) << QgsPoint( 0, -1 ) );
  cc.addCurve( ll2 );

  QgsLineString *ll3 = new QgsLineString();
  ll3->setPoints( QgsPointSequence() << QgsPoint( 0, -1 ) << QgsPoint( 1, 1 ) );
  cc.addCurve( ll3 );

  double ccArea = 0.0;
  cc.sumUpArea( ccArea );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                << QgsPoint( 0, 2 ) <<  QgsPoint( -1, 0 )
                << QgsPoint( 0, -1 ) << QgsPoint( 1, 1 ) );
  double lsArea = 0.0;
  ls.sumUpArea( lsArea );

  QGSCOMPARENEAR( ccArea, lsArea, 4 * std::numeric_limits<double>::epsilon() );
}

void TestQgsCompoundCurve::segmentLength()
{
  QgsCompoundCurve cc;

  QCOMPARE( cc.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( cc.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( cc.segmentLength( QgsVertexId( -1, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( cc.segmentLength( QgsVertexId( 1, 1, 0 ) ), 31.4159, 0.001 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( -9, 32 ) << QgsPoint( 1, 42 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( cc.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( cc.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 42 ) << QgsPoint( 10, 42 ) );
  cc.addCurve( ls.clone() );

  QCOMPARE( cc.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( cc.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( cc.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 4 ) ), 9.0 );
  QCOMPARE( cc.segmentLength( QgsVertexId( 0, 0, 5 ) ), 0.0 );
}

void TestQgsCompoundCurve::angle()
{
  QgsCompoundCurve cc;
  QgsCircularString cs;

  ( void )cc.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )cc.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  cc.addCurve( cs.clone() );

  ( void )cc.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  cc.addCurve( cs.clone() );

  ( void )cc.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  ( void )cc.vertexAngle( QgsVertexId( 0, 0, 1 ) ); //just want no crash, any answer is meaningless

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  cc.addCurve( cs.clone() );

  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 2 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  cc.addCurve( cs.clone() );

  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141593, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );

  ( void )cc.vertexAngle( QgsVertexId( 0, 0, 20 ) ); // no crash

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 )
                << QgsPoint( 0, 2 ) << QgsPoint( -1, 3 ) << QgsPoint( 0, 4 ) );
  cc.addCurve( cs.clone() );

  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 1.5708, 0.0001 );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 4 ) << QgsPoint( -1, 3 )
                << QgsPoint( 0, 2 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  cc.addCurve( cs.clone() );

  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 4.712389, 0.0001 );

  // with second curve
  QgsLineString ls38;
  ls38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, -1 ) );
  cc.addCurve( ls38.clone() );

  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 3.926991, 0.0001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.141593, 0.0001 );

  //closed circular string
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  cc.addCurve( cs.clone() );

  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0, 0.00001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.00001 );
  QGSCOMPARENEAR( cc.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0, 0.00001 );
}

void TestQgsCompoundCurve::boundary()
{
  QgsCompoundCurve cc;

  QVERIFY( !cc.boundary() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                << QgsPoint( 1, 1 ) );
  cc.addCurve( cs.clone() );

  QgsAbstractGeometry *boundary = cc.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );

  delete boundary;

  // closed string = no boundary
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  cc.addCurve( cs.clone() );
  QVERIFY( !cc.boundary() );

  //boundary with z
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  cc.addCurve( cs.clone() );

  boundary = cc.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->geometryN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->z(), 10.0 );
  QCOMPARE( mpBoundary->geometryN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );

  delete boundary;
}

void TestQgsCompoundCurve::boundingBox()
{
  // test that bounding box is updated after every modification to the circular string
  QgsCompoundCurve cc;
  QgsCircularString cs;

  QVERIFY( cc.boundingBox().isNull() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );

  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( -5, -10 )
                << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );

  QByteArray wkbToAppend = cc.asWkb();
  cc.clear();

  QVERIFY( cc.boundingBox().isNull() );

  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  cc.addCurve( cs.clone() );

  QCOMPARE( cc.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );

  cc.fromWkb( wkbToAppendPtr );
  QCOMPARE( cc.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );

  cc.fromWkt( QStringLiteral( "CompoundCurve(CircularString( 5 10, 6 10, 5.5 9 ))" ) );
  QCOMPARE( cc.boundingBox(), QgsRectangle( 5, 9, 6.125, 10.25 ) );

  cc.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QgsRectangle r = cc.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -3.014, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 14.014, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), -7.0146, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 12.4988, 0.01 );

  cc.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  r = cc.boundingBox();

  QGSCOMPARENEAR( r.xMinimum(), -10.294, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 12.294, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 31.856, 0.01 );

  cc.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  r = cc.boundingBox();

  QGSCOMPARENEAR( r.xMinimum(), 5, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 6.125, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 10.25, 0.01 );
}

void TestQgsCompoundCurve::interpolate()
{
  QgsCompoundCurve cc;

  std::unique_ptr< QgsPoint > interpolateResult( cc.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolateResult.get() );

  cc.fromWkt( QStringLiteral( "CompoundCurveZM( ( 5 0 -1 -2, 10 0 1 2 ), CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  interpolateResult.reset( cc.interpolatePoint( 0 ) );

  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (5 0 -1 -2)" ) );

  interpolateResult.reset( cc.interpolatePoint( -1 ) );
  QVERIFY( !interpolateResult.get() );

  interpolateResult.reset( cc.interpolatePoint( 100000 ) );
  QVERIFY( !interpolateResult.get() );

  interpolateResult.reset( cc.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (6 0 -0.6 -1.2)" ) );

  interpolateResult.reset( cc.interpolatePoint( 7 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11.42 0.91 5.73 6.73)" ) );

  interpolateResult.reset( cc.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (6.5 0 -0.4 -0.8)" ) );

  interpolateResult.reset( cc.interpolatePoint( cc.length() ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (12 0 13 14)" ) );

  cc.fromWkt( QStringLiteral( "CompoundCurveZ( ( 5 0 -1, 10 0 1 ), CircularStringZ (10 0 1, 11 1 3, 12 0 13))" ) );
  interpolateResult.reset( cc.interpolatePoint( 1 ) );

  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZ (6 0 -0.6)" ) );

  cc.fromWkt( QStringLiteral( "CompoundCurveM( ( 5 0 -1, 10 0 1 ), CircularStringM (10 0 1, 11 1 3, 12 0 13))" ) );
  interpolateResult.reset( cc.interpolatePoint( 1 ) );

  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointM (6 0 -0.6)" ) );

  cc.fromWkt( QStringLiteral( "CompoundCurve( ( 5 0, 10 0 ), CircularString (10 0, 11 1, 12 0))" ) );
  interpolateResult.reset( cc.interpolatePoint( 1 ) );

  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "Point (6 0)" ) );
}

void TestQgsCompoundCurve::swapXy()
{
  QgsCompoundCurve cc;

  cc.swapXy(); //no crash

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  cc.addCurve( cs.clone() );
  cc.swapXy();

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24))" ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM ) );
  cc.addCurve( ls.clone() );
  cc.swapXy();

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24),(111 12 23 24, 122 22 33 34))" ) );
}

void TestQgsCompoundCurve::reversed()
{
  QgsCompoundCurve cc;
  std::unique_ptr< QgsCompoundCurve > reversed( cc.reversed() );

  QVERIFY( reversed->isEmpty() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence()
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                << QgsPoint( QgsWkbTypes::PointZM, 23, 32, 7, 8 ) );
  cc.addCurve( ls.clone() );

  reversed.reset( cc.reversed() );

  QCOMPARE( reversed->numPoints(), 4 );
  QVERIFY( dynamic_cast< const QgsLineString * >( reversed->curveAt( 0 ) ) );
  QVERIFY( dynamic_cast< const QgsCircularString * >( reversed->curveAt( 1 ) ) );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );

  QgsPoint pt;
  Qgis::VertexType v;
  reversed->pointAt( 0, pt, v );

  reversed->pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );

  reversed->pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );

  reversed->pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
}

void TestQgsCompoundCurve::isClosed()
{
  QgsCompoundCurve cc;
  QgsCircularString cs;

  QVERIFY( !cc.isClosed() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 2 )
                << QgsPoint( 11, 22 ) << QgsPoint( 1, 22 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( !cc.isClosed() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 1, 2 ) );
  cc.addCurve( ls.clone() );

  QVERIFY( cc.isClosed() );

  //test that m values aren't considered when testing for closedness
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.isClosed() );
}

void TestQgsCompoundCurve::close()
{
  QgsLineString ls;
  QgsCompoundCurve cc;

  cc.close();
  QVERIFY( cc.isEmpty() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 1, 22 ) );
  cc.addCurve( ls.clone() );

  QCOMPARE( cc.numPoints(), 3 );
  QVERIFY( !cc.isClosed() );

  cc.close();
  QCOMPARE( cc.numPoints(), 4 );
  QVERIFY( cc.isClosed() );

  QgsPoint pt;
  Qgis::VertexType v;
  cc.pointAt( 3, pt, v );

  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );

  cc.close();
  QCOMPARE( cc.numPoints(), 4 );
  QVERIFY( cc.isClosed() );
}

void TestQgsCompoundCurve::transformVertices()
{
  // transform vertices
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 5 );
  };

  QgsCompoundCurve cc;
  cc.transformVertices( transform ); //no crash

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) );
  cc.addCurve( cs.clone() );
  cc.transformVertices( transform );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (3 5 7 9, 13 5 7 9, 13 15 17 19, 113 15 27 29, 3 5 7 9))" ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 111, 23, 24, QgsWkbTypes::PointZM ) );
  cc.addCurve( ls.clone() );
  cc.transformVertices( transform );

  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (5 8 11 14, 15 8 11 14, 15 18 21 24, 115 18 31 34, 5 8 11 14),(14 114 27 29, 24 125 37 39, 3 114 27 29))" ) );
}

void TestQgsCompoundCurve::transformWithClass()
{
  // transform using class
  QgsCompoundCurve cc;
  TestTransformer transformer;

  // no crash
  QVERIFY( cc.transform( &transformer ) );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.transform( &transformer ) );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (3 16 8 3, 33 16 8 3, 33 26 18 13, 333 26 28 23, 3 16 8 3))" ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 111, 23, 24, QgsWkbTypes::PointZM ) );
  cc.addCurve( ls.clone() );

  QVERIFY( cc.transform( &transformer ) );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (9 30 13 2, 99 30 13 2, 99 40 23 12, 999 40 33 22, 9 30 13 2),(36 125 28 23, 66 136 38 33, 3 125 28 23))" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !cc.transform( &failTransformer ) );

}

void TestQgsCompoundCurve::crsTransform()
{
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                << QgsPoint( 6474985, -3526584 ) );
  QgsCompoundCurve cc;
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 6474985, -3526584 )
                << QgsPoint( 6504985, -3526584 ) );
  cc.addCurve( ls.clone() );

  cc.transform( tr, Qgis::TransformDirection::Forward );

  QGSCOMPARENEAR( cc.xAt( 0 ), 175.771, 0.001 );
  QGSCOMPARENEAR( cc.yAt( 0 ), -39.724, 0.001 );
  QGSCOMPARENEAR( cc.xAt( 1 ), 176.959, 0.001 );
  QGSCOMPARENEAR( cc.yAt( 1 ), -38.7999, 0.001 );
  QGSCOMPARENEAR( cc.xAt( 2 ), 177.315211, 0.001 );
  QGSCOMPARENEAR( cc.yAt( 2 ), -38.799974, 0.001 );
  QGSCOMPARENEAR( cc.boundingBox().xMinimum(), 175.770033, 0.001 );
  QGSCOMPARENEAR( cc.boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( cc.boundingBox().xMaximum(), 177.315211, 0.001 );
  QGSCOMPARENEAR( cc.boundingBox().yMaximum(), -38.7999, 0.001 );
}

void TestQgsCompoundCurve::crs3dTransformAndReverse()
{
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  //3d CRS transform
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  QgsCompoundCurve cc;
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 6474985, -3526584, 3, 4 )
                << QgsPoint( 6504985, -3526584, 5, 6 ) );
  cc.addCurve( ls.clone() );

  cc.transform( tr, Qgis::TransformDirection::Forward );

  QgsPoint pt;
  Qgis::VertexType v;
  cc.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.x(), 175.771, 0.001 );
  QGSCOMPARENEAR( pt.y(), -39.724, 0.001 );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
  QCOMPARE( pt.m(), 2.0 );

  cc.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 176.959, 0.001 );
  QGSCOMPARENEAR( pt.y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( pt.z(), 3.0, 0.001 );
  QCOMPARE( pt.m(), 4.0 );

  cc.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 177.315211, 0.001 );
  QGSCOMPARENEAR( pt.y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( pt.z(), 5.0, 0.001 );
  QCOMPARE( pt.m(), 6.0 );

  //reverse transform
  cc.transform( tr, Qgis::TransformDirection::Reverse );

  cc.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.x(), 6374985, 100 );
  QGSCOMPARENEAR( pt.y(), -3626584, 100 );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
  QCOMPARE( pt.m(), 2.0 );

  cc.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 6474985, 100 );
  QGSCOMPARENEAR( pt.y(), -3526584, 100 );
  QGSCOMPARENEAR( pt.z(), 3.0, 0.001 );
  QCOMPARE( pt.m(), 4.0 );

  cc.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 6504985, 100 );
  QGSCOMPARENEAR( pt.y(), -3526584, 100 );
  QGSCOMPARENEAR( pt.z(), 5.0, 0.001 );
  QCOMPARE( pt.m(), 6.0 );

#if PROJ_VERSION_MAJOR<6 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //z value transform
  cc.transform( tr, Qgis::TransformDirection::Forward, true );

  cc.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.z(), -19.249066, 0.001 );

  cc.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.z(), -21.092128, 0.001 );

  cc.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.z(), -19.370485, 0.001 );

  cc.transform( tr, Qgis::TransformDirection::Reverse, true );

  cc.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.z(), 1, 0.001 );

  cc.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.z(), 3, 0.001 );

  cc.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.z(), 5, 0.001 );
#endif

}

void TestQgsCompoundCurve::QTransformation()
{
  QTransform qtr = QTransform::fromScale( 2, 3 );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QgsCompoundCurve cc;
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 13, 13, 14 ) );
  cc.addCurve( ls.clone() );
  cc.transform( qtr, 5, 2, 4, 3 );

  QgsPoint pt;
  Qgis::VertexType v;
  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 2, 6, 11, 16 ) );

  cc.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 22, 36, 31, 46 ) );

  cc.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 42, 39, 31, 46 ) );
  QCOMPARE( cc.boundingBox(), QgsRectangle( 2, 6, 42, 39 ) );
}

void TestQgsCompoundCurve::coordinateSequence()
{
  QgsCompoundCurve cc;
  QgsCoordinateSequence coords = cc.coordinateSequence();

  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QVERIFY( coords.at( 0 ).at( 0 ).isEmpty() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) <<
                QgsPoint( QgsWkbTypes::PointZM, 31, 32, 16, 17 ) );
  cc.addCurve( ls.clone() );

  coords = cc.coordinateSequence();

  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QCOMPARE( coords.at( 0 ).at( 0 ).count(), 4 );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 3 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 16, 17 ) );
}

void TestQgsCompoundCurve::points()
{
  QgsCompoundCurve cc;
  QgsPointSequence points;
  cc.points( points );

  QVERIFY( points.isEmpty() );

  QgsCircularString cl;
  cl.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  cc.addCurve( cl.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  cc.addCurve( ls.clone() );
  cc.points( points );

  QCOMPARE( points.count(), 4 );
  QCOMPARE( points.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( points.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 ) );
  QCOMPARE( points.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QCOMPARE( points.at( 3 ), QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
}

void TestQgsCompoundCurve::segmentize()
{
  QgsCompoundCurve cc;
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  cc.addCurve( cs.clone() );

  std::unique_ptr<QgsLineString> segmentized( static_cast< QgsLineString * >( cc.segmentize() ) );

  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );

  QCOMPARE( segmentized->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), cs.pointN( cc.numPoints() - 1 ) );

  //segmentize with Z/M
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 10, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 2, 21, 24 ) );
  cc.addCurve( cs.clone() );

  segmentized.reset( static_cast< QgsLineString * >( cc.segmentize() ) );

  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), cs.pointN( cc.numPoints() - 1 ) );

  //segmentize an empty line
  cc.clear();
  segmentized.reset( static_cast< QgsLineString * >( cc.segmentize() ) );

  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
}

void TestQgsCompoundCurve::substring()
{
  QgsCompoundCurve cc;
  std::unique_ptr< QgsCompoundCurve > substringResult( cc.curveSubstring( 1, 2 ) ); // no crash

  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );

  cc.fromWkt( QStringLiteral( "CompoundCurveZM( ( 5 0 -1 -2, 10 0 1 2 ), CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  substringResult.reset( cc.curveSubstring( 0, 0 ) );

  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 5 0 -1 -2))" ) );

  substringResult.reset( cc.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );

  substringResult.reset( cc.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );

  substringResult.reset( cc.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 6 0 -0.6 -1.2))" ) );

  substringResult.reset( cc.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 6 0 -0.6 -1.2))" ) );

  substringResult.reset( cc.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 10 0 1 2),CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );

  substringResult.reset( cc.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 10 0 1 2),CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );

  substringResult.reset( cc.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 10 0 1 2),CircularStringZM (10 0 1 2, 10.46 0.84 2.27 3.27, 11.42 0.91 5.73 6.73))" ) );

  substringResult.reset( cc.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 6.5 0 -0.4 -0.8))" ) );

  cc.fromWkt( QStringLiteral( "CompoundCurveZ( ( 5 0 -1, 10 0 1 ), CircularStringZ (10 0 1, 11 1 3, 12 0 13))" ) );
  substringResult.reset( cc.curveSubstring( 1, 7 ) );

  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveZ ((6 0 -0.6, 10 0 1),CircularStringZ (10 0 1, 10.46 0.84 2.27, 11.42 0.91 5.73))" ) );

  cc.fromWkt( QStringLiteral( "CompoundCurveM( ( 5 0 -1, 10 0 1 ), CircularStringM (10 0 1, 11 1 3, 12 0 13))" ) );
  substringResult.reset( cc.curveSubstring( 1, 7 ) );

  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurveM ((6 0 -0.6, 10 0 1),CircularStringM (10 0 1, 10.46 0.84 2.27, 11.42 0.91 5.73))" ) );

  cc.fromWkt( QStringLiteral( "CompoundCurve( ( 5 0, 10 0 ), CircularString (10 0, 11 1, 12 0))" ) );
  substringResult.reset( cc.curveSubstring( 1, 7 ) );

  QCOMPARE( substringResult->asWkt( 2 ),
            QStringLiteral( "CompoundCurve ((6 0, 10 0),CircularString (10 0, 10.46 0.84, 11.42 0.91))" ) );
}

void TestQgsCompoundCurve::convertTo()
{
  QgsCompoundCurve cc;

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  cc.addCurve( cs.clone() );

  QVERIFY( cc.convertTo( QgsWkbTypes::CompoundCurve ) );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( cc.convertTo( QgsWkbTypes::CompoundCurveZ ) );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZ );

  QgsPoint pt;
  Qgis::VertexType v;

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );
  QVERIFY( cc.convertTo( QgsWkbTypes::CompoundCurveZM ) );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2 ) );

  cc.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 5 ) );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );
  QVERIFY( cc.convertTo( QgsWkbTypes::CompoundCurveM ) );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveM );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2 ) );

  cc.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 0, 6 ) );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( cc.convertTo( QgsWkbTypes::CompoundCurve ) );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  cc.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( !cc.convertTo( QgsWkbTypes::Polygon ) );
}

void TestQgsCompoundCurve::curveToLine()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QgsCompoundCurve cc;
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  cc.addCurve( ls.clone() );

  std::unique_ptr<QgsLineString> segmentized( static_cast< QgsLineString * >( cc.curveToLine() ) );

  QCOMPARE( segmentized->numPoints(), 182 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), ls.pointN( ls.numPoints() - 1 ) );
}

void TestQgsCompoundCurve::toCurveType()
{
  QgsCompoundCurve cc;
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  cc.addCurve( cs.clone() );
  std::unique_ptr< QgsCurve > curve( cc.toCurveType() );

  QCOMPARE( curve->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curve->numPoints(), 3 );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 1, 25 ) );
  cc.addCurve( ls.clone() );
  curve.reset( cc.toCurveType() );

  QCOMPARE( curve->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curve->numPoints(), 4 );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 1, 25 ) );
}

void TestQgsCompoundCurve::asQPolygonF()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QgsCompoundCurve cc;
  cc.addCurve( cs.clone() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 23, 22 ) );
  cc.addCurve( ls.clone() );

  QPolygonF poly = cc.asQPolygonF();

  QCOMPARE( poly.count(), 183 );
  QCOMPARE( poly.at( 0 ).x(), 1.0 );
  QCOMPARE( poly.at( 0 ).y(), 2.0 );
  QCOMPARE( poly.at( 182 ).x(), 23.0 );
  QCOMPARE( poly.at( 182 ).y(), 22.0 );
}

void TestQgsCompoundCurve::toFromWKB()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QgsCompoundCurve cc;
  cc.addCurve( cs.clone() );

  QByteArray wkb = cc.asWkb();

  QCOMPARE( wkb.size(), cc.wkbSize() );

  cc.clear();
  QgsConstWkbPtr wkbPtr( wkb );
  cc.fromWkb( wkbPtr );

  QCOMPARE( cc.numPoints(), 4 );
  QCOMPARE( cc.vertexCount(), 4 );
  QCOMPARE( cc.nCoordinates(), 4 );
  QCOMPARE( cc.ringCount(), 1 );
  QCOMPARE( cc.partCount(), 1 );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( cc.is3D() );
  QVERIFY( cc.isMeasure() );
  QCOMPARE( cc.nCurves(), 1 );

  QCOMPARE( qgis::down_cast< const QgsCircularString *>( cc.curveAt( 0 ) )->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( cc.curveAt( 0 ) )->pointN( 1 ), cs.pointN( 1 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( cc.curveAt( 0 ) )->pointN( 2 ), cs.pointN( 2 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( cc.curveAt( 0 ) )->pointN( 3 ), cs.pointN( 3 ) );

  //bad WKB - check for no crash
  cc.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !cc.fromWkb( nullPtr ) );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );

  QgsPoint point( 1, 2 );
  wkb = point.asWkb();
  wkbPtr = QgsConstWkbPtr( wkb );

  QVERIFY( !cc.fromWkb( wkbPtr ) );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
}

void TestQgsCompoundCurve::toFromWKT()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QgsCompoundCurve cc;
  cc.addCurve( cs.clone() );

  QString wkt = cc.asWkt();
  QVERIFY( !wkt.isEmpty() );

  cc.clear();
  QVERIFY( cc.fromWkt( wkt ) );

  QCOMPARE( cc.numPoints(), 4 );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( cc.is3D() );
  QVERIFY( cc.isMeasure() );

  QCOMPARE( qgis::down_cast< const QgsCircularString *>( cc.curveAt( 0 ) )->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( cc.curveAt( 0 ) )->pointN( 1 ), cs.pointN( 1 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( cc.curveAt( 0 ) )->pointN( 2 ), cs.pointN( 2 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( cc.curveAt( 0 ) )->pointN( 3 ), cs.pointN( 3 ) );

  //bad WKT
  QVERIFY( !cc.fromWkt( "Polygon()" ) );
  QVERIFY( cc.isEmpty() );
  QCOMPARE( cc.numPoints(), 0 );
  QVERIFY( !cc.is3D() );
  QVERIFY( !cc.isMeasure() );
  QCOMPARE( cc.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( !cc.fromWkt( "CompoundCurve(LineString(0 0, 1 1),Point( 2 2 ))" ) );
}

void TestQgsCompoundCurve::exportImport()
{
  //asGML2
  QgsCompoundCurve exportCurve;
  QgsCircularString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 ) << QgsPoint( 51, 52 ) );
  exportCurve.addCurve( exportLine.clone() );

  QgsLineString exportLineString;
  exportLineString.setPoints( QgsPointSequence() << QgsPoint( 51, 52 )
                              << QgsPoint( 61, 62 ) );
  exportCurve.addCurve( exportLineString.clone() );

  QgsCircularString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );
  QgsCompoundCurve exportCurveFloat;
  exportCurveFloat.addCurve( exportLineFloat.clone() );
  QgsLineString exportLineStringFloat;
  exportLineStringFloat.setPoints( QgsPointSequence() << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 )
                                   << QgsPoint( 3 + 1 / 3.0, 3 + 2 / 3.0 ) );
  exportCurveFloat.addCurve( exportLineStringFloat.clone() );

  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52 61,62</coordinates></LineString>" ) );
  QString result = elemToString( exportCurve.asGml2( doc ) );
  QGSCOMPAREGML( result, expectedGML2 );

  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667 3.333,3.667</coordinates></LineString>" ) );
  result = elemToString( exportCurveFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( result, expectedGML2prec3 );

  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCompoundCurve().asGml2( doc ) ), expectedGML2empty );


  //asGML3
  QString expectedGML3( QStringLiteral( "<CompositeCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">51 52 61 62</posList></LineString></curveMember></CompositeCurve>" ) );
  result = elemToString( exportCurve.asGml3( doc ) );
  QCOMPARE( result, expectedGML3 );

  QString expectedGML3prec3( QStringLiteral( "<CompositeCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 2.667 3.333 3.667</posList></LineString></curveMember></CompositeCurve>" ) );
  result = elemToString( exportCurveFloat.asGml3( doc, 3 ) );
  QCOMPARE( result, expectedGML3prec3 );

  QString expectedGML3empty( QStringLiteral( "<CompositeCurve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCompoundCurve().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( "{\"coordinates\":[[31.0,32.0],[41.0,42.0],[51.0,52.0],[61.0,62.0]],\"type\":\"LineString\"}" );
  result = exportCurve.asJson();
  QCOMPARE( result, expectedJson );

  QString expectedJsonPrec3( "{\"coordinates\":[[0.333,0.667],[1.333,1.667],[2.333,2.667],[3.333,3.667]],\"type\":\"LineString\"}" );
  result = exportCurveFloat.asJson( 3 );
  QCOMPARE( result, expectedJsonPrec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>31,32,0 41,42,0 51,52,0 61,62,0</coordinates></LineString>" ) );
  QCOMPARE( exportCurve.asKml(), expectedKml );

  QString expectedKmlPrec3( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>0.333,0.667,0 1.333,1.667,0 2.333,2.667,0 3.333,3.667,0</coordinates></LineString>" ) );
  QCOMPARE( exportCurveFloat.asKml( 3 ), expectedKmlPrec3 );
}

void TestQgsCompoundCurve::addToPainterPath()
{
  // note most tests are in test_qgsgeometry.py
  QgsCompoundCurve cc;
  QgsCircularString cs;
  QPainterPath pPath;
  cc.addToPainterPath( pPath );

  QVERIFY( pPath.isEmpty() );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )
                << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 ) );
  cc.addCurve( cs.clone() );
  cc.addToPainterPath( pPath );

  QGSCOMPARENEAR( pPath.currentPosition().x(), 21.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 2.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 31, 12, 3 ) );
  cc.addCurve( ls.clone() );
  pPath = QPainterPath();
  cc.addToPainterPath( pPath );

  QGSCOMPARENEAR( pPath.currentPosition().x(), 31.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );

  // even number of points - should still work
  cc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  cc.addCurve( cs.clone() );

  pPath = QPainterPath();
  cc.addToPainterPath( pPath );

  QGSCOMPARENEAR( pPath.currentPosition().x(), 11.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );
}

void TestQgsCompoundCurve::compoundCurveCondense_data()
{
  QTest::addColumn<QString>( "curve" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "compound curve empty" ) << QStringLiteral( "COMPOUNDCURVE()" ) << QStringLiteral( "CompoundCurve EMPTY" );
  QTest::newRow( "compound curve one line" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 1 2))" ) << QStringLiteral( "CompoundCurve ((1 1, 1 2))" );
  QTest::newRow( "compound curve one circular string" ) << QStringLiteral( "COMPOUNDCURVE(CIRCULARSTRING(1 1, 1 2, 2 2))" ) << QStringLiteral( "CompoundCurve (CircularString (1 1, 1 2, 2 2))" );
  QTest::newRow( "compound curve can't condense" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),CIRCULARSTRING(1 1, 1 2, 2 2),(2 2, 2 3),CIRCULARSTRING(2 3, 2 0, 2 1))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1),CircularString (1 1, 1 2, 2 2),(2 2, 2 3),CircularString (2 3, 2 0, 2 1))" );
  QTest::newRow( "compound curve two lines" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),(1 1, 1 0, 3 0))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1, 1 0, 3 0))" );
  QTest::newRow( "compound curve three lines" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),(1 1, 1 0, 3 0),(3 0, 4 0, 5 0))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1, 1 0, 3 0, 4 0, 5 0))" );
  QTest::newRow( "compound curve two lines and cs" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),(1 1, 1 0, 3 0),CIRCULARSTRING(3 0, 4 0, 5 0))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1, 1 0, 3 0),CircularString (3 0, 4 0, 5 0))" );
  QTest::newRow( "compound curve two cs" ) << QStringLiteral( "COMPOUNDCURVE(CIRCULARSTRING(1 5, 1 4, 1 1),CIRCULARSTRING(1 1, 1 0, 3 0))" ) << QStringLiteral( "CompoundCurve (CircularString (1 5, 1 4, 1 1, 1 0, 3 0))" );
  QTest::newRow( "compound curve complex" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),(1 1, 1 0, 3 0),CIRCULARSTRING(3 0, 4 0, 5 0),(5 0, 6 0),(6 0, 7 0),CIRCULARSTRING(7 0, 8 0, 9 0),CIRCULARSTRING(9 0, 10 1, 10 0),(10 0, 10 -1))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1, 1 0, 3 0),CircularString (3 0, 4 0, 5 0),(5 0, 6 0, 7 0),CircularString (7 0, 8 0, 9 0, 10 1, 10 0),(10 0, 10 -1))" );
}

void TestQgsCompoundCurve::compoundCurveCondense()
{
  QFETCH( QString, curve );
  QFETCH( QString, expected );

  QgsGeometry g = QgsGeometry::fromWkt( curve );
  qgsgeometry_cast< QgsCompoundCurve * >( g.get() )->condenseCurves();

  QCOMPARE( g.asWkt(), expected );
}


QGSTEST_MAIN( TestQgsCompoundCurve )
#include "testqgscompoundcurve.moc"
