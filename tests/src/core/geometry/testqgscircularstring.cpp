/***************************************************************************
     testqgscircularstring.cpp
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
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgscoordinatetransform.h"
#include "testgeometryutils.h"
#include "testtransformer.h"

class TestQgsCircularString: public QObject
{
    Q_OBJECT
  private slots:
    void emptyConstructor();
    void constructorFrom3Points();
    void constructorFrom2PointsAndCenter();
    void constructorFromArray();
    void constructorFromArrayZ();
    void constructorFromArrayM();
    void constructorFromArrayZM();
    void addZValue();
    void addMValue();
    void dropZValue();
    void dropMValue();
    void swapXy();
    void pointN();
    void setPoints();
    void setPointsZM();
    void setPointsEmpty();
    void setPointsMixedDimensionality();
    void append();
    void appendZM();
    void gettersSetters();
    void gettersSettersZMWithLine2D();
    void insertVertex();
    void nextVertex();
    void vertexAtPointAt();
    void filterVertices();
    void transformVertices();
    void moveVertex();
    void deleteVertex();
    void startEndPoint();
    void points();
    void coordinateSequence();
    void clone();
    void clear();
    void equality();
    void isClosed();
    void isRing();
    void orientation();
    void length();
    void segmentLength();
    void angle();
    void sumUpArea();
    void closestSegment();
    void boundary();
    void boundingBox();
    void centroid();
    void segmentize();
    void interpolate();
    void substring();
    void reversed();
    void removeDuplicateNodes();
    void crsTransform();
    void transform();
    void asQPolygonF();
    void convertTo();
    void toCurveType();
    void curveToLine();
    void toFromWKB();
    void toFromWKT();
    void exportImport();
    void addToPainterPath();
};

void TestQgsCircularString::emptyConstructor()
{
  QgsCircularString cs;

  QVERIFY( cs.isEmpty() );
  QCOMPARE( cs.numPoints(), 0 );
  QCOMPARE( cs.vertexCount(), 0 );
  QCOMPARE( cs.nCoordinates(), 0 );
  QCOMPARE( cs.ringCount(), 0 );
  QCOMPARE( cs.partCount(), 0 );
  QVERIFY( !cs.is3D() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.wktTypeStr(), QString( "CircularString" ) );
  QCOMPARE( cs.geometryType(), QString( "CircularString" ) );
  QCOMPARE( cs.dimension(), 1 );
  QVERIFY( cs.hasCurvedSegments() );
  QCOMPARE( cs.area(), 0.0 );
  QCOMPARE( cs.perimeter(), 0.0 );

  QgsPointSequence pts;
  cs.points( pts );
  QVERIFY( pts.empty() );
}

void TestQgsCircularString::constructorFrom3Points()
{
  QgsCircularString cs( QgsPoint( 1, 2 ), QgsPoint( 21, 22 ), QgsPoint( 31, 2 ) );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), 22.0 );
  QCOMPARE( cs.xAt( 2 ), 31.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );

  cs = QgsCircularString( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ),
                          QgsPoint( QgsWkbTypes::PointZ, 21, 22, 23 ),
                          QgsPoint( QgsWkbTypes::PointZ, 31, 2, 33 ) );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.pointN( 0 ).z(), 3.0 );
  QVERIFY( std::isnan( cs.pointN( 0 ).m() ) );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), 22.0 );
  QCOMPARE( cs.pointN( 1 ).z(), 23.0 );
  QCOMPARE( cs.xAt( 2 ), 31.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );
  QCOMPARE( cs.pointN( 2 ).z(), 33.0 );

  cs = QgsCircularString( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ),
                          QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 23 ),
                          QgsPoint( QgsWkbTypes::PointM, 31, 2, 0, 33 ) );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.pointN( 0 ).m(), 3.0 );
  QVERIFY( std::isnan( cs.pointN( 0 ).z() ) );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), 22.0 );
  QCOMPARE( cs.pointN( 1 ).m(), 23.0 );
  QCOMPARE( cs.xAt( 2 ), 31.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );
  QCOMPARE( cs.pointN( 2 ).m(), 33.0 );

  cs = QgsCircularString( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ),
                          QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ),
                          QgsPoint( QgsWkbTypes::PointZM, 31, 2, 33, 34 ) );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.pointN( 0 ).z(), 3.0 );
  QCOMPARE( cs.pointN( 0 ).m(), 4.0 );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), 22.0 );
  QCOMPARE( cs.pointN( 1 ).z(), 23.0 );
  QCOMPARE( cs.pointN( 1 ).m(), 24.0 );
  QCOMPARE( cs.xAt( 2 ), 31.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );
  QCOMPARE( cs.pointN( 2 ).z(), 33.0 );
  QCOMPARE( cs.pointN( 2 ).m(), 34.0 );
}

void TestQgsCircularString::constructorFrom2PointsAndCenter()
{
  QgsCircularString cs = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( 1, 2 ), QgsPoint( 31, 2 ), QgsPoint( 21, 2 ) );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), 22.0 );
  QCOMPARE( cs.xAt( 2 ), 31.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );

  cs = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( 1, 2 ), QgsPoint( 31, 2 ), QgsPoint( 21, 2 ), false );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), -18.0 );
  QCOMPARE( cs.xAt( 2 ), 31.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );

  cs = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ),
       QgsPoint( QgsWkbTypes::PointZ, 32, 2, 33 ),
       QgsPoint( QgsWkbTypes::PointZ, 21, 2, 23 ) );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.pointN( 0 ).z(), 3.0 );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), 22.0 );
  QCOMPARE( cs.pointN( 1 ).z(), 23.0 );
  QCOMPARE( cs.xAt( 2 ), 32.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );
  QCOMPARE( cs.pointN( 2 ).z(), 33.0 );

  cs = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ),
       QgsPoint( QgsWkbTypes::PointM, 31, 2, 0, 33 ),
       QgsPoint( QgsWkbTypes::PointM, 21, 2, 0, 23 ) );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.pointN( 0 ).m(), 3.0 );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), 22.0 );
  QCOMPARE( cs.pointN( 1 ).m(), 23.0 );
  QCOMPARE( cs.xAt( 2 ), 31.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );
  QCOMPARE( cs.pointN( 2 ).m(), 33.0 );

  cs = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ),
       QgsPoint( QgsWkbTypes::PointZM, 31, 2, 33, 34 ),
       QgsPoint( QgsWkbTypes::PointZM, 21, 2, 23, 24 ) );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.pointN( 0 ).z(), 3.0 );
  QCOMPARE( cs.pointN( 0 ).m(), 4.0 );
  QCOMPARE( cs.xAt( 1 ), 21.0 );
  QCOMPARE( cs.yAt( 1 ), 22.0 );
  QCOMPARE( cs.pointN( 1 ).z(), 23.0 );
  QCOMPARE( cs.pointN( 1 ).m(), 24.0 );
  QCOMPARE( cs.xAt( 2 ), 31.0 );
  QCOMPARE( cs.yAt( 2 ), 2.0 );
  QCOMPARE( cs.pointN( 2 ).z(), 33.0 );
  QCOMPARE( cs.pointN( 2 ).m(), 34.0 );
}

void TestQgsCircularString::setPoints()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );

  QVERIFY( !cs.isEmpty() );
  QCOMPARE( cs.numPoints(), 1 );
  QCOMPARE( cs.vertexCount(), 1 );
  QCOMPARE( cs.nCoordinates(), 1 );
  QCOMPARE( cs.ringCount(), 1 );
  QCOMPARE( cs.partCount(), 1 );
  QVERIFY( !cs.is3D() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( cs.hasCurvedSegments() );
  QCOMPARE( cs.area(), 0.0 );
  QCOMPARE( cs.perimeter(), 0.0 );

  QgsPointSequence pts;
  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );

  cs = QgsCircularString();
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );

  QVERIFY( !cs.isEmpty() );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.vertexCount(), 3 );
  QCOMPARE( cs.nCoordinates(), 3 );
  QCOMPARE( cs.ringCount(), 1 );
  QCOMPARE( cs.partCount(), 1 );
  QVERIFY( !cs.is3D() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( cs.hasCurvedSegments() );

  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );

  //setting first vertex should set linestring z/m type
  cs = QgsCircularString();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QVERIFY( !cs.isEmpty() );
  QVERIFY( cs.is3D() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( cs.wktTypeStr(), QString( "CircularStringZ" ) );

  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  cs = QgsCircularString();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QVERIFY( !cs.isEmpty() );
  QVERIFY( !cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( cs.wktTypeStr(), QString( "CircularStringM" ) );

  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );

  cs = QgsCircularString();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( !cs.isEmpty() );
  QVERIFY( cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.wktTypeStr(), QString( "CircularStringZM" ) );
  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
}

void TestQgsCircularString::clear()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  cs.clear();

  QVERIFY( cs.isEmpty() );
  QCOMPARE( cs.numPoints(), 0 );
  QCOMPARE( cs.vertexCount(), 0 );
  QCOMPARE( cs.nCoordinates(), 0 );
  QCOMPARE( cs.ringCount(), 0 );
  QCOMPARE( cs.partCount(), 0 );
  QVERIFY( !cs.is3D() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
}

void TestQgsCircularString::setPointsEmpty()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );

  //setPoints with empty list, should clear linestring
  cs.setPoints( QgsPointSequence() );
  QVERIFY( cs.isEmpty() );
  QCOMPARE( cs.numPoints(), 0 );
  QCOMPARE( cs.vertexCount(), 0 );
  QCOMPARE( cs.nCoordinates(), 0 );
  QCOMPARE( cs.ringCount(), 0 );
  QCOMPARE( cs.partCount(), 0 );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );

  QgsPointSequence pts;
  cs.points( pts );
  QVERIFY( pts.empty() );
}

void TestQgsCircularString::setPointsZM()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );

  //with z
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  QCOMPARE( cs.numPoints(), 2 );
  QVERIFY( cs.is3D() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );

  QgsPointSequence pts;
  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
            << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //with m
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  QCOMPARE( cs.numPoints(), 2 );
  QVERIFY( !cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );

  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
            << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //with zm
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );

  QCOMPARE( cs.numPoints(), 2 );
  QVERIFY( cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );

  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
            << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
}

void TestQgsCircularString::setPointsMixedDimensionality()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );

  QCOMPARE( cs.numPoints(), 2 );
  QVERIFY( cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );

  QgsPointSequence pts;
  cs.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
            << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );
}

void TestQgsCircularString::pointN()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );

  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //out of range - just want no crash here
  QgsPoint bad = cs.pointN( -1 );
  bad = cs.pointN( 100 );
}

void TestQgsCircularString::gettersSetters()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );

  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.xAt( 1 ), 11.0 );
  QCOMPARE( cs.xAt( 2 ), 21.0 );
  QCOMPARE( cs.xAt( -1 ), 0.0 ); //out of range
  QCOMPARE( cs.xAt( 11 ), 0.0 ); //out of range

  cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 2 ) );
  QCOMPARE( cs.xAt( 0 ), 51.0 );

  cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 12 ) );
  QCOMPARE( cs.xAt( 1 ), 61.0 );

  cs.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 71.0, 2 ) ); //out of range
  cs.moveVertex( QgsVertexId( 0, 0, 11 ), QgsPoint( 71.0, 2 ) ); //out of range

  QCOMPARE( cs.yAt( 0 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.yAt( 2 ), 22.0 );
  QCOMPARE( cs.yAt( -1 ), 0.0 ); //out of range
  QCOMPARE( cs.yAt( 11 ), 0.0 ); //out of range

  cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 52 ) );
  QCOMPARE( cs.yAt( 0 ), 52.0 );

  cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 62 ) );
  QCOMPARE( cs.yAt( 1 ), 62.0 );

  QCOMPARE( cs.pointN( 0 ).z(), 3.0 );
  QCOMPARE( cs.pointN( 1 ).z(), 13.0 );
  QCOMPARE( cs.pointN( 2 ).z(), 23.0 );

  cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53 ) );
  QCOMPARE( cs.pointN( 0 ).z(), 53.0 );

  cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 63 ) );
  QCOMPARE( cs.pointN( 1 ).z(), 63.0 );

  QCOMPARE( cs.pointN( 0 ).m(), 4.0 );
  QCOMPARE( cs.pointN( 1 ).m(), 14.0 );
  QCOMPARE( cs.pointN( 2 ).m(), 24.0 );

  cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53, 54 ) );
  QCOMPARE( cs.pointN( 0 ).m(), 54.0 );

  cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 53, 64 ) );
  QCOMPARE( cs.pointN( 1 ).m(), 64.0 );
}

void TestQgsCircularString::gettersSettersZMWithLine2D()
{
  //check zAt/setZAt with non-3d linestring
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 )
                << QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 24 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( cs.pointN( 0 ).z() ) );
  QVERIFY( std::isnan( cs.pointN( 1 ).z() ) );
  cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53 ) );
  cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 63 ) );

  //check mAt/setMAt with non-measure linestring
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( cs.pointN( 0 ).m() ) );
  QVERIFY( std::isnan( cs.pointN( 1 ).m() ) );
  cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 0, 53 ) );
  cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 0, 63 ) );
}

void TestQgsCircularString::equality()
{
  QgsCircularString cs1;
  QgsCircularString cs2;

  QVERIFY( cs1 == cs2 );
  QVERIFY( !( cs1 != cs2 ) );

  cs1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  QVERIFY( !( cs1 == cs2 ) ); //different number of vertices
  QVERIFY( cs1 != cs2 );

  cs2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  QVERIFY( cs1 == cs2 );
  QVERIFY( !( cs1 != cs2 ) );

  cs1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  cs2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  QVERIFY( cs1 == cs2 ); //check non-integer equality
  QVERIFY( !( cs1 != cs2 ) );

  cs1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) << QgsPoint( 7, 8 ) );
  cs2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) << QgsPoint( 6, 9 ) );
  QVERIFY( !( cs1 == cs2 ) ); //different coordinates
  QVERIFY( cs1 != cs2 );

  QgsCircularString cs3;
  cs3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );

  QVERIFY( !( cs1 == cs3 ) ); //different dimension
  QVERIFY( cs1 != cs3 );

  QgsCircularString cs4;
  cs4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                 << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );

  QVERIFY( !( cs3 == cs4 ) ); //different z coordinates
  QVERIFY( cs3 != cs4 );

  QgsCircularString cs5;
  cs5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                 << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                 << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  QgsCircularString cs6;
  cs6.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                 << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                 << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );

  QVERIFY( !( cs5 == cs6 ) ); //different m values
  QVERIFY( cs5 != cs6 );

  QVERIFY( cs6 != QgsLineString() );
}

void TestQgsCircularString::isClosed()
{
  QgsCircularString cs;

  QVERIFY( !cs.isClosed2D() );
  QVERIFY( !cs.isClosed() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 2 ) << QgsPoint( 11, 22 )
                << QgsPoint( 1, 22 ) );

  QVERIFY( !cs.isClosed2D() );
  QVERIFY( !cs.isClosed() );
  QCOMPARE( cs.numPoints(), 4 );
  QCOMPARE( cs.area(), 0.0 );
  QCOMPARE( cs.perimeter(), 0.0 );

  //test that m values aren't considered when testing for closedness
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );

  QVERIFY( cs.isClosed2D() );
  QVERIFY( cs.isClosed() );

  // test with z
  cs.addZValue( 123.0 );

  QVERIFY( cs.isClosed2D() );
  QVERIFY( cs.isClosed() );

  QgsPoint pEnd = cs.endPoint();
  pEnd.setZ( 234.0 );
  cs.moveVertex( QgsVertexId( 0, 0, cs.numPoints() - 1 ), pEnd );

  QVERIFY( cs.isClosed2D() );
  QVERIFY( !cs.isClosed() );
}

void TestQgsCircularString::asQPolygonF()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 ) );

  QPolygonF poly = cs.asQPolygonF();

  QCOMPARE( poly.count(), 181 );
  QCOMPARE( poly.at( 0 ).x(), 1.0 );
  QCOMPARE( poly.at( 0 ).y(), 2.0 );
  QCOMPARE( poly.at( 180 ).x(), 11.0 );
  QCOMPARE( poly.at( 180 ).y(), 22.0 );
}

void TestQgsCircularString::clone()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 2 )
                << QgsPoint( 11, 22 ) << QgsPoint( 1, 22 ) );
  std::unique_ptr<QgsCircularString> cloned( cs.clone() );

  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->vertexCount(), 4 );
  QCOMPARE( cloned->ringCount(), 1 );
  QCOMPARE( cloned->partCount(), 1 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), cs.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), cs.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), cs.pointN( 3 ) );

  //clone with Z/M
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  cloned.reset( cs.clone() );

  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( cloned->is3D() );
  QVERIFY( cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), cs.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), cs.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), cs.pointN( 3 ) );

  //clone an empty line
  cs.clear();
  cloned.reset( cs.clone() );

  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularString );
}

void TestQgsCircularString::segmentize()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  std::unique_ptr<QgsLineString> segmentized( static_cast< QgsLineString * >( cs.segmentize() ) );

  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), cs.pointN( cs.numPoints() - 1 ) );

  //segmentize with Z/M
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 10, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 2, 21, 24 ) );
  segmentized.reset( static_cast< QgsLineString * >( cs.segmentize() ) );

  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), cs.pointN( cs.numPoints() - 1 ) );

  //segmentize an empty line
  cs.clear();
  segmentized.reset( static_cast< QgsLineString * >( cs.segmentize() ) );

  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
}

void TestQgsCircularString::toFromWKB()
{
  QgsCircularString cs1;
  cs1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QByteArray wkb1 = cs1.asWkb();

  QCOMPARE( wkb1.size(), cs1.wkbSize() );

  QgsCircularString cs2;
  QgsConstWkbPtr wkb1ptr( wkb1 );
  cs2.fromWkb( wkb1ptr );

  QCOMPARE( cs2.numPoints(), 4 );
  QCOMPARE( cs2.vertexCount(), 4 );
  QCOMPARE( cs2.nCoordinates(), 4 );
  QCOMPARE( cs2.ringCount(), 1 );
  QCOMPARE( cs2.partCount(), 1 );
  QCOMPARE( cs2.wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( cs2.is3D() );
  QVERIFY( cs2.isMeasure() );
  QCOMPARE( cs2.pointN( 0 ), cs1.pointN( 0 ) );
  QCOMPARE( cs2.pointN( 1 ), cs1.pointN( 1 ) );
  QCOMPARE( cs2.pointN( 2 ), cs1.pointN( 2 ) );
  QCOMPARE( cs2.pointN( 3 ), cs1.pointN( 3 ) );

  //bad WKB - check for no crash
  cs2.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !cs2.fromWkb( nullPtr ) );
  QCOMPARE( cs2.wkbType(), QgsWkbTypes::CircularString );

  QgsPoint point( 1, 2 );
  QByteArray wkb16 = point.asWkb();
  QgsConstWkbPtr wkb16ptr( wkb16 );

  QVERIFY( !cs2.fromWkb( wkb16ptr ) );
  QCOMPARE( cs2.wkbType(), QgsWkbTypes::CircularString );
}

void TestQgsCircularString::toFromWKT()
{
  QgsCircularString cs1;
  cs1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QString wkt = cs1.asWkt();

  QVERIFY( !wkt.isEmpty() );

  QgsCircularString cs2;
  QVERIFY( cs2.fromWkt( wkt ) );

  QCOMPARE( cs2.numPoints(), 4 );
  QCOMPARE( cs2.wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( cs2.is3D() );
  QVERIFY( cs2.isMeasure() );
  QCOMPARE( cs2.pointN( 0 ), cs1.pointN( 0 ) );
  QCOMPARE( cs2.pointN( 1 ), cs1.pointN( 1 ) );
  QCOMPARE( cs2.pointN( 2 ), cs1.pointN( 2 ) );
  QCOMPARE( cs2.pointN( 3 ), cs1.pointN( 3 ) );

  //bad WKT
  QVERIFY( !cs2.fromWkt( "Polygon()" ) );
  QVERIFY( cs2.isEmpty() );
  QCOMPARE( cs2.numPoints(), 0 );
  QVERIFY( !cs2.is3D() );
  QVERIFY( !cs2.isMeasure() );
  QCOMPARE( cs2.wkbType(), QgsWkbTypes::CircularString );
}

void TestQgsCircularString::exportImport()
{
  //asGML2
  QgsCircularString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 )
                        << QgsPoint( 51, 52 ) );

  QgsCircularString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );

  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLine.asGml2( doc ) ), expectedGML2 );

  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLineFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );

  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCircularString().asGml2( doc ) ), expectedGML2empty );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></ArcString></segments></Curve>" ) );
  QCOMPARE( elemToString( exportLine.asGml3( doc ) ), expectedGML3 );

  QString expectedGML3prec3( QStringLiteral( "<Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></ArcString></segments></Curve>" ) );
  QCOMPARE( elemToString( exportLineFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );

  QString expectedGML3empty( QStringLiteral( "<Curve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCircularString().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( QStringLiteral( "{\"coordinates\":[[31.0,32.0],[41.0,42.0],[51.0,52.0]],\"type\":\"LineString\"}" ) );
  QCOMPARE( exportLine.asJson(), expectedJson );

  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[[0.333,0.667],[1.333,1.667],[2.333,2.667]],\"type\":\"LineString\"}" ) );
  QCOMPARE( exportLineFloat.asJson( 3 ), expectedJsonPrec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>31,32,0 41,42,0 51,52,0</coordinates></LineString>" ) );
  QCOMPARE( exportLine.asKml(), expectedKml );

  QString expectedKmlPrec3( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>0.333,0.667,0 1.333,1.667,0 2.333,2.667,0</coordinates></LineString>" ) );
  QCOMPARE( exportLineFloat.asKml( 3 ), expectedKmlPrec3 );
}

void TestQgsCircularString::length()
{
  QgsCircularString cs;
  QCOMPARE( cs.length(), 0.0 );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  QGSCOMPARENEAR( cs.length(), 26.1433, 0.001 );
}

void TestQgsCircularString::startEndPoint()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  QCOMPARE( cs.startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( cs.endPoint(), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  //bad start/end points. Test that this doesn't crash.
  cs.clear();
  QVERIFY( cs.startPoint().isEmpty() );
  QVERIFY( cs.endPoint().isEmpty() );
}

void TestQgsCircularString::curveToLine()
{
  //curveToLine - no segmentation required, so should return a clone
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  std::unique_ptr<QgsLineString> curveToLine( static_cast< QgsLineString * >( cs.curveToLine() ) );

  QCOMPARE( curveToLine->numPoints(), 181 );
  QCOMPARE( curveToLine->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( curveToLine->is3D() );
  QVERIFY( curveToLine->isMeasure() );
  QCOMPARE( curveToLine->pointN( 0 ), cs.pointN( 0 ) );
  QCOMPARE( curveToLine->pointN( curveToLine->numPoints() - 1 ), cs.pointN( cs.numPoints() - 1 ) );
}

void TestQgsCircularString::points()
{
  QgsCircularString cs;
  QgsPointSequence points;
  cs.points( points );

  QVERIFY( cs.isEmpty() );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  cs.points( points );

  QCOMPARE( points.count(), 3 );
  QCOMPARE( points.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( points.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 ) );
  QCOMPARE( points.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
}

void TestQgsCircularString::crsTransform()
{
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) );// want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                << QgsPoint( 6474985, -3526584 ) );
  cs.transform( tr, Qgis::TransformDirection::Forward );

  QGSCOMPARENEAR( cs.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( cs.boundingBox().xMinimum(), 175.771, 0.001 );
  QGSCOMPARENEAR( cs.boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( cs.boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( cs.boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  cs = QgsCircularString();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  cs.transform( tr, Qgis::TransformDirection::Forward );

  QGSCOMPARENEAR( cs.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 0 ).z(), 1.0, 0.001 );
  QCOMPARE( cs.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( cs.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 1 ).z(), 3.0, 0.001 );
  QCOMPARE( cs.pointN( 1 ).m(), 4.0 );

  //reverse transform
  cs.transform( tr, Qgis::TransformDirection::Reverse );

  QGSCOMPARENEAR( cs.pointN( 0 ).x(), 6374985, 0.01 );
  QGSCOMPARENEAR( cs.pointN( 0 ).y(), -3626584, 0.01 );
  QGSCOMPARENEAR( cs.pointN( 0 ).z(), 1, 0.001 );
  QCOMPARE( cs.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( cs.pointN( 1 ).x(), 6474985, 0.01 );
  QGSCOMPARENEAR( cs.pointN( 1 ).y(), -3526584, 0.01 );
  QGSCOMPARENEAR( cs.pointN( 1 ).z(), 3, 0.001 );
  QCOMPARE( cs.pointN( 1 ).m(), 4.0 );

#if 0 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //z value transform
  cs.transform( tr, Qgis::TransformDirection::Forward, true );

  QGSCOMPARENEAR( cs.pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 1 ).z(), -21.092128, 0.001 );

  cs.transform( tr, Qgis::TransformDirection::Reverse, true );

  QGSCOMPARENEAR( cs.pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( cs.pointN( 1 ).z(), 3.0, 0.001 );
#endif
}

void TestQgsCircularString::transform()
{
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  cs.transform( qtr );

  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 2, 6, 3, 4 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 22, 36, 13, 14 ) );
  QCOMPARE( cs.boundingBox(), QgsRectangle( 2, 6, 22, 36 ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  cs.transform( QTransform::fromScale( 1, 1 ), 3, 2, 4, 3 );

  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 9, 16 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 29, 46 ) );

}

void TestQgsCircularString::insertVertex()
{
  QgsCircularString cs;

  //cannot insert vertex in empty line
  QVERIFY( !cs.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( cs.numPoints(), 0 );

  //2d line
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );

  QVERIFY( cs.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 4.0, 7.0 ) ) );

  QCOMPARE( cs.numPoints(), 5 );
  QVERIFY( !cs.is3D() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( 4.0, 7.0 ) );
  QGSCOMPARENEAR( cs.pointN( 2 ).x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( cs.pointN( 2 ).y(), 9.930870, 0.01 );
  QCOMPARE( cs.pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( cs.pointN( 4 ), QgsPoint( 1.0, 22.0 ) );

  QVERIFY( cs.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( cs.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 16.0 ) ) );

  QCOMPARE( cs.numPoints(), 9 );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QGSCOMPARENEAR( cs.pointN( 1 ).x(), 4.363083, 0.01 );
  QGSCOMPARENEAR( cs.pointN( 1 ).y(), 5.636917, 0.01 );
  QCOMPARE( cs.pointN( 2 ), QgsPoint( 8.0, 9.0 ) );
  QCOMPARE( cs.pointN( 3 ), QgsPoint( 18.0, 16.0 ) );
  QGSCOMPARENEAR( cs.pointN( 4 ).x(), 5.876894, 0.01 );
  QGSCOMPARENEAR( cs.pointN( 4 ).y(), 8.246211, 0.01 );
  QCOMPARE( cs.pointN( 5 ), QgsPoint( 4.0, 7.0 ) );
  QGSCOMPARENEAR( cs.pointN( 6 ).x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( cs.pointN( 6 ).y(), 9.930870, 0.01 );
  QCOMPARE( cs.pointN( 7 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( cs.pointN( 8 ), QgsPoint( 1.0, 22.0 ) );

  //insert vertex at end
  QVERIFY( !cs.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 31.0, 32.0 ) ) );

  //insert vertex past end
  QVERIFY( !cs.insertVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( cs.numPoints(), 9 );

  //insert vertex before start
  QVERIFY( !cs.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( cs.numPoints(), 9 );

  //insert 4d vertex in 4d line
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  QVERIFY( cs.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );

  QCOMPARE( cs.numPoints(), 5 );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( cs.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );

  QCOMPARE( cs.numPoints(), 7 );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );

  QVERIFY( cs.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 4, 103, 104 ) ) );

  QCOMPARE( cs.numPoints(), 5 );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 2, 4 ) );
}

void TestQgsCircularString::moveVertex()
{
  //empty line
  QgsCircularString cs;

  QVERIFY( !cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( cs.isEmpty() );

  //valid line
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );

  QVERIFY( cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( cs.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );

  QCOMPARE( cs.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( cs.pointN( 2 ), QgsPoint( 26.0, 27.0 ) );

  //out of range
  QVERIFY( !cs.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !cs.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );

  QCOMPARE( cs.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( cs.pointN( 2 ), QgsPoint( 26.0, 27.0 ) );

  //move 4d point in 4d line
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  QVERIFY( cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) );

  //move 2d point in 4d line, existing z/m should be maintained
  QVERIFY( cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 34, 35 ) ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 34, 35, 12, 13 ) );

  //move 4d point in 2d line
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );

  QVERIFY( cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 3, 4, 2, 3 ) ) );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( 3, 4 ) );
}

void TestQgsCircularString::deleteVertex()
{
  //empty line
  QgsCircularString cs;

  QVERIFY( cs.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( cs.isEmpty() );

  //valid line
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );

  //out of range vertices
  QVERIFY( !cs.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !cs.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( cs.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );

  QCOMPARE( cs.numPoints(), 2 );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );

  //removing the next vertex removes all remaining vertices
  QVERIFY( cs.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( cs.numPoints(), 0 );
  QVERIFY( cs.isEmpty() );

  QVERIFY( cs.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( cs.isEmpty() );

  //removing a vertex from a 3 point circular string should remove the whole line
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  QCOMPARE( cs.numPoints(), 3 );

  cs.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( cs.numPoints(), 0 );
}

void TestQgsCircularString::reversed()
{
  //reversed
  QgsCircularString cs;
  std::unique_ptr< QgsCircularString > reversed( cs.reversed() );

  QVERIFY( reversed->isEmpty() );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  reversed.reset( cs.reversed() );

  QCOMPARE( reversed->numPoints(), 3 );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  QCOMPARE( reversed->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( reversed->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( reversed->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
}

void TestQgsCircularString::addZValue()
{
  QgsCircularString cs;

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( cs.addZValue() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );

  cs.clear();
  QVERIFY( cs.addZValue() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );

  //2d line
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  QVERIFY( cs.addZValue( 2 ) );

  QVERIFY( cs.is3D() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  QVERIFY( !cs.addZValue( 4 ) ); //already has z value, test that existing z is unchanged
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );

  //linestring with m
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );

  QVERIFY( cs.addZValue( 5 ) );

  QVERIFY( cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );
}

void TestQgsCircularString::addMValue()
{
  QgsCircularString cs;

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( cs.addMValue() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );

  cs.clear();
  QVERIFY( cs.addMValue() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );

  //2d line
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  QVERIFY( cs.addMValue( 2 ) );

  QVERIFY( !cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  QVERIFY( !cs.addMValue( 4 ) ); //already has m value, test that existing m is unchanged
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );

  //linestring with z
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) );

  QVERIFY( cs.addMValue( 5 ) );

  QVERIFY( cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
}

void TestQgsCircularString::dropZValue()
{
  QgsCircularString cs;

  QVERIFY( !cs.dropZValue() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !cs.dropZValue() );

  cs.addZValue( 1.0 );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QVERIFY( cs.is3D() );
  QVERIFY( cs.dropZValue() );
  QVERIFY( !cs.is3D() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !cs.dropZValue() ); //already dropped

  //linestring with m
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );

  QVERIFY( cs.dropZValue() );

  QVERIFY( !cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
}

void TestQgsCircularString::dropMValue()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  QVERIFY( !cs.dropMValue() );

  cs.addMValue( 1.0 );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QVERIFY( cs.isMeasure() );
  QVERIFY( cs.dropMValue() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !cs.dropMValue() ); //already dropped

  //linestring with z
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );

  QVERIFY( cs.dropMValue() );

  QVERIFY( !cs.isMeasure() );
  QVERIFY( cs.is3D() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3, 0 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3, 0 ) );
}

void TestQgsCircularString::convertTo()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  QVERIFY( cs.convertTo( QgsWkbTypes::CircularString ) );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );

  QVERIFY( cs.convertTo( QgsWkbTypes::CircularStringZ ) );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );

  QVERIFY( cs.convertTo( QgsWkbTypes::CircularStringZM ) );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2 ) );
  cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 5 ) );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );
  //cs.setMAt( 0, 6.0 );

  QVERIFY( cs.convertTo( QgsWkbTypes::CircularStringM ) );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2 ) );
  cs.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 0, 6 ) );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );

  QVERIFY( cs.convertTo( QgsWkbTypes::CircularString ) );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( 1, 2 ) );

  QVERIFY( !cs.convertTo( QgsWkbTypes::Polygon ) );
}

void TestQgsCircularString::isRing()
{
  QgsCircularString cs;
  QVERIFY( !cs.isRing() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  QVERIFY( !cs.isRing() ); //<4 points

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  QVERIFY( !cs.isRing() ); //not closed

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  QVERIFY( cs.isRing() );
}

void TestQgsCircularString::coordinateSequence()
{
  QgsCircularString cs;
  QgsCoordinateSequence coords = cs.coordinateSequence();

  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QVERIFY( coords.at( 0 ).at( 0 ).isEmpty() );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  coords = cs.coordinateSequence();

  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QCOMPARE( coords.at( 0 ).at( 0 ).count(), 3 );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
}

void TestQgsCircularString::nextVertex()
{
  QgsCircularString cs;

  QgsVertexId v;
  QgsPoint p;
  QVERIFY( !cs.nextVertex( v, p ) );

  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !cs.nextVertex( v, p ) );

  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !cs.nextVertex( v, p ) );

  //CircularString
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  v = QgsVertexId( 0, 0, 2 ); //out of range
  QVERIFY( !cs.nextVertex( v, p ) );

  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( cs.nextVertex( v, p ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );

  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QVERIFY( !cs.nextVertex( v, p ) );

  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) ); //test that ring number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );

  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );

  // with Z
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );

  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );

  QVERIFY( !cs.nextVertex( v, p ) );

  // with M
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );

  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !cs.nextVertex( v, p ) );

  // with ZM
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );

  QVERIFY( cs.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  QVERIFY( !cs.nextVertex( v, p ) );
}

void TestQgsCircularString::vertexAtPointAt()
{
  QgsCircularString cs;

  cs.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  cs.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash

  QgsPoint p;
  Qgis::VertexType type;

  QVERIFY( !cs.pointAt( -10, p, type ) );
  QVERIFY( !cs.pointAt( 10, p, type ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );

  cs.vertexAt( QgsVertexId( 0, 0, -10 ) );
  cs.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash

  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QVERIFY( !cs.pointAt( -10, p, type ) );
  QVERIFY( !cs.pointAt( 10, p, type ) );
  QVERIFY( cs.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( cs.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );
  QVERIFY( cs.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  // with Z
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );

  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  QVERIFY( cs.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( cs.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );
  QVERIFY( cs.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22, 23 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  // with M
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 )
                << QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );

  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QVERIFY( cs.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( cs.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );
  QVERIFY( cs.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );

  // with ZM
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );

  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( cs.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QVERIFY( cs.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( cs.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, Qgis::VertexType::Curve );
  QVERIFY( cs.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsCircularString::centroid()
{
  QgsCircularString cs;
  QCOMPARE( cs.centroid(), QgsPoint() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QCOMPARE( cs.centroid(), QgsPoint( 5, 10 ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 20, 10 ) << QgsPoint( 2, 9 ) );
  QgsPoint centroid = cs.centroid();

  QGSCOMPARENEAR( centroid.x(), 7.333, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 6.333, 0.001 );
}

void TestQgsCircularString::closestSegment()
{
  QgsCircularString cs;
  QgsVertexId v;

  int leftOf = 0;
  QgsPoint p( 0, 0 ); // reset all coords to zero
  ( void )cs.closestSegment( QgsPoint( 1, 2 ), p, v ); //empty line, just want no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QVERIFY( cs.closestSegment( QgsPoint( 5, 10 ), p, v ) < 0 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2.0, 0.0001 );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 5.5, 11.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 7, 16 ), p, v, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 5.5, 13.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  // point directly on segment
  QCOMPARE( cs.closestSegment( QgsPoint( 5, 15 ), p, v, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  //clockwise string
  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 15 )
                << QgsPoint( 7, 12 ) << QgsPoint( 5, 10 ) );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5, 0.01 );
  QGSCOMPARENEAR( p.y(), 10, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 5.5, 11.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 7, 16 ), p, v, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( cs.closestSegment( QgsPoint( 5.5, 13.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  // point directly on segment
  QCOMPARE( cs.closestSegment( QgsPoint( 5, 15 ), p, v, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 0 );
}

void TestQgsCircularString::sumUpArea()
{
  QgsCircularString cs;
  double area = 1.0; //sumUpArea adds to area, so start with non-zero value

  cs.sumUpArea( area );
  QCOMPARE( area, 1.0 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  cs.sumUpArea( area );

  QCOMPARE( area, 1.0 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                << QgsPoint( 10, 10 ) );
  cs.sumUpArea( area );

  QCOMPARE( area, 1.0 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  cs.sumUpArea( area );

  QGSCOMPARENEAR( area, 4.141593, 0.0001 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 )
                << QgsPoint( 0, 2 ) );
  cs.sumUpArea( area );

  QGSCOMPARENEAR( area, 7.283185, 0.0001 );

  // full circle
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 4, 0 ) << QgsPoint( 0, 0 ) );
  area = 0.0;
  cs.sumUpArea( area );

  QGSCOMPARENEAR( area, 12.566370614359172, 0.0001 );
}

void TestQgsCircularString::boundingBox()
{
  // test that bounding box is updated after every modification to the circular string
  QgsCircularString cs;
  QVERIFY( cs.boundingBox().isNull() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QCOMPARE( cs.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( -5, -10 )
                << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  QCOMPARE( cs.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );

  QByteArray wkbToAppend = cs.asWkb();
  cs.clear();
  QVERIFY( cs.boundingBox().isNull() );

  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  cs.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                << QgsPoint( 10, 15 ) );

  QCOMPARE( cs.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );

  cs.fromWkb( wkbToAppendPtr );
  QCOMPARE( cs.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );

  cs.fromWkt( QStringLiteral( "CircularString( 5 10, 6 10, 5.5 9 )" ) );
  QCOMPARE( cs.boundingBox(), QgsRectangle( 5, 9, 6.125, 10.25 ) );

  cs.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QgsRectangle r = cs.boundingBox();

  QGSCOMPARENEAR( r.xMinimum(), -3.014, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 14.014, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), -7.0146, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 12.4988, 0.01 );

  cs.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  r = cs.boundingBox();

  QGSCOMPARENEAR( r.xMinimum(), -10.294, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 12.294, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 31.856, 0.01 );

  cs.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  r = cs.boundingBox();

  QGSCOMPARENEAR( r.xMinimum(), 5, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 6.125, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 10.25, 0.01 );
}

void TestQgsCircularString::angle()
{
  QgsCircularString cs;

  ( void )cs.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )cs.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  ( void )cs.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  ( void )cs.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  ( void )cs.vertexAngle( QgsVertexId( 0, 0, 1 ) ); //just want no crash, any answer is meaningless

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );

  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );

  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141593, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  ( void )cs.vertexAngle( QgsVertexId( 0, 0, 20 ) ); // no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 )
                << QgsPoint( -1, 3 ) << QgsPoint( 0, 4 ) );

  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 1.5708, 0.0001 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 4 )
                << QgsPoint( -1, 3 ) << QgsPoint( 0, 2 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );

  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 4.712389, 0.0001 );

  //closed circular string
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );

  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0, 0.00001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.00001 );
  QGSCOMPARENEAR( cs.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0, 0.00001 );
}

void TestQgsCircularString::boundary()
{
  QgsCircularString cs;

  QVERIFY( !cs.boundary() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QgsAbstractGeometry *boundary = cs.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->pointN( 0 )->x(), 0.0 );
  QCOMPARE( mpBoundary->pointN( 0 )->y(), 0.0 );
  QCOMPARE( mpBoundary->pointN( 1 )->x(), 1.0 );
  QCOMPARE( mpBoundary->pointN( 1 )->y(), 1.0 );
  delete boundary;

  // closed string = no boundary
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QVERIFY( !cs.boundary() );

  //boundary with z
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundary = cs.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->pointN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( mpBoundary->pointN( 0 )->x(), 0.0 );
  QCOMPARE( mpBoundary->pointN( 0 )->y(), 0.0 );
  QCOMPARE( mpBoundary->pointN( 0 )->z(), 10.0 );
  QCOMPARE( mpBoundary->pointN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );

  delete boundary;
}

void TestQgsCircularString::addToPainterPath()
{
  // note most tests are in test_qgsgeometry.py
  QgsCircularString path;
  QPainterPath pPath;

  path.addToPainterPath( pPath );
  QVERIFY( pPath.isEmpty() );

  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )
                  << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 ) );
  path.addToPainterPath( pPath );

  QGSCOMPARENEAR( pPath.currentPosition().x(), 21.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 2.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );

  // even number of points - should still work
  pPath = QPainterPath();
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  path.addToPainterPath( pPath );

  QGSCOMPARENEAR( pPath.currentPosition().x(), 11.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );
}

void TestQgsCircularString::toCurveType()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  std::unique_ptr< QgsCurve > curveType( cs.toCurveType() );

  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( curveType->numPoints(), 3 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
}

void TestQgsCircularString::segmentLength()
{
  QgsCircularString cs;

  QCOMPARE( cs.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );

  QCOMPARE( cs.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( cs.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( cs.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( cs.segmentLength( QgsVertexId( -1, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( cs.segmentLength( QgsVertexId( 1, 1, 0 ) ), 31.4159, 0.001 );

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 1, 22 ) << QgsPoint( -9, 32 ) << QgsPoint( 1, 42 ) );

  QCOMPARE( cs.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( cs.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( cs.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( cs.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );
}

void TestQgsCircularString::removeDuplicateNodes()
{
  QgsCircularString cs;

  QVERIFY( !cs.removeDuplicateNodes() );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );

  QVERIFY( !cs.removeDuplicateNodes() );
  QCOMPARE( cs.asWkt(), QStringLiteral( "CircularString (11 2, 11 12, 111 12)" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) );

  QVERIFY( !cs.removeDuplicateNodes() );
  QCOMPARE( cs.asWkt(), QStringLiteral( "CircularString (11 2, 11 12, 11 2)" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 10, 3 )
                << QgsPoint( 11.01, 1.99 ) << QgsPoint( 9, 3 ) << QgsPoint( 11, 2 ) );

  QVERIFY( !cs.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 10 3, 11.01 1.99, 9 3, 11 2)" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 )
                << QgsPoint( 11.02, 2.01 ) << QgsPoint( 11, 12 )
                << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );

  QVERIFY( !cs.removeDuplicateNodes() );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99)" ) );
  QVERIFY( cs.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !cs.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11 12, 111 12, 111.01 11.99)" ) );

  // don't create degenerate lines
  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) );

  QVERIFY( !cs.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularString (11 2)" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) );

  QVERIFY( !cs.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99)" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 )
                << QgsPoint( 11, 2 ) );

  QVERIFY( !cs.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99, 11 2)" ) );

  // with z
  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 )
                << QgsPoint( 11.02, 2.01, 3 ) << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) );

  QVERIFY( cs.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularStringZ (11 2 1, 11 12 4, 111 12 5)" ) );

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 )
                << QgsPoint( 11.02, 2.01, 3 ) << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) );

  QVERIFY( !cs.removeDuplicateNodes( 0.02, true ) );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularStringZ (11 2 1, 11.01 1.99 2, 11.02 2.01 3, 11 12 4, 111 12 5)" ) );
}

void TestQgsCircularString::swapXy()
{
  QgsCircularString cs;
  cs.swapXy(); // no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  cs.swapXy();

  QCOMPARE( cs.asWkt(), QStringLiteral( "CircularStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24)" ) );
}

void TestQgsCircularString::filterVertices()
{
  QgsCircularString cs;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() < 5;
  };

  cs.filterVertices( filter ); // no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  cs.filterVertices( filter );

  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularStringZM (1 2 3 4, 4 12 13 14)" ) );
}

void TestQgsCircularString::transformVertices()
{
  QgsCircularString cs;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 7 );
  };

  cs.transformVertices( transform ); // no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  cs.transformVertices( transform );

  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularStringZM (3 5 7 11, 6 15 17 21, 113 15 27 31)" ) );

  // transform using class
  cs = QgsCircularString();
  TestTransformer transformer;

  QVERIFY( cs.transform( &transformer ) ); // no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );

  QVERIFY( cs.transform( &transformer ) );
  QCOMPARE( cs.asWkt( 2 ), QStringLiteral( "CircularStringZM (3 16 8 3, 12 26 18 13, 333 26 28 23)" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !cs.transform( &failTransformer ) );
}

void TestQgsCircularString::substring()
{
  QgsCircularString cs;

  std::unique_ptr< QgsCircularString > substringResult( cs.curveSubstring( 1, 2 ) ); // no crash
  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );

  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 )
                << QgsPoint( 11, 1, 3, 4 ) << QgsPoint( 12, 0, 13, 14 ) );

  substringResult.reset( cs.curveSubstring( 0, 0 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 10 0 1 2, 10 0 1 2)" ) );

  substringResult.reset( cs.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );

  substringResult.reset( cs.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );

  substringResult.reset( cs.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 10.12 0.48 1.64 2.64, 10.46 0.84 2.27 3.27)" ) );

  substringResult.reset( cs.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.46 0.84 2.27 3.27, 10.46 0.84 2.27 3.27)" ) );

  substringResult.reset( cs.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)" ) );

  substringResult.reset( cs.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14)" ) );

  substringResult.reset( cs.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14)" ) );

  substringResult.reset( cs.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.68 0.95 2.59 3.59, 10.93 1 2.91 3.91)" ) );

  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 14 -1 13 14, 16 1 23 24 )
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) << QgsPoint( 11, 1, 3, 4 )
                << QgsPoint( 12, 0, 13, 14 ) << QgsPoint( 14, -1, 13, 14 ) << QgsPoint( 16, 1, 23, 24 ) );

  substringResult.reset( cs.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.68 0.95 2.59 3.59, 10.93 1 2.91 3.91)" ) );

  substringResult.reset( cs.curveSubstring( 1, 10 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14, 14 -1 13 14, 16 1 23 24)" ) );

  substringResult.reset( cs.curveSubstring( 1, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14, 13.1 -0.88 13 14, 14.5 -0.9 14.65 15.65)" ) );

  substringResult.reset( cs.curveSubstring( 0, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 13.1 -0.88 13 14, 14.5 -0.9 14.65 15.65)" ) );

  substringResult.reset( cs.curveSubstring( 5, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (13.51 -0.98 13 14, 14.01 -1 13.03 14.03, 14.5 -0.9 14.65 15.65)" ) );

  substringResult.reset( cs.curveSubstring( 5, 1000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (13.51 -0.98 13 14, 15.19 -0.53 17.2 18.2, 16 1 23 24)" ) );

  substringResult.reset( cs.curveSubstring( QgsGeometryUtils::distanceToVertex( cs, QgsVertexId( 0, 0, 2 ) ), QgsGeometryUtils::distanceToVertex( cs, QgsVertexId( 0, 0, 4 ) ) ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (12 0 13 14, 14.36 -0.94 14.19 15.19, 16 1 23 24)" ) );

  // CircularStringZ
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1 ) << QgsPoint( 11, 1, 3 ) << QgsPoint( 12, 0, 13 )
                << QgsPoint( 14, -1, 13 ) << QgsPoint( 16, 1, 23 ) );

  substringResult.reset( cs.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZ (10.46 0.84 2.27, 11.48 0.88 6.18, 12 0 13, 14 -1 13, 16 1 23)" ) );

  // CircularStringM
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 1 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 1, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 12, 0, 0, 13 )
                << QgsPoint( QgsWkbTypes::PointM, 14, -1, 0, 13 )
                << QgsPoint( QgsWkbTypes::PointM, 16, 1, 0, 23 ) );
  substringResult.reset( cs.curveSubstring( 1, 20 ) );

  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringM (10.46 0.84 2.27, 11.48 0.88 6.18, 12 0 13, 14 -1 13, 16 1 23)" ) );

  // CircularString
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 0 ) << QgsPoint( 11, 1 )
                << QgsPoint( 12, 0 ) << QgsPoint( 14, -1 ) << QgsPoint( 16, 1 ) );

  substringResult.reset( cs.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularString (10.46 0.84, 11.48 0.88, 12 0, 14 -1, 16 1)" ) );
}

void TestQgsCircularString::interpolate()
{
  QgsCircularString cs;

  std::unique_ptr< QgsPoint > interpolated( cs.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolated.get() );

  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 )
                << QgsPoint( 11, 1, 3, 4 ) << QgsPoint( 12, 0, 13, 14 ) );

  interpolated.reset( cs.interpolatePoint( 0 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZM (10 0 1 2)" ) );

  interpolated.reset( cs.interpolatePoint( -1 ) );
  QVERIFY( !interpolated.get() );

  interpolated.reset( cs.interpolatePoint( 100000 ) );
  QVERIFY( !interpolated.get() );

  interpolated.reset( cs.interpolatePoint( 1 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZM (10.46 0.84 2.27 3.27)" ) );

  interpolated.reset( cs.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZM (10.93 1 2.91 3.91)" ) );

  interpolated.reset( cs.interpolatePoint( cs.length() ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZM (12 0 13 14)" ) );

  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 14 -1 13 14, 16 1 23 24 )
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) << QgsPoint( 11, 1, 3, 4 )
                << QgsPoint( 12, 0, 13, 14 ) << QgsPoint( 14, -1, 13, 14 ) << QgsPoint( 16, 1, 23, 24 ) );

  interpolated.reset( cs.interpolatePoint( 1 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZM (10.46 0.84 2.27 3.27)" ) );

  interpolated.reset( cs.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZM (10.93 1 2.91 3.91)" ) );

  interpolated.reset( cs.interpolatePoint( 10 ) );
  QVERIFY( !interpolated.get() );

  interpolated.reset( cs.interpolatePoint( 6 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZM (14.5 -0.9 14.65 15.65)" ) );

  interpolated.reset( cs.interpolatePoint( 5 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZM (13.51 -0.98 13 14)" ) );

  // CircularStringZ
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1 ) << QgsPoint( 11, 1, 3 )
                << QgsPoint( 12, 0, 13 ) << QgsPoint( 14, -1, 13 ) << QgsPoint( 16, 1, 23 ) );

  interpolated.reset( cs.interpolatePoint( 1 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointZ (10.46 0.84 2.27)" ) );

  // CircularStringM
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 1 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 1, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 12, 0, 0, 13 )
                << QgsPoint( QgsWkbTypes::PointM, 14, -1, 0, 13 )
                << QgsPoint( QgsWkbTypes::PointM, 16, 1, 0, 23 ) );

  interpolated.reset( cs.interpolatePoint( 1 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "PointM (10.46 0.84 2.27)" ) );

  // CircularString
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 0 ) << QgsPoint( 11, 1 )
                << QgsPoint( 12, 0 ) << QgsPoint( 14, -1 ) << QgsPoint( 16, 1 ) );

  interpolated.reset( cs.interpolatePoint( 1 ) );
  QCOMPARE( interpolated->asWkt( 2 ), QStringLiteral( "Point (10.46 0.84)" ) );
}

void TestQgsCircularString::orientation()
{
  QgsCircularString cs;
  ( void )cs.orientation(); // no crash

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 )
                << QgsPoint( 1, 1 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( cs.orientation(), Qgis::AngularDirection::Clockwise );

  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( cs.orientation(), Qgis::AngularDirection::CounterClockwise );
}

void TestQgsCircularString::constructorFromArray()
{
  // test creating circular strings from arrays
  QVector< double > xx;
  QVector< double > yy;
  xx << 1 << 2 << 3;
  yy << 11 << 12 << 13;
  QgsCircularString cs( xx, yy );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.xAt( 2 ), 3.0 );
  QCOMPARE( cs.yAt( 2 ), 13.0 );

  // unbalanced
  xx = QVector< double >() << 1 << 2;
  yy = QVector< double >() << 11 << 12 << 13;
  cs = QgsCircularString( xx, yy );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.numPoints(), 2 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );

  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12;
  cs = QgsCircularString( xx, yy );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.numPoints(), 2 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
}

void TestQgsCircularString::constructorFromArrayZ()
{
  QVector< double > xx;
  QVector< double > yy;
  QVector< double > zz;
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  zz = QVector< double >() << 21 << 22 << 23;
  QgsCircularString cs( xx, yy, zz );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.pointN( 0 ).z(), 21.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.pointN( 1 ).z(), 22.0 );
  QCOMPARE( cs.xAt( 2 ), 3.0 );
  QCOMPARE( cs.yAt( 2 ), 13.0 );
  QCOMPARE( cs.pointN( 2 ).z(), 23.0 );

  // unbalanced -> z ignored
  zz = QVector< double >() << 21 << 22;
  cs = QgsCircularString( xx, yy, zz );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.xAt( 2 ), 3.0 );
  QCOMPARE( cs.yAt( 2 ), 13.0 );

  // unbalanced -> z truncated
  zz = QVector< double >() << 21 << 22 << 23 << 24;
  cs = QgsCircularString( xx, yy, zz );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.pointN( 0 ).z(), 21.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.pointN( 1 ).z(), 22.0 );
  QCOMPARE( cs.xAt( 2 ), 3.0 );
  QCOMPARE( cs.yAt( 2 ), 13.0 );
  QCOMPARE( cs.pointN( 2 ).z(), 23.0 );
}

void TestQgsCircularString::constructorFromArrayM()
{
  QVector< double > xx;
  QVector< double > yy;
  QVector< double > mm;
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  mm = QVector< double >() << 21 << 22 << 23;
  QgsCircularString cs( xx, yy, QVector< double >(), mm );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.pointN( 0 ).m(), 21.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.pointN( 1 ).m(), 22.0 );
  QCOMPARE( cs.xAt( 2 ), 3.0 );
  QCOMPARE( cs.yAt( 2 ), 13.0 );
  QCOMPARE( cs.pointN( 2 ).m(), 23.0 );

  // unbalanced -> m ignored
  mm = QVector< double >() << 21 << 22;
  cs = QgsCircularString( xx, yy, QVector< double >(), mm );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.xAt( 2 ), 3.0 );
  QCOMPARE( cs.yAt( 2 ), 13.0 );

  // unbalanced -> m truncated
  mm = QVector< double >() << 21 << 22 << 23 << 24;
  cs = QgsCircularString( xx, yy, QVector< double >(), mm );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.pointN( 0 ).m(), 21.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.pointN( 1 ).m(), 22.0 );
  QCOMPARE( cs.xAt( 2 ), 3.0 );
  QCOMPARE( cs.yAt( 2 ), 13.0 );
  QCOMPARE( cs.pointN( 2 ).m(), 23.0 );
}

void TestQgsCircularString::constructorFromArrayZM()
{
  QVector< double > xx;
  QVector< double > yy;
  QVector< double > zz;
  QVector< double > mm;
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  zz = QVector< double >() << 21 << 22 << 23;
  mm = QVector< double >() << 31 << 32 << 33;
  QgsCircularString cs( xx, yy, zz, mm );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.xAt( 0 ), 1.0 );
  QCOMPARE( cs.yAt( 0 ), 11.0 );
  QCOMPARE( cs.pointN( 0 ).z(), 21.0 );
  QCOMPARE( cs.pointN( 0 ).m(), 31.0 );
  QCOMPARE( cs.xAt( 1 ), 2.0 );
  QCOMPARE( cs.yAt( 1 ), 12.0 );
  QCOMPARE( cs.pointN( 1 ).z(), 22.0 );
  QCOMPARE( cs.pointN( 1 ).m(), 32.0 );
  QCOMPARE( cs.xAt( 2 ), 3.0 );
  QCOMPARE( cs.yAt( 2 ), 13.0 );
  QCOMPARE( cs.pointN( 2 ).z(), 23.0 );
  QCOMPARE( cs.pointN( 2 ).m(), 33.0 );
}

void TestQgsCircularString::append()
{
  //append to empty
  QgsCircularString cs;
  cs.append( nullptr );

  QVERIFY( cs.isEmpty() );
  QCOMPARE( cs.numPoints(), 0 );

  std::unique_ptr<QgsCircularString> toAppend( new QgsCircularString() );
  cs.append( toAppend.get() );

  QVERIFY( cs.isEmpty() );
  QCOMPARE( cs.numPoints(), 0 );

  toAppend->setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 12 )
                       << QgsPoint( 21, 22 ) );
  cs.append( toAppend.get() );

  QVERIFY( !cs.is3D() );
  QVERIFY( !cs.isMeasure() );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.vertexCount(), 3 );
  QCOMPARE( cs.nCoordinates(), 3 );
  QCOMPARE( cs.ringCount(), 1 );
  QCOMPARE( cs.partCount(), 1 );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( cs.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( cs.pointN( 2 ), toAppend->pointN( 2 ) );

  //add more points
  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 21, 22 )
                       << QgsPoint( 41, 42 ) << QgsPoint( 51, 52 ) );
  cs.append( toAppend.get() );

  QCOMPARE( cs.numPoints(), 5 );
  QCOMPARE( cs.vertexCount(), 5 );
  QCOMPARE( cs.nCoordinates(), 5 );
  QCOMPARE( cs.ringCount(), 1 );
  QCOMPARE( cs.partCount(), 1 );
  QCOMPARE( cs.pointN( 2 ), toAppend->pointN( 0 ) );
  QCOMPARE( cs.pointN( 3 ), toAppend->pointN( 1 ) );
  QCOMPARE( cs.pointN( 4 ), toAppend->pointN( 2 ) );

  //append another line the closes the original geometry.
  //Make sure there are not duplicate points except start and end point
  cs.clear();
  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 1, 1 )
                       << QgsPoint( 5, 5 )
                       << QgsPoint( 10, 1 ) );
  cs.append( toAppend.get() );

  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.vertexCount(), 3 );

  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 10, 1 )
                       << QgsPoint( 5, 2 )
                       << QgsPoint( 1, 1 ) );
  cs.append( toAppend.get() );

  QVERIFY( cs.isClosed() );
  QCOMPARE( cs.numPoints(), 5 );
  QCOMPARE( cs.vertexCount(), 5 );
}

void TestQgsCircularString::appendZM()
{
  //check dimensionality is inherited from append line if initially empty
  QgsCircularString cs;
  std::unique_ptr<QgsCircularString> toAppend( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 53, 54 ) );
  cs.append( toAppend.get() );

  QVERIFY( cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.numPoints(), 3 );
  QCOMPARE( cs.ringCount(), 1 );
  QCOMPARE( cs.partCount(), 1 );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( cs.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( cs.pointN( 2 ), toAppend->pointN( 2 ) );

  //append points with z to non z circular string
  cs.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 31, 32 )
                << QgsPoint( QgsWkbTypes::Point, 41, 42 )
                << QgsPoint( QgsWkbTypes::Point, 51, 52 ) );

  QVERIFY( !cs.is3D() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );

  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 141, 142, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 151, 152, 53, 54 ) );
  cs.append( toAppend.get() );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( 31, 32 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( 41, 42 ) );
  QCOMPARE( cs.pointN( 2 ), QgsPoint( 51, 52 ) );
  QCOMPARE( cs.pointN( 3 ), QgsPoint( 141, 142 ) );
  QCOMPARE( cs.pointN( 4 ), QgsPoint( 151, 152 ) );

  //append points without z/m to circularstring with z & m
  cs.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 11, 21 )
                << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 12, 22 )
                << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 13, 23 ) );

  QVERIFY( cs.is3D() );
  QVERIFY( cs.isMeasure() );
  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );

  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 51, 52 )
                       << QgsPoint( 141, 142 )
                       << QgsPoint( 151, 152 ) );
  cs.append( toAppend.get() );

  QCOMPARE( cs.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( cs.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 11, 21 ) );
  QCOMPARE( cs.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 41, 42, 12, 22 ) );
  QCOMPARE( cs.pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 51, 52, 13, 23 ) );
  QCOMPARE( cs.pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 141, 142 ) );
  QCOMPARE( cs.pointN( 4 ), QgsPoint( QgsWkbTypes::PointZM, 151, 152 ) );
}

QGSTEST_MAIN( TestQgsCircularString )
#include "testqgscircularstring.moc"
