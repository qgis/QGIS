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
  explicit FilterExcludePoint( const QgsPoint& p ) : mPoint( p ) {}

  bool acceptMatch( const QgsPointLocator::Match& match ) { return match.point() != mPoint; }

  QgsPoint mPoint;
};

struct FilterExcludeEdge : public QgsPointLocator::MatchFilter
{
  FilterExcludeEdge( const QgsPoint& p1, const QgsPoint& p2 )
      : mP1( p1 )
      , mP2( p2 )
  {}

  bool acceptMatch( const QgsPointLocator::Match& match )
  {
    QgsPoint p1, p2;
    match.edgePoints( p1, p2 );
    return !( p1 == mP1 && p2 == mP2 ) && !( p1 == mP2 && p2 == mP1 );
  }

  QgsPoint mP1, mP2;
};


class TestQgsPointLocator : public QObject
{
    Q_OBJECT
  public:
    TestQgsPointLocator()
        : mVL( 0 )
    {}

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
      QgsPointLocator::Match m = loc.nearestVertex( pt, 999 );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m.point(), QgsPoint( 1, 1 ) );
      QCOMPARE( m.distance(), sqrt( 2.0 ) );
      QCOMPARE( m.vertexIndex(), 2 );
    }

    void testNearestEdge()
    {
      QgsPointLocator loc( mVL );
      QgsPoint pt( 1.1, 0.5 );
      QgsPointLocator::Match m = loc.nearestEdge( pt, 999 );
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

#if 0 // verticesInRect() not implemented
    void testVerticesInRect()
    {
      QgsPointLocator loc( mVL );
      QgsPointLocator::MatchList lst = loc.verticesInRect( QgsPoint( 1, 0 ), 2 );
      QCOMPARE( lst.count(), 4 );
      QCOMPARE( lst[0].point(), QgsPoint( 1, 0 ) );
      QCOMPARE( lst[0].distance(), 0. );
      QCOMPARE( lst[1].point(), QgsPoint( 1, 1 ) );
      QCOMPARE( lst[1].distance(), 1. );
      QCOMPARE( lst[2].point(), QgsPoint( 0, 1 ) );
      QCOMPARE( lst[2].distance(), sqrt( 2 ) );

      QgsPointLocator::MatchList lst2 = loc.verticesInRect( QgsPoint( 1, 0 ), 1 );
      QCOMPARE( lst2.count(), 2 );

      // test match filtering
      FilterExcludePoint myFilter( QgsPoint( 1, 0 ) );
      QgsPointLocator::MatchList lst3 = loc.verticesInRect( QgsPoint( 1, 0 ), 1, &myFilter );
      QCOMPARE( lst3.count(), 1 );
      QCOMPARE( lst3[0].point(), QgsPoint( 1, 1 ) );
    }
#endif

    void testEdgesInTolerance()
    {
      QgsPointLocator loc( mVL );
      QgsPointLocator::MatchList lst = loc.edgesInRect( QgsPoint( 0, 0 ), 2 );
      QCOMPARE( lst.count(), 3 );

      QgsPointLocator::MatchList lst2 = loc.edgesInRect( QgsPoint( 0, 0 ), 0.9 );
      QCOMPARE( lst2.count(), 1 );

      // test match filtering
      FilterExcludeEdge myFilter( QgsPoint( 1, 0 ), QgsPoint( 0, 1 ) );
      QgsPointLocator::MatchList lst3 = loc.edgesInRect( QgsPoint( 0, 0 ), 2, &myFilter );
      QCOMPARE( lst3.count(), 2 );
    }


    void testLayerUpdates()
    {

      QgsPointLocator loc( mVL );

      QgsPointLocator::Match mAddV0 = loc.nearestVertex( QgsPoint( 12, 12 ), 999 );
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
      QgsPointLocator::Match mAddV = loc.nearestVertex( QgsPoint( 12, 12 ), 999 );
      QVERIFY( mAddV.isValid() );
      QCOMPARE( mAddV.point(), QgsPoint( 11, 11 ) );
      QgsPointLocator::Match mAddE = loc.nearestEdge( QgsPoint( 11.1, 10.5 ), 999 );
      QVERIFY( mAddE.isValid() );
      QCOMPARE( mAddE.point(), QgsPoint( 11, 10.5 ) );
      QgsPointLocator::MatchList mAddA = loc.pointInPolygon( QgsPoint( 10.8, 10.8 ) );
      QVERIFY( mAddA.count() == 1 );

      // change geometry
      QgsGeometry* newGeom = new QgsGeometry( *ff.constGeometry() );
      newGeom->moveVertex( 10, 10, 2 ); // change 11,11 to 10,10
      mVL->changeGeometry( ff.id(), newGeom );
      delete newGeom;

      // verify it is changed in the point locator
      QgsPointLocator::Match mChV = loc.nearestVertex( QgsPoint( 12, 12 ), 999 );
      QVERIFY( mChV.isValid() );
      QVERIFY( mChV.point() != QgsPoint( 11, 11 ) ); // that point does not exist anymore
      mChV = loc.nearestVertex( QgsPoint( 9, 9 ), 999 );
      QVERIFY( mChV.isValid() );
      QVERIFY( mChV.point() == QgsPoint( 10, 10 ) ); // updated point

      // delete feature
      bool resD = mVL->deleteFeature( ff.id() );
      QVERIFY( resD );

      // verify it is deleted from the point locator
      QgsPointLocator::Match mDelV = loc.nearestVertex( QgsPoint( 12, 12 ), 999 );
      QVERIFY( mDelV.isValid() );
      QCOMPARE( mDelV.point(), QgsPoint( 1, 1 ) );

      mVL->rollBack();
    }

    void testExtent()
    {
      QgsRectangle bbox1( 10, 10, 11, 11 ); // out of layer's bounds
      QgsPointLocator loc1( mVL, 0, &bbox1 );

      QgsPointLocator::Match m1 = loc1.nearestVertex( QgsPoint( 2, 2 ), 999 );
      QVERIFY( !m1.isValid() );

      QgsRectangle bbox2( 0, 0, 1, 1 ); // in layer's bounds
      QgsPointLocator loc2( mVL, 0, &bbox2 );

      QgsPointLocator::Match m2 = loc2.nearestVertex( QgsPoint( 2, 2 ), 999 );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.point(), QgsPoint( 1, 1 ) );
    }
};

QTEST_MAIN( TestQgsPointLocator )

#include "testqgspointlocator.moc"
