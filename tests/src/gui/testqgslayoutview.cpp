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
#include "qgslayout.h"
#include "qgslayoutview.h"
#include "qgslayoutviewtool.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutitem.h"
#include "qgslayoutviewrubberband.h"
#include "qgslayoutitemregistryguiutils.h"
#include "qgstestutils.h"
#include <QtTest/QSignalSpy>

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
    void registryUtils();
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
  QgsLayout *layout = new QgsLayout();
  QgsLayoutView *view = new QgsLayoutView();

  QSignalSpy spyLayoutChanged( view, &QgsLayoutView::layoutSet );
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

  QSignalSpy spySetTool( view, &QgsLayoutView::toolSet );
  QSignalSpy spyToolActivated( tool, &QgsLayoutViewTool::activated );
  QSignalSpy spyToolActivated2( tool2, &QgsLayoutViewTool::activated );
  QSignalSpy spyToolDeactivated( tool, &QgsLayoutViewTool::deactivated );
  QSignalSpy spyToolDeactivated2( tool2, &QgsLayoutViewTool::deactivated );
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

class LoggingTool : public QgsLayoutViewTool
{
  public:

    LoggingTool( QgsLayoutView *view )
      : QgsLayoutViewTool( view, "logging" )
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
  QgsLayoutView *view = new QgsLayoutView();
  QgsLayout *layout = new QgsLayout();
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

  QPointF point( 80, 60 );
  QMouseEvent press( QEvent::MouseButtonPress, point,
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QMouseEvent move( QEvent::MouseMove, point,
                    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QMouseEvent releases( QEvent::MouseButtonRelease, point,
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QMouseEvent dblClick( QEvent::MouseButtonDblClick, point,
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QWheelEvent wheelEvent( point, 10,
                          Qt::LeftButton, Qt::NoModifier );
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
class TestItem : public QgsLayoutItem
{
  public:

    TestItem( QgsLayout *layout ) : QgsLayoutItem( layout ) {}
    ~TestItem() {}

    //implement pure virtual methods
    int type() const override { return QgsLayoutItemRegistry::LayoutItem + 101; }
    void draw( QPainter *, const QStyleOptionGraphicsItem *, QWidget * ) override
    {    }
};

QgsLayout *mLayout = nullptr;
QString mReport;

bool renderCheck( QString testName, QImage &image, int mismatchCount );

void TestQgsLayoutView::registryUtils()
{
  // add a dummy item to registry
  auto create = []( QgsLayout * layout, const QVariantMap & )->QgsLayoutItem*
  {
    return new TestItem( layout );
  };

  auto createRubberBand = []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewRectangularRubberBand( view );
  };

  QgsLayoutItemMetadata *metadata = new QgsLayoutItemMetadata( 2, QStringLiteral( "my type" ), QIcon(), create );
  metadata->setRubberBandCreationFunction( createRubberBand );
  QVERIFY( QgsApplication::layoutItemRegistry()->addLayoutItemType( metadata ) );

  QgsLayoutView *view = new QgsLayoutView();
  //should use metadata's method
  QgsLayoutViewRubberBand *band = QgsApplication::layoutItemRegistry()->createItemRubberBand( 2, view );
  QVERIFY( band );
  QVERIFY( dynamic_cast< QgsLayoutViewRectangularRubberBand * >( band ) );
  QCOMPARE( band->view(), view );
  delete band;

  //manually register a prototype
  QgsLayoutItemRegistryGuiUtils::setItemRubberBandPrototype( 2, new QgsLayoutViewEllipticalRubberBand() );
  band = QgsApplication::layoutItemRegistry()->createItemRubberBand( 2, view );
  QVERIFY( band );
  QVERIFY( dynamic_cast< QgsLayoutViewEllipticalRubberBand * >( band ) );
  QCOMPARE( band->view(), view );

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
