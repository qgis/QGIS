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

#include "qgstest.h"
#include <QObject>
#include <QString>

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgspointlocator.h"
#include "qgspolygon.h"


struct FilterExcludePoint : public QgsPointLocator::MatchFilter
{
  explicit FilterExcludePoint( const QgsPointXY &p ) : mPoint( p ) {}

  bool acceptMatch( const QgsPointLocator::Match &match ) override { return match.point() != mPoint; }

  QgsPointXY mPoint;
};

struct FilterExcludeEdge : public QgsPointLocator::MatchFilter
{
  FilterExcludeEdge( const QgsPointXY &p1, const QgsPointXY &p2 )
    : mP1( p1 )
    , mP2( p2 )
  {}

  bool acceptMatch( const QgsPointLocator::Match &match ) override
  {
    QgsPointXY p1, p2;
    match.edgePoints( p1, p2 );
    return !( p1 == mP1 && p2 == mP2 ) && !( p1 == mP2 && p2 == mP1 );
  }

  QgsPointXY mP1, mP2;
};


class TestQgsPointLocator : public QObject
{
    Q_OBJECT
  public:
    TestQgsPointLocator() = default;

  private:
    QgsVectorLayer *mVL = nullptr;

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      // Will make sure the settings dir with the style file for color ramp is created
      QgsApplication::createDatabase();
      QgsApplication::showSettings();

      // vector layer with a triangle:
      // (0,1) +---+ (1,1)
      //        \  |
      //         \ |
      //          \|
      //           + (1,0)
      mVL = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      QgsFeature ff( 0 );
      QgsPolygonXY polygon;
      QgsPolylineXY polyline;
      polyline << QgsPointXY( 0, 1 ) << QgsPointXY( 1, 0 ) << QgsPointXY( 1, 1 ) << QgsPointXY( 0, 1 );
      polygon << polyline;
      const QgsGeometry ffGeom = QgsGeometry::fromPolygonXY( polygon );
      ff.setGeometry( ffGeom );
      QgsFeatureList flist;
      flist << ff;
      mVL->dataProvider()->addFeatures( flist );

      QgsProject::instance()->addMapLayer( mVL );
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testNearestVertex()
    {
      QgsPointLocator loc( mVL );
      const QgsPointXY pt( 2, 2 );
      const QgsPointLocator::Match m = loc.nearestVertex( pt, 999 );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m.point(), QgsPointXY( 1, 1 ) );
      QCOMPARE( m.distance(), std::sqrt( 2.0 ) );
      QCOMPARE( m.vertexIndex(), 2 );
    }

    void testNearestEdge()
    {
      QgsPointLocator loc( mVL );
      const QgsPointXY pt( 1.1, 0.5 );
      const QgsPointLocator::Match m = loc.nearestEdge( pt, 999 );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasEdge() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m.point(), QgsPointXY( 1, 0.5 ) );
      QCOMPARE( m.distance(), 0.1 );
      QCOMPARE( m.vertexIndex(), 1 );
      QgsPointXY pt1, pt2;
      m.edgePoints( pt1, pt2 );
      QCOMPARE( pt1, QgsPointXY( 1, 0 ) );
      QCOMPARE( pt2, QgsPointXY( 1, 1 ) );
    }

    void testNearestArea()
    {
      QgsPointLocator loc( mVL );
      const QgsPointXY pt1( 1.1, 0.5 );
      const QgsPointLocator::Match m1 = loc.nearestArea( pt1, 0 );
      QVERIFY( !m1.isValid() );

      const QgsPointXY pt2( 0.9, 0.9 );
      const QgsPointLocator::Match m2 = loc.nearestArea( pt2, 0 );
      QVERIFY( m2.isValid() );
      QVERIFY( m2.hasArea() );
      QCOMPARE( m2.layer(), mVL );
      QCOMPARE( m2.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m2.point(), QgsPointXY( 0.9, 0.9 ) );
      QCOMPARE( m2.distance(), 0.0 );

      const QgsPointXY pt3( 1.1, 1.1 );
      const QgsPointLocator::Match m3 = loc.nearestArea( pt3, 999 );
      QVERIFY( m3.isValid() );
      QVERIFY( m3.hasArea() );
      QCOMPARE( m3.layer(), mVL );
      QCOMPARE( m3.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m3.point(), QgsPointXY( 1.0, 1.0 ) );
      QCOMPARE( m3.distance(), .1 * std::sqrt( 2 ) );
    }

    void testPointInPolygon()
    {
      QgsPointLocator loc( mVL );
      QgsPointLocator::MatchList mValid = loc.pointInPolygon( QgsPointXY( 0.8, 0.8 ) );
      QCOMPARE( mValid.count(), 1 );
      const QgsPointLocator::Match m = mValid[0];
      QVERIFY( m.isValid() );
      QVERIFY( m.hasArea() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );

      const QgsPointLocator::MatchList mInvalid = loc.pointInPolygon( QgsPointXY( 0, 0 ) );
      QCOMPARE( mInvalid.count(), 0 );
    }

