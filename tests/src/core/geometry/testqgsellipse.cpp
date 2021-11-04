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
    void constructor();
    void fromExtent();
    void fromCenterPoint();
    void fromCenter2Points();
    void fromFoci();
    void settersGetters();
    void equality();
    void points();
    void quadrant();
    void area();
    void perimeter();
    void focusDistance();
    void eccentricity();
    void boundingBox();
    void orientedBoundingBox();
    void toString();
    void toLineString();
    void toPolygon();
};


void TestQgsEllipse::constructor()
{
  QgsEllipse elp;
  QVERIFY( elp.center().isEmpty() );
  QCOMPARE( elp.semiMajorAxis(), 0.0 );
  QCOMPARE( elp.semiMinorAxis(), 0.0 );
  QCOMPARE( elp.azimuth(), 90.0 );
  QVERIFY( elp.isEmpty() );

  elp = QgsEllipse( QgsPoint( 5, 10 ), 3, 2 );
  QVERIFY( elp.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp.semiMajorAxis(), 3.0 );
  QCOMPARE( elp.semiMinorAxis(), 2.0 );
  QCOMPARE( elp.azimuth(), 90.0 );
  QVERIFY( !elp.isEmpty() );

  elp = QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 45 );
  QVERIFY( elp.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp.semiMajorAxis(), 3.0 );
  QCOMPARE( elp.semiMinorAxis(), 2.0 );
  QCOMPARE( elp.azimuth(), 45.0 );
  QVERIFY( !elp.isEmpty() );

  elp = QgsEllipse( QgsPoint( 5, 10 ), 2, 3, 45 );
  QVERIFY( elp.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp.semiMajorAxis(), 3.0 );
  QCOMPARE( elp.semiMinorAxis(), 2.0 );
  QCOMPARE( elp.azimuth(), 135.0 );
  QVERIFY( !elp.isEmpty() );

  QCOMPARE( QgsEllipse().isEmpty(), QgsEllipse( QgsPoint(), 0, 0, 90 ).isEmpty() );
}

void TestQgsEllipse::fromExtent()
{
  QgsEllipse elp = QgsEllipse( QgsPoint( 2.5, 5 ), 2.5, 5 );

  QVERIFY( QgsEllipse::fromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 10 ) ) == elp );
  QVERIFY( QgsEllipse::fromExtent( QgsPoint( 5, 10 ), QgsPoint( 0, 0 ) ) == elp );
  QVERIFY( QgsEllipse::fromExtent( QgsPoint( 5, 0 ), QgsPoint( 0, 10 ) ) == elp );
  QVERIFY( QgsEllipse::fromExtent( QgsPoint( -5, 0 ), QgsPoint( 0, 10 ) ) != elp );
}

void TestQgsEllipse::fromCenterPoint()
{
  QgsEllipse elp = QgsEllipse( QgsPoint( 2.5, 5 ), 2.5, 5 );

  QVERIFY( QgsEllipse::fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( 5, 10 ) ) == elp );
  QVERIFY( QgsEllipse::fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( -0, 0 ) ) == elp );
  QVERIFY( QgsEllipse::fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( 0, -10 ) ) != elp );
}

void TestQgsEllipse::fromCenter2Points()
{
  QgsEllipse elp = QgsEllipse( QgsPoint( 2.5, 5 ), 2.5, 5 );

  QVERIFY( QgsEllipse::fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ) )
           == QgsEllipse( QgsPoint( 2.5, 5 ), 5, 5, 180 ) );
  QVERIFY( QgsEllipse::fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 7.5 ), QgsPoint( 7.5, 5 ) ) != elp ); //same ellipse with different azimuth
  QVERIFY( QgsEllipse::fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 2.5 ), QgsPoint( 7.5, 5 ) ) != elp ); //same ellipse with different azimuth
  QVERIFY( QgsEllipse::fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 0 ), QgsPoint( 5, 5 ) ) == elp );
  QVERIFY( QgsEllipse::fromCenter2Points( QgsPoint( 5, 10 ), QgsPoint( 5, 10 ).project( 3, 45 ), QgsPoint( 5, 10 ).project( 2, 90 + 45 ) )
           == QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 45 ) );
}

