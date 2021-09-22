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
    void testDeleteItem();
    void testMoveItem();
    void testMoveNode();
    void testDeleteNode();
    void testAddNode();

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
  QgsProject::instance()->clear();
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

void TestQgsMapToolEditAnnotation::testDeleteItem()
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

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) ) );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );

  QgsAnnotationPolygonItem *item2 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 4 ), QgsPoint( 5, 4 ), QgsPoint( 5, 9 ), QgsPoint( 1, 9 ), QgsPoint( 1, 4 ) } ) ) );
  item2->setZIndex( 2 );
  const QString i2id = layer->addItem( item2 );

  QgsAnnotationPolygonItem *item3 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 7, 1 ), QgsPoint( 8, 1 ), QgsPoint( 8, 2 ), QgsPoint( 7, 2 ), QgsPoint( 7, 1 ) } ) ) );
  item3->setZIndex( 3 );
  const QString i3id = layer->addItem( item3 );

  layer->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 3 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolModifyAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  TestQgsMapToolUtils utils( &tool );

  // no selected item
  utils.mouseMove( 9, 9 );
  utils.mouseClick( 9, 9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.keyClick( Qt::Key_Delete );
  QCOMPARE( qgis::listToSet( layer->items().keys() ), QSet< QString >( { i1id, i2id, i3id } ) );

  // with selected item
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.keyClick( Qt::Key_Delete );
  QCOMPARE( qgis::listToSet( layer->items().keys() ), QSet< QString >( { i2id, i3id } ) );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();

  utils.mouseMove( 1.5, 4.5 );
  utils.mouseClick( 1.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.keyClick( Qt::Key_Delete );
  QCOMPARE( qgis::listToSet( layer->items().keys() ), QSet< QString >( { i3id } ) );
}

void TestQgsMapToolEditAnnotation::testMoveItem()
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

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) ) );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt(), QStringLiteral( "Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))" ) );

  layer->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolModifyAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spy( &tool, &QgsMapToolModifyAnnotation::itemSelected );
  TestQgsMapToolUtils utils( &tool );

  // click on item
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 1 ).toString(), i1id );

  // a second left click on the item will start moving the item
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  // second click isn't selecting an item, so no new signals should be emitted
  QCOMPARE( spy.count(), 1 );

  // move mouse and click to end item move
  utils.mouseMove( 4.5, 4.5 );
  utils.mouseClick( 4.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that item was moved
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt(), QStringLiteral( "Polygon ((4 4, 8 4, 8 8, 4 8, 4 4))" ) );

  // start a new move
  // click on item -- it should already be selected, so this will start a new move, not emit the itemSelected signal
  utils.mouseMove( 4.6, 4.5 );
  utils.mouseClick( 4.6, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );

  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that item was moved
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt( 1 ), QStringLiteral( "Polygon ((0.9 1, 4.9 1, 4.9 5, 0.9 5, 0.9 1))" ) );

  // start a move then cancel it via right click
  utils.mouseMove( 1.6, 1.6 );
  utils.mouseClick( 1.6, 1.6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );

  utils.mouseMove( 4.5, 4.5 );
  utils.mouseClick( 4.5, 4.5, Qt::RightButton, Qt::KeyboardModifiers(), true );
  // check that item was NOT moved
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt( 1 ), QStringLiteral( "Polygon ((0.9 1, 4.9 1, 4.9 5, 0.9 5, 0.9 1))" ) );

  // cancel a move via escape key
  utils.mouseMove( 1.6, 1.6 );
  utils.mouseClick( 1.6, 1.6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );

  utils.mouseMove( 4.5, 4.5 );
  // escape should cancel
  utils.keyClick( Qt::Key_Escape );
  //... so next click is not "finish move", but "clear selection"
  utils.mouseClick( 4.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  // check that item was NOT moved
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt( 1 ), QStringLiteral( "Polygon ((0.9 1, 4.9 1, 4.9 5, 0.9 5, 0.9 1))" ) );
}

