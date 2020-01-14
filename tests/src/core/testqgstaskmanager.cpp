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
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsproxyprogresstask.h"
#include <QObject>
#include "qgstest.h"

class TestTask : public QgsTask
{
    Q_OBJECT

  public:

    TestTask( const QString &desc = QString() ) : QgsTask( desc )
    {
      qDebug() << "created task " << desc;
    }

    TestTask( const QString &desc, const QgsTask::Flags &flags ) : QgsTask( desc, flags )
    {
      qDebug() << "created task " << desc;
    }

    ~TestTask() override
    {
      qDebug() << "deleting task " << description();
    }

    void emitProgressChanged( double progress ) { setProgress( progress ); }
    void emitTaskStopped() {  }
    void emitTaskCompleted() { }

    bool runCalled =  false ;

  protected:

    bool run() override
    {
      runCalled = true;
      return true;
    }

};

class ProgressReportingTask : public QgsTask
{
    Q_OBJECT

  public:

    ProgressReportingTask( const QString &desc = QString() ) : QgsTask( desc )
    {
      qDebug() << "created task " << desc;
    }

    ~ProgressReportingTask() override
    {
      qDebug() << "deleting task " << description();
    }

    void emitProgressChanged( double progress ) { setProgress( progress ); }

    bool finished =  false ;
    bool terminated =  false ;

  public slots:
    void finish() { finished = true; }
    void terminate() { terminated = true; }

  protected:

    bool run() override
    {
      while ( !finished && !terminated && !isCanceled() ) {}
      return finished;
    }

};

class TestTerminationTask : public TestTask
{
    Q_OBJECT

  public:

    TestTerminationTask( const QString &desc = QString() ) : TestTask( desc ) {}

    ~TestTerminationTask() override
    {
      //make sure task has been terminated by manager prior to deletion
      QVERIFY( status() == QgsTask::Terminated );
    }

  protected:

    bool run() override
    {
      while ( !isCanceled() )
      {}
      return false;
    }
};

class CancelableTask : public QgsTask
{
    Q_OBJECT

  public:

    CancelableTask( const QString &desc = QString() ) : QgsTask( desc )
    {
      qDebug() << "created task " << desc;
    }

    ~CancelableTask() override
    {
      qDebug() << "deleting task " << description();

      int i = 1;
      i++;
    }

  protected:

    bool run() override
    {
      while ( !isCanceled() )
      {}
      return true;
    }
};

class SuccessTask : public QgsTask
{
    Q_OBJECT

  public:

    SuccessTask( const QString &desc = QString() ) : QgsTask( desc )
    {
      qDebug() << "created task " << desc;
    }

    ~SuccessTask() override
    {
      qDebug() << "deleting task " << description();
    }

  protected:

    bool run() override
    {
      return true;
    }
};

class FailTask : public QgsTask
{
    Q_OBJECT

  public:

    FailTask( const QString &desc = QString() ) : QgsTask( desc )
    {
      qDebug() << "created task " << desc;
    }

    ~FailTask() override
    {
      qDebug() << "deleting task " << description();
    }

  protected:

    bool run() override
    {
      return false;
    }

};

class FinishTask : public QgsTask
{
    Q_OBJECT

  public:

    FinishTask( bool *result, const QString &desc )
      : QgsTask( desc )
      , desiredResult( false )
      , resultObtained( result )
    {
      qDebug() << "created task " << desc;
    }

    ~FinishTask() override
    {
      qDebug() << "deleting task " << description();
    }

    bool desiredResult;
    bool *resultObtained = nullptr;

  protected:

    bool run() override
    {
      return desiredResult;
    }

