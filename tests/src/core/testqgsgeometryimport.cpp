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
#include "qgspointxy.h"
#include "qgswkbptr.h"
#include "qgsgeos.h"
#include <QPolygonF>


#include "qgstest.h"

#include <QObject>

class TestQgsGeometryImport: public QObject
{
    Q_OBJECT

  private slots:

    void initTestCase();

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

    void delimiters_data();
    void delimiters();

  private:
    bool compareLineStrings( const QgsPolylineXY &polyline, QVariantList &line );

    GEOSContextHandle_t geos = nullptr;
};

void TestQgsGeometryImport::initTestCase()
{
  geos = initGEOS_r( nullptr, nullptr );
}

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

  const QgsGeometry geom = QgsGeometry::fromWkt( wktString );

  QCOMPARE( geom.wkbType(), QgsWkbTypes::Point );
  const QgsPointXY point = geom.asPoint();

  QGSCOMPARENEAR( point.x(), x, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( point.y(), y, 4 * std::numeric_limits<double>::epsilon() );
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
  const char byteOrder = QgsApplication::endian();
  unsigned char *geomPtr = new unsigned char[21];
  QgsWkbPtr wkb( geomPtr, 21 );
  wkb << byteOrder << QgsWkbTypes::Point << x << y;

  QgsGeometry geom;
  geom.fromWkb( geomPtr, 21 );
  const QgsPointXY point = geom.asPoint();

  QCOMPARE( geom.wkbType(), QgsWkbTypes::Point );
  QGSCOMPARENEAR( point.x(), x, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( point.y(), y, 4 * std::numeric_limits<double>::epsilon() );
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

  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geos, 1, 2 );
  GEOSCoordSeq_setX_r( geos, coord, 0, x );
  GEOSCoordSeq_setY_r( geos, coord, 0, y );
  GEOSGeometry *geosPt = GEOSGeom_createPoint_r( geos, coord );

  const QgsGeometry geom = QgsGeos::geometryFromGeos( geosPt );
  QVERIFY( geom.wkbType() == QgsWkbTypes::Point );

  const QgsPointXY geomPt = geom.asPoint();

  QGSCOMPARENEAR( x, geomPt.x(), 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( y, geomPt.y(), 4 * std::numeric_limits<double>::epsilon() );
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

  const QgsGeometry geom = QgsGeometry::fromWkt( wktString );
  QCOMPARE( geom.wkbType(), QgsWkbTypes::LineString );

  const QgsPolylineXY polyLine = geom.asPolyline();
  QVERIFY( compareLineStrings( polyLine, line ) );
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

  const char byteOrder = QgsApplication::endian();
  const int wkbSize = 1 + 2 * sizeof( int ) + line.size() * 2 * sizeof( double );
  unsigned char *geomPtr = new unsigned char[wkbSize];
  QgsWkbPtr wkb( geomPtr, wkbSize );
  wkb << byteOrder << QgsWkbTypes::LineString << line.size();

  for ( int i = 0; i < line.size(); ++i )
  {
    const QPointF linePt = line.at( i ).toPointF();
    wkb << linePt.x() << linePt.y();
  }

  QgsGeometry geom;
  geom.fromWkb( geomPtr, wkbSize );

  QVERIFY( geom.wkbType() == QgsWkbTypes::LineString );
  const QgsPolylineXY polyline = geom.asPolyline();
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

  //create geos coord sequence first
  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geos, line.count(), 2 );
  for ( int i = 0; i < line.count(); i++ )
  {
    const QPointF pt = line.at( i ).toPointF();
    GEOSCoordSeq_setX_r( geos, coord, i, pt.x() );
    GEOSCoordSeq_setY_r( geos, coord, i, pt.y() );
  }
  GEOSGeometry *geosLine = GEOSGeom_createLineString_r( geos, coord );
  const QgsGeometry geom = QgsGeos::geometryFromGeos( geosLine );
  QVERIFY( geom.wkbType() == QgsWkbTypes::LineString );

  const QgsPolylineXY polyline = geom.asPolyline();
  QVERIFY( compareLineStrings( polyline, line ) );
}


bool TestQgsGeometryImport::compareLineStrings( const QgsPolylineXY &polyline, QVariantList &line )
{
  const bool sizeEqual = ( polyline.size() == line.size() );
  if ( !sizeEqual )
  {
    return false;
  }

  for ( int i = 0; i < polyline.size(); ++i )
  {
    const QgsPointXY &polylinePt = polyline.at( i );
    const QPointF linePt = line.at( i ).toPointF();
    if ( !qgsDoubleNear( polylinePt.x(), linePt.x() ) || !qgsDoubleNear( polylinePt.y(), linePt.y() ) )
    {
      return false;
    }
  }
  return true;
}


void TestQgsGeometryImport::delimiters_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<QString>( "expected" );
  QTest::newRow( "tab delimiter" ) <<  QStringLiteral( "POINT (180398\t5459331)" ) << QStringLiteral( "Point (180398 5459331)" );
  QTest::newRow( "newline" ) <<  QStringLiteral( "POINT\n(1\n3)" ) << QStringLiteral( "Point (1 3)" );
  QTest::newRow( "tab and newline" ) <<  QStringLiteral( "POINT\t\n(1\t\n3)" ) << QStringLiteral( "Point (1 3)" );
  QTest::newRow( "tab, newline and space" ) <<  QStringLiteral( "POINT\n (1\t\n 3)" ) << QStringLiteral( "Point (1 3)" );

  QTest::newRow( "tab delimiter" ) <<  QStringLiteral( "LINESTRING\t(30\t10,\t10\t30,\t40\t40)" ) << QStringLiteral( "LineString (30 10, 10 30, 40 40)" );
  QTest::newRow( "newline delimiter" ) <<  QStringLiteral( "LINESTRING\n(30\n10,\n10\n30,\n40\n40)" ) << QStringLiteral( "LineString (30 10, 10 30, 40 40)" );
  QTest::newRow( "mixed delimiter" ) <<  QStringLiteral( "LINESTRING\n(30\t10, 10\t30,\n40\t40)" ) << QStringLiteral( "LineString (30 10, 10 30, 40 40)" );

  QTest::newRow( "tab delimiter" ) <<  QStringLiteral( "Polygon\t(\t(30\t10,\t10\t30,\t40\t40,30\t10)\t)" ) << QStringLiteral( "Polygon ((30 10, 10 30, 40 40, 30 10))" );
  QTest::newRow( "newline delimiter" ) <<  QStringLiteral( "\nPolygon\n(\n(30\n10,\n10\n30,\n40\n40,30\n10)\n)\n" ) << QStringLiteral( "Polygon ((30 10, 10 30, 40 40, 30 10))" );
  QTest::newRow( "mixed delimiter" ) <<  QStringLiteral( " Polygon (\t(30\n10,\t10\n30,\t40 40,30\n10)\t)\n" ) << QStringLiteral( "Polygon ((30 10, 10 30, 40 40, 30 10))" );
}

void TestQgsGeometryImport::delimiters()
{
  QFETCH( QString, input );
  QFETCH( QString, expected );

  const QgsGeometry gInput = QgsGeometry::fromWkt( input );
  QCOMPARE( gInput.asWkt(), expected );
}

QGSTEST_MAIN( TestQgsGeometryImport )
#include "testqgsgeometryimport.moc"
