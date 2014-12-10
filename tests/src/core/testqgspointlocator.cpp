/***************************************************************************
     testqgspointlocator.cpp
     --------------------------------------
    Date                 : November 2014
    Copyright            : (C) 2014 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>
#include <QObject>
#include <QString>

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometry.h"
#include "qgsmaplayerregistry.h"
#include "qgspointlocator.h"


class TestQgsPointLocator : public QObject
{
    Q_OBJECT
  private:
    QgsVectorLayer* mVL;
  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      // Will make sure the settings dir with the style file for color ramp is created
      QgsApplication::createDB();
      QgsApplication::showSettings();

      // vector layer with a triangle:
      // (0,1) +---+ (1,1)
      //        \  |
      //         \ |
      //          \|
      //           + (1,0)
      mVL = new QgsVectorLayer( "Polygon", "x", "memory" );
      QgsFeature ff( 0 );
      QgsPolygon polygon;
      QgsPolyline polyline;
      polyline << QgsPoint( 0, 1 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 );
      polygon << polyline;
      ff.setGeometry( QgsGeometry::fromPolygon( polygon ) );
      QgsFeatureList flist;
      flist << ff;
      mVL->dataProvider()->addFeatures( flist );

      QgsMapLayerRegistry::instance()->addMapLayer( mVL );
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testNearestVertex()
    {
      QgsPointLocator loc( mVL );
      QgsPoint pt( 2, 2 );
      QgsPointLocator::Match m = loc.nearestVertex( pt );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m.point(), QgsPoint( 1, 1 ) );
      QCOMPARE( m.distance(), sqrt( 2 ) );
      QCOMPARE( m.vertexIndex(), 2 );
    }

    void testNearestEdge()
    {
      QgsPointLocator loc( mVL );
      QgsPoint pt( 1.1, 0.5 );
      QgsPointLocator::Match m = loc.nearestEdge( pt );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasEdge() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m.point(), QgsPoint( 1, 0.5 ) );
      QCOMPARE( m.distance(), 0.1 );
      QCOMPARE( m.vertexIndex(), 1 );
    }

    void testPointInPolygon()
    {
      QgsPointLocator loc( mVL );
      QgsPointLocator::MatchList mValid = loc.pointInPolygon( QgsPoint( 0.8, 0.8 ) );
      QCOMPARE( mValid.count(), 1 );
      QgsPointLocator::Match m = mValid[0];
      QVERIFY( m.isValid() );
      QVERIFY( m.hasArea() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );

      QgsPointLocator::MatchList mInvalid = loc.pointInPolygon( QgsPoint( 0, 0 ) );
      QCOMPARE( mInvalid.count(), 0 );
    }

    // TODO: intersection tests
};

QTEST_MAIN( TestQgsPointLocator )

#include "testqgspointlocator.moc"

