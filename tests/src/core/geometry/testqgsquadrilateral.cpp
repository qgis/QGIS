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
    void constructor();
    void constructorEmpty();
    void constructorWhenColinear();
    void constructorWhenAntiParallelogram();
    void fromRectangle();
    void rectangleFromExtent();
    void rectangleFromCenterPoint();
    void rectangleFrom3points();
    void squareFromDiagonal();
    void setPoint();
    void equals();
    void areaPerimeter();
    void toString();
    void toPolygonToLineString();
};

void TestQgsQuadrilateral::constructor()
{
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                         QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QVERIFY( quad.isValid() );

  QgsPointSequence pts = quad.points();

  QCOMPARE( pts.at( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( pts.at( 1 ), QgsPoint( 0, 5 ) );
  QCOMPARE( pts.at( 2 ), QgsPoint( 5, 5 ) );
  QCOMPARE( pts.at( 3 ), QgsPoint( 5, 0 ) );
}

void TestQgsQuadrilateral::constructorEmpty()
{
  QgsQuadrilateral quad;
  QgsPointSequence pts = quad.points();

  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );
  QVERIFY( !quad.isValid() );
}

void TestQgsQuadrilateral::constructorWhenColinear()
{
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                         QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  QVERIFY( !quad.isValid() );

  QgsPointSequence pts = quad.points();

  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );

  QgsQuadrilateral quadXY( QgsPointXY( 0, 0 ), QgsPointXY( 0, 5 ),
                           QgsPointXY( 0, 10 ), QgsPointXY( 10, 10 ) );
  QVERIFY( !quadXY.isValid() );

  pts = quadXY.points();

  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );
}

void TestQgsQuadrilateral::constructorWhenAntiParallelogram()
{
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ),
                         QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QVERIFY( !quad.isValid() );

  QgsPointSequence pts = quad.points();
  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );

  QgsQuadrilateral quadXY( QgsPointXY( 0, 0 ), QgsPointXY( 5, 5 ),
                           QgsPointXY( 5, 0 ), QgsPointXY( 0, 5 ) );
  QVERIFY( !quadXY.isValid() );

  pts = quadXY.points();

  QVERIFY( pts.at( 0 ).isEmpty() );
  QVERIFY( pts.at( 1 ).isEmpty() );
  QVERIFY( pts.at( 2 ).isEmpty() );
  QVERIFY( pts.at( 3 ).isEmpty() );
}

void TestQgsQuadrilateral::fromRectangle()
{
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                         QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );

  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 0, 0 ), QgsPointXY( 0, 0 ) ) ),
            QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 0, 0 ), QgsPointXY( 5, 5 ) ) ),
            quad ) ;
  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 5, 5 ), QgsPointXY( 0, 0 ) ) ),
            quad ) ;
}

void TestQgsQuadrilateral::rectangleFromExtent()
{
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                         QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral quadZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ),
                          QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QgsQuadrilateral quadM( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ),
                          QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointM ),
                          QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointM ),
                          QgsPoint( 5, 0, 10, 20, QgsWkbTypes::PointM ) );
  QgsQuadrilateral quadZM( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ),
                           QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointZM ),
                           QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointZM ),
                           QgsPoint( 5, 0, 10, 20, QgsWkbTypes::PointZM ) );

  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ) ),
            QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ) ),
            QgsQuadrilateral() );

  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ) ),
            quad );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ),
            quad );
  // Z
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0, 10 ), QgsPoint( 5, 5 ) ),
            quadZ );
  QVERIFY( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) )
           != quadZ );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ),
            quad ); // Z and M are only taken from the first point
  // M
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 5, 5 ) ),
            quadM );
  QVERIFY( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointM ) )
           != quadM );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ),
            quad ); // Z and M are only taken from the first point
  // ZM
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 5, 5 ) ),
            quadZM );
  QVERIFY( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10, 20 ) )
           != quadZM );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10, 20 ) ),
            quad ); // Z and M are only taken from the first point
}

