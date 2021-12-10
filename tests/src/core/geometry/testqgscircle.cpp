/***************************************************************************
     testqgscircle.cpp
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

#include "qgscircle.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspoint.h"

#include "testgeometryutils.h"

class TestQgsCircle: public QObject
{
    Q_OBJECT
  private slots:
    void constructor();
    void from2Points();
    void fromExtent();
    void from3Points();
    void fromCenterDiameter();
    void fromCenterPoint();
    void from3Tangents();
    void from3TangentsWithParallels();
    void from3tangentsMulti();
    void minimalCircleFrom3points();
    void setterGetter();
    void equality();
    void areaPerimeter();
    void boundingBox();
    void northQuadrant();
    void contains();
    void intersections();
    void tangentToPoint();
    void outerTangents();
    void innerTangents();
    void toPolygon();
    void toPolygonoOriented();
    void toCircularString();
    void toString();
    void asGML();
};

void TestQgsCircle::constructor()
{
  QgsCircle circ;
  QVERIFY( circ.center().isEmpty() );
  QCOMPARE( circ.radius(), 0.0 );
  QCOMPARE( circ.azimuth(), 0.0 );
  QVERIFY( circ.isEmpty() );

  circ = QgsCircle( QgsPoint( 5, 10 ), 3 );
  QVERIFY( circ.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( circ.radius(), 3.0 );
  QCOMPARE( circ.azimuth(), 0.0 );
  QVERIFY( !circ.isEmpty() );

  circ = QgsCircle( QgsPoint( 5, 10 ), 3, 45 );
  QVERIFY( circ.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( circ.radius(), 3.0 );
  QCOMPARE( circ.azimuth(), 45.0 );
  QVERIFY( !circ.isEmpty() );
}

void TestQgsCircle::from2Points()
{
  QVERIFY( QgsCircle().from2Points( QgsPoint( -5, 0 ), QgsPoint( 5, 0 ) )
           == QgsCircle( QgsPoint( 0, 0 ), 5, 90 ) );
  QVERIFY( QgsCircle().from2Points( QgsPoint( 0, -5 ), QgsPoint( 0, 5 ) )
           == QgsCircle( QgsPoint( 0, 0 ), 5, 0 ) );
}

void TestQgsCircle::fromExtent()
{
  QVERIFY( QgsCircle().fromExtent( QgsPoint( -5, -5 ), QgsPoint( 5, 5 ) )
           == QgsCircle( QgsPoint( 0, 0 ), 5, 0 ) );
  QVERIFY( QgsCircle().fromExtent( QgsPoint( -7.5, -2.5 ), QgsPoint( 2.5, 200.5 ) ).isEmpty() );
}

void TestQgsCircle::from3Points()
{
  QVERIFY( QgsCircle().from3Points( QgsPoint( -5, 0 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) )
           == QgsCircle( QgsPoint( 0, 0 ), 5 ) );
  QVERIFY( QgsCircle().from3Points( QgsPoint( 5, 0 ), QgsPoint( 6, 0 ), QgsPoint( 7, 0 ) ).isEmpty() );
}

void TestQgsCircle::fromCenterDiameter()
{
  QVERIFY( QgsCircle().fromCenterDiameter( QgsPoint( 0, 0 ), 10 )
           == QgsCircle( QgsPoint( 0, 0 ), 5, 0 ) );
  QVERIFY( QgsCircle().fromCenterDiameter( QgsPoint( 2, 100 ), -10 )
           == QgsCircle( QgsPoint( 2, 100 ), 5, 0 ) );
  QVERIFY( QgsCircle().fromCenterDiameter( QgsPoint( 2, 100 ), -10, 45 )
           == QgsCircle( QgsPoint( 2, 100 ), 5, 45 ) );
}

void TestQgsCircle::fromCenterPoint()
{
  QVERIFY( QgsCircle().fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ) )
           == QgsCircle( QgsPoint( 0, 0 ), 5, 90 ) );
  QVERIFY( QgsCircle().fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ) )
           == QgsCircle( QgsPoint( 0, 0 ), 5, 0 ) );
  QVERIFY( QgsCircle().fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 5 * std::cos( 45 * M_PI / 180.0 ), 5 * std::sin( 45 * M_PI / 180.0 ) ) )
           == QgsCircle( QgsPoint( 0, 0 ), 5, 45 ) );
}

void TestQgsCircle::from3Tangents()
{
  // Tangents from circle tri1( 0,0 ; 0,5 ), tri2( 0,0 ; 5,0 ), tri3( 5,0 ; 0,5 )
  QgsCircle circ = QgsCircle::from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 0, 1 ),
                   QgsPoint( 2, 0 ), QgsPoint( 3, 0 ),
                   QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 1.4645, 1.4645 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 1.4645, 0.0001 );
}

void TestQgsCircle::from3TangentsWithParallels()
{
  // with parallels
  QgsCircle circ = QgsCircle::from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                   QgsPoint( 1, 0 ), QgsPoint( 1, 5 ),
                   QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QVERIFY( circ.isEmpty() );

  circ = QgsCircle::from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                                   QgsPoint( 5, 0 ), QgsPoint( 0, 5 ),
                                   QgsPoint( 1, 0 ), QgsPoint( 1, 5 ) );
  QVERIFY( circ.isEmpty() );

  circ = QgsCircle::from3Tangents( QgsPoint( 5, 0 ), QgsPoint( 0, 5 ),
                                   QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                                   QgsPoint( 1, 0 ), QgsPoint( 1, 5 ) );
  QVERIFY( circ.isEmpty() );

  // with 2 parallels
  const double epsilon = 1e-8;
  circ = QgsCircle::from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ),
                                   QgsPoint( 5, 5 ), QgsPoint( 10, 5 ),
                                   QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ) );
  QVERIFY( circ.isEmpty() );

  circ = QgsCircle::from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ),
                                   QgsPoint( 5, 5 ), QgsPoint( 10, 5 ),
                                   QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ),
                                   epsilon, QgsPoint( 2, 0 ) );

  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 1.4645, 2.5000 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 2.5, 0.0001 );

  circ = QgsCircle::from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ),
                                   QgsPoint( 5, 5 ), QgsPoint( 10, 5 ),
                                   QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ),
                                   epsilon, QgsPoint( 3, 0 ) );

  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 8.5355, 2.5000 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 2.5, 0.0001 );
}

void TestQgsCircle::from3tangentsMulti()
{
  const double epsilon = 1e-8;

  QVector<QgsCircle> circles;
  QgsCircle circ;

  circles = QgsCircle::from3TangentsMulti( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ),
            QgsPoint( 5, 5 ), QgsPoint( 10, 5 ),
            QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ) );
  QCOMPARE( circles.count(), 2 );

  circ = circles.at( 0 );
  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 8.5355, 2.5000 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 2.5, 0.0001 );

  circ = circles.at( 1 );
  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 1.4645, 2.5000 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 2.5, 0.0001 );

  circles = QgsCircle::from3TangentsMulti( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ),
            QgsPoint( 5, 5 ), QgsPoint( 10, 5 ),
            QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ),
            epsilon, QgsPoint( 2, 0 ) );
  QCOMPARE( circles.count(), 1 );

  circ = circles.at( 0 );
  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 1.4645, 2.5000 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 2.5, 0.0001 );

  circles = QgsCircle::from3TangentsMulti( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ),
            QgsPoint( 5, 5 ), QgsPoint( 10, 5 ),
            QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ),
            epsilon, QgsPoint( 3, 0 ) );
  QCOMPARE( circles.count(), 1 );

  circles = QgsCircle::from3TangentsMulti( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ),
            QgsPoint( 5, 5 ), QgsPoint( 10, 5 ),
            QgsPoint( 15, 5 ), QgsPoint( 20, 5 ) );
  QVERIFY( circles.isEmpty() );

  // check that Z dimension is ignored in case of using tangents
  circ = QgsCircle::from3Tangents( QgsPoint( 0, 0, 333 ), QgsPoint( 0, 1, 1 ),
                                   QgsPoint( 2, 0, 2 ), QgsPoint( 3, 0, 3 ),
                                   QgsPoint( 5, 0, 4 ), QgsPoint( 0, 5, 5 ) );
  QCOMPARE( circ.center().is3D(), false );
}

void TestQgsCircle::minimalCircleFrom3points()
{
  // equivalent to from2Points
  QgsCircle circ = QgsCircle::minimalCircleFrom3Points( QgsPoint( 0, 5 ), QgsPoint( 0, -5 ), QgsPoint( 1, 2 ) );

  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 5.0, 0.0001 );
  QCOMPARE( circ,  QgsCircle::from2Points( QgsPoint( 0, 5 ), QgsPoint( 0, -5 ) ) );

  circ = QgsCircle::minimalCircleFrom3Points( QgsPoint( 0, -5 ), QgsPoint( 1, 2 ), QgsPoint( 0, 5 ) );

  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 5.0, 0.0001 );
  QCOMPARE( circ,  QgsCircle().from2Points( QgsPoint( 0, 5 ), QgsPoint( 0, -5 ) ) );

  circ = QgsCircle::minimalCircleFrom3Points( QgsPoint( 1, 2 ), QgsPoint( 0, 5 ), QgsPoint( 0, -5 ) );

  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 5.0, 0.0001 );
  QCOMPARE( circ,  QgsCircle().from2Points( QgsPoint( 0, 5 ), QgsPoint( 0, -5 ) ) );

  // equivalent to from3Points
  circ = QgsCircle::minimalCircleFrom3Points( QgsPoint( 0, 5 ), QgsPoint( 5, 0 ), QgsPoint( -5, 0 ) );

  QGSCOMPARENEARPOINT( circ.center(), QgsPoint( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( circ.radius(), 5.0, 0.0001 );
}

void TestQgsCircle::setterGetter()
{
  QgsCircle circ;
  circ.setAzimuth( 45 );
  QCOMPARE( circ.azimuth(), 45.0 );

  circ.setRadius( 50 );
  QCOMPARE( circ.radius(), 50.0 );
  QCOMPARE( circ.semiMajorAxis(), 50.0 );
  QCOMPARE( circ.semiMinorAxis(), 50.0 );

  circ.setSemiMajorAxis( 250 );
  QCOMPARE( circ.radius(), 250.0 );
  QCOMPARE( circ.semiMajorAxis(), 250.0 );
  QCOMPARE( circ.semiMinorAxis(), 250.0 );

  circ.setSemiMinorAxis( 8250 );
  QCOMPARE( circ.radius(), 8250.0 );
  QCOMPARE( circ.semiMajorAxis(), 8250.0 );
  QCOMPARE( circ.semiMinorAxis(), 8250.0 );

  circ.setCenter( QgsPoint( 5, 10 ) );
  QVERIFY( circ.center() == QgsPoint( 5, 10 ) );
  QVERIFY( circ.rcenter() == QgsPoint( 5, 10 ) );
  circ.rcenter() = QgsPoint( 25, 310 );
  QVERIFY( circ.center() == QgsPoint( 25, 310 ) );
}

void TestQgsCircle::equality()
{
  QCOMPARE( QgsCircle().isEmpty(), QgsCircle( QgsPoint(), 0, 0 ).isEmpty() );
  QVERIFY( !( QgsCircle() == QgsCircle( QgsPoint( 0, 0 ), 0, 0.0005 ) ) );

  QVERIFY( QgsCircle( QgsPoint( 5, 10 ), 3 ) == QgsCircle( QgsPoint( 5, 10 ), 3, 0 ) );
  QVERIFY( QgsCircle( QgsPoint( 5, 10 ), 3 ) != QgsCircle( QgsPoint( 5, 10 ), 3, 45 ) );
}

void TestQgsCircle::areaPerimeter()
{
  QGSCOMPARENEAR( 314.1593, QgsCircle( QgsPoint( 0, 0 ), 10 ).area(), 0.0001 );
  QGSCOMPARENEAR( 31.4159, QgsCircle( QgsPoint( 0, 0 ), 5 ).perimeter(), 0.0001 );
}

void TestQgsCircle::boundingBox()
{
  QVERIFY( QgsRectangle( QgsPointXY( -2.5, -2.5 ), QgsPointXY( 2.5, 2.5 ) )
           == QgsCircle( QgsPoint( 0, 0 ), 2.5, 0 ).boundingBox() );
  QVERIFY( QgsRectangle( QgsPointXY( -2.5, -2.5 ), QgsPointXY( 2.5, 2.5 ) )
           == QgsCircle( QgsPoint( 0, 0 ), 2.5, 45 ).boundingBox() );
}

void TestQgsCircle::northQuadrant()
{
  QgsPointSequence pts = QgsCircle( QgsPoint( 0, 0 ), 5 ).northQuadrant();

  QVERIFY( pts.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( -5, 0 ) );

  pts = QgsCircle( QgsPoint( 0, 0 ), 5, 123 ).northQuadrant();

  QVERIFY( pts.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( -5, 0 ) );

  pts = QgsCircle( QgsPoint( 0, 0 ), 5, 456 ).northQuadrant();

  QVERIFY( pts.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( -5, 0 ) );

  pts = QgsCircle( QgsPoint( 0, 0 ), 5, -789l ).northQuadrant();

  QVERIFY( pts.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( -5, 0 ) );
}

void TestQgsCircle::contains()
{
  QgsPoint pt;
  pt = QgsPoint( 1, 1 );
  QVERIFY( QgsCircle( QgsPoint( 0, 0 ), 5 ).contains( pt ) );
  pt = QgsPoint( 0, 5 );
  QVERIFY( QgsCircle( QgsPoint( 0, 0 ), 5 ).contains( pt ) );
  pt = QgsPoint( 6, 1 );
  QVERIFY( !QgsCircle( QgsPoint( 0, 0 ), 5 ).contains( pt ) );
}

void TestQgsCircle::intersections()
{
  QgsCircle circ( QgsPoint( 0, 0 ), 1 );
  QgsPoint int1;
  QgsPoint int2;

  QCOMPARE( circ.intersections( QgsCircle( QgsPoint( 2, 0 ), 0.5 ), int1, int2 ), 0 );
  QCOMPARE( circ.intersections( QgsCircle( QgsPoint( 0.5, 0.1 ), 0.2 ), int1, int2 ), 0 );

  // one intersection
  QCOMPARE( circ.intersections( QgsCircle( QgsPoint( 3, 0 ), 2 ), int1, int2 ), 1 );
  QCOMPARE( int1, QgsPoint( 1, 0 ) );
  QCOMPARE( int2, QgsPoint( 1, 0 ) );

  // two intersections
  circ = QgsCircle( QgsPoint( 5, 3 ), 2 );

  QCOMPARE( circ.intersections( QgsCircle( QgsPoint( 7, -1 ), 4 ), int1, int2 ), 2 );

  QCOMPARE( int1.wkbType(), QgsWkbTypes::Point );
  QGSCOMPARENEAR( int1.x(), 3.8, 0.001 );
  QGSCOMPARENEAR( int1.y(), 1.4, 0.001 );

  QCOMPARE( int2.wkbType(), QgsWkbTypes::Point );
  QGSCOMPARENEAR( int2.x(), 7.0, 0.001 );
  QGSCOMPARENEAR( int2.y(), 3.0, 0.001 );

  // with z
  circ = QgsCircle( QgsPoint( 5, 3, 11 ), 2 );

  QCOMPARE( circ.intersections( QgsCircle( QgsPoint( 7, -1, 5 ), 4 ), int1, int2, true ), 0 );
  QCOMPARE( circ.intersections( QgsCircle( QgsPoint( 7, -1, 11 ), 4 ), int1, int2, true ), 2 );

  QCOMPARE( int1.wkbType(), QgsWkbTypes::PointZ );
  QGSCOMPARENEAR( int1.x(), 3.8, 0.001 );
  QGSCOMPARENEAR( int1.y(), 1.4, 0.001 );
  QGSCOMPARENEAR( int1.z(), 11.0, 0.001 );

  QCOMPARE( int2.wkbType(), QgsWkbTypes::PointZ );
  QGSCOMPARENEAR( int2.x(), 7.0, 0.001 );
  QGSCOMPARENEAR( int2.y(), 3.0, 0.001 );
  QGSCOMPARENEAR( int2.z(), 11.0, 0.001 );
}

void TestQgsCircle::tangentToPoint()
{
  QgsPointXY tan1;
  QgsPointXY tan2;

  QVERIFY( !QgsCircle( QgsPoint( 1, 2 ), 4 ).tangentToPoint( QgsPointXY( 1, 2 ), tan1, tan2 ) );
  QVERIFY( QgsCircle( QgsPoint( 1, 2 ), 4 ).tangentToPoint( QgsPointXY( 8, 4 ), tan1, tan2 ) );

  QGSCOMPARENEAR( tan1.x(), 4.03, 0.01 );
  QGSCOMPARENEAR( tan1.y(), -0.61, 0.01 );
  QGSCOMPARENEAR( tan2.x(), 2.2, 0.01 );
  QGSCOMPARENEAR( tan2.y(), 5.82, 0.01 );
}

void TestQgsCircle::outerTangents()
{
  QgsPointXY l1p1, l1p2, l2p1, l2p2;

  QCOMPARE( QgsCircle( QgsPoint( 1, 2 ), 4 ).outerTangents( QgsCircle( QgsPoint( 2, 3 ), 1 ), l1p1, l1p2, l2p1, l2p2 ), 0 );
  QCOMPARE( QgsCircle( QgsPoint( 1, 2 ), 1 ).outerTangents( QgsCircle( QgsPoint( 10, 3 ), 4 ), l1p1, l1p2, l2p1, l2p2 ), 2 );

  QGSCOMPARENEAR( l1p1.x(), 0.566, 0.01 );
  QGSCOMPARENEAR( l1p1.y(), 2.901, 0.01 );
  QGSCOMPARENEAR( l1p2.x(), 8.266, 0.01 );
  QGSCOMPARENEAR( l1p2.y(), 6.604, 0.01 );
  QGSCOMPARENEAR( l2p1.x(), 0.7749, 0.01 );
  QGSCOMPARENEAR( l2p1.y(), 1.025, 0.01 );
  QGSCOMPARENEAR( l2p2.x(), 9.099, 0.01 );
  QGSCOMPARENEAR( l2p2.y(), -0.897, 0.01 );
}

void TestQgsCircle::innerTangents()
{
  QgsPointXY l1p1, l1p2, l2p1, l2p2;

  QCOMPARE( QgsCircle( QgsPoint( 1, 2 ), 4 ).innerTangents( QgsCircle( QgsPoint( 2, 3 ), 1 ), l1p1, l1p2, l2p1, l2p2 ), 0 );
  QCOMPARE( QgsCircle( QgsPoint( 0, 0 ), 4 ).innerTangents( QgsCircle( QgsPoint( 8, 0 ), 5 ), l1p1, l1p2, l2p1, l2p2 ), 0 );
  QCOMPARE( QgsCircle( QgsPoint( 0, 0 ), 4 ).innerTangents( QgsCircle( QgsPoint( 8, 0 ), 4 ), l1p1, l1p2, l2p1, l2p2 ), 0 );
  QCOMPARE( QgsCircle( QgsPoint( 1, 2 ), 1 ).innerTangents( QgsCircle( QgsPoint( 10, 3 ), 4 ), l1p1, l1p2, l2p1, l2p2 ), 2 );

  QGSCOMPARENEAR( l1p1.x(), 7.437, 0.01 );
  QGSCOMPARENEAR( l1p1.y(), 6.071, 0.01 );
  QGSCOMPARENEAR( l1p2.x(), 1.641, 0.01 );
  QGSCOMPARENEAR( l1p2.y(), 1.232, 0.01 );
  QGSCOMPARENEAR( l2p1.x(), 8.173, 0.01 );
  QGSCOMPARENEAR( l2p1.y(), -0.558, 0.01 );
  QGSCOMPARENEAR( l2p2.x(), 1.457, 0.01 );
  QGSCOMPARENEAR( l2p2.y(), 2.89, 0.01 );

}

void TestQgsCircle::toPolygon()
{
  QgsPointSequence pts;
  std::unique_ptr< QgsPolygon > pol( new QgsPolygon() );

  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5 ).toPolygon( 4 ) );

  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );

  pol->exteriorRing()->points( pts );

  QCOMPARE( pts.length(), 5 );
  QVERIFY( pts.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( -5, 0 ) );
  QVERIFY( pts.at( 4 ) == QgsPoint( 0, 5 ) );
}

void TestQgsCircle::toPolygonoOriented()
{
  QgsPointSequence pts;
  std::unique_ptr< QgsPolygon > pol( new QgsPolygon() );

  double val = 5 * std::sin( M_PI / 4 );

  //45
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 45 ).toPolygon( 4 ) );

  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );

  pol->exteriorRing()->points( pts );

  QCOMPARE( pts.length(), 5 );
  QVERIFY( pts.at( 0 ) == QgsPoint( val, val ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( val, -val ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( -val, -val ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( -val, val ) );
  QVERIFY( pts.at( 4 ) == QgsPoint( val, val ) );

  //135
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 135 ).toPolygon( 4 ) );

  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );

  pol->exteriorRing()->points( pts );

  QCOMPARE( pts.length(), 5 );
  QVERIFY( pts.at( 0 ) == QgsPoint( val, -val ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( -val, -val ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( -val, val ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( val, val ) );
  QVERIFY( pts.at( 4 ) == QgsPoint( val, -val ) );

  //225
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 225 ).toPolygon( 4 ) );

  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );

  pol->exteriorRing()->points( pts );

  QCOMPARE( pts.length(), 5 );
  QVERIFY( pts.at( 0 ) == QgsPoint( -val, -val ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( -val, val ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( val, val ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( val, -val ) );
  QVERIFY( pts.at( 4 ) == QgsPoint( -val, -val ) );

  //315
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 315 ).toPolygon( 4 ) );

  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );

  pol->exteriorRing()->points( pts );

  QCOMPARE( pts.length(), 5 );
  QVERIFY( pts.at( 0 ) == QgsPoint( -val, val ) );
  QVERIFY( pts.at( 1 ) == QgsPoint( val, val ) );
  QVERIFY( pts.at( 2 ) == QgsPoint( val, -val ) );
  QVERIFY( pts.at( 3 ) == QgsPoint( -val, -val ) );
  QVERIFY( pts.at( 4 ) == QgsPoint( -val, val ) );
}

void TestQgsCircle::toCircularString()
{
  std::unique_ptr< QgsCircularString > cs( QgsCircle( QgsPoint( 0, 0 ), 5 ).toCircularString() );
  QCOMPARE( cs->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 0, 0 5)" ) );

  cs.reset( QgsCircle( QgsPoint( 0, 0 ), 5 ).toCircularString( true ) );
  QCOMPARE( cs->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 0, 0 5)" ) );

  cs.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 315 ).toCircularString() );
  QCOMPARE( cs->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 0, 0 5)" ) );

  cs.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 315 ).toCircularString( true ) );
  QCOMPARE( cs->asWkt( 2 ), QString( "CircularString (-3.54 3.54, 3.54 3.54, 3.54 -3.54, -3.54 -3.54, -3.54 3.54)" ) );
}

void TestQgsCircle::toString()
{
  QgsCircle circ;
  QCOMPARE( circ.toString(), QString( "Empty" ) );

  circ = QgsCircle( QgsPoint( 5, 10 ), 3 );
  QCOMPARE( circ.toString(), QString( "Circle (Center: Point (5 10), Radius: 3, Azimuth: 0)" ) );

  circ = QgsCircle( QgsPoint( 5, 10 ), 3, 45 );
  QCOMPARE( circ.toString(), QString( "Circle (Center: Point (5 10), Radius: 3, Azimuth: 45)" ) );
}

void TestQgsCircle::asGML()
{
  // asGML3
  QDomDocument doc( "gml" );

  QgsCircle exportCircle;
  QString expectedGML3( QStringLiteral( "<Circle xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( exportCircle.asGml3( doc ) ), expectedGML3 );

  exportCircle = QgsCircle( QgsPoint( 1, 1 ), 3 );
  expectedGML3 = QString( QStringLiteral( "<Circle xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 4 4 1 1 -2</posList></Circle>" ) );
  QGSCOMPAREGML( elemToString( exportCircle.asGml3( doc ) ), expectedGML3 );

  // asGML2
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,4 1.05,4 1.1,4 1.16,4 1.21,3.99 1.26,3.99 1.31,3.98 1.37,3.98 1.42,3.97 1.47,3.96 1.52,3.95 1.57,3.94 1.62,3.93 1.67,3.92 1.73,3.91 1.78,3.9 1.83,3.88 1.88,3.87 1.93,3.85 1.98,3.84 2.03,3.82 2.08,3.8 2.12,3.78 2.17,3.76 2.22,3.74 2.27,3.72 2.32,3.7 2.36,3.67 2.41,3.65 2.45,3.62 2.5,3.6 2.55,3.57 2.59,3.54 2.63,3.52 2.68,3.49 2.72,3.46 2.76,3.43 2.81,3.4 2.85,3.36 2.89,3.33 2.93,3.3 2.97,3.26 3.01,3.23 3.05,3.19 3.08,3.16 3.12,3.12 3.16,3.08 3.19,3.05 3.23,3.01 3.26,2.97 3.3,2.93 3.33,2.89 3.36,2.85 3.4,2.81 3.43,2.76 3.46,2.72 3.49,2.68 3.52,2.63 3.54,2.59 3.57,2.55 3.6,2.5 3.62,2.45 3.65,2.41 3.67,2.36 3.7,2.32 3.72,2.27 3.74,2.22 3.76,2.17 3.78,2.12 3.8,2.08 3.82,2.03 3.84,1.98 3.85,1.93 3.87,1.88 3.88,1.83 3.9,1.78 3.91,1.73 3.92,1.67 3.93,1.62 3.94,1.57 3.95,1.52 3.96,1.47 3.97,1.42 3.98,1.37 3.98,1.31 3.99,1.26 3.99,1.21 4,1.16 4,1.1 4,1.05 4,1 4,0.95 4,0.9 4,0.84 3.99,0.79 3.99,0.74 3.98,0.69 3.98,0.63 3.97,0.58 3.96,0.53 3.95,0.48 3.94,0.43 3.93,0.38 3.92,0.33 3.91,0.27 3.9,0.22 3.88,0.17 3.87,0.12 3.85,0.07 3.84,0.02 3.82,-0.03 3.8,-0.08 3.78,-0.12 3.76,-0.17 3.74,-0.22 3.72,-0.27 3.7,-0.32 3.67,-0.36 3.65,-0.41 3.62,-0.45 3.6,-0.5 3.57,-0.55 3.54,-0.59 3.52,-0.63 3.49,-0.68 3.46,-0.72 3.43,-0.76 3.4,-0.81 3.36,-0.85 3.33,-0.89 3.3,-0.93 3.26,-0.97 3.23,-1.01 3.19,-1.05 3.16,-1.08 3.12,-1.12 3.08,-1.16 3.05,-1.19 3.01,-1.23 2.97,-1.26 2.93,-1.3 2.89,-1.33 2.85,-1.36 2.81,-1.4 2.76,-1.43 2.72,-1.46 2.68,-1.49 2.63,-1.52 2.59,-1.54 2.55,-1.57 2.5,-1.6 2.45,-1.62 2.41,-1.65 2.36,-1.67 2.32,-1.7 2.27,-1.72 2.22,-1.74 2.17,-1.76 2.12,-1.78 2.08,-1.8 2.03,-1.82 1.98,-1.84 1.93,-1.85 1.88,-1.87 1.83,-1.88 1.78,-1.9 1.73,-1.91 1.67,-1.92 1.62,-1.93 1.57,-1.94 1.52,-1.95 1.47,-1.96 1.42,-1.97 1.37,-1.98 1.31,-1.98 1.26,-1.99 1.21,-1.99 1.16,-2 1.1,-2 1.05,-2 1,-2 0.95,-2 0.9,-2 0.84,-2 0.79,-1.99 0.74,-1.99 0.69,-1.98 0.63,-1.98 0.58,-1.97 0.53,-1.96 0.48,-1.95 0.43,-1.94 0.38,-1.93 0.33,-1.92 0.27,-1.91 0.22,-1.9 0.17,-1.88 0.12,-1.87 0.07,-1.85 0.02,-1.84 -0.03,-1.82 -0.08,-1.8 -0.12,-1.78 -0.17,-1.76 -0.22,-1.74 -0.27,-1.72 -0.32,-1.7 -0.36,-1.67 -0.41,-1.65 -0.45,-1.62 -0.5,-1.6 -0.55,-1.57 -0.59,-1.54 -0.63,-1.52 -0.68,-1.49 -0.72,-1.46 -0.76,-1.43 -0.81,-1.4 -0.85,-1.36 -0.89,-1.33 -0.93,-1.3 -0.97,-1.26 -1.01,-1.23 -1.05,-1.19 -1.08,-1.16 -1.12,-1.12 -1.16,-1.08 -1.19,-1.05 -1.23,-1.01 -1.26,-0.97 -1.3,-0.93 -1.33,-0.89 -1.36,-0.85 -1.4,-0.81 -1.43,-0.76 -1.46,-0.72 -1.49,-0.68 -1.52,-0.63 -1.54,-0.59 -1.57,-0.55 -1.6,-0.5 -1.62,-0.45 -1.65,-0.41 -1.67,-0.36 -1.7,-0.32 -1.72,-0.27 -1.74,-0.22 -1.76,-0.17 -1.78,-0.12 -1.8,-0.08 -1.82,-0.03 -1.84,0.02 -1.85,0.07 -1.87,0.12 -1.88,0.17 -1.9,0.22 -1.91,0.27 -1.92,0.33 -1.93,0.38 -1.94,0.43 -1.95,0.48 -1.96,0.53 -1.97,0.58 -1.98,0.63 -1.98,0.69 -1.99,0.74 -1.99,0.79 -2,0.84 -2,0.9 -2,0.95 -2,1 -2,1.05 -2,1.1 -2,1.16 -1.99,1.21 -1.99,1.26 -1.98,1.31 -1.98,1.37 -1.97,1.42 -1.96,1.47 -1.95,1.52 -1.94,1.57 -1.93,1.62 -1.92,1.67 -1.91,1.73 -1.9,1.78 -1.88,1.83 -1.87,1.88 -1.85,1.93 -1.84,1.98 -1.82,2.03 -1.8,2.08 -1.78,2.12 -1.76,2.17 -1.74,2.22 -1.72,2.27 -1.7,2.32 -1.67,2.36 -1.65,2.41 -1.62,2.45 -1.6,2.5 -1.57,2.55 -1.54,2.59 -1.52,2.63 -1.49,2.68 -1.46,2.72 -1.43,2.76 -1.4,2.81 -1.36,2.85 -1.33,2.89 -1.3,2.93 -1.26,2.97 -1.23,3.01 -1.19,3.05 -1.16,3.08 -1.12,3.12 -1.08,3.16 -1.05,3.19 -1.01,3.23 -0.97,3.26 -0.93,3.3 -0.89,3.33 -0.85,3.36 -0.81,3.4 -0.76,3.43 -0.72,3.46 -0.68,3.49 -0.63,3.52 -0.59,3.54 -0.55,3.57 -0.5,3.6 -0.45,3.62 -0.41,3.65 -0.36,3.67 -0.32,3.7 -0.27,3.72 -0.22,3.74 -0.17,3.76 -0.12,3.78 -0.08,3.8 -0.03,3.82 0.02,3.84 0.07,3.85 0.12,3.87 0.17,3.88 0.22,3.9 0.27,3.91 0.33,3.92 0.38,3.93 0.43,3.94 0.48,3.95 0.53,3.96 0.58,3.97 0.63,3.98 0.69,3.98 0.74,3.99 0.79,3.99 0.84,4 0.9,4 0.95,4 1,4</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportCircle.asGml2( doc, 2 ) ), expectedGML2 );
}

QGSTEST_MAIN( TestQgsCircle )
#include "testqgscircle.moc"
