/***************************************************************************
     testqgsdistancearea.cpp
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
#include <QtTest/QtTest>
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qgsapplication.h>
//header for class being tested
#include <qgsdistancearea.h>
#include <qgspoint.h>
#include "qgslogger.h"
#include "qgsgeometryfactory.h"
#include "qgsgeometry.h"
#include "qgis.h"
#include "qgstestutils.h"


class TestQgsDistanceArea: public QObject
{

    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void basic();
    void test_distances();
    void unit_conversions();
    void regression13601();
    void collections();
    void measureUnits();
    void measureAreaAndUnits();
    void emptyPolygon();
    void regression14675();

};

void TestQgsDistanceArea::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsDistanceArea::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsDistanceArea::basic()
{
  QgsPoint p1( 1.0, 3.0 ), p2( -2.0, -1.0 );
  QgsDistanceArea daA;
  double resultA, resultB, resultC;

  daA.setEllipsoid( GEO_NONE );
  resultA = daA.measureLine( p1, p2 );
  QCOMPARE( resultA, 5.0 );

  // Now, on an ellipsoid. Always less?
  daA.setSourceCrs( 3006 );
  daA.setEllipsoid( "WGS84" );
  daA.setEllipsoidalMode( true );
  resultA = daA.measureLine( p1, p2 );
  QVERIFY( resultA < 5.0 );

  // Test copy constructor
  QgsDistanceArea daB( daA );
  resultB = daB.measureLine( p1, p2 );
  QCOMPARE( resultA, resultB );

  // Different Ellipsoid
  daB.setEllipsoid( "WGS72" );
  resultB = daB.measureLine( p1, p2 );
  QVERIFY( ! qFuzzyCompare( resultA, resultB ) );

  // Test assignment
  QSharedPointer<QgsDistanceArea> daC( new QgsDistanceArea );
  *daC = daB;
  resultC = daC->measureLine( p1, p2 );
  QCOMPARE( resultB, resultC );

  // Use parameter setting of ellipsoid radii (from WGS72 )
  daA.setEllipsoid( 6378135.0, 6378135.0 - ( 6378135.0 / 298.26 ) );
  resultA = daA.measureLine( p1, p2 );
  QCOMPARE( resultA, resultB );
}

void TestQgsDistanceArea::test_distances()
{
  // Read the file of Geod data
  // Column 0 (first) is latitude point 1
  // Column 1 is longitude point 1
  // Column 3 is latitude point 2
  // Column 4 is longitude point 3
  // Column 6 is the resulting distance in meters on the WGS84 ellipsoid
  // Note: lat is north/south, so the QgsPoint should be ( long, lat )
  // See http://geographiclib.sourceforge.net/html/geodesic.html#testgeod

  // Set up DA
  QgsDistanceArea myDa;
  myDa.setSourceAuthId( "EPSG:4030" );
  myDa.setEllipsoidalMode( true );
  myDa.setEllipsoid( "WGS84" );

  QString myFileName = QString( TEST_DATA_DIR ) + "/GeodTest-nano.dat";

  QFile myFile( myFileName );
  if ( ! myFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QFAIL( "Couldn't open file" );
    return;
  }
  QTextStream in( & myFile );
  while ( !in.atEnd() )
  {
    QString line = in.readLine();
    // Some test points (antipodal) does not converge with the chosen algorithm!
    // See calcaulator at http://www.movable-type.co.uk/scripts/latlong-vincenty.html
    // These are commented out.
    if ( line[0] != '#' )
    {
      QStringList myLineList = line.split( ' ' ); // Split fields on space.
      // Create points
      QgsPoint p1( myLineList[1].toDouble(), myLineList[0].toDouble() );
      QgsPoint p2( myLineList[4].toDouble(), myLineList[3].toDouble() );
      double result = myDa.measureLine( p1, p2 );
      // QgsDebugMsg( QString( "Distance from %1 to %2 is %3" ).arg( p1.toString( 15 ) ).arg( p2.toString( 15 ) ).arg( result, 0, 'g', 15 ) );
      // QgsDebugMsg( QString( "Distance should be %1" ).arg( myLineList[6] ) );
      // Check result is less than 0.5mm from expected.
      QVERIFY( qAbs( result -  myLineList[6].toDouble() ) < 0.0005 );
    }
  }

}

void TestQgsDistanceArea::unit_conversions()
{
  // Do some very simple test of conversion and units
  QgsDistanceArea myDa;
  myDa.setEllipsoidalMode( false );

  double inputValue;
  QGis::UnitType inputUnit;
  QGis::UnitType outputUnit;

  inputValue = 10000.0;
  inputUnit = QGis::Meters;
  outputUnit = QGis::Feet;
  //outputUnit = QGis::Meters;

  // First, convert from sq.meter to sq.feet
  Q_NOWARN_DEPRECATED_PUSH
  myDa.convertMeasurement( inputValue, inputUnit, outputUnit, true );
  QVERIFY( qAbs( inputValue - 107639.1041671 ) <= 0.0000001 );

  // The print a text unit. This is i18n, so we should ignore the unit
  // and use the locale settings for separation of digits.
  QString myTxt = QgsDistanceArea::textUnit( inputValue, 7, inputUnit, true, false );
  QString expectedTxt = QLocale::system().toString( 2.4710538146717, 'g', 1 + 7 );
  QVERIFY( myTxt.startsWith( expectedTxt ) ); // Ignore units for now.
  Q_NOWARN_DEPRECATED_POP
}

void TestQgsDistanceArea::regression13601()
{
  //test regression #13601
  QgsDistanceArea calc;
  calc.setEllipsoidalMode( true );
  calc.setEllipsoid( "NONE" );
  calc.setSourceCrs( 1108L );
  QgsGeometry geom( QgsGeometryFactory::geomFromWkt( "Polygon ((252000 1389000, 265000 1389000, 265000 1385000, 252000 1385000, 252000 1389000))" ) );
  QGSCOMPARENEAR( calc.measureArea( &geom ), 52000000, 0.0001 );
}

void TestQgsDistanceArea::collections()
{
  Q_NOWARN_DEPRECATED_PUSH
  //test measuring for collections
  QgsDistanceArea myDa;
  myDa.setSourceAuthId( "EPSG:4030" );
  myDa.setEllipsoidalMode( true );
  myDa.setEllipsoid( "WGS84" );

  //collection of lines, should be sum of line length
  QgsGeometry lines( QgsGeometryFactory::geomFromWkt( "GeometryCollection( LineString(0 36.53, 5.76 -48.16), LineString(0 25.54, 24.20 36.70) )" ) );
  double result = myDa.measure( &lines ); //should measure length
  QGSCOMPARENEAR( result, 12006159, 1 );
  result = myDa.measureLength( &lines );
  QGSCOMPARENEAR( result, 12006159, 1 );
  result = myDa.measureArea( &lines );
  QVERIFY( qgsDoubleNear( result, 0 ) );

  //collection of polygons
  QgsGeometry polys( QgsGeometryFactory::geomFromWkt( "GeometryCollection( Polygon((0 36.53, 5.76 -48.16, 0 25.54, 0 36.53)), Polygon((10 20, 15 20, 15 10, 10 20)) )" ) );
  result = myDa.measure( &polys ); //should measure area
  QGSCOMPARENEAR( result, 670434859475LL, 1 );
  result = myDa.measureArea( &polys );
  QGSCOMPARENEAR( result, 670434859475LL, 1 );
  result = myDa.measureLength( &polys );
  QVERIFY( qgsDoubleNear( result, 0 ) );

  //mixed collection
  QgsGeometry mixed( QgsGeometryFactory::geomFromWkt( "GeometryCollection( LineString(0 36.53, 5.76 -48.16), LineString(0 25.54, 24.20 36.70), Polygon((0 36.53, 5.76 -48.16, 0 25.54, 0 36.53)), Polygon((10 20, 15 20, 15 10, 10 20)) )" ) );
  result = myDa.measure( &mixed ); //should measure area
  QGSCOMPARENEAR( result, 670434859475LL, 1 );
  //measure area specifically
  result = myDa.measureArea( &mixed );
  QGSCOMPARENEAR( result, 670434859475LL, 1 );
  //measure length
  result = myDa.measureLength( &mixed );
  QGSCOMPARENEAR( result, 12006159, 1 );

  Q_NOWARN_DEPRECATED_POP
}

void TestQgsDistanceArea::measureUnits()
{
  //test regression #13610
  QgsDistanceArea calc;
  calc.setEllipsoidalMode( false );
  calc.setEllipsoid( "NONE" );
  calc.setSourceCrs( 254L );
  QGis::UnitType units;
  QgsPoint p1( 1341683.9854275715, 408256.9562717728 );
  QgsPoint p2( 1349321.7807031618, 408256.9562717728 );

  double result = calc.measureLine( p1, p2, units );
  //no OTF, result will be in CRS unit (feet)
  QCOMPARE( units, QGis::Feet );
  QGSCOMPARENEAR( result, 7637.7952755903825, 0.001 );

  calc.setEllipsoidalMode( true );
  calc.setEllipsoid( "WGS84" );
  result = calc.measureLine( p1, p2, units );
  //OTF, result will be in meters
  QCOMPARE( units, QGis::Meters );
  QGSCOMPARENEAR( result, 2328.0988253106957, 0.001 );
}

void TestQgsDistanceArea::measureAreaAndUnits()
{
  QgsDistanceArea da;
  da.setSourceCrs( 3452 );
  da.setEllipsoidalMode( false );
  da.setEllipsoid( "NONE" );
  QgsCoordinateReferenceSystem daCRS;
  daCRS.createFromSrsId( da.sourceCrsId() );
  QgsPolyline ring;
  ring << QgsPoint( 0, 0 )
  << QgsPoint( 1, 0 )
  << QgsPoint( 1, 1 )
  << QgsPoint( 2, 1 )
  << QgsPoint( 2, 2 )
  << QgsPoint( 0, 2 )
  << QgsPoint( 0, 0 );
  QgsPolygon poly;
  poly << ring;

  QScopedPointer< QgsGeometry > polygon( QgsGeometry::fromPolygon( poly ) );

  // We check both the measured area AND the units, in case the logic regarding
  // ellipsoids and units changes in future
  double area = da.measureArea( polygon.data() );
  QgsUnitTypes::AreaUnit units = da.areaUnits();

  QgsDebugMsg( QString( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );

  QVERIFY(( qgsDoubleNear( area, 3.0, 0.00000001 ) && units == QgsUnitTypes::SquareDegrees )
          || ( qgsDoubleNear( area, 37176087091.5, 0.1 ) && units == QgsUnitTypes::SquareMeters ) );

  da.setEllipsoid( "WGS84" );
  area = da.measureArea( polygon.data() );
  units = da.areaUnits();

  QgsDebugMsg( QString( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );
  QVERIFY(( qgsDoubleNear( area, 3.0, 0.00000001 ) && units == QgsUnitTypes::SquareDegrees )
          || ( qgsDoubleNear( area, 37176087091.5, 0.1 ) && units == QgsUnitTypes::SquareMeters ) );

  da.setEllipsoidalMode( true );
  area = da.measureArea( polygon.data() );
  units = da.areaUnits();

  QgsDebugMsg( QString( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );
  // should always be in Meters Squared
  QGSCOMPARENEAR( area, 37416879192.9, 0.1 );
  QCOMPARE( units, QgsUnitTypes::SquareMeters );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, QgsUnitTypes::SquareMiles );
  QGSCOMPARENEAR( area, 14446.7378, 0.001 );

  // now try with a source CRS which is in feet
  ring.clear();
  ring << QgsPoint( 1850000, 4423000 )
  << QgsPoint( 1851000, 4423000 )
  << QgsPoint( 1851000, 4424000 )
  << QgsPoint( 1852000, 4424000 )
  << QgsPoint( 1852000, 4425000 )
  << QgsPoint( 1851000, 4425000 )
  << QgsPoint( 1850000, 4423000 );
  poly.clear();
  poly << ring;
  polygon.reset( QgsGeometry::fromPolygon( poly ) );

  da.setSourceCrs( 27469 );
  da.setEllipsoidalMode( false );
  // measurement should be in square feet
  area = da.measureArea( polygon.data() );
  units = da.areaUnits();
  QgsDebugMsg( QString( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );
  QGSCOMPARENEAR( area, 2000000, 0.001 );
  QCOMPARE( units, QgsUnitTypes::SquareFeet );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, QgsUnitTypes::SquareYards );
  QGSCOMPARENEAR( area, 222222.2222, 0.001 );

  da.setEllipsoidalMode( true );
  // now should be in Square Meters again
  area = da.measureArea( polygon.data() );
  units = da.areaUnits();
  QgsDebugMsg( QString( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );
  QGSCOMPARENEAR( area, 184149.37, 1.0 );
  QCOMPARE( units, QgsUnitTypes::SquareMeters );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, QgsUnitTypes::SquareYards );
  QgsDebugMsg( QString( "measured %1 in sq yrds" ).arg( area ) );
  QGSCOMPARENEAR( area, 220240.8172549, 0.00001 );
}

void TestQgsDistanceArea::emptyPolygon()
{
  QgsDistanceArea da;
  da.setSourceCrs( 3452 );
  da.setEllipsoidalMode( true );
  da.setEllipsoid( "WGS84" );

  //test that measuring an empty polygon doesn't crash
  da.measurePolygon( QList< QgsPoint >() );
}

void TestQgsDistanceArea::regression14675()
{
  //test regression #14675
  QgsDistanceArea calc;
  calc.setEllipsoidalMode( true );
  calc.setEllipsoid( "GRS80" );
  calc.setSourceCrs( 145L );
  QgsGeometry geom( QgsGeometryFactory::geomFromWkt( "Polygon ((917593.5791854317067191 6833700.00807378999888897, 917596.43389983859378844 6833700.67099479306489229, 917599.53056440979707986 6833700.78673478215932846, 917593.5791854317067191 6833700.00807378999888897))" ) );
  //lots of tolerance here - the formulas get quite unstable with small areas due to division by very small floats
  QGSCOMPARENEAR( calc.measureArea( &geom ), 0.83301, 0.02 );
}

QTEST_MAIN( TestQgsDistanceArea )
#include "testqgsdistancearea.moc"




