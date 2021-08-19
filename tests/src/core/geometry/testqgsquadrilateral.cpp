/***************************************************************************
     testqgsquadrilateral.cpp
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
#include "qgsquadrilateral.h"

#include "testgeometryutils.h"

class TestQgsQuadrilateral: public QObject
{
    Q_OBJECT
  private slots:
    void quadrilateral();
};


void TestQgsQuadrilateral::quadrilateral()
{

  // default
  QgsQuadrilateral quad_init;
  QgsPointSequence pts = quad_init.points();
  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );
  QVERIFY( !quad_init.isValid() );

  // colinear
  QgsQuadrilateral quad4points_col( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  QVERIFY( !quad4points_col.isValid() );
  pts = quad4points_col.points();
  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );


  QgsQuadrilateral quad4pointsXY_col( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  QVERIFY( !quad4pointsXY_col.isValid() );
  pts = quad4pointsXY_col.points();
  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );


  // anti parallelogram
  QgsQuadrilateral quad4points_anti( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QVERIFY( !quad4points_anti.isValid() );
  pts = quad4points_anti.points();
  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );


  QgsQuadrilateral quad4pointsXY_anti( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QVERIFY( !quad4pointsXY_anti.isValid() );
  pts = quad4pointsXY_anti.points();
  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );

  // valid
  QgsQuadrilateral quad4points_valid( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QVERIFY( quad4points_valid.isValid() );
  pts = quad4points_valid.points();
  QCOMPARE( pts.at( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( pts.at( 1 ), QgsPoint( 0, 5 ) );
  QCOMPARE( pts.at( 2 ), QgsPoint( 5, 5 ) );
  QCOMPARE( pts.at( 3 ), QgsPoint( 5, 0 ) );

  // setPoint
  QVERIFY( quad4points_valid.setPoint( QgsPoint( -1, -1 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( quad4points_valid.setPoint( QgsPoint( -1, 6 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( quad4points_valid.setPoint( QgsPoint( 6, 6 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( quad4points_valid.setPoint( QgsPoint( 6, -1 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( quad4points_valid.isValid() );
  pts = quad4points_valid.points();
  QCOMPARE( pts.at( 0 ), QgsPoint( -1, -1 ) );
  QCOMPARE( pts.at( 1 ), QgsPoint( -1, 6 ) );
  QCOMPARE( pts.at( 2 ), QgsPoint( 6, 6 ) );
  QCOMPARE( pts.at( 3 ), QgsPoint( 6, -1 ) );

  // invalid: must have same type
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -1, -1, 10 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -1, 6, 10 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 6, 6, 10 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 6, -1, 10 ), QgsQuadrilateral::Point4 ) );

  // invalid self-intersection
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 7, 3 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 3, 7 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 3, -7 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 7, 3 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 3, -7 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -7, 3 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 3, 7 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -7, 3 ), QgsQuadrilateral::Point4 ) );

  // invalid colinear
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 6, -2 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -2, 6 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 6, 7 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -2, -1 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 7, -1 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -1, 7 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -1, -2 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 7, 6 ), QgsQuadrilateral::Point4 ) );

//equals
  QVERIFY( QgsQuadrilateral( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) ) !=
           QgsQuadrilateral( QgsPoint( 0.01, 0.01 ), QgsPoint( 0.01, 5.01 ), QgsPoint( 5.01, 5.01 ), QgsPoint( 5.01, 0.01 ) ) );
  QVERIFY( QgsQuadrilateral( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) ).equals(
             QgsQuadrilateral( QgsPoint( 0.01, 0.01 ), QgsPoint( 0.01, 5.01 ), QgsPoint( 5.01, 5.01 ), QgsPoint( 5.01, 0.01 ) ), 1e-1 ) );
  QVERIFY( QgsQuadrilateral( QgsPoint( 0, 0, 0 ), QgsPoint( 0, 5, -0.02 ), QgsPoint( 5, 5, 0 ), QgsPoint( 5, 0, -0.02 ) ).equals(
             QgsQuadrilateral( QgsPoint( 0.01, 0.01, 0.01 ), QgsPoint( 0.01, 5.01, 0 ), QgsPoint( 5.01, 5.01, -0.01 ), QgsPoint( 5.01, 0.01, 0.04 ) ), 1e-1 ) );

// rectangleFromExtent
  QgsQuadrilateral rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral rectangleFromExtentZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ) ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ) ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ) ), rectangleFromExtent );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ), rectangleFromExtent );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0, 10 ), QgsPoint( 5, 5 ) ), rectangleFromExtentZ );
  QVERIFY( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ) != rectangleFromExtentZ );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ), rectangleFromExtent );

// squareFromDiagonal
  QgsQuadrilateral squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral squareFromDiagonalZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QgsQuadrilateral squareFromDiagonalInv( QgsPoint( 5, 5 ), QgsPoint( 5, 0 ), QgsPoint( 0, 0 ), QgsPoint( 0, 5 ) );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ) ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ) ), squareFromDiagonal );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ) != squareFromDiagonal );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ).equals( squareFromDiagonalInv, 1E-8 ) );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0, 10 ), QgsPoint( 5, 5 ) ), squareFromDiagonalZ );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ) != squareFromDiagonalZ );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ), squareFromDiagonal );

// rectangleFromCenterPoint
  QgsQuadrilateral rectangleFromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral rectangleFromCenterPointZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 2.5, 2.5 ) ), QgsQuadrilateral() ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 5 ) ), rectangleFromCenterPoint ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 0 ) ), rectangleFromCenterPoint ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 5 ) ), rectangleFromCenterPoint ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 0 ) ), rectangleFromCenterPoint ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 5 ) ), rectangleFromCenterPointZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 0 ) ), rectangleFromCenterPointZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 0, 5 ) ), rectangleFromCenterPointZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 0, 0 ) ), rectangleFromCenterPointZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 0, 10 ) ), rectangleFromCenterPoint ) ;

// fromRectangle
  QgsQuadrilateral fromRectangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 0, 0 ), QgsPointXY( 0, 0 ) ) ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 0, 0 ), QgsPointXY( 5, 5 ) ) ), fromRectangle ) ;
  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 5, 5 ), QgsPointXY( 0, 0 ) ) ), fromRectangle ) ;
// rectangleFrom3points
  QgsQuadrilateral rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 5 ), QgsQuadrilateral::Distance ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 5 ), QgsQuadrilateral::Projected ), QgsQuadrilateral() );

  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5, 10 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );

  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4, 10 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 4, 10 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 5 ), QgsPoint( 0, 5, 5 ), QgsPoint( 5, 5, 0 ), QgsQuadrilateral::Projected ).toString( 2 ),
            QString( "Quadrilateral (Point 1: PointZ (0 0 5), Point 2: PointZ (0 5 5), Point 3: PointZ (5 5 0), Point 4: PointZ (5 0 0))" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 5 ), QgsPoint( 0, 5, 5 ), QgsPoint( 5, 5, 10 ), QgsQuadrilateral::Projected ).toString( 2 ),
            QString( "Quadrilateral (Point 1: PointZ (0 0 5), Point 2: PointZ (0 5 5), Point 3: PointZ (5 5 10), Point 4: PointZ (5 0 10))" ) );
// toString
  QCOMPARE( QgsQuadrilateral( ).toString(), QString( "Empty" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 2.5, 2.5 ) ).toString(), QString( "Empty" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 0 ) ).toString(), QString( "Quadrilateral (Point 1: Point (0 0), Point 2: Point (0 5), Point 3: Point (5 5), Point 4: Point (5 0))" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 0 ) ).toString(), QString( "Quadrilateral (Point 1: PointZ (0 0 10), Point 2: PointZ (0 5 10), Point 3: PointZ (5 5 10), Point 4: PointZ (5 0 10))" ) );

// toPolygon / toLineString
  QCOMPARE( quad_init.toPolygon()->asWkt(), QgsPolygon().asWkt() );
  QCOMPARE( quad_init.toLineString()->asWkt(), QgsLineString().asWkt() );
  QgsLineString ext, extZ;
  QgsPolygon polyg, polygZ;
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral quadZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  ext.fromWkt( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" );
  QCOMPARE( quad.toLineString()->asWkt(), ext.asWkt() );
  polyg.fromWkt( "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0))" );
  QCOMPARE( quad.toPolygon()->asWkt(), polyg.asWkt() );

  extZ.fromWkt( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" );
  QCOMPARE( quadZ.toLineString()->asWkt(), extZ.asWkt() );
  QCOMPARE( quadZ.toLineString( true )->asWkt(), ext.asWkt() );
  polygZ.fromWkt( "PolygonZ ((0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10))" );
  QCOMPARE( quadZ.toPolygon()->asWkt(), polygZ.asWkt() );
  QCOMPARE( quadZ.toPolygon( true )->asWkt(), polyg.asWkt() );


// area / perimeter

  QCOMPARE( QgsQuadrilateral().area(), 0.0 );
  QCOMPARE( QgsQuadrilateral().perimeter(), 0.0 );

  QVERIFY( qgsDoubleNear( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).area(), 25.0 ) );
  QVERIFY( qgsDoubleNear( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).perimeter(), 20 ) );
}


QGSTEST_MAIN( TestQgsQuadrilateral )
#include "testqgsquadrilateral.moc"
