/***************************************************************************
     testqgssnappingutils.cpp
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
#include "qgssnappingutils.h"
#include "qgssnappingconfig.h"


struct FilterExcludePoint : public QgsPointLocator::MatchFilter
{
  explicit FilterExcludePoint( const QgsPointXY &p ) : mPoint( p ) {}

  bool acceptMatch( const QgsPointLocator::Match &match ) { return match.point() != mPoint; }

  QgsPointXY mPoint;
};


class TestQgsSnappingUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsSnappingUtils() = default;

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
      QgsGeometry polygonGeom = QgsGeometry::fromPolygonXY( polygon );
      ff.setGeometry( polygonGeom );
      QgsFeatureList flist;
      flist << ff;
      mVL->dataProvider()->addFeatures( flist );

      QgsProject::instance()->addMapLayer( mVL );
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testSnapModeCurrent()
    {
      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      u.setCurrentLayer( mVL );

      // first try with no snapping enabled
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( false );
      snappingConfig.setTolerance( 10 );
      snappingConfig.setUnits( QgsTolerance::Pixels );
      snappingConfig.setMode( QgsSnappingConfig::ActiveLayer );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m0 = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( !m0.isValid() );
      QVERIFY( !m0.hasVertex() );

      // now enable snapping
      snappingConfig.setEnabled( true );
      snappingConfig.setType( QgsSnappingConfig::Vertex );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.point(), QgsPointXY( 1, 0 ) );

      QgsPointLocator::Match m2 = u.snapToMap( QPoint( 0, 100 ) );
      QVERIFY( !m2.isValid() );
      QVERIFY( !m2.hasVertex() );

      // do not consider edges in the following test - on 32-bit platforms
      // result was an edge match very close to (1,0) instead of being exactly (1,0)

      snappingConfig.setType( QgsSnappingConfig::Vertex );
      u.setConfig( snappingConfig );

      // test with filtering
      FilterExcludePoint myFilter( QgsPointXY( 1, 0 ) );
      QgsPointLocator::Match m3 = u.snapToMap( QPoint( 100, 100 ), &myFilter );
      QVERIFY( !m3.isValid() );
    }

    void testSnapModeAll()
    {
      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      QgsSnappingConfig snappingConfig = u.config();
      u.setMapSettings( mapSettings );
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( QgsSnappingConfig::AllLayers );
      u.setConfig( snappingConfig );

      // right now there are no layers in map settings - snapping will fail

      QgsPointLocator::Match m = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( !m.isValid() );

      // now check with our layer
      mapSettings.setLayers( QList<QgsMapLayer *>() << mVL );
      u.setMapSettings( mapSettings );

      QgsPointLocator::Match m2 = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.point(), QgsPointXY( 1, 0 ) );
    }

    void testSnapModeAdvanced()
    {
      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      QgsSnappingConfig snappingConfig = u.config();
      u.setMapSettings( mapSettings );
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( QgsSnappingConfig::AdvancedConfiguration );
      snappingConfig.setIndividualLayerSettings( mVL, QgsSnappingConfig::IndividualLayerSettings( true, QgsSnappingConfig::Vertex, 10, QgsTolerance::Pixels ) );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.point(), QgsPointXY( 1, 0 ) );

      // test with filtering
      FilterExcludePoint myFilter( QgsPointXY( 1, 0 ) );
      QgsPointLocator::Match m2 = u.snapToMap( QPoint( 100, 100 ), &myFilter );
      QVERIFY( !m2.isValid() );
    }

    void testSnapOnIntersection()
    {
      // testing with a layer with two crossing linestrings
      // (0,1)  x  x (1,1)
      //         \/
      //         /\    .
      // (0,0)  x  x (1,0)
      QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      QgsPolylineXY polyline1, polyline2;
      polyline1 << QgsPointXY( 0, 0 ) << QgsPointXY( 1, 1 );
      polyline2 << QgsPointXY( 1, 0 ) << QgsPointXY( 0, 1 );
      QgsFeature f1;
      QgsGeometry f1g = QgsGeometry::fromPolylineXY( polyline1 ) ;
      f1.setGeometry( f1g );
      QgsFeature f2;
      QgsGeometry f2g = QgsGeometry::fromPolylineXY( polyline2 );
      f2.setGeometry( f2g );
      QgsFeatureList flist;
      flist << f1 << f2;
      vl->dataProvider()->addFeatures( flist );

      QVERIFY( vl->dataProvider()->featureCount() == 2 );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( QgsSnappingConfig::AdvancedConfiguration );
      QgsSnappingConfig::IndividualLayerSettings layerSettings( true, QgsSnappingConfig::Vertex, 0.1, QgsTolerance::ProjectUnits );
      snappingConfig.setIndividualLayerSettings( vl, layerSettings );
      u.setConfig( snappingConfig );

      // no snapping on intersections by default - should find nothing
      QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 0.45, 0.5 ) );
      QVERIFY( !m.isValid() );

      snappingConfig.setIntersectionSnapping( true );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( 0.45, 0.5 ) );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.type(), QgsPointLocator::Vertex );
      QCOMPARE( m2.point(), QgsPointXY( 0.5, 0.5 ) );

      delete vl;
    }
};

QGSTEST_MAIN( TestQgsSnappingUtils )

#include "testqgssnappingutils.moc"


