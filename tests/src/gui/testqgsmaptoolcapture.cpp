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

#include "qgstest.h"
#include "qgsguiutils.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"
#include "qgsannotationlayer.h"
#include "qgsadvanceddigitizingdockwidget.h"

class TestQgsMapToolCapture : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolCapture() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void addVertexNoLayer();
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
