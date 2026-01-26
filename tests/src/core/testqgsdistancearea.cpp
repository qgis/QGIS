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
#include "qgsapplication.h"
#include "qgstest.h"

#include <QFile>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextStream>

//header for class being tested
#include <qgsdistancearea.h>
#include <qgspoint.h>
#include "qgslogger.h"
#include "qgsgeometryfactory.h"
#include "qgsgeometry.h"
#include "qgis.h"
#include "qgsproject.h"
#include <memory>

class TestQgsDistanceArea : public QObject
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
    void regression61299();
};

void TestQgsDistanceArea::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths true means that all path will be inited from prefix
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

  daA.setEllipsoid( Qgis::geoNone() );
  resultA = daA.measureLine( p1, p2 );
  QCOMPARE( resultA, 5.0 );

  // Now, on an ellipsoid. Always less?
  daA.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:32442"_s ), QgsProject::instance()->transformContext() );
  daA.setEllipsoid( u"WGS84"_s );
  resultA = daA.measureLine( p1, p2 );
  QVERIFY( resultA < 5.0 );

  // Test copy constructor
  QgsDistanceArea daB( daA );
  resultB = daB.measureLine( p1, p2 );
  QCOMPARE( resultA, resultB );

  // Different Ellipsoid
  daB.setEllipsoid( u"WGS72"_s );
  resultB = daB.measureLine( p1, p2 );
  QVERIFY( !qFuzzyCompare( resultA, resultB ) );

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
  da.setEllipsoid( Qgis::geoNone() );
  QVERIFY( !da.willUseEllipsoid() );
  QCOMPARE( da.ellipsoid(), Qgis::geoNone() );
}

void TestQgsDistanceArea::cache()
{
  // test that ellipsoid can be retrieved correctly from cache
  QgsDistanceArea da;

  // warm cache
  QVERIFY( da.setEllipsoid( u"Ganymede2000"_s ) );
  QVERIFY( da.willUseEllipsoid() );
  QCOMPARE( da.ellipsoidSemiMajor(), 2632345.0 );
  QCOMPARE( da.ellipsoidSemiMinor(), 2632345.0 );
  QCOMPARE( da.ellipsoid(), u"Ganymede2000"_s );

  // a second time, so ellipsoid is fetched from cache
  QgsDistanceArea da2;
  QVERIFY( da2.setEllipsoid( u"Ganymede2000"_s ) );
  QVERIFY( da2.willUseEllipsoid() );
  QCOMPARE( da2.ellipsoidSemiMajor(), 2632345.0 );
  QCOMPARE( da2.ellipsoidSemiMinor(), 2632345.0 );
  QCOMPARE( da2.ellipsoid(), u"Ganymede2000"_s );

  // using parameters
  QgsDistanceArea da3;
  QVERIFY( da3.setEllipsoid( u"PARAMETER:2631400:2341350"_s ) );
  QVERIFY( da3.willUseEllipsoid() );
  QCOMPARE( da3.ellipsoidSemiMajor(), 2631400.0 );
  QCOMPARE( da3.ellipsoidSemiMinor(), 2341350.0 );
  QGSCOMPARENEAR( da3.ellipsoidInverseFlattening(), 9.07223, 0.00001 );
  QCOMPARE( da3.ellipsoid(), u"PARAMETER:2631400:2341350"_s );

  // again, to check parameters with cache
  QgsDistanceArea da4;
  QVERIFY( da4.setEllipsoid( u"PARAMETER:2631400:2341350"_s ) );
  QVERIFY( da4.willUseEllipsoid() );
  QCOMPARE( da4.ellipsoidSemiMajor(), 2631400.0 );
  QCOMPARE( da4.ellipsoidSemiMinor(), 2341350.0 );
  QGSCOMPARENEAR( da4.ellipsoidInverseFlattening(), 9.07223, 0.00001 );
  QCOMPARE( da4.ellipsoid(), u"PARAMETER:2631400:2341350"_s );

  // invalid
  QgsDistanceArea da5;
  QVERIFY( !da5.setEllipsoid( u"MyFirstEllipsoid"_s ) );
  QVERIFY( !da5.willUseEllipsoid() );
  QCOMPARE( da5.ellipsoid(), Qgis::geoNone() );

  // invalid again, should be cached
  QgsDistanceArea da6;
  QVERIFY( !da6.setEllipsoid( u"MyFirstEllipsoid"_s ) );
  QVERIFY( !da6.willUseEllipsoid() );
  QCOMPARE( da6.ellipsoid(), Qgis::geoNone() );
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
  myDa.setSourceCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"EPSG:4030"_s ), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( u"WGS84"_s );

  const QString myFileName = QStringLiteral( TEST_DATA_DIR ) + "/GeodTest-nano.dat";

  QFile myFile( myFileName );
  if ( !myFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QFAIL( "Couldn't open file" );
    return;
  }
  QTextStream in( &myFile );
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
      // QgsDebugMsgLevel( u"Distance from %1 to %2 is %3"_s.arg( p1.toString( 15 ) ).arg( p2.toString( 15 ) ).arg( result, 0, 'g', 15 ), 2 );
      // QgsDebugMsgLevel( u"Distance should be %1"_s.arg( myLineList[6] ), 2 );
      // Check result is less than 0.5mm from expected.
      QGSCOMPARENEAR( result, myLineList[6].toDouble(), 0.0005 );
    }
  }
}

