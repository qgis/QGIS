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
    void dataStream();
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
  QgsRectangle rect = QgsRectangle::fromWkt( QStringLiteral( "POLYGON((0 0,1 0,1 1,0 1,0 0))" ) );
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
  rect3 = rect2.intersect( &rect1 );
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
  QRectF qRectF = rect1.toRectF();
  QCOMPARE( qRectF.width(), 100.0 );
  QCOMPARE( qRectF.height(), 200.0 );
  QCOMPARE( qRectF.x(), 10.0 );
  QCOMPARE( qRectF.y(), 20.0 );
  QgsRectangle rect4 = QgsRectangle( qRectF );
  QCOMPARE( rect4.toString( 2 ), QString( "10.00,20.00 : 110.00,220.00" ) );

  // 250 wide, 500 high
  QgsRectangle rect2;
  rect2.setXMinimum( 10.0 );
  rect2.setYMinimum( 20.0 );
  rect2.setXMaximum( 260.0 );
  rect2.setYMaximum( 520.0 );

  // Scale by 2.5, keeping bottom left as is.
  QgsPointXY p( 135.0, 270.0 );
  rect1.scale( 2.5, &p );

  QVERIFY( rect2.xMinimum() == rect1.xMinimum() );
  QVERIFY( rect2.yMinimum() == rect1.yMinimum() );
  QVERIFY( rect2.xMaximum() == rect1.xMaximum() );
  QVERIFY( rect2.yMaximum() == rect1.yMaximum() );
  QVERIFY( rect1 == rect2 );
}

void TestQgsRectangle::operators()
{
  QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );
  QgsVector v = QgsVector( 1.0, 2.0 );
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
  QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );

  //convert to and from a QVariant
  QVariant var = QVariant::fromValue( rect1 );
  QVERIFY( var.isValid() );
  QVERIFY( var.canConvert< QgsRectangle >() );
  QVERIFY( !var.canConvert< QgsReferencedRectangle >() );

  QgsRectangle rect2 = qvariant_cast<QgsRectangle>( var );
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
  QVariant var = QVariant::fromValue( rect1 );
  QVERIFY( var.isValid() );

  // not great - we'd ideally like this to pass, but it doesn't:
  // QVERIFY( !var.canConvert< QgsRectangle >() );

  QVERIFY( var.canConvert< QgsReferencedRectangle >() );

  QgsReferencedRectangle rect2 = qvariant_cast<QgsReferencedRectangle>( var );
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

}

void TestQgsRectangle::buffered()
{
  QgsRectangle rect = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );
  QgsRectangle rect1 = rect.buffered( 11 );
  QCOMPARE( rect1.xMinimum(), -1.0 );
  QCOMPARE( rect1.yMinimum(), 9.0 );
  QCOMPARE( rect1.xMaximum(), 121.0 );
  QCOMPARE( rect1.yMaximum(), 231.0 );

  rect = QgsRectangle( -110.0, -220.0, -10.0, -20.0 );
  QgsRectangle rect2 = rect.buffered( 11 );
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

void TestQgsRectangle::dataStream()
{
  QgsRectangle original( 10.1, 20.2, 110.3, 220.4 );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << original;

  QgsRectangle result;
  ds.device()->seek( 0 );
  ds >> result;

  QCOMPARE( result, original );
}

QGSTEST_MAIN( TestQgsRectangle )
#include "testqgsrectangle.moc"
