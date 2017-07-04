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

QGSTEST_MAIN( TestQgsLayoutView )
#include "testqgslayoutview.moc"
