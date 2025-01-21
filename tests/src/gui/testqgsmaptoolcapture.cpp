/***************************************************************************
    testqgsmaptooledit.cpp
     --------------------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QCoreApplication>

#include "qgsmapcanvassnappingutils.h"
#include "qgstest.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"
#include "qgsannotationlayer.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "testqgsmaptoolutils.h"


class TestQgsMapToolCapture : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolCapture() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void addVertexNoLayer();
    void addPointNoLayerSnapping();
    void addVertexNonVectorLayer();
    void addVertexNonVectorLayerTransform();
};

void TestQgsMapToolCapture::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsMapToolCapture::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolCapture::init()
{
}

void TestQgsMapToolCapture::cleanup()
{
}

void TestQgsMapToolCapture::addVertexNoLayer()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  canvas.setCurrentLayer( nullptr );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolCapture tool( &canvas, &cadDock, QgsMapToolCapture::CaptureLine );
  canvas.setMapTool( &tool );

  // even though we don't have any asssociated layer, adding vertices should still be allowed
  QCOMPARE( tool.addVertex( QgsPoint( 5, 5 ), QgsPointLocator::Match() ), 0 );

  QCOMPARE( tool.captureCurve()->asWkt(), QStringLiteral( "CompoundCurve ((5 5))" ) );

  // the nextPoint method must also handle no layer
  QgsPoint layerPoint;
  QCOMPARE( tool.nextPoint( QgsPoint( 5, 6 ), layerPoint ), 0 );
  QCOMPARE( layerPoint.x(), 5.0 );
  QCOMPARE( layerPoint.y(), 6.0 );
}

void TestQgsMapToolCapture::addPointNoLayerSnapping()
{
  // checks that snapping works even if no layer is set as current layer

  QgsPoint p = QgsPoint( 2556607, 1115175 );

  QgsProject::instance()->clear();

  QgsMapCanvas canvas;
  canvas.resize( 600, 600 );
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:2056" ) ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.setExtent( QgsRectangle( p.x() - 50, p.y() - 50, p.x() + 50, p.y() + 50 ) );
  canvas.show(); // to make the canvas resize

  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:2056" ), QStringLiteral( "point" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( layer );
  QgsFeature f( layer->fields() );
  f.setGeometry( QgsGeometry( p.clone() ) );
  QgsFeatureList flist { f };
  layer->dataProvider()->addFeatures( flist );
  canvas.setLayers( { layer } );

  QgsMapSettings mapSettings = canvas.mapSettings();
  QVERIFY( mapSettings.hasValidSettings() );

  QgsSnappingUtils u;
  u.setMapSettings( mapSettings );

  QgsSnappingConfig snappingConfig = u.config();
  snappingConfig.setEnabled( true );
  snappingConfig.setTolerance( 10 );
  snappingConfig.setUnits( Qgis::MapToolUnit::Pixels );
  snappingConfig.setMode( Qgis::SnappingMode::AllLayers );
  snappingConfig.setTypeFlag( Qgis::SnappingType::Vertex );
  u.setConfig( snappingConfig );

  QgsMapCanvasSnappingUtils *snappingUtils = new QgsMapCanvasSnappingUtils( &canvas, this );
  snappingUtils->setConfig( snappingConfig );
  snappingUtils->setMapSettings( mapSettings );
  snappingUtils->locatorForLayer( layer )->init();

  canvas.setSnappingUtils( snappingUtils );

  canvas.setCurrentLayer( nullptr );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );

  QgsMapToolCapture tool( &canvas, &cadDock, QgsMapToolCapture::CaptureLine );
  canvas.setMapTool( &tool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &tool );
  utils.mouseClick( p.x() + .5, p.y() + .5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QgsPoint toolPoint = tool.captureCurve()->vertexAt( QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( toolPoint, p );
}

void TestQgsMapToolCapture::addVertexNonVectorLayer()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( QStringLiteral( "test" ), QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayers( { layer } );

  canvas.setLayers( { layer } );
  canvas.setCurrentLayer( layer );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolCapture tool( &canvas, &cadDock, QgsMapToolCapture::CaptureLine );
  canvas.setMapTool( &tool );

  // even though we don't have a vector layer selected, adding vertices should still be allowed
  QCOMPARE( tool.addVertex( QgsPoint( 5, 5 ), QgsPointLocator::Match() ), 0 );

  QCOMPARE( tool.captureCurve()->asWkt(), QStringLiteral( "CompoundCurve ((5 5))" ) );

  // the nextPoint method must also accept non vector layers
  QgsPoint layerPoint;
  QCOMPARE( tool.nextPoint( QgsPoint( 5, 6 ), layerPoint ), 0 );
  QCOMPARE( layerPoint.x(), 5.0 );
  QCOMPARE( layerPoint.y(), 6.0 );
}

void TestQgsMapToolCapture::addVertexNonVectorLayerTransform()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( QStringLiteral( "test" ), QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  layer->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayers( { layer } );

  canvas.setLayers( { layer } );
  canvas.setCurrentLayer( layer );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolCapture tool( &canvas, &cadDock, QgsMapToolCapture::CaptureLine );
  canvas.setMapTool( &tool );

  // even though we don't have a vector layer selected, adding vertices should still be allowed
  QCOMPARE( tool.addVertex( QgsPoint( 5, 5 ), QgsPointLocator::Match() ), 0 );

  QCOMPARE( tool.captureCurve()->asWkt( 0 ), QStringLiteral( "CompoundCurve ((556597 557305))" ) );

  // the nextPoint method must also accept non vector layers
  QgsPoint layerPoint;
  QCOMPARE( tool.nextPoint( QgsPoint( 5, 6 ), layerPoint ), 0 );
  QGSCOMPARENEAR( layerPoint.x(), 556597, 10 );
  QGSCOMPARENEAR( layerPoint.y(), 669141, 10 );
}

QGSTEST_MAIN( TestQgsMapToolCapture )
#include "testqgsmaptoolcapture.moc"
