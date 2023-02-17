/***************************************************************************
     testqgsfeaturerequest.cpp
     -------------------------
    Date                 : May 2021
    Copyright            : (C) 2021 Sandro Santilli
    Email                : strk at kbt dot io
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
#include <QStringList>
#include <QSettings>

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgssymbol.h"
#include "qgssimplifymethod.h"
#include "qgslinesymbol.h"
#include "qgsexpressioncontextutils.h"

class TestQgsFeatureRequest: public QObject
{
    Q_OBJECT

  private:
    void testDefaultConstructed( const QgsFeatureRequest &f );

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void constructorTest(); //test default constructors
    void copyConstructorTest(); //test copy constructor
    void assignmentOperatorTest(); //test copy constructor


  private:

};

void TestQgsFeatureRequest::initTestCase()
{

}

void TestQgsFeatureRequest::cleanupTestCase()
{

}

void TestQgsFeatureRequest::init()
{

}

void TestQgsFeatureRequest::cleanup()
{

}

void TestQgsFeatureRequest::testDefaultConstructed( const QgsFeatureRequest &f )
{
  // Test public getter members
  QCOMPARE( f.filterType(), QgsFeatureRequest::FilterType::FilterNone );
  QCOMPARE( f.spatialFilterType(), Qgis::SpatialFilterType::NoFilter );
  QCOMPARE( f.filterRect(), QgsRectangle() );
  QVERIFY( f.referenceGeometry().isEmpty() );
  QCOMPARE( f.distanceWithin(), double() );
  // NOTE: this is not QgsFeatureId() because QgsFeatureId is just
  //       a typedef for an integer, so doesn't have QGIS-specific
  //       behavior encoded in a default constructor
  QCOMPARE( f.filterFid(), -1 ); // I think FID_NULL should be used
  QCOMPARE( f.filterFids(), QgsFeatureIds() );
  QCOMPARE( f.invalidGeometryCheck(), QgsFeatureRequest::InvalidGeometryCheck::GeometryNoCheck );
  QCOMPARE( f.invalidGeometryCallback(), nullptr );
  QCOMPARE( f.filterExpression(), nullptr );
  // Disabled because:
  //  1. expressionContext is non-const at time of writing this test
  //  2. QgsExpressionContext does not have an equality operator and
  //     this method always returns a valid pointer
  //QCOMPARE( *const_cast< QgsFeatureRequest & > (f).expressionContext(), QgsExpressionContext() );
  QCOMPARE( f.orderBy(), QgsFeatureRequest::OrderBy() );
  QCOMPARE( f.limit(), -1LL ); // I think 0 could be used to mean no limit
  QCOMPARE( f.flags(), QgsFeatureRequest::Flags() );
  QCOMPARE( f.subsetOfAttributes(), QgsAttributeList() );
  QCOMPARE( f.simplifyMethod(), QgsSimplifyMethod() );
  QCOMPARE( f.destinationCrs(), QgsCoordinateReferenceSystem() );
  QCOMPARE( f.transformContext(), QgsCoordinateTransformContext() );
  QCOMPARE( f.transformErrorCallback(), nullptr );
  QCOMPARE( f.timeout(), -1 ); // I think 0 could be used to mean no timeout
  QCOMPARE( f.requestMayBeNested(), false );
  QCOMPARE( f.feedback(), nullptr );
}

void TestQgsFeatureRequest::constructorTest()
{
  const QgsFeatureRequest f;
  testDefaultConstructed( f );
}

void TestQgsFeatureRequest::copyConstructorTest()
{
  const QgsFeatureRequest f;
  const QgsFeatureRequest f2( f );
  testDefaultConstructed( f2 );
}

void TestQgsFeatureRequest::assignmentOperatorTest()
{
  const QgsFeatureRequest f;
  QgsFeatureRequest f2;

  // modify all members of f2
  f2.setFilterRect( QgsRectangle( 10, 10, 20, 20 ) );
  f2.setDistanceWithin( QgsGeometry::fromWkt( "POINT(10 15)" ), 12 );
  f2.setFilterFid( 52 );
  f2.setFilterFids( {3, 4} );
  f2.setInvalidGeometryCheck( QgsFeatureRequest::InvalidGeometryCheck::GeometrySkipInvalid );
  f2.setInvalidGeometryCallback( []( const QgsFeature & ) {} );
  f2.setFilterExpression( "this not that" );
  QgsExpressionContextScope *scope = QgsExpressionContextUtils::globalScope();
  f2.setExpressionContext( QgsExpressionContext( { scope } ) );
  f2.addOrderBy( "someField" );
  f2.setLimit( 5 );
  f2.setFlags( QgsFeatureRequest::NoGeometry );
  f2.setSubsetOfAttributes( {1, 2} );
  QgsSimplifyMethod sm;
  sm.setMethodType( QgsSimplifyMethod::PreserveTopology );
  f2.setSimplifyMethod( sm );
  // TODO: modify more members of f2
  //f2.setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:3111") );

  f2 = f;

  testDefaultConstructed( f2 );
}


QGSTEST_MAIN( TestQgsFeatureRequest )
#include "testqgsfeaturerequest.moc"
