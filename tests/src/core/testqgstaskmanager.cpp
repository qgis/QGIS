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
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include <QObject>
#include <QSharedPointer>
#include <QtTest/QtTest>

//enable to allow fragile tests which intermittently fail
#define WITH_FLAKY_TESTS

class TestTask : public QgsTask
{
    Q_OBJECT

  public:

    TestTask( const QString& desc = QString() ) : QgsTask( desc ), runCalled( false ) {}
    TestTask( const QString& desc, const QgsTask::Flags& flags ) : QgsTask( desc, flags ), runCalled( false ) {}

    void emitProgressChanged( double progress ) { setProgress( progress ); }
    void emitTaskStopped() { terminated(); }
    void emitTaskCompleted() { completed(); }

    bool runCalled;

  protected:

    TaskResult _run() override
    {
      runCalled = true;
      return ResultPending;
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

    TaskResult _run() override
    {
      while ( !isCancelled() )
        {}
      return ResultFail;
    }
};

class CancelableTask : public QgsTask
{
    Q_OBJECT

  public:

    ~CancelableTask()
    {
      int i = 1;
      i++;

    }

  protected:

    TaskResult _run() override
    {
      while ( !isCancelled() )
        {}
      return ResultSuccess;
    }
};

class SuccessTask : public QgsTask
{
    Q_OBJECT

  protected:

    TaskResult _run() override
    {
      return ResultSuccess;
    }
};

class FailTask : public QgsTask
{
    Q_OBJECT

  protected:

    TaskResult _run() override
    {
      return ResultFail;
    }

};

class FinishTask : public QgsTask
{
    Q_OBJECT

  public:

    FinishTask( TaskResult* result )
        : desiredResult( QgsTask::ResultPending )
        , resultObtained( result )
    {}

    TaskResult desiredResult;
    TaskResult* resultObtained;

  protected:

    TaskResult _run() override
    {
      return desiredResult;
    }

    void finished( TaskResult result ) override
    {
      Q_ASSERT( QApplication::instance()->thread() == QThread::currentThread() );
      *resultObtained = result;
    }
};

void flushEvents()
{
  for ( int i = 0; i < 1000; ++i )
  {
    QCoreApplication::processEvents();
  }
}

class TestQgsTaskManager : public QObject
{
    Q_OBJECT
  public:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void task();
    void taskResult();
    void taskFinished();
    void subTask();
    void addTask();

#ifdef WITH_FLAKY_TESTS
    void taskTerminationBeforeDelete();
#endif

    void taskId();
    void progressChanged();
#ifdef WITH_FLAKY_TESTS
    void statusChanged();
#endif

    void allTasksFinished();
    void activeTasks();

    void holdTask();
    void dependancies();
    void layerDependencies();
#ifdef WITH_FLAKY_TESTS
    void managerWithSubTasks();
#endif

    void managerWithSubTasks2();

    void managerWithSubTasks3();
};

void TestQgsTaskManager::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsTaskManager::cleanupTestCase()
{
  QgsApplication::exitQgis();
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
  QVERIFY( task->flags() & QgsTask::CanCancel );

  QSignalSpy startedSpy( task.data(), &QgsTask::begun );
  QSignalSpy statusSpy( task.data(), &QgsTask::statusChanged );

  task->run();
  QCOMPARE( task->status(), QgsTask::Running );
  QVERIFY( task->isActive() );
  QVERIFY( task->runCalled );
  QCOMPARE( startedSpy.count(), 1 );
  QCOMPARE( statusSpy.count(), 1 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.last().at( 0 ).toInt() ), QgsTask::Running );

  //test that calling stopped sets correct state
  QSignalSpy stoppedSpy( task.data(), &QgsTask::taskTerminated );
  task->emitTaskStopped();
  QCOMPARE( task->status(), QgsTask::Terminated );
  QVERIFY( !task->isActive() );
  QCOMPARE( stoppedSpy.count(), 1 );
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.last().at( 0 ).toInt() ), QgsTask::Terminated );

  //test that calling completed sets correct state
  task.reset( new TestTask() );
  QSignalSpy completeSpy( task.data(), &QgsTask::taskCompleted );
  QSignalSpy statusSpy2( task.data(), &QgsTask::statusChanged );
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
  task.reset( new TestTask( "desc", 0 ) );
  QVERIFY( !task->canCancel() );
  QVERIFY( !( task->flags() & QgsTask::CanCancel ) );
  task.reset( new TestTask( "desc", QgsTask::CanCancel ) );
  QVERIFY( task->canCancel() );
  QVERIFY( task->flags() & QgsTask::CanCancel );
}