    void finished( bool result ) override
    {
      QVERIFY( QApplication::instance()->thread() == QThread::currentThread() );
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

class WaitTask : public QgsTask
{
  public:

    WaitTask( const QString &desc = QString() ) : QgsTask( desc )
    {
      qDebug() << "created task " << desc;
    }

    ~WaitTask() override
    {
      qDebug() << "deleting task " << description();
    }

  protected:

    bool run() override
    {
      QThread::sleep( 2 );
      return true;
    }
};

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
    void taskTerminationBeforeDelete();
    void taskId();
    void waitForFinished();
    void waitForFinishedBeforeStart();
    void progressChanged();
    void statusChanged();
    void allTasksFinished();
    void activeTasks();
    void holdTask();
    void dependencies();
    void layerDependencies();
    void managerWithSubTasks();
    void managerWithSubTasks2();
    void managerWithSubTasks3();
    void cancelBeforeStart();
    void proxyTask();
    void proxyTask2();
    void scopedProxyTask();
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
  std::unique_ptr< TestTask > task( new TestTask( QStringLiteral( "test_task_desc" ) ) );
  QCOMPARE( task->status(), QgsTask::Queued );
  QCOMPARE( task->description(), QStringLiteral( "test_task_desc" ) );
  QVERIFY( !task->isActive() );
  QVERIFY( task->canCancel() );
  QVERIFY( task->flags() & QgsTask::CanCancel );

  QSignalSpy startedSpy( task.get(), &QgsTask::begun );
  QSignalSpy statusSpy( task.get(), &QgsTask::statusChanged );

  task->start();
  QVERIFY( task->runCalled );
  QCOMPARE( startedSpy.count(), 1 );
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.at( 0 ).at( 0 ).toInt() ), QgsTask::Running );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.at( 1 ).at( 0 ).toInt() ), QgsTask::Complete );

  //test that calling stopped sets correct state
  std::unique_ptr< FailTask > failTask( new FailTask( QStringLiteral( "task_fail" ) ) );
  QSignalSpy stoppedSpy( failTask.get(), &QgsTask::taskTerminated );
  QSignalSpy statusSpy2( failTask.get(), &QgsTask::statusChanged );
  failTask->start();
  QCOMPARE( failTask->status(), QgsTask::Terminated );
  QVERIFY( !failTask->isActive() );
  QCOMPARE( stoppedSpy.count(), 1 );
  QCOMPARE( statusSpy2.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy2.last().at( 0 ).toInt() ), QgsTask::Terminated );

  //test that calling completed sets correct state
  task.reset( new TestTask( QStringLiteral( "test_task_3" ) ) );
  QSignalSpy completeSpy( task.get(), &QgsTask::taskCompleted );
  QSignalSpy statusSpy3( task.get(), &QgsTask::statusChanged );
  task->start();
  QCOMPARE( task->status(), QgsTask::Complete );
  QVERIFY( !task->isActive() );
  QCOMPARE( completeSpy.count(), 1 );
  QCOMPARE( statusSpy3.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy3.last().at( 0 ).toInt() ), QgsTask::Complete );

  // test that canceling tasks which have not begin immediately ends them
  task.reset( new TestTask( QStringLiteral( "test_task_4" ) ) );
  task->cancel(); // Queued task
  QCOMPARE( task->status(), QgsTask::Terminated );
  task.reset( new TestTask( QStringLiteral( "test_task_5" ) ) );
  task->hold(); // OnHold task
  task->cancel();
  QCOMPARE( task->status(), QgsTask::Terminated );

  // test flags
  task.reset( new TestTask( QStringLiteral( "test_task_6" ), nullptr ) );
  QVERIFY( !task->canCancel() );
  QVERIFY( !( task->flags() & QgsTask::CanCancel ) );
  task.reset( new TestTask( QStringLiteral( "test_task_7" ), QgsTask::CanCancel ) );
  QVERIFY( task->canCancel() );
  QVERIFY( task->flags() & QgsTask::CanCancel );
}

void TestQgsTaskManager::taskResult()
{
  std::unique_ptr< QgsTask > task( new SuccessTask( QStringLiteral( "task_result_1" ) ) );
  QCOMPARE( task->status(), QgsTask::Queued );
  QSignalSpy statusSpy( task.get(), &QgsTask::statusChanged );

  task->start();
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.at( 0 ).at( 0 ).toInt() ), QgsTask::Running );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.at( 1 ).at( 0 ).toInt() ), QgsTask::Complete );
  QCOMPARE( task->status(), QgsTask::Complete );

  task.reset( new FailTask( QStringLiteral( "task_result_2" ) ) );
  QCOMPARE( task->status(), QgsTask::Queued );
  QSignalSpy statusSpy2( task.get(), &QgsTask::statusChanged );

  task->start();
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

  // null task
  QVERIFY( !manager.addTask( nullptr ) );

  //add a task
  CancelableTask *task = new CancelableTask( QStringLiteral( "add_task_1" ) );
  long id = manager.addTask( task );

  QCOMPARE( id, 1L );
  QCOMPARE( manager.tasks().count(), 1 );
  QCOMPARE( manager.count(), 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
  while ( !task->isActive() )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QVERIFY( task->isActive() );
  QCOMPARE( task->status(), QgsTask::Running );

  //retrieve task
  QCOMPARE( manager.task( 1L ), task );
  QCOMPARE( manager.tasks().at( 0 ), task );

  //add a second task
  CancelableTask *task2 = new CancelableTask( QStringLiteral( "add_task_2" ) );
  id = manager.addTask( task2 );
  QCOMPARE( id, 2L );
  QCOMPARE( manager.tasks().count(), 2 );
  QCOMPARE( manager.count(), 2 );
  QCOMPARE( manager.task( 1L ), task );
  QVERIFY( manager.tasks().contains( task ) );
  QCOMPARE( manager.task( 2L ), task2 );
  QVERIFY( manager.tasks().contains( task2 ) );
  while ( !task2->isActive() )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QVERIFY( task2->isActive() );
  QCOMPARE( task2->status(), QgsTask::Running );

  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 2LL );

  task->cancel();
  task2->cancel();

  while ( manager.countActiveTasks() > 0 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
}

