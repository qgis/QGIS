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
#include "qgstest.h"
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
#include "qgsproject.h"
#include <memory>

class TestQgsDistanceArea: public QObject
{

    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void basic();
    void noEllipsoid();
    void cache();
    void test_distances();
    void regression13601();
    void collections();
    void measureUnits();
    void measureAreaAndUnits();
    void emptyPolygon();
    void regression14675();
    void regression16820();

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
  QgsPointXY p1( 1.0, 3.0 ), p2( -2.0, -1.0 );
  QgsDistanceArea daA;
  double resultA, resultB, resultC;

  daA.setEllipsoid( geoNone() );
  resultA = daA.measureLine( p1, p2 );
  QCOMPARE( resultA, 5.0 );

  // Now, on an ellipsoid. Always less?
  daA.setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32442" ) ), QgsProject::instance()->transformContext() );
  daA.setEllipsoid( QStringLiteral( "WGS84" ) );
  resultA = daA.measureLine( p1, p2 );
  QVERIFY( resultA < 5.0 );

  // Test copy constructor
  QgsDistanceArea daB( daA );
  resultB = daB.measureLine( p1, p2 );
  QCOMPARE( resultA, resultB );

  // Different Ellipsoid
  daB.setEllipsoid( QStringLiteral( "WGS72" ) );
  resultB = daB.measureLine( p1, p2 );
  QVERIFY( ! qFuzzyCompare( resultA, resultB ) );

  // Test assignment
  const std::shared_ptr<QgsDistanceArea> daC( new QgsDistanceArea );
  *daC = daB;
  resultC = daC->measureLine( p1, p2 );
  QCOMPARE( resultB, resultC );

  // Use parameter setting of ellipsoid radii (from WGS72 )
  daA.setEllipsoid( 6378135.0, 6378135.0 - ( 6378135.0 / 298.26 ) );
  resultA = daA.measureLine( p1, p2 );
  QCOMPARE( resultA, resultB );
}

void TestQgsDistanceArea::noEllipsoid()
{
  QgsDistanceArea da;
  da.setEllipsoid( geoNone() );
  QVERIFY( !da.willUseEllipsoid() );
  QCOMPARE( da.ellipsoid(), geoNone() );
}

void TestQgsDistanceArea::cache()
{
  // test that ellipsoid can be retrieved correctly from cache
  QgsDistanceArea da;

  // warm cache
  QVERIFY( da.setEllipsoid( QStringLiteral( "Ganymede2000" ) ) );
  QVERIFY( da.willUseEllipsoid() );
  QCOMPARE( da.ellipsoidSemiMajor(), 2632345.0 );
  QCOMPARE( da.ellipsoidSemiMinor(), 2632345.0 );
  QCOMPARE( da.ellipsoid(), QStringLiteral( "Ganymede2000" ) );

  // a second time, so ellipsoid is fetched from cache
  QgsDistanceArea da2;
  QVERIFY( da2.setEllipsoid( QStringLiteral( "Ganymede2000" ) ) );
  QVERIFY( da2.willUseEllipsoid() );
  QCOMPARE( da2.ellipsoidSemiMajor(), 2632345.0 );
  QCOMPARE( da2.ellipsoidSemiMinor(), 2632345.0 );
  QCOMPARE( da2.ellipsoid(), QStringLiteral( "Ganymede2000" ) );

  // using parameters
  QgsDistanceArea da3;
  QVERIFY( da3.setEllipsoid( QStringLiteral( "PARAMETER:2631400:2341350" ) ) );
  QVERIFY( da3.willUseEllipsoid() );
  QCOMPARE( da3.ellipsoidSemiMajor(), 2631400.0 );
  QCOMPARE( da3.ellipsoidSemiMinor(), 2341350.0 );
  QGSCOMPARENEAR( da3.ellipsoidInverseFlattening(), 9.07223, 0.00001 );
  QCOMPARE( da3.ellipsoid(), QStringLiteral( "PARAMETER:2631400:2341350" ) );

  // again, to check parameters with cache
  QgsDistanceArea da4;
  QVERIFY( da4.setEllipsoid( QStringLiteral( "PARAMETER:2631400:2341350" ) ) );
  QVERIFY( da4.willUseEllipsoid() );
  QCOMPARE( da4.ellipsoidSemiMajor(), 2631400.0 );
  QCOMPARE( da4.ellipsoidSemiMinor(), 2341350.0 );
  QGSCOMPARENEAR( da4.ellipsoidInverseFlattening(), 9.07223, 0.00001 );
  QCOMPARE( da4.ellipsoid(), QStringLiteral( "PARAMETER:2631400:2341350" ) );

  // invalid
  QgsDistanceArea da5;
  QVERIFY( !da5.setEllipsoid( QStringLiteral( "MyFirstEllipsoid" ) ) );
  QVERIFY( !da5.willUseEllipsoid() );
  QCOMPARE( da5.ellipsoid(), geoNone() );

  // invalid again, should be cached
  QgsDistanceArea da6;
  QVERIFY( !da6.setEllipsoid( QStringLiteral( "MyFirstEllipsoid" ) ) );
  QVERIFY( !da6.willUseEllipsoid() );
  QCOMPARE( da6.ellipsoid(), geoNone() );
}