void TestQgsQuadrilateral::rectangleFromCenterPoint()
{
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                         QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral quadZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ),
                          QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QgsQuadrilateral quadM( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ),
                          QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointM ),
                          QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointM ),
                          QgsPoint( 5, 0, 10, 20, QgsWkbTypes::PointM ) );
  QgsQuadrilateral quadZM( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ),
                           QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointZM ),
                           QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointZM ),
                           QgsPoint( 5, 0, 10, 20, QgsWkbTypes::PointZM ) );

  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 2.5, 2.5 ) ),
            QgsQuadrilateral() ) ;

  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 5 ) ),
            quad ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 0 ) ),
            quad ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 5 ) ),
            quad ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 0 ) ),
            quad ) ;

  // Z
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 5 ) ),
            quadZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 0 ) ),
            quadZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 0, 5 ) ),
            quadZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 0, 0 ) ),
            quadZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 0, 10 ) ),
            quad ) ;  // Z and M are only taken from the first point
  // M
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 5, 5 ) ),
            quadM ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 5, 0 ) ),
            quadM ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 0, 5 ) ),
            quadM ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 0, 0 ) ),
            quadM ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ) ),
            quad ) ; // Z and M are only taken from the first point

  // ZM
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 5, 5 ) ),
            quadM ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 5, 0 ) ),
            quadM ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 0, 5 ) ),
            quadM ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 0, 0 ) ),
            quadM ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ) ),
            quad ) ; // Z and M are only taken from the first point

}

void TestQgsQuadrilateral::rectangleFrom3points()
{
  QgsQuadrilateral rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                                         QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );

  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 5 ), QgsQuadrilateral::Distance ),
            QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 5 ), QgsQuadrilateral::Projected ),
            QgsQuadrilateral() );

  // Z
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

  // M
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringM (0 0 20, 0 5 20, 5 5 20, 5 0 20, 0 0 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringM (0 0 20, 0 5 20, 5 5 20, 5 0 20, 0 0 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointM ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringM (0 0 20, 0 5 20, 5 5 20, 5 0 20, 0 0 20)" ) );

  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringM (0 0 20, 0 5 20, 5 5 20, 5 0 20, 0 0 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringM (0 0 20, 0 5 20, 5 5 20, 5 0 20, 0 0 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4, 10, 20, QgsWkbTypes::PointM ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringM (0 0 20, 0 5 20, 5 5 20, 5 0 20, 0 0 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 5, 4, 10, 20, QgsWkbTypes::PointM ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringM (0 0 20, 0 5 20, 5 5 20, 5 0 20, 0 0 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 5, 10, QgsWkbTypes::PointM ), QgsPoint( 0, 5, 5, 10, QgsWkbTypes::PointM ), QgsPoint( 5, 5, 0, 20, QgsWkbTypes::PointM ), QgsQuadrilateral::Projected ).toString( 2 ),
            QString( "Quadrilateral (Point 1: PointM (0 0 10), Point 2: PointM (0 5 10), Point 3: PointM (5 5 10), Point 4: PointM (5 0 10))" ) ); // The first M is taken
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 5, 10, QgsWkbTypes::PointM ), QgsPoint( 0, 5, 5, 10, QgsWkbTypes::PointM ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointM ), QgsQuadrilateral::Projected ).toString( 2 ),
            QString( "Quadrilateral (Point 1: PointM (0 0 10), Point 2: PointM (0 5 10), Point 3: PointM (5 5 10), Point 4: PointM (5 0 10))" ) ); // The first M is taken

  // ZM
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZM (0 0 10 20, 0 5 10 20, 5 5 10 20, 5 0 10 20, 0 0 10 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZM (0 0 10 20, 0 5 10 20, 5 5 10 20, 5 0 10 20, 0 0 10 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointZM ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZM (0 0 10 20, 0 5 10 20, 5 5 10 20, 5 0 10 20, 0 0 10 20)" ) );

  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZM (0 0 10 20, 0 5 10 20, 5 5 10 20, 5 0 10 20, 0 0 10 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZM (0 0 10 20, 0 5 10 20, 5 5 10 20, 5 0 10 20, 0 0 10 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4, 10, 20, QgsWkbTypes::PointZM ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZM (0 0 10 20, 0 5 10 20, 5 5 10 20, 5 0 10 20, 0 0 10 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 5, 4, 10, 20, QgsWkbTypes::PointZM ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZM (0 0 10 20, 0 5 10 20, 5 5 10 20, 5 0 10 20, 0 0 10 20)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 5, 10, QgsWkbTypes::PointZM ), QgsPoint( 0, 5, 5, 10, QgsWkbTypes::PointZM ), QgsPoint( 5, 5, 0, 20, QgsWkbTypes::PointZM ), QgsQuadrilateral::Projected ).toString( 2 ),
            QString( "Quadrilateral (Point 1: PointZM (0 0 5 10), Point 2: PointZM (0 5 5 10), Point 3: PointZM (5 5 0 10), Point 4: PointZM (5 0 0 10))" ) ); // The first M is taken
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 5, 10, QgsWkbTypes::PointZM ), QgsPoint( 0, 5, 5, 10, QgsWkbTypes::PointZM ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointZM ), QgsQuadrilateral::Projected ).toString( 2 ),
            QString( "Quadrilateral (Point 1: PointZM (0 0 5 10), Point 2: PointZM (0 5 5 10), Point 3: PointZM (5 5 10 10), Point 4: PointZM (5 0 10 10))" ) ); // The first M is taken

}

