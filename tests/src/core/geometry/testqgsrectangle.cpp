/***************************************************************************
     testqgsrectangle.cpp
     --------------------------------------
    Date                 : Tue 14 Aug 2012
    Copyright            : (C) 2012 by Magnus Homann
    Email                : magnus at homann dot se
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
//header for class being tested
#include <qgsrectangle.h>
#include <qgspoint.h>
#include "qgslogger.h"
#include "qgsreferencedgeometry.h"

class TestQgsRectangle: public QObject
{
    Q_OBJECT
  private slots:
    void isEmpty();
    void fromWkt();
    void constructor();
    void constructorTwoPoints();
    void set();
    void setXY();
    void fromCenter();
    void manipulate();
    void regression6194();
    void operators();
    void asVariant();
    void referenced();
    void minimal();
    void grow();
    void include();
    void buffered();
    void isFinite();
    void combine();
    void dataStream();
    void scale();
    void snappedToGrid();
    void distanceToPoint();
    void center();
};

void TestQgsRectangle::isEmpty()
{
  QVERIFY( QgsRectangle().isEmpty() );
  QVERIFY( QgsRectangle( 1, 2, 1, 4 ).isEmpty() );
  QVERIFY( QgsRectangle( 2, 1, 4, 1 ).isEmpty() );
  //constructor should normalize
  QVERIFY( !QgsRectangle( 2, 2, 1, 4 ).isEmpty() );
  QVERIFY( !QgsRectangle( 1, 2, 2, 1 ).isEmpty() );

  QgsRectangle r( 2, 2, 3, 4 );
  r.setXMaximum( 1 );
  QVERIFY( r.isEmpty() );
  r = QgsRectangle( 2, 2, 3, 4 );
  r.setYMaximum( 1 );
  QVERIFY( r.isEmpty() );
}

void TestQgsRectangle::fromWkt()
{
  const QgsRectangle rect = QgsRectangle::fromWkt( QStringLiteral( "POLYGON((0 0,1 0,1 1,0 1,0 0))" ) );
  QVERIFY( ! rect.isEmpty() );
  QCOMPARE( rect.xMinimum(), 0.0 );
  QCOMPARE( rect.yMinimum(), 0.0 );
  QCOMPARE( rect.xMaximum(), 1.0 );
  QCOMPARE( rect.yMaximum(), 1.0 );

  QVERIFY( rect == QgsRectangle::fromWkt( rect.asWktPolygon() ) );

  QVERIFY( QgsRectangle::fromWkt( QStringLiteral( "LINESTRING (0 0,1 0,1 1,0 1,0 0)" ) ).isEmpty() );
  QVERIFY( QgsRectangle::fromWkt( QStringLiteral( "MULTIPOLYGON(((0 0,3 0,3 3,0 3,0 0)),((1 1, 1 2, 2 2, 2 1)))" ) ).isEmpty() );
  QVERIFY( QgsRectangle::fromWkt( QStringLiteral( "MULTIPOLYGON(((0 0,3 0,3 3,0 3,0 0), (10 10,13 10,13 13,10 13,10 10)))" ) ).isEmpty() );
  QVERIFY( QgsRectangle::fromWkt( QStringLiteral( "POLYGON((0 0,1 0,1 1,0 1,0 1))" ) ).isEmpty() );
  QVERIFY( QgsRectangle::fromWkt( QStringLiteral( "POLYGON((0 0,1 0,1 1,0 1,0 1))" ) ).isEmpty() );
  QVERIFY( QgsRectangle::fromWkt( QStringLiteral( "POLYGON((0 0,1 0,1 1,0 1))" ) ).isEmpty() );
}

void TestQgsRectangle::constructor()
{
  const QgsRectangle r( 1, 2, 13, 14 );
  QCOMPARE( r.xMinimum(), 1.0 );
  QCOMPARE( r.xMaximum(), 13.0 );
  QCOMPARE( r.yMinimum(), 2.0 );
  QCOMPARE( r.yMaximum(), 14.0 );

  // auto normalized
  const QgsRectangle r2( 13, 14, 1, 2 );
  QCOMPARE( r2.xMinimum(), 1.0 );
  QCOMPARE( r2.xMaximum(), 13.0 );
  QCOMPARE( r2.yMinimum(), 2.0 );
  QCOMPARE( r2.yMaximum(), 14.0 );

  // no normalization
  const QgsRectangle r3( 13, 14, 1, 2, false );
  QCOMPARE( r3.xMinimum(), 13.0 );
  QCOMPARE( r3.xMaximum(), 1.0 );
  QCOMPARE( r3.yMinimum(), 14.0 );
  QCOMPARE( r3.yMaximum(), 2.0 );
}

void TestQgsRectangle::constructorTwoPoints()
{
  const QgsRectangle r( QgsPointXY( 1, 2 ), QgsPointXY( 13, 14 ) );
  QCOMPARE( r.xMinimum(), 1.0 );
  QCOMPARE( r.xMaximum(), 13.0 );
  QCOMPARE( r.yMinimum(), 2.0 );
  QCOMPARE( r.yMaximum(), 14.0 );

  // auto normalized
  const QgsRectangle r2( QgsPointXY( 13, 14 ), QgsPointXY( 1, 2 ) );
  QCOMPARE( r2.xMinimum(), 1.0 );
  QCOMPARE( r2.xMaximum(), 13.0 );
  QCOMPARE( r2.yMinimum(), 2.0 );
  QCOMPARE( r2.yMaximum(), 14.0 );

  // no normalization
  const QgsRectangle r3( QgsPointXY( 13, 14 ), QgsPointXY( 1, 2 ), false );
  QCOMPARE( r3.xMinimum(), 13.0 );
  QCOMPARE( r3.xMaximum(), 1.0 );
  QCOMPARE( r3.yMinimum(), 14.0 );
  QCOMPARE( r3.yMaximum(), 2.0 );
}

void TestQgsRectangle::set()
{
  QgsRectangle r( QgsPointXY( 1, 2 ), QgsPointXY( 13, 14 ) );
  // auto normalized
  r.set( QgsPointXY( 13, 14 ), QgsPointXY( 1, 2 ) );
  QCOMPARE( r.xMinimum(), 1.0 );
  QCOMPARE( r.xMaximum(), 13.0 );
  QCOMPARE( r.yMinimum(), 2.0 );
  QCOMPARE( r.yMaximum(), 14.0 );

  // no normalization
  r.set( QgsPointXY( 13, 14 ), QgsPointXY( 1, 2 ), false );
  QCOMPARE( r.xMinimum(), 13.0 );
  QCOMPARE( r.xMaximum(), 1.0 );
  QCOMPARE( r.yMinimum(), 14.0 );
  QCOMPARE( r.yMaximum(), 2.0 );
}

void TestQgsRectangle::setXY()
{
  QgsRectangle r( QgsPointXY( 111, 112 ), QgsPointXY( 113, 114 ) );
  // auto normalized
  r.set( 13, 14, 1, 2 );
  QCOMPARE( r.xMinimum(), 1.0 );
  QCOMPARE( r.xMaximum(), 13.0 );
  QCOMPARE( r.yMinimum(), 2.0 );
  QCOMPARE( r.yMaximum(), 14.0 );

  // no normalization
  r.set( 13, 14, 1, 2, false );
  QCOMPARE( r.xMinimum(), 13.0 );
  QCOMPARE( r.xMaximum(), 1.0 );
  QCOMPARE( r.yMinimum(), 14.0 );
  QCOMPARE( r.yMaximum(), 2.0 );
}

void TestQgsRectangle::fromCenter()
{
  QgsRectangle rect = QgsRectangle::fromCenterAndSize( QgsPointXY( 12, 21 ), 20, 40 );
  QVERIFY( ! rect.isEmpty() );
  QCOMPARE( rect.xMinimum(), 2.0 );
  QCOMPARE( rect.yMinimum(), 1.0 );
  QCOMPARE( rect.xMaximum(), 22.0 );
  QCOMPARE( rect.yMaximum(), 41.0 );

  rect = QgsRectangle::fromCenterAndSize( QgsPointXY( 12, 21 ), 0, 40 );
  QCOMPARE( rect.xMinimum(), 12.0 );
  QCOMPARE( rect.yMinimum(), 1.0 );
  QCOMPARE( rect.xMaximum(), 12.0 );
  QCOMPARE( rect.yMaximum(), 41.0 );

  rect = QgsRectangle::fromCenterAndSize( QgsPointXY( 12, 21 ), 20, 0 );
  QCOMPARE( rect.xMinimum(), 2.0 );
  QCOMPARE( rect.yMinimum(), 21.0 );
  QCOMPARE( rect.xMaximum(), 22.0 );
  QCOMPARE( rect.yMaximum(), 21.0 );
}

void TestQgsRectangle::manipulate()
{
  // Set up two intersecting rectangles and normalize
  QgsRectangle rect1, rect2, rect3;
  rect1.set( 4.0, 5.0, 1.0, 2.0 );
  rect2.set( 3.0, 3.0, 7.0, 1.0 );
  // Check intersection
  QVERIFY( rect2.intersects( rect1 ) );
  // Create intersection
  rect3 = rect2.intersect( rect1 );
  // Check width and height (real numbers, careful)
  QCOMPARE( rect3.width(), 1.0 );
  QCOMPARE( rect3.height(), 1.0 );
  // And check that the result is contained in both
  QVERIFY( rect1.contains( rect3 ) );
  QVERIFY( rect2.contains( rect3 ) );
  QVERIFY( ! rect2.contains( rect1 ) );

  // Create the union
  rect3.combineExtentWith( rect1 );
  rect3.combineExtentWith( rect2 );
  // Check union
  QVERIFY( rect3 == QgsRectangle( 1.0, 1.0, 7.0, 5.0 ) );
}

void TestQgsRectangle::regression6194()
{
  // 100 wide, 200 high
  QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );

  // Test conversion to QRectF and back
  const QRectF qRectF = rect1.toRectF();
  QCOMPARE( qRectF.width(), 100.0 );
  QCOMPARE( qRectF.height(), 200.0 );
  QCOMPARE( qRectF.x(), 10.0 );
  QCOMPARE( qRectF.y(), 20.0 );
  const QgsRectangle rect4 = QgsRectangle( qRectF );
  QCOMPARE( rect4.toString( 2 ), QString( "10.00,20.00 : 110.00,220.00" ) );

  // 250 wide, 500 high
  QgsRectangle rect2;
  rect2.setXMinimum( 10.0 );
  rect2.setYMinimum( 20.0 );
  rect2.setXMaximum( 260.0 );
  rect2.setYMaximum( 520.0 );

  // Scale by 2.5, keeping bottom left as is.
  const QgsPointXY p( 135.0, 270.0 );
  rect1.scale( 2.5, &p );

  QVERIFY( rect2.xMinimum() == rect1.xMinimum() );
  QVERIFY( rect2.yMinimum() == rect1.yMinimum() );
  QVERIFY( rect2.xMaximum() == rect1.xMaximum() );
  QVERIFY( rect2.yMaximum() == rect1.yMaximum() );
  QVERIFY( rect1 == rect2 );
}

void TestQgsRectangle::operators()
{
  const QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );
  const QgsVector v = QgsVector( 1.0, 2.0 );
  QgsRectangle rect2 = rect1 + v;
  QVERIFY( rect1 != rect2 );
  QCOMPARE( rect2.height(), rect1.height() );
  QCOMPARE( rect2.width(), rect1.width() );
  QCOMPARE( rect2.xMinimum(), 11.0 );
  QCOMPARE( rect2.yMinimum(), 22.0 );

  rect2 -= rect2.center() - rect1.center();
  QVERIFY( rect1 == rect2 );

  rect2 += v * 2.5;
  QCOMPARE( rect2.xMinimum(), 12.5 );
  QCOMPARE( rect2.yMinimum(), 25.0 );

  rect2 = rect1 - v;
  QCOMPARE( rect2.xMinimum(), 9.0 );
  QCOMPARE( rect2.yMinimum(), 18.0 );
  QCOMPARE( rect2.height(), rect1.height() );
  QCOMPARE( rect2.width(), rect1.width() );
}

void TestQgsRectangle::asVariant()
{
  const QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( rect1 );
  QVERIFY( var.isValid() );
  QVERIFY( var.canConvert< QgsRectangle >() );
  QVERIFY( !var.canConvert< QgsReferencedRectangle >() );

  const QgsRectangle rect2 = qvariant_cast<QgsRectangle>( var );
  QCOMPARE( rect2.xMinimum(), rect1.xMinimum() );
  QCOMPARE( rect2.yMinimum(), rect1.yMinimum() );
  QCOMPARE( rect2.height(), rect1.height() );
  QCOMPARE( rect2.width(), rect1.width() );
}

void TestQgsRectangle::referenced()
{
  QgsReferencedRectangle rect1 = QgsReferencedRectangle( QgsRectangle( 10.0, 20.0, 110.0, 220.0 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  QCOMPARE( rect1.crs().authid(), QStringLiteral( "EPSG:3111" ) );
  rect1.setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ) );
  QCOMPARE( rect1.crs().authid(), QStringLiteral( "EPSG:28356" ) );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( rect1 );
  QVERIFY( var.isValid() );

  // not great - we'd ideally like this to pass, but it doesn't:
  // QVERIFY( !var.canConvert< QgsRectangle >() );

  QVERIFY( var.canConvert< QgsReferencedRectangle >() );

  const QgsReferencedRectangle rect2 = qvariant_cast<QgsReferencedRectangle>( var );
  QCOMPARE( rect2.xMinimum(), rect1.xMinimum() );
  QCOMPARE( rect2.yMinimum(), rect1.yMinimum() );
  QCOMPARE( rect2.height(), rect1.height() );
  QCOMPARE( rect2.width(), rect1.width() );
  QCOMPARE( rect2.crs().authid(), QStringLiteral( "EPSG:28356" ) );
}

void TestQgsRectangle::minimal()
{
  QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );
  rect1.setMinimal();
  QVERIFY( rect1.isEmpty() );
  QVERIFY( rect1.isNull() );
}

void TestQgsRectangle::grow()
{
  QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );
  rect1.grow( 11 );
  QCOMPARE( rect1.xMinimum(), -1.0 );
  QCOMPARE( rect1.yMinimum(), 9.0 );
  QCOMPARE( rect1.xMaximum(), 121.0 );
  QCOMPARE( rect1.yMaximum(), 231.0 );

  QgsRectangle rect2 = QgsRectangle( -110.0, -220.0, -10.0, -20.0 );
  rect2.grow( 11 );
  QCOMPARE( rect2.xMinimum(), -121.0 );
  QCOMPARE( rect2.yMinimum(), -231.0 );
  QCOMPARE( rect2.xMaximum(), 1.0 );
  QCOMPARE( rect2.yMaximum(), -9.0 );
}

void TestQgsRectangle::include()
{
  QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );
  // inside
  rect1.include( QgsPointXY( 15, 50 ) );
  QCOMPARE( rect1.xMinimum(), 10.0 );
  QCOMPARE( rect1.yMinimum(), 20.0 );
  QCOMPARE( rect1.xMaximum(), 110.0 );
  QCOMPARE( rect1.yMaximum(), 220.0 );

  rect1.include( QgsPointXY( 5, 50 ) );
  QCOMPARE( rect1.xMinimum(), 5.0 );
  QCOMPARE( rect1.yMinimum(), 20.0 );
  QCOMPARE( rect1.xMaximum(), 110.0 );
  QCOMPARE( rect1.yMaximum(), 220.0 );

  rect1.include( QgsPointXY( 15, 12 ) );
  QCOMPARE( rect1.xMinimum(), 5.0 );
  QCOMPARE( rect1.yMinimum(), 12.0 );
  QCOMPARE( rect1.xMaximum(), 110.0 );
  QCOMPARE( rect1.yMaximum(), 220.0 );

  rect1.include( QgsPointXY( 115, 12 ) );
  QCOMPARE( rect1.xMinimum(), 5.0 );
  QCOMPARE( rect1.yMinimum(), 12.0 );
  QCOMPARE( rect1.xMaximum(), 115.0 );
  QCOMPARE( rect1.yMaximum(), 220.0 );

  rect1.include( QgsPointXY( 115, 242 ) );
  QCOMPARE( rect1.xMinimum(), 5.0 );
  QCOMPARE( rect1.yMinimum(), 12.0 );
  QCOMPARE( rect1.xMaximum(), 115.0 );
  QCOMPARE( rect1.yMaximum(), 242.0 );

  rect1.setMinimal();

  rect1.include( QgsPointXY( 15, 50 ) );
  QCOMPARE( rect1.xMinimum(), 15.0 );
  QCOMPARE( rect1.yMinimum(), 50.0 );
  QCOMPARE( rect1.xMaximum(), 15.0 );
  QCOMPARE( rect1.yMaximum(), 50.0 );

  rect1.include( QgsPointXY( 5, 30 ) );
  QCOMPARE( rect1.xMinimum(), 5.0 );
  QCOMPARE( rect1.yMinimum(), 30.0 );
  QCOMPARE( rect1.xMaximum(), 15.0 );
  QCOMPARE( rect1.yMaximum(), 50.0 );

  rect1.setMinimal();

  rect1.include( QgsPointXY( 5, 30 ) );
  QCOMPARE( rect1.xMinimum(), 5.0 );
  QCOMPARE( rect1.yMinimum(), 30.0 );
  QCOMPARE( rect1.xMaximum(), 5.0 );
  QCOMPARE( rect1.yMaximum(), 30.0 );

  rect1.include( QgsPointXY( 15, 50 ) );
  QCOMPARE( rect1.xMinimum(), 5.0 );
  QCOMPARE( rect1.yMinimum(), 30.0 );
  QCOMPARE( rect1.xMaximum(), 15.0 );
  QCOMPARE( rect1.yMaximum(), 50.0 );
}

void TestQgsRectangle::buffered()
{
  QgsRectangle rect = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );
  const QgsRectangle rect1 = rect.buffered( 11 );
  QCOMPARE( rect1.xMinimum(), -1.0 );
  QCOMPARE( rect1.yMinimum(), 9.0 );
  QCOMPARE( rect1.xMaximum(), 121.0 );
  QCOMPARE( rect1.yMaximum(), 231.0 );

  rect = QgsRectangle( -110.0, -220.0, -10.0, -20.0 );
  const QgsRectangle rect2 = rect.buffered( 11 );
  QCOMPARE( rect2.xMinimum(), -121.0 );
  QCOMPARE( rect2.yMinimum(), -231.0 );
  QCOMPARE( rect2.xMaximum(), 1.0 );
  QCOMPARE( rect2.yMaximum(), -9.0 );
}

void TestQgsRectangle::isFinite()
{
  QVERIFY( QgsRectangle( 1, 2, 3, 4 ).isFinite() );
  QVERIFY( !QgsRectangle( std::numeric_limits<double>::infinity(), 2, 3, 4 ).isFinite() );
  QVERIFY( !QgsRectangle( 1, std::numeric_limits<double>::infinity(), 3, 4 ).isFinite() );
  QVERIFY( !QgsRectangle( 1, 2, std::numeric_limits<double>::infinity(), 4 ).isFinite() );
  QVERIFY( !QgsRectangle( 1, 2, 3, std::numeric_limits<double>::infinity() ).isFinite() );
  QVERIFY( !QgsRectangle( std::numeric_limits<double>::quiet_NaN(), 2, 3, 4 ).isFinite() );
  QVERIFY( !QgsRectangle( 1, std::numeric_limits<double>::quiet_NaN(), 3, 4 ).isFinite() );
  QVERIFY( !QgsRectangle( 1, 2, std::numeric_limits<double>::quiet_NaN(), 4 ).isFinite() );
  QVERIFY( !QgsRectangle( 1, 2, 3, std::numeric_limits<double>::quiet_NaN() ).isFinite() );
}

void TestQgsRectangle::combine()
{
  QgsRectangle rect;
  // combine extent of null rectangle with valid rectangle
  rect.combineExtentWith( QgsRectangle( 1, 2, 3, 4 ) );
  QCOMPARE( rect.xMinimum(), 1.0 );
  QCOMPARE( rect.yMinimum(), 2.0 );
  QCOMPARE( rect.xMaximum(), 3.0 );
  QCOMPARE( rect.yMaximum(), 4.0 );

  // combine extent of valid rectangle with null rectangle
  rect = QgsRectangle( 1, 2, 3, 4 );
  rect.combineExtentWith( QgsRectangle() );
  QCOMPARE( rect.xMinimum(), 1.0 );
  QCOMPARE( rect.yMinimum(), 2.0 );
  QCOMPARE( rect.xMaximum(), 3.0 );
  QCOMPARE( rect.yMaximum(), 4.0 );
}

void TestQgsRectangle::dataStream()
{
  const QgsRectangle original( 10.1, 20.2, 110.3, 220.4 );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << original;

  QgsRectangle result;
  ds.device()->seek( 0 );
  ds >> result;

  QCOMPARE( result, original );
}

void TestQgsRectangle::scale()
{
  QgsRectangle rect( 10, 20, 30, 60 );
  rect.scale( 2 );
  QCOMPARE( rect, QgsRectangle( 0, 0, 40, 80 ) );
  rect.scale( .5 );
  QCOMPARE( rect, QgsRectangle( 10, 20, 30, 60 ) );
  const QgsPointXY center( 10, 20 );

  // with center
  rect.scale( 2, &center );
  QCOMPARE( rect, QgsRectangle( -10, -20, 30, 60 ) );

  // scaled
  QCOMPARE( rect, QgsRectangle( 10, 20, 30, 60 ).scaled( 2, &center ) );
}

void TestQgsRectangle::snappedToGrid()
{
  const QgsRectangle original( 10.123, 20.333, 10.788, 20.788 );
  const QgsRectangle snapped = original.snappedToGrid( 0.1 );

  const QgsRectangle control( 10.1, 20.3, 10.8, 20.8 );

  QVERIFY( qgsDoubleNear( snapped.xMinimum(), control.xMinimum(), 0.000001 ) );
  QVERIFY( qgsDoubleNear( snapped.xMaximum(), control.xMaximum(), 0.000001 ) );
  QVERIFY( qgsDoubleNear( snapped.yMinimum(), control.yMinimum(), 0.000001 ) );
  QVERIFY( qgsDoubleNear( snapped.yMaximum(), control.yMaximum(), 0.000001 ) );

  QCOMPARE( QgsRectangle().snappedToGrid( 0.1 ), QgsRectangle() );
}

void TestQgsRectangle::distanceToPoint()
{
  const QgsRectangle rect( 10, 100, 20, 110 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 10, 100 ) ), 0.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 20, 100 ) ), 0.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 15, 100 ) ), 0.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 10, 110 ) ), 0.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 20, 110 ) ), 0.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 15, 110 ) ), 0.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 10, 105 ) ), 0.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 20, 100 ) ), 0.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 0, 100 ) ), 10.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 35, 100 ) ), 15.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 15, 95 ) ), 5.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 15, 120 ) ), 10.0, 0.000000001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 5, 95 ) ), 7.071068, 0.00001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 25, 95 ) ), 7.071068, 0.00001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 5, 115 ) ), 7.071068, 0.00001 );
  QGSCOMPARENEAR( rect.distance( QgsPointXY( 25, 115 ) ), 7.071068, 0.00001 );
}

void TestQgsRectangle::center()
{
  QgsRectangle rect( 10, 100, 20, 110 );
  QCOMPARE( rect.center().x(), 15.0 );
  QCOMPARE( rect.center().y(), 105.0 );
  rect = QgsRectangle( 10, 100, 10, 100 );
  QCOMPARE( rect.center().x(), 10.0 );
  QCOMPARE( rect.center().y(), 100.0 );
  rect = QgsRectangle( -10, -100, 10, 100 );
  QCOMPARE( rect.center().x(), 0.0 );
  QCOMPARE( rect.center().y(), 0.0 );
  // a "maximal" rect
  rect = QgsRectangle( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
  QCOMPARE( rect.center().x(), 0.0 );
  QCOMPARE( rect.center().y(), 0.0 );
}

QGSTEST_MAIN( TestQgsRectangle )
#include "testqgsrectangle.moc"