void TestQgsDistanceArea::test_distances()
{
  // Read the file of Geod data
  // Column 0 (first) is latitude point 1
  // Column 1 is longitude point 1
  // Column 3 is latitude point 2
  // Column 4 is longitude point 3
  // Column 6 is the resulting distance in meters on the WGS84 ellipsoid
  // Note: lat is north/south, so the QgsPointXY should be ( long, lat )
  // See http://geographiclib.sourceforge.net/html/geodesic.html#testgeod

  // Set up DA
  QgsDistanceArea myDa;
  myDa.setSourceCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4030" ) ), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QStringLiteral( "WGS84" ) );

  const QString myFileName = QStringLiteral( TEST_DATA_DIR ) + "/GeodTest-nano.dat";

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
      const QgsPointXY p1( myLineList[1].toDouble(), myLineList[0].toDouble() );
      const QgsPointXY p2( myLineList[4].toDouble(), myLineList[3].toDouble() );
      const double result = myDa.measureLine( p1, p2 );
      // QgsDebugMsg( QStringLiteral( "Distance from %1 to %2 is %3" ).arg( p1.toString( 15 ) ).arg( p2.toString( 15 ) ).arg( result, 0, 'g', 15 ) );
      // QgsDebugMsg( QStringLiteral( "Distance should be %1" ).arg( myLineList[6] ) );
      // Check result is less than 0.5mm from expected.
      QGSCOMPARENEAR( result, myLineList[6].toDouble(), 0.0005 );
    }
  }

}

void TestQgsDistanceArea::regression13601()
{
  //test regression #13601
  QgsDistanceArea calc;
  calc.setEllipsoid( QStringLiteral( "NONE" ) );
  calc.setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3148" ) ), QgsProject::instance()->transformContext() );
  const QgsGeometry geom( QgsGeometryFactory::geomFromWkt( QStringLiteral( "Polygon ((252000 1389000, 265000 1389000, 265000 1385000, 252000 1385000, 252000 1389000))" ) ).release() );
  QGSCOMPARENEAR( calc.measureArea( geom ), 52000000, 0.0001 );
}