#if 0 // verticesInRect() not implemented
    void testVerticesInRect()
    {
      QgsPointLocator loc( mVL );
      QgsPointLocator::MatchList lst = loc.verticesInRect( QgsPointXY( 1, 0 ), 2 );
      QCOMPARE( lst.count(), 4 );
      QCOMPARE( lst[0].point(), QgsPointXY( 1, 0 ) );
      QCOMPARE( lst[0].distance(), 0. );
      QCOMPARE( lst[1].point(), QgsPointXY( 1, 1 ) );
      QCOMPARE( lst[1].distance(), 1. );
      QCOMPARE( lst[2].point(), QgsPointXY( 0, 1 ) );
      QCOMPARE( lst[2].distance(), std::sqrt( 2 ) );

      QgsPointLocator::MatchList lst2 = loc.verticesInRect( QgsPointXY( 1, 0 ), 1 );
      QCOMPARE( lst2.count(), 2 );

      // test match filtering
      FilterExcludePoint myFilter( QgsPointXY( 1, 0 ) );
      QgsPointLocator::MatchList lst3 = loc.verticesInRect( QgsPointXY( 1, 0 ), 1, &myFilter );
      QCOMPARE( lst3.count(), 1 );
      QCOMPARE( lst3[0].point(), QgsPointXY( 1, 1 ) );
    }
