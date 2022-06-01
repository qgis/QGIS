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
#include "qgslogger.h"

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
    void constructorFlags();
    void scaleFactor_data();
    void transform_data();
    void transform();
#if PROJ_VERSION_MAJOR>7 || (PROJ_VERSION_MAJOR == 7 && PROJ_VERSION_MINOR >= 2)
    void transformEpoch_data();
    void transformEpoch();
    void dynamicToDynamicErrorHandler();
#endif
    void transformLKS();
    void transformContextNormalize();
    void transform2DPoint();
    void transformErrorMultiplePoints();
    void transformErrorOnePoint();
    void testDeprecated4240to4326();
    void testCustomProjTransform();
    void testTransformationIsPossible();
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
  const QgsCoordinateTransform uninitialized;
  const QgsCoordinateTransform uninitializedCopy( uninitialized );
  QVERIFY( !uninitializedCopy.isValid() );

  const QgsCoordinateReferenceSystem source( QStringLiteral( "EPSG:3111" ) );
  const QgsCoordinateReferenceSystem destination( QStringLiteral( "EPSG:4326" ) );

  const QgsCoordinateTransform original( source, destination, QgsProject::instance() );
  QVERIFY( original.isValid() );

  QgsCoordinateTransform copy( original );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.sourceCrs().authid(), original.sourceCrs().authid() );
  QCOMPARE( copy.destinationCrs().authid(), original.destinationCrs().authid() );

  // force detachement of copy
  const QgsCoordinateReferenceSystem newDest( QStringLiteral( "EPSG:3857" ) );
  copy.setDestinationCrs( newDest );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.destinationCrs().authid(), QString( "EPSG:3857" ) );
  QCOMPARE( original.destinationCrs().authid(), QString( "EPSG:4326" ) );
}

void TestQgsCoordinateTransform::assignment()
{
  const QgsCoordinateTransform uninitialized;
  QgsCoordinateTransform uninitializedCopy;
  uninitializedCopy = uninitialized;
  QVERIFY( !uninitializedCopy.isValid() );

  const QgsCoordinateReferenceSystem source( QStringLiteral( "EPSG:3111" ) );
  const QgsCoordinateReferenceSystem destination( QStringLiteral( "EPSG:4326" ) );

  const QgsCoordinateTransform original( source, destination, QgsProject::instance() );
  QVERIFY( original.isValid() );

  QgsCoordinateTransform copy;
  copy = original;
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.sourceCrs().authid(), original.sourceCrs().authid() );
  QCOMPARE( copy.destinationCrs().authid(), original.destinationCrs().authid() );

  // force detachement of copy
  const QgsCoordinateReferenceSystem newDest( QStringLiteral( "EPSG:3857" ) );
  copy.setDestinationCrs( newDest );
  QVERIFY( copy.isValid() );
  QCOMPARE( copy.destinationCrs().authid(), QStringLiteral( "EPSG:3857" ) );
  QCOMPARE( original.destinationCrs().authid(), QStringLiteral( "EPSG:4326" ) );

  // test assigning back to invalid
  copy = uninitialized;
  QVERIFY( !copy.isValid() );
  QVERIFY( original.isValid() );
}

