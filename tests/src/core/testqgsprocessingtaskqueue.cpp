/***************************************************************************
                         testqgsprocessingtaskqueue.cpp
                         ------------------------------
    begin                : December 2024
    copyright            : (C) 2024 by Nassim Lanckmann
    email                : nassim dot lanckmann at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingtaskqueue.h"
#include "qgstest.h"

#include <QObject>
#include <QSignalSpy>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsProcessingTaskQueue : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void testSingleton();
    void testAddTask();
    void testRemoveTask();
    void testMoveTaskUp();
    void testMoveTaskDown();
    void testClear();
    void testQueueChanged();
    void testTaskProperties();

  private:
    QgsProcessingTaskQueue *mQueue = nullptr;
};

void TestQgsProcessingTaskQueue::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsProcessingTaskQueue::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingTaskQueue::init()
{
  mQueue = QgsProcessingTaskQueue::instance();
  mQueue->clear();
}

void TestQgsProcessingTaskQueue::cleanup()
{}

void TestQgsProcessingTaskQueue::testSingleton()
{
  QgsProcessingTaskQueue *queue1 = QgsProcessingTaskQueue::instance();
  QgsProcessingTaskQueue *queue2 = QgsProcessingTaskQueue::instance();
  QCOMPARE( queue1, queue2 );
}

void TestQgsProcessingTaskQueue::testAddTask()
{
  QVERIFY( mQueue->isEmpty() );
  QCOMPARE( mQueue->count(), 0 );

  QVariantMap params;
  params[u"INPUT"_s] = u"test"_s;

  mQueue->addTask( u"native:buffer"_s, params, u"Test task"_s );

  QVERIFY( !mQueue->isEmpty() );
  QCOMPARE( mQueue->count(), 1 );

  const QList<QgsProcessingQueuedTask> tasks = mQueue->tasks();
  QCOMPARE( tasks.count(), 1 );
  QCOMPARE( tasks.at( 0 ).algorithmId(), u"native:buffer"_s );
  QCOMPARE( tasks.at( 0 ).description(), u"Test task"_s );
  QCOMPARE( tasks.at( 0 ).parameters(), params );
}

void TestQgsProcessingTaskQueue::testRemoveTask()
{
  mQueue->addTask( u"native:buffer"_s, QVariantMap(), u"Task 1"_s );
  mQueue->addTask( u"native:clip"_s, QVariantMap(), u"Task 2"_s );
  mQueue->addTask( u"native:dissolve"_s, QVariantMap(), u"Task 3"_s );

  QCOMPARE( mQueue->count(), 3 );

  QVERIFY( mQueue->removeTask( 1 ) );
  QCOMPARE( mQueue->count(), 2 );

  const QList<QgsProcessingQueuedTask> tasks = mQueue->tasks();
  QCOMPARE( tasks.at( 0 ).description(), u"Task 1"_s );
  QCOMPARE( tasks.at( 1 ).description(), u"Task 3"_s );

  QVERIFY( !mQueue->removeTask( -1 ) );
  QVERIFY( !mQueue->removeTask( 10 ) );
  QCOMPARE( mQueue->count(), 2 );
}

void TestQgsProcessingTaskQueue::testMoveTaskUp()
{
  mQueue->addTask( u"native:buffer"_s, QVariantMap(), u"Task 1"_s );
  mQueue->addTask( u"native:clip"_s, QVariantMap(), u"Task 2"_s );
  mQueue->addTask( u"native:dissolve"_s, QVariantMap(), u"Task 3"_s );

  QVERIFY( mQueue->moveTaskUp( 1 ) );

  const QList<QgsProcessingQueuedTask> tasks = mQueue->tasks();
  QCOMPARE( tasks.at( 0 ).description(), u"Task 2"_s );
  QCOMPARE( tasks.at( 1 ).description(), u"Task 1"_s );
  QCOMPARE( tasks.at( 2 ).description(), u"Task 3"_s );

  QVERIFY( !mQueue->moveTaskUp( 0 ) );
  QVERIFY( !mQueue->moveTaskUp( -1 ) );
  QVERIFY( !mQueue->moveTaskUp( 10 ) );
}

void TestQgsProcessingTaskQueue::testMoveTaskDown()
{
  mQueue->addTask( u"native:buffer"_s, QVariantMap(), u"Task 1"_s );
  mQueue->addTask( u"native:clip"_s, QVariantMap(), u"Task 2"_s );
  mQueue->addTask( u"native:dissolve"_s, QVariantMap(), u"Task 3"_s );

  QVERIFY( mQueue->moveTaskDown( 0 ) );

  const QList<QgsProcessingQueuedTask> tasks = mQueue->tasks();
  QCOMPARE( tasks.at( 0 ).description(), u"Task 2"_s );
  QCOMPARE( tasks.at( 1 ).description(), u"Task 1"_s );
  QCOMPARE( tasks.at( 2 ).description(), u"Task 3"_s );

  QVERIFY( !mQueue->moveTaskDown( 2 ) );
  QVERIFY( !mQueue->moveTaskDown( -1 ) );
  QVERIFY( !mQueue->moveTaskDown( 10 ) );
}

void TestQgsProcessingTaskQueue::testClear()
{
  mQueue->addTask( u"native:buffer"_s, QVariantMap(), u"Task 1"_s );
  mQueue->addTask( u"native:clip"_s, QVariantMap(), u"Task 2"_s );
  mQueue->addTask( u"native:dissolve"_s, QVariantMap(), u"Task 3"_s );

  QCOMPARE( mQueue->count(), 3 );
  QVERIFY( !mQueue->isEmpty() );

  mQueue->clear();

  QCOMPARE( mQueue->count(), 0 );
  QVERIFY( mQueue->isEmpty() );
}

void TestQgsProcessingTaskQueue::testQueueChanged()
{
  QSignalSpy spy( mQueue, &QgsProcessingTaskQueue::queueChanged );

  mQueue->addTask( u"native:buffer"_s, QVariantMap(), u"Task 1"_s );
  QCOMPARE( spy.count(), 1 );

  mQueue->addTask( u"native:clip"_s, QVariantMap(), u"Task 2"_s );
  QCOMPARE( spy.count(), 2 );

  mQueue->removeTask( 0 );
  QCOMPARE( spy.count(), 3 );

  mQueue->moveTaskUp( 0 );
  QCOMPARE( spy.count(), 3 );

  mQueue->addTask( u"native:dissolve"_s, QVariantMap(), u"Task 3"_s );
  QCOMPARE( spy.count(), 4 );

  mQueue->moveTaskDown( 0 );
  QCOMPARE( spy.count(), 5 );

  mQueue->clear();
  QCOMPARE( spy.count(), 6 );
}

void TestQgsProcessingTaskQueue::testTaskProperties()
{
  QVariantMap params;
  params[u"INPUT"_s] = u"layer1"_s;
  params[u"DISTANCE"_s] = 10.0;

  const QgsProcessingQueuedTask task( u"native:buffer"_s, params, u"Buffer task"_s );

  QCOMPARE( task.algorithmId(), u"native:buffer"_s );
  QCOMPARE( task.description(), u"Buffer task"_s );
  QCOMPARE( task.parameters(), params );
  QCOMPARE( task.parameters().value( u"INPUT"_s ).toString(), u"layer1"_s );
  QCOMPARE( task.parameters().value( u"DISTANCE"_s ).toDouble(), 10.0 );
}

QGSTEST_MAIN( TestQgsProcessingTaskQueue )
#include "testqgsprocessingtaskqueue.moc"