void TestQgsTaskManager::taskTerminationBeforeDelete()
{
  //test that task is terminated by manager prior to delete
  QgsTaskManager *manager = new QgsTaskManager();

  //TestTerminationTask will assert that it's been terminated prior to deletion
  TestTask *task = new TestTerminationTask( QStringLiteral( "termination_task_1" ) );
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
  flushEvents();
}

void TestQgsTaskManager::taskFinished()
{
  // test that finished is called and passed correct result, and that it is called
  // from main thread
  QgsTaskManager manager;

  bool resultObtained = false;
  FinishTask *task = new FinishTask( &resultObtained, QStringLiteral( "finished_task_1" ) );
  task->desiredResult = true;
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
  QCOMPARE( resultObtained, true );

  task = new FinishTask( &resultObtained, QStringLiteral( "finished_task_2" ) );
  task->desiredResult = false;
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
  QCOMPARE( resultObtained, false );
}

void TestQgsTaskManager::subTask()
{
  QgsTaskManager manager;

  // parent with one subtask
  ProgressReportingTask *parent = new ProgressReportingTask( QStringLiteral( "sub_task_parent_task_1" ) );
  QPointer<ProgressReportingTask> subTask( new ProgressReportingTask( QStringLiteral( "sub_task_sub_task_1" ) ) );

  parent->addSubTask( subTask );

  // subtask should be deleted with parent
  delete parent;
  QVERIFY( !subTask.data() );

  // parent with grand children
  parent = new ProgressReportingTask( QStringLiteral( "sub_task_parent_task_2" ) );
  subTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_task_2" ) );
  QPointer< ProgressReportingTask> subsubTask( new ProgressReportingTask( QStringLiteral( "sub_task_subsub_task_2" ) ) );
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  delete parent;
  QVERIFY( !subTask.data() );
  QVERIFY( !subsubTask.data() );

  // test parent task progress
  parent = new ProgressReportingTask( QStringLiteral( "sub_task_parent_task_3" ) );
  subTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_task_3" ) );
  QPointer< ProgressReportingTask > subTask2( new ProgressReportingTask( QStringLiteral( "sub_task_sub_task_3a" ) ) );

  parent->addSubTask( subTask );
  parent->addSubTask( subTask2 );

  manager.addTask( parent );
  while ( parent->status() != QgsTask::Running
          && subTask->status() != QgsTask::Running
          && subTask2->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  // test progress calculation
  QSignalSpy spy( parent, &QgsTask::progressChanged );
  parent->emitProgressChanged( 50 );
  flushEvents();
  QCOMPARE( std::round( parent->progress() ), 17.0 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( std::round( spy.last().at( 0 ).toDouble() ), 17.0 );

  subTask->emitProgressChanged( 100 );
  flushEvents();
  QCOMPARE( std::round( parent->progress() ), 50.0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( std::round( spy.last().at( 0 ).toDouble() ), 50.0 );

  subTask2->finish();
  while ( subTask2->status() != QgsTask::Complete )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( std::round( parent->progress() ), 83.0 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( std::round( spy.last().at( 0 ).toDouble() ), 83.0 );

  parent->emitProgressChanged( 100 );
  QCOMPARE( std::round( parent->progress() ), 100.0 );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( std::round( spy.last().at( 0 ).toDouble() ), 100.0 );
  parent->terminate();
  subTask->terminate();

  // test canceling task with subtasks
  parent = new ProgressReportingTask( QStringLiteral( "sub_task_parent_task_4" ) );
  subTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_task_4" ) );
  subsubTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_sub_task_4" ) );
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  parent->cancel();
  QCOMPARE( subsubTask->status(), QgsTask::Terminated );
  QCOMPARE( subTask->status(), QgsTask::Terminated );
  QCOMPARE( parent->status(), QgsTask::Terminated );

  delete parent;

  // test that if a subtask terminates the parent task is canceled
  parent = new ProgressReportingTask( QStringLiteral( "sub_task_parent_task_5" ) );
  subTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_task_5" ) );
  subsubTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_sub_task_5" ) );
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  manager.addTask( parent );
  while ( subsubTask->status() != QgsTask::Running
          && subTask->status() != QgsTask::Running
          && parent->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }

  QSignalSpy parentTerminated( parent, &QgsTask::taskTerminated );
  QSignalSpy subTerminated( subTask, &QgsTask::taskTerminated );
  QSignalSpy subsubTerminated( subsubTask, &QgsTask::taskTerminated );

  subsubTask->terminate();
  while ( subsubTask->status() == QgsTask::Running
          || subTask->status() == QgsTask::Running
          || parent->status() == QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QVERIFY( parentTerminated.count() > 0 );
  QVERIFY( subTerminated.count() > 0 );
  QVERIFY( subsubTerminated.count() > 0 );

  // test that a task is not marked complete until all subtasks are complete
  parent = new ProgressReportingTask( QStringLiteral( "sub_task_parent_task_6" ) );
  subTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_task_6" ) );
  subsubTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_sub_task_6" ) );
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );
  manager.addTask( parent );
  while ( subsubTask->status() != QgsTask::Running
          && subTask->status() != QgsTask::Running
          && parent->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  parent->finish();
  flushEvents();

  //should still be running
  QCOMPARE( ( int )parent->status(), ( int )QgsTask::Running );
  subTask->finish();
  flushEvents();
  QCOMPARE( parent->status(), QgsTask::Running );
  QCOMPARE( ( int )subTask->status(), ( int )QgsTask::Running );

  QSignalSpy parentFinished( parent, &QgsTask::taskCompleted );
  QSignalSpy subFinished( subTask, &QgsTask::taskCompleted );
  QSignalSpy subsubFinished( subsubTask, &QgsTask::taskCompleted );

  subsubTask->finish();
  while ( subsubTask->status() == QgsTask::Running
          || subTask->status() == QgsTask::Running
          || parent->status() == QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QVERIFY( parentFinished.count() > 0 );
  QVERIFY( subFinished.count() > 0 );
  QVERIFY( subsubFinished.count() > 0 );

  // another test
  parent = new ProgressReportingTask( QStringLiteral( "sub_task_parent_task_7" ) );
  subTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_task_7" ) );
  subsubTask = new ProgressReportingTask( QStringLiteral( "sub_task_sub_sub_task_7" ) );
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );
  manager.addTask( parent );
  while ( subsubTask->status() != QgsTask::Running
          || subTask->status() != QgsTask::Running
          || parent->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  QCOMPARE( ( int )parent->status(), ( int )QgsTask::Running );
  QCOMPARE( ( int )subsubTask->status(), ( int )QgsTask::Running );
  QCOMPARE( ( int )subTask->status(), ( int )QgsTask::Running );
  subTask->finish();
  flushEvents();
  QCOMPARE( parent->status(), QgsTask::Running );
  QCOMPARE( subTask->status(), QgsTask::Running );
  QCOMPARE( subsubTask->status(), QgsTask::Running );
  subsubTask->finish();
  while ( subsubTask->status() == QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( subsubTask->status(), QgsTask::Complete );
  QCOMPARE( subTask->status(), QgsTask::Complete );
  QCOMPARE( parent->status(), QgsTask::Running );
  QSignalSpy parentFinished2( parent, &QgsTask::taskCompleted );
  parent->finish();
  while ( parent->status() == QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QVERIFY( parentFinished2.count() > 0 );
}

void TestQgsTaskManager::taskId()
{
  //test finding task IDs

  //create manager with some tasks
  QgsTaskManager manager;
  TestTask *task = new TestTask( QStringLiteral( "task_id_1" ) );
  TestTask *task2 = new TestTask( QStringLiteral( "task_id_2" ) );
  manager.addTask( task );
  manager.addTask( task2 );

  //also a task not in the manager
  TestTask *task3 = new TestTask( QStringLiteral( "task_id_3" ) );

  QCOMPARE( manager.taskId( nullptr ), -1L );
  QCOMPARE( manager.taskId( task ), 1L );
  QCOMPARE( manager.taskId( task2 ), 2L );
  QCOMPARE( manager.taskId( task3 ), -1L );

  delete task3;
  flushEvents();
}

void TestQgsTaskManager::waitForFinished()
{
  if ( !QgsTest::runFlakyTests() )
    QSKIP( "This test is disabled on Travis CI environment" );

  QgsTaskManager manager;
  QEventLoop loop;

  ProgressReportingTask *finishedTask = new ProgressReportingTask();
  connect( finishedTask, &ProgressReportingTask::begun, &loop, &QEventLoop::quit );
  manager.addTask( finishedTask );
  if ( finishedTask->status() != QgsTask::Running )
    loop.exec();

  // we have to run the timer in a different thread, because waitForFinished will block this thread, meaning the
  // event loop never runs and the timer timeout signal never gets through...
  QThread *timerThread = new QThread();
  connect( timerThread, &QThread::finished, timerThread, &QThread::deleteLater );
  QTimer *timer = new QTimer( nullptr );
  connect( timer, &QTimer::timeout, finishedTask, &ProgressReportingTask::finish, Qt::DirectConnection );
  timer->moveToThread( timerThread );
  connect( timerThread, &QThread::started, timer, [ = ]
  {
    timer->start( 2000 );
  } );
  connect( timerThread, &QThread::finished, timer, &QTimer::deleteLater );
  timerThread->start();

  QCOMPARE( finishedTask->waitForFinished(), true );
  QCOMPARE( finishedTask->status(), QgsTask::Complete );

  timerThread->quit();

  ProgressReportingTask *failedTask = new ProgressReportingTask();
  connect( failedTask, &ProgressReportingTask::begun, &loop, &QEventLoop::quit );
  manager.addTask( failedTask );
  if ( failedTask->status() != QgsTask::Running )
    loop.exec();

  timerThread = new QThread();
  connect( timerThread, &QThread::finished, timerThread, &QThread::deleteLater );
  timer = new QTimer( nullptr );
  connect( timer, &QTimer::timeout, failedTask, &ProgressReportingTask::terminate, Qt::DirectConnection );
  timer->moveToThread( timerThread );
  connect( timerThread, &QThread::started, timer, [ = ]
  {
    timer->start( 500 );
  } );
  connect( timerThread, &QThread::finished, timer, &QTimer::deleteLater );
  timerThread->start();

  QCOMPARE( failedTask->waitForFinished(), true );
  QCOMPARE( failedTask->status(), QgsTask::Terminated );

  timerThread->quit();

  ProgressReportingTask *timeoutTooShortTask = new ProgressReportingTask();
  connect( timeoutTooShortTask, &ProgressReportingTask::begun, &loop, &QEventLoop::quit );
  manager.addTask( timeoutTooShortTask );
  if ( timeoutTooShortTask->status() != QgsTask::Running )
    loop.exec();

  timerThread = new QThread();
  connect( timerThread, &QThread::finished, timerThread, &QThread::deleteLater );
  timer = new QTimer( nullptr );
  connect( timer, &QTimer::timeout, timeoutTooShortTask, &ProgressReportingTask::finish, Qt::DirectConnection );
  timer->moveToThread( timerThread );
  connect( timerThread, &QThread::started, timer, [ = ]
  {
    timer->start( 1000 );
  } );
  connect( timerThread, &QThread::finished, timer, &QTimer::deleteLater );
  timerThread->start();

  QCOMPARE( timeoutTooShortTask->waitForFinished( 20 ), false );
  QCOMPARE( timeoutTooShortTask->status(), QgsTask::Running );

  timerThread->quit();

  flushEvents();
}

void TestQgsTaskManager::waitForFinishedBeforeStart()
{
  // backup max thread count and force it to 1 so there is only one slot in task manager queue
  int maxThreadCount = QThreadPool::globalInstance()->maxThreadCount();
  QThreadPool::globalInstance()->setMaxThreadCount( 1 );

  QgsTaskManager manager;

  // add a wait task so the test task is not started when we call waitforfinished
  QPointer<QgsTask> waitTask = new WaitTask( "wait_task" );
  manager.addTask( waitTask );

  QPointer<QgsTask> testTask = new TestTask( "test_task" );
  manager.addTask( testTask );

  testTask->waitForFinished();

  QgsTask::TaskStatus status = testTask->status();
  QCOMPARE( status, QgsTask::Complete );

  waitTask->waitForFinished();
  QCOMPARE( waitTask->status(), QgsTask::Complete );

  // wait for task to be removed from active task
  while ( manager.count() > 0 )
  {
    QCoreApplication::processEvents();
  }

  // restore max thread count (for other tests)
  QThreadPool::globalInstance()->setMaxThreadCount( maxThreadCount );

  flushEvents();
}

void TestQgsTaskManager::progressChanged()
{
  // check that progressChanged signals emitted by tasks result in progressChanged signal from manager
  QgsTaskManager manager;
  ProgressReportingTask *task = new ProgressReportingTask();
  ProgressReportingTask *task2 = new ProgressReportingTask();

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
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( spy.last().at( 1 ).toDouble(), 50.0 );
  //multiple running tasks, so finalTaskProgressChanged(double) should not be emitted
  QCOMPARE( spy2.count(), 0 );

  task2->emitProgressChanged( 75.0 );
  QCOMPARE( task2->progress(), 75.0 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 2LL );
  QCOMPARE( spy.last().at( 1 ).toDouble(), 75.0 );
  QCOMPARE( spy2.count(), 0 );

  task->finish();
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

  ProgressReportingTask *task3 = new ProgressReportingTask();
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

  task2->finish();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  task3->emitProgressChanged( 30.0 );
  //single running task, so finalTaskProgressChanged(double) should be emitted
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( spy2.last().at( 0 ).toDouble(), 30.0 );
  task3->finish();
}

void TestQgsTaskManager::statusChanged()
{
  // check that statusChanged signals emitted by tasks result in statusChanged signal from manager
  QgsTaskManager manager;
  ProgressReportingTask *task = new ProgressReportingTask();
  ProgressReportingTask *task2 = new ProgressReportingTask();

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
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 2LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( spy.last().at( 1 ).toInt() ), QgsTask::Running );

  task->terminate();
  while ( task->status() == QgsTask::Running || manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( spy.last().at( 1 ).toInt() ), QgsTask::Terminated );

  task2->finish();
  while ( task2->status() == QgsTask::Running || manager.countActiveTasks() > 0 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toLongLong(), 2LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( spy.last().at( 1 ).toInt() ), QgsTask::Complete );
}

void TestQgsTaskManager::allTasksFinished()
{
  // check that allTasksFinished signal is correctly emitted by manager
  QgsTaskManager manager;
  ProgressReportingTask *task = new ProgressReportingTask();
  ProgressReportingTask *task2 = new ProgressReportingTask();
  manager.addTask( task );
  manager.addTask( task2 );
  while ( task->status() != QgsTask::Running || task2->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();

  QSignalSpy spy( &manager, &QgsTaskManager::allTasksFinished );

  task->terminate();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 0 );
  task2->finish();
  while ( manager.countActiveTasks() > 0 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 1 );

  ProgressReportingTask *task3 = new ProgressReportingTask();
  ProgressReportingTask *task4 = new ProgressReportingTask();
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
  task3->terminate();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 1 );
  ProgressReportingTask *task5 = new ProgressReportingTask();
  manager.addTask( task5 );
  while ( task5->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  task4->terminate();
  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( spy.count(), 1 );
  task5->terminate();
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
  ProgressReportingTask *task = new ProgressReportingTask();
  ProgressReportingTask *task2 = new ProgressReportingTask();
  QSignalSpy spy( &manager, &QgsTaskManager::countActiveTasksChanged );
  manager.addTask( task );
  while ( task->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask * >() << task ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 1 );
  manager.addTask( task2 );
  while ( task2->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask * >() << task << task2 ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 2 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 2 );
  task->finish();
  while ( task->status() == QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( manager.activeTasks().toSet(), ( QList< QgsTask * >() << task2 ).toSet() );
  QCOMPARE( manager.countActiveTasks(), 1 );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toInt(), 1 );
  task2->finish();
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
  CancelableTask *task = new CancelableTask();
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
  task->cancel();
}

void TestQgsTaskManager::dependencies()
{
  QgsTaskManager manager;

  //test that canceling tasks cancels all tasks which are dependent on them
  CancelableTask *task = new CancelableTask();
  task->hold();
  CancelableTask *childTask = new CancelableTask();
  childTask->hold();
  CancelableTask *grandChildTask = new CancelableTask();
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

  // test that tasks are queued until dependencies are resolved
  task = new CancelableTask();
  childTask = new CancelableTask();
  childTask->hold();
  taskId = manager.addTask( QgsTaskManager::TaskDefinition( task, QgsTaskList() << childTask ) );
  childTaskId = manager.addTask( childTask );
  QVERIFY( !manager.dependenciesSatisfied( taskId ) );
  QVERIFY( manager.dependenciesSatisfied( childTaskId ) );

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
  childTask->cancel(); // Note: CancelableTask signals successful completion when canceled!
  //wait for childTask to complete
  while ( childTask->isActive() )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QVERIFY( manager.dependenciesSatisfied( taskId ) );
  QCOMPARE( childTask->status(), QgsTask::Complete );
  //wait for task to spin up
  while ( !task->isActive() )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( task->status(), QgsTask::Running );
  task->cancel(); // Note: CancelableTask signals successful completion when canceled!


  // test circular dependency detection
  task = new CancelableTask();
  task->hold();
  childTask = new CancelableTask();
  childTask->hold();
  grandChildTask = new CancelableTask();
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
  QgsVectorLayer *layer1 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "layer1" ), QStringLiteral( "memory" ) );
  QVERIFY( layer1->isValid() );
  QgsVectorLayer *layer2 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "layer2" ), QStringLiteral( "memory" ) );
  QVERIFY( layer2->isValid() );
  QgsVectorLayer *layer3 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string&field=col2:string&field=col3:string" ), QStringLiteral( "layer3" ), QStringLiteral( "memory" ) );
  QVERIFY( layer3->isValid() );
  QgsProject::instance()->addMapLayers( QList< QgsMapLayer * >() << layer1 << layer2 << layer3 );

  QgsTaskManager manager;

  //test that remove layers cancels all tasks which are dependent on them
  TestTask *task = new TestTask();
  task->hold();
  task->setDependentLayers( QList< QgsMapLayer * >() << layer2 << layer3 );
  QCOMPARE( task->dependentLayers(), QList< QgsMapLayer * >() << layer2 << layer3 );
  long taskId = manager.addTask( task );
  QCOMPARE( manager.dependentLayers( taskId ), QList< QgsMapLayer * >() << layer2 << layer3 );
  QVERIFY( manager.tasksDependentOnLayer( nullptr ).isEmpty() );
  QCOMPARE( manager.tasksDependentOnLayer( layer2 ), QList< QgsTask * >() << task );
  QCOMPARE( manager.tasksDependentOnLayer( layer3 ), QList< QgsTask * >() << task );

  QCOMPARE( task->status(), QgsTask::OnHold );
  //removing layer1 should have no effect
  QgsProject::instance()->removeMapLayers( QList< QgsMapLayer * >() << layer1 );
  QCOMPARE( task->status(), QgsTask::OnHold );
  //removing layer3 should cancel task
  QgsProject::instance()->removeMapLayers( QList< QgsMapLayer * >() << layer3 );
  while ( task->status() != QgsTask::Terminated )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( task->status(), QgsTask::Terminated );

  QgsProject::instance()->removeMapLayers( QList< QgsMapLayer * >() << layer2 );
}

void TestQgsTaskManager::managerWithSubTasks()
{
  if ( !QgsTest::runFlakyTests() )
    QSKIP( "This test is disabled on Travis CI environment" );

  // parent with subtasks
  ProgressReportingTask *parent = new ProgressReportingTask( QStringLiteral( "parent" ) );
  ProgressReportingTask *subTask = new ProgressReportingTask( QStringLiteral( "subtask" ) );
  ProgressReportingTask *subsubTask = new ProgressReportingTask( QStringLiteral( "subsubtask" ) );
  subTask->addSubTask( subsubTask );
  parent->addSubTask( subTask );

  QgsTaskManager *manager = new QgsTaskManager();
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
  QCOMPARE( spyProgress.last().at( 0 ).toLongLong(), 1LL );
  // subTask itself is 50% done, so with it's child task it's sitting at overall 25% done
  // (one task 50%, one task not started)
  // parent task has two tasks (itself + subTask), and subTask is 25% done.... so parent
  // task is 12.5% done. yes-- these numbers are correct!
  QCOMPARE( spyProgress.last().at( 1 ).toInt(), 13 );

  subsubTask->emitProgressChanged( 100 );
  QCOMPARE( spyProgress.count(), 2 );
  QCOMPARE( spyProgress.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( spyProgress.last().at( 1 ).toInt(), 38 );
  parent->emitProgressChanged( 50 );
  QCOMPARE( spyProgress.count(), 3 );
  QCOMPARE( spyProgress.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( spyProgress.last().at( 1 ).toInt(), 63 );

  //manager should not emit statusChanged signals for subtasks
  QSignalSpy statusSpy( manager, &QgsTaskManager::statusChanged );
  QCOMPARE( statusSpy.count(), 0 );
  subsubTask->finish();
  while ( subsubTask->status() != QgsTask::Complete )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( statusSpy.count(), 1 );
  QCOMPARE( statusSpy.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.last().at( 1 ).toInt() ), QgsTask::Running );

  subTask->finish();
  while ( subTask->status() != QgsTask::Complete )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( statusSpy.count(), 1 );

  parent->finish();
  while ( parent->status() != QgsTask::Complete )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( statusSpy.last().at( 0 ).toLongLong(), 1LL );
  QCOMPARE( static_cast< QgsTask::TaskStatus >( statusSpy.last().at( 1 ).toInt() ), QgsTask::Complete );


  subsubTask->finish();
  subTask->finish();
  parent->finish();
  delete manager;

}

void TestQgsTaskManager::managerWithSubTasks2()
{
  //test dependencies

  //test 1
  QgsTaskManager *manager2 = new QgsTaskManager();
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
  TestTask *parent = new TestTask( QStringLiteral( "parent" ) );
  parent->hold();
  TestTask *subTask = new TestTask( QStringLiteral( "subtask" ) );
  subTask->hold();
  TestTask *subTask2 = new TestTask( QStringLiteral( "subtask2" ) );
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

void TestQgsTaskManager::cancelBeforeStart()
{
  // add a lot of tasks to the manager, so that some are queued and can't start immediately
  // then cancel them all!
  QList< QgsTask * > tasks;
  QgsTaskManager manager;
  for ( int i = 0; i < 30; ++i )
  {
    QgsTask *task = new CancelableTask();
    tasks << task;
    manager.addTask( task );
  }

  for ( QgsTask *t : qgis::as_const( tasks ) )
  {
    t->cancel();
  }

  for ( QgsTask *t : qgis::as_const( tasks ) )
  {
    t->waitForFinished();
  }

  while ( manager.countActiveTasks() > 1 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
}

void TestQgsTaskManager::proxyTask()
{
  if ( !QgsTest::runFlakyTests() )
    QSKIP( "This test is disabled on Travis CI environment" );

  QgsProxyProgressTask *proxyTask = new QgsProxyProgressTask( QString() );

  // finalize before task gets a chance to start
  QgsTaskManager manager;
  proxyTask->finalize( false );
  QPointer< QgsTask > p( proxyTask );

  manager.addTask( proxyTask );

  // should all be ok, no deadlock...
  while ( p )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
}

void TestQgsTaskManager::proxyTask2()
{
  if ( !QgsTest::runFlakyTests() )
    QSKIP( "This test is disabled on Travis CI environment" );

  QgsProxyProgressTask *proxyTask = new QgsProxyProgressTask( QString() );

  // finalize before task gets a chance to start
  QgsTaskManager manager;
  QPointer< QgsTask > p( proxyTask );
  manager.addTask( proxyTask );

  // should all be ok, no deadlock...
  while ( proxyTask->status() != QgsTask::Running )
  {
    QCoreApplication::processEvents();
  }
  proxyTask->finalize( false );
  while ( p )
  {
    QCoreApplication::processEvents();
  }

  flushEvents();
}

void TestQgsTaskManager::scopedProxyTask()
{
  if ( !QgsTest::runFlakyTests() )
    QSKIP( "This test is disabled on Travis CI environment" );

  {
    // task finishes before it can start
    QgsScopedProxyProgressTask task{ QString() };
  }

  // should all be ok, no deadlock...
  while ( QgsApplication::taskManager()->countActiveTasks() == 0 )
  {
    QCoreApplication::processEvents();
  }
  while ( QgsApplication::taskManager()->countActiveTasks() > 0 )
  {
    QCoreApplication::processEvents();
  }
  flushEvents();
}

QGSTEST_MAIN( TestQgsTaskManager )
#include "testqgstaskmanager.moc"
