/***************************************************************************
     testqgspoint.cpp
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

#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "testtransformer.h"
#include "qgscoordinatetransform.h"
#include "testgeometryutils.h"

class TestQgsPoint: public QObject
{
    Q_OBJECT
  private slots:
    void constructorv2();
    void constructor();
    void constructorZ();
    void constructorM();
    void constructorZM();
    void constructor25D();
    void clear();
    void clone();
    void assignment();
    void equality();
    void operators();
    void addDimension();
    void dropDimension();
    void swapXy();
    void settersGetters();
    void nextVertex();
    void vertexAt();
    void vertexNumberFromVertexId();
    void adjacentVertices();
    void vertexIterator();
    void coordinateSequence();
    void insertDeleteVertex();
    void moveVertex();
    void vertexAngle();
    void removeDuplicateNodes();
    void project();
    void closestSegment();
    void counts();
    void measures();
    void distance();
    void distance3D();
    void segmentLength();
    void azimuth();
    void inclination();
    void boundary();
    void boundingBox();
    void boundingBoxIntersects();
    void filterVertices();
    void transformVertices();
    void transformWithClass();
    void crsTransform();
    void qTransform();
    void convertTo();
    void toCurveType();
    void toQPointF();
    void toFromWkb();
    void toFromWkt();
    void exportImport();
};

void TestQgsPoint::constructorv2()
{
  QgsPoint pt;
  QString error;

  QVERIFY( pt.isEmpty() );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( pt.asWkt(), QStringLiteral( "Point EMPTY" ) );
  QVERIFY( pt.isValid( error ) );

  pt.setX( 1.0 );

  QVERIFY( pt.isEmpty() );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( pt.asWkt(), QStringLiteral( "Point EMPTY" ) );
  QVERIFY( pt.isValid( error ) );

  pt.setY( 2.0 );

  QVERIFY( !pt.isEmpty() );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( pt.asWkt(), QStringLiteral( "Point (1 2)" ) );
}

void TestQgsPoint::constructor()
{
  QgsPoint pt1( 5.0, 6.0 );

  QCOMPARE( pt1.x(), 5.0 );
  QCOMPARE( pt1.y(), 6.0 );
  QVERIFY( !pt1.isEmpty() );
  QVERIFY( !pt1.is3D() );
  QVERIFY( !pt1.isMeasure() );
  QCOMPARE( pt1.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( pt1.wktTypeStr(), QString( "Point" ) );

  QgsPointXY ptXY;
  QgsPoint pt2( ptXY );

  QVERIFY( std::isnan( pt2.x() ) );
  QVERIFY( std::isnan( pt2.y() ) );
  QVERIFY( pt2.isEmpty() );
  QVERIFY( !pt2.is3D() );
  QVERIFY( !pt2.isMeasure() );
  QCOMPARE( pt2.wkbType(), QgsWkbTypes::Point );

  QgsPoint pt3( QgsPointXY( 3.0, 4.0 ) );

  QCOMPARE( pt3.x(), 3.0 );
  QCOMPARE( pt3.y(), 4.0 );
  QVERIFY( !pt3.isEmpty() );
  QVERIFY( !pt3.is3D() );
  QVERIFY( !pt3.isMeasure() );
  QCOMPARE( pt3.wkbType(), QgsWkbTypes::Point );

  QgsPoint pt4( QPointF( 7.0, 9.0 ) );

  QCOMPARE( pt4.x(), 7.0 );
  QCOMPARE( pt4.y(), 9.0 );
  QVERIFY( !pt4.isEmpty() );
  QVERIFY( !pt4.is3D() );
  QVERIFY( !pt4.isMeasure() );
  QCOMPARE( pt4.wkbType(), QgsWkbTypes::Point );

  QgsPoint pt5( QgsWkbTypes::Point, 11.0, 13.0 );

  QCOMPARE( pt5.x(), 11.0 );
  QCOMPARE( pt5.y(), 13.0 );
  QVERIFY( !pt5.isEmpty() );
  QVERIFY( !pt5.is3D() );
  QVERIFY( !pt5.isMeasure() );
  QCOMPARE( pt5.wkbType(), QgsWkbTypes::Point );

  QgsPoint pt6( QgsWkbTypes::Point );

  QVERIFY( !pt6.is3D() );
  QVERIFY( !pt6.isMeasure() );
}

void TestQgsPoint::constructorZ()
{
  QgsPoint pt1( QgsWkbTypes::PointZ, 11.0, 13.0, 15.0 );

  QCOMPARE( pt1.x(), 11.0 );
  QCOMPARE( pt1.y(), 13.0 );
  QCOMPARE( pt1.z(), 15.0 );
  QVERIFY( !pt1.isEmpty() );
  QVERIFY( pt1.is3D() );
  QVERIFY( !pt1.isMeasure() );
  QCOMPARE( pt1.wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( pt1.wktTypeStr(), QString( "PointZ" ) );

  QgsPoint pt2( QgsWkbTypes::PointZ, 11.0, 13.0, 17.0, 18.0 );

  QCOMPARE( pt2.x(), 11.0 );
  QCOMPARE( pt2.y(), 13.0 );
  QVERIFY( std::isnan( pt2.m() ) );
  QCOMPARE( pt2.z(), 17.0 );
  QCOMPARE( pt2.wkbType(), QgsWkbTypes::PointZ );

  QgsPoint pt3( QgsWkbTypes::PointZ );

  QVERIFY( pt3.is3D() );
  QVERIFY( !pt3.isMeasure() );
}

void TestQgsPoint::constructorM()
{
  QgsPoint pt1( QgsWkbTypes::PointM, 11.0, 13.0, 0.0, 17.0 );

  QCOMPARE( pt1.x(), 11.0 );
  QCOMPARE( pt1.y(), 13.0 );
  QCOMPARE( pt1.m(), 17.0 );
  QVERIFY( !pt1.isEmpty() );
  QVERIFY( !pt1.is3D() );
  QVERIFY( pt1.isMeasure() );
  QCOMPARE( pt1.wkbType(), QgsWkbTypes::PointM );
  QCOMPARE( pt1.wktTypeStr(), QString( "PointM" ) );

  QgsPoint pt2( QgsWkbTypes::PointM, 11.0, 13.0, 15.0, 17.0 );

  QCOMPARE( pt2.x(), 11.0 );
  QCOMPARE( pt2.y(), 13.0 );
  QVERIFY( std::isnan( pt2.z() ) );
  QCOMPARE( pt2.m(), 17.0 );
  QCOMPARE( pt2.wkbType(), QgsWkbTypes::PointM );

  QgsPoint pt3( QgsWkbTypes::PointM );

  QVERIFY( !pt3.is3D() );
  QVERIFY( pt3.isMeasure() );

}

void TestQgsPoint::constructorZM()
{
  QgsPoint pt1( QgsWkbTypes::PointZM, 11.0, 13.0, 0.0, 17.0 );

  QCOMPARE( pt1.x(), 11.0 );
  QCOMPARE( pt1.y(), 13.0 );
  QCOMPARE( pt1.m(), 17.0 );
  QVERIFY( !pt1.isEmpty() );
  QVERIFY( pt1.is3D() );
  QVERIFY( pt1.isMeasure() );
  QCOMPARE( pt1.wkbType(), QgsWkbTypes::PointZM );
  QCOMPARE( pt1.wktTypeStr(), QString( "PointZM" ) );

  QgsPoint pt2( QgsWkbTypes::PointZM );

  QVERIFY( pt2.is3D() );
  QVERIFY( pt2.isMeasure() );

#if 0 //should trigger an assert
  //try creating a point with a nonsense WKB type
  QgsPoint pt3( QgsWkbTypes::PolygonZM, 11.0, 13.0, 9.0, 17.0 );
  QCOMPARE( pt3.wkbType(), QgsWkbTypes::Unknown );
#endif
}

void TestQgsPoint::constructor25D()
{
  QgsPoint pt( QgsWkbTypes::Point25D, 21.0, 23.0, 25.0 );

  QCOMPARE( pt.x(), 21.0 );
  QCOMPARE( pt.y(), 23.0 );
  QCOMPARE( pt.z(), 25.0 );
  QVERIFY( !pt.isEmpty() );
  QVERIFY( pt.is3D() );
  QVERIFY( !pt.isMeasure() );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point25D );
}

void TestQgsPoint::clear()
{
  QgsPoint pt( 5.0, 6.0 );

  pt.clear();
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );
  QVERIFY( std::isnan( pt.x() ) );
  QVERIFY( std::isnan( pt.y() ) );
}

void TestQgsPoint::clone()
{
  QgsPoint pt( QgsWkbTypes::PointZM, 9.0, 3.0, 13.0, 23.0 );

  std::unique_ptr< QgsPoint >clone( pt.clone() );
  QVERIFY( pt == *clone );
}

void TestQgsPoint::assignment()
{
  QgsPoint pt1( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QgsPoint pt2( 6.0, 7.0 );

  pt2 = pt1;
  QVERIFY( pt2 == pt1 );
}

void TestQgsPoint::equality()
{
  QgsPoint pt1, pt2;

  QVERIFY( pt1.isEmpty() );
  QVERIFY( pt2.isEmpty() );
  QVERIFY( pt2 == pt1 );

  pt1.setX( 1 );
  pt2.setY( 1 );

  QVERIFY( pt1.isEmpty() );
  QVERIFY( pt2.isEmpty() );
  QVERIFY( pt2 != pt1 );

  pt1.setY( 1 );
  pt2.setX( 1 );

  QVERIFY( !pt1.isEmpty() );
  QVERIFY( !pt2.isEmpty() );
  QVERIFY( pt2 == pt1 );

  // Test when X/Y/Z/M dimensions set at NaN

  // Y
  pt1.setY( std::numeric_limits<double>::quiet_NaN() );

  QVERIFY( pt1.isEmpty() );
  QVERIFY( !pt2.isEmpty() );
  QVERIFY( pt2 != pt1 );

  pt2.setY( std::numeric_limits<double>::quiet_NaN() );

  QVERIFY( pt1.isEmpty() );
  QVERIFY( pt2.isEmpty() );
  QVERIFY( pt2 == pt1 );

  pt1.setY( 1 );
  pt2.setY( 1 );

  // Z
  pt1.addZValue( std::numeric_limits<double>::quiet_NaN() );
  pt2.addZValue( 1 );

  QVERIFY( !pt1.isEmpty() );
  QVERIFY( !pt2.isEmpty() );
  QVERIFY( pt2 != pt1 );

  pt2.setZ( std::numeric_limits<double>::quiet_NaN() );
  QVERIFY( pt2 == pt1 );

  // M
  pt1.addMValue( std::numeric_limits<double>::quiet_NaN() );
  pt2.addMValue( 1 );
  QVERIFY( pt2 != pt1 );

  pt2.setM( std::numeric_limits<double>::quiet_NaN() );
  QVERIFY( pt2 == pt1 );


  QVERIFY( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ==
           QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) );

  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 2 / 3.0, 1 / 3.0 ) ==
              QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );

  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 1 / 3.0, 1 / 3.0 ) ==
              QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );

  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 ) ==
              QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );

  QVERIFY( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) ==
           QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) );

  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) ==
              QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 1 / 3.0 ) ) );

  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 2 / 3.0 ) ==
              QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) ) );

  QVERIFY( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) ==
           QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) );

  QVERIFY( !( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) ==
              QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 0.0, 1 / 3.0 ) ) );

  QVERIFY( !( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) ==
              QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 2 / 3.0 ) ) );

  QVERIFY( QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 2 / 3.0, 1 / 3.0 ) ==
           QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 2 / 3.0, 1 / 3.0 ) );

  QVERIFY( QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) ==
           QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) );

  QVERIFY( !( QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) ==
              QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 2 / 3.0 ) ) );

  //test inequality operator
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) !=
              QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );

  QVERIFY( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) !=
           QgsPoint( QgsWkbTypes::PointZ, 2 / 3.0, 1 / 3.0 ) );

  QgsLineString ls;
  QVERIFY( pt1 != ls );
  QVERIFY( !( pt1 == ls ) );
}

void TestQgsPoint::operators()
{
  QgsPoint pt1( 1, 2 );
  QgsPoint pt2( 3, 5 );

  QCOMPARE( pt2 - pt1, QgsVector( 2, 3 ) );
  QCOMPARE( pt1 - pt2, QgsVector( -2, -3 ) );

//  pt1 = QgsPoint( 1, 2 ); ???
  QCOMPARE( pt1 + QgsVector( 3, 5 ), QgsPoint( 4, 7 ) );

  pt1 += QgsVector( 3, 5 );
  QCOMPARE( pt1, QgsPoint( 4, 7 ) );

  QCOMPARE( pt1 - QgsVector( 3, 5 ), QgsPoint( 1, 2 ) );

  pt1 -= QgsVector( 3, 5 );
  QCOMPARE( pt1, QgsPoint( 1, 2 ) );
}

void TestQgsPoint::addDimension()
{
  //addZValue
  QgsPoint pt( 1.0, 2.0 );
  QVERIFY( pt.addZValue( 5.0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 5.0 ) );
  QVERIFY( !pt.addZValue( 6.0 ) );

  //addMValue
  pt = QgsPoint( 1.0, 2.0 );
  QVERIFY( pt.addMValue( 5.0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 5.0 ) );
  QVERIFY( !pt.addMValue( 6.0 ) );
}

void TestQgsPoint::dropDimension()
{
  //dropZ
  QgsPoint pt( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 );
  QVERIFY( pt.dropZValue() );
  QCOMPARE( pt, QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !pt.dropZValue() );

  pt = QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QVERIFY( pt.dropZValue() );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 4.0 ) );
  QVERIFY( !pt.dropZValue() );

  pt = QgsPoint( QgsWkbTypes::Point25D, 1.0, 2.0, 3.0 );
  QVERIFY( pt.dropZValue() );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) );
  QVERIFY( !pt.dropZValue() );

  //dropM
  pt = QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 );
  QVERIFY( pt.dropMValue() );
  QCOMPARE( pt, QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !pt.dropMValue() );

  pt = QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QVERIFY( pt.dropMValue() );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0, 0.0 ) );
  QVERIFY( !pt.dropMValue() );

}

void TestQgsPoint::swapXy()
{
  QgsPoint pt( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  pt.swapXy();

  QCOMPARE( pt.x(), 2.2 );
  QCOMPARE( pt.y(), 1.1 );
  QCOMPARE( pt.z(), 3.3 );
  QCOMPARE( pt.m(), 4.4 );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::PointZM );
}

void TestQgsPoint::settersGetters()
{
  QgsPoint ptZM( QgsWkbTypes::PointZM );
  QgsPoint pt;

  //x
  ptZM.setX( 5.0 );
  QCOMPARE( ptZM.x(), 5.0 );
  QCOMPARE( ptZM.rx(), 5.0 );

  ptZM.rx() = 9.0;
  QCOMPARE( ptZM.x(), 9.0 );

  //y
  ptZM.setY( 7.0 );
  QCOMPARE( ptZM.y(), 7.0 );
  QCOMPARE( ptZM.ry(), 7.0 );

  ptZM.ry() = 3.0;
  QCOMPARE( ptZM.y(), 3.0 );

  //z
  ptZM.setZ( 17.0 );
  QCOMPARE( ptZM.is3D(), true );
  QCOMPARE( ptZM.z(), 17.0 );
  QCOMPARE( ptZM.rz(), 17.0 );

  ptZM.rz() = 13.0;
  QCOMPARE( ptZM.z(), 13.0 );

  pt.setZ( 5.0 );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );
  QVERIFY( std::isnan( pt.z() ) );

  //m
  ptZM.setM( 27.0 );
  QCOMPARE( ptZM.m(), 27.0 );
  QCOMPARE( ptZM.rm(), 27.0 );

  ptZM.rm() = 23.0;
  QCOMPARE( ptZM.m(), 23.0 );

  pt.setM( 9.0 );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );
  QVERIFY( std::isnan( pt.m() ) );

  //other checks
  QCOMPARE( ptZM.geometryType(), QString( "Point" ) );
  QCOMPARE( ptZM.dimension(), 0 );
}

void TestQgsPoint::nextVertex()
{
  QgsPoint pt1( 3.0, 4.0 );
  QgsPoint pt2;
  QgsVertexId v( 0, 0, -1 );

  QVERIFY( pt1.nextVertex( v, pt2 ) );
  QCOMPARE( pt2, pt1 );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );

  //no more vertices
  QVERIFY( !pt1.nextVertex( v, pt2 ) );

  v = QgsVertexId( 0, 1, -1 ); //test that ring number is maintained
  QVERIFY( pt1.nextVertex( v, pt2 ) );
  QCOMPARE( pt2, pt1 );
  QCOMPARE( v, QgsVertexId( 0, 1, 0 ) );

  v = QgsVertexId( 1, 0, -1 ); //test that part number is maintained
  QVERIFY( pt1.nextVertex( v, pt2 ) );
  QCOMPARE( pt2, pt1 );
  QCOMPARE( v, QgsVertexId( 1, 0, 0 ) );
}

void TestQgsPoint::vertexAt()
{
  QgsPoint pt( 3.0, 4.0 );

  //will always be same as point
  QCOMPARE( pt.vertexAt( QgsVertexId() ), pt );
  QCOMPARE( pt.vertexAt( QgsVertexId( 0, 0, 0 ) ), pt );
}

void TestQgsPoint::vertexNumberFromVertexId()
{
  QgsPoint pt( 1, 2 );

  QCOMPARE( pt.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( pt.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  QCOMPARE( pt.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
}

void TestQgsPoint::adjacentVertices()
{
  //both should be invalid
  QgsPoint pt( 3.0, 4.0 );

  QgsVertexId v( 1, 0, 0 );
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );

  pt.adjacentVertices( v, prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
}

void TestQgsPoint::vertexIterator()
{
  QgsPoint pt( 3.0, 4.0 );

  QgsAbstractGeometry::vertex_iterator it1 = pt.vertices_begin();
  QgsAbstractGeometry::vertex_iterator it1end = pt.vertices_end();

  QCOMPARE( *it1, pt );
  QCOMPARE( it1.vertexId(), QgsVertexId( 0, 0, 0 ) );

  ++it1;
  QCOMPARE( it1, it1end );

  // Java-style iterator
  QgsVertexIterator it2( &pt );

  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), pt );
  QVERIFY( !it2.hasNext() );
}

void TestQgsPoint::coordinateSequence()
{
  QgsPoint pt( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QgsCoordinateSequence coord = pt.coordinateSequence();

  QCOMPARE( coord.count(), 1 );
  QCOMPARE( coord.at( 0 ).count(), 1 );
  QCOMPARE( coord.at( 0 ).at( 0 ).count(), 1 );
  QCOMPARE( coord.at( 0 ).at( 0 ).at( 0 ), pt );
}

void TestQgsPoint::insertDeleteVertex()
{
  //low level editing
  //insertVertex should have no effect
  QgsPoint pt( QgsWkbTypes::PointZM, 3.0, 4.0, 6.0, 7.0 );

  pt.insertVertex( QgsVertexId( 1, 2, 3 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 6.0, 7.0 ) );

  //deleteVertex - should do nothing, but not crash
  pt = QgsPoint( 2.0, 3.0 );
  pt.deleteVertex( QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 2.0, 3.0 ) );
}

void TestQgsPoint::moveVertex()
{
  QgsPoint pt( QgsWkbTypes::PointZM, 3.0, 4.0, 6.0, 7.0 );

  pt.moveVertex( QgsVertexId( 0, 0, 0 ),
                 QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  //invalid vertex id, should not crash
  pt.moveVertex( QgsVertexId( 1, 2, 3 ),
                 QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );

  //move PointZM using Point
  pt.moveVertex( QgsVertexId( 0, 0, 0 ),
                 QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11.0, 12.0, 1.0, 2.0 ) );

  //move PointZM using PointZ
  pt.moveVertex( QgsVertexId( 0, 0, 0 ),
                 QgsPoint( QgsWkbTypes::PointZ, 21.0, 22.0, 23.0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21.0, 22.0, 23.0, 2.0 ) );

  //move PointZM using PointM
  pt.moveVertex( QgsVertexId( 0, 0, 0 ),
                 QgsPoint( QgsWkbTypes::PointM, 31.0, 32.0, 0.0, 43.0 ) );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 31.0, 32.0, 23.0, 43.0 ) );

  //move Point using PointZM (z/m should be ignored)
  pt = QgsPoint( 3.0, 4.0 );
  pt.moveVertex( QgsVertexId( 0, 0, 0 ),
                 QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  QCOMPARE( pt, QgsPoint( 2.0, 3.0 ) );
}

void TestQgsPoint::vertexAngle()
{
  QgsPoint pt( 3.0, 4.0 );

  //undefined, but check that it doesn't crash
  ( void )pt.vertexAngle( QgsVertexId() );
}

void TestQgsPoint::removeDuplicateNodes()
{
  QgsPoint pt( 1, 2 );

  QVERIFY( !pt.removeDuplicateNodes() );
  QCOMPARE( pt.x(), 1.0 );
  QCOMPARE( pt.y(), 2.0 );
}

void TestQgsPoint::project()
{
  // 2D
  QgsPoint pt( 1, 2 );

  QCOMPARE( pt.project( 1, 0 ), QgsPoint( 1, 3 ) );
  QCOMPARE( pt.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );
  QCOMPARE( pt.project( 1.5, 90 ), QgsPoint( 2.5, 2 ) );
  QCOMPARE( pt.project( 1.5, 90, 90 ), QgsPoint( 2.5, 2 ) ); // stay QgsWkbTypes::Point
  QCOMPARE( pt.project( 2, 180 ), QgsPoint( 1, 0 ) );
  QCOMPARE( pt.project( 5, 270 ), QgsPoint( -4, 2 ) );
  QCOMPARE( pt.project( 6, 360 ), QgsPoint( 1, 8 ) );
  QCOMPARE( pt.project( 5, 450 ), QgsPoint( 6, 2 ) );
  QCOMPARE( pt.project( 5, 450, 450 ), QgsPoint( 6, 2 ) );  // stay QgsWkbTypes::Point
  QCOMPARE( pt.project( -1, 0 ), QgsPoint( 1, 1 ) );
  QCOMPARE( pt.project( 1.5, -90 ), QgsPoint( -0.5, 2 ) );

  // PointZ
  pt.addZValue( 0 );

  QCOMPARE( pt.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 1 ) );
  QCOMPARE( pt.project( 2, 180, 180 ), QgsPoint( QgsWkbTypes::PointZ,  1, 2, -2 ) );
  QCOMPARE( pt.project( 5, 270, 270 ), QgsPoint( QgsWkbTypes::PointZ,  6, 2, 0 ) );
  QCOMPARE( pt.project( 6, 360, 360 ), QgsPoint( QgsWkbTypes::PointZ,  1, 2, 6 ) );
  QCOMPARE( pt.project( -1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, -1 ) );
  QCOMPARE( pt.project( 1.5, -90, -90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 0 ) );

  // PointM
  pt.dropZValue();
  pt.addMValue( 5.0 );

  QCOMPARE( pt.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 3, 0, 5 ) );
  QCOMPARE( pt.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointM, 6, 2, 0, 5 ) );

  // PointZM
  pt.addZValue( 0 );
  QCOMPARE( pt.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 1, 5 ) );

  // 3D
  pt = QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 );

  QCOMPARE( pt.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 3, 2 ) );
  QCOMPARE( pt.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( pt.project( 1.5, 90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );
  QCOMPARE( pt.project( 1.5, 90, 90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );
  QCOMPARE( pt.project( 2, 180 ), QgsPoint( QgsWkbTypes::PointZ, 1, 0, 2 ) );
  QCOMPARE( pt.project( 2, 180, 180 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 ) );
  QCOMPARE( pt.project( 5, 270 ), QgsPoint( QgsWkbTypes::PointZ, -4, 2, 2 ) );
  QCOMPARE( pt.project( 5, 270, 270 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( pt.project( 6, 360 ), QgsPoint( QgsWkbTypes::PointZ, 1, 8, 2 ) );
  QCOMPARE( pt.project( 6, 360, 360 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 8 ) );
  QCOMPARE( pt.project( 5, 450 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( pt.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( pt.project( -1, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2 ) );
  QCOMPARE( pt.project( -1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 1 ) );
  QCOMPARE( pt.project( 1.5, -90 ), QgsPoint( QgsWkbTypes::PointZ, -0.5, 2, 2 ) );
  QCOMPARE( pt.project( 1.5, -90, -90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );

  // PointM
  pt.addMValue( 5.0 );

  QCOMPARE( pt.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 3, 2, 5 ) );
  QCOMPARE( pt.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( pt.project( 5, 450 ), QgsPoint( QgsWkbTypes::PointZM, 6, 2, 2, 5 ) );
  QCOMPARE( pt.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointZM, 6, 2, 2, 5 ) );
}

void TestQgsPoint::closestSegment()
{
  QgsPoint pt( 3.0, 4.0 );
  QgsPoint closest;
  QgsVertexId after;
  int leftOf;

  // return error - points have no segments
  QVERIFY( pt.closestSegment( QgsPoint( 4.0, 6.0 ), closest, after, &leftOf ) < 0 );
  QCOMPARE( leftOf, 0 );
}

void TestQgsPoint::counts()
{
  QgsPoint pt( 2.0, 3.0 );

  QCOMPARE( pt.vertexCount(), 1 );
  QCOMPARE( pt.ringCount(), 1 );
  QCOMPARE( pt.partCount(), 1 );
  QCOMPARE( pt.nCoordinates(), 1 );
}

void TestQgsPoint::measures()
{
  //and other abstract geometry methods

  QgsPoint pt( 2.0, 3.0 );

  QCOMPARE( pt.length(), 0.0 );
  QCOMPARE( pt.perimeter(), 0.0 );
  QCOMPARE( pt.area(), 0.0 );
  QCOMPARE( pt.centroid(), pt );
  QVERIFY( !pt.hasCurvedSegments() );

  std::unique_ptr< QgsPoint >segmented( static_cast< QgsPoint *>( pt.segmentize() ) );
  QCOMPARE( *segmented, pt );
}

void TestQgsPoint::distance()
{
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 2, 2 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 3, 2 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 3, 2 ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 1, 3 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 1, 4 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 1, 4 ), 2.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distance( QgsPoint( 1, -4 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distance( 1, -4 ), 2.0 );

  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 2, 2 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 3, 2 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 3, 2 ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 1, 3 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 1, 4 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 1, 4 ), 4.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distanceSquared( QgsPoint( 1, -4 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distanceSquared( 1, -4 ), 4.0 );
}

void TestQgsPoint::distance3D()
{
  QCOMPARE( QgsPoint( 0, 0 ).distanceSquared3D( QgsPoint( 1, 1 ) ), 2.0 );
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( 1, 1, 0 ) ) );
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ) ) ) );
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( 2, 2, 2 ) ) );
  QVERIFY( std::isnan( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ).distanceSquared3D( QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ).distanceSquared3D( 1, 1, 0 ), 6.0 );
  QVERIFY( std::isnan( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( QgsPoint( 0, 0 ) ) ) );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( 0, 0, 0 ), 12.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ) ), 48.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( 2, 2, 2 ), 48.0 );


  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 3, 2, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( 1, 3, 2 ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 4, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( 1, 1, 4 ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -4, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -2, 0 ).distance3D( 1, 1, -4 ), 2.0 );
}

void TestQgsPoint::segmentLength()
{
  QgsPoint pt( 1, 2 );

  QCOMPARE( pt.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( pt.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( pt.segmentLength( QgsVertexId( -1, 0, 1 ) ), 0.0 );
  QCOMPARE( pt.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( pt.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
}

void TestQgsPoint::azimuth()
{
  QgsPoint pt( 1, 2 );
  QCOMPARE( pt.azimuth( QgsPoint( 1, 2 ) ), 0.0 );
  QCOMPARE( pt.azimuth( QgsPoint( 1, 3 ) ), 0.0 );
  QCOMPARE( pt.azimuth( QgsPoint( 2, 2 ) ), 90.0 );
  QCOMPARE( pt.azimuth( QgsPoint( 1, 0 ) ), 180.0 );
  QCOMPARE( pt.azimuth( QgsPoint( 0, 2 ) ), -90.0 );
}

void TestQgsPoint::inclination()
{
  QCOMPARE( QgsPoint( 1, 2 ).inclination(
              QgsPoint( 1, 2 ) ), 90.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ) ), 90.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 90 ) ), 90.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, -90 ) ), 90.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 0 ) ), 0.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 180 ) ), 180.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, -180 ) ), 180.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 720 ) ), 0.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 45 ) ), 45.0 );

  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination(
              QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 135 ) ), 135.0 );
}

void TestQgsPoint::boundary()
{
  QgsPoint pt( 1.0, 2.0 );
  QVERIFY( !pt.boundary() );
}

void TestQgsPoint::boundingBox()
{
  QgsPoint pt( 1.0, 2.0 );

  QCOMPARE( pt.boundingBox(), QgsRectangle( 1.0, 2.0, 1.0, 2.0 ) );

  //modify points and test that bounding box is updated accordingly
  pt.setX( 3.0 );
  QCOMPARE( pt.boundingBox(), QgsRectangle( 3.0, 2.0, 3.0, 2.0 ) );

  pt.setY( 6.0 );
  QCOMPARE( pt.boundingBox(), QgsRectangle( 3.0, 6.0, 3.0, 6.0 ) );

  pt.rx() = 4.0;
  QCOMPARE( pt.boundingBox(), QgsRectangle( 4.0, 6.0, 4.0, 6.0 ) );

  pt.ry() = 9.0;
  QCOMPARE( pt.boundingBox(), QgsRectangle( 4.0, 9.0, 4.0, 9.0 ) );

  pt.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 11.0, 13.0 ) );
  QCOMPARE( pt.boundingBox(), QgsRectangle( 11.0, 13.0, 11.0, 13.0 ) );

  pt = QgsPoint( 21.0, 23.0 );
  QCOMPARE( pt.boundingBox(), QgsRectangle( 21.0, 23.0, 21.0, 23.0 ) );
}

void TestQgsPoint::boundingBoxIntersects()
{
  QVERIFY( QgsPoint( 1, 2 ).boundingBoxIntersects(
             QgsRectangle( 0, 0.5, 1.5, 3 ) ) );
  QVERIFY( !QgsPoint( 1, 2 ).boundingBoxIntersects(
             QgsRectangle( 3, 0.5, 3.5, 3 ) ) );
  QVERIFY( !QgsPoint().boundingBoxIntersects(
             QgsRectangle( 0, 0.5, 3.5, 3 ) ) );
}

void TestQgsPoint::filterVertices()
{
  QgsPoint pt( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  pt.filterVertices( []( const QgsPoint & )-> bool { return false; } );
  QCOMPARE( pt.x(), 1.1 );
  QCOMPARE( pt.y(), 2.2 );
  QCOMPARE( pt.z(), 3.3 );
  QCOMPARE( pt.m(), 4.4 );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::PointZM );
}

void TestQgsPoint::transformVertices()
{
  QgsPoint pt( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );

  pt.transformVertices( []( const QgsPoint & pt )-> QgsPoint
  {
    return QgsPoint( pt.x() + 2, pt.y() + 3, pt.z() + 1, pt.m() + 8 );
  } );

  QCOMPARE( pt.x(), 3.1 );
  QCOMPARE( pt.y(), 5.2 );
  QCOMPARE( pt.z(), 4.3 );
  QCOMPARE( pt.m(), 12.4 );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::PointZM );

  // no dimensionality change allowed
  pt.transformVertices( []( const QgsPoint & pt )-> QgsPoint
  {
    return QgsPoint( pt.x() + 2, pt.y() + 3 );
  } );

  QCOMPARE( pt.x(), 5.1 );
  QCOMPARE( pt.y(), 8.2 );
  QVERIFY( std::isnan( pt.z() ) );
  QVERIFY( std::isnan( pt.m() ) );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::PointZM );

  pt = QgsPoint( 2, 3 );
  pt.transformVertices( []( const QgsPoint & pt )-> QgsPoint
  {
    return QgsPoint( pt.x() + 2, pt.y() + 3, 7, 8 );
  } );

  QCOMPARE( pt.x(), 4.0 );
  QCOMPARE( pt.y(), 6.0 );
  QVERIFY( std::isnan( pt.z() ) );
  QVERIFY( std::isnan( pt.m() ) );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );
}

void TestQgsPoint::transformWithClass()
{
  QgsPoint pt( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );

  QVERIFY( !pt.transform( nullptr ) );

  TestTransformer transformer;

  QVERIFY( pt.transform( &transformer ) );
  QCOMPARE( pt.x(), 3.3 );
  QCOMPARE( pt.y(), 16.2 );
  QCOMPARE( pt.z(), 8.3 );
  QCOMPARE( pt.m(), 3.4 );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::PointZM );

  TestFailTransformer failTransformer;
  QVERIFY( !pt.transform( &failTransformer ) );
}

void TestQgsPoint::crsTransform()
{
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  QgsPoint pt( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 );
  pt.transform( tr, Qgis::TransformDirection::Forward );

  QGSCOMPARENEAR( pt.x(), 175.771, 0.001 );
  QGSCOMPARENEAR( pt.y(), -39.724, 0.001 );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
  QCOMPARE( pt.m(), 2.0 );

  pt.transform( tr, Qgis::TransformDirection::Reverse );

  QGSCOMPARENEAR( pt.x(), 6374985, 1 );
  QGSCOMPARENEAR( pt.y(), -3626584, 1 );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
  QCOMPARE( pt.m(), 2.0 );

#if PROJ_VERSION_MAJOR<6 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //test with z transform
  pt.transform( tr, Qgis::TransformDirection::Forward, true );
  QGSCOMPARENEAR( pt.z(), -19.249, 0.001 );

  pt.transform( tr, Qgis::TransformDirection::Reverse, true );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
#endif
}

void TestQgsPoint::qTransform()
{
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsPoint pt( QgsWkbTypes::PointZM, 10, 20, 30, 40 );

  pt.transform( qtr );
  QVERIFY( pt == QgsPoint( QgsWkbTypes::PointZM, 20, 60, 30, 40 ) );

  pt.transform( QTransform::fromScale( 1, 1 ), 11, 2, 3, 4 );
  QVERIFY( pt == QgsPoint( QgsWkbTypes::PointZM, 20, 60, 71, 163 ) );
}

void TestQgsPoint::convertTo()
{
  QgsPoint pt( 1.0, 2.0 );

  QVERIFY( pt.convertTo( QgsWkbTypes::Point ) );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );

  QVERIFY( pt.convertTo( QgsWkbTypes::PointZ ) );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::PointZ );

  pt.setZ( 5.0 );

  QVERIFY( pt.convertTo( QgsWkbTypes::Point25D ) );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point25D );
  QCOMPARE( pt.z(), 5.0 );

  QVERIFY( pt.convertTo( QgsWkbTypes::PointZM ) );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::PointZM );
  QCOMPARE( pt.z(), 5.0 );

  pt.setM( 9.0 );

  QVERIFY( pt.convertTo( QgsWkbTypes::PointM ) );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::PointM );
  QVERIFY( std::isnan( pt.z() ) );
  QCOMPARE( pt.m(), 9.0 );

  QVERIFY( pt.convertTo( QgsWkbTypes::Point ) );
  QCOMPARE( pt.wkbType(), QgsWkbTypes::Point );
  QVERIFY( std::isnan( pt.z() ) );
  QVERIFY( std::isnan( pt.m() ) );

  QVERIFY( !pt.convertTo( QgsWkbTypes::Polygon ) );
}

void TestQgsPoint::toCurveType()
{
  QgsPoint pt( QgsWkbTypes::PointZM, 9.0, 3.0, 13.0, 23.0 );

  std::unique_ptr< QgsPoint >clone( pt.toCurveType() );
  QVERIFY( pt == *clone );
}

void TestQgsPoint::toQPointF()
{
  QgsPoint pt( 5.0, 9.0 );

  QPointF result = pt.toQPointF();
  QGSCOMPARENEAR( result.x(), 5.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( result.y(), 9.0, 4 * std::numeric_limits<double>::epsilon() );
}

void TestQgsPoint::toFromWkb()
{
  QgsPoint pt1( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );

  QByteArray wkb = pt1.asWkb();
  QCOMPARE( wkb.size(), pt1.wkbSize() );

  QgsPoint pt2;
  QgsConstWkbPtr wkbPtr( wkb );
  pt2.fromWkb( wkbPtr );

  QVERIFY( pt2 == pt1 );

  //bad WKB - check for no crash
  pt2 = QgsPoint( 1, 2 );
  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !pt2.fromWkb( nullPtr ) );
  QCOMPARE( pt2.wkbType(), QgsWkbTypes::Point );

  QgsLineString ls;
  pt2 = QgsPoint( 1, 2 );
  QByteArray wkbLine = ls.asWkb();

  QCOMPARE( wkbLine.size(), ls.wkbSize() );

  QgsConstWkbPtr wkbLinePtr( wkbLine );
  QVERIFY( !pt2.fromWkb( wkbLinePtr ) );
  QCOMPARE( pt2.wkbType(), QgsWkbTypes::Point );
}

void TestQgsPoint::toFromWkt()
{
  QgsPoint pt1( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );

  QString wkt = pt1.asWkt();
  QVERIFY( !wkt.isEmpty() );

  QgsPoint pt2;

  QVERIFY( pt2.fromWkt( wkt ) );
  QVERIFY( pt2 == pt1 );

  QVERIFY( pt2.fromWkt( QStringLiteral( "Point(1 2 3)" ) ) );
  QVERIFY( pt2 == QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  QVERIFY( pt2.fromWkt( QStringLiteral( "Point(1 2 3 4)" ) ) );
  QVERIFY( pt2 == QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  //bad WKT
  QVERIFY( !pt2.fromWkt( "Polygon()" ) );
  QVERIFY( !pt2.fromWkt( "Point(1 )" ) );

  // with rounding
  QCOMPARE( QgsPoint( 12345.678, 12345.678 ).asWkt( 1 ), QStringLiteral( "Point (12345.7 12345.7)" ) );
  QCOMPARE( QgsPoint( 12345.678, 12345.678 ).asWkt( 2 ), QStringLiteral( "Point (12345.68 12345.68)" ) );
  QCOMPARE( QgsPoint( 12345.678, 12345.678 ).asWkt( 0 ), QStringLiteral( "Point (12346 12346)" ) );
  QCOMPARE( QgsPoint( 12345.678, 12345.678 ).asWkt( -1 ), QStringLiteral( "Point (12350 12350)" ) );
  QCOMPARE( QgsPoint( 12345.678, 12345.678 ).asWkt( -2 ), QStringLiteral( "Point (12300 12300)" ) );
  QCOMPARE( QgsPoint( 12345.678, 12345.678 ).asWkt( -3 ), QStringLiteral( "Point (12000 12000)" ) );
}

void TestQgsPoint::exportImport()
{
  //asGML2
  QgsPoint exportPoint( 1, 2 );
  QgsPoint exportPointFloat( 1 / 3.0, 2 / 3.0 );
  QDomDocument doc( QStringLiteral( "gml" ) );

  QString expectedGML2( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,2</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPoint.asGml2( doc ) ), expectedGML2 );

  QString expectedGML2prec3( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1 2</pos></Point>" ) );
  QCOMPARE( elemToString( exportPoint.asGml3( doc ) ), expectedGML3 );

  QString expectedGML3prec3( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0.333 0.667</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );

  QgsPoint exportPointZ( 1, 2, 3 );

  QString expectedGML3Z( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"3\">1 2 3</pos></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointZ.asGml3( doc, 3 ) ), expectedGML3Z );

  //asGML2 inverted axis
  QgsPoint exportPointInvertedAxis( 1, 2 );
  QgsPoint exportPointFloatInvertedAxis( 1 / 3.0, 2 / 3.0 );
  QDomDocument docInvertedAxis( QStringLiteral( "gml" ) );

  QString expectedGML2InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2,1</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointInvertedAxis.asGml2( docInvertedAxis, 17, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML2InvertedAxis );

  QString expectedGML2prec3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.333</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointFloatInvertedAxis.asGml2( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML2prec3InvertedAxis );

  //asGML3 inverted axis
  QString expectedGML3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">2 1</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointInvertedAxis.asGml3( docInvertedAxis, 17, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3InvertedAxis );

  QString expectedGML3prec3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0.667 0.333</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointFloatInvertedAxis.asGml3( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3prec3InvertedAxis );

  QgsPoint exportPointZInvertedAxis( 1, 2, 3 );

  QString expectedGML3ZInvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"3\">2 1 3</pos></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointZInvertedAxis.asGml3( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3ZInvertedAxis );


  //asJSON
  QString expectedJson( QStringLiteral( "{\"coordinates\":[1.0,2.0,3.0],\"type\":\"Point\"}" ) );
  QCOMPARE( exportPointZ.asJson(), expectedJson );

  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[0.333,0.667],\"type\":\"Point\"}" ) );
  QCOMPARE( exportPointFloat.asJson( 3 ), expectedJsonPrec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<Point><coordinates>1,2</coordinates></Point>" ) );
  QCOMPARE( exportPoint.asKml(), expectedKml );

  QString expectedKmlPrec3( QStringLiteral( "<Point><coordinates>0.333,0.667</coordinates></Point>" ) );
  QCOMPARE( exportPointFloat.asKml( 3 ), expectedKmlPrec3 );
}

QGSTEST_MAIN( TestQgsPoint )
#include "testqgspoint.moc"




