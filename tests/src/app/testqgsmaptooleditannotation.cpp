/***************************************************************************
    testqgsmaptooleditannotation.cpp
     --------------------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 by Nyall Dawson
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
#include "qgsmaptooledit.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"
#include "qgsannotationlayer.h"
#include "qgsannotationpolygonitem.h"
#include "qgsproject.h"
#include "testqgsmaptoolutils.h"
#include "qgsmapmouseevent.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmaptoolmodifyannotation.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsrendereditemresults.h"

#include <QSignalSpy>

class TestQgsMapToolEditAnnotation : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolEditAnnotation() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testSelectItem();

};

void TestQgsMapToolEditAnnotation::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsMapToolEditAnnotation::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolEditAnnotation::init()
{
}

void TestQgsMapToolEditAnnotation::cleanup()
{
}

void TestQgsMapToolEditAnnotation::testSelectItem()
{
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( QStringLiteral( "test" ), QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer->isValid() );
  QgsAnnotationLayer *layer2 = new QgsAnnotationLayer( QStringLiteral( "test" ), QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer2->isValid() );
  QgsProject::instance()->addMapLayers( { layer, layer2 } );

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) ) );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );

  QgsAnnotationPolygonItem *item2 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 4 ), QgsPoint( 5, 4 ), QgsPoint( 5, 9 ), QgsPoint( 1, 9 ), QgsPoint( 1, 4 ) } ) ) );
  item2->setZIndex( 2 );
  const QString i2id = layer->addItem( item2 );

  QgsAnnotationPolygonItem *item3 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 7, 1 ), QgsPoint( 8, 1 ), QgsPoint( 8, 2 ), QgsPoint( 7, 2 ), QgsPoint( 7, 1 ) } ) ) );
  item3->setZIndex( 3 );
  const QString i3id = layer2->addItem( item3 );

  layer->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  layer2->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  canvas.setLayers( { layer, layer2 } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 3 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolModifyAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spy( &tool, &QgsMapToolModifyAnnotation::itemSelected );
  TestQgsMapToolUtils utils( &tool );

  // click outside of items
  utils.mouseMove( 9, 9 );
  utils.mouseClick( 9, 9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 0 );

  // click on items
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 1 ).toString(), i1id );
  // second click on same item -- no signal
  utils.mouseMove( 1.6, 1.5 );
  utils.mouseClick( 1.6, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );

  // overlapping items, highest z order should be selected
  utils.mouseMove( 1.5, 4.5 );
  utils.mouseClick( 1.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 1 ).toString(), i2id );

  utils.mouseMove( 7.5, 1.5 );
  utils.mouseClick( 7.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.at( 2 ).at( 1 ).toString(), i3id );

  // click on no item - should clear selection
  QSignalSpy selectionClearedSpy( &tool, &QgsMapToolModifyAnnotation::selectionCleared );
  utils.mouseMove( 9.5, 9.5 );
  utils.mouseClick( 9.5, 9.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( selectionClearedSpy.count(), 1 );
}


QGSTEST_MAIN( TestQgsMapToolEditAnnotation )
#include "testqgsmaptooleditannotation.moc"
