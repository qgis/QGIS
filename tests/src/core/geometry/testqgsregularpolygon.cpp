/***************************************************************************
     testqgsregularpolygon.cpp
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

#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsregularpolygon.h"

#include "testgeometryutils.h"

class TestQgsRegularPolygon: public QObject
{
    Q_OBJECT
  private slots:
    void regularPolygon();
};

void TestQgsRegularPolygon::regularPolygon()
{
  // constructors
  QgsRegularPolygon rp1 = QgsRegularPolygon();
  QVERIFY( rp1.center().isEmpty() );
  QVERIFY( rp1.firstVertex().isEmpty() );
  QCOMPARE( rp1.numberSides(), static_cast< unsigned int >( 0 ) );
  QCOMPARE( rp1.radius(), 0.0 );
  QVERIFY( rp1.isEmpty() );

  QgsRegularPolygon rp2;
  QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 0, 2, QgsRegularPolygon::InscribedCircle );
  QVERIFY( rp2.isEmpty() );
  QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 0, 5, static_cast< QgsRegularPolygon::ConstructionOption >( 4 ) );
  QVERIFY( rp2.isEmpty() );

  rp2 = QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 0, 5, QgsRegularPolygon::InscribedCircle );
  QVERIFY( !rp2.isEmpty() );
  QCOMPARE( rp2.center(), QgsPoint( 0, 0 ) );
  QCOMPARE( rp2.firstVertex(), QgsPoint( 0, 5 ) );
  QCOMPARE( rp2.numberSides(), static_cast< unsigned int>( 5 ) );
  QCOMPARE( rp2.radius(), 5.0 );
  QGSCOMPARENEAR( rp2.apothem(), 4.0451, 10E-4 );
  QVERIFY( rp2 ==  QgsRegularPolygon( QgsPoint( 0, 0 ), -5, 0, 5, QgsRegularPolygon::InscribedCircle ) );

  QgsRegularPolygon rp3 = QgsRegularPolygon( QgsPoint( 0, 0 ), rp2.apothem(), 36.0, 5, QgsRegularPolygon::CircumscribedCircle );
  QVERIFY( rp2 == rp3 );
  QVERIFY( rp2 == QgsRegularPolygon( QgsPoint( 0, 0 ), -rp2.apothem(), 36.0, 5, QgsRegularPolygon::CircumscribedCircle ) );
  QVERIFY( rp1 != rp3 );
  QVERIFY( rp1 != QgsRegularPolygon( QgsPoint( 5, 5 ), rp2.apothem(), 36.0, 5, QgsRegularPolygon::CircumscribedCircle ) );
  QVERIFY( rp1 != QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 36.0, 5, QgsRegularPolygon::CircumscribedCircle ) );
  QVERIFY( rp1 != QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 36.0, 5, QgsRegularPolygon::InscribedCircle ) );

  QgsRegularPolygon rp4 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), 2, QgsRegularPolygon::InscribedCircle );
  QVERIFY( rp4.isEmpty() );
  rp4 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 5 ), 5, static_cast< QgsRegularPolygon::ConstructionOption >( 4 ) );
  QVERIFY( rp4.isEmpty() );
  rp4 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), 5, QgsRegularPolygon::InscribedCircle );
  QVERIFY( rp4 == rp2 );

  QgsRegularPolygon rp5 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ).project( rp2.apothem(), 36.0 ), 2, QgsRegularPolygon::CircumscribedCircle );
  QVERIFY( rp5.isEmpty() );
  rp5 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ).project( rp2.apothem(), 36.0 ), 5, static_cast< QgsRegularPolygon::ConstructionOption >( 4 ) );
  QVERIFY( rp5.isEmpty() );
  rp5 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ).project( rp2.apothem(), 36.0 ), 5, QgsRegularPolygon::CircumscribedCircle );
  QVERIFY( rp5 == rp2 );

  QgsRegularPolygon rp6 = QgsRegularPolygon( QgsPoint( 0, 5 ), QgsPoint( 0, 0 ).project( 5.0, 72 ), 5 );
  QVERIFY( rp6 == rp2 );


  // setters and getters
  QgsRegularPolygon rp7 = QgsRegularPolygon();

  rp7.setCenter( QgsPoint( 5, 5 ) );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.center(), QgsPoint( 5, 5 ) );

  rp7.setNumberSides( 2 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), static_cast< unsigned int >( 0 ) );
  rp7.setNumberSides( 5 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), static_cast< unsigned int >( 5 ) );
  rp7.setNumberSides( 2 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), static_cast< unsigned int >( 5 ) );
  rp7.setNumberSides( 3 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), static_cast< unsigned int >( 3 ) );

  rp7.setRadius( -6 );
  QVERIFY( !rp7.isEmpty() );
  QCOMPARE( rp7.radius(), 6.0 );
  QCOMPARE( rp7.firstVertex(), rp7.center().project( 6, 0 ) );

  rp7.setFirstVertex( QgsPoint( 4, 4 ) );
  QCOMPARE( rp7.firstVertex(), QgsPoint( 4, 4 ) );
  QCOMPARE( rp7.radius(), rp7.center().distance3D( QgsPoint( 4, 4 ) ) );

  rp7 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), 5, QgsRegularPolygon::InscribedCircle );
  rp7.setCenter( QgsPoint( 5, 5 ) );
  QCOMPARE( rp7.radius(), 5.0 );
  QCOMPARE( rp7.firstVertex(), QgsPoint( 5, 10 ) );
  rp7.setNumberSides( 3 );
  QCOMPARE( rp7.radius(), 5.0 );
  QCOMPARE( rp7.firstVertex(), QgsPoint( 5, 10 ) );
  rp7.setNumberSides( 2 );
  QCOMPARE( rp7.radius(), 5.0 );
  QCOMPARE( rp7.firstVertex(), QgsPoint( 5, 10 ) );

  // measures
  QGSCOMPARENEAR( rp1.length(), 0.0, 10e-4 );
  QGSCOMPARENEAR( rp1.area(), 0.0, 10e-4 );
  QGSCOMPARENEAR( rp1.perimeter(), 0.0, 10e-4 );
  QGSCOMPARENEAR( rp2.length(), 5.8779, 10e-4 );
  QGSCOMPARENEAR( rp2.area(), 59.4410, 10e-4 );
  QGSCOMPARENEAR( rp2.perimeter(), 29.3893, 10e-4 );
  QCOMPARE( rp2.interiorAngle(), 108.0 );
  QCOMPARE( rp2.centralAngle(), 72.0 );
  QgsRegularPolygon rp8 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ), 5 );
  QGSCOMPARENEAR( rp8.area(), 43.0119, 10e-4 );
  QCOMPARE( rp8.perimeter(), 25.0 );
  QCOMPARE( rp8.length(), 5.0 );
  QCOMPARE( rp8.interiorAngle(), 108.0 );
  QCOMPARE( rp8.centralAngle(), 72.0 );
  rp8.setNumberSides( 4 );
  QCOMPARE( rp8.interiorAngle(), 90.0 );
  QCOMPARE( rp8.centralAngle(), 90.0 );
  rp8.setNumberSides( 3 );
  QCOMPARE( rp8.interiorAngle(), 60.0 );
  QCOMPARE( rp8.centralAngle(), 120.0 );

  //points
  rp8 = QgsRegularPolygon(); // empty
  QgsPointSequence points = rp8.points();
  QVERIFY( points.isEmpty() );
  rp8 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), 3, QgsRegularPolygon::InscribedCircle );
  points = rp8.points();
  QCOMPARE( points.count(), 3 );
  QCOMPARE( points.at( 0 ), QgsPoint( 0, 5 ) );
  QGSCOMPARENEAR( points.at( 1 ).x(), 4.33, 0.01 );
  QGSCOMPARENEAR( points.at( 1 ).y(), -2.4999, 0.01 );
  QGSCOMPARENEAR( points.at( 2 ).x(), -4.33, 0.01 );
  QGSCOMPARENEAR( points.at( 2 ).y(), -2.4999, 0.01 );

  //test conversions
  // circle
  QVERIFY( QgsCircle( QgsPoint( 0, 0 ), 5 ) == rp2.circumscribedCircle() );
  QVERIFY( rp2.inscribedCircle() == QgsRegularPolygon( QgsPoint( 0, 0 ), rp2.apothem(), 36.0, 5, QgsRegularPolygon::InscribedCircle ).circumscribedCircle() );

  // triangle
  QCOMPARE( QgsTriangle(), rp2.toTriangle() );
  QCOMPARE( QgsTriangle(), QgsRegularPolygon().toTriangle() );
  QgsRegularPolygon rp9 = QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 0, 3, QgsRegularPolygon::InscribedCircle );

  QVERIFY( QgsCircle( QgsPoint( 0, 0 ), 5 ) == rp9.toTriangle().circumscribedCircle() );

  QgsRegularPolygon rp10 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 4 ), 4 );
  QVector<QgsTriangle> rp10_tri = rp10.triangulate();
  QCOMPARE( rp10_tri.length(), static_cast< int >( rp10.numberSides() ) );
  QVERIFY( rp10_tri.at( 0 ) == QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 4 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 1 ) == QgsTriangle( QgsPoint( 0, 4 ), QgsPoint( 4, 4 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 2 ) == QgsTriangle( QgsPoint( 4, 4 ), QgsPoint( 4, 0 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 3 ) == QgsTriangle( QgsPoint( 4, 0 ), QgsPoint( 0, 0 ), rp10.center() ) );

  QVERIFY( QgsRegularPolygon().triangulate().isEmpty() );

  // polygon
  std::unique_ptr< QgsPolygon > toP( QgsRegularPolygon().toPolygon() );
  QVERIFY( toP->isEmpty() );

  QgsPointSequence ptsPol;
  std::unique_ptr< QgsPolygon > pol( new QgsPolygon() );
  pol.reset( rp10.toPolygon() );
  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );

  pol->exteriorRing()->points( ptsPol );
  QCOMPARE( ptsPol.length(), 5 );
  QVERIFY( ptsPol.at( 0 ) == QgsPoint( 0, 0 ) );
  QVERIFY( ptsPol.at( 1 ) == QgsPoint( 0, 4 ) );
  QVERIFY( ptsPol.at( 2 ) == QgsPoint( 4, 4 ) );
  QVERIFY( ptsPol.at( 3 ) == QgsPoint( 4, 0 ) );
  QVERIFY( ptsPol.at( 4 ) == QgsPoint( 0, 0 ) );
  ptsPol.pop_back();

  std::unique_ptr< QgsLineString > l( QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), 1, QgsRegularPolygon::InscribedCircle ).toLineString() );
  QVERIFY( l->isEmpty() );
  l.reset( rp10.toLineString( ) );
  QCOMPARE( l->numPoints(), 5 );
  QCOMPARE( l->pointN( 0 ), l->pointN( 4 ) );
  QgsPointSequence pts_l;
  l->points( pts_l );
  pts_l.pop_back();
  QCOMPARE( ptsPol, pts_l );

  //test toString
  QCOMPARE( rp1.toString(), QString( "Empty" ) );
  QCOMPARE( rp2.toString(), QString( "RegularPolygon (Center: Point (0 0), First Vertex: Point (0 5), Radius: 5, Azimuth: 0)" ) );

}


QGSTEST_MAIN( TestQgsRegularPolygon )
#include "testqgsregularpolygon.moc"
