/***************************************************************************
    testqgslayoutview.cpp
     --------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgslayoutview.h"
#include "qgslayoutviewtool.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutitem.h"
#include "qgslayoutviewrubberband.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutitemwidget.h"
#include "qgsproject.h"
#include "qgslayout.h"
#include <QtTest/QSignalSpy>
#include <QSvgGenerator>
#include <QPrinter>

class TestQgsLayoutView: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void basic();
    void tool();
    void events();
    void guiRegistry();
    void rubberBand();

  private:

};

void TestQgsLayoutView::initTestCase()
{

}

void TestQgsLayoutView::cleanupTestCase()
{
}

void TestQgsLayoutView::init()
{
}

void TestQgsLayoutView::cleanup()
{
}

void TestQgsLayoutView::basic()
{
  QgsProject p;
  QgsLayout *layout = new QgsLayout( &p );
  QgsLayoutView *view = new QgsLayoutView();

  const QSignalSpy spyLayoutChanged( view, &QgsLayoutView::layoutSet );
  view->setCurrentLayout( layout );
  QCOMPARE( view->currentLayout(), layout );
  QCOMPARE( spyLayoutChanged.count(), 1 );

  delete view;
  delete layout;
}

void TestQgsLayoutView::tool()
{
  QgsLayoutView *view = new QgsLayoutView();
  QgsLayoutViewTool *tool = new QgsLayoutViewTool( view, QStringLiteral( "name" ) );
  QgsLayoutViewTool *tool2 = new QgsLayoutViewTool( view, QStringLiteral( "name2" ) );

  QVERIFY( tool->isClickAndDrag( QPoint( 0, 10 ), QPoint( 5, 10 ) ) );
  QVERIFY( tool->isClickAndDrag( QPoint( 0, 10 ), QPoint( 5, 15 ) ) );
  QVERIFY( tool->isClickAndDrag( QPoint( 5, 10 ), QPoint( 5, 15 ) ) );
  QVERIFY( !tool->isClickAndDrag( QPoint( 0, 10 ), QPoint( 1, 11 ) ) );
  QVERIFY( !tool->isClickAndDrag( QPoint( 1, 10 ), QPoint( 1, 11 ) ) );
  QVERIFY( !tool->isClickAndDrag( QPoint( 0, 10 ), QPoint( 1, 10 ) ) );
  QVERIFY( !tool->isClickAndDrag( QPoint( 0, 10 ), QPoint( 0, 10 ) ) );

  const QSignalSpy spySetTool( view, &QgsLayoutView::toolSet );
  const QSignalSpy spyToolActivated( tool, &QgsLayoutViewTool::activated );
  const QSignalSpy spyToolActivated2( tool2, &QgsLayoutViewTool::activated );
  const QSignalSpy spyToolDeactivated( tool, &QgsLayoutViewTool::deactivated );
  const QSignalSpy spyToolDeactivated2( tool2, &QgsLayoutViewTool::deactivated );
  view->setTool( tool );
  QCOMPARE( view->tool(), tool );
  QCOMPARE( spySetTool.count(), 1 );
  QCOMPARE( spyToolActivated.count(), 1 );
  QCOMPARE( spyToolDeactivated.count(), 0 );

  view->setTool( tool2 );
  QCOMPARE( view->tool(), tool2 );
  QCOMPARE( spySetTool.count(), 2 );
  QCOMPARE( spyToolActivated.count(), 1 );
  QCOMPARE( spyToolDeactivated.count(), 1 );
  QCOMPARE( spyToolActivated2.count(), 1 );
  QCOMPARE( spyToolDeactivated2.count(), 0 );

  delete tool2;
  QVERIFY( !view->tool() );
  QCOMPARE( spySetTool.count(), 3 );
  QCOMPARE( spyToolActivated.count(), 1 );
  QCOMPARE( spyToolDeactivated.count(), 1 );
  QCOMPARE( spyToolActivated2.count(), 1 );
  QCOMPARE( spyToolDeactivated2.count(), 1 );

  delete view;
}

class LoggingTool : public QgsLayoutViewTool // clazy:exclude=missing-qobject-macro
{
  public:

    LoggingTool( QgsLayoutView *view )
      : QgsLayoutViewTool( view, QStringLiteral( "logging" ) )
    {}

    bool receivedMoveEvent = false;
    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override
    {
      receivedMoveEvent = true;
      QCOMPARE( event->layoutPoint().x(), 8.0 );
      QCOMPARE( event->layoutPoint().y(), 6.0 );
    }

    bool receivedDoubleClickEvent = false;
    void layoutDoubleClickEvent( QgsLayoutViewMouseEvent *event ) override
    {
      receivedDoubleClickEvent = true;
      QCOMPARE( event->layoutPoint().x(), 8.0 );
      QCOMPARE( event->layoutPoint().y(), 6.0 );
    }

    bool receivedPressEvent = false;
    void layoutPressEvent( QgsLayoutViewMouseEvent *event ) override
    {
      receivedPressEvent  = true;
      QCOMPARE( event->layoutPoint().x(), 8.0 );
      QCOMPARE( event->layoutPoint().y(), 6.0 );
    }

    bool receivedReleaseEvent = false;
    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override
    {
      receivedReleaseEvent  = true;
      QCOMPARE( event->layoutPoint().x(), 8.0 );
      QCOMPARE( event->layoutPoint().y(), 6.0 );
    }

    bool receivedWheelEvent = false;
    void wheelEvent( QWheelEvent * ) override
    {
      receivedWheelEvent = true;
    }

    bool receivedKeyPressEvent = false;
    void keyPressEvent( QKeyEvent * ) override
    {
      receivedKeyPressEvent  = true;
    }

    bool receivedKeyReleaseEvent = false;
    void keyReleaseEvent( QKeyEvent * ) override
    {
      receivedKeyReleaseEvent = true;
    }
};

void TestQgsLayoutView::events()
{
  QgsProject p;
  QgsLayoutView *view = new QgsLayoutView();
  QgsLayout *layout = new QgsLayout( &p );
  view->setCurrentLayout( layout );
  layout->setSceneRect( 0, 0, 1000, 1000 );
  view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  view->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  view->setFrameStyle( 0 );
  view->resize( 100, 100 );
  view->setFixedSize( 100, 100 );
  QCOMPARE( view->width(), 100 );
  QCOMPARE( view->height(), 100 );

  QTransform transform;
  transform.scale( 10, 10 );
  view->setTransform( transform );

  LoggingTool *tool = new LoggingTool( view );
  view->setTool( tool );

  const QPointF point( 80, 60 );
  QMouseEvent press( QEvent::MouseButtonPress, point,
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QMouseEvent move( QEvent::MouseMove, point,
                    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QMouseEvent releases( QEvent::MouseButtonRelease, point,
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QMouseEvent dblClick( QEvent::MouseButtonDblClick, point,
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QWheelEvent wheelEvent( point, QPointF(), QPoint( 0, 10 ), QPoint( 0, 10 ),
                          Qt::LeftButton, Qt::NoModifier, Qt::NoScrollPhase, false );
  QKeyEvent keyPress( QEvent::KeyPress, 10, Qt::NoModifier );
  QKeyEvent keyRelease( QEvent::KeyRelease, 10, Qt::NoModifier );

  view->mouseMoveEvent( &move );
  QVERIFY( tool->receivedMoveEvent );
  view->mousePressEvent( &press );
  QVERIFY( tool->receivedPressEvent );
  view->mouseReleaseEvent( &releases );
  QVERIFY( tool->receivedReleaseEvent );
  view->mouseDoubleClickEvent( &dblClick );
  QVERIFY( tool->receivedDoubleClickEvent );
  view->wheelEvent( &wheelEvent );
  QVERIFY( tool->receivedWheelEvent );
  view->keyPressEvent( &keyPress );
  QVERIFY( tool->receivedKeyPressEvent );
  view->keyReleaseEvent( &keyRelease );
  QVERIFY( tool->receivedKeyReleaseEvent );
}

//simple item for testing, since some methods in QgsLayoutItem are pure virtual
class TestItem : public QgsLayoutItem // clazy:exclude=missing-qobject-macro
{
  public:

    TestItem( QgsLayout *layout ) : QgsLayoutItem( layout ) {}

    int mFlag = 0;

    //implement pure virtual methods
    int type() const override { return QgsLayoutItemRegistry::LayoutItem + 101; }
    void draw( QgsLayoutItemRenderContext & ) override
    {    }
};

void TestQgsLayoutView::guiRegistry()
{
  // test QgsLayoutItemGuiRegistry
  QgsLayoutItemGuiRegistry registry;

  // empty registry
  QVERIFY( !registry.itemMetadata( -1 ) );
  QVERIFY( registry.itemMetadataIds().isEmpty() );
  QCOMPARE( registry.metadataIdForItemType( 0 ), -1 );
  QVERIFY( !registry.createItemWidget( nullptr ) );
  QVERIFY( !registry.createItemWidget( nullptr ) );
  const std::unique_ptr< TestItem > testItem = std::make_unique< TestItem >( nullptr );
  QVERIFY( !registry.createItemWidget( testItem.get() ) ); // not in registry

  const QSignalSpy spyTypeAdded( &registry, &QgsLayoutItemGuiRegistry::typeAdded );

  // add a dummy item to registry
  auto createWidget = []( QgsLayoutItem * item )->QgsLayoutItemBaseWidget *
  {
    return new QgsLayoutItemBaseWidget( nullptr, item );
  };

  auto createRubberBand = []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewRectangularRubberBand( view );
  };

  QgsLayoutItemGuiMetadata *metadata = new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutItem + 101, QStringLiteral( "mytype" ), QIcon(), createWidget, createRubberBand );
  QVERIFY( registry.addLayoutItemGuiMetadata( metadata ) );
  QCOMPARE( spyTypeAdded.count(), 1 );
  int uuid = registry.itemMetadataIds().value( 0 );
  QCOMPARE( spyTypeAdded.value( 0 ).at( 0 ).toInt(), uuid );
  QCOMPARE( registry.metadataIdForItemType( QgsLayoutItemRegistry::LayoutItem + 101 ), uuid );

  // duplicate type id is allowed
  metadata = new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutItem + 101, QStringLiteral( "mytype" ), QIcon(), createWidget, createRubberBand );
  QVERIFY( registry.addLayoutItemGuiMetadata( metadata ) );
  QCOMPARE( spyTypeAdded.count(), 2 );
  //retrieve metadata
  QVERIFY( !registry.itemMetadata( -1 ) );
  QCOMPARE( registry.itemMetadataIds().count(), 2 );
  QCOMPARE( registry.metadataIdForItemType( QgsLayoutItemRegistry::LayoutItem + 101 ), uuid );

  QVERIFY( registry.itemMetadata( uuid ) );
  QCOMPARE( registry.itemMetadata( uuid )->visibleName(), QStringLiteral( "mytype" ) );

  QWidget *widget = registry.createItemWidget( testItem.get() );
  QVERIFY( widget );
  delete widget;

  QgsLayoutView *view = new QgsLayoutView();
  //should use metadata's method
  QgsLayoutViewRubberBand *band = registry.createItemRubberBand( uuid, view );
  QVERIFY( band );
  QVERIFY( dynamic_cast< QgsLayoutViewRectangularRubberBand * >( band ) );
  QCOMPARE( band->view(), view );
  delete band;

  // groups
  QVERIFY( registry.addItemGroup( QgsLayoutItemGuiGroup( QStringLiteral( "g1" ) ) ) );
  QCOMPARE( registry.itemGroup( QStringLiteral( "g1" ) ).id, QStringLiteral( "g1" ) );
  // can't add duplicate group
  QVERIFY( !registry.addItemGroup( QgsLayoutItemGuiGroup( QStringLiteral( "g1" ) ) ) );

  //creating item
  QgsLayoutItem *item = registry.createItem( uuid, nullptr );
  QVERIFY( !item );
  QgsApplication::layoutItemRegistry()->addLayoutItemType( new QgsLayoutItemMetadata( QgsLayoutItemRegistry::LayoutItem + 101, QStringLiteral( "my type" ), QStringLiteral( "my types" ), []( QgsLayout * layout )->QgsLayoutItem*
  {
    return new TestItem( layout );
  } ) );

  item = registry.createItem( uuid, nullptr );
  QVERIFY( item );
  QCOMPARE( item->type(), QgsLayoutItemRegistry::LayoutItem + 101 );
  QCOMPARE( static_cast< TestItem * >( item )->mFlag, 0 );
  delete item;

  // override create func
  metadata = new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutItem + 101, QStringLiteral( "mytype" ), QIcon(), createWidget, createRubberBand );
  metadata->setItemCreationFunction( []( QgsLayout * layout )->QgsLayoutItem*
  {
    TestItem *item = new TestItem( layout );
    item->mFlag = 2;
    return item;
  } );
  QVERIFY( registry.addLayoutItemGuiMetadata( metadata ) );
  uuid = spyTypeAdded.at( spyTypeAdded.count() - 1 ).at( 0 ).toInt();
  item = registry.createItem( uuid, nullptr );
  QVERIFY( item );
  QCOMPARE( item->type(), QgsLayoutItemRegistry::LayoutItem + 101 );
  QCOMPARE( static_cast< TestItem * >( item )->mFlag, 2 );
  delete item;

}

void TestQgsLayoutView::rubberBand()
{
  QgsLayoutViewRectangularRubberBand band;
  band.setBrush( QBrush( QColor( 255, 0, 0 ) ) );
  QCOMPARE( band.brush().color(), QColor( 255, 0, 0 ) );
  band.setPen( QPen( QColor( 0, 255, 0 ) ) );
  QCOMPARE( band.pen().color(), QColor( 0, 255, 0 ) );
}

QGSTEST_MAIN( TestQgsLayoutView )
#include "testqgslayoutview.moc"