void TestQgsDistanceArea::regression13601()
{
  //test regression #13601
  QgsDistanceArea calc;
  calc.setEllipsoid( u"NONE"_s );
  calc.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:3148"_s ), QgsProject::instance()->transformContext() );
  const QgsGeometry geom( QgsGeometryFactory::geomFromWkt( u"Polygon ((252000 1389000, 265000 1389000, 265000 1385000, 252000 1385000, 252000 1389000))"_s ).release() );
  QGSCOMPARENEAR( calc.measureArea( geom ), 52000000, 0.0001 );
}

void TestQgsDistanceArea::collections()
{
  //test measuring for collections
  QgsDistanceArea myDa;
  myDa.setSourceCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"EPSG:4030"_s ), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( u"WGS84"_s );

  //collection of lines, should be sum of line length
  const QgsGeometry lines( QgsGeometryFactory::geomFromWkt( u"GeometryCollection( LineString(0 36.53, 5.76 -48.16), LineString(0 25.54, 24.20 36.70) )"_s ).release() );
  double result = myDa.measureLength( lines );
  QGSCOMPARENEAR( result, 12006159, 1 );
  result = myDa.measureArea( lines );
  QGSCOMPARENEAR( result, 0, 4 * std::numeric_limits<double>::epsilon() );

  //collection of polygons

  const QgsGeometry poly1 = QgsGeometry::fromWkt( u"Polygon((0 36.53, 5.76 -48.16, 0 25.54, 0 36.53))"_s );
  result = myDa.measureArea( poly1 );
  QGSCOMPARENEAR( result, 439881520607.079712, 1 );
  result = myDa.measureLength( poly1 );
  QGSCOMPARENEAR( result, 0, 4 * std::numeric_limits<double>::epsilon() );
  const QgsGeometry poly2 = QgsGeometry::fromWkt( u"Polygon((10 20, 15 20, 15 10, 10 20))"_s );
  result = myDa.measureArea( poly2 );
  QGSCOMPARENEAR( result, 290350317025.906982, 1 );
  result = myDa.measureLength( poly2 );
  QGSCOMPARENEAR( result, 0, 4 * std::numeric_limits<double>::epsilon() );

  const QgsGeometry polys( QgsGeometryFactory::geomFromWkt( u"GeometryCollection( Polygon((0 36.53, 5.76 -48.16, 0 25.54, 0 36.53)), Polygon((10 20, 15 20, 15 10, 10 20)) )"_s ).release() );
  result = myDa.measureArea( polys );
  QGSCOMPARENEAR( result, 730231837632.98669, 1 );
  result = myDa.measureLength( polys );
  QGSCOMPARENEAR( result, 0, 4 * std::numeric_limits<double>::epsilon() );

  //mixed collection
  const QgsGeometry mixed( QgsGeometryFactory::geomFromWkt( u"GeometryCollection( LineString(0 36.53, 5.76 -48.16), LineString(0 25.54, 24.20 36.70), Polygon((0 36.53, 5.76 -48.16, 0 25.54, 0 36.53)), Polygon((10 20, 15 20, 15 10, 10 20)) )"_s ).release() );
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
  calc.setEllipsoid( u"NONE"_s );
  calc.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:2272"_s ), QgsProject::instance()->transformContext() );
  Qgis::DistanceUnit units;
  const QgsPointXY p1( 1341683.9854275715, 408256.9562717728 );
  const QgsPointXY p2( 1349321.7807031618, 408256.9562717728 );

  double result = calc.measureLine( p1, p2 );
  units = calc.lengthUnits();
  //no OTF, result will be in CRS unit (feet)
  QCOMPARE( units, Qgis::DistanceUnit::FeetUSSurvey );
  QGSCOMPARENEAR( result, 7637.7952755903825, 0.001 );

  calc.setEllipsoid( u"WGS84"_s );
  units = calc.lengthUnits();
  result = calc.measureLine( p1, p2 );
  //OTF, result will be in meters
  QCOMPARE( units, Qgis::DistanceUnit::Meters );
  QGSCOMPARENEAR( result, 2328.0988253106957, 0.001 );
}

