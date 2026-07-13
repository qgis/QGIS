/***************************************************************************
                         testqgsdistanceutils.cpp
                         ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <vector>

#include "qgscoordinatereferencesystem.h"
#include "qgsdistancearea.h"
#include "qgsdistanceutils.h"
#include "qgsprocessingfeedback.h"
#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

class TestQgsDistanceUtils : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsDistanceUtils()
      : QgsTest( u"Distance Utils Test"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testEmptyTargets();
    void testNeighboursSearch();
    void testEllipsoidalVsCartesian();
};

void TestQgsDistanceUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsDistanceUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsDistanceUtils::testEmptyTargets()
{
  QgsDistanceArea da;
  QgsPointXY source( 0, 0 );
  std::vector<std::pair<QgsFeatureId, QgsPointXY>> targets;

  std::vector<QgsDistanceUtils::NeighborResult> results = QgsDistanceUtils::nearestNeighbors( source, targets, da, 5 );
  QVERIFY( results.empty() );
}

void TestQgsDistanceUtils::testNeighboursSearch()
{
  QgsDistanceArea da;
  da.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ), QgsCoordinateTransformContext() );

  QgsPointXY source( 0, 0 );
  std::vector<std::pair<QgsFeatureId, QgsPointXY>> targets = { { 1, QgsPointXY( 0, 10 ) }, { 2, QgsPointXY( 0, 5 ) } };

  std::vector<QgsDistanceUtils::NeighborResult> results = QgsDistanceUtils::nearestNeighbors( source, targets, da, 0 );
  QCOMPARE( results.size(), static_cast<size_t>( 2 ) );
  QCOMPARE( results[0].featureId, 2LL );
  QCOMPARE( results[1].featureId, 1LL );

  results = QgsDistanceUtils::nearestNeighbors( source, targets, da, 1 );
  QCOMPARE( results.size(), static_cast<size_t>( 1 ) );
  QCOMPARE( results[0].featureId, 2LL );

  results = QgsDistanceUtils::nearestNeighbors( source, targets, da, 10 );
  QCOMPARE( results.size(), static_cast<size_t>( 2 ) );
  QCOMPARE( results[0].featureId, 2LL );
  QCOMPARE( results[1].featureId, 1LL );
}

void TestQgsDistanceUtils::testEllipsoidalVsCartesian()
{
  // Verify that ellipsoidal calculations return actual nearest neighbour when Cartesian
  // measurement gives different neighbor. See GH #45493.
  QgsDistanceArea da;
  da.setSourceCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), QgsCoordinateTransformContext() );

  QgsPointXY sourcePoint( 0.0, 60.0 );
  std::vector<std::pair<QgsFeatureId, QgsPointXY>> targets = {
    { 1, QgsPointXY( 1.0, 60.0 ) }, // Cartesian distance from source 1 deg, ellipsoid 55799.5
    { 2, QgsPointXY( 0.0, 60.6 ) }  // Cartesian distance from source 0.6 deg, ellipsoid 66850.4
  };

  std::vector<QgsDistanceUtils::NeighborResult> results = QgsDistanceUtils::nearestNeighbors( sourcePoint, targets, da, 1 );
  QCOMPARE( results.size(), static_cast<size_t>( 1 ) );
  QCOMPARE( results[0].featureId, 2LL );

  // ellipsoid calculations
  da.setEllipsoid( u"WGS84"_s );
  QVERIFY( da.willUseEllipsoid() );

  results = QgsDistanceUtils::nearestNeighbors( sourcePoint, targets, da, 1 );
  QCOMPARE( results.size(), static_cast<size_t>( 1 ) );
  QCOMPARE( results[0].featureId, 1LL );
}

QGSTEST_MAIN( TestQgsDistanceUtils )
#include "testqgsdistanceutils.moc"
