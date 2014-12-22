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


struct FilterExcludePoint : public QgsPointLocator::MatchFilter
{
  FilterExcludePoint( const QgsPoint& p ) : mPoint( p ) {}

  bool acceptMatch( const QgsPointLocator::Match& match ) { return match.point() != mPoint; }

  QgsPoint mPoint;
};


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
      QgsPoint pt1, pt2;
      m.edgePoints( pt1, pt2 );
      QCOMPARE( pt1, QgsPoint( 1, 0 ) );
      QCOMPARE( pt2, QgsPoint( 1, 1 ) );
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

    void testVerticesInTolerance()
    {
      QgsPointLocator loc( mVL );
      QgsPointLocator::MatchList lst = loc.verticesInTolerance( QgsPoint( 1, 0 ), 2 );
      QCOMPARE( lst.count(), 4 );
      QCOMPARE( lst[0].point(), QgsPoint( 1, 0 ) );
      QCOMPARE( lst[0].distance(), 0. );
      QCOMPARE( lst[1].point(), QgsPoint( 1, 1 ) );
      QCOMPARE( lst[1].distance(), 1. );
      QCOMPARE( lst[2].point(), QgsPoint( 0, 1 ) );
      QCOMPARE( lst[2].distance(), sqrt( 2 ) );

      QgsPointLocator::MatchList lst2 = loc.verticesInTolerance( QgsPoint( 1, 0 ), 1 );
      QCOMPARE( lst2.count(), 2 );

      // test match filtering
      FilterExcludePoint myFilter( QgsPoint( 1, 0 ) );
      QgsPointLocator::MatchList lst3 = loc.verticesInTolerance( QgsPoint( 1, 0 ), 1, &myFilter );
      QCOMPARE( lst3.count(), 1 );
      QCOMPARE( lst3[0].point(), QgsPoint( 1, 1 ) );
    }

    void testEdgesInTolerance()
    {
      QgsPointLocator loc( mVL );
      QgsPointLocator::MatchList lst = loc.edgesInTolerance( QgsPoint( 0, 0 ), 2 );
      QCOMPARE( lst.count(), 3 );
      QCOMPARE( lst[0].point(), QgsPoint( 0.5, 0.5 ) );
      QCOMPARE( lst[0].distance(), sqrt( 2 ) / 2 );
      QVERIFY( lst[1].point() == QgsPoint( 0, 1 ) || lst[1].point() == QgsPoint( 1, 0 ) );
      QCOMPARE( lst[1].distance(), 1. );
      QVERIFY( lst[2].point() == QgsPoint( 0, 1 ) || lst[2].point() == QgsPoint( 1, 0 ) );
      QCOMPARE( lst[2].distance(), 1. );

      QgsPointLocator::MatchList lst2 = loc.edgesInTolerance( QgsPoint( 0, 0 ), 0.9 );
      QCOMPARE( lst2.count(), 1 );

      // test match filtering
      FilterExcludePoint myFilter( QgsPoint( 0.5, 0.5 ) );
      QgsPointLocator::MatchList lst3 = loc.edgesInTolerance( QgsPoint( 0, 0 ), 2, &myFilter );
      QCOMPARE( lst3.count(), 2 );
      QVERIFY( lst3[0].point() == QgsPoint( 0, 1 ) || lst3[0].point() == QgsPoint( 1, 0 ) );
    }


    void testLayerUpdates()
    {

      QgsPointLocator loc( mVL );

      QgsPointLocator::Match mAddV0 = loc.nearestVertex( QgsPoint( 12, 12 ) );
      QVERIFY( mAddV0.isValid() );
      QCOMPARE( mAddV0.point(), QgsPoint( 1, 1 ) );

      mVL->startEditing();

      // add a new feature
      QgsFeature ff( 0 );
      QgsPolygon polygon;
      QgsPolyline polyline;
      polyline << QgsPoint( 10, 11 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) << QgsPoint( 10, 11 );
      polygon << polyline;
      ff.setGeometry( QgsGeometry::fromPolygon( polygon ) );
      QgsFeatureList flist;
      flist << ff;
      bool resA = mVL->addFeature( ff );
      QVERIFY( resA );

      // verify it is added in the point locator
      QgsPointLocator::Match mAddV = loc.nearestVertex( QgsPoint( 12, 12 ) );
      QVERIFY( mAddV.isValid() );
      QCOMPARE( mAddV.point(), QgsPoint( 11, 11 ) );
      QgsPointLocator::Match mAddE = loc.nearestEdge( QgsPoint( 11.1, 10.5 ) );
      QVERIFY( mAddE.isValid() );
      QCOMPARE( mAddE.point(), QgsPoint( 11, 10.5 ) );
      QgsPointLocator::MatchList mAddA = loc.pointInPolygon( QgsPoint( 10.8, 10.8 ) );
      QVERIFY( mAddA.count() == 1 );

      // change geometry
      QgsGeometry* newGeom = new QgsGeometry( *ff.geometry() );
      newGeom->moveVertex( 10, 10, 2 ); // change 11,11 to 10,10
      mVL->changeGeometry( ff.id(), newGeom );
      delete newGeom;

      // verify it is changed in the point locator
      QgsPointLocator::Match mChV = loc.nearestVertex( QgsPoint( 12, 12 ) );
      QVERIFY( mChV.isValid() );
      QVERIFY( mChV.point() != QgsPoint( 11, 11 ) ); // that point does not exist anymore
      mChV = loc.nearestVertex( QgsPoint( 9, 9 ) );
      QVERIFY( mChV.isValid() );
      QVERIFY( mChV.point() == QgsPoint( 10, 10 ) ); // updated point

      // delete feature
      bool resD = mVL->deleteFeature( ff.id() );
      QVERIFY( resD );

      // verify it is deleted from the point locator
      QgsPointLocator::Match mDelV = loc.nearestVertex( QgsPoint( 12, 12 ) );
      QVERIFY( mDelV.isValid() );
      QCOMPARE( mDelV.point(), QgsPoint( 1, 1 ) );

      mVL->rollBack();
    }
};

QTEST_MAIN( TestQgsPointLocator )

#include "testqgspointlocator.moc"

