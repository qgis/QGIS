/***************************************************************************
     testqgspointxy.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
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
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

//qgis includes...
#include <qgsapplication.h>
#include <qgsgeometry.h>
//header for class being tested
#include <qgspoint.h>
#include "qgsreferencedgeometry.h"

class TestQgsPointXY: public QgsTest
{
    Q_OBJECT
  public:

    TestQgsPointXY() : QgsTest( QStringLiteral( "QgsPointXY Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void equality();
    void gettersSetters();
    void constructors();
    void toQPointF();
    void operators();
    void toString();
    void asWkt();
    void sqrDist();
    void distance();
    void compare();
    void project();
    void vector(); //tests for QgsVector
    void asVariant();
    void referenced();
    void isEmpty();

  private:
    QgsPointXY mPoint1;
    QgsPointXY mPoint2;
    QgsPointXY mPoint3;
    QgsPointXY mPoint4;
};

void TestQgsPointXY::init()
{
  //
  // Reset / reinitialize the geometries before each test is run
  //
  mPoint1 = QgsPointXY( 20.0, -20.0 );
  mPoint2 = QgsPointXY( -80.0, 20.0 );
  mPoint3 = QgsPointXY( -80.0, -20.0 );
  mPoint4 = QgsPointXY( 80.0, 20.0 );
}

void TestQgsPointXY::cleanup()
{
  // will be called after every testfunction.
}

void TestQgsPointXY::equality()
{
  const QgsPointXY point1( 5.0, 9.0 );
  const QgsPointXY point2( 5.0, 9.0 );
  QCOMPARE( point1, point2 );
  QgsPointXY point3( 5.0, 6.0 );
  QVERIFY( !( point3 == point1 ) );
  QVERIFY( point3 != point1 );
  QgsPointXY point4( 8.0, 9.0 );
  QVERIFY( !( point4 == point1 ) );
  QVERIFY( point4 != point1 );
  QVERIFY( !( point4 == point3 ) );
  QVERIFY( point4 != point3 );

  QVERIFY( QgsPointXY() != point1 );
  QVERIFY( QgsPointXY() != QgsPointXY( 0, 0 ) );
  QVERIFY( point1 != QgsPointXY() );
  QVERIFY( QgsPointXY( 0, 0 ) != QgsPointXY() );
}

void TestQgsPointXY::gettersSetters()
{
  QgsPointXY point;
  point.setX( 1.0 );
  QCOMPARE( point.x(), 1.0 );
  point.setY( 2.0 );
  QCOMPARE( point.y(), 2.0 );
  point.set( 3.0, 4.0 );
  QCOMPARE( point.x(), 3.0 );
  QCOMPARE( point.y(), 4.0 );
}

void TestQgsPointXY::constructors()
{
  const QgsPointXY point1 = QgsPointXY( 20.0, -20.0 );
  QCOMPARE( point1.x(), 20.0 );
  QCOMPARE( point1.y(), -20.0 );
  const QgsPointXY &point2( point1 );
  QCOMPARE( point2, point1 );

  const QPointF sourceQPointF( 20.0, -20.0 );
  const QgsPointXY fromQPointF( sourceQPointF );
  QCOMPARE( fromQPointF.x(), 20.0 );
  QCOMPARE( fromQPointF.y(), -20.0 );

  const QPointF sourceQPoint( 20, -20 );
  const QgsPointXY fromQPoint( sourceQPoint );
  QCOMPARE( fromQPoint.x(), 20.0 );
  QCOMPARE( fromQPoint.y(), -20.0 );
}

void TestQgsPointXY::toQPointF()
{
  const QgsPointXY point( 20.0, -20.0 );
  const QPointF result = point.toQPointF();
  QCOMPARE( result.x(), 20.0 );
  QCOMPARE( result.y(), -20.0 );
}

void TestQgsPointXY::operators()
{
  QgsPointXY p( 1, 2 );
  QCOMPARE( p - QgsVector( 3, 5 ), QgsPointXY( -2, -3 ) );
  p -= QgsVector( 3, 5 );
  QCOMPARE( p, QgsPointXY( -2, -3 ) );

  p = QgsPointXY( 1, 2 );
  QCOMPARE( p + QgsVector( 3, 5 ), QgsPointXY( 4, 7 ) );
  p += QgsVector( 3, 5 );
  QCOMPARE( p, QgsPointXY( 4, 7 ) );

  p = QgsPointXY( 1, 2 );
  QCOMPARE( p * 3, QgsPointXY( 3, 6 ) );
  p *= 3;
  QCOMPARE( p, QgsPointXY( 3, 6 ) );

  QCOMPARE( p / 3.0, QgsPointXY( 1, 2 ) );
  p /= 3;
  QCOMPARE( p, QgsPointXY( 1, 2 ) );
}

void TestQgsPointXY::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::showSettings();
}

void TestQgsPointXY::toString()
{
  QCOMPARE( mPoint1.toString( 2 ), QString( "20.00,-20.00" ) );
  QCOMPARE( QgsPointXY().toString( 2 ), QString( "0.00,0.00" ) );
}

void TestQgsPointXY::asWkt()
{
  QCOMPARE( QgsPointXY().asWkt(), QString( "POINT EMPTY" ) );
  QCOMPARE( mPoint1.asWkt(), QString( "POINT(20 -20)" ) );
}

void TestQgsPointXY::sqrDist()
{
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( QgsPointXY( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( 2, 2 ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( QgsPointXY( 3, 2 ) ), 4.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( 3, 2 ), 4.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( QgsPointXY( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( 1, 3 ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( QgsPointXY( 1, 4 ) ), 4.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( 1, 4 ), 4.0 );
  QCOMPARE( QgsPointXY( 1, -2 ).sqrDist( QgsPointXY( 1, -4 ) ), 4.0 );
  QCOMPARE( QgsPointXY( 1, -2 ).sqrDist( 1, -4 ), 4.0 );
}

void TestQgsPointXY::distance()
{
  QCOMPARE( QgsPointXY( 1, 2 ).distance( QgsPointXY( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( 2, 2 ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( QgsPointXY( 3, 2 ) ), 2.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( 3, 2 ), 2.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( QgsPointXY( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( 1, 3 ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( QgsPointXY( 1, 4 ) ), 2.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( 1, 4 ), 2.0 );
  QCOMPARE( QgsPointXY( 1, -2 ).distance( QgsPointXY( 1, -4 ) ), 2.0 );
  QCOMPARE( QgsPointXY( 1, -2 ).distance( 1, -4 ), 2.0 );
}

void TestQgsPointXY::compare()
{
  const QgsPointXY point1( 5.000000000001, 9.0 );
  const QgsPointXY point2( 5.0, 8.999999999999999 );
  QVERIFY( point1.compare( point2, 0.00000001 ) );
  const QgsPointXY point3( 5.0, 6.0 );
  QVERIFY( !( point3.compare( point1 ) ) );
  const QgsPointXY point4( 10 / 3.0, 12 / 7.0 );
  QVERIFY( point4.compare( QgsPointXY( 10 / 3.0, 12 / 7.0 ) ) );
}

void TestQgsPointXY::project()
{
  // test projecting a point
  const QgsPointXY p( 1, 2 );
  QVERIFY( p.project( 1, 0 ).compare( QgsPointXY( 1, 3 ), 0.0000000001 ) );
  QVERIFY( p.project( 1.5, 90 ).compare( QgsPointXY( 2.5, 2 ), 0.0000000001 ) );
  QVERIFY( p.project( 2, 180 ).compare( QgsPointXY( 1, 0 ), 0.0000000001 ) );
  QVERIFY( p.project( 5, 270 ).compare( QgsPointXY( -4, 2 ), 0.0000000001 ) );
  QVERIFY( p.project( 6, 360 ).compare( QgsPointXY( 1, 8 ), 0.0000000001 ) );
  QVERIFY( p.project( 5, 450 ).compare( QgsPointXY( 6, 2 ), 0.0000000001 ) );
  QVERIFY( p.project( -1, 0 ).compare( QgsPointXY( 1, 1 ), 0.0000000001 ) );
  QVERIFY( p.project( 1.5, -90 ).compare( QgsPointXY( -0.5, 2 ), 0.0000000001 ) );
}

void TestQgsPointXY::vector()
{
  //equality
  QVERIFY( QgsVector( 1, 2 ) == QgsVector( 1, 2 ) );
  QVERIFY( QgsVector( 1, 2 ) != QgsVector( 3, 2 ) );
  QVERIFY( QgsVector( 1, 2 ) != QgsVector( 1, 3 ) );

  //test constructors, x(), y() accessors
  QgsVector v1;
  QCOMPARE( v1.x(), 0.0 );
  QCOMPARE( v1.y(), 0.0 );
  QgsVector v2( 1.0, 2.0 );
  QCOMPARE( v2.x(), 1.0 );
  QCOMPARE( v2.y(), 2.0 );

  // operator-
  QCOMPARE( ( -v2 ).x(), -1.0 );
  QCOMPARE( ( -v2 ).y(), -2.0 );

  // operator*
  QCOMPARE( ( v2 * 2.0 ).x(), 2.0 );
  QCOMPARE( ( v2 * 2.0 ).y(), 4.0 );

  // operator/
  QCOMPARE( ( v2 / 2.0 ).x(), 0.5 );
  QCOMPARE( ( v2 / 2.0 ).y(), 1.0 );

  // QgsVector * QgsVector
  QCOMPARE( ( v2 * v2 ), 5.0 );

  // length
  QCOMPARE( v1.length(), 0.0 );
  QGSCOMPARENEAR( v2.length(), std::sqrt( 5.0 ), 0.000000001 );
  QCOMPARE( v2.lengthSquared(), 5.0 );

  // perpVector
  QCOMPARE( QgsVector( 2, 3 ).perpVector().x(), -3.0 );
  QCOMPARE( QgsVector( 2, 3 ).perpVector().y(), 2.0 );

  // angle
  QGSCOMPARENEAR( QgsVector( 0, 1 ).angle(), M_PI_2, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 1, 0 ).angle(), 0, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( -1, 0 ).angle(), M_PI, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 0, -1 ).angle(), 3 * M_PI_2, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 0, 0 ).angle(), 0, 0.0000001 );

  QGSCOMPARENEAR( QgsVector( 0, 1 ).angle( QgsVector( 0, 1 ) ), 0, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 1, 0 ).angle( QgsVector( 0, 1 ) ), M_PI_2, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 0, 1 ).angle( QgsVector( -1, 0 ) ), M_PI_2, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 1, 0 ).angle( QgsVector( -1, 0 ) ), M_PI, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( -1, 0 ).angle( QgsVector( 0, 0 ) ), -M_PI, 0.0000001 );

  // rotateBy
  QGSCOMPARENEAR( QgsVector( 0, 1 ).rotateBy( M_PI_2 ).x(), -1.0, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 0, 1 ).rotateBy( M_PI_2 ).y(), 0.0, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 0, 1 ).rotateBy( M_PI ).x(), 0.0, 0.0000001 );
  QGSCOMPARENEAR( QgsVector( 0, 1 ).rotateBy( M_PI ).y(), -1.0, 0.0000001 );

  // normalized
  QCOMPARE( QgsVector( 0, 2 ).normalized().x(), 0.0 );
  QCOMPARE( QgsVector( 0, 2 ).normalized().y(), 1.0 );
  QCOMPARE( QgsVector( 2, 0 ).normalized().x(), 1.0 );
  QCOMPARE( QgsVector( 2, 0 ).normalized().y(), 0.0 );

  // operator +, -
  v1 = QgsVector( 1, 3 );
  v2 = QgsVector( 2, 5 );
  QgsVector v3 = v1 + v2;
  QCOMPARE( v3.x(), 3.0 );
  QCOMPARE( v3.y(), 8.0 );
  v3 = v1 - v2;
  QCOMPARE( v3.x(), -1.0 );
  QCOMPARE( v3.y(), -2.0 );
  // operator +=, -=
  v1 += v2;
  QCOMPARE( v1.x(), 3.0 );
  QCOMPARE( v1.y(), 8.0 );
  v1 -= v2;
  QCOMPARE( v1.x(), 1.0 );
  QCOMPARE( v1.y(), 3.0 );

  // 2d cross product
  QCOMPARE( QgsVector( 1, 3 ).crossProduct( QgsVector( 6, 9 ) ), -9.0 );
}

void TestQgsPointXY::asVariant()
{
  const QgsPointXY p1 = QgsPointXY( 10.0, 20.0 );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( p1 );
  QVERIFY( var.isValid() );
  QCOMPARE( var.userType(), QMetaType::type( "QgsPointXY" ) );

  const QgsPointXY p2 = qvariant_cast<QgsPointXY>( var );
  QCOMPARE( p2.x(), p1.x() );
  QCOMPARE( p2.y(), p1.y() );
}

void TestQgsPointXY::referenced()
{
  QgsReferencedPointXY p1 = QgsReferencedPointXY( QgsPointXY( 10.0, 20.0 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  QCOMPARE( p1.crs().authid(), QStringLiteral( "EPSG:3111" ) );
  p1.setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ) );
  QCOMPARE( p1.crs().authid(), QStringLiteral( "EPSG:28356" ) );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( p1 );
  QVERIFY( var.isValid() );

  // not great - we'd ideally like this to pass, but it doesn't:
  // QVERIFY( !var.canConvert< QgsPointXY >() );

  QCOMPARE( var.userType(), QMetaType::type( "QgsReferencedPointXY" ) );

  const QgsReferencedPointXY p2 = qvariant_cast<QgsReferencedPointXY>( var );
  QCOMPARE( p2.x(), p1.x() );
  QCOMPARE( p2.y(), p1.y() );
  QCOMPARE( p2.crs().authid(), QStringLiteral( "EPSG:28356" ) );
}

void TestQgsPointXY::isEmpty()
{
  QgsPointXY pointEmpty;
  QVERIFY( pointEmpty.isEmpty() );
  QCOMPARE( pointEmpty.x(), 0.0 );
  QCOMPARE( pointEmpty.y(), 0.0 );
  pointEmpty.setX( 7 );
  QVERIFY( ! pointEmpty.isEmpty() );
  QCOMPARE( pointEmpty.x(), 7.0 );
  QCOMPARE( pointEmpty.y(), 0.0 );
  pointEmpty = QgsPointXY();
  QVERIFY( pointEmpty.isEmpty() );
  QCOMPARE( pointEmpty.x(), 0.0 );
  QCOMPARE( pointEmpty.y(), 0.0 );
  pointEmpty.setY( 4 );
  QVERIFY( ! pointEmpty.isEmpty() );
  QCOMPARE( pointEmpty.x(), 0.0 );
  QCOMPARE( pointEmpty.y(), 4.0 );

  QVERIFY( QgsPointXY( QgsPoint() ).isEmpty() );
  // "can't" be empty
  QVERIFY( ! QgsPointXY( QPoint() ).isEmpty() );
  QVERIFY( ! QgsPointXY( QPointF() ).isEmpty() );
}

QGSTEST_MAIN( TestQgsPointXY )
#include "testqgspointxy.moc"