void TestQgsCoordinateTransform::isValid()
{
  const QgsCoordinateTransform tr;
  QVERIFY( !tr.isValid() );

  const QgsCoordinateReferenceSystem srs1( QStringLiteral( "EPSG:3994" ) );
  const QgsCoordinateReferenceSystem srs2( QStringLiteral( "EPSG:4326" ) );

  // valid source, invalid destination
  const QgsCoordinateTransform tr2( srs1, QgsCoordinateReferenceSystem(), QgsProject::instance() );
  QVERIFY( !tr2.isValid() );

  // invalid source, valid destination
  const QgsCoordinateTransform tr3( QgsCoordinateReferenceSystem(), srs2, QgsProject::instance() );
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
  const QgsCoordinateTransform tr;
  //invalid transform shortcircuits
  QVERIFY( tr.isShortCircuited() );

  const QgsCoordinateReferenceSystem srs1( QStringLiteral( "EPSG:3994" ) );
  const QgsCoordinateReferenceSystem srs2( QStringLiteral( "EPSG:4326" ) );

  // valid source, invalid destination
  const QgsCoordinateTransform tr2( srs1, QgsCoordinateReferenceSystem(), QgsProject::instance() );
  QVERIFY( tr2.isShortCircuited() );

  // invalid source, valid destination
  const QgsCoordinateTransform tr3( QgsCoordinateReferenceSystem(), srs2, QgsProject::instance() );
  QVERIFY( tr3.isShortCircuited() );

  // equal, valid source and destination
  const QgsCoordinateTransform tr4( srs1, srs1, QgsProject::instance() );
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
  QgsCoordinateTransformContext original;
  original.addCoordinateOperation( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj" ) );

  QgsCoordinateTransformContext copy( original );
  QMap< QPair< QString, QString >, QString > expected;
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj" ) );
  QCOMPARE( original.coordinateOperations(), expected );
  QCOMPARE( copy.coordinateOperations(), expected );

  // trigger detach
  copy.addCoordinateOperation( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj2" ) );
  QCOMPARE( original.coordinateOperations(), expected );

  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj2" ) );
  QCOMPARE( copy.coordinateOperations(), expected );

  // copy via assignment
  QgsCoordinateTransformContext copy2;
  copy2 = original;
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj" ) );
  QCOMPARE( original.coordinateOperations(), expected );
  QCOMPARE( copy2.coordinateOperations(), expected );

  copy2.addCoordinateOperation( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj2" ) );
  QCOMPARE( original.coordinateOperations(), expected );
  expected.insert( qMakePair( QStringLiteral( "EPSG:3111" ), QStringLiteral( "EPSG:3113" ) ), QStringLiteral( "proj2" ) );
  QCOMPARE( copy2.coordinateOperations(), expected );
}

void TestQgsCoordinateTransform::scaleFactor()
{
  QFETCH( QgsCoordinateReferenceSystem, sourceCrs );
  QFETCH( QgsCoordinateReferenceSystem, destCrs );
  QFETCH( QgsRectangle, rect );
  QFETCH( double, factor );

  const QgsCoordinateTransform ct( sourceCrs, destCrs, QgsProject::instance() );
  try
  {
    QGSCOMPARENEAR( ct.scaleFactor( rect ), factor, 0.000001 );
  }
  catch ( QgsCsException & )
  {
    QVERIFY( false );
  }

}

void TestQgsCoordinateTransform::constructorFlags()
{
  const QgsCoordinateReferenceSystem srs1( QStringLiteral( "EPSG:3994" ) );
  const QgsCoordinateReferenceSystem srs2( QStringLiteral( "EPSG:4326" ) );

  // no flags
  QgsCoordinateTransform tr( srs1, srs2, QgsProject::instance() );
  QVERIFY( !tr.mBallparkTransformsAreAppropriate );
  QVERIFY( !tr.isShortCircuited() );
  QVERIFY( !tr.mIgnoreImpossible );

  QgsCoordinateTransform tr2( srs1, srs2, QgsProject::instance(), Qgis::CoordinateTransformationFlag::BallparkTransformsAreAppropriate );
  QVERIFY( tr2.mBallparkTransformsAreAppropriate );
  QVERIFY( !tr2.isShortCircuited() );
  QVERIFY( !tr2.mIgnoreImpossible );

  QgsCoordinateTransform tr3( srs1, srs2, QgsProject::instance(), Qgis::CoordinateTransformationFlag::IgnoreImpossibleTransformations | Qgis::CoordinateTransformationFlag::BallparkTransformsAreAppropriate );
  QVERIFY( tr3.mBallparkTransformsAreAppropriate );
  QVERIFY( !tr3.isShortCircuited() );
  QVERIFY( tr3.mIgnoreImpossible );

  QgsCoordinateTransform tr4( srs1, srs2, QgsProject::instance(), Qgis::CoordinateTransformationFlag::IgnoreImpossibleTransformations );
  QVERIFY( !tr4.mBallparkTransformsAreAppropriate );
  QVERIFY( !tr4.isShortCircuited() );
  QVERIFY( tr4.mIgnoreImpossible );

#if (PROJ_VERSION_MAJOR>8 || (PROJ_VERSION_MAJOR==8 && PROJ_VERSION_MINOR >= 1 ) )
  QgsCoordinateTransform tr5( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
                              QgsCoordinateReferenceSystem( QStringLiteral( "ESRI:104903" ) ),
                              QgsProject::instance(),
                              Qgis::CoordinateTransformationFlag::IgnoreImpossibleTransformations );
  QVERIFY( !tr5.mBallparkTransformsAreAppropriate );
  // crses are from two different celestial bodies, the transform is impossible and should be short-circuited
  QVERIFY( tr5.isShortCircuited() );
  QVERIFY( tr5.mIgnoreImpossible );
#endif
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
      << 2545059.0 << 2393190.0 << static_cast< int >( Qgis::TransformDirection::Forward ) << 145.512750 << -37.961375 << 0.000015;
  QTest::newRow( "From geographic" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3111 )
      << 145.512750 <<  -37.961375 << static_cast< int >( Qgis::TransformDirection::Forward ) << 2545059.0 << 2393190.0 << 1.5;
  QTest::newRow( "From geographic to geographic" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4164 )
      << 145.512750 <<  -37.961375 << static_cast< int >( Qgis::TransformDirection::Forward ) << 145.510966 <<  -37.961741 << 0.0001;
  QTest::newRow( "To geographic (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3111 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 145.512750 <<  -37.961375 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 2545059.0 << 2393190.0 << 1.5;
  QTest::newRow( "From geographic (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3111 )
      << 2545058.9675128171 << 2393190.0509782173 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 145.512750 << -37.961375 << 0.000015;
  QTest::newRow( "From geographic to geographic reverse" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4164 )
      << 145.510966 <<  -37.961741 << static_cast< int >( Qgis::TransformDirection::Reverse ) <<  145.512750 <<  -37.961375 << 0.0001;
  QTest::newRow( "From LKS92/TM to Baltic93/TM" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3059 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 25884 )
      << 725865.850 << 198519.947 << static_cast< int >( Qgis::TransformDirection::Forward ) << 725865.850 << 6198519.947 << 0.001;
  QTest::newRow( "From LKS92/TM to Baltic93/TM (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3059 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 25884 )
      << 725865.850 << 6198519.947 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 725865.850 << 198519.947 << 0.001;
  QTest::newRow( "From LKS92/TM to WGS84" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3059 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 725865.850 << 198519.947 << static_cast< int >( Qgis::TransformDirection::Forward ) << 27.61113711 << 55.87910378 << 0.00000001;
  QTest::newRow( "From LKS92/TM to WGS84 (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3059 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 27.61113711 << 55.87910378 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 725865.850 << 198519.947 << 0.001;
  QTest::newRow( "From BNG to WGS84" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 7467023.96 << -5527971.74 << static_cast< int >( Qgis::TransformDirection::Forward ) << 51.400222 << 0.000025 << 0.4;
  QTest::newRow( "From BNG to WGS84 2" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 246909.0 << 54108.0 << static_cast< int >( Qgis::TransformDirection::Forward ) << -4.153951 <<  50.366908  << 0.4;
  QTest::newRow( "From BNG to WGS84 (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << 51.400222 << 0.000025 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 7467023.96 << -5527971.74 << 22000.0;
  QTest::newRow( "From WGS84 to BNG (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << 7467023.96 << -5527971.74 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 51.400222 << 0.000025 << 0.4;
  QTest::newRow( "From WGS84 to BNG" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 4326 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << 51.400222 << 0.000025 << static_cast< int >( Qgis::TransformDirection::Forward ) << 7467023.96 << -5527971.74 << 22000.0;
  QTest::newRow( "From BNG to 3857" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3857 )
      << 7467023.96 << -5527971.74 << static_cast< int >( Qgis::TransformDirection::Forward ) << 5721846.47 << 2.78 << 43000.0;
  QTest::newRow( "From BNG to 3857 (reverse)" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 27700 )
      << QgsCoordinateReferenceSystem::fromEpsgId( 3857 )
      << 5721846.47 << 2.78 << static_cast< int >( Qgis::TransformDirection::Reverse )  << 7467023.96 << -5527971.74 << 22000.0;
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
  const QgsCoordinateTransform ct( sourceCrs, destCrs, QgsProject::instance() );

  ct.transformInPlace( x, y, z, static_cast<  Qgis::TransformDirection >( direction ) );
  QGSCOMPARENEAR( x, outX, precision );
  QGSCOMPARENEAR( y, outY, precision );
}

#if PROJ_VERSION_MAJOR>7 || (PROJ_VERSION_MAJOR == 7 && PROJ_VERSION_MINOR >= 2)
void TestQgsCoordinateTransform::transformEpoch_data()
{
  QTest::addColumn<QgsCoordinateReferenceSystem>( "sourceCrs" );
  QTest::addColumn<double>( "sourceEpoch" );
  QTest::addColumn<QgsCoordinateReferenceSystem>( "destCrs" );
  QTest::addColumn<double>( "destEpoch" );
  QTest::addColumn<double>( "srcX" );
  QTest::addColumn<double>( "srcY" );
  QTest::addColumn<int>( "direction" );
  QTest::addColumn<double>( "outX" );
  QTest::addColumn<double>( "outY" );
  QTest::addColumn<double>( "precision" );

  QTest::newRow( "GDA2020 to ITRF at central epoch -- no coordinate change expected" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 7844 ) // GDA2020
      << std::numeric_limits< double >::quiet_NaN()
      << QgsCoordinateReferenceSystem::fromEpsgId( 9000 ) // ITRF2014
      << 2020.0
      << 150.0 << -30.0 << static_cast< int >( Qgis::TransformDirection::Forward ) << 150.0 << -30.0 << 0.0000000001;

  QTest::newRow( "GDA2020 to ITRF at central epoch (reverse) -- no coordinate change expected" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 7844 ) // GDA2020
      << std::numeric_limits< double >::quiet_NaN()
      << QgsCoordinateReferenceSystem::fromEpsgId( 9000 ) // ITRF2014
      << 2020.0
      << 150.0 << -30.0 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 150.0 << -30.0 << 0.0000000001;

  QTest::newRow( "ITRF at central epoch to GDA2020 -- no coordinate change expected" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 9000 ) // ITRF2014
      << 2020.0
      << QgsCoordinateReferenceSystem::fromEpsgId( 7844 ) // GDA2020
      << std::numeric_limits< double >::quiet_NaN()
      << 150.0 << -30.0 << static_cast< int >( Qgis::TransformDirection::Forward ) << 150.0 << -30.0 << 0.0000000001;

  QTest::newRow( "ITRF at central epoch to GDA2020 (reverse) -- no coordinate change expected" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 9000 ) // ITRF2014
      << 2020.0
      << QgsCoordinateReferenceSystem::fromEpsgId( 7844 ) // GDA2020
      << std::numeric_limits< double >::quiet_NaN()
      << 150.0 << -30.0 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 150.0 << -30.0 << 0.0000000001;

  QTest::newRow( "GDA2020 to ITRF at 2030 -- coordinate change expected" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 7844 ) // GDA2020
      << std::numeric_limits< double >::quiet_NaN()
      << QgsCoordinateReferenceSystem::fromEpsgId( 9000 ) // ITRF2014
      << 2030.0
      << 150.0 << -30.0 << static_cast< int >( Qgis::TransformDirection::Forward ) << 150.0000022212 << -29.9999950478 << 0.0000001;

  QTest::newRow( "GDA2020 to ITRF at 2030 (reverse) -- coordinate change expected" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 7844 ) // GDA2020
      << std::numeric_limits< double >::quiet_NaN()
      << QgsCoordinateReferenceSystem::fromEpsgId( 9000 ) // ITRF2014
      << 2030.0
      << 150.0000022212 << -29.9999950478 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 150.0 << -30.0 << 0.0000001;

  QTest::newRow( "ITRF at 2030 to GDA2020-- coordinate change expected" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 9000 ) // ITRF2014
      << 2030.0
      << QgsCoordinateReferenceSystem::fromEpsgId( 7844 ) // GDA2020
      << std::numeric_limits< double >::quiet_NaN()
      << 150.0000022212 << -29.9999950478 << static_cast< int >( Qgis::TransformDirection::Forward ) << 150.0 << -30.0 << 0.0000001;

  QTest::newRow( "ITRF at 2030 to GDA2020 (reverse) -- coordinate change expected" )
      << QgsCoordinateReferenceSystem::fromEpsgId( 9000 ) // ITRF2014
      << 2030.0
      << QgsCoordinateReferenceSystem::fromEpsgId( 7844 ) // GDA2020
      << std::numeric_limits< double >::quiet_NaN()
      << 150.0 << -30.0 << static_cast< int >( Qgis::TransformDirection::Reverse ) << 150.0000022212 << -29.9999950478 << 0.0000001;
}

void TestQgsCoordinateTransform::transformEpoch()
{
  QFETCH( QgsCoordinateReferenceSystem, sourceCrs );
  QFETCH( double, sourceEpoch );
  QFETCH( QgsCoordinateReferenceSystem, destCrs );
  QFETCH( double, destEpoch );
  QFETCH( double, srcX );
  QFETCH( double, srcY );
  QFETCH( int, direction );
  QFETCH( double, outX );
  QFETCH( double, outY );
  QFETCH( double, precision );

  sourceCrs.setCoordinateEpoch( sourceEpoch );
  destCrs.setCoordinateEpoch( destEpoch );

  double z = 0;
  QgsCoordinateTransform ct( sourceCrs, destCrs, QgsProject::instance() );

  double x = srcX;
  double y = srcY;
  ct.transformInPlace( x, y, z, static_cast< Qgis::TransformDirection >( direction ) );
  QGSCOMPARENEAR( x, outX, precision );
  QGSCOMPARENEAR( y, outY, precision );

  // make a second transform so that it's fetched from the cache this time
  QgsCoordinateTransform ct2( sourceCrs, destCrs, QgsProject::instance() );
  x = srcX;
  y = srcY;
  ct2.transformInPlace( x, y, z, static_cast< Qgis::TransformDirection >( direction ) );
  QGSCOMPARENEAR( x, outX, precision );
  QGSCOMPARENEAR( y, outY, precision );
}

void TestQgsCoordinateTransform::dynamicToDynamicErrorHandler()
{
  // test that warnings are raised when attempting a dynamic crs to dynamic crs transformation (not supported by PROJ)
  int counter = 0;
  QgsCoordinateTransform::setDynamicCrsToDynamicCrsWarningHandler( [&counter]( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem & )
  {
    counter++;
  } );

  // no warnings -- no coordinate epoch set (although we should consider a different warning in this situation!!)
  QgsCoordinateReferenceSystem src( QStringLiteral( "EPSG:9000" ) );
  QgsCoordinateReferenceSystem dest( QStringLiteral( "EPSG:9000" ) );
  QgsCoordinateTransform t( src, dest, QgsCoordinateTransformContext() );
  QCOMPARE( counter, 0 );

  // no warnings, static to static
  src = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7844" ) );
  dest = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) );
  t = QgsCoordinateTransform( src, dest, QgsCoordinateTransformContext() );
  QCOMPARE( counter, 0 );

  // no warnings, static to dynamic
  src = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7844" ) );
  dest = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:9000" ) );
  dest.setCoordinateEpoch( 2030 );
  t = QgsCoordinateTransform( src, dest, QgsCoordinateTransformContext() );
  QCOMPARE( counter, 0 );

  // no warnings, dynamic to static
  src = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:9000" ) );
  src.setCoordinateEpoch( 2030 );
  dest = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7844" ) );
  t = QgsCoordinateTransform( src, dest, QgsCoordinateTransformContext() );
  QCOMPARE( counter, 0 );

  // no warnings, same dynamic CRS to same dynamic CRS with same epoch
  src = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:9000" ) );
  src.setCoordinateEpoch( 2030 );
  dest = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:9000" ) );
  dest.setCoordinateEpoch( 2030 );
  t = QgsCoordinateTransform( src, dest, QgsCoordinateTransformContext() );
  QCOMPARE( counter, 0 );

  // yes warnings, dynamic CRS to dynamic CRS with different epoch
  src = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:9000" ) );
  src.setCoordinateEpoch( 2030 );
  dest = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:9000" ) );
  dest.setCoordinateEpoch( 2025 );
  t = QgsCoordinateTransform( src, dest, QgsCoordinateTransformContext() );
  QCOMPARE( counter, 1 );

  QgsCoordinateTransform::setDynamicCrsToDynamicCrsWarningHandler( nullptr );
}
#endif

void TestQgsCoordinateTransform::transformBoundingBox()
{
  //test transforming a bounding box which crosses the 180 degree longitude line
  const QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  const QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4326" ) );

  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );
  const QgsRectangle crossingRect( 6374985, -3626584, 7021195, -3272435 );
  QgsRectangle resultRect = tr.transformBoundingBox( crossingRect, Qgis::TransformDirection::Forward, true );
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
  tr = QgsCoordinateTransform( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ), QgsProject::instance() );
  const QgsRectangle rect( -99999999999, 99999999999, -99999999998, 99999999998 );
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
  const QgsCoordinateReferenceSystem LKS92 = QgsCoordinateReferenceSystem::fromEpsgId( 3059 );
  QVERIFY( LKS92.isValid() );
  const QgsCoordinateReferenceSystem Baltic93 = QgsCoordinateReferenceSystem::fromEpsgId( 25884 );
  QVERIFY( Baltic93.isValid() );
  const QgsCoordinateReferenceSystem WGS84 = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( WGS84.isValid() );

  const QgsCoordinateTransform Lks2Balt( LKS92, Baltic93, QgsProject::instance() );
  QVERIFY( Lks2Balt.isValid() );
  const QgsCoordinateTransform Lks2Wgs( LKS92, WGS84, QgsProject::instance() );
  QVERIFY( Lks2Wgs.isValid() );

  QPolygonF sPoly = QgsGeometry::fromWkt( QStringLiteral( "Polygon (( 725865.850 198519.947, 363511.181 263208.769, 717694.697 333650.333, 725865.850 198519.947 ))" ) ).asQPolygonF();

  Lks2Balt.transformPolygon( sPoly, Qgis::TransformDirection::Forward );

  QGSCOMPARENEAR( sPoly.at( 0 ).x(), 725865.850, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 0 ).y(), 6198519.947, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 1 ).x(), 363511.181, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 1 ).y(), 6263208.769, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 2 ).x(), 717694.697, 0.001 );
  QGSCOMPARENEAR( sPoly.at( 2 ).y(), 6333650.333, 0.001 );
}

void TestQgsCoordinateTransform::transformContextNormalize()
{
  // coordinate operation for WGS84 to 27700
  const QString coordOperation = QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=WGS84 +step +inv +proj=helmert +x=446.448 +y=-125.157 +z=542.06 +rx=0.15 +ry=0.247 +rz=0.842 +s=-20.489 +convention=position_vector +step +inv +proj=cart +ellps=airy +step +proj=pop +v_3 +step +proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy" );
  QgsCoordinateTransformContext context;
  context.addCoordinateOperation( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsCoordinateReferenceSystem::fromEpsgId( 27700 ), coordOperation );

  const QgsCoordinateTransform ct( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsCoordinateReferenceSystem::fromEpsgId( 27700 ), context );
  QVERIFY( ct.isValid() );
  QgsPointXY p( -4.17477, 50.3657 );
  QgsPointXY p2 = ct.transform( p );
  QGSCOMPARENEAR( p2.x(), 245424.604645, 0.01 );
  QGSCOMPARENEAR( p2.y(), 54016.813093, 0.01 );

  p = ct.transform( p2, Qgis::TransformDirection::Reverse );
  QGSCOMPARENEAR( p.x(), -4.17477, 0.01 );
  QGSCOMPARENEAR( p.y(), 50.3657, 0.01 );

  const QgsCoordinateTransform ct2( QgsCoordinateReferenceSystem::fromEpsgId( 27700 ), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), context );
  QVERIFY( ct2.isValid() );
  p = QgsPointXY( 245424.604645, 54016.813093 );
  p2 = ct2.transform( p );
  QGSCOMPARENEAR( p2.x(), -4.17477, 0.01 );
  QGSCOMPARENEAR( p2.y(), 50.3657, 0.01 );

  p = ct2.transform( p2, Qgis::TransformDirection::Reverse );
  QGSCOMPARENEAR( p.x(), 245424.604645, 0.01 );
  QGSCOMPARENEAR( p.y(), 54016.813093, 0.01 );
}

void TestQgsCoordinateTransform::transform2DPoint()
{
  // Check that we properly handle 2D point transform
  const QgsCoordinateTransformContext context;
  const QgsCoordinateTransform ct( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsCoordinateReferenceSystem::fromEpsgId( 3857 ), context );
  QVERIFY( ct.isValid() );
  const QgsPoint pt( 0.0, 0.0 );
  double x = pt.x();
  double y = pt.y();
  double z = pt.z();
  ct.transformInPlace( x, y, z );

  QGSCOMPARENEAR( x, 0.0, 0.01 );
  QGSCOMPARENEAR( y, 0.0, 0.01 );
  QVERIFY( std::isnan( z ) );
}

void TestQgsCoordinateTransform::transformErrorMultiplePoints()
{
  // Check that we don't throw an exception when transforming multiple
  // points and at least one fails.
  const QgsCoordinateTransformContext context;
  const QgsCoordinateTransform ct( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsCoordinateReferenceSystem::fromEpsgId( 3857 ), context );
  QVERIFY( ct.isValid() );
  double x[] = { 0, -1000 };
  double y[] = { 0, 0 };
  double z[] = { 0, 0 };
  ct.transformCoords( 2, x, y, z );
  QGSCOMPARENEAR( x[0], 0, 0.01 );
  QGSCOMPARENEAR( y[0], 0, 0.01 );
  QVERIFY( !std::isfinite( x[1] ) );
  QVERIFY( !std::isfinite( y[1] ) );
}


void TestQgsCoordinateTransform::transformErrorOnePoint()
{
  const QgsCoordinateTransformContext context;
  const QgsCoordinateTransform ct( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsCoordinateReferenceSystem::fromEpsgId( 3857 ), context );
  QVERIFY( ct.isValid() );
  double x[] = {  -1000 };
  double y[] = {  0 };
  double z[] = {  0 };
  try
  {
    ct.transformCoords( 1, x, y, z );
    QVERIFY( false );
  }
  catch ( QgsCsException & )
  {
  }
}

void TestQgsCoordinateTransform::testDeprecated4240to4326()
{
  // test creating a coordinate transform between EPSG 4240 and EPSG 4326 using a deprecated coordinate operation
  // see https://github.com/qgis/QGIS/issues/33121

  QgsCoordinateTransformContext context;
  const QgsCoordinateReferenceSystem src( QStringLiteral( "EPSG:4240" ) );
  const QgsCoordinateReferenceSystem dest( QStringLiteral( "EPSG:4326" ) );

  // first use default transform
  const QgsCoordinateTransform defaultTransform( src, dest, context );
  QCOMPARE( defaultTransform.coordinateOperation(), QString() );
  QCOMPARE( defaultTransform.instantiatedCoordinateOperationDetails().proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=evrst30 +step +proj=helmert +x=293 +y=836 +z=318 +rx=0.5 +ry=1.6 +rz=-2.8 +s=2.1 +convention=position_vector +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg" ) );
  QVERIFY( defaultTransform.isValid() );

  const QgsPointXY p( 102.5, 7.5 );
  QgsPointXY p2 = defaultTransform.transform( p );
  QGSCOMPARENEAR( p2.x(), 102.494938, 0.000001 );
  QGSCOMPARENEAR( p2.y(), 7.502624, 0.000001 );

  QgsPointXY p3 = defaultTransform.transform( p2, Qgis::TransformDirection::Reverse );
  QGSCOMPARENEAR( p3.x(), 102.5, 0.000001 );
  QGSCOMPARENEAR( p3.y(), 7.5, 0.000001 );

  // and in reverse
  const QgsCoordinateTransform defaultTransformRev( dest, src, context );
  QVERIFY( defaultTransformRev.isValid() );
  QCOMPARE( defaultTransformRev.coordinateOperation(), QString() );
  QgsDebugMsg( defaultTransformRev.instantiatedCoordinateOperationDetails().proj );
  QCOMPARE( defaultTransformRev.instantiatedCoordinateOperationDetails().proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=WGS84 +step +inv +proj=helmert +x=293 +y=836 +z=318 +rx=0.5 +ry=1.6 +rz=-2.8 +s=2.1 +convention=position_vector +step +inv +proj=cart +ellps=evrst30 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg" ) );

  p2 = defaultTransformRev.transform( QgsPointXY( 102.494938, 7.502624 ) );
  QGSCOMPARENEAR( p2.x(), 102.5, 0.000001 );
  QGSCOMPARENEAR( p2.y(), 7.5, 0.000001 );

  p3 = defaultTransformRev.transform( p2, Qgis::TransformDirection::Reverse );
  QGSCOMPARENEAR( p3.x(), 102.494938, 0.000001 );
  QGSCOMPARENEAR( p3.y(), 7.502624, 0.000001 );

  // now force use of deprecated transform
  const QString deprecatedProj = QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=evrst30 +step +proj=helmert +x=209 +y=818 +z=290 +step +inv +proj=cart +ellps=WGS84 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg" );
  context.addCoordinateOperation( src, dest, deprecatedProj );

  const QgsCoordinateTransform deprecatedTransform( src, dest, context );
  QVERIFY( deprecatedTransform.isValid() );
  QCOMPARE( deprecatedTransform.coordinateOperation(), deprecatedProj );
  QCOMPARE( deprecatedTransform.instantiatedCoordinateOperationDetails().proj, deprecatedProj );

  p2 = deprecatedTransform.transform( p );
  QGSCOMPARENEAR( p2.x(), 102.496547, 0.000001 );
  QGSCOMPARENEAR( p2.y(), 7.502139, 0.000001 );

  p3 = deprecatedTransform.transform( p2, Qgis::TransformDirection::Reverse );
  QGSCOMPARENEAR( p3.x(), 102.5, 0.000001 );
  QGSCOMPARENEAR( p3.y(), 7.5, 0.000001 );

  // and in reverse
  const QgsCoordinateTransform deprecatedTransformRev( dest, src, context );
  QVERIFY( deprecatedTransformRev.isValid() );
  QCOMPARE( deprecatedTransformRev.coordinateOperation(), deprecatedProj );
  QCOMPARE( deprecatedTransformRev.instantiatedCoordinateOperationDetails().proj, deprecatedProj );

  p2 = deprecatedTransformRev.transform( QgsPointXY( 102.496547, 7.502139 ) );
  QGSCOMPARENEAR( p2.x(), 102.5, 0.000001 );
  QGSCOMPARENEAR( p2.y(), 7.5, 0.000001 );

  p3 = deprecatedTransformRev.transform( p2, Qgis::TransformDirection::Reverse );
  QGSCOMPARENEAR( p3.x(), 102.496547, 0.000001 );
  QGSCOMPARENEAR( p3.y(), 7.502139, 0.000001 );
}

void TestQgsCoordinateTransform::testCustomProjTransform()
{
  // test custom proj string
  // refs https://github.com/qgis/QGIS/issues/32928
  const QgsCoordinateReferenceSystem ss( QgsCoordinateReferenceSystem::fromProj( QStringLiteral( "+proj=longlat +ellps=GRS80 +towgs84=1,2,3,4,5,6,7 +no_defs" ) ) );
  const QgsCoordinateReferenceSystem dd( QStringLiteral( "EPSG:4326" ) );
  const QgsCoordinateTransform ct( ss, dd, QgsCoordinateTransformContext() );
  QVERIFY( ct.isValid() );
  QgsDebugMsg( ct.instantiatedCoordinateOperationDetails().proj );
  QCOMPARE( ct.instantiatedCoordinateOperationDetails().proj,
            QStringLiteral( "+proj=pipeline "
                            "+step +proj=unitconvert +xy_in=deg +xy_out=rad "
                            "+step +proj=push +v_3 "
                            "+step +proj=cart +ellps=GRS80 "
                            "+step +proj=helmert +x=1 +y=2 +z=3 +rx=4 +ry=5 +rz=6 +s=7 +convention=position_vector "
                            "+step +inv +proj=cart +ellps=WGS84 "
                            "+step +proj=pop +v_3 "
                            "+step +proj=unitconvert +xy_in=rad +xy_out=deg" ) );
}

void TestQgsCoordinateTransform::testTransformationIsPossible()
{
  QVERIFY( !QgsCoordinateTransform::isTransformationPossible( QgsCoordinateReferenceSystem(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) ) );
  QVERIFY( !QgsCoordinateTransform::isTransformationPossible( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), QgsCoordinateReferenceSystem() ) );
  QVERIFY( !QgsCoordinateTransform::isTransformationPossible( QgsCoordinateReferenceSystem(), QgsCoordinateReferenceSystem() ) );

  QVERIFY( QgsCoordinateTransform::isTransformationPossible( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
#if (PROJ_VERSION_MAJOR>8 || (PROJ_VERSION_MAJOR==8 && PROJ_VERSION_MINOR >= 1 ) )
  // crses from two different celestial bodies => transformation is not possible
  QVERIFY( !QgsCoordinateTransform::isTransformationPossible( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
           QgsCoordinateReferenceSystem( QStringLiteral( "ESRI:104903" ) ) ) );
#endif
}


QGSTEST_MAIN( TestQgsCoordinateTransform )
#include "testqgscoordinatetransform.moc"
