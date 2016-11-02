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

    TaskResult run() override
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

    TaskResult run() override
    {
      while ( !isCancelled() )
        {}
      return ResultFail;
    }
};

class SuccessTask : public TestTask
{
    Q_OBJECT

  protected:

    TaskResult run() override
    {
      return ResultSuccess;
    }
};

class FailTask : public TestTask
{
    Q_OBJECT

  protected:

    TaskResult run() override
    {
      return ResultFail;
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
    void taskResult();
    void createInstance();
    void addTask();
    void deleteTask();
    //void taskTerminationBeforeDelete();
    void taskId();
    void progressChanged();
    void statusChanged();
    void allTasksFinished();
    void activeTasks();
    void holdTask();
    void dependancies();
    void layerDependencies();

  private:

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

void TestQgsTaskManager::taskResult()
{
  QScopedPointer< TestTask > task( new SuccessTask() );
  QCOMPARE( task->status(), QgsTask::Queued );
  QSignalSpy statusSpy( task.data(), SIGNAL( statusChanged( int ) ) );

  task->start();
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.at( 0 ).at( 0 ).toInt() ), QgsTask::Running );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.at( 1 ).at( 0 ).toInt() ), QgsTask::Complete );
  QCOMPARE( task->status(), QgsTask::Complete );

  task.reset( new FailTask() );
  QCOMPARE( task->status(), QgsTask::Queued );
  QSignalSpy statusSpy2( task.data(), SIGNAL( statusChanged( int ) ) );

  task->start();
  QCOMPARE( statusSpy2.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy2.at( 0 ).at( 0 ).toInt() ), QgsTask::Running );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy2.at( 1 ).at( 0 ).toInt() ), QgsTask::Terminated );
  QCOMPARE( task->status(), QgsTask::Terminated );
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

#if 0
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
    {}

  // if task is not terminated assert will trip
  delete manager;
  QApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
}
#endif

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
  QSignalSpy spy2( &manager, SIGNAL( progressChanged( double ) ) );

  task->emitProgressChanged( 50.0 );
  QCOMPARE( task->progress(), 50.0 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 0LL );
  QCOMPARE( spy.last().at( 1 ).toDouble(), 50.0 );
  //multiple running tasks, so progressChanged(double) should not be emitted
  QCOMPARE( spy2.count(), 0 );

  task2->emitProgressChanged( 75.0 );
  QCOMPARE( task2->progress(), 75.0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( spy.last().at( 1 ).toDouble(), 75.0 );
  QCOMPARE( spy2.count(), 0 );

  task->emitTaskCompleted();
  task2->emitProgressChanged( 80.0 );
  //single running task, so progressChanged(double) should be emitted
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( spy2.last().at( 0 ).toDouble(), 80.0 );

  TestTask* task3 = new TestTask();
  manager.addTask( task3 );
  //multiple running tasks, so progressChanged(double) should not be emitted
  task2->emitProgressChanged( 81.0 );
  QCOMPARE( spy2.count(), 1 );

  task2->emitTaskStopped();
  task3->emitProgressChanged( 30.0 );
  //single running task, so progressChanged(double) should be emitted
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( spy2.last().at( 0 ).toDouble(), 30.0 );
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

void TestQgsTaskManager::allTasksFinished()
{
  // check that allTasksFinished signal is correctly emitted by manager
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();
  manager.addTask( task );
  manager.addTask( task2 );
  while ( task2->status() != QgsTask::Running ) { }

  QSignalSpy spy( &manager, SIGNAL( allTasksFinished() ) );

  task->emitTaskStopped();
  while ( task->status() == QgsTask::Running ) { }
  QCOMPARE( spy.count(), 0 );
  task2->emitTaskCompleted();
  while ( task2->status() == QgsTask::Running ) { }
  QCOMPARE( spy.count(), 1 );

  TestTask* task3 = new TestTask();
  TestTask* task4 = new TestTask();
  manager.addTask( task3 );
  while ( task3->status() != QgsTask::Running ) { }
  manager.addTask( task4 );
  while ( task4->status() != QgsTask::Running ) { }
  task3->emitTaskStopped();
  while ( task3->status() == QgsTask::Running ) { }
  QCOMPARE( spy.count(), 1 );
  TestTask* task5 = new TestTask();
  manager.addTask( task5 );
  while ( task5->status() != QgsTask::Running ) { }
  task4->emitTaskStopped();
  while ( task4->status() == QgsTask::Running ) { }
  QCOMPARE( spy.count(), 1 );
  task5->emitTaskStopped();
  while ( task5->status() == QgsTask::Running ) { }
  QCOMPARE( spy.count(), 2 );
}

void TestQgsTaskManager::activeTasks()
{
  // check that statusChanged signals emitted by tasks result in statusChanged signal from manager
  QgsTaskManager manager;
  TestTask* task = new TestTask();
  TestTask* task2 = new TestTask();
  QSignalSpy spy( &manager, SIGNAL( countActiveTasksChanged( int ) ) );
  manager.addTask( task );
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask* >() << task ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 1 );
  manager.addTask( task2 );
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask* >() << task << task2 ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 2 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 2 );
  task->emitTaskCompleted();
  while ( task->status() == QgsTask::Running ) { }
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask* >() << task2 ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 1 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 1 );
  task2->emitTaskCompleted();
  while ( task2->status() == QgsTask::Running ) { }
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

  long taskId = manager.addTask( task, QgsTaskList() << childTask );
  long childTaskId = manager.addTask( childTask, QgsTaskList() << grandChildTask );
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
  taskId = manager.addTask( task, QgsTaskList() << childTask );
  childTaskId = manager.addTask( childTask );
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
  task->emitTaskCompleted();


  // test circular dependency detection
  task = new TestTask();
  task->hold();
  childTask = new TestTask();
  childTask->hold();
  grandChildTask = new TestTask();
  grandChildTask->hold();

  taskId = manager.addTask( task, QgsTaskList() << childTask );
  childTaskId = manager.addTask( childTask, QgsTaskList() << grandChildTask );
  grandChildTaskId = manager.addTask( grandChildTask, QgsTaskList() << task );

  QVERIFY( manager.hasCircularDependencies( taskId ) );
  QVERIFY( manager.hasCircularDependencies( childTaskId ) );
  QVERIFY( manager.hasCircularDependencies( grandChildTaskId ) );

  //expect all these circular tasks to be terminated
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


QTEST_MAIN( TestQgsTaskManager )
#include "testqgstaskmanager.moc"