void TestQgsMapToolEditAnnotation::testMoveNode()
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

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) ) );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt(), QStringLiteral( "Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))" ) );

  layer->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolModifyAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spy( &tool, &QgsMapToolModifyAnnotation::itemSelected );
  TestQgsMapToolUtils utils( &tool );

  // click on item
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 1 ).toString(), i1id );

  // click on a node
  utils.mouseMove( 5, 5 );
  utils.mouseClick( 5, 5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  // second click isn't selecting an item, so no new signals should be emitted
  QCOMPARE( spy.count(), 1 );

  // move mouse and click to end node move
  utils.mouseMove( 4.5, 4.5 );
  utils.mouseClick( 4.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that item was moved
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt(), QStringLiteral( "Polygon ((1 1, 5 1, 4.5 4.5, 1 5, 1 1))" ) );

  // start a new move node
  // click on item -- it should already be selected, so this will start a new move, not emit the itemSelected signal
  utils.mouseMove( 5, 1 );
  utils.mouseClick( 5, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );

  utils.mouseMove( 5.5, 1.5 );
  utils.mouseClick( 5.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that item was moved
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt( 1 ), QStringLiteral( "Polygon ((1 1, 5.5 1.5, 4.5 4.5, 1 5, 1 1))" ) );

  // start a move then cancel it via right click
  utils.mouseMove( 4.5, 4.5 );
  utils.mouseClick( 4.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );

  utils.mouseMove( 4.9, 4.9 );
  utils.mouseClick( 4.9, 4.9, Qt::RightButton, Qt::KeyboardModifiers(), true );
  // check that node was NOT moved
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt( 1 ), QStringLiteral( "Polygon ((1 1, 5.5 1.5, 4.5 4.5, 1 5, 1 1))" ) );

  // cancel a move via escape key
  utils.mouseMove( 4.5, 4.5 );
  utils.mouseClick( 4.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );

  utils.mouseMove( 4.9, 4.9 );
  // escape should cancel
  utils.keyClick( Qt::Key_Escape );
  //... so next click is not "finish move", but "clear selection"
  utils.mouseClick( 6.5, 6.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  // check that node was NOT moved
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt( 1 ), QStringLiteral( "Polygon ((1 1, 5.5 1.5, 4.5 4.5, 1 5, 1 1))" ) );
}

void TestQgsMapToolEditAnnotation::testDeleteNode()
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

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) ) );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt(), QStringLiteral( "Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))" ) );

  layer->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolModifyAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spy( &tool, &QgsMapToolModifyAnnotation::itemSelected );
  TestQgsMapToolUtils utils( &tool );

  // click on item
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 1 ).toString(), i1id );

  // click on a node
  utils.mouseMove( 5, 5 );
  utils.mouseClick( 5, 5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  // second click isn't selecting an item, so no new signals should be emitted
  QCOMPARE( spy.count(), 1 );

  // delete node
  utils.keyClick( Qt::Key_Delete );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that node was deleted
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt(), QStringLiteral( "Polygon ((1 1, 5 1, 1 5, 1 1))" ) );

  // start a new delete node
  // click on item -- it should already be selected, so this will start a new move, not emit the itemSelected signal
  utils.mouseMove( 5, 1 );
  utils.mouseClick( 5, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );

  utils.keyClick( Qt::Key_Delete );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();

  // check that node was deleted -- this should delete the whole item, as its geometry was cleared
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 0 );
  QVERIFY( !layer->item( i1id ) );
}

void TestQgsMapToolEditAnnotation::testAddNode()
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

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) ) );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt(), QStringLiteral( "Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))" ) );

  layer->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolModifyAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spy( &tool, &QgsMapToolModifyAnnotation::itemSelected );
  TestQgsMapToolUtils utils( &tool );

  // click on item
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 1 ).toString(), i1id );

  // double-click a segment
  utils.mouseMove( 5, 3 );
  utils.mouseDoubleClick( 5, 3, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  // second click isn't selecting an item, so no new signals should be emitted
  QCOMPARE( spy.count(), 1 );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that node was added
  QCOMPARE( qgis::down_cast< QgsAnnotationPolygonItem * >( layer->item( i1id ) )->geometry()->asWkt(), QStringLiteral( "Polygon ((1 1, 5 1, 5 3, 5 5, 1 5, 1 1))" ) );
}


QGSTEST_MAIN( TestQgsMapToolEditAnnotation )
#include "testqgsmaptooleditannotation.moc"
