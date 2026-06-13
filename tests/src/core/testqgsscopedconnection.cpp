/***************************************************************************
  testqgsscopedconnection.cpp
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscopedconnection.h"
#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

class TestQgsScopedConnection : public QObject
{
    Q_OBJECT

  private slots:
    void testBasic();
    void testManualDisconnect();
    void testMove();
    void testMoveAssignment();
};

void TestQgsScopedConnection::testBasic()
{
  QAction action;
  int triggerCount = 0;
  {
    // NOTE -- scope of connection is "this", we want the connection NOT to be automatically
    // cleaned up by Qt itself.
    QgsScopedConnection guard( QObject::connect( &action, &QAction::triggered, this, [&]() { triggerCount++; } ) );

    // connection is active
    action.trigger();
    QCOMPARE( triggerCount, 1 );
  }

  // connection should be disconnected, no increment
  action.trigger();
  QCOMPARE( triggerCount, 1 );
}

void TestQgsScopedConnection::testManualDisconnect()
{
  QAction action;
  int triggerCount = 0;
  {
    // NOTE -- scope of connection is "this", we want the connection NOT to be automatically
    // cleaned up by Qt itself.
    QgsScopedConnection guard( QObject::connect( &action, &QAction::triggered, this, [&]() { triggerCount++; } ) );

    // connection is active
    action.trigger();
    QCOMPARE( triggerCount, 1 );

    guard.disconnect();

    // should have been disconnected, no increment
    action.trigger();
    QCOMPARE( triggerCount, 1 );
  }

  // connection should be disconnected, no increment
  action.trigger();
  QCOMPARE( triggerCount, 1 );
}

void TestQgsScopedConnection::testMove()
{
  QAction action( nullptr );
  int triggerCount = 0;

  QgsScopedConnection outerGuard;
  {
    QgsScopedConnection innerGuard( QObject::connect( &action, &QAction::triggered, [&]() { triggerCount++; } ) );

    action.trigger();
    QCOMPARE( triggerCount, 1 );

    // move the connection out of the inner scope
    outerGuard = QgsScopedConnection( std::move( innerGuard ) );
  }

  // innerGuard is destructed, but outerGuard should be managing the connection now
  action.trigger();
  QCOMPARE( triggerCount, 2 );

  outerGuard.disconnect();
  // connection should be disconnected, no increment
  action.trigger();
  QCOMPARE( triggerCount, 2 );
}

void TestQgsScopedConnection::testMoveAssignment()
{
  QAction action( nullptr );
  int triggerCount = 0;

  QgsScopedConnection outerGuard;
  {
    QgsScopedConnection innerGuard( QObject::connect( &action, &QAction::triggered, [&]() { triggerCount++; } ) );

    action.trigger();
    QCOMPARE( triggerCount, 1 );

    // move the connection out of the inner scope
    outerGuard = std::move( innerGuard );
  }

  // innerGuard is destructed, but outerGuard should be managing the connection now
  action.trigger();
  QCOMPARE( triggerCount, 2 );

  outerGuard.disconnect();
  // connection should be disconnected, no increment
  action.trigger();
  QCOMPARE( triggerCount, 2 );
}

QGSTEST_MAIN( TestQgsScopedConnection )
#include "testqgsscopedconnection.moc"