#endif

    void testEdgesInTolerance()
    {
      QgsPointLocator loc( mVL );
      const QgsPointLocator::MatchList lst = loc.edgesInRect( QgsPointXY( 0, 0 ), 2 );
      QCOMPARE( lst.count(), 3 );

      const QgsPointLocator::MatchList lst2 = loc.edgesInRect( QgsPointXY( 0, 0 ), 0.9 );
      QCOMPARE( lst2.count(), 1 );

      // test match filtering
      FilterExcludeEdge myFilter( QgsPointXY( 1, 0 ), QgsPointXY( 0, 1 ) );
      const QgsPointLocator::MatchList lst3 = loc.edgesInRect( QgsPointXY( 0, 0 ), 2, &myFilter );
      QCOMPARE( lst3.count(), 2 );
    }

    void testVerticesInTolerance()
    {
      QgsPointLocator loc( mVL );
      const QgsPointLocator::MatchList lst = loc.verticesInRect( QgsPointXY( 0, 2 ), 0.5 );
      QCOMPARE( lst.count(), 0 );

      QgsPointLocator::MatchList lst2 = loc.verticesInRect( QgsPointXY( 0, 1.5 ), 0.5 );
      QCOMPARE( lst2.count(), 2 );  // one matching point, but it is the first point in ring, so it is duplicated
      QCOMPARE( lst2[0].vertexIndex(), 0 );
      QCOMPARE( lst2[1].vertexIndex(), 3 );

      QgsPointLocator::MatchList lst3 = loc.verticesInRect( QgsPointXY( 0, 1.5 ), 1 );
      QCOMPARE( lst3.count(), 3 );
      QCOMPARE( lst3[0].vertexIndex(), 0 );
      QCOMPARE( lst3[1].vertexIndex(), 2 );
      QCOMPARE( lst3[2].vertexIndex(), 3 );
    }

    void testLayerUpdates()
    {

      QgsPointLocator loc( mVL );

      const QgsPointLocator::Match mAddV0 = loc.nearestVertex( QgsPointXY( 12, 12 ), 999 );
      QVERIFY( mAddV0.isValid() );
      QCOMPARE( mAddV0.point(), QgsPointXY( 1, 1 ) );

      mVL->startEditing();

      // add a new feature
      QgsFeature ff( 0 );
      QgsPolygonXY polygon;
      QgsPolylineXY polyline;
      polyline << QgsPointXY( 10, 11 ) << QgsPointXY( 11, 10 ) << QgsPointXY( 11, 11 ) << QgsPointXY( 10, 11 );
      polygon << polyline;
      const QgsGeometry ffGeom = QgsGeometry::fromPolygonXY( polygon ) ;
      ff.setGeometry( ffGeom );
      QgsFeatureList flist;
      flist << ff;
      const bool resA = mVL->addFeature( ff );
      QVERIFY( resA );

      // verify it is added in the point locator
      const QgsPointLocator::Match mAddV = loc.nearestVertex( QgsPointXY( 12, 12 ), 999 );
      QVERIFY( mAddV.isValid() );
      QCOMPARE( mAddV.point(), QgsPointXY( 11, 11 ) );
      const QgsPointLocator::Match mAddE = loc.nearestEdge( QgsPointXY( 11.1, 10.5 ), 999 );
      QVERIFY( mAddE.isValid() );
      QCOMPARE( mAddE.point(), QgsPointXY( 11, 10.5 ) );
      const QgsPointLocator::MatchList mAddA = loc.pointInPolygon( QgsPointXY( 10.8, 10.8 ) );
      QVERIFY( mAddA.count() == 1 );

      // change geometry
      QgsGeometry *newGeom = new QgsGeometry( ff.geometry() );
      newGeom->moveVertex( 10, 10, 2 ); // change 11,11 to 10,10
      mVL->changeGeometry( ff.id(), *newGeom );
      delete newGeom;

      // verify it is changed in the point locator
      QgsPointLocator::Match mChV = loc.nearestVertex( QgsPointXY( 12, 12 ), 999 );
      QVERIFY( mChV.isValid() );
      QVERIFY( mChV.point() != QgsPointXY( 11, 11 ) ); // that point does not exist anymore
      mChV = loc.nearestVertex( QgsPointXY( 9, 9 ), 999 );
      QVERIFY( mChV.isValid() );
      QVERIFY( mChV.point() == QgsPointXY( 10, 10 ) ); // updated point

      // delete feature
      const bool resD = mVL->deleteFeature( ff.id() );
      QVERIFY( resD );

      // verify it is deleted from the point locator
      const QgsPointLocator::Match mDelV = loc.nearestVertex( QgsPointXY( 12, 12 ), 999 );
      QVERIFY( mDelV.isValid() );
      QCOMPARE( mDelV.point(), QgsPointXY( 1, 1 ) );

      mVL->rollBack();
    }

    void testExtent()
    {
      const QgsRectangle bbox1( 10, 10, 11, 11 ); // out of layer's bounds
      QgsPointLocator loc1( mVL, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), &bbox1 );

      const QgsPointLocator::Match m1 = loc1.nearestVertex( QgsPointXY( 2, 2 ), 999 );
      QVERIFY( !m1.isValid() );

      const QgsRectangle bbox2( 0, 0, 1, 1 ); // in layer's bounds
      QgsPointLocator loc2( mVL, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), &bbox2 );

      const QgsPointLocator::Match m2 = loc2.nearestVertex( QgsPointXY( 2, 2 ), 999 );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.point(), QgsPointXY( 1, 1 ) );
    }

    void testNullGeometries()
    {
      QgsVectorLayer *vlNullGeom = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      QgsFeature ff( 0 );
      ff.setGeometry( QgsGeometry() );
      QgsFeatureList flist;
      flist << ff;
      vlNullGeom->dataProvider()->addFeatures( flist );

      QgsPointLocator loc( vlNullGeom, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), nullptr );

      const QgsPointLocator::Match m1 = loc.nearestVertex( QgsPointXY( 2, 2 ), std::numeric_limits<double>::max() );
      QVERIFY( !m1.isValid() );

      const QgsPointLocator::Match m2 = loc.nearestEdge( QgsPointXY( 2, 2 ), std::numeric_limits<double>::max() );
      QVERIFY( !m2.isValid() );

      delete vlNullGeom;
    }

    void testEmptyGeometries()
    {
      QgsVectorLayer *vlEmptyGeom = new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      QgsFeature ff( 0 );
      QgsGeometry g;
      g.set( new QgsPolygon() );
      ff.setGeometry( g );
      QgsFeatureList flist;
      flist << ff;
      vlEmptyGeom->dataProvider()->addFeatures( flist );

      QgsPointLocator loc( vlEmptyGeom, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), nullptr );

      const QgsPointLocator::Match m1 = loc.nearestVertex( QgsPointXY( 2, 2 ), std::numeric_limits<double>::max() );
      QVERIFY( !m1.isValid() );

      const QgsPointLocator::Match m2 = loc.nearestEdge( QgsPointXY( 2, 2 ), std::numeric_limits<double>::max() );
      QVERIFY( !m2.isValid() );

      delete vlEmptyGeom;
    }

    void testAsynchronousMode()
    {
      QgsPointLocator loc( mVL, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), nullptr );
      const QgsPointXY pt( 2, 2 );

      QEventLoop loop;
      connect( &loc, &QgsPointLocator::initFinished, &loop, &QEventLoop::quit );

      // locator is not ready yet
      QgsPointLocator::Match m = loc.nearestVertex( pt, 999, nullptr, true );
      QVERIFY( !m.isValid() );
      QVERIFY( loc.mIsIndexing );

      // we block until initFinished is called from another thread
      loop.exec();

      QVERIFY( !loc.mIsIndexing );

      // now locator is ready
      m = loc.nearestVertex( pt, 999 );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m.point(), QgsPointXY( 1, 1 ) );
      QCOMPARE( m.distance(), std::sqrt( 2.0 ) );
      QCOMPARE( m.vertexIndex(), 2 );
    }

    void testLayerUpdatesAsynchronous()
    {

      QgsPointLocator loc( mVL );

      QEventLoop loop;
      connect( &loc, &QgsPointLocator::initFinished, &loop, &QEventLoop::quit );

      // trigger locator initialization
      const QgsPointLocator::Match mAddV0 = loc.nearestVertex( QgsPointXY( 12, 12 ), 999, nullptr, true );

      // locator is not ready yet
      QVERIFY( !mAddV0.isValid() );
      QVERIFY( loc.mIsIndexing );

      mVL->startEditing();

      // add a new feature
      QgsFeature ff( 0 );
      QgsPolygonXY polygon;
      QgsPolylineXY polyline;
      polyline << QgsPointXY( 10, 11 ) << QgsPointXY( 11, 10 ) << QgsPointXY( 11, 11 ) << QgsPointXY( 10, 11 );
      polygon << polyline;
      const QgsGeometry ffGeom = QgsGeometry::fromPolygonXY( polygon ) ;
      ff.setGeometry( ffGeom );
      QgsFeatureList flist;
      flist << ff;
      const bool resA = mVL->addFeature( ff );
      QVERIFY( resA );

      // indexing is still running, change geometry
      QgsGeometry *newGeom = new QgsGeometry( ff.geometry() );
      newGeom->moveVertex( 10, 10, 2 ); // change 11,11 to 10,10
      mVL->changeGeometry( ff.id(), *newGeom );
      delete newGeom;

      // we block until initFinished is called from another thread
      loop.exec();

      QVERIFY( !loc.mIsIndexing );

      // verify it is changed in the point locator
      QgsPointLocator::Match mChV = loc.nearestVertex( QgsPointXY( 12, 12 ), 999 );
      QVERIFY( mChV.isValid() );
      QVERIFY( mChV.point() != QgsPointXY( 11, 11 ) ); // that point does not exist anymore
      mChV = loc.nearestVertex( QgsPointXY( 9, 9 ), 999 );
      QVERIFY( mChV.isValid() );
      QVERIFY( mChV.point() == QgsPointXY( 10, 10 ) ); // updated point

      // delete feature while no indexing is running
      const bool resD = mVL->deleteFeature( ff.id() );
      QVERIFY( resD );

      // verify it is deleted from the point locator
      const QgsPointLocator::Match mDelV = loc.nearestVertex( QgsPointXY( 12, 12 ), 999 );
      QVERIFY( mDelV.isValid() );
      QCOMPARE( mDelV.point(), QgsPointXY( 1, 1 ) );

      mVL->rollBack();
    }

    void testWaitForIndexingFinished()
    {
      QgsPointLocator loc( mVL, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), nullptr );
      const QgsPointXY pt( 2, 2 );

      // locator is not ready yet
      QgsPointLocator::Match m = loc.nearestVertex( pt, 999, nullptr, true );
      QVERIFY( !m.isValid() );
      QVERIFY( loc.mIsIndexing );

      // non relaxed call, this will block until the first indexing is finished
      // so the match point has to be valid
      m = loc.nearestVertex( pt, 999, nullptr );
      QVERIFY( m.isValid() );
      QVERIFY( !loc.mIsIndexing );

      // now locator is ready
      m = loc.nearestVertex( pt, 999 );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.layer(), mVL );
      QCOMPARE( m.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m.point(), QgsPointXY( 1, 1 ) );
      QCOMPARE( m.distance(), std::sqrt( 2.0 ) );
      QCOMPARE( m.vertexIndex(), 2 );
    }

    void testDeleteLocator()
    {
      QgsPointLocator *loc = new QgsPointLocator( mVL, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), nullptr );
      const QgsPointXY pt( 2, 2 );

      // delete locator while we are indexing (could happen when closing project for instance)
      loc->nearestVertex( pt, 999, nullptr, true );
      delete loc;
    }

    void testEmptyLayer()
    {
      // Issue https://github.com/qgis/QGIS/issues/33449

      // Create an empty layer, add one feature and check that we can snap on this feature
      QgsVectorLayer layer( QStringLiteral( "Polygon" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      QgsProject::instance()->addMapLayer( &layer );

      QgsPointLocator loc( &layer );

      // init locator (no rtree in locator because there is no feature in layer)
      const QgsPointXY pt( 2, 2 );
      QgsPointLocator::Match m = loc.nearestVertex( pt, 999 );
      QVERIFY( loc.mIsEmptyLayer );
      QVERIFY( !loc.mRTree );

      layer.startEditing();
      QgsFeature ff( 0 );
      QgsPolygonXY polygon;
      QgsPolylineXY polyline;
      polyline << QgsPointXY( 0, 1 ) << QgsPointXY( 1, 0 ) << QgsPointXY( 1, 1 ) << QgsPointXY( 0, 1 );
      polygon << polyline;
      const QgsGeometry ffGeom = QgsGeometry::fromPolygonXY( polygon );
      ff.setGeometry( ffGeom );

      layer.addFeature( ff );

      QVERIFY( !loc.mIsEmptyLayer );
      QVERIFY( loc.mRTree );

      // Check is inserted feature is well known from the locator (even in relaxed mode,
      // no need to index and wait for finished)
      m = loc.nearestVertex( pt, 999, nullptr, true );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.layer(), &layer );
      QCOMPARE( m.point(), QgsPointXY( 1, 1 ) );
      QCOMPARE( m.distance(), std::sqrt( 2.0 ) );
      QCOMPARE( m.vertexIndex(), 2 );
    }

    void testInterpolatedPoint()
    {
      std::unique_ptr<QgsVectorLayer> curveLayer( new QgsVectorLayer( QStringLiteral( "CircularStringZ" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      const QgsGeometry f1g = QgsGeometry::fromWkt( "CircularStringZ (0 0 0, 5 5 5, 0 10 10)" ) ;
      f1.setGeometry( f1g );
      QgsFeatureList f1list;
      f1list << f1;
      curveLayer->dataProvider()->addFeatures( f1list );
      QVERIFY( curveLayer->dataProvider()->featureCount() == 1 );

      QgsPointLocator loc1( curveLayer.get(), QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), nullptr );
      const QgsPointLocator::Match m1 = loc1.nearestEdge( QgsPointXY( 3, 8 ), std::numeric_limits<double>::max() );
      QVERIFY( m1.isValid() );
      QVERIFY( m1.hasEdge() );
      QCOMPARE( m1.layer(), curveLayer.get() );
      QCOMPARE( m1.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m1.point(), QgsPointXY( 3.53553390593273775, 8.53553390593273775 ) );
      QCOMPARE( m1.distance(), 0.757359312881 );
      QCOMPARE( m1.vertexIndex(), 1 );
      QgsPointXY pt1, pt2;
      m1.edgePoints( pt1, pt2 );
      QCOMPARE( pt1, QgsPointXY( 5, 5 ) );
      QCOMPARE( pt2, QgsPointXY( 0, 10 ) );
      QCOMPARE( m1.interpolatedPoint( QgsCoordinateReferenceSystem() ), QgsPoint( 3.53553390593273775, 8.53553390593273775, 7.70598050073098584 ) );

      std::unique_ptr<QgsVectorLayer> lineLayer( new QgsVectorLayer( QStringLiteral( "LineStringZ" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) ) );
      QgsFeature f2;
      const QgsGeometry f2g = QgsGeometry::fromWkt( "LineStringZ (0 0 0, 5 5 5, 0 10 10)" ) ;
      f2.setGeometry( f2g );
      QgsFeatureList f2list;
      f2list << f2;
      lineLayer->dataProvider()->addFeatures( f2list );
      QVERIFY( lineLayer->dataProvider()->featureCount() == 1 );

      QgsPointLocator loc2( lineLayer.get(), QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), nullptr );
      const QgsPointLocator::Match m2 = loc2.nearestEdge( QgsPointXY( 3, 8 ), std::numeric_limits<double>::max() );
      QVERIFY( m2.isValid() );
      QVERIFY( m2.hasEdge() );
      QCOMPARE( m2.layer(), lineLayer.get() );
      QCOMPARE( m2.featureId(), ( QgsFeatureId )1 );
      QCOMPARE( m2.point(), QgsPointXY( 2.5, 7.5 ) );
      QCOMPARE( m2.distance(), 0.707106781187 );
      QCOMPARE( m2.vertexIndex(), 1 );
      m2.edgePoints( pt1, pt2 );
      QCOMPARE( pt1, QgsPointXY( 5, 5 ) );
      QCOMPARE( pt2, QgsPointXY( 0, 10 ) );
      QCOMPARE( m2.interpolatedPoint( QgsCoordinateReferenceSystem() ), QgsPoint( 2.5, 7.5, 7.5 ) );
    }
};

QGSTEST_MAIN( TestQgsPointLocator )

#include "testqgspointlocator.moc"