void TestQgsEllipse::fromFoci()
{
  // horizontal
  QgsEllipse elp = QgsEllipse::fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) );

  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), std::sqrt( 32.0 ), std::sqrt( 16.0 ), 90.0 ) == elp );
  QGSCOMPARENEARPOINT( QgsPoint( 4, 0 ), elp.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( -4, 0 ), elp.foci().at( 1 ), 1e-8 );

  elp = QgsEllipse::fromFoci( QgsPoint( 4, 0 ), QgsPoint( -4, 0 ), QgsPoint( 0, 4 ) );

  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), std::sqrt( 32.0 ), std::sqrt( 16.0 ), 270.0 ) == elp );
  QGSCOMPARENEARPOINT( QgsPoint( -4, 0 ), elp.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 4, 0 ), elp.foci().at( 1 ), 1e-8 );

  // vertical
  elp = QgsEllipse::fromFoci( QgsPoint( 45, -15 ), QgsPoint( 45, 10 ), QgsPoint( 55, 0 ) );

  QVERIFY( QgsEllipse( QgsPoint( 45, -2.5 ), 16.084946, 10.123017725, 0.0 ) == elp );

  elp = QgsEllipse::fromFoci( QgsPoint( 45, 10 ), QgsPoint( 45, -15 ), QgsPoint( 55, 0 ) );

  QVERIFY( QgsEllipse( QgsPoint( 45, -2.5 ), 16.084946, 10.123017725, 180.0 ) == elp );
  QGSCOMPARENEARPOINT( QgsPoint( 45, -15 ), elp.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 45, 10 ), elp.foci().at( 1 ), 1e-8 );

  // oriented
  // first quadrant
  elp = QgsEllipse::fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, 20 ), QgsPoint( 15, 20 ) );

  QVERIFY( QgsEllipse( QgsPoint( 17.5, 15.0 ), 10.5901699437, 5.55892970251, 90.0 - 33.690067526 ) == elp );
  QGSCOMPARENEARPOINT( QgsPoint( 25, 20 ), elp.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp.foci().at( 1 ), 1e-8 );

  // second quadrant
  elp = QgsEllipse::fromFoci( QgsPoint( 10, 10 ), QgsPoint( 5, 20 ), QgsPoint( 15, 20 ) );

  QVERIFY( QgsEllipse( QgsPoint( 7.5, 15.0 ), 10.5901699437, 8.99453719974, 360 - 26.56505117 ) == elp );
  QGSCOMPARENEARPOINT( QgsPoint( 5, 20 ), elp.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp.foci().at( 1 ), 1e-8 );

  // third quadrant
  elp = QgsEllipse::fromFoci( QgsPoint( 10, 10 ), QgsPoint( 5, -5 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 7.5, 2.5 ), 19.0530819616, 17.3355107289893, 198.434948822922 ) == elp );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp.foci().at( 1 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 5, -5 ), elp.foci().at( 0 ), 1e-8 );
  // fourth quadrant
  elp = QgsEllipse::fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, -5 ), QgsPoint( 15, 20 ) );

  QVERIFY( QgsEllipse( QgsPoint( 17.5, 2.5 ), 19.0530819616, 15.82782146, 135 ) == elp );
  QGSCOMPARENEARPOINT( QgsPoint( 25, -5 ), elp.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp.foci().at( 1 ), 1e-8 );
}