void TestQgsDistanceArea::collections()
{
  //test measuring for collections
  QgsDistanceArea myDa;
  myDa.setSourceCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4030" ) ), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QStringLiteral( "WGS84" ) );

  //collection of lines, should be sum of line length
  const QgsGeometry lines( QgsGeometryFactory::geomFromWkt( QStringLiteral( "GeometryCollection( LineString(0 36.53, 5.76 -48.16), LineString(0 25.54, 24.20 36.70) )" ) ).release() );
  double result = myDa.measureLength( lines );
  QGSCOMPARENEAR( result, 12006159, 1 );
  result = myDa.measureArea( lines );
  QGSCOMPARENEAR( result, 0, 4 * std::numeric_limits<double>::epsilon() );

  //collection of polygons

  const QgsGeometry poly1 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 36.53, 5.76 -48.16, 0 25.54, 0 36.53))" ) );
  result = myDa.measureArea( poly1 );
  QGSCOMPARENEAR( result, 439881520607.079712, 1 );
  result = myDa.measureLength( poly1 );
  QGSCOMPARENEAR( result, 0, 4 * std::numeric_limits<double>::epsilon() );
  const QgsGeometry poly2 = QgsGeometry::fromWkt( QStringLiteral( "Polygon((10 20, 15 20, 15 10, 10 20))" ) );
  result = myDa.measureArea( poly2 );
  QGSCOMPARENEAR( result, 290350317025.906982, 1 );
  result = myDa.measureLength( poly2 );
  QGSCOMPARENEAR( result, 0, 4 * std::numeric_limits<double>::epsilon() );

  const QgsGeometry polys( QgsGeometryFactory::geomFromWkt( QStringLiteral( "GeometryCollection( Polygon((0 36.53, 5.76 -48.16, 0 25.54, 0 36.53)), Polygon((10 20, 15 20, 15 10, 10 20)) )" ) ).release() );
  result = myDa.measureArea( polys );
  QGSCOMPARENEAR( result, 730231837632.98669, 1 );
  result = myDa.measureLength( polys );
  QGSCOMPARENEAR( result, 0, 4 * std::numeric_limits<double>::epsilon() );

  //mixed collection
  const QgsGeometry mixed( QgsGeometryFactory::geomFromWkt( QStringLiteral( "GeometryCollection( LineString(0 36.53, 5.76 -48.16), LineString(0 25.54, 24.20 36.70), Polygon((0 36.53, 5.76 -48.16, 0 25.54, 0 36.53)), Polygon((10 20, 15 20, 15 10, 10 20)) )" ) ).release() );
  //measure area specifically
  result = myDa.measureArea( mixed );
  QGSCOMPARENEAR( result, 730231837632.98669, 1 );
  //measure length
  result = myDa.measureLength( mixed );
  QGSCOMPARENEAR( result, 12006159, 1 );
}

void TestQgsDistanceArea::measureUnits()
{
  //test regression #13610
  QgsDistanceArea calc;
  calc.setEllipsoid( QStringLiteral( "NONE" ) );
  calc.setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:2272" ) ), QgsProject::instance()->transformContext() );
  QgsUnitTypes::DistanceUnit units;
  const QgsPointXY p1( 1341683.9854275715, 408256.9562717728 );
  const QgsPointXY p2( 1349321.7807031618, 408256.9562717728 );

  double result = calc.measureLine( p1, p2 );
  units = calc.lengthUnits();
  //no OTF, result will be in CRS unit (feet)
  QCOMPARE( units, QgsUnitTypes::DistanceFeet );
  QGSCOMPARENEAR( result, 7637.7952755903825, 0.001 );

  calc.setEllipsoid( QStringLiteral( "WGS84" ) );
  units = calc.lengthUnits();
  result = calc.measureLine( p1, p2 );
  //OTF, result will be in meters
  QCOMPARE( units, QgsUnitTypes::DistanceMeters );
  QGSCOMPARENEAR( result, 2328.0988253106957, 0.001 );
}