void TestQgsDistanceArea::measureAreaAndUnits()
{
  QgsDistanceArea da;
  da.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), QgsProject::instance()->transformContext() );
  da.setEllipsoid( u"NONE"_s );
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
  Qgis::AreaUnit units = da.areaUnits();

  QgsDebugMsgLevel( u"measured %1 in %2"_s.arg( area ).arg( QgsUnitTypes::toString( units ) ), 1 );

  QVERIFY( ( qgsDoubleNear( area, 3.0, 0.00000001 ) && units == Qgis::AreaUnit::SquareDegrees ) || ( qgsDoubleNear( area, 37176087091.5, 0.1 ) && units == Qgis::AreaUnit::SquareMeters ) );

  da.setEllipsoid( u"WGS84"_s );
  area = da.measureArea( polygon );
  units = da.areaUnits();
  QgsDebugMsgLevel( u"measured %1 in %2"_s.arg( area ).arg( QgsUnitTypes::toString( units ) ), 1 );
  // should always be in Meters Squared
  QGSCOMPARENEAR( area, 36922805935.961571, 0.1 );
  QCOMPARE( units, Qgis::AreaUnit::SquareMeters );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, Qgis::AreaUnit::SquareMiles );
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

  da.setSourceCrs( QgsCoordinateReferenceSystem( u"ESRI:102635"_s ), QgsProject::instance()->transformContext() );
  da.setEllipsoid( u"NONE"_s );
  // measurement should be in square feet
  area = da.measureArea( polygon );
  units = da.areaUnits();
  QgsDebugMsgLevel( u"measured %1 in %2"_s.arg( area ).arg( QgsUnitTypes::toString( units ) ), 1 );
  QGSCOMPARENEAR( area, 2000000, 0.001 );
  QCOMPARE( units, Qgis::AreaUnit::SquareFeet );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, Qgis::AreaUnit::SquareYards );
  QGSCOMPARENEAR( area, 222222.2222, 0.001 );

  da.setEllipsoid( u"WGS84"_s );
  // now should be in Square Meters again
  area = da.measureArea( polygon );
  units = da.areaUnits();
  QgsDebugMsgLevel( u"measured %1 in %2"_s.arg( area ).arg( QgsUnitTypes::toString( units ) ), 1 );
  QGSCOMPARENEAR( area, 185825.206903, 1.0 );
  QCOMPARE( units, Qgis::AreaUnit::SquareMeters );

  // test converting the resultant area
  area = da.convertAreaMeasurement( area, Qgis::AreaUnit::SquareYards );
  QgsDebugMsgLevel( u"measured %1 in sq yrds"_s.arg( area ), 1 );
  QGSCOMPARENEAR( area, 222245.097808, 1.0 );
}

void TestQgsDistanceArea::emptyPolygon()
{
  QgsDistanceArea da;
  da.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), QgsProject::instance()->transformContext() );
  da.setEllipsoid( u"WGS84"_s );

  //test that measuring an empty polygon doesn't crash
  da.measurePolygon( QVector<QgsPointXY>() );
}

