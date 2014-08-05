/***************************************************************************
     testqgsgeometryimport.cpp
     --------------------------------------
    Date                 : 03 Sept 2014
    Copyright            : (C) 2014 by Marco Hugentobler
    Email                : marco@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgspoint.h"
#include <QPolygonF>

#include <QtTest>
#include <QObject>

class TestQgsGeometryImport: public QObject
{
    Q_OBJECT

  private slots:
    void pointWkt_data();
    void pointWkt();

    void pointWkb_data();
    void pointWkb();

    void pointGeos_data();
    void pointGeos();

    void linestringWkt_data();
    void linestringWkt();

    void linestringWkb_data();
    void linestringWkb();

    void linestringGeos_data();
    void linestringGeos();

  private:
    bool compareLineStrings( const QgsPolyline& polyline, QVariantList& line );
};

void TestQgsGeometryImport::pointWkt_data()
{
  QTest::addColumn<QString>( "wktString" );
  QTest::addColumn<double>( "x" );
  QTest::addColumn<double>( "y" );

  QTest::newRow( "point_wkt_1" ) << "POINT(30 10)" << 30.0 << 10.0;
}

void TestQgsGeometryImport::pointWkt()
{
  QFETCH( QString, wktString );
  QFETCH( double, x );
  QFETCH( double, y );

  QgsGeometry* geom = QgsGeometry::fromWkt( wktString );

  QCOMPARE( geom->wkbType(), QGis::WKBPoint );
  QgsPoint point = geom->asPoint();
  delete geom;

  QVERIFY( qgsDoubleNear( point.x(), x ) );
  QVERIFY( qgsDoubleNear( point.y(), y ) );
}

void TestQgsGeometryImport::pointWkb_data()
{
  QTest::addColumn<double>( "x" );
  QTest::addColumn<double>( "y" );

  QTest::newRow( "point_wkb_1" ) << 30.0 << 10.0;
}

void TestQgsGeometryImport::pointWkb()
{
  QFETCH( double, x );
  QFETCH( double, y );

  //create wkb
  char byteOrder = QgsApplication::endian();
  unsigned char* geomPtr = new unsigned char[21];
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder << QGis::WKBPoint << x << y;

  QgsGeometry geom;
  geom.fromWkb( geomPtr, 21 );
  QgsPoint point = geom.asPoint();

  QCOMPARE( geom.wkbType(), QGis::WKBPoint );
  QVERIFY( qgsDoubleNear( point.x(), x ) );
  QVERIFY( qgsDoubleNear( point.y(), y ) );
}

void TestQgsGeometryImport::pointGeos_data()
{
  QTest::addColumn<double>( "x" );
  QTest::addColumn<double>( "y" );

  QTest::newRow( "point_geos_1" ) << 30.0 << 10.0;
}

void TestQgsGeometryImport::pointGeos()
{
  QFETCH( double, x );
  QFETCH( double, y );

  GEOSContextHandle_t geosctxt = QgsGeometry::getGEOSHandler();

  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
  GEOSCoordSeq_setX_r( geosctxt, coord, 0, x );
  GEOSCoordSeq_setY_r( geosctxt, coord, 0, y );
  GEOSGeometry* geosPt = GEOSGeom_createPoint_r( geosctxt, coord );

  QgsGeometry geom;
  geom.fromGeos( geosPt );
  QVERIFY( geom.wkbType() == QGis::WKBPoint );

  QgsPoint geomPt = geom.asPoint();

  QVERIFY( qgsDoubleNear( x, geomPt.x() ) );
  QVERIFY( qgsDoubleNear( y, geomPt.y() ) );
}

void TestQgsGeometryImport::linestringWkt_data()
{
  QTest::addColumn<QString>( "wktString" );
  QTest::addColumn<QVariantList>( "line" );

  QVariantList line;
  line << QVariant( QPointF( 30.0, 10.0 ) ) << QVariant( QPointF( 10.0, 30.0 ) ) << QVariant( QPointF( 40.0, 40.0 ) );

  QTest::newRow( "linestring_wkt_1" ) << "LINESTRING (30 10, 10 30, 40 40)" << line;
}

void TestQgsGeometryImport::linestringWkt()
{
  QFETCH( QString, wktString );
  QFETCH( QVariantList, line );

  QgsGeometry* geom = QgsGeometry::fromWkt( wktString );
  QCOMPARE( geom->wkbType(), QGis::WKBLineString );

  QgsPolyline polyLine = geom->asPolyline();
  QVERIFY( compareLineStrings( polyLine, line ) );
  delete geom;
}

void TestQgsGeometryImport::linestringWkb_data()
{
  QTest::addColumn<QVariantList>( "line" );
  QVariantList line;
  line << QVariant( QPointF( 30.0, 10.0 ) ) << QVariant( QPointF( 10.0, 30.0 ) ) << QVariant( QPointF( 40.0, 40.0 ) );
  QTest::newRow( "linestring_wkb_1" ) << line;
}

void TestQgsGeometryImport::linestringWkb()
{
  QFETCH( QVariantList, line );

  char byteOrder = QgsApplication::endian();
  int wkbSize = 1 + 2 * sizeof( int ) + line.size() * 2 * sizeof( double );
  unsigned char* geomPtr = new unsigned char[wkbSize];
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder << QGis::WKBLineString << line.size();

  for ( int i = 0; i < line.size(); ++i )
  {
    QPointF linePt = line.at( i ).toPointF();
    wkb << linePt.x() << linePt.y();
  }

  QgsGeometry geom;
  geom.fromWkb( geomPtr, wkbSize );

  QVERIFY( geom.wkbType() == QGis::WKBLineString );
  QgsPolyline polyline = geom.asPolyline();
  QVERIFY( compareLineStrings( polyline, line ) );
}

void TestQgsGeometryImport::linestringGeos_data()
{
  QTest::addColumn<QVariantList>( "line" );
  QVariantList line;
  line << QVariant( QPointF( 30.0, 10.0 ) ) << QVariant( QPointF( 10.0, 30.0 ) ) << QVariant( QPointF( 40.0, 40.0 ) );
  QTest::newRow( "linestring_geos_1" ) << line;
}

void TestQgsGeometryImport::linestringGeos()
{
  QFETCH( QVariantList, line );

  GEOSContextHandle_t geosctxt = QgsGeometry::getGEOSHandler();

  //create geos coord sequence first
  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, line.count(), 2 );
  for ( int i = 0; i < line.count(); i++ )
  {
    QPointF pt = line.at( i ).toPointF();
    GEOSCoordSeq_setX_r( geosctxt, coord, i, pt.x() );
    GEOSCoordSeq_setY_r( geosctxt, coord, i, pt.y() );
  }
  GEOSGeometry* geosLine = GEOSGeom_createLineString_r( geosctxt, coord );
  QgsGeometry geom;
  geom.fromGeos( geosLine );
  QVERIFY( geom.wkbType() == QGis::WKBLineString );

  QgsPolyline polyline = geom.asPolyline();
  QVERIFY( compareLineStrings( polyline, line ) );
}

bool TestQgsGeometryImport::compareLineStrings( const QgsPolyline& polyline, QVariantList& line )
{
  bool sizeEqual = ( polyline.size() == line.size() );
  if ( !sizeEqual )
  {
    return false;
  }

  for ( int i = 0; i < polyline.size(); ++i )
  {
    const QgsPoint& polylinePt = polyline.at( i );
    QPointF linePt = line.at( i ).toPointF();
    if ( !qgsDoubleNear( polylinePt.x(), linePt.x() ) || !qgsDoubleNear( polylinePt.y(), linePt.y() ) )
    {
      return false;
    }
  }
  return true;
}

QTEST_MAIN( TestQgsGeometryImport )
#include "testqgsgeometryimport.moc"

