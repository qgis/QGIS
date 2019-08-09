/***************************************************************************
                         testqgscoordinatetransform.cpp
                         -----------------------
    begin                : October 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscoordinatetransform.h"
#include "qgsapplication.h"
#include "qgsrectangle.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsproject.h"
#include <QObject>
#include "qgstest.h"
#include "qgsexception.h"

class TestQgsCoordinateTransform: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void transformBoundingBox();
    void copy();
    void assignment();
    void isValid();
    void isShortCircuited();
    void contextShared();
    void scaleFactor();
    void scaleFactor_data();
    void transform_data();
    void transform();
    void transformLKS();
    void transformContextNormalize();
};


void TestQgsCoordinateTransform::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsCoordinateTransform::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCoordinateTransform::copy()
{
  QgsCoordinateTransform uninitialized;
  QgsCoordinateTransform uninitializedCopy( uninitialized );
  QVERIFY( !uninitializedCopy.isValid() );

  QgsCoordinateReferenceSystem source;
  source.createFromId( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsCoordinateReferenceSystem destination;
  destination.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );

  QgsCoordinateTransform original( source, destination, QgsProject::instance() );
  QVERIFY( original.isValid() );

  QgsCoordinateTransform copy( original );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.sourceCrs().authid(), original.sourceCrs().authid() );
  QCOMPARE( copy.destinationCrs().authid(), original.destinationCrs().authid() );

  // force detachement of copy
  QgsCoordinateReferenceSystem newDest;
  newDest.createFromId( 3857, QgsCoordinateReferenceSystem::EpsgCrsId );
  copy.setDestinationCrs( newDest );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.destinationCrs().authid(), QString( "EPSG:3857" ) );
  QCOMPARE( original.destinationCrs().authid(), QString( "EPSG:4326" ) );
}

void TestQgsCoordinateTransform::assignment()
{
  QgsCoordinateTransform uninitialized;
  QgsCoordinateTransform uninitializedCopy;
  uninitializedCopy = uninitialized;
  QVERIFY( !uninitializedCopy.isValid() );

  QgsCoordinateReferenceSystem source;
  source.createFromId( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsCoordinateReferenceSystem destination;
  destination.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );

  QgsCoordinateTransform original( source, destination, QgsProject::instance() );
  QVERIFY( original.isValid() );

  QgsCoordinateTransform copy;
  copy = original;
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.sourceCrs().authid(), original.sourceCrs().authid() );
  QCOMPARE( copy.destinationCrs().authid(), original.destinationCrs().authid() );

  // force detachement of copy
  QgsCoordinateReferenceSystem newDest;
  newDest.createFromId( 3857, QgsCoordinateReferenceSystem::EpsgCrsId );
  copy.setDestinationCrs( newDest );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.destinationCrs().authid(), QString( "EPSG:3857" ) );
  QCOMPARE( original.destinationCrs().authid(), QString( "EPSG:4326" ) );

  // test assigning back to invalid
  copy = uninitialized;
  QVERIFY( !copy.isValid() );
  QVERIFY( original.isValid() );
}

void TestQgsCoordinateTransform::isValid()
{
  QgsCoordinateTransform tr;
  QVERIFY( !tr.isValid() );

  QgsCoordinateReferenceSystem srs1;
  srs1.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem srs2;
  srs2.createFromSrid( 4326 );

  // valid source, invalid destination
  QgsCoordinateTransform tr2( srs1, QgsCoordinateReferenceSystem(), QgsProject::instance() );
  QVERIFY( !tr2.isValid() );

  // invalid source, valid destination
  QgsCoordinateTransform tr3( QgsCoordinateReferenceSystem(), srs2, QgsProject::instance() );
  QVERIFY( !tr3.isValid() );

  // valid source, valid destination
  QgsCoordinateTransform tr4( srs1, srs2, QgsProject::instance() );
  QVERIFY( tr4.isValid() );

  // try to invalidate by setting source as invalid
  tr4.setSourceCrs( QgsCoordinateReferenceSystem() );
  QVERIFY( !tr4.isValid() );

  QgsCoordinateTransform tr5( srs1, srs2, QgsProject::instance() );
  // try to invalidate by setting destination as invalid
  tr5.setDestinationCrs( QgsCoordinateReferenceSystem() );
  QVERIFY( !tr5.isValid() );
}

void TestQgsCoordinateTransform::isShortCircuited()
{
  QgsCoordinateTransform tr;
  //invalid transform shortcircuits
  QVERIFY( tr.isShortCircuited() );

  QgsCoordinateReferenceSystem srs1;
  srs1.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem srs2;
  srs2.createFromSrid( 4326 );

  // valid source, invalid destination
  QgsCoordinateTransform tr2( srs1, QgsCoordinateReferenceSystem(), QgsProject::instance() );
  QVERIFY( tr2.isShortCircuited() );

  // invalid source, valid destination
  QgsCoordinateTransform tr3( QgsCoordinateReferenceSystem(), srs2, QgsProject::instance() );
  QVERIFY( tr3.isShortCircuited() );

  // equal, valid source and destination
  QgsCoordinateTransform tr4( srs1, srs1, QgsProject::instance() );
  QVERIFY( tr4.isShortCircuited() );

  // valid but different source and destination
  QgsCoordinateTransform tr5( srs1, srs2, QgsProject::instance() );
  QVERIFY( !tr5.isShortCircuited() );

  // try to short circuit by changing dest
  tr5.setDestinationCrs( srs1 );
  QVERIFY( tr5.isShortCircuited() );
}

void TestQgsCoordinateTransform::contextShared()
{
  //test implicit sharing of QgsCoordinateTransformContext
#if PROJ_VERSION_MAJOR >= 6
  QgsCoordinateTransformContext original;
  original.addCoordinateOperation( QgsCoordinateReferenceSystem( 3111 ), QgsCoordinateReferenceSystem( 3113 ), QStringLiteral( "proj" ) );

  QgsCoordinateTransformContext copy( original );
  QMap< QPair< QString, QString >, QString > expected;
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj" ) );
  QCOMPARE( original.coordinateOperations(), expected );
  QCOMPARE( copy.coordinateOperations(), expected );

  // trigger detach
  copy.addCoordinateOperation( QgsCoordinateReferenceSystem( 3111 ), QgsCoordinateReferenceSystem( 3113 ), QStringLiteral( "proj2" ) );
  QCOMPARE( original.coordinateOperations(), expected );

  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj2" ) );
  QCOMPARE( copy.coordinateOperations(), expected );

  // copy via assignment
  QgsCoordinateTransformContext copy2;
  copy2 = original;
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj" ) );
  QCOMPARE( original.coordinateOperations(), expected );
  QCOMPARE( copy2.coordinateOperations(), expected );

  copy2.addCoordinateOperation( QgsCoordinateReferenceSystem( 3111 ), QgsCoordinateReferenceSystem( 3113 ), QStringLiteral( "proj2" ) );
  QCOMPARE( original.coordinateOperations(), expected );
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj2" ) );
  QCOMPARE( copy2.coordinateOperations(), expected );
#else
  QgsCoordinateTransformContext original;
  Q_NOWARN_DEPRECATED_PUSH
  original.addSourceDestinationDatumTransform( QgsCoordinateReferenceSystem( 3111 ), QgsCoordinateReferenceSystem( 3113 ), 1, 2 );

  QgsCoordinateTransformContext copy( original );
  QMap< QPair< QString, QString >, QgsDatumTransform::TransformPair > expected;
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QgsDatumTransform::TransformPair( 1, 2 ) );
  QCOMPARE( original.sourceDestinationDatumTransforms(), expected );
  QCOMPARE( copy.sourceDestinationDatumTransforms(), expected );

  // trigger detach
  copy.addSourceDestinationDatumTransform( QgsCoordinateReferenceSystem( 3111 ), QgsCoordinateReferenceSystem( 3113 ), 3, 4 );
  QCOMPARE( original.sourceDestinationDatumTransforms(), expected );

  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QgsDatumTransform::TransformPair( 3, 4 ) );
  QCOMPARE( copy.sourceDestinationDatumTransforms(), expected );

  // copy via assignment
  QgsCoordinateTransformContext copy2;
  copy2 = original;
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QgsDatumTransform::TransformPair( 1, 2 ) );
  QCOMPARE( original.sourceDestinationDatumTransforms(), expected );
  QCOMPARE( copy2.sourceDestinationDatumTransforms(), expected );

  copy2.addSourceDestinationDatumTransform( QgsCoordinateReferenceSystem( 3111 ), QgsCoordinateReferenceSystem( 3113 ), 3, 4 );
  QCOMPARE( original.sourceDestinationDatumTransforms(), expected );
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QgsDatumTransform::TransformPair( 3, 4 ) );
  QCOMPARE( copy2.sourceDestinationDatumTransforms(), expected );

  Q_NOWARN_DEPRECATED_POP
#endif
}

void TestQgsCoordinateTransform::scaleFactor()
{
  QFETCH( QgsCoordinateReferenceSystem, sourceCrs );
  QFETCH( QgsCoordinateReferenceSystem, destCrs );
  QFETCH( QgsRectangle, rect );
  QFETCH( double, factor );

  QgsCoordinateTransform ct( sourceCrs, destCrs, QgsProject::instance() );
  try
  {
    QGSCOMPARENEAR( ct.scaleFactor( rect ), factor, 0.000001 );
  }
  catch ( QgsCsException & )
  {
    QVERIFY( false );
  }

}

void TestQgsCoordinateTransform::scaleFactor_data()
{
  QTest::addColumn<QgsCoordinateReferenceSystem>( "sourceCrs" );
  QTest::addColumn<QgsCoordinateReferenceSystem>( "destCrs" );
  QTest::addColumn<QgsRectangle>( "rect" );
  QTest::addColumn<double>( "factor" );

  QTest::newRow( "Different map units" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 2056 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsRectangle( 2550000, 1200000, 2550100, 1200100 )
      << 1.1223316038381985e-5;
  QTest::newRow( "Same map units" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3111 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 28355 )
      << QgsRectangle( 2560536.7, 2331787.5, 2653161.1, 2427370.4 )
      << 0.999632;
  QTest::newRow( "Same CRS" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 2056 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 2056 )
      << QgsRectangle( 2550000, 1200000, 2550100, 1200100 )
      << 1.0;
}

void TestQgsCoordinateTransform::transform_data()
{
  QTest::addColumn<QgsCoordinateReferenceSystem>( "sourceCrs" );
  QTest::addColumn<QgsCoordinateReferenceSystem>( "destCrs" );
  QTest::addColumn<double>( "x" );
  QTest::addColumn<double>( "y" );
  QTest::addColumn<int>( "direction" );
  QTest::addColumn<double>( "outX" );
  QTest::addColumn<double>( "outY" );
  QTest::addColumn<double>( "precision" );

  QTest::newRow( "To geographic" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3111 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 2545059.0 << 2393190.0 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << 145.512750 << -37.961375 << 0.000001;
  QTest::newRow( "From geographic" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3111 )
      << 145.512750 <<  -37.961375 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << 2545059.0 << 2393190.0 << 0.1;
  QTest::newRow( "From geographic to geographic" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4164 )
      << 145.512750 <<  -37.961375 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << 145.510966 <<  -37.961741 << 0.0001;
  QTest::newRow( "To geographic (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3111 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 145.512750 <<  -37.961375 << static_cast< int >( QgsCoordinateTransform::ReverseTransform ) << 2545059.0 << 2393190.0 << 0.1;
  QTest::newRow( "From geographic (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3111 )
      << 2545058.9675128171 << 2393190.0509782173 << static_cast< int >( QgsCoordinateTransform::ReverseTransform ) << 145.512750 << -37.961375 << 0.000001;
  QTest::newRow( "From geographic to geographic reverse" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4164 )
      << 145.510966 <<  -37.961741 << static_cast< int >( QgsCoordinateTransform::ReverseTransform ) <<  145.512750 <<  -37.961375 << 0.0001;
  QTest::newRow( "From LKS92/TM to Baltic93/TM" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3059 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 25884 )
      << 725865.850 << 198519.947 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << 725865.850 << 6198519.947 << 0.001;
  QTest::newRow( "From LKS92/TM to Baltic93/TM (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3059 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 25884 )
      << 725865.850 << 6198519.947 << static_cast< int >( QgsCoordinateTransform::ReverseTransform ) << 725865.850 << 198519.947 << 0.001;
  QTest::newRow( "From LKS92/TM to WGS84" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3059 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 725865.850 << 198519.947 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << 27.61113711 << 55.87910378 << 0.00000001;
  QTest::newRow( "From LKS92/TM to WGS84 (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3059 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 27.61113711 << 55.87910378 << static_cast< int >( QgsCoordinateTransform::ReverseTransform ) << 725865.850 << 198519.947 << 0.001;
  QTest::newRow( "From BNG to WGS84" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 7467023.96 << -5527971.74 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << 51.400222 << 0.000025 << 0.4;
  QTest::newRow( "From BNG to WGS84 2" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 246909.0 << 54108.0 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << -4.153951 <<  50.366908  << 0.4;
  QTest::newRow( "From BNG to WGS84 (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 51.400222 << 0.000025 << static_cast< int >( QgsCoordinateTransform::ReverseTransform ) << 7467023.96 << -5527971.74 << 22000.0;
  QTest::newRow( "From WGS84 to BNG (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << 7467023.96 << -5527971.74 << static_cast< int >( QgsCoordinateTransform::ReverseTransform ) << 51.400222 << 0.000025 << 0.4;
  QTest::newRow( "From WGS84 to BNG" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << 51.400222 << 0.000025 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << 7467023.96 << -5527971.74 << 22000.0;
  QTest::newRow( "From BNG to 3857" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3857 )
      << 7467023.96 << -5527971.74 << static_cast< int >( QgsCoordinateTransform::ForwardTransform ) << 5721846.47 << 2.78 << 43000.0;
  QTest::newRow( "From BNG to 3857 (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3857 )
      << 5721846.47 << 2.78 << static_cast< int >( QgsCoordinateTransform::ReverseTransform )  << 7467023.96 << -5527971.74 << 22000.0;
}

void TestQgsCoordinateTransform::transform()
{
  QFETCH( QgsCoordinateReferenceSystem, sourceCrs );
  QFETCH( QgsCoordinateReferenceSystem, destCrs );
  QFETCH( double, x );
  QFETCH( double, y );
  QFETCH( int, direction );
  QFETCH( double, outX );
  QFETCH( double, outY );
  QFETCH( double, precision );

  double z = 0;
  QgsCoordinateTransform ct( sourceCrs, destCrs, QgsProject::instance() );

  ct.transformInPlace( x, y, z, static_cast<  QgsCoordinateTransform::TransformDirection >( direction ) );
  QGSCOMPARENEAR( x, outX, precision );
  QGSCOMPARENEAR( y, outY, precision );
}

void TestQgsCoordinateTransform::transformBoundingBox()
{
  //test transforming a bounding box which crosses the 180 degree longitude line
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4326 );

  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );
  QgsRectangle crossingRect( 6374985, -3626584, 7021195, -3272435 );
  QgsRectangle resultRect = tr.transformBoundingBox( crossingRect, QgsCoordinateTransform::ForwardTransform, true );
  QgsRectangle expectedRect;
  expectedRect.setXMinimum( 175.771 );
  expectedRect.setYMinimum( -39.7222 );
  expectedRect.setXMaximum( -176.549 );
  expectedRect.setYMaximum( -36.3951 );

  qDebug( "BBox transform x min: %.17f", resultRect.xMinimum() );
  qDebug( "BBox transform x max: %.17f", resultRect.xMaximum() );
  qDebug( "BBox transform y min: %.17f", resultRect.yMinimum() );
  qDebug( "BBox transform y max: %.17f", resultRect.yMaximum() );

  QGSCOMPARENEAR( resultRect.xMinimum(), expectedRect.xMinimum(), 0.001 );
  QGSCOMPARENEAR( resultRect.yMinimum(), expectedRect.yMinimum(), 0.001 );
  QGSCOMPARENEAR( resultRect.xMaximum(), expectedRect.xMaximum(), 0.001 );
  QGSCOMPARENEAR( resultRect.yMaximum(), expectedRect.yMaximum(), 0.001 );

  // test transforming a bounding box, resulting in an invalid transform - exception must be thrown
  tr = QgsCoordinateTransform( QgsCoordinateReferenceSystem( 4326 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ), QgsProject::instance() );
  QgsRectangle rect( -99999999999, 99999999999, -99999999998, 99999999998 );
  bool errorObtained = false;
  try
  {
    resultRect = tr.transformBoundingBox( rect );
  }
  catch ( QgsCsException & )
  {
    errorObtained = true;
  }
  QVERIFY( errorObtained );
}

void TestQgsCoordinateTransform::transformLKS()
{
  QgsCoordinateReferenceSystem LKS92 = QgsCoordinateReferenceSystem::fromEpsgId( 3059 );
  QVERIFY( LKS92.isValid() );
  QgsCoordinateReferenceSystem Baltic93 = QgsCoordinateReferenceSystem::fromEpsgId( 25884 );
  QVERIFY( Baltic93.isValid() );
  QgsCoordinateReferenceSystem WGS84 = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( WGS84.isValid() );

  QgsCoordinateTransform Lks2Balt( LKS92, Baltic93, QgsProject::instance() );
  QVERIFY( Lks2Balt.isValid() );
  QgsCoordinateTransform Lks2Wgs( LKS92, WGS84, QgsProject::instance() );
  QVERIFY( Lks2Wgs.isValid() );

  QPolygonF sPoly = QgsGeometry::fromWkt( QStringLiteral( "Polygon (( 725865.850 198519.947, 363511.181 263208.769, 717694.697 333650.333, 725865.850 198519.947 ))" ) ).asQPolygonF();

  Lks2Balt.transformPolygon( sPoly, QgsCoordinateTransform::ForwardTransform );

  QGSCOMPARENEAR( sPoly.at( 0 ).x(), 725865.850, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 0 ).y(), 6198519.947, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 1 ).x(), 363511.181, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 1 ).y(), 6263208.769, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 2 ).x(), 717694.697, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 2 ).y(), 6333650.333, 0.001 );
}

void TestQgsCoordinateTransform::transformContextNormalize()
{
#if PROJ_VERSION_MAJOR >= 6
  // coordinate operation for WGS84 to 27700
  const QString coordOperation = QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=WGS84 +step +inv +proj=helmert +x=446.448 +y=-125.157 +z=542.06 +rx=0.15 +ry=0.247 +rz=0.842 +s=-20.489 +convention=position_vector +step +inv +proj=cart +ellps=airy +step +proj=pop +v_3 +step +proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy" );
  QgsCoordinateTransformContext context;
  context.addCoordinateOperation( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsCoordinateReferenceSystem::fromEpsgId( 27700 ), coordOperation );

  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsCoordinateReferenceSystem::fromEpsgId( 27700 ), context );
  QVERIFY( ct.isValid() );
  QgsPointXY p( -4.17477, 50.3657 );
  QgsPointXY p2 = ct.transform( p );
  QGSCOMPARENEAR( p2.x(), 245424.604645, 0.01 );
  QGSCOMPARENEAR( p2.y(), 54016.813093, 0.01 );

  p = ct.transform( p2, QgsCoordinateTransform::ReverseTransform );
  QGSCOMPARENEAR( p.x(), -4.17477, 0.01 );
  QGSCOMPARENEAR( p.y(), 50.3657, 0.01 );

  QgsCoordinateTransform ct2( QgsCoordinateReferenceSystem::fromEpsgId( 27700 ), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), context );
  QVERIFY( ct2.isValid() );
  p = QgsPointXY( 245424.604645, 54016.813093 );
  p2 = ct2.transform( p );
  QGSCOMPARENEAR( p2.x(), -4.17477, 0.01 );
  QGSCOMPARENEAR( p2.y(), 50.3657, 0.01 );

  p = ct2.transform( p2, QgsCoordinateTransform::ReverseTransform );
  QGSCOMPARENEAR( p.x(), 245424.604645, 0.01 );
  QGSCOMPARENEAR( p.y(), 54016.813093, 0.01 );

#endif
}

QGSTEST_MAIN( TestQgsCoordinateTransform )
#include "testqgscoordinatetransform.moc"
