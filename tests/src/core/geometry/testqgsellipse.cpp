/***************************************************************************
     testqgsellipse.cpp
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

#include "qgsellipse.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspoint.h"

#include "testgeometryutils.h"

class TestQgsEllipse: public QObject
{
    Q_OBJECT
  private slots:
    void ellipse();
};

void TestQgsEllipse::ellipse()
{
  //test constructors
  QgsEllipse elp1;
  QVERIFY( elp1.center().isEmpty() );
  QCOMPARE( elp1.semiMajorAxis(), 0.0 );
  QCOMPARE( elp1.semiMinorAxis(), 0.0 );
  QCOMPARE( elp1.azimuth(), 90.0 );
  QVERIFY( elp1.isEmpty() );

  QgsEllipse elp2( QgsPoint( 5, 10 ), 3, 2 );
  QVERIFY( elp2.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp2.semiMajorAxis(), 3.0 );
  QCOMPARE( elp2.semiMinorAxis(), 2.0 );
  QCOMPARE( elp2.azimuth(), 90.0 );
  QVERIFY( !elp2.isEmpty() );

  QgsEllipse elp3( QgsPoint( 5, 10 ), 3, 2, 45 );
  QVERIFY( elp3.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp3.semiMajorAxis(), 3.0 );
  QCOMPARE( elp3.semiMinorAxis(), 2.0 );
  QCOMPARE( elp3.azimuth(), 45.0 );
  QVERIFY( !elp3.isEmpty() );

  QgsEllipse elp4( QgsPoint( 5, 10 ), 2, 3, 45 );
  QVERIFY( elp4.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp4.semiMajorAxis(), 3.0 );
  QCOMPARE( elp4.semiMinorAxis(), 2.0 );
  QCOMPARE( elp4.azimuth(), 135.0 );
  QVERIFY( !elp4.isEmpty() );

  //test toString
  QCOMPARE( elp1.toString(), QString( "Empty" ) );
  QCOMPARE( elp2.toString(), QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 90)" ) );
  QCOMPARE( elp3.toString(), QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 45)" ) );
  QCOMPARE( elp4.toString(), QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 135)" ) );

  //test equality operator
  QCOMPARE( QgsEllipse().isEmpty(), QgsEllipse( QgsPoint(), 0, 0, 90 ).isEmpty() );
  QVERIFY( !( QgsEllipse() == QgsEllipse( QgsPoint( 0, 0 ), 0, 0, 0.0005 ) ) );
  QVERIFY( elp2 == QgsEllipse( QgsPoint( 5, 10 ), 2, 3, 0 ) );
  QVERIFY( elp2 != elp3 );
  QVERIFY( elp3 != elp4 );
  QVERIFY( elp4 == QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 90 + 45 ) );

  //test setter and getter
  elp1.setAzimuth( 45 );
  QCOMPARE( elp1.azimuth(), 45.0 );

  elp1.setSemiMajorAxis( 50 );
  QCOMPARE( elp1.semiMajorAxis(), 50.0 );

  // axis_b > axis_a
  elp1.setSemiMinorAxis( 70 );
  QCOMPARE( elp1.semiMajorAxis(), 70.0 );
  QCOMPARE( elp1.semiMinorAxis(), 50.0 );
  // axis_b < axis_a
  elp1.setSemiMinorAxis( 3 );
  QCOMPARE( elp1.semiMinorAxis(), 3.0 );
  QCOMPARE( elp1.semiMajorAxis(), 70.0 );

  elp1.setSemiMajorAxis( 2 );
  QCOMPARE( elp1.semiMinorAxis(), 2.0 );
  QCOMPARE( elp1.semiMajorAxis(), 3.0 );

  elp1.setCenter( QgsPoint( 5, 10 ) );
  QVERIFY( elp1.center() == QgsPoint( 5, 10 ) );
  QVERIFY( elp1.rcenter() == QgsPoint( 5, 10 ) );
  elp1.rcenter() = QgsPoint( 25, 310 );
  QVERIFY( elp1.center() == QgsPoint( 25, 310 ) );

  //test "alt" constructors
  // fromExtent
  QgsEllipse elp_alt = QgsEllipse( QgsPoint( 2.5, 5 ), 2.5, 5 );
  QVERIFY( QgsEllipse().fromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 10 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromExtent( QgsPoint( 5, 10 ), QgsPoint( 0, 0 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromExtent( QgsPoint( 5, 0 ), QgsPoint( 0, 10 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromExtent( QgsPoint( -5, 0 ), QgsPoint( 0, 10 ) ) != elp_alt );
  // fromCenterPoint
  QVERIFY( QgsEllipse().fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( 5, 10 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( -0, 0 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( 0, -10 ) ) != elp_alt );
  // fromCenter2Points
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ) ) ==
           QgsEllipse( QgsPoint( 2.5, 5 ), 5, 5, 180 ) );
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 7.5 ), QgsPoint( 7.5, 5 ) ) != elp_alt ); //same ellipse with different azimuth
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 2.5 ), QgsPoint( 7.5, 5 ) ) != elp_alt ); //same ellipse with different azimuth
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 0 ), QgsPoint( 5, 5 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 5, 10 ), QgsPoint( 5, 10 ).project( 3, 45 ), QgsPoint( 5, 10 ).project( 2, 90 + 45 ) ) ==
           QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 45 ) );

  // fromFoci
  // horizontal
  QgsEllipse elp_hor = QgsEllipse().fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) );
  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), std::sqrt( 32.0 ), std::sqrt( 16.0 ), 90.0 ) == elp_hor );
  QGSCOMPARENEARPOINT( QgsPoint( 4, 0 ), elp_hor.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( -4, 0 ), elp_hor.foci().at( 1 ), 1e-8 );
  elp_hor = QgsEllipse().fromFoci( QgsPoint( 4, 0 ), QgsPoint( -4, 0 ), QgsPoint( 0, 4 ) );
  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), std::sqrt( 32.0 ), std::sqrt( 16.0 ), 270.0 ) == elp_hor );
  QGSCOMPARENEARPOINT( QgsPoint( -4, 0 ), elp_hor.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 4, 0 ), elp_hor.foci().at( 1 ), 1e-8 );

  // vertical
  QgsEllipse elp_ver = QgsEllipse().fromFoci( QgsPoint( 45, -15 ), QgsPoint( 45, 10 ), QgsPoint( 55, 0 ) );
  QVERIFY( QgsEllipse( QgsPoint( 45, -2.5 ), 16.084946, 10.123017725, 0.0 ) == elp_ver );
  elp_ver = QgsEllipse().fromFoci( QgsPoint( 45, 10 ), QgsPoint( 45, -15 ), QgsPoint( 55, 0 ) );
  QVERIFY( QgsEllipse( QgsPoint( 45, -2.5 ), 16.084946, 10.123017725, 180.0 ) == elp_ver );
  QGSCOMPARENEARPOINT( QgsPoint( 45, -15 ), elp_ver.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 45, 10 ), elp_ver.foci().at( 1 ), 1e-8 );
  // oriented
  // first quadrant
  QgsEllipse elp_ori = QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, 20 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 17.5, 15.0 ), 10.5901699437, 5.55892970251, 90.0 - 33.690067526 ) == elp_ori );
  QGSCOMPARENEARPOINT( QgsPoint( 25, 20 ), elp_ori.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp_ori.foci().at( 1 ), 1e-8 );
  // second quadrant
  elp_ori = QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 5, 20 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 7.5, 15.0 ), 10.5901699437, 8.99453719974, 360 - 26.56505117 ) == elp_ori );
  QGSCOMPARENEARPOINT( QgsPoint( 5, 20 ), elp_ori.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp_ori.foci().at( 1 ), 1e-8 );
  // third quadrant
  elp_ori = QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 5, -5 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 7.5, 2.5 ), 19.0530819616, 17.3355107289893, 198.434948822922 ) == elp_ori );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp_ori.foci().at( 1 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 5, -5 ), elp_ori.foci().at( 0 ), 1e-8 );
  // fourth quadrant
  elp_ori = QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, -5 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 17.5, 2.5 ), 19.0530819616, 15.82782146, 135 ) == elp_ori );
  QGSCOMPARENEARPOINT( QgsPoint( 25, -5 ), elp_ori.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp_ori.foci().at( 1 ), 1e-8 );

  // test quadrant
  QgsEllipse elpq( QgsPoint( 5, 10 ), 3, 2, 45 );
  QgsPointSequence q = elpq.quadrant();
  QGSCOMPARENEARPOINT( q.at( 0 ), QgsPoint( 7.1213, 12.1213 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 3 ), QgsPoint( 3.5858, 11.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 2 ), QgsPoint( 2.8787, 7.8787 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 1 ), QgsPoint( 6.4142, 8.5858 ), 0.001 );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 );
  q.clear();
  q = elpq.quadrant();
  QCOMPARE( q.at( 3 ), QgsPoint( 0, 2 ) );
  QCOMPARE( q.at( 0 ), QgsPoint( 5, 0 ) );
  QCOMPARE( q.at( 1 ), QgsPoint( 0, -2 ) );
  QCOMPARE( q.at( 2 ), QgsPoint( -5, 0 ) );

  elpq = QgsEllipse( QgsPoint( QgsWkbTypes::PointZM, 0, 0, 123, 321 ), 5, 2, 0 );
  q.clear();
  q = elpq.quadrant();
  QCOMPARE( q.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 0, 5, 123, 321 ) );
  QCOMPARE( q.at( 3 ), QgsPoint( QgsWkbTypes::PointZM, -2, 0, 123, 321 ) );
  QCOMPARE( q.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 0, -5, 123, 321 ) );
  QCOMPARE( q.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 0, 123, 321 ) );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 2.5, 2, 315 );
  q.clear();
  q = elpq.quadrant();
  QGSCOMPARENEARPOINT( q.at( 1 ), QgsPoint( 1.4142, 1.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 2 ), QgsPoint( 1.7678, -1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 3 ), QgsPoint( -1.4142, -1.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 0 ), QgsPoint( -1.7678, 1.7678 ), 0.001 );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 45 );
  q.clear();
  q = elpq.quadrant();
  QGSCOMPARENEARPOINT( q.at( 3 ), QgsPoint( -1.7678, 1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 0 ), QgsPoint( 3.5355, 3.5355 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 1 ), QgsPoint( 1.7678, -1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 2 ), QgsPoint( -3.5355, -3.5355 ), 0.001 );

  //test conversion
  // points
  QgsPointSequence pts;
  pts = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).points( 4 );
  q = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).quadrant();
  QCOMPARE( pts.length(), 4 );
  QGSCOMPARENEARPOINT( q.at( 0 ), pts.at( 0 ), 2 );
  QGSCOMPARENEARPOINT( q.at( 1 ), pts.at( 1 ), 2 );
  QGSCOMPARENEARPOINT( q.at( 2 ), pts.at( 2 ), 2 );
  QGSCOMPARENEARPOINT( q.at( 3 ), pts.at( 3 ), 2 );

  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).points( 2 ).isEmpty() ); // segments too low

  // linestring
  std::unique_ptr< QgsLineString > l( new QgsLineString() );

  l.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toLineString( 2 ) );
  QVERIFY( l->isEmpty() ); // segments too low

  l.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toLineString( 4 ) );
  QCOMPARE( l->numPoints(), 5 ); // closed linestring
  QgsPointSequence pts_l;
  l->points( pts_l );
  pts_l.pop_back();
  QCOMPARE( pts, pts_l );

  // polygon
  std::unique_ptr< QgsPolygon > p1( new QgsPolygon() );

  p1.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toPolygon( 2 ) );
  QVERIFY( p1->isEmpty() ); // segments too low

  p1.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toPolygon( 4 ) );
  q = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  p1.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 ).toPolygon( 4 ) );
  q = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 ).quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  p1.reset( elpq.toPolygon( 4 ) );
  q = elpq.quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  // oriented bounding box
  std::unique_ptr< QgsPolygon > ombb( QgsEllipse().orientedBoundingBox() );
  QVERIFY( ombb->isEmpty() );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2 );
  ombb.reset( new QgsPolygon() );
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 5, 2 ) << QgsPoint( 5, -2 ) << QgsPoint( -5, -2 ) << QgsPoint( -5, 2 ) );
  ombb->setExteriorRing( ext );
  std::unique_ptr< QgsPolygon >ombb2( elpq.orientedBoundingBox() );
  QCOMPARE( ombb->asWkt( 2 ), ombb2->asWkt( 2 ) );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 45 );
  ombb.reset( elpq.orientedBoundingBox() );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1.7678, 5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 5.3033, 1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( -1.7678, -5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( -5.3033, -1.7678 ), 0.0001 );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 315 );
  ombb.reset( elpq.orientedBoundingBox() );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( -5.3033, 1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( -1.7678, 5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 5.3033, -1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 1.7678, -5.3033 ), 0.0001 );

  // bounding box
  QCOMPARE( QgsEllipse().boundingBox(), QgsRectangle() );
  ombb.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2 ).orientedBoundingBox() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 2 ).boundingBox(), ombb->boundingBox() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 5 ).boundingBox(), QgsRectangle( QgsPointXY( -5, -5 ), QgsPointXY( 5, 5 ) ) );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 5, 60 ).boundingBox(), QgsRectangle( QgsPointXY( -5, -5 ), QgsPointXY( 5, 5 ) ) );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 45 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -11.1803, -11.1803 ), QgsPointXY( 11.1803, 11.1803 ) ).toString( 4 ).toStdString() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 60 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -12.12436, -10.14889 ), QgsPointXY( 12.12436, 10.14889 ) ).toString( 4 ).toStdString() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 60 + 90 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -10.14889, -12.12436 ), QgsPointXY( 10.14889, 12.12436 ) ).toString( 4 ).toStdString() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 300 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -12.12436, -10.14889 ), QgsPointXY( 12.12436, 10.14889 ) ).toString( 4 ).toStdString() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 300 - 90 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -10.14889, -12.12436 ), QgsPointXY( 10.14889, 12.12436 ) ).toString( 4 ).toStdString() );

  // focus
  QCOMPARE( QgsEllipse().fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) ).focusDistance(), 4.0 );
  QGSCOMPARENEAR( QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, 20 ), QgsPoint( 15, 20 ) ).focusDistance(), 9.01388, 0.0001 );

  // eccentricity
  QCOMPARE( QgsEllipse().fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) ).eccentricity(), 0.7071067811865475 );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 3, 3 ).eccentricity(), 0.0 );
  QVERIFY( std::isnan( QgsEllipse().eccentricity() ) );

  // area
  QGSCOMPARENEAR( 31.4159, QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).area(), 0.0001 );
  // perimeter
  p1.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 45 ).toPolygon( 10000 ) );
  QGSCOMPARENEAR( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 45 ).perimeter(), p1->perimeter(), 0.001 );

}


QGSTEST_MAIN( TestQgsEllipse )
#include "testqgsellipse.moc"