void TestQgsEllipse::settersGetters()
{
  QgsEllipse elp;

  elp.setAzimuth( 45 );
  QCOMPARE( elp.azimuth(), 45.0 );

  elp.setSemiMajorAxis( 50 );
  QCOMPARE( elp.semiMajorAxis(), 50.0 );

  // axis_b > axis_a
  elp.setSemiMinorAxis( 70 );
  QCOMPARE( elp.semiMajorAxis(), 70.0 );
  QCOMPARE( elp.semiMinorAxis(), 50.0 );

  // axis_b < axis_a
  elp.setSemiMinorAxis( 3 );
  QCOMPARE( elp.semiMinorAxis(), 3.0 );
  QCOMPARE( elp.semiMajorAxis(), 70.0 );

  elp.setSemiMajorAxis( 2 );
  QCOMPARE( elp.semiMinorAxis(), 2.0 );
  QCOMPARE( elp.semiMajorAxis(), 3.0 );

  elp.setCenter( QgsPoint( 5, 10 ) );
  QVERIFY( elp.center() == QgsPoint( 5, 10 ) );
  QVERIFY( elp.rcenter() == QgsPoint( 5, 10 ) );

  elp.rcenter() = QgsPoint( 25, 310 );
  QVERIFY( elp.center() == QgsPoint( 25, 310 ) );
}

void TestQgsEllipse::equality()
{
  QVERIFY( !( QgsEllipse()
              == QgsEllipse( QgsPoint( 0, 0 ), 0, 0, 0.0005 ) ) );
  QVERIFY( QgsEllipse( QgsPoint( 5, 10 ), 3, 2 )
           == QgsEllipse( QgsPoint( 5, 10 ), 2, 3, 0 ) );
  QVERIFY( QgsEllipse( QgsPoint( 5, 10 ), 3, 2 )
           != QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 45 ) );
  QVERIFY( QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 45 )
           != QgsEllipse( QgsPoint( 5, 10 ), 2, 3, 45 ) );
  QVERIFY( QgsEllipse( QgsPoint( 5, 10 ), 2, 3, 45 )
           == QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 90 + 45 ) );
}

void TestQgsEllipse::points()
{
  QgsPointSequence pts = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).points( 4 );
  QgsPointSequence quad = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).quadrant();

  QCOMPARE( pts.length(), 4 );
  QGSCOMPARENEARPOINT( quad.at( 0 ), pts.at( 0 ), 2 );
  QGSCOMPARENEARPOINT( quad.at( 1 ), pts.at( 1 ), 2 );
  QGSCOMPARENEARPOINT( quad.at( 2 ), pts.at( 2 ), 2 );
  QGSCOMPARENEARPOINT( quad.at( 3 ), pts.at( 3 ), 2 );

  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).points( 2 ).isEmpty() ); // segments too low
}

void TestQgsEllipse::quadrant()
{
  QgsEllipse elp( QgsPoint( 5, 10 ), 3, 2, 45 );
  QgsPointSequence quad = elp.quadrant();

  QGSCOMPARENEARPOINT( quad.at( 0 ), QgsPoint( 7.1213, 12.1213 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 3 ), QgsPoint( 3.5858, 11.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 2 ), QgsPoint( 2.8787, 7.8787 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 1 ), QgsPoint( 6.4142, 8.5858 ), 0.001 );

  elp = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 );
  quad.clear();
  quad = elp.quadrant();

  QCOMPARE( quad.at( 3 ), QgsPoint( 0, 2 ) );
  QCOMPARE( quad.at( 0 ), QgsPoint( 5, 0 ) );
  QCOMPARE( quad.at( 1 ), QgsPoint( 0, -2 ) );
  QCOMPARE( quad.at( 2 ), QgsPoint( -5, 0 ) );

  elp = QgsEllipse( QgsPoint( QgsWkbTypes::PointZM, 0, 0, 123, 321 ), 5, 2, 0 );
  quad.clear();
  quad = elp.quadrant();

  QCOMPARE( quad.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 0, 5, 123, 321 ) );
  QCOMPARE( quad.at( 3 ), QgsPoint( QgsWkbTypes::PointZM, -2, 0, 123, 321 ) );
  QCOMPARE( quad.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 0, -5, 123, 321 ) );
  QCOMPARE( quad.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 0, 123, 321 ) );

  elp = QgsEllipse( QgsPoint( 0, 0 ), 2.5, 2, 315 );
  quad.clear();
  quad = elp.quadrant();

  QGSCOMPARENEARPOINT( quad.at( 1 ), QgsPoint( 1.4142, 1.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 2 ), QgsPoint( 1.7678, -1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 3 ), QgsPoint( -1.4142, -1.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 0 ), QgsPoint( -1.7678, 1.7678 ), 0.001 );

  elp = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 45 );
  quad.clear();
  quad = elp.quadrant();

  QGSCOMPARENEARPOINT( quad.at( 3 ), QgsPoint( -1.7678, 1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 0 ), QgsPoint( 3.5355, 3.5355 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 1 ), QgsPoint( 1.7678, -1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( quad.at( 2 ), QgsPoint( -3.5355, -3.5355 ), 0.001 );
}

void TestQgsEllipse::area()
{
  QGSCOMPARENEAR( 31.4159, QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).area(), 0.0001 );
}