void TestQgsQuadrilateral::squareFromDiagonal()
{
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                         QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral quadZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ),
                          QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QgsQuadrilateral quadM( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointM ),
                          QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 5, 0, 10, 20, QgsWkbTypes::PointM ) );
  QgsQuadrilateral quadZM( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 0, 5, 10, 20, QgsWkbTypes::PointZM ),
                           QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 5, 0, 10, 20, QgsWkbTypes::PointZM ) );
  QgsQuadrilateral quadInv( QgsPoint( 5, 5 ), QgsPoint( 5, 0 ),
                            QgsPoint( 0, 0 ), QgsPoint( 0, 5 ) );

  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ) ),
            QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ) ),
            quad );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) )
           != quad );

  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ).equals( quadInv, 1E-8 ) );

  // Z
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0, 10 ), QgsPoint( 5, 5 ) ),
            quadZ );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) )
           != quadZ );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ),
            quad ); // Z and M are only taken from the first point
  // M
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointM ), QgsPoint( 5, 5 ) ),
            quadM );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointM ) )
           != quadM );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointM ) ),
            quad ); // Z and M are only taken from the first point
  // ZM
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0, 10, 20, QgsWkbTypes::PointZM ), QgsPoint( 5, 5 ) ),
            quadZM );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointZM ) )
           != quadZM );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10, 20, QgsWkbTypes::PointZM ) ),
            quad ); // Z and M are only taken from the first point
}