void TestQgsDistanceArea::measureAreaAndUnits()
{
  QgsDistanceArea da;
  da.setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsProject::instance()->transformContext() );
  da.setEllipsoid( QStringLiteral( "NONE" ) );
  QgsPolylineXY ring;
  ring << QgsPointXY( 0, 0 )
       << QgsPointXY( 1, 0 )
       << QgsPointXY( 1, 1 )
       << QgsPointXY( 2, 1 )
       << QgsPointXY( 2, 2 )
       << QgsPointXY( 0, 2 )
       << QgsPointXY( 0, 0 );
  QgsPolygonXY poly;
  poly << ring;

  QgsGeometry polygon( QgsGeometry::fromPolygonXY( poly ) );

  // We check both the measured area AND the units, in case the logic regarding
  // ellipsoids and units changes in future
  double area = da.measureArea( polygon );
  QgsUnitTypes::AreaUnit units = da.areaUnits();

  QgsDebugMsg( QStringLiteral( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );

  QVERIFY( ( qgsDoubleNear( area, 3.0, 0.00000001 ) && units == QgsUnitTypes::AreaSquareDegrees )
           || ( qgsDoubleNear( area, 37176087091.5, 0.1 ) && units == QgsUnitTypes::AreaSquareMeters ) );

  da.setEllipsoid( QStringLiteral( "WGS84" ) );
  area = da.measureArea( polygon );
  units = da.areaUnits();
  QgsDebugMsg( QStringLiteral( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );
  // should always be in Meters Squared
  QGSCOMPARENEAR( area, 36922805935.961571, 0.1 );
  QCOMPARE( units, QgsUnitTypes::AreaSquareMeters );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, QgsUnitTypes::AreaSquareMiles );
  QGSCOMPARENEAR( area, 14255.975071, 0.001 );

  // now try with a source CRS which is in feet
  ring.clear();
  ring << QgsPointXY( 1850000, 4423000 )
       << QgsPointXY( 1851000, 4423000 )
       << QgsPointXY( 1851000, 4424000 )
       << QgsPointXY( 1852000, 4424000 )
       << QgsPointXY( 1852000, 4425000 )
       << QgsPointXY( 1851000, 4425000 )
       << QgsPointXY( 1850000, 4423000 );
  poly.clear();
  poly << ring;
  polygon = QgsGeometry::fromPolygonXY( poly );

  da.setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "ESRI:102635" ) ), QgsProject::instance()->transformContext() );
  da.setEllipsoid( QStringLiteral( "NONE" ) );
  // measurement should be in square feet
  area = da.measureArea( polygon );
  units = da.areaUnits();
  QgsDebugMsg( QStringLiteral( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );
  QGSCOMPARENEAR( area, 2000000, 0.001 );
  QCOMPARE( units, QgsUnitTypes::AreaSquareFeet );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, QgsUnitTypes::AreaSquareYards );
  QGSCOMPARENEAR( area, 222222.2222, 0.001 );

  da.setEllipsoid( QStringLiteral( "WGS84" ) );
  // now should be in Square Meters again
  area = da.measureArea( polygon );
  units = da.areaUnits();
  QgsDebugMsg( QStringLiteral( "measured %1 in %2" ).arg( area ).arg( QgsUnitTypes::toString( units ) ) );
  QGSCOMPARENEAR( area, 185825.206903, 1.0 );
  QCOMPARE( units, QgsUnitTypes::AreaSquareMeters );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, QgsUnitTypes::AreaSquareYards );
  QgsDebugMsg( QStringLiteral( "measured %1 in sq yrds" ).arg( area ) );
  QGSCOMPARENEAR( area, 222245.097808, 1.0 );
}

void TestQgsDistanceArea::emptyPolygon()
{
  QgsDistanceArea da;
  da.setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsProject::instance()->transformContext() );
  da.setEllipsoid( QStringLiteral( "WGS84" ) );

  //test that measuring an empty polygon doesn't crash
  da.measurePolygon( QVector< QgsPointXY >() );
}

void TestQgsDistanceArea::regression14675()
{
  //test regression #14675
  QgsDistanceArea calc;
  calc.setEllipsoid( QStringLiteral( "GRS80" ) );
  calc.setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:2154" ) ), QgsProject::instance()->transformContext() );
  const QgsGeometry geom( QgsGeometryFactory::geomFromWkt( QStringLiteral( "Polygon ((917593.5791854317067191 6833700.00807378999888897, 917596.43389983859378844 6833700.67099479306489229, 917599.53056440979707986 6833700.78673478215932846, 917593.5791854317067191 6833700.00807378999888897))" ) ).release() );
  QGSCOMPARENEAR( calc.measureArea( geom ), 0.861747, 0.001 );
}

void TestQgsDistanceArea::regression16820()
{
  QgsDistanceArea calc;
  calc.setEllipsoid( QStringLiteral( "WGS84" ) );
  calc.setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32634" ) ), QgsProject::instance()->transformContext() );
  const QgsGeometry geom( QgsGeometryFactory::geomFromWkt( QStringLiteral( "Polygon ((110250.54038314701756462 5084495.57398066483438015, 110243.46975068224128336 5084507.17200060561299324, 110251.23908144699817058 5084506.68309532757848501, 110251.2394439501222223 5084506.68307251576334238, 110250.54048078990308568 5084495.57553235255181789, 110250.54038314701756462 5084495.57398066483438015))" ) ).release() );
  QGSCOMPARENEAR( calc.measureArea( geom ), 43.201092, 0.001 );
}

QGSTEST_MAIN( TestQgsDistanceArea )
#include "testqgsdistancearea.moc"