void TestQgsEllipse::perimeter()
{
  std::unique_ptr< QgsPolygon > poly( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 45 ).toPolygon( 10000 ) );

  QGSCOMPARENEAR( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 45 ).perimeter(), poly->perimeter(), 0.001 );
}

void TestQgsEllipse::focusDistance()
{
  QgsEllipse elp = QgsEllipse::fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) );
  QCOMPARE( elp.focusDistance(), 4.0 );

  elp = QgsEllipse::fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, 20 ), QgsPoint( 15, 20 ) );
  QGSCOMPARENEAR( elp.focusDistance(), 9.01388, 0.0001 );
}

void TestQgsEllipse::eccentricity()
{
  QgsEllipse elp = QgsEllipse::fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) );
  QCOMPARE( elp.eccentricity(), 0.7071067811865475 );

  elp = QgsEllipse( QgsPoint( 0, 0 ), 3, 3 );
  QCOMPARE( elp.eccentricity(), 0.0 );

  QVERIFY( std::isnan( QgsEllipse().eccentricity() ) );
}

void TestQgsEllipse::boundingBox()
{
  QCOMPARE( QgsEllipse().boundingBox(), QgsRectangle() );

  std::unique_ptr< QgsPolygon > poly( QgsEllipse( QgsPoint( 0, 0 ), 5, 2 ).orientedBoundingBox() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 2 ).boundingBox(), poly->boundingBox() );

  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 5 ).boundingBox(),
            QgsRectangle( QgsPointXY( -5, -5 ), QgsPointXY( 5, 5 ) ) );

  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 5, 60 ).boundingBox(),
            QgsRectangle( QgsPointXY( -5, -5 ), QgsPointXY( 5, 5 ) ) );

  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 45 ).boundingBox().toString( 4 ).toStdString(),
            QgsRectangle( QgsPointXY( -11.1803, -11.1803 ), QgsPointXY( 11.1803, 11.1803 ) ).toString( 4 ).toStdString() );

  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 60 ).boundingBox().toString( 4 ).toStdString(),
            QgsRectangle( QgsPointXY( -12.12436, -10.14889 ), QgsPointXY( 12.12436, 10.14889 ) ).toString( 4 ).toStdString() );

  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 60 + 90 ).boundingBox().toString( 4 ).toStdString(),
            QgsRectangle( QgsPointXY( -10.14889, -12.12436 ), QgsPointXY( 10.14889, 12.12436 ) ).toString( 4 ).toStdString() );

  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 300 ).boundingBox().toString( 4 ).toStdString(),
            QgsRectangle( QgsPointXY( -12.12436, -10.14889 ), QgsPointXY( 12.12436, 10.14889 ) ).toString( 4 ).toStdString() );

  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 300 - 90 ).boundingBox().toString( 4 ).toStdString(),
            QgsRectangle( QgsPointXY( -10.14889, -12.12436 ), QgsPointXY( 10.14889, 12.12436 ) ).toString( 4 ).toStdString() );
}