void TestQgsQuadrilateral::setPoint()
{
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                         QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );

  QVERIFY( quad.setPoint( QgsPoint( -1, -1 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( quad.setPoint( QgsPoint( -1, 6 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( quad.setPoint( QgsPoint( 6, 6 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( quad.setPoint( QgsPoint( 6, -1 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( quad.isValid() );

  QgsPointSequence pts = quad.points();

  QCOMPARE( pts.at( 0 ), QgsPoint( -1, -1 ) );
  QCOMPARE( pts.at( 1 ), QgsPoint( -1, 6 ) );
  QCOMPARE( pts.at( 2 ), QgsPoint( 6, 6 ) );
  QCOMPARE( pts.at( 3 ), QgsPoint( 6, -1 ) );

  // invalid: must have same type
  QVERIFY( !quad.setPoint( QgsPoint( -1, -1, 10 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad.setPoint( QgsPoint( -1, 6, 10 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 6, 6, 10 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 6, -1, 10 ), QgsQuadrilateral::Point4 ) );

  // invalid self-intersection
  QVERIFY( !quad.setPoint( QgsPoint( 7, 3 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 3, 7 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 3, -7 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 7, 3 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 3, -7 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad.setPoint( QgsPoint( -7, 3 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 3, 7 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( !quad.setPoint( QgsPoint( -7, 3 ), QgsQuadrilateral::Point4 ) );

  // invalid colinear
  QVERIFY( !quad.setPoint( QgsPoint( 6, -2 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad.setPoint( QgsPoint( -2, 6 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 6, 7 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad.setPoint( QgsPoint( -2, -1 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 7, -1 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad.setPoint( QgsPoint( -1, 7 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad.setPoint( QgsPoint( -1, -2 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( !quad.setPoint( QgsPoint( 7, 6 ), QgsQuadrilateral::Point4 ) );
}

void TestQgsQuadrilateral::equals()
{
  QgsQuadrilateral quad1;
  QgsQuadrilateral quad2;

  QVERIFY( QgsQuadrilateral() == QgsQuadrilateral() );

  quad1 = QgsQuadrilateral( QgsPoint( 0.01, 0.01 ), QgsPoint( 0.01, 5.01 ), QgsPoint( 5.01, 5.01 ), QgsPoint( 5.01, 0.01 ) );
  QVERIFY( QgsQuadrilateral() != quad1 );

  quad1 = QgsQuadrilateral( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  quad2 = QgsQuadrilateral( QgsPoint( 0.01, 0.01 ), QgsPoint( 0.01, 5.01 ), QgsPoint( 5.01, 5.01 ), QgsPoint( 5.01, 0.01 ) );
  QVERIFY( quad1 != quad2 );

  quad1 = QgsQuadrilateral( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  quad2 = QgsQuadrilateral( QgsPoint( 0.01, 0.01 ), QgsPoint( 0.01, 5.01 ), QgsPoint( 5.01, 5.01 ), QgsPoint( 5.01, 0.01 ) );
  QVERIFY( quad1.equals( quad2, 1e-1 ) );

  quad1 = QgsQuadrilateral( QgsPoint( 0, 0, 0 ), QgsPoint( 0, 5, -0.02 ), QgsPoint( 5, 5, 0 ), QgsPoint( 5, 0, -0.02 ) );
  quad2 = QgsQuadrilateral( QgsPoint( 0.01, 0.01, 0.01 ), QgsPoint( 0.01, 5.01, 0 ), QgsPoint( 5.01, 5.01, -0.01 ), QgsPoint( 5.01, 0.01, 0.04 ) );
  QVERIFY( quad1.equals( quad2, 1e-1 ) );

  quad1 = QgsQuadrilateral( QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointM, 0, 5, 0, 1 ),
                            QgsPoint( QgsWkbTypes::PointM, 5, 5, 0, 1 ), QgsPoint( QgsWkbTypes::PointM, 5, 0, 0, 1 ) );
  quad2 = QgsQuadrilateral( QgsPoint( QgsWkbTypes::PointM, 0.01, 0.01, 0, 1.01 ), QgsPoint( QgsWkbTypes::PointM, 0.01, 5.01, 0, 1.01 ),
                            QgsPoint( QgsWkbTypes::PointM, 5.01, 5.01, 0, 1.01 ), QgsPoint( QgsWkbTypes::PointM, 5.01, 0.01, 0, 1.01 ) );
  QVERIFY( quad1.equals( quad2, 1e-1 ) );
}

void TestQgsQuadrilateral::areaPerimeter()
{
  QCOMPARE( QgsQuadrilateral().area(), 0.0 );
  QCOMPARE( QgsQuadrilateral().perimeter(), 0.0 );

  QgsQuadrilateral quad = QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ),
                          QgsQuadrilateral::Projected );
  QVERIFY( qgsDoubleNear( quad.area(), 25.0 ) );
  QVERIFY( qgsDoubleNear( quad.perimeter(), 20 ) );
}

void TestQgsQuadrilateral::toString()
{
  QCOMPARE( QgsQuadrilateral( ).toString(), QString( "Empty" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 2.5, 2.5 ) ).toString(), QString( "Empty" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 0 ) ).toString(),
            QString( "Quadrilateral (Point 1: Point (0 0), Point 2: Point (0 5), Point 3: Point (5 5), Point 4: Point (5 0))" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 0 ) ).toString(),
            QString( "Quadrilateral (Point 1: PointZ (0 0 10), Point 2: PointZ (0 5 10), Point 3: PointZ (5 5 10), Point 4: PointZ (5 0 10))" ) );
}

void TestQgsQuadrilateral::toPolygonToLineString()
{
  QgsQuadrilateral quad;
  QCOMPARE( quad.toPolygon()->asWkt(), QgsPolygon().asWkt() );
  QCOMPARE( quad.toLineString()->asWkt(), QgsLineString().asWkt() );

  QgsLineString ext, extZ;
  QgsPolygon polyg, polygZ;
  QgsQuadrilateral quadZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ),
                          QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  quad = QgsQuadrilateral( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ),
                           QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );

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
}

QGSTEST_MAIN( TestQgsQuadrilateral )
#include "testqgsquadrilateral.moc"
