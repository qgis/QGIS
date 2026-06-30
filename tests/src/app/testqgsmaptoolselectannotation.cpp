/***************************************************************************
    testqgsmaptoolselectannotation.cpp
     --------------------------------------
    Date                 : December 2025
    Copyright            : (C) 2025 by Mathieu Pellerin
    Email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsannotationlayer.h"
#include "qgsannotationpolygonitem.h"
#include "qgsannotationrectangletextitem.h"
#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmaptoolselectannotation.h"
#include "qgspolygon.h"
#include "qgsproject.h"
#include "qgsrendereditemresults.h"
#include "qgstest.h"
#include "testqgsmaptoolutils.h"

#include <QCoreApplication>
#include <QSignalSpy>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsMapToolSelectAnnotation : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolSelectAnnotation() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testSelectItem();
    void testDeleteItem();
    void testMoveItem();
    void testMoveItemRotatedCanvas();
    void testRotateItem();
    void testResizeRotatedItem();
};

void TestQgsMapToolSelectAnnotation::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsMapToolSelectAnnotation::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolSelectAnnotation::init()
{}

void TestQgsMapToolSelectAnnotation::cleanup()
{}

void TestQgsMapToolSelectAnnotation::testSelectItem()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( u"test"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer->isValid() );
  QgsAnnotationLayer *layer2 = new QgsAnnotationLayer( u"test"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer2->isValid() );
  QgsProject::instance()->addMapLayers( { layer, layer2 } );

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem(
    new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) )
  );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );

  QgsAnnotationPolygonItem *item2 = new QgsAnnotationPolygonItem(
    new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 4 ), QgsPoint( 1.6, 4 ), QgsPoint( 1.6, 4.6 ), QgsPoint( 1, 4.6 ), QgsPoint( 1, 4 ) } ) )
  );
  item2->setZIndex( 0 );
  const QString i2id = layer->addItem( item2 );

  QgsAnnotationPolygonItem *item3 = new QgsAnnotationPolygonItem(
    new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 7, 1 ), QgsPoint( 8, 1 ), QgsPoint( 8, 2 ), QgsPoint( 7, 2 ), QgsPoint( 7, 1 ) } ) )
  );
  item3->setZIndex( 3 );
  const QString i3id = layer2->addItem( item3 );

  layer->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  layer2->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  canvas.setLayers( { layer, layer2 } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 3 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolSelectAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spySingleItemSelected( &tool, &QgsMapToolSelectAnnotation::singleItemSelected );
  QSignalSpy spySelectedItemsChanged( &tool, &QgsMapToolSelectAnnotation::selectedItemsChanged );
  TestQgsMapToolUtils utils( &tool );

  // click outside of items
  utils.mouseMove( 9, 9 );
  utils.mouseClick( 9, 9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spySingleItemSelected.count(), 0 );
  QCOMPARE( spySelectedItemsChanged.count(), 0 );

  // click on items
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spySingleItemSelected.count(), 1 );
  QCOMPARE( spySelectedItemsChanged.count(), 1 );
  QCOMPARE( spySingleItemSelected.at( 0 ).at( 1 ).toString(), i1id );

  // click outside of items to deselect
  utils.mouseMove( 9, 9 );
  utils.mouseClick( 9, 9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spySingleItemSelected.count(), 1 );
  QCOMPARE( spySelectedItemsChanged.count(), 2 );

  // overlapping items, smallest area item should be selected
  utils.mouseMove( 1.5, 4.5 );
  utils.mouseClick( 1.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spySingleItemSelected.count(), 2 );
  QCOMPARE( spySelectedItemsChanged.count(), 3 );
  QCOMPARE( spySingleItemSelected.at( 1 ).at( 1 ).toString(), i2id );

  // click on third item
  utils.mouseMove( 7.5, 1.5 );
  utils.mouseClick( 7.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spySingleItemSelected.count(), 3 );
  QCOMPARE( spySelectedItemsChanged.count(), 4 );
  QCOMPARE( spySingleItemSelected.at( 2 ).at( 1 ).toString(), i3id );

  // click on third item using shift to toggle selection off
  utils.mouseMove( 7.5, 1.5 );
  utils.mouseClick( 7.5, 1.5, Qt::LeftButton, Qt::ShiftModifier, true );
  QCOMPARE( spySingleItemSelected.count(), 3 );
  QCOMPARE( spySelectedItemsChanged.count(), 5 );

  // click on third item using shift to toggle selection back on
  utils.mouseMove( 7.5, 1.5 );
  utils.mouseClick( 7.5, 1.5, Qt::LeftButton, Qt::ShiftModifier, true );
  QCOMPARE( spySingleItemSelected.count(), 4 );
  QCOMPARE( spySelectedItemsChanged.count(), 6 );

  // click on no item - should clear selection
  QSignalSpy spySelectionCleared( &tool, &QgsMapToolSelectAnnotation::selectionCleared );
  utils.mouseMove( 9.5, 9.5 );
  utils.mouseClick( 9.5, 9.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spySingleItemSelected.count(), 4 );
  QCOMPARE( spySelectedItemsChanged.count(), 7 );
  QCOMPARE( spySelectionCleared.count(), 1 );
}

void TestQgsMapToolSelectAnnotation::testDeleteItem()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( u"test"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayers( { layer } );

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem(
    new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) )
  );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );

  QgsAnnotationPolygonItem *item2 = new QgsAnnotationPolygonItem(
    new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 4 ), QgsPoint( 5, 4 ), QgsPoint( 5, 9 ), QgsPoint( 1, 9 ), QgsPoint( 1, 4 ) } ) )
  );
  item2->setZIndex( 2 );
  const QString i2id = layer->addItem( item2 );

  QgsAnnotationPolygonItem *item3 = new QgsAnnotationPolygonItem(
    new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 7, 1 ), QgsPoint( 8, 1 ), QgsPoint( 8, 2 ), QgsPoint( 7, 2 ), QgsPoint( 7, 1 ) } ) )
  );
  item3->setZIndex( 3 );
  const QString i3id = layer->addItem( item3 );

  layer->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 3 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolSelectAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  TestQgsMapToolUtils utils( &tool );

  // no selected item
  utils.mouseMove( 9, 9 );
  utils.mouseClick( 9, 9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.keyClick( Qt::Key_Delete );
  QCOMPARE( qgis::listToSet( layer->items().keys() ), QSet<QString>( { i1id, i2id, i3id } ) );

  // with selected item
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.keyClick( Qt::Key_Delete );
  QCOMPARE( qgis::listToSet( layer->items().keys() ), QSet<QString>( { i2id, i3id } ) );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();

  utils.mouseMove( 1.5, 4.5 );
  utils.mouseClick( 1.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.keyClick( Qt::Key_Delete );
  QCOMPARE( qgis::listToSet( layer->items().keys() ), QSet<QString>( { i3id } ) );
}

void TestQgsMapToolSelectAnnotation::testMoveItem()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( u"test"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayers( { layer } );

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem(
    new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) )
  );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );
  QCOMPARE( qgis::down_cast<QgsAnnotationPolygonItem *>( layer->item( i1id ) )->geometry()->asWkt(), u"Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))"_s );

  layer->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolSelectAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spy( &tool, &QgsMapToolSelectAnnotation::singleItemSelected );
  TestQgsMapToolUtils utils( &tool );

  // click on item
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 1 ).toString(), i1id );

  // a mouse press on the item will start moving the item
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  // move mouse and click to end item move
  utils.mouseMove( 4.5, 4.5, Qt::LeftButton );
  utils.mouseMove( 4.5, 4.5, Qt::LeftButton );
  utils.mouseClick( 4.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that item was moved
  QCOMPARE( qgis::down_cast<QgsAnnotationPolygonItem *>( layer->item( i1id ) )->geometry()->asWkt(), u"Polygon ((4 4, 8 4, 8 8, 4 8, 4 4))"_s );

  // start a new move
  utils.mouseMove( 4.6, 4.5 );
  utils.mouseClick( 4.6, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 1.5, 1.5, Qt::LeftButton );
  utils.mouseMove( 1.5, 1.5, Qt::LeftButton );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that item was moved
  QCOMPARE( qgis::down_cast<QgsAnnotationPolygonItem *>( layer->item( i1id ) )->geometry()->asWkt( 1 ), u"Polygon ((0.9 1, 4.9 1, 4.9 5, 0.9 5, 0.9 1))"_s );
}


void TestQgsMapToolSelectAnnotation::testMoveItemRotatedCanvas()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  // rotate the map canvas: a screen-space drag must still translate the item
  // correctly, taking the canvas rotation into account.
  canvas.setRotation( 90 );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( u"test"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayers( { layer } );

  QgsAnnotationPolygonItem *item1 = new QgsAnnotationPolygonItem(
    new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( 1, 1 ), QgsPoint( 5, 1 ), QgsPoint( 5, 5 ), QgsPoint( 1, 5 ), QgsPoint( 1, 1 ) } ) )
  );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );
  QCOMPARE( qgis::down_cast<QgsAnnotationPolygonItem *>( layer->item( i1id ) )->geometry()->asWkt(), u"Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))"_s );

  layer->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolSelectAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spy( &tool, &QgsMapToolSelectAnnotation::singleItemSelected );
  TestQgsMapToolUtils utils( &tool );

  // click on item to select it
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 1 ).toString(), i1id );

  // a mouse press on the item will start moving the item
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  // drag from map (1.5, 1.5) to map (4.5, 4.5): despite the 90 degree canvas
  // rotation, the item should follow the cursor and move by (3, 3) in map units.
  utils.mouseMove( 4.5, 4.5, Qt::LeftButton );
  utils.mouseMove( 4.5, 4.5, Qt::LeftButton );
  utils.mouseClick( 4.5, 4.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  // check that the item was moved by (3, 3) in map units
  QCOMPARE( qgis::down_cast<QgsAnnotationPolygonItem *>( layer->item( i1id ) )->geometry()->asWkt( 0 ), u"Polygon ((4 4, 8 4, 8 8, 4 8, 4 4))"_s );
}


void TestQgsMapToolSelectAnnotation::testRotateItem()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( u"test"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayers( { layer } );

  QgsAnnotationRectangleTextItem *item1 = new QgsAnnotationRectangleTextItem( u"test"_s, QgsRectangle( 2, 2, 6, 6 ) );
  item1->setZIndex( 1 );
  const QString i1id = layer->addItem( item1 );
  QCOMPARE( qgis::down_cast<QgsAnnotationRectangleTextItem *>( layer->item( i1id ) )->rotation(), 0.0 );

  layer->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolSelectAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  QSignalSpy spy( &tool, &QgsMapToolSelectAnnotation::singleItemSelected );
  TestQgsMapToolUtils utils( &tool );

  // click on the item to select it
  utils.mouseMove( 4, 4 );
  utils.mouseClick( 4, 4, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 1 ).toString(), i1id );

  QList<QgsAnnotationItemRubberBand *> selected = tool.selectedItems();
  QCOMPARE( selected.size(), 1 );

  // rotating the rubber band via its handles rotates the underlying item
  tool.attemptRotateBy( selected.first(), 30 );
  QCOMPARE( qgis::down_cast<QgsAnnotationRectangleTextItem *>( layer->item( i1id ) )->rotation(), 30.0 );

  // a further rotation accumulates
  tool.attemptRotateBy( selected.first(), 20 );
  QCOMPARE( qgis::down_cast<QgsAnnotationRectangleTextItem *>( layer->item( i1id ) )->rotation(), 50.0 );

  // the item bounds (center and size) are unchanged by rotation
  const QgsRectangle bounds = qgis::down_cast<QgsAnnotationRectangleTextItem *>( layer->item( i1id ) )->bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), 2.0, 0.001 );
  QGSCOMPARENEAR( bounds.yMinimum(), 2.0, 0.001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 6.0, 0.001 );
  QGSCOMPARENEAR( bounds.yMaximum(), 6.0, 0.001 );
}


void TestQgsMapToolSelectAnnotation::testResizeRotatedItem()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  canvas.setFrameStyle( QFrame::NoFrame );
  canvas.resize( 600, 600 );
  canvas.setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  canvas.show(); // to make the canvas resize

  QgsAnnotationLayer *layer = new QgsAnnotationLayer( u"test"_s, QgsAnnotationLayer::LayerOptions( QgsProject::instance()->transformContext() ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayers( { layer } );

  QgsAnnotationRectangleTextItem *item1 = new QgsAnnotationRectangleTextItem( u"test"_s, QgsRectangle( 2, 2, 6, 6 ) );
  item1->setZIndex( 1 );
  // rotate the item: resizing must keep the dragged corner anchored, so the
  // reconstructed (unrotated) bounds are shifted accordingly.
  item1->setRotation( 90 );
  const QString i1id = layer->addItem( item1 );

  layer->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  canvas.setLayers( { layer } );
  while ( !canvas.isDrawing() )
  {
    QgsApplication::processEvents();
  }
  canvas.waitWhileRendering();
  QCOMPARE( canvas.renderedItemResults()->renderedItems().size(), 1 );

  QgsAdvancedDigitizingDockWidget cadDock( &canvas );
  QgsMapToolSelectAnnotation tool( &canvas, &cadDock );
  canvas.setMapTool( &tool );

  TestQgsMapToolUtils utils( &tool );

  // click on the item to select it
  utils.mouseMove( 4, 4 );
  utils.mouseClick( 4, 4, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QList<QgsAnnotationItemRubberBand *> selected = tool.selectedItems();
  QCOMPARE( selected.size(), 1 );

  // map (2,2)-(6,6) maps to the scene rectangle (120,240)-(360,480) with the
  // 600x600 canvas covering the 10x10 extent. The new scene rectangle is 300x240
  // pixels (5x4 map units). Because the item is rotated 90 degrees, the dragged
  // corner is kept anchored: the resulting unrotated bounds are recentred.
  tool.attemptSetSceneRect( selected.first(), QRectF( 120, 240, 300, 240 ) );

  const QgsRectangle bounds = qgis::down_cast<QgsAnnotationRectangleTextItem *>( layer->item( i1id ) )->bounds();
  // the resized bounds keep the new pixel size (5 x 4 map units) ...
  QGSCOMPARENEAR( bounds.width(), 5.0, 0.01 );
  QGSCOMPARENEAR( bounds.height(), 4.0, 0.01 );
  // ... recentred to anchor the rotated corner
  QGSCOMPARENEAR( bounds.xMinimum(), -2.5, 0.01 );
  QGSCOMPARENEAR( bounds.yMinimum(), 1.5, 0.01 );
  QGSCOMPARENEAR( bounds.xMaximum(), 2.5, 0.01 );
  QGSCOMPARENEAR( bounds.yMaximum(), 5.5, 0.01 );
}


QGSTEST_MAIN( TestQgsMapToolSelectAnnotation )
#include "testqgsmaptoolselectannotation.moc"
