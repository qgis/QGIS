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
#include "qgscategorizedsymbolrenderer.h"
#include "qgssettings.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"

struct FilterExcludePoint : public QgsPointLocator::MatchFilter
{
  explicit FilterExcludePoint( const QgsPointXY &p ) : mPoint( p ) {}

  bool acceptMatch( const QgsPointLocator::Match &match ) override { return match.point() != mPoint; }

  QgsPointXY mPoint;
};


class TestQgsSnappingUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsSnappingUtils() = default;

  private:
    QgsVectorLayer *mVL = nullptr;
    QgsFeature f1, f2;
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
      mVL = new QgsVectorLayer( QStringLiteral( "Polygon?field=fld:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      int idx = mVL->fields().indexFromName( QStringLiteral( "fld" ) );
      QVERIFY( idx != -1 );
      f1.initAttributes( 1 );
      f2.initAttributes( 1 );

      QgsPolygonXY polygon;
      QgsPolylineXY polyline;
      polyline << QgsPointXY( 0, 1 ) << QgsPointXY( 1, 0 ) << QgsPointXY( 1, 1 ) << QgsPointXY( 0, 1 );
      polygon << polyline;
      QgsGeometry polygonGeom = QgsGeometry::fromPolygonXY( polygon );
      f1.setGeometry( polygonGeom );
      f1.setAttribute( idx, QVariant( 2 ) );
      QgsFeatureList flist;
      flist << f1;

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

    void testSnapInvisible()
    {
      QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
      renderer->setClassAttribute( QStringLiteral( "fld" ) );
      renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );
      renderer->addCategory( QgsRendererCategory( "2", QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ), QStringLiteral( "2" ) ) );
      mVL->setRenderer( renderer );

      //create legend with symbology nodes for categorized renderer
      QgsLayerTree *root = new QgsLayerTree();
      QgsLayerTreeLayer *n = new QgsLayerTreeLayer( mVL );
      root->addChildNode( n );
      QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
      m->refreshLayerLegend( n );

      //test that all nodes are initially checked
      QList<QgsLayerTreeModelLegendNode *> nodes = m->layerLegendNodes( n );
      QCOMPARE( nodes.length(), 1 );
      Q_FOREACH ( QgsLayerTreeModelLegendNode *ln, nodes )
      {
        QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Checked );
      }


      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      u.setEnableSnappingForInvisibleFeature( false );
      u.setCurrentLayer( mVL );

      // first try with no snapping enabled
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( false );
      snappingConfig.setTolerance( 10 );
      snappingConfig.setUnits( QgsTolerance::Pixels );
      snappingConfig.setMode( QgsSnappingConfig::ActiveLayer );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m0 = u.snapToMap( QPoint( 2, 2 ) );
      QVERIFY( !m0.isValid() );
      QVERIFY( !m0.hasVertex() );

      // now enable snapping
      snappingConfig.setEnabled( true );
      snappingConfig.setType( QgsSnappingConfig::Vertex );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m5 = u.snapToMap( QPoint( 2, 2 ) );
      QVERIFY( m5.isValid() );
      QVERIFY( m5.hasVertex() );
      QCOMPARE( m5.point(), QgsPointXY( 0, 1 ) );

      //uncheck all and test that all nodes are unchecked
      static_cast< QgsSymbolLegendNode * >( nodes.at( 0 ) )->uncheckAllItems();
      Q_FOREACH ( QgsLayerTreeModelLegendNode *ln, nodes )
      {
        QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Unchecked );
      }
      mVL->dataChanged(); /* refresh index */

      m5 = u.snapToMap( QPoint( 2, 2 ) );
      QVERIFY( !m5.isValid() );
      QVERIFY( !m5.hasVertex() );
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

    void testSnapOnIntersectionCurveZ()
    {
      // testing with a layer with curve and Z
      std::unique_ptr<QgsVectorLayer> vCurveZ( new QgsVectorLayer( QStringLiteral( "CircularStringZ" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      QgsGeometry f1g = QgsGeometry::fromWkt( "CircularStringZ (0 0 0, 5 5 5, 0 10 10)" ) ;
      f1.setGeometry( f1g );
      QgsFeature f2;
      QgsGeometry f2g = QgsGeometry::fromWkt( "CircularStringZ (8 0 20, 5 3 30, 8 10 40)" );
      f2.setGeometry( f2g );
      QgsFeature f3;
      QgsGeometry f3g = QgsGeometry::fromWkt( "CircularStringZ" );
      f3.setGeometry( f3g );
      QgsFeature f4;
      // TODO: Issues with Curves, should/must(?) have at least two points.
      QgsGeometry f4g = QgsGeometry::fromWkt( "CircularStringZ (1 2 3)" );
      f4.setGeometry( f4g );
      QgsFeatureList flist;
      flist << f1 << f2 << f3 << f4;
      vCurveZ->dataProvider()->addFeatures( flist );

      QVERIFY( vCurveZ->dataProvider()->featureCount() == 4 );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( QgsSnappingConfig::AdvancedConfiguration );
      QgsSnappingConfig::IndividualLayerSettings layerSettings( true, QgsSnappingConfig::Vertex, 0.2, QgsTolerance::ProjectUnits );
      snappingConfig.setIntersectionSnapping( true );
      snappingConfig.setIndividualLayerSettings( vCurveZ.get(), layerSettings );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( 4.7, 3.7 ) );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.type(), QgsPointLocator::Vertex );
      QGSCOMPARENEAR( m2.point().x(), 4.8, 0.001 );
      QGSCOMPARENEAR( m2.point().y(), 3.6, 0.001 );
    }
    void testSnapOnIntersectionMultiGeom()
    {
      std::unique_ptr<QgsVectorLayer> vMulti( new QgsVectorLayer( QStringLiteral( "MultiLineStringZ" ), QStringLiteral( "m" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      QgsGeometry f1g = QgsGeometry::fromWkt( "MultiLineStringZ ((0 0 0, 0 5 5), (5 0 10, 5 5 10))" );
      f1.setGeometry( f1g );
      QgsFeature f2;
      QgsGeometry f2g = QgsGeometry::fromWkt( "MultiLineStringZ ((-1 2.5 50, 10 2.5 55))" );
      f2.setGeometry( f2g );

      QgsFeatureList flist;
      flist << f1 << f2 ;
      vMulti->dataProvider()->addFeatures( flist );

      QVERIFY( vMulti->dataProvider()->featureCount() == 2 );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( QgsSnappingConfig::AdvancedConfiguration );
      QgsSnappingConfig::IndividualLayerSettings layerSettings( true, QgsSnappingConfig::Vertex, 0.2, QgsTolerance::ProjectUnits );
      snappingConfig.setIntersectionSnapping( true );
      snappingConfig.setIndividualLayerSettings( vMulti.get(), layerSettings );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 0, 2.6 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::Vertex );
      QCOMPARE( m.point(), QgsPointXY( 0.0, 2.5 ) );

      QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( 5, 2.6 ) );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.type(), QgsPointLocator::Vertex );
      QCOMPARE( m2.point(), QgsPointXY( 5.0, 2.5 ) );

    }

    void testSnapOnCurrentLayer()
    {
      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u( nullptr, true );
      u.setMapSettings( mapSettings );
      u.setCurrentLayer( mVL );

      QgsPointLocator::Match m = u.snapToCurrentLayer( QPoint( 100, 100 ), QgsPointLocator::Vertex );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.point(), QgsPointXY( 1, 0 ) );
    }

};

QGSTEST_MAIN( TestQgsSnappingUtils )

#include "testqgssnappingutils.moc"