void TestQgsTaskManager::taskResult()
{
  QScopedPointer< QgsTask > task( new SuccessTask() );
  QCOMPARE( task->status(), QgsTask::Queued );
  QSignalSpy statusSpy( task.data(), &QgsTask::statusChanged );

  task->run();
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.at( 0 ).at( 0 ).toInt() ), QgsTask::Running );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.at( 1 ).at( 0 ).toInt() ), QgsTask::Complete );
  QCOMPARE( task->status(), QgsTask::Complete );

  task.reset( new FailTask() );
  QCOMPARE( task->status(), QgsTask::Queued );
  QSignalSpy statusSpy2( task.data(), &QgsTask::statusChanged );

  task->run();
  QCOMPARE( statusSpy2.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy2.at( 0 ).at( 0 ).toInt() ), QgsTask::Running );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy2.at( 1 ).at( 0 ).toInt() ), QgsTask::Terminated );
  QCOMPARE( task->status(), QgsTask::Terminated );

  QApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
}

void TestQgsTaskManager::addTask()
{
  //create an empty manager
  QgsTaskManager manager;

  //should be empty
  QVERIFY( manager.tasks().isEmpty() );
  QCOMPARE( manager.count(), 0 );
  QVERIFY( !manager.task( 0L ) );

  QSignalSpy spy( &manager, &QgsTaskManager::taskAdded );

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
  QVERIFY( manager.tasks().contains( task ) );
  QCOMPARE( manager.task( 1L ), task2 );
  QVERIFY( manager.tasks().contains( task2 ) );

  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
}

#ifdef WITH_FLAKY_TESTS
// we don't run this by default - the sendPostedEvents call is fragile
void TestQgsTaskManager::taskTerminationBeforeDelete()
{
  //test that task is terminated by manager prior to delete
  QgsTaskManager* manager = new QgsTaskManager();

  //TestTerminationTask will assert that it's been terminated prior to deletion
  TestTask* task = new TestTerminationTask();
  manager->addTask( task );

  // wait till task spins up
  while ( !task->isActive() )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  // if task is not terminated assert will trip
  delete manager;
  QApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
}
#endif

