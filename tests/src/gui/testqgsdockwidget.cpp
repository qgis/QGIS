/***************************************************************************
    testqgsdockwidget.cpp
     ----------------------
    Date                 : June 2016
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtTest/QtTest>

#include "qgsdockwidget.h"
#include <QApplication>
#include <QMainWindow>

class TestQgsDockWidget: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testSignals();
    void testUserVisible();
    void testSetUserVisible();

  private:

};

void TestQgsDockWidget::initTestCase()
{
}

void TestQgsDockWidget::cleanupTestCase()
{
}

void TestQgsDockWidget::init()
{
}

void TestQgsDockWidget::cleanup()
{
}

void TestQgsDockWidget::testSignals()
{
  QWidget* w = new QWidget();
  QApplication::setActiveWindow( w ); //required for focus events
  QgsDockWidget* d = new QgsDockWidget( w );

  QSignalSpy spyClosedStateChanged( d, SIGNAL( closedStateChanged( bool ) ) );
  QSignalSpy spyClosed( d, SIGNAL( closed() ) );
  QSignalSpy spyOpenedStateChanged( d, SIGNAL( openedStateChanged( bool ) ) );
  QSignalSpy spyOpened( d, SIGNAL( opened() ) );

  w->show();

  d->show();
  QCOMPARE( spyClosedStateChanged.count(), 1 );
  QCOMPARE( spyClosedStateChanged.last().at( 0 ).toBool(), false );
  QCOMPARE( spyOpenedStateChanged.count(), 1 );
  QCOMPARE( spyOpenedStateChanged.last().at( 0 ).toBool(), true );
  QCOMPARE( spyClosed.count(), 0 );
  QCOMPARE( spyOpened.count(), 1 );

  d->close();
  QCOMPARE( spyClosedStateChanged.count(), 2 );
  QCOMPARE( spyClosedStateChanged.last().at( 0 ).toBool(), true );
  QCOMPARE( spyOpenedStateChanged.count(), 2 );
  QCOMPARE( spyOpenedStateChanged.last().at( 0 ).toBool(), false );
  QCOMPARE( spyClosed.count(), 1 );
  QCOMPARE( spyOpened.count(), 1 );

  delete w;
}

void TestQgsDockWidget::testUserVisible()
{
  QgsDockWidget* w = new QgsDockWidget();
  QVERIFY( !w->isUserVisible() );

  w->show();
  QVERIFY( w->isUserVisible() );

  w->hide();
  QVERIFY( !w->isUserVisible() );
  delete w;
}

void TestQgsDockWidget::testSetUserVisible()
{
  QMainWindow* w = new QMainWindow();
  QApplication::setActiveWindow( w ); //required for focus events
  QgsDockWidget* d1 = new QgsDockWidget( w );
  QgsDockWidget* d2 = new QgsDockWidget( w );
  w->addDockWidget( Qt::RightDockWidgetArea, d1 );
  w->addDockWidget( Qt::RightDockWidgetArea, d2 );
  w->tabifyDockWidget( d1, d2 );
  w->show();

  QVERIFY( d2->isUserVisible() );
  QVERIFY( !d1->isUserVisible() );

  // showing dock widgets

  // already visible
  d2->setUserVisible( true );
  QVERIFY( d2->isUserVisible() );
  QVERIFY( d2->isVisible() );
  QVERIFY( !d1->isUserVisible() );
  QVERIFY( d1->isVisible() );

  // visible, but hidden by other dock
  d1->setUserVisible( true );
  QVERIFY( !d2->isUserVisible() );
  QVERIFY( d2->isVisible() );
  QVERIFY( d1->isUserVisible() );
  QVERIFY( d1->isVisible() );

  // hidden
  d2->hide();
  d2->setUserVisible( true );
  QVERIFY( d2->isUserVisible() );
  QVERIFY( d2->isVisible() );
  QVERIFY( !d1->isUserVisible() );
  QVERIFY( d1->isVisible() );

  // hiding dock widgets

  // already hidden by other tab
  d1->setUserVisible( false );
  QVERIFY( d2->isUserVisible() );
  QVERIFY( d2->isVisible() );
  QVERIFY( !d1->isUserVisible() );
  QVERIFY( d1->isVisible() );

  // already hidden
  d2->hide();
  d1->raise(); //shouldn't be necessary outside of tests
  QVERIFY( !d2->isUserVisible() );
  QVERIFY( !d2->isVisible() );
  QVERIFY( d1->isUserVisible() );
  QVERIFY( d1->isVisible() );

  d2->setUserVisible( false );
  QVERIFY( !d2->isUserVisible() );
  QVERIFY( !d2->isVisible() );
  QVERIFY( d1->isUserVisible() );
  QVERIFY( d1->isVisible() );

  // setting active dock as not user visible should hide it
  d2->show();
  d1->raise();
  QVERIFY( !d2->isUserVisible() );
  QVERIFY( d2->isVisible() );
  QVERIFY( d1->isUserVisible() );
  QVERIFY( d1->isVisible() );

  d1->setUserVisible( false );
  QVERIFY( d2->isVisible() );
  QVERIFY( !d1->isUserVisible() );
  QVERIFY( !d1->isVisible() );

  delete w;

}

QTEST_MAIN( TestQgsDockWidget )
#include "testqgsdockwidget.moc"