void TestQgsDistanceArea::regression14675()
{
  //test regression #14675
  QgsDistanceArea calc;
  calc.setEllipsoid( u"GRS80"_s );
  calc.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:2154"_s ), QgsProject::instance()->transformContext() );
  const QgsGeometry geom( QgsGeometryFactory::geomFromWkt( u"Polygon ((917593.5791854317067191 6833700.00807378999888897, 917596.43389983859378844 6833700.67099479306489229, 917599.53056440979707986 6833700.78673478215932846, 917593.5791854317067191 6833700.00807378999888897))"_s ).release() );
  QGSCOMPARENEAR( calc.measureArea( geom ), 0.861747, 0.001 );
}

void TestQgsDistanceArea::regression16820()
{
  QgsDistanceArea calc;
  calc.setEllipsoid( u"WGS84"_s );
  calc.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:32634"_s ), QgsProject::instance()->transformContext() );
  const QgsGeometry geom( QgsGeometryFactory::geomFromWkt( u"Polygon ((110250.54038314701756462 5084495.57398066483438015, 110243.46975068224128336 5084507.17200060561299324, 110251.23908144699817058 5084506.68309532757848501, 110251.2394439501222223 5084506.68307251576334238, 110250.54048078990308568 5084495.57553235255181789, 110250.54038314701756462 5084495.57398066483438015))"_s ).release() );
  QGSCOMPARENEAR( calc.measureArea( geom ), 43.201092, 0.001 );
}

void TestQgsDistanceArea::regression61299()
{
  // Create custom CRS from WKT
  const QgsCoordinateReferenceSystem userCrs = QgsCoordinateReferenceSystem::fromWkt( R"WKT(
PROJCRS["Hanseong PCS",
   BASEGEOGCRS["WGS 84",
       DATUM["World Geodetic System 1984",
           ELLIPSOID["WGS 84",6378137,298.257223563,
               LENGTHUNIT["metre",1]],
           ID["EPSG",6326]],
       PRIMEM["Greenwich",0,
           ANGLEUNIT["Degree",0.0174532925199433],
           ID["EPSG",8901]]],
   CONVERSION["unnamed",
       METHOD["Mercator (variant B)",
           ID["EPSG",9805]],
       PARAMETER["Latitude of 1st standard parallel",37.5,
           ANGLEUNIT["Degree",0.0174532925199433],
           ID["EPSG",8823]],
       PARAMETER["Longitude of natural origin",100,
           ANGLEUNIT["Degree",0.0174532925199433],
           ID["EPSG",8802]],
       PARAMETER["False easting",0,
           LENGTHUNIT["metre",1],
           ID["EPSG",8806]],
       PARAMETER["False northing",0,
           LENGTHUNIT["metre",1],
           ID["EPSG",8807]]],
   CS[Cartesian,2],
       AXIS["(E)",east,
           ORDER[1],
           LENGTHUNIT["metre",1,
               ID["EPSG",9001]]],
       AXIS["(N)",north,
           ORDER[2],
           LENGTHUNIT["metre",1,
               ID["EPSG",9001]]]]

)WKT" );

  QVERIFY( userCrs.isValid() );
  QgsProject::instance()->setCrs( userCrs, false );
  QgsProject::instance()->setEllipsoid( u"PARAMETER:6378137:6356752.3142451793"_s );

  QgsDistanceArea calc;
  QVERIFY( calc.setEllipsoid( QgsProject::instance()->ellipsoid() ) );
  calc.setSourceCrs( userCrs, QgsProject::instance()->transformContext() );
  QgsPointXY pt1( 110, 38 );
  QgsPointXY pt2( 111, 38 );
  // Transform to the user CRS
  QgsCoordinateTransform transform( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), userCrs, QgsProject::instance()->transformContext() );
  pt1 = transform.transform( pt1 );
  pt2 = transform.transform( pt2 );

  // This will fail if the ellipsoid is not set correctly
  QVERIFY( calc.ellipsoidCrs().isValid() );

  const double result = calc.measureLine( pt1, pt2 );
  QVERIFY( !std::isnan( result ) );
}


QGSTEST_MAIN( TestQgsDistanceArea )
#include "testqgsdistancearea.moc"
