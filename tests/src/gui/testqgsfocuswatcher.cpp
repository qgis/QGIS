/***************************************************************************
    testqgsfocuswatcher.cpp
     ----------------------
    Date                 : April 2016
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

#include "qgsfocuswatcher.h"
#include <QApplication>
#include <QLineEdit>

class TestQgsFocusWatcher: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testSignals();

  private:

};

void TestQgsFocusWatcher::initTestCase()
{
}

void TestQgsFocusWatcher::cleanupTestCase()
{
}

void TestQgsFocusWatcher::init()
{
}

void TestQgsFocusWatcher::cleanup()
{
}

void TestQgsFocusWatcher::testSignals()
{
  QWidget* w = new QWidget();
  QLineEdit* e1 = new QLineEdit( w );
  QLineEdit* e2 = new QLineEdit( w );
  QApplication::setActiveWindow( w ); //required for focus events
  e1->setFocus();

  QgsFocusWatcher* watcher1 = new QgsFocusWatcher( e1 );
  QgsFocusWatcher* watcher2 = new QgsFocusWatcher( e2 );

  QSignalSpy spyFocusIn1( watcher1, SIGNAL( focusIn() ) );
  QSignalSpy spyFocusOut1( watcher1, SIGNAL( focusOut() ) );
  QSignalSpy spyFocusChanged1( watcher1, SIGNAL( focusChanged( bool ) ) );
  QSignalSpy spyFocusIn2( watcher2, SIGNAL( focusIn() ) );
  QSignalSpy spyFocusOut2( watcher2, SIGNAL( focusOut() ) );
  QSignalSpy spyFocusChanged2( watcher2, SIGNAL( focusChanged( bool ) ) );

  e2->setFocus();
  QCOMPARE( spyFocusIn1.count(), 0 );
  QCOMPARE( spyFocusOut1.count(), 1 );
  QCOMPARE( spyFocusChanged1.count(), 1 );
  QCOMPARE( spyFocusChanged1.last().at( 0 ).toBool(), false );
  QCOMPARE( spyFocusIn2.count(), 1 );
  QCOMPARE( spyFocusOut2.count(), 0 );
  QCOMPARE( spyFocusChanged2.count(), 1 );
  QCOMPARE( spyFocusChanged2.last().at( 0 ).toBool(), true );

  e1->setFocus();
  QCOMPARE( spyFocusIn1.count(), 1 );
  QCOMPARE( spyFocusOut1.count(), 1 );
  QCOMPARE( spyFocusChanged1.count(), 2 );
  QCOMPARE( spyFocusChanged1.last().at( 0 ).toBool(), true );
  QCOMPARE( spyFocusIn2.count(), 1 );
  QCOMPARE( spyFocusOut2.count(), 1 );
  QCOMPARE( spyFocusChanged2.count(), 2 );
  QCOMPARE( spyFocusChanged2.last().at( 0 ).toBool(), false );

  delete w;
}


QTEST_MAIN( TestQgsFocusWatcher )
#include "testqgsfocuswatcher.moc"
