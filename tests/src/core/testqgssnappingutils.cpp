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
#include "qgssymbol.h"

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
      const int idx = mVL->fields().indexFromName( QStringLiteral( "fld" ) );
      QVERIFY( idx != -1 );
      f1.initAttributes( 1 );
      f2.initAttributes( 1 );

      QgsPolygonXY polygon;
      QgsPolylineXY polyline;
      polyline << QgsPointXY( 0, 1 ) << QgsPointXY( 1, 0 ) << QgsPointXY( 1, 1 ) << QgsPointXY( 0, 1 );
      polygon << polyline;
      const QgsGeometry polygonGeom = QgsGeometry::fromPolygonXY( polygon );
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
      snappingConfig.setMode( Qgis::SnappingMode::ActiveLayer );
      u.setConfig( snappingConfig );

      const QgsPointLocator::Match m0 = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( !m0.isValid() );
      QVERIFY( !m0.hasVertex() );

      // now enable snapping
      snappingConfig.setEnabled( true );
      snappingConfig.setTypeFlag( Qgis::SnappingType::Vertex );
      u.setConfig( snappingConfig );

      const QgsPointLocator::Match m = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.point(), QgsPointXY( 1, 0 ) );

      const QgsPointLocator::Match m2 = u.snapToMap( QPoint( 0, 100 ) );
      QVERIFY( !m2.isValid() );
      QVERIFY( !m2.hasVertex() );

      // do not consider edges in the following test - on 32-bit platforms
      // result was an edge match very close to (1,0) instead of being exactly (1,0)

      snappingConfig.setTypeFlag( Qgis::SnappingType::Vertex );
      u.setConfig( snappingConfig );

      // test with filtering
      FilterExcludePoint myFilter( QgsPointXY( 1, 0 ) );
      const QgsPointLocator::Match m3 = u.snapToMap( QPoint( 100, 100 ), &myFilter );
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
      const QList<QgsLayerTreeModelLegendNode *> nodes = m->layerLegendNodes( n );
      QCOMPARE( nodes.length(), 1 );
      for ( QgsLayerTreeModelLegendNode *ln : nodes )
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
      snappingConfig.setMode( Qgis::SnappingMode::ActiveLayer );
      u.setConfig( snappingConfig );

      const QgsPointLocator::Match m0 = u.snapToMap( QPoint( 2, 2 ) );
      QVERIFY( !m0.isValid() );
      QVERIFY( !m0.hasVertex() );

      // now enable snapping
      snappingConfig.setEnabled( true );
      snappingConfig.setTypeFlag( Qgis::SnappingType::Vertex );
      u.setConfig( snappingConfig );

      QgsPointLocator::Match m5 = u.snapToMap( QPoint( 2, 2 ) );
      QVERIFY( m5.isValid() );
      QVERIFY( m5.hasVertex() );
      QCOMPARE( m5.point(), QgsPointXY( 0, 1 ) );

      //uncheck all and test that all nodes are unchecked
      static_cast< QgsSymbolLegendNode * >( nodes.at( 0 ) )->uncheckAllItems();
      for ( QgsLayerTreeModelLegendNode *ln : nodes )
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
      snappingConfig.setTypeFlag( Qgis::SnappingType::Vertex );
      snappingConfig.setMode( Qgis::SnappingMode::AllLayers );
      u.setConfig( snappingConfig );

      // right now there are no layers in map settings - snapping will fail

      const QgsPointLocator::Match m = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( !m.isValid() );

      // now check with our layer
      mapSettings.setLayers( QList<QgsMapLayer *>() << mVL );
      u.setMapSettings( mapSettings );

      const QgsPointLocator::Match m2 = u.snapToMap( QPoint( 100, 100 ) );
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
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      snappingConfig.setIndividualLayerSettings( mVL, QgsSnappingConfig::IndividualLayerSettings( true, Qgis::SnappingType::Vertex, 10, QgsTolerance::Pixels, -1.0, -1.0 ) );
      u.setConfig( snappingConfig );

      const QgsPointLocator::Match m = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.point(), QgsPointXY( 1, 0 ) );

      // test with filtering
      FilterExcludePoint myFilter( QgsPointXY( 1, 0 ) );
      const QgsPointLocator::Match m2 = u.snapToMap( QPoint( 100, 100 ), &myFilter );
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
      const QgsGeometry f1g = QgsGeometry::fromPolylineXY( polyline1 ) ;
      f1.setGeometry( f1g );
      QgsFeature f2;
      const QgsGeometry f2g = QgsGeometry::fromPolylineXY( polyline2 );
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
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      const QgsSnappingConfig::IndividualLayerSettings layerSettings( true, Qgis::SnappingType::Vertex, 0.1, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      snappingConfig.setIndividualLayerSettings( vl, layerSettings );
      u.setConfig( snappingConfig );

      // no snapping on intersections by default - should find nothing
      const QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 0.45, 0.5 ) );
      QVERIFY( !m.isValid() );

      snappingConfig.setIntersectionSnapping( true );
      u.setConfig( snappingConfig );

      const QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( 0.45, 0.5 ) );
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
      const QgsGeometry f1g = QgsGeometry::fromWkt( "CircularStringZ (0 0 0, 5 5 5, 0 10 10)" ) ;
      f1.setGeometry( f1g );
      QgsFeature f2;
      const QgsGeometry f2g = QgsGeometry::fromWkt( "CircularStringZ (8 0 20, 5 3 30, 8 10 40)" );
      f2.setGeometry( f2g );
      QgsFeature f3;
      const QgsGeometry f3g = QgsGeometry::fromWkt( "CircularStringZ" );
      f3.setGeometry( f3g );
      QgsFeature f4;
      // TODO: Issues with Curves, should/must(?) have at least two points.
      const QgsGeometry f4g = QgsGeometry::fromWkt( "CircularStringZ (1 2 3)" );
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
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      const QgsSnappingConfig::IndividualLayerSettings layerSettings( true, Qgis::SnappingType::Vertex, 0.2, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      snappingConfig.setIntersectionSnapping( true );
      snappingConfig.setIndividualLayerSettings( vCurveZ.get(), layerSettings );
      u.setConfig( snappingConfig );

      const QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( 4.7, 3.7 ) );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.type(), QgsPointLocator::Vertex );
      QGSCOMPARENEAR( m2.point().x(), 4.8, 0.001 );
      QGSCOMPARENEAR( m2.point().y(), 3.6, 0.001 );
    }
    void testSnapOnIntersectionMultiGeom()
    {
      std::unique_ptr<QgsVectorLayer> vMulti( new QgsVectorLayer( QStringLiteral( "MultiLineStringZ" ), QStringLiteral( "m" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      const QgsGeometry f1g = QgsGeometry::fromWkt( "MultiLineStringZ ((0 0 0, 0 5 5), (5 0 10, 5 5 10))" );
      f1.setGeometry( f1g );
      QgsFeature f2;
      const QgsGeometry f2g = QgsGeometry::fromWkt( "MultiLineStringZ ((-1 2.5 50, 10 2.5 55))" );
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
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      const QgsSnappingConfig::IndividualLayerSettings layerSettings( true, Qgis::SnappingType::Vertex, 0.2, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      snappingConfig.setIntersectionSnapping( true );
      snappingConfig.setIndividualLayerSettings( vMulti.get(), layerSettings );
      u.setConfig( snappingConfig );

      const QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 0, 2.6 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::Vertex );
      QCOMPARE( m.point(), QgsPointXY( 0.0, 2.5 ) );

      const QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( 5, 2.6 ) );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.type(), QgsPointLocator::Vertex );
      QCOMPARE( m2.point(), QgsPointXY( 5.0, 2.5 ) );

    }
    void testSnapOnCentroidAndMiddleSegment()
    {
      std::unique_ptr<QgsVectorLayer> vSnapCentroidMiddle( new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "m" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      const QgsGeometry f1g = QgsGeometry::fromWkt( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" );
      f1.setGeometry( f1g );

      QgsFeatureList flist;
      flist << f1;
      vSnapCentroidMiddle->dataProvider()->addFeatures( flist );
      QVERIFY( vSnapCentroidMiddle->dataProvider()->featureCount() == 1 );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      const QgsSnappingConfig::IndividualLayerSettings layerSettings( true, static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::MiddleOfSegment | Qgis::SnappingType::Centroid ), 0.2, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      snappingConfig.setIndividualLayerSettings( vSnapCentroidMiddle.get(), layerSettings );
      u.setConfig( snappingConfig );

      const QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 0, 2.6 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::MiddleOfSegment );
      QCOMPARE( m.point(), QgsPointXY( 0.0, 2.5 ) );

      const QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( 2.5, 2.6 ) );
      QVERIFY( m2.isValid() );
      QCOMPARE( m2.type(), QgsPointLocator::Centroid );
      QCOMPARE( m2.point(), QgsPointXY( 2.5, 2.5 ) );
    }

    void testSnapOnLineEndpoints()
    {
      std::unique_ptr<QgsVectorLayer> vSnapCentroidMiddle( new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "m" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      const QgsGeometry f1g = QgsGeometry::fromWkt( "LineString (0 0, 0 5, 5 5, 5 0)" );
      f1.setGeometry( f1g );

      QgsFeatureList flist;
      flist << f1;
      vSnapCentroidMiddle->dataProvider()->addFeatures( flist );
      QVERIFY( vSnapCentroidMiddle->dataProvider()->featureCount() == 1 );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      const QgsSnappingConfig::IndividualLayerSettings layerSettings( true, static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::LineEndpoint ), 0.2, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      snappingConfig.setIndividualLayerSettings( vSnapCentroidMiddle.get(), layerSettings );
      u.setConfig( snappingConfig );

      // snap to start
      QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 0, -0.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 0.0, 0.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 0 );

      // snap to end
      m = u.snapToMap( QgsPointXY( 5, -0.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 5.0, 0.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 3 );

      // try to snap to a non start/end vertex
      const QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( -0.1, 5 ) );
      QVERIFY( !m2.isValid() );
    }

    void testSnapOnLineEndpointsMultiLine()
    {
      std::unique_ptr<QgsVectorLayer> vSnapCentroidMiddle( new QgsVectorLayer( QStringLiteral( "MultiLineString" ), QStringLiteral( "m" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      const QgsGeometry f1g = QgsGeometry::fromWkt( "MultiLineString ((0 0, 0 5, 5 5, 5 0), (0 -0.1, 0 -5, 5 -0.5))" );
      f1.setGeometry( f1g );

      QgsFeatureList flist;
      flist << f1;
      vSnapCentroidMiddle->dataProvider()->addFeatures( flist );
      QVERIFY( vSnapCentroidMiddle->dataProvider()->featureCount() == 1 );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      const QgsSnappingConfig::IndividualLayerSettings layerSettings( true, static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::LineEndpoint ), 0.2, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      snappingConfig.setIndividualLayerSettings( vSnapCentroidMiddle.get(), layerSettings );
      u.setConfig( snappingConfig );

      // snap to start
      QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 0, 0.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 0.0, 0.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 0 );

      m = u.snapToMap( QgsPointXY( 0, -0.07 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 0.0, -0.1 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 4 );

      // snap to end
      m = u.snapToMap( QgsPointXY( 5, -0.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 5.0, 0.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 3 );

      m = u.snapToMap( QgsPointXY( 5, -0.4 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 5.0, -0.5 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 6 );

      // try to snap to a non start/end vertex
      QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( -0.1, 5 ) );
      QVERIFY( !m2.isValid() );

      m2 = u.snapToMap( QgsPointXY( 0, -5 ) );
      QVERIFY( !m2.isValid() );
    }

    void testSnapOnPolygonEndpoints()
    {
      std::unique_ptr<QgsVectorLayer> vSnapCentroidMiddle( new QgsVectorLayer( QStringLiteral( "Polygon" ), QStringLiteral( "m" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      const QgsGeometry f1g = QgsGeometry::fromWkt( "Polygon ((1 0, 0 5, 5 5, 5 0, 1 0),(3 2, 3.5 2, 3.5 3, 3 2))" );
      f1.setGeometry( f1g );

      QgsFeatureList flist;
      flist << f1;
      vSnapCentroidMiddle->dataProvider()->addFeatures( flist );
      QVERIFY( vSnapCentroidMiddle->dataProvider()->featureCount() == 1 );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      const QgsSnappingConfig::IndividualLayerSettings layerSettings( true, static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::LineEndpoint ), 0.2, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      snappingConfig.setIndividualLayerSettings( vSnapCentroidMiddle.get(), layerSettings );
      u.setConfig( snappingConfig );

      // snap to start of exterior
      QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 1, -0.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 1.0, 0.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 0 );

      // snap to ring start
      m = u.snapToMap( QgsPointXY( 3, 2.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 3.0, 2.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 5 );

      // try to snap to a non start/end vertex
      QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( -0.1, 5 ) );
      QVERIFY( !m2.isValid() );
      m2 = u.snapToMap( QgsPointXY( 3.51, 3 ) );
      QVERIFY( !m2.isValid() );
    }

    void testSnapOnMultiPolygonEndpoints()
    {
      std::unique_ptr<QgsVectorLayer> vSnapCentroidMiddle( new QgsVectorLayer( QStringLiteral( "MultiPolygon" ), QStringLiteral( "m" ), QStringLiteral( "memory" ) ) );
      QgsFeature f1;
      const QgsGeometry f1g = QgsGeometry::fromWkt( "MultiPolygon (((1 0, 0 5, 5 5, 5 0, 1 0),(3 2, 3.5 2, 3.5 3, 3 2)), ((10 0, 10 5, 15 5, 15 0, 10 0),(13 2, 13.5 2, 13.5 3, 13 2)) )" );
      f1.setGeometry( f1g );

      QgsFeatureList flist;
      flist << f1;
      vSnapCentroidMiddle->dataProvider()->addFeatures( flist );
      QVERIFY( vSnapCentroidMiddle->dataProvider()->featureCount() == 1 );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      u.setMapSettings( mapSettings );
      QgsSnappingConfig snappingConfig = u.config();
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      const QgsSnappingConfig::IndividualLayerSettings layerSettings( true, static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::LineEndpoint ), 0.2, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      snappingConfig.setIndividualLayerSettings( vSnapCentroidMiddle.get(), layerSettings );
      u.setConfig( snappingConfig );

      // snap to start of exterior
      QgsPointLocator::Match m = u.snapToMap( QgsPointXY( 1, -0.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 1.0, 0.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 0 );

      m = u.snapToMap( QgsPointXY( 10, -0.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 10.0, 0.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 9 );

      // snap to ring start
      m = u.snapToMap( QgsPointXY( 3, 2.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 3.0, 2.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 5 );

      m = u.snapToMap( QgsPointXY( 13, 2.1 ) );
      QVERIFY( m.isValid() );
      QCOMPARE( m.type(), QgsPointLocator::LineEndpoint );
      QCOMPARE( m.point(), QgsPointXY( 13.0, 2.0 ) );
      QVERIFY( m.hasLineEndpoint() );
      QVERIFY( !m.hasEdge() );
      QCOMPARE( m.vertexIndex(), 14 );

      // try to snap to a non start/end vertex
      QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( -0.1, 5 ) );
      QVERIFY( !m2.isValid() );
      m2 = u.snapToMap( QgsPointXY( 3.51, 3 ) );
      QVERIFY( !m2.isValid() );
      m2 = u.snapToMap( QgsPointXY( 10, 5 ) );
      QVERIFY( !m2.isValid() );
      m2 = u.snapToMap( QgsPointXY( 13.51, 3 ) );
      QVERIFY( !m2.isValid() );
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

      const QgsPointLocator::Match m = u.snapToCurrentLayer( QPoint( 100, 100 ), QgsPointLocator::Vertex );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.point(), QgsPointXY( 1, 0 ) );
    }

    void testSnapScaleDependency()
    {
      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
      //Cannot set a specific scale directly, so play with Dpi in map settings, default scale is now 43295.7
      mapSettings.setOutputDpi( 1 );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      QgsSnappingConfig snappingConfig = u.config();
      u.setMapSettings( mapSettings );
      snappingConfig.setEnabled( true );
      snappingConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
      snappingConfig.setScaleDependencyMode( QgsSnappingConfig::Disabled );
      snappingConfig.setIndividualLayerSettings( mVL, QgsSnappingConfig::IndividualLayerSettings( true, Qgis::SnappingType::Vertex, 10, QgsTolerance::Pixels, -1.0, -1.0 ) );
      u.setConfig( snappingConfig );

      //No limit on scale
      const QgsPointLocator::Match m = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m.isValid() );
      QVERIFY( m.hasVertex() );
      QCOMPARE( m.point(), QgsPointXY( 1, 0 ) );

      snappingConfig.setScaleDependencyMode( QgsSnappingConfig::Global );
      snappingConfig.setMinimumScale( 0.0 );
      snappingConfig.setMaximumScale( 0.0 );
      u.setConfig( snappingConfig );

      //Global settings for scale limit, but scale are set to 0 -> snapping enabled
      const QgsPointLocator::Match m1 = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m1.isValid() );
      QVERIFY( m1.hasVertex() );

      snappingConfig.setScaleDependencyMode( QgsSnappingConfig::Global );
      snappingConfig.setMinimumScale( 10000.0 );// 1/10000 scale
      snappingConfig.setMaximumScale( 1000.0 );// 1/1000 scale
      u.setConfig( snappingConfig );

      //Global settings for scale limit, but scale outside min max range -> no snapping
      const QgsPointLocator::Match m2 = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m2.isValid() == false );
      QVERIFY( m2.hasVertex() == false );

      snappingConfig.setScaleDependencyMode( QgsSnappingConfig::Global );
      snappingConfig.setMinimumScale( 100000.0 );
      snappingConfig.setMaximumScale( 1000.0 );
      u.setConfig( snappingConfig );

      //Global settings for scale limit, scale inside min max range -> snapping enabled
      const QgsPointLocator::Match m3 = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m3.isValid() );
      QVERIFY( m3.hasVertex() );

      snappingConfig.setScaleDependencyMode( QgsSnappingConfig::PerLayer );
      snappingConfig.setIndividualLayerSettings( mVL, QgsSnappingConfig::IndividualLayerSettings( true, Qgis::SnappingType::Vertex, 10, QgsTolerance::Pixels, 10000.0, 1000.0 ) );
      u.setConfig( snappingConfig );

      //Per layer settings, but scale outside min max range of layer -> no snapping
      const QgsPointLocator::Match m4 = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m4.isValid() == false );
      QVERIFY( m4.hasVertex() == false );

      snappingConfig.setScaleDependencyMode( QgsSnappingConfig::PerLayer );
      snappingConfig.setIndividualLayerSettings( mVL, QgsSnappingConfig::IndividualLayerSettings( true, Qgis::SnappingType::Vertex, 10, QgsTolerance::Pixels, 100000.0, 1000.0 ) );
      u.setConfig( snappingConfig );

      //Per layer settings, scale inside min max range of layer -> snapping enabled
      const QgsPointLocator::Match m5 = u.snapToMap( QPoint( 100, 100 ) );
      QVERIFY( m5.isValid() );
      QVERIFY( m5.hasVertex() );
    }

    void testExtraSnapLayers()
    {
      // START COPYPASTE
      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 100, 100 ) );
      mapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      QgsSnappingUtils u;
      QgsSnappingConfig snappingConfig = u.config();
      u.setMapSettings( mapSettings );
      snappingConfig.setEnabled( true );
      snappingConfig.setTypeFlag( Qgis::SnappingType::Vertex );
      snappingConfig.setMode( Qgis::SnappingMode::AllLayers );
      snappingConfig.setTolerance( 5 );
      snappingConfig.setUnits( QgsTolerance::Pixels );
      u.setConfig( snappingConfig );

      // additional vector layer
      QgsVectorLayer *extraVL = new QgsVectorLayer( QStringLiteral( "Point?field=fId:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      extraVL->startEditing();

      // we start with one point: (5, 5) (at 50, 50 on screen)
      QgsFeature f3( extraVL->fields() );
      f3.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 0.50, 0.50 ) ) );
      extraVL->addFeature( f3 );
      QVERIFY( extraVL->featureCount() == 1 );

      // Without the extra snapping layer, we have no snap
      const QgsPointLocator::Match m1 = u.snapToMap( QgsPointXY( 0.50, 0.50 ) );
      QVERIFY( !m1.isValid() );

      // We add the snapping layer, we have snap
      u.addExtraSnapLayer( extraVL );
      const QgsPointLocator::Match m2 = u.snapToMap( QgsPointXY( 0.50, 0.50 ) );
      QVERIFY( m2.isValid() );

      // We add to the snapping layer, the snap changed
      QgsFeature f4( extraVL->fields() );
      f4.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 0.75, 0.75 ) ) );
      extraVL->addFeature( f4 );
      QVERIFY( extraVL->featureCount() == 2 );
      const QgsPointLocator::Match m3 = u.snapToMap( QgsPointXY( 0.50, 0.50 ) );
      const QgsPointLocator::Match m4 = u.snapToMap( QgsPointXY( 0.75, 0.75 ) );
      QVERIFY( m3.isValid() );
      QVERIFY( m4.isValid() );

      // We remove the snapping layer, we have no snap
      u.removeExtraSnapLayer( extraVL );
      const QgsPointLocator::Match m5 = u.snapToMap( QgsPointXY( 0.50, 0.50 ) );
      const QgsPointLocator::Match m6 = u.snapToMap( QgsPointXY( 0.75, 0.75 ) );
      QVERIFY( !m5.isValid() );
      QVERIFY( !m6.isValid() );
    }
};

QGSTEST_MAIN( TestQgsSnappingUtils )

#include "testqgssnappingutils.moc"