void TestQgsEllipse::orientedBoundingBox()
{
  std::unique_ptr< QgsPolygon > poly1( QgsEllipse().orientedBoundingBox() );
  QVERIFY( poly1->isEmpty() );

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 5, 2 ) << QgsPoint( 5, -2 )
                  << QgsPoint( -5, -2 ) << QgsPoint( -5, 2 ) );
  poly1.reset( new QgsPolygon() );
  poly1->setExteriorRing( ext );

  QgsEllipse elp( QgsPoint( 0, 0 ), 5, 2 );
  std::unique_ptr< QgsPolygon > poly2( elp.orientedBoundingBox() );

  QCOMPARE( poly1->asWkt( 2 ), poly2->asWkt( 2 ) );

  elp = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 45 );
  poly1.reset( elp.orientedBoundingBox() );

  QGSCOMPARENEARPOINT( poly1->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1.7678, 5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( poly1->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 5.3033, 1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( poly1->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( -1.7678, -5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( poly1->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( -5.3033, -1.7678 ), 0.0001 );

  elp = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 315 );
  poly1.reset( elp.orientedBoundingBox() );

  QGSCOMPARENEARPOINT( poly1->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( -5.3033, 1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( poly1->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( -1.7678, 5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( poly1->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 5.3033, -1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( poly1->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 1.7678, -5.3033 ), 0.0001 );
}

void TestQgsEllipse::toString()
{
  QCOMPARE( QgsEllipse().toString(),
            QString( "Empty" ) );
  QCOMPARE( QgsEllipse( QgsPoint( 5, 10 ), 3, 2 ).toString(),
            QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 90)" ) );
  QCOMPARE( QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 45 ).toString(),
            QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 45)" ) );
  QCOMPARE( QgsEllipse( QgsPoint( 5, 10 ), 2, 3, 45 ).toString(),
            QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 135)" ) );
}

void TestQgsEllipse::toLineString()
{
  std::unique_ptr< QgsLineString > ls( new QgsLineString() );

  ls.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toLineString( 2 ) );
  QVERIFY( ls->isEmpty() ); // segments too low

  ls.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toLineString( 4 ) );
  QCOMPARE( ls->numPoints(), 5 ); // closed linestring

  QgsPointSequence pts_ls;
  ls->points( pts_ls );
  pts_ls.pop_back();
  QgsPointSequence pts = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).points( 4 );
  QCOMPARE( pts, pts_ls );
}

void TestQgsEllipse::toPolygon()
{
  std::unique_ptr< QgsPolygon > poly( new QgsPolygon() );

  poly.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toPolygon( 2 ) );
  QVERIFY( poly->isEmpty() ); // segments too low

  poly.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toPolygon( 4 ) );
  QgsPointSequence quad = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).quadrant();

  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 0 ) ), quad.at( 0 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 1 ) ), quad.at( 1 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 2 ) ), quad.at( 2 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 3 ) ), quad.at( 3 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 4 ) ), quad.at( 0 ) );
  QCOMPARE( 0, poly->numInteriorRings() );
  QCOMPARE( 5, poly->exteriorRing()->numPoints() );

  poly.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 ).toPolygon( 4 ) );
  quad = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 ).quadrant();

  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 0 ) ), quad.at( 0 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 1 ) ), quad.at( 1 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 2 ) ), quad.at( 2 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 3 ) ), quad.at( 3 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 4 ) ), quad.at( 0 ) );
  QCOMPARE( 0, poly->numInteriorRings() );
  QCOMPARE( 5, poly->exteriorRing()->numPoints() );

  QgsEllipse elp( QgsPoint( 0, 0 ), 5, 2.5, 45 );
  poly.reset( elp.toPolygon( 4 ) );
  quad = elp.quadrant();

  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 0 ) ), quad.at( 0 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 1 ) ), quad.at( 1 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 2 ) ), quad.at( 2 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 3 ) ), quad.at( 3 ) );
  QCOMPARE( poly->vertexAt( QgsVertexId( 0, 0, 4 ) ), quad.at( 0 ) );
  QCOMPARE( 0, poly->numInteriorRings() );
  QCOMPARE( 5, poly->exteriorRing()->numPoints() );
}


QGSTEST_MAIN( TestQgsEllipse )
#include "testqgsellipse.moc"