void TestQgsTaskManager::taskFinished()
{
  // test that finished is called and passed correct result, and that it is called
  // from main thread
  QgsTaskManager manager;

  QgsTask::TaskResult* resultObtained = new QgsTask::TaskResult;
  *resultObtained = QgsTask::ResultPending;
  FinishTask* task = new FinishTask( resultObtained );
  task->desiredResult = QgsTask::ResultSuccess;
  manager.addTask( task );
  while ( task->status() == QgsTask::Running
          || task->status() == QgsTask::Queued )
  {
    QCoreApplication::processEvents();
  }
  while ( manager.countActiveTasks() > 0 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( *resultObtained, QgsTask::ResultSuccess );

  task = new FinishTask( resultObtained );
  task->desiredResult = QgsTask::ResultFail;
  manager.addTask( task );

  while ( task->status() == QgsTask::Running
          || task->status() == QgsTask::Queued ) { }
  while ( manager.countActiveTasks() > 0 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( *resultObtained, QgsTask::ResultFail );
}

void TestQgsTaskManager::subTask()
{
  // parent with one subtask
  TestTask* parent = new TestTask();
  QPointer<TestTask> subTask( new TestTask() );

  parent->addSubTask( subTask );

  // subtask should be deleted with parent
  delete parent;
  QVERIFY( !subTask.data() );

  // parent with grand children
  parent = new TestTask();
  subTask = new TestTask();
  QPointer< TestTask> subsubTask( new TestTask() );
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  delete parent;
  QVERIFY( !subTask.data() );
  QVERIFY( !subsubTask.data() );


  // test parent task progress
  parent = new TestTask();
  subTask = new TestTask();
  QPointer< TestTask > subTask2( new TestTask() );

  parent->addSubTask( subTask );
  parent->addSubTask( subTask2 );

  // test progress calculation
  QSignalSpy spy( parent, &QgsTask::progressChanged );
  parent->emitProgressChanged( 50 );
  QCOMPARE( qRound( parent->progress() ), 17 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( qRound( spy.last().at( 0 ).toDouble() ), 17 );

  subTask->emitProgressChanged( 100 );
  QCOMPARE( qRound( parent->progress() ), 50 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( qRound( spy.last().at( 0 ).toDouble() ), 50 );

  subTask2->emitTaskCompleted();
  QCOMPARE( qRound( parent->progress() ), 83 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( qRound( spy.last().at( 0 ).toDouble() ), 83 );

  parent->emitProgressChanged( 100 );
  QCOMPARE( qRound( parent->progress() ), 100 );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( qRound( spy.last().at( 0 ).toDouble() ), 100 );
  delete parent;

  // test canceling task with subtasks
  parent = new TestTask();
  subTask = new TestTask();
  subsubTask = new TestTask();
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  parent->cancel();
  QCOMPARE( subsubTask->status(), QgsTask::Terminated );
  QCOMPARE( subTask->status(), QgsTask::Terminated );
  QCOMPARE( parent->status(), QgsTask::Terminated );

  delete parent;

  // test that if a subtask terminates the parent task is cancelled
  parent = new TestTask();
  subTask = new TestTask();
  subsubTask = new TestTask();
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  subsubTask->emitTaskStopped();
  QCOMPARE( subsubTask->status(), QgsTask::Terminated );
  QCOMPARE( subTask->status(), QgsTask::Terminated );
  QCOMPARE( parent->status(), QgsTask::Terminated );
  delete parent;

  // test that a task is not marked complete until all subtasks are complete
  parent = new TestTask();
  subTask = new TestTask();
  subsubTask = new TestTask();
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  parent->emitTaskCompleted();

  QCOMPARE( subsubTask->status(), QgsTask::Queued );
  QCOMPARE( subTask->status(), QgsTask::Queued );
  //should still be running
  QCOMPARE(( int )parent->status(), ( int )QgsTask::Running );
  subTask->emitTaskCompleted();
  QCOMPARE( parent->status(), QgsTask::Running );
  QCOMPARE( subTask->status(), QgsTask::Running );
  subsubTask->emitTaskCompleted();
  QCOMPARE( subsubTask->status(), QgsTask::Complete );
  QCOMPARE( subTask->status(), QgsTask::Complete );
  QCOMPARE( parent->status(), QgsTask::Complete );
  delete parent;

  // another test
  parent = new TestTask();
  subTask = new TestTask();
  subsubTask = new TestTask();
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  QCOMPARE( parent->status(), QgsTask::Queued );
  QCOMPARE( subsubTask->status(), QgsTask::Queued );
  QCOMPARE( subTask->status(), QgsTask::Queued );
  subTask->emitTaskCompleted();
  QCOMPARE( parent->status(), QgsTask::Queued );
  QCOMPARE( subTask->status(), QgsTask::Running );
  QCOMPARE( subsubTask->status(), QgsTask::Queued );
  subsubTask->emitTaskCompleted();
  QCOMPARE( subsubTask->status(), QgsTask::Complete );
  QCOMPARE( subTask->status(), QgsTask::Complete );
  QCOMPARE( parent->status(), QgsTask::Queued );
  parent->emitTaskCompleted();
  QCOMPARE( subsubTask->status(), QgsTask::Complete );
  QCOMPARE( subTask->status(), QgsTask::Complete );
  QCOMPARE( parent->status(), QgsTask::Complete );
  delete parent;
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

  while ( task->status() != QgsTask::Running ||
          task2->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( task->status(), QgsTask::Running );
  QCOMPARE( task2->status(), QgsTask::Running );

  QSignalSpy spy( &manager, &QgsTaskManager::progressChanged );
  QSignalSpy spy2( &manager, &QgsTaskManager::finalTaskProgressChanged );

  task->emitProgressChanged( 50.0 );
  QCOMPARE( task->progress(), 50.0 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( spy.last().at( 1 ).toDouble(), 50.0 );
  //multiple running tasks, so finalTaskProgressChanged(double) should not be emitted
  QCOMPARE( spy2.count(), 0 );

  task2->emitProgressChanged( 75.0 );
  QCOMPARE( task2->progress(), 75.0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( spy.last().at( 1 ).toDouble(), 75.0 );
  QCOMPARE( spy2.count(), 0 );

  task->emitTaskCompleted();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( task2->status(), QgsTask::Running );
  task2->emitProgressChanged( 80.0 );
  //single running task, so finalTaskProgressChanged(double) should be emitted
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( spy2.last().at( 0 ).toDouble(), 80.0 );

  TestTask* task3 = new TestTask();
  manager.addTask( task3 );
  while ( task3->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  //multiple running tasks, so finalTaskProgressChanged(double) should not be emitted
  task2->emitProgressChanged( 81.0 );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( task2->status(), QgsTask::Running );
  QCOMPARE( task3->status(), QgsTask::Running );

  task2->emitTaskStopped();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  task3->emitProgressChanged( 30.0 );
  //single running task, so finalTaskProgressChanged(double) should be emitted
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( spy2.last().at( 0 ).toDouble(), 30.0 );
}

#ifdef WITH_FLAKY_TESTS
void TestQgsTaskManager::statusChanged()
{
  // check that statusChanged signals emitted by tasks result in statusChanged signal from manager
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();

  manager.addTask( task );
  while ( task->status() != QgsTask::Running || manager.countActiveTasks() < 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  QSignalSpy spy( &manager, &QgsTaskManager::statusChanged );
  manager.addTask( task2 );
  while ( task2->status() != QgsTask::Running || manager.countActiveTasks() < 2 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
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
#endif

void TestQgsTaskManager::allTasksFinished()
{
  // check that allTasksFinished signal is correctly emitted by manager
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();
  manager.addTask( task );
  manager.addTask( task2 );
  while ( task->status() != QgsTask::Running || task2->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  QSignalSpy spy( &manager, &QgsTaskManager::allTasksFinished );

  task->emitTaskStopped();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 0 );
  task2->emitTaskCompleted();
  while ( manager.countActiveTasks() > 0 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 1 );

  TestTask* task3 = new TestTask();
  TestTask* task4 = new TestTask();
  manager.addTask( task3 );
  while ( task3->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  manager.addTask( task4 );
  while ( task4->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  QCoreApplication::processEvents();
  task3->emitTaskStopped();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 1 );
  TestTask* task5 = new TestTask();
  manager.addTask( task5 );
  while ( task5->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  task4->emitTaskStopped();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 1 );
  task5->emitTaskStopped();
  while ( manager.countActiveTasks() > 0 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 2 );
}

void TestQgsTaskManager::activeTasks()
{
  // check that statusChanged signals emitted by tasks result in statusChanged signal from manager
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();
  QSignalSpy spy( &manager, &QgsTaskManager::countActiveTasksChanged );
  manager.addTask( task );
  while ( task->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask* >() << task ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 1 );
  manager.addTask( task2 );
  while ( task2->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask* >() << task << task2 ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 2 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 2 );
  task->emitTaskCompleted();
  while ( task->status() == QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask* >() << task2 ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 1 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 1 );
  task2->emitTaskCompleted();
  while ( task2->status() == QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QVERIFY( manager.activeTasks().isEmpty() );
  QCOMPARE( manager.countActiveTasks(), 0 );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 0 );
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
  while ( task->status() == QgsTask::Queued )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
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

  long taskId = manager.addTask( QgsTaskManager::TaskDefinition( task, QgsTaskList() << childTask ) );
  long childTaskId = manager.addTask( QgsTaskManager::TaskDefinition( childTask, QgsTaskList() << grandChildTask ) );
  long grandChildTaskId = manager.addTask( grandChildTask );

  // check dependency resolution
  QCOMPARE( manager.dependencies( grandChildTaskId ), QSet< long >() );
  QCOMPARE( manager.dependencies( childTaskId ), QSet< long >() << grandChildTaskId );
  QCOMPARE( manager.dependencies( taskId ), QSet< long >() << childTaskId << grandChildTaskId );

  QVERIFY( !manager.hasCircularDependencies( taskId ) );
  QVERIFY( !manager.hasCircularDependencies( childTaskId ) );
  QVERIFY( !manager.hasCircularDependencies( grandChildTaskId ) );

  grandChildTask->cancel();
  QCOMPARE( childTask->status(), QgsTask::Terminated );
  QCOMPARE( task->status(), QgsTask::Terminated );

  // test that tasks are queued until dependancies are resolved
  task = new TestTask();
  childTask = new TestTask();
  childTask->hold();
  taskId = manager.addTask( QgsTaskManager::TaskDefinition( task, QgsTaskList() << childTask ) );
  childTaskId = manager.addTask( childTask );
  QVERIFY( !manager.dependenciesSatisified( taskId ) );
  QVERIFY( manager.dependenciesSatisified( childTaskId ) );

  QCOMPARE( childTask->status(), QgsTask::OnHold );
  QCOMPARE( task->status(), QgsTask::Queued );

  childTask->unhold();
  //wait for childTask to spin up
  while ( !childTask->isActive() )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( childTask->status(), QgsTask::Running );
  QCOMPARE( task->status(), QgsTask::Queued );
  childTask->emitTaskCompleted();
  //wait for childTask to complete
  while ( childTask->isActive() )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QVERIFY( manager.dependenciesSatisified( taskId ) );
  QCOMPARE( childTask->status(), QgsTask::Complete );
  //wait for task to spin up
  while ( !task->isActive() )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( task->status(), QgsTask::Running );
  task->emitTaskCompleted();


  // test circular dependency detection
  task = new TestTask();
  task->hold();
  childTask = new TestTask();
  childTask->hold();
  grandChildTask = new TestTask();
  grandChildTask->hold();

  taskId = manager.addTask( QgsTaskManager::TaskDefinition( task, QgsTaskList() << childTask ) );
  childTaskId = manager.addTask( QgsTaskManager::TaskDefinition( childTask, QgsTaskList() << grandChildTask ) );
  grandChildTaskId = manager.addTask( QgsTaskManager::TaskDefinition( grandChildTask, QgsTaskList() << task ) );

  //expect all these circular tasks to be terminated due to circular dependencies
  QCOMPARE( task->status(), QgsTask::Terminated );
  QCOMPARE( childTask->status(), QgsTask::Terminated );
  QCOMPARE( grandChildTask->status(), QgsTask::Terminated );
}

void TestQgsTaskManager::layerDependencies()
{
  //make some layers
  QgsVectorLayer* layer1 = new QgsVectorLayer( "Point?field=col1:string&field=col2:string&field=col3:string", "layer1", "memory" );
  QVERIFY( layer1->isValid() );
  QgsVectorLayer* layer2 = new QgsVectorLayer( "Point?field=col1:string&field=col2:string&field=col3:string", "layer2", "memory" );
  QVERIFY( layer2->isValid() );
  QgsVectorLayer* layer3 = new QgsVectorLayer( "Point?field=col1:string&field=col2:string&field=col3:string", "layer3", "memory" );
  QVERIFY( layer3->isValid() );
  QgsMapLayerRegistry::instance()->addMapLayers( QList< QgsMapLayer* >() << layer1 << layer2 << layer3 );

  QgsTaskManager manager;

  //test that remove layers cancels all tasks which are dependant on them
  TestTask* task = new TestTask();
  task->hold();
  long taskId = manager.addTask( task );
  manager.setDependentLayers( taskId, QStringList() << layer2->id() << layer3->id() );

  QCOMPARE( task->status(), QgsTask::OnHold );
  //removing layer1 should have no effect
  QgsMapLayerRegistry::instance()->removeMapLayers( QList< QgsMapLayer* >() << layer1 );
  QCOMPARE( task->status(), QgsTask::OnHold );
  //removing layer3 should cancel task
  QgsMapLayerRegistry::instance()->removeMapLayers( QList< QgsMapLayer* >() << layer3 );
  QCOMPARE( task->status(), QgsTask::Terminated );

  QgsMapLayerRegistry::instance()->removeMapLayers( QList< QgsMapLayer* >() << layer2 );
}

#ifdef WITH_FLAKY_TESTS
void TestQgsTaskManager::managerWithSubTasks()
{
  return;
  // parent with subtasks
  TestTask* parent = new TestTask( "parent" );
  TestTask* subTask = new TestTask( "subtask" );
  TestTask* subsubTask = new TestTask( "subsubtask" );
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  QgsTaskManager* manager = new QgsTaskManager();
  QSignalSpy spy( manager, &QgsTaskManager::taskAdded );
  QSignalSpy spyProgress( manager, &QgsTaskManager::progressChanged );

  manager->addTask( parent );
  // manager should only report 1 task added
  QCOMPARE( manager->tasks().count(), 1 );
  QVERIFY( manager->tasks().contains( parent ) );
  QCOMPARE( manager->count(), 1 );
  QCOMPARE( manager->countActiveTasks(), 1 );
  QCOMPARE( manager->activeTasks().count(), 1 );
  QVERIFY( manager->activeTasks().contains( parent ) );
  QCOMPARE( spy.count(), 1 );

  //manager should not directly listen to progress reports from subtasks
  //(only parent tasks, which themselves include their subtask progress)
  QCOMPARE( spyProgress.count(), 0 );
  subTask->emitProgressChanged( 50 );
  QCOMPARE( spyProgress.count(), 1 );
  QCOMPARE( spyProgress.last().at( 0 ).toLongLong(), 0LL );
  // subTask itself is 50% done, so with it's child task it's sitting at overall 25% done
  // (one task 50%, one task not started)
  // parent task has two tasks (itself + subTask), and subTask is 25% done.... so parent
  // task is 12.5% done. yes-- these numbers are correct!
  QCOMPARE( spyProgress.last().at( 1 ).toInt(), 13 );
  subsubTask->emitProgressChanged( 100 );
  QCOMPARE( spyProgress.count(), 2 );
  QCOMPARE( spyProgress.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( spyProgress.last().at( 1 ).toInt(), 38 );
  parent->emitProgressChanged( 50 );
  QCOMPARE( spyProgress.count(), 3 );
  QCOMPARE( spyProgress.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( spyProgress.last().at( 1 ).toInt(), 63 );

  //manager should not emit statusChanged signals for subtasks
  QSignalSpy statusSpy( manager, &QgsTaskManager::statusChanged );
  QCOMPARE( statusSpy.count(), 0 );
  subsubTask->emitTaskCompleted();
  while ( subsubTask->status() != QgsTask::Complete )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( statusSpy.count(), 1 );
  QCOMPARE( statusSpy.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.last().at( 1 ).toInt() ), QgsTask::Running );

  subTask->emitTaskCompleted();
  while ( subTask->status() != QgsTask::Complete )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( statusSpy.count(), 1 );

  parent->emitTaskCompleted();
  while ( parent->status() != QgsTask::Complete )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( statusSpy.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.last().at( 1 ).toInt() ), QgsTask::Complete );


  subsubTask->emitTaskCompleted();
  subTask->emitTaskCompleted();
  parent->emitTaskCompleted();
  delete manager;

}

void TestQgsTaskManager::managerWithSubTasks2()
{
  //test dependencies

  //test 1
  QgsTaskManager* manager2 = new QgsTaskManager();
  QPointer< CancelableTask > parent( new CancelableTask() );
// parent->hold();
  QPointer< CancelableTask > subTask( new CancelableTask() );
  //subTask->hold();
  QPointer< CancelableTask > subTask2( new CancelableTask() );
  //subTask2->hold();

  parent->addSubTask( subTask, QgsTaskList() << subTask2 );
  parent->addSubTask( subTask2 );
  manager2->addTask( parent );
  while ( parent->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  long parentId = manager2->taskId( parent );
  long subTaskId = manager2->taskId( subTask );
  long subTask2Id = manager2->taskId( subTask2 );

  QCOMPARE( manager2->dependencies( parentId ), QSet< long >() );
  QCOMPARE( manager2->dependencies( subTaskId ), QSet< long >() << subTask2Id );
  QCOMPARE( manager2->dependencies( subTask2Id ), QSet< long >() );
  delete manager2;
}

void TestQgsTaskManager::managerWithSubTasks3()
{
  //test 2
  QgsTaskManager manager3;
  TestTask* parent = new TestTask( "parent" );
  parent->hold();
  TestTask* subTask = new TestTask( "subtask" );
  subTask->hold();
  TestTask* subTask2 = new TestTask( "subtask2" );
  subTask2->hold();

  parent->addSubTask( subTask, QgsTaskList() << subTask2 );
  parent->addSubTask( subTask2, QgsTaskList(), QgsTask::ParentDependsOnSubTask );
  manager3.addTask( parent );

  long parentId = manager3.taskId( parent );
  long subTaskId = manager3.taskId( subTask );
  long subTask2Id = manager3.taskId( subTask2 );

  QCOMPARE( manager3.dependencies( parentId ), QSet< long >() << subTask2Id );
  QCOMPARE( manager3.dependencies( subTaskId ), QSet< long >() << subTask2Id );
  QCOMPARE( manager3.dependencies( subTask2Id ), QSet< long >() );
}
#endif

QTEST_MAIN( TestQgsTaskManager )
#include "testqgstaskmanager.moc"
