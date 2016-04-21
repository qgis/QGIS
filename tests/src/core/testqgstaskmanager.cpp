/***************************************************************************
                         testqgstaskmanager.cpp
                         ----------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstaskmanager.h"
#include <QObject>
#include <QSharedPointer>
#include <QtTest/QtTest>

class TestTask : public QgsTask
{
    Q_OBJECT

  public:

    TestTask( const QString& desc = QString() ) : QgsTask( desc ), runCalled( false ) {}
    TestTask( const QString& desc, const QgsTask::Flags& flags ) : QgsTask( desc, flags ), runCalled( false ) {}

    void emitProgressChanged( double progress ) { setProgress( progress ); }
    void emitTaskStopped() { stopped(); }
    void emitTaskCompleted() { completed(); }

    bool runCalled;

  protected:

    void run() override
    {
      runCalled = true;
    }

};

class TestTerminationTask : public TestTask
{
    Q_OBJECT

  public:

    ~TestTerminationTask()
    {
      //make sure task has been terminated by manager prior to deletion
      Q_ASSERT( status() == QgsTask::Terminated );
    }

  protected:

    void run() override
    {
      while ( !isCancelled() )
        {}
      stopped();
    }
};


class TestQgsTaskManager : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void task();
    void createInstance();
    void addTask();
    void deleteTask();
    void taskTerminationBeforeDelete();
    void taskId();
    void progressChanged();
    void statusChanged();
    void holdTask();
    void dependancies();

  private:

};

void TestQgsTaskManager::initTestCase()
{

}

void TestQgsTaskManager::cleanupTestCase()
{

}

void TestQgsTaskManager::init()
{

}

void TestQgsTaskManager::cleanup()
{

}

void TestQgsTaskManager::task()
{
  QScopedPointer< TestTask > task( new TestTask( "desc" ) );
  QCOMPARE( task->status(), QgsTask::Queued );
  QCOMPARE( task->description(), QString( "desc" ) );
  QVERIFY( !task->isActive() );
  QVERIFY( task->canCancel() );
  QVERIFY( task->flags() & QgsTask::CanReportProgress );

  QSignalSpy startedSpy( task.data(), SIGNAL( begun() ) );
  QSignalSpy statusSpy( task.data(), SIGNAL( statusChanged( int ) ) );

  task->start();
  QCOMPARE( task->status(), QgsTask::Running );
  QVERIFY( task->isActive() );
  QVERIFY( task->runCalled );
  QCOMPARE( startedSpy.count(), 1 );
  QCOMPARE( statusSpy.count(), 1 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.last().at( 0 ).toInt() ), QgsTask::Running );

  //test that calling stopped sets correct state
  QSignalSpy stoppedSpy( task.data(), SIGNAL( taskStopped() ) );
  task->emitTaskStopped();
  QCOMPARE( task->status(), QgsTask::Terminated );
  QVERIFY( !task->isActive() );
  QCOMPARE( stoppedSpy.count(), 1 );
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.last().at( 0 ).toInt() ), QgsTask::Terminated );

  //test that calling completed sets correct state
  task.reset( new TestTask() );
  QSignalSpy completeSpy( task.data(), SIGNAL( taskCompleted() ) );
  QSignalSpy statusSpy2( task.data(), SIGNAL( statusChanged( int ) ) );
  task->emitTaskCompleted();
  QCOMPARE( task->status(), QgsTask::Complete );
  QVERIFY( !task->isActive() );
  QCOMPARE( completeSpy.count(), 1 );
  QCOMPARE( statusSpy2.count(), 1 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy2.last().at( 0 ).toInt() ), QgsTask::Complete );

  // test that cancelling tasks which have not begin immediately ends them
  task.reset( new TestTask() );
  task->cancel(); // Queued task
  QCOMPARE( task->status(), QgsTask::Terminated );
  task.reset( new TestTask() );
  task->hold(); // OnHold task
  task->cancel();
  QCOMPARE( task->status(), QgsTask::Terminated );

  // test flags
  task.reset( new TestTask( "desc", QgsTask::CanReportProgress ) );
  QVERIFY( !task->canCancel() );
  QVERIFY( task->flags() & QgsTask::CanReportProgress );
  QVERIFY( !( task->flags() & QgsTask::CanCancel ) );
  task.reset( new TestTask( "desc", QgsTask::CanCancel ) );
  QVERIFY( task->canCancel() );
  QVERIFY( !( task->flags() & QgsTask::CanReportProgress ) );
  QVERIFY( task->flags() & QgsTask::CanCancel );
}


void TestQgsTaskManager::createInstance()
{
  QgsTaskManager* manager = QgsTaskManager::instance();
  QVERIFY( manager );
}

void TestQgsTaskManager::addTask()
{
  //create an empty manager
  QgsTaskManager manager;

  //should be empty
  QVERIFY( manager.tasks().isEmpty() );
  QCOMPARE( manager.count(), 0 );
  QVERIFY( !manager.task( 0L ) );

  QSignalSpy spy( &manager, SIGNAL( taskAdded( long ) ) );

  //add a task
  TestTask* task = new TestTask();
  long id = manager.addTask( task );
  QCOMPARE( id, 0L );
  QCOMPARE( manager.tasks().count(), 1 );
  QCOMPARE( manager.count(), 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 0LL );

  //retrieve task
  QCOMPARE( manager.task( 0L ), task );
  QCOMPARE( manager.tasks().at( 0 ), task );

  //add a second task
  TestTask* task2 = new TestTask();
  id = manager.addTask( task2 );
  QCOMPARE( id, 1L );
  QCOMPARE( manager.tasks().count(), 2 );
  QCOMPARE( manager.count(), 2 );
  QCOMPARE( manager.task( 0L ), task );
  QCOMPARE( manager.tasks().at( 0 ), task );
  QCOMPARE( manager.task( 1L ), task2 );
  QCOMPARE( manager.tasks().at( 1 ), task2 );

  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
}

void TestQgsTaskManager::deleteTask()
{
  //create manager with some tasks
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();
  TestTask* task3 = new TestTask();
  manager.addTask( task );
  manager.addTask( task2 );
  manager.addTask( task3 );

  QSignalSpy spy( &manager, SIGNAL( taskAboutToBeDeleted( long ) ) );

  //try deleting a non-existant task
  QVERIFY( !manager.deleteTask( 56 ) );
  QCOMPARE( spy.count(), 0 );

  //try deleting a task by ID
  QVERIFY( manager.deleteTask( 1 ) );
  QCOMPARE( manager.tasks().count(), 2 );
  QVERIFY( !manager.task( 1 ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );

  //can't delete twice
  QVERIFY( !manager.deleteTask( 1 ) );
  QCOMPARE( spy.count(), 1 );

  //delete task by reference
  QVERIFY( manager.deleteTask( task ) );
  QCOMPARE( manager.tasks().count(), 1 );
  QVERIFY( !manager.task( 0 ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 0LL );
}

void TestQgsTaskManager::taskTerminationBeforeDelete()
{
  //test that task is terminated by manager prior to delete
  QgsTaskManager* manager = new QgsTaskManager();

  //TestTerminationTask will assert that it's been terminated prior to deletion
  TestTask* task = new TestTerminationTask();
  manager->addTask( task );

  // wait till task spins up
  while ( !task->isActive() )
    {}

  // if task is not terminated assert will trip
  delete manager;
  QApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
}

void TestQgsTaskManager::taskId()
{
  //test finding task IDs

  //create manager with some tasks
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();
  manager.addTask( task );
  manager.addTask( task2 );

  //also a task not in the manager
  TestTask* task3 = new TestTask();

  QCOMPARE( manager.taskId( nullptr ), -1L );
  QCOMPARE( manager.taskId( task ), 0L );
  QCOMPARE( manager.taskId( task2 ), 1L );
  QCOMPARE( manager.taskId( task3 ), -1L );

  delete task3;
}

void TestQgsTaskManager::progressChanged()
{
  // check that progressChanged signals emitted by tasks result in progressChanged signal from manager
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();
  manager.addTask( task );
  manager.addTask( task2 );

  QSignalSpy spy( &manager, SIGNAL( progressChanged( long, double ) ) );

  task->emitProgressChanged( 50.0 );
  QCOMPARE( task->progress(), 50.0 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( spy.last().at( 1 ).toDouble(), 50.0 );

  task2->emitProgressChanged( 75.0 );
  QCOMPARE( task2->progress(), 75.0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( spy.last().at( 1 ).toDouble(), 75.0 );
}

void TestQgsTaskManager::statusChanged()
{
  // check that statusChanged signals emitted by tasks result in statusChanged signal from manager
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();
  manager.addTask( task );
  manager.addTask( task2 );

  QSignalSpy spy( &manager, SIGNAL( statusChanged( long, int ) ) );

  task->start();
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( spy.last().at( 1 ).toInt() ), QgsTask::Running );

  task->emitTaskStopped();
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( spy.last().at( 1 ).toInt() ), QgsTask::Terminated );

  task2->emitTaskCompleted();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( spy.last().at( 1 ).toInt() ), QgsTask::Complete );
}

void TestQgsTaskManager::holdTask()
{
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  //hold task
  task->hold();
  manager.addTask( task );
  //should not be started
  QCOMPARE( task->status(), QgsTask::OnHold );

  task->unhold();
  // wait for task to spin up
  while ( task->status() == QgsTask::Queued ) {}
  QCOMPARE( task->status(), QgsTask::Running );
}

void TestQgsTaskManager::dependancies()
{
  QgsTaskManager manager;

  //test that cancelling tasks cancels all tasks which are dependant on them
  TestTask* task = new TestTask();
  task->hold();
  TestTask* childTask = new TestTask();
  childTask->hold();
  TestTask* grandChildTask = new TestTask();
  grandChildTask->hold();

  manager.addTask( task, QgsTaskList() << childTask );
  manager.addTask( childTask, QgsTaskList() << grandChildTask );
  manager.addTask( grandChildTask );

  grandChildTask->cancel();
  QCOMPARE( childTask->status(), QgsTask::Terminated );
  QCOMPARE( task->status(), QgsTask::Terminated );

  // test that tasks are queued until dependancies are resolved
  task = new TestTask();
  childTask = new TestTask();
  childTask->hold();
  long taskId = manager.addTask( task, QgsTaskList() << childTask );
  long childTaskId = manager.addTask( childTask );
  QVERIFY( !manager.dependenciesSatisified( taskId ) );
  QVERIFY( manager.dependenciesSatisified( childTaskId ) );

  QCOMPARE( childTask->status(), QgsTask::OnHold );
  QCOMPARE( task->status(), QgsTask::Queued );

  childTask->unhold();
  //wait for childTask to spin up
  while ( !childTask->isActive() ) {}
  QCOMPARE( childTask->status(), QgsTask::Running );
  QCOMPARE( task->status(), QgsTask::Queued );
  childTask->emitTaskCompleted();
  //wait for childTask to complete
  while ( childTask->isActive() ) {}
  QVERIFY( manager.dependenciesSatisified( taskId ) );
  QCOMPARE( childTask->status(), QgsTask::Complete );
  //wait for task to spin up
  while ( !task->isActive() ) {}
  QCOMPARE( task->status(), QgsTask::Running );
}


QTEST_MAIN( TestQgsTaskManager )
#include "testqgstaskmanager.moc"
