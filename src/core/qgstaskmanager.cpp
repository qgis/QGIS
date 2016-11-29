/***************************************************************************
                          qgstaskmanager.cpp
                          ------------------
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
#include <QtConcurrentRun>


//
// QgsTask
//

QgsTask::QgsTask( const QString &name, const Flags& flags )
    : QObject()
    , mFlags( flags )
    , mDescription( name )
    , mStatus( Queued )
    , mOverallStatus( Queued )
    , mProgress( 0.0 )
    , mTotalProgress( 0.0 )
    , mShouldTerminate( false )
    , mStartCount( 0 )
{
  setAutoDelete( false );
}

QgsTask::~QgsTask()
{
  Q_ASSERT_X( mStatus != Running, "delete", QString( "status was %1" ).arg( mStatus ).toLatin1() );

  QThreadPool::globalInstance()->cancel( this );

  Q_FOREACH ( const SubTask& subTask, mSubTasks )
  {
    delete subTask.task;
  }
}

void QgsTask::run()
{
  mStartCount++;
  Q_ASSERT( mStartCount == 1 );

  if ( mStatus != Queued )
    return;

  mStatus = Running;
  mOverallStatus = Running;
  emit statusChanged( Running );
  emit begun();
  TaskResult result = _run();
  switch ( result )
  {
    case ResultSuccess:
      completed();
      break;

    case ResultFail:
      terminated();
      break;

    case ResultPending:
      // nothing to do - task will call completed() or stopped()
      // in it's own time
      break;

  }
}

void QgsTask::cancel()
{
#if QT_VERSION < 0x050500
  //can't cancel with qt < 5.5
  return;
#else

  mShouldTerminate = true;
  QThreadPool::globalInstance()->cancel( this );

  if ( mStatus == Queued || mStatus == OnHold )
  {
    // immediately terminate unstarted jobs
    terminated();
  }
  else if ( mStatus == Terminated )
  {
    processSubTasksForTermination();
  }

  Q_FOREACH ( const SubTask& subTask, mSubTasks )
  {
    subTask.task->cancel();
  }
#endif
}

void QgsTask::hold()
{
  if ( mStatus == Queued )
  {
    mStatus = OnHold;
    processSubTasksForHold();
  }

  Q_FOREACH ( const SubTask& subTask, mSubTasks )
  {
    subTask.task->hold();
  }
}

void QgsTask::unhold()
{
  if ( mStatus == OnHold )
  {
    mStatus = Queued;
    mOverallStatus = Queued;
    emit statusChanged( Queued );
  }

  Q_FOREACH ( const SubTask& subTask, mSubTasks )
  {
    subTask.task->unhold();
  }
}

void QgsTask::addSubTask( QgsTask* subTask, const QgsTaskList& dependencies,
                          SubTaskDependency subTaskDependency )
{
  mSubTasks << SubTask( subTask, dependencies, subTaskDependency );
  connect( subTask, &QgsTask::progressChanged, this, [=] { setProgress( mProgress ); } );
  connect( subTask, &QgsTask::statusChanged, this, &QgsTask::subTaskStatusChanged );
}

void QgsTask::subTaskStatusChanged( int status )
{
  QgsTask* subTask = qobject_cast< QgsTask* >( sender() );
  if ( !subTask )
    return;

  if ( status == Running && mStatus == Queued )
  {
    mOverallStatus = Running;
  }
  else if ( status == Complete && mStatus == Complete )
  {
    //check again if all subtasks are complete
    processSubTasksForCompletion();
  }
  else if (( status == Complete || status == Terminated ) && mStatus == Terminated )
  {
    //check again if all subtasks are terminated
    processSubTasksForTermination();
  }
  else if (( status == Complete || status == Terminated || status == OnHold ) && mStatus == OnHold )
  {
    processSubTasksForHold();
  }
  else if ( status == Terminated )
  {
    //uh oh...
    cancel();
  }
}

void QgsTask::setProgress( double progress )
{
  mProgress = progress;

  if ( !mSubTasks.isEmpty() )
  {
    // calculate total progress including subtasks

    double totalProgress = 0.0;
    Q_FOREACH ( const SubTask& subTask, mSubTasks )
    {
      if ( subTask.task->status() == QgsTask::Complete )
      {
        totalProgress += 100.0;
      }
      else
      {
        totalProgress += subTask.task->progress();
      }
    }
    progress = ( progress + totalProgress ) / ( mSubTasks.count() + 1 );
  }

  mTotalProgress = progress;
  emit progressChanged( mTotalProgress );
}

void QgsTask::completed()
{
  mStatus = Complete;
  processSubTasksForCompletion();
}

void QgsTask::processSubTasksForCompletion()
{
  bool subTasksCompleted = true;
  Q_FOREACH ( const SubTask& subTask, mSubTasks )
  {
    if ( subTask.task->status() != Complete )
    {
      subTasksCompleted = false;
      break;
    }
  }

  if ( mStatus == Complete && subTasksCompleted )
  {
    mOverallStatus = Complete;
    setProgress( 100.0 );
    emit statusChanged( Complete );
    emit taskCompleted();
  }
  else if ( mStatus == Complete )
  {
    // defer completion until all subtasks are complete
    mOverallStatus = Running;
  }
}

void QgsTask::processSubTasksForTermination()
{
  bool subTasksTerminated = true;
  Q_FOREACH ( const SubTask& subTask, mSubTasks )
  {
    if ( subTask.task->status() != Terminated && subTask.task->status() != Complete )
    {
      subTasksTerminated = false;
      break;
    }
  }

  if ( mStatus == Terminated && subTasksTerminated && mOverallStatus != Terminated )
  {
    mOverallStatus = Terminated;
    emit statusChanged( Terminated );
    emit taskTerminated();
  }
  else if ( mStatus == Terminated && !subTasksTerminated )
  {
    // defer termination until all subtasks are terminated (or complete)
    mOverallStatus = Running;
  }
}

void QgsTask::processSubTasksForHold()
{
  bool subTasksRunning = false;
  Q_FOREACH ( const SubTask& subTask, mSubTasks )
  {
    if ( subTask.task->status() == Running )
    {
      subTasksRunning = true;
      break;
    }
  }

  if ( mStatus == OnHold && !subTasksRunning && mOverallStatus != OnHold )
  {
    mOverallStatus = OnHold;
    emit statusChanged( OnHold );
  }
  else if ( mStatus == OnHold && subTasksRunning )
  {
    // defer hold until all subtasks finish running
    mOverallStatus = Running;
  }
}

void QgsTask::terminated()
{
  mStatus = Terminated;
  processSubTasksForTermination();
}


//
// QgsTaskManager
//

QgsTaskManager::QgsTaskManager( QObject* parent )
    : QObject( parent )
    , mTaskMutex( new QReadWriteLock() )
    , mActiveTaskMutex( new QReadWriteLock() )
    , mParentTaskMutex( new QReadWriteLock() )
    , mSubTaskMutex( new QReadWriteLock() )
    , mDependenciesMutex( new QReadWriteLock() )
    , mLayerDependenciesMutex( new QReadWriteLock() )
    , mNextTaskId( 0 )
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ),
           this, SLOT( layersWillBeRemoved( QStringList ) ) );
}

QgsTaskManager::~QgsTaskManager()
{
  //first tell all tasks to cancel
  cancelAll();

  //then clean them up, including waiting for them to terminate
  mTaskMutex->lockForRead();
  QMap< long, TaskInfo > tasks = mTasks;
  mTasks.detach();
  mTaskMutex->unlock();
  QMap< long, TaskInfo >::const_iterator it = tasks.constBegin();
  for ( ; it != tasks.constEnd(); ++it )
  {
    cleanupAndDeleteTask( it.value().task );
  }

  delete mTaskMutex;
  delete mActiveTaskMutex;
  delete mSubTaskMutex;
  delete mParentTaskMutex;
  delete mDependenciesMutex;
  delete mLayerDependenciesMutex;
}

long QgsTaskManager::addTask( QgsTask* task, int priority )
{
  return addTaskPrivate( task, QgsTaskList(), false, priority );
}

long QgsTaskManager::addTask( const QgsTaskManager::TaskDefinition& definition, int priority )
{
  return addTaskPrivate( definition.task,
                         definition.dependencies,
                         false,
                         priority );
}


long QgsTaskManager::addTaskPrivate( QgsTask* task, QgsTaskList dependencies, bool isSubTask, int priority )
{
  long taskId = mNextTaskId++;

  mTaskMutex->lockForWrite();
  mTasks.insert( taskId, TaskInfo( task, priority ) );
  mTaskMutex->unlock();

  if ( isSubTask )
  {
    mSubTaskMutex->lockForWrite();
    mSubTasks << task;
    mSubTaskMutex->unlock();
  }
  else
  {
    mParentTaskMutex->lockForWrite();
    mParentTasks << task;
    mParentTaskMutex->unlock();
  }

  connect( task, &QgsTask::statusChanged, this, &QgsTaskManager::taskStatusChanged );
  if ( !isSubTask )
  {
    connect( task, &QgsTask::progressChanged, this, &QgsTaskManager::taskProgressChanged );
  }

  // add all subtasks, must be done before dependency resolution
  Q_FOREACH ( const QgsTask::SubTask& subTask, task->mSubTasks )
  {
    switch ( subTask.dependency )
    {
      case QgsTask::ParentDependsOnSubTask:
        dependencies << subTask.task;
        break;

      case QgsTask::SubTaskIndependent:
        //nothing
        break;
    }
    //recursively add sub tasks
    addTaskPrivate( subTask.task, subTask.dependencies, true, priority );
  }

  if ( !dependencies.isEmpty() )
  {
    mTaskDependencies.insert( taskId, dependencies );
  }

  if ( hasCircularDependencies( taskId ) )
  {
    task->cancel();
  }

  if ( !isSubTask )
  {
    emit taskAdded( taskId );
    processQueue();
  }

  return taskId;
}

QgsTask* QgsTaskManager::task( long id ) const
{
  QReadLocker ml( mTaskMutex );
  QgsTask* t = nullptr;
  if ( mTasks.contains( id ) )
    t = mTasks.value( id ).task;
  return t;
}

QList<QgsTask*> QgsTaskManager::tasks() const
{
  QReadLocker ml( mParentTaskMutex );
  return mParentTasks.toList();
}

int QgsTaskManager::count() const
{
  QReadLocker ml( mParentTaskMutex );
  return mParentTasks.count();
}

long QgsTaskManager::taskId( QgsTask *task ) const
{
  if ( !task )
    return -1;

  QReadLocker ml( mTaskMutex );
  QMap< long, TaskInfo >::const_iterator it = mTasks.constBegin();
  for ( ; it != mTasks.constEnd(); ++it )
  {
    if ( it.value().task == task )
    {
      return it.key();
    }
  }
  return -1;
}

void QgsTaskManager::cancelAll()
{
  mParentTaskMutex->lockForRead();
  QSet< QgsTask* > parents = mParentTasks;
  parents.detach();
  mParentTaskMutex->unlock();

  Q_FOREACH ( QgsTask* task, parents )
  {
    task->cancel();
  }
}

bool QgsTaskManager::dependenciesSatisified( long taskId ) const
{
  mDependenciesMutex->lockForRead();
  QMap< long, QgsTaskList > dependencies = mTaskDependencies;
  dependencies.detach();
  mDependenciesMutex->unlock();

  if ( !dependencies.contains( taskId ) )
    return true;

  Q_FOREACH ( QgsTask* task, dependencies.value( taskId ) )
  {
    if ( task->status() != QgsTask::Complete )
      return false;
  }

  return true;
}

QSet<long> QgsTaskManager::dependencies( long taskId ) const
{
  QSet<long> results;
  if ( resolveDependencies( taskId, taskId, results ) )
    return results;
  else
    return QSet<long>();
}

bool QgsTaskManager::resolveDependencies( long firstTaskId, long currentTaskId, QSet<long>& results ) const
{
  mDependenciesMutex->lockForRead();
  QMap< long, QgsTaskList > dependencies = mTaskDependencies;
  dependencies.detach();
  mDependenciesMutex->unlock();

  if ( !dependencies.contains( currentTaskId ) )
    return true;

  Q_FOREACH ( QgsTask* task, dependencies.value( currentTaskId ) )
  {
    long dependentTaskId = taskId( task );
    if ( dependentTaskId >= 0 )
    {
      if ( dependentTaskId == firstTaskId )
        // circular
        return false;

      //add task as dependent
      results.insert( dependentTaskId );
      //plus all its other dependencies
      QSet< long > newTaskDeps;
      if ( !resolveDependencies( firstTaskId, dependentTaskId, newTaskDeps ) )
        return false;

      if ( newTaskDeps.contains( firstTaskId ) )
      {
        // circular
        return false;
      }

      results.unite( newTaskDeps );
    }
  }

  return true;
}

bool QgsTaskManager::hasCircularDependencies( long taskId ) const
{
  QSet< long > d;
  return !resolveDependencies( taskId, taskId, d );
}

void QgsTaskManager::setDependentLayers( long taskId, const QStringList& layerIds )
{
  QWriteLocker ml( mLayerDependenciesMutex );
  mLayerDependencies.insert( taskId, layerIds );
}

QStringList QgsTaskManager::dependentLayers( long taskId ) const
{
  QReadLocker ml( mLayerDependenciesMutex );
  return mLayerDependencies.value( taskId, QStringList() );
}

QList<QgsTask*> QgsTaskManager::activeTasks() const
{
  QReadLocker ml( mActiveTaskMutex );
  QReadLocker pl( mParentTaskMutex );
  QSet< QgsTask* > activeTasks = mActiveTasks;
  activeTasks.intersect( mParentTasks );
  return activeTasks.toList();
}

int QgsTaskManager::countActiveTasks() const
{
  QReadLocker ml( mActiveTaskMutex );
  QReadLocker pl( mParentTaskMutex );
  QSet< QgsTask* > tasks = mActiveTasks;
  return tasks.intersect( mParentTasks ).count();
}

void QgsTaskManager::taskProgressChanged( double progress )
{
  QgsTask* task = qobject_cast< QgsTask* >( sender() );

  //find ID of task
  long id = taskId( task );
  if ( id < 0 )
    return;

  emit progressChanged( id, progress );

  if ( countActiveTasks() == 1 )
  {
    emit finalTaskProgressChanged( progress );
  }
}

void QgsTaskManager::taskStatusChanged( int status )
{
  QgsTask* task = qobject_cast< QgsTask* >( sender() );

  //find ID of task
  long id = taskId( task );
  if ( id < 0 )
    return;

  if ( status == QgsTask::Terminated || status == QgsTask::Complete )
  {
    QgsTask::TaskResult result = status == QgsTask::Complete ? QgsTask::ResultSuccess
                                 : QgsTask::ResultFail;
    task->finished( result );
  }

  if ( status == QgsTask::Terminated )
  {
    //recursively cancel dependant tasks
    cancelDependentTasks( id );
  }

  mParentTaskMutex->lockForRead();
  bool isParent = mParentTasks.contains( task );
  mParentTaskMutex->unlock();
  if ( isParent )
  {
    // don't emit status changed for subtasks
    emit statusChanged( id, status );
  }

  processQueue();

  if ( status == QgsTask::Terminated || status == QgsTask::Complete )
  {
    cleanupAndDeleteTask( task );
  }

}

void QgsTaskManager::layersWillBeRemoved( const QStringList& layerIds )
{
  mLayerDependenciesMutex->lockForRead();
  // scan through layers to be removed
  QMap< long, QStringList > layerDependencies = mLayerDependencies;
  layerDependencies.detach();
  mLayerDependenciesMutex->unlock();

  Q_FOREACH ( const QString& layerId, layerIds )
  {
    // scan through tasks with layer dependencies
    for ( QMap< long, QStringList >::const_iterator it = layerDependencies.constBegin();
          it != layerDependencies.constEnd(); ++it )
    {
      if ( !it.value().contains( layerId ) )
      {
        //task not dependent on this layer
        continue;
      }

      QgsTask* dependentTask = task( it.key() );
      if ( dependentTask && ( dependentTask->status() != QgsTask::Complete || dependentTask->status() != QgsTask::Terminated ) )
      {
        // incomplete task is dependent on this layer!
        dependentTask->cancel();
      }
    }
  }
}


bool QgsTaskManager::cleanupAndDeleteTask( QgsTask *task )
{
  if ( !task )
    return false;

  long id = taskId( task );
  if ( id < 0 )
    return false;

  task->disconnect( this );

  mDependenciesMutex->lockForWrite();
  if ( mTaskDependencies.contains( id ) )
    mTaskDependencies.remove( id );
  mDependenciesMutex->unlock();

  emit taskAboutToBeDeleted( id );

  mParentTaskMutex->lockForRead();
  bool isParent = mParentTasks.contains( task );
  mParentTaskMutex->unlock();

  mParentTaskMutex->lockForWrite();
  mParentTasks.remove( task );
  mParentTaskMutex->unlock();

  mSubTaskMutex->lockForWrite();
  mSubTasks.remove( task );
  mSubTaskMutex->unlock();

  mTaskMutex->lockForWrite();
  mTasks.remove( id );
  mTaskMutex->unlock();

  mLayerDependenciesMutex->lockForWrite();
  mLayerDependencies.remove( id );
  mLayerDependenciesMutex->unlock();

  if ( task->status() != QgsTask::Complete || task->status() != QgsTask::Terminated )
  {
    if ( isParent )
    {
      // delete task when it's terminated
      connect( task, &QgsTask::taskCompleted, task, &QgsTask::deleteLater );
      connect( task, &QgsTask::taskTerminated, task, &QgsTask::deleteLater );
    }
    task->cancel();
  }
  else
  {
#if QT_VERSION >= 0x050500
    QThreadPool::globalInstance()->cancel( task );
#endif
    if ( isParent )
    {
      //task already finished, kill it
      task->deleteLater();
    }
  }

  // at this stage (hopefully) dependent tasks have been cancelled or queued
  mDependenciesMutex->lockForWrite();
  for ( QMap< long, QgsTaskList >::iterator it = mTaskDependencies.begin(); it != mTaskDependencies.end(); ++it )
  {
    if ( it.value().contains( task ) )
    {
      it.value().removeAll( task );
    }
  }
  mDependenciesMutex->unlock();

  return true;
}

void QgsTaskManager::processQueue()
{
  int prevActiveCount = countActiveTasks();
  mActiveTaskMutex->lockForWrite();
  mActiveTasks.clear();
  mTaskMutex->lockForWrite();
  for ( QMap< long, TaskInfo >::iterator it = mTasks.begin(); it != mTasks.end(); ++it )
  {
    QgsTask* task = it.value().task;
    if ( task && task->mStatus == QgsTask::Queued && dependenciesSatisified( it.key() ) && it.value().added.testAndSetRelaxed( 0, 1 ) )
    {
      QThreadPool::globalInstance()->start( task, it.value().priority );
    }

    if ( task && ( task->mStatus != QgsTask::Complete && task->mStatus != QgsTask::Terminated ) )
    {
      mActiveTasks << task;
    }
  }
  mTaskMutex->unlock();
  mActiveTaskMutex->unlock();

  mParentTaskMutex->lockForRead();
  bool allFinished = mActiveTasks.isEmpty();
  mParentTaskMutex->unlock();
  if ( allFinished )
  {
    emit allTasksFinished();
  }

  int newActiveCount = countActiveTasks();
  if ( prevActiveCount != newActiveCount )
  {
    emit countActiveTasksChanged( newActiveCount );
  }
}

void QgsTaskManager::cancelDependentTasks( long taskId )
{
  QgsTask* cancelledTask = task( taskId );

  //deep copy
  mDependenciesMutex->lockForRead();
  QMap< long, QgsTaskList > taskDependencies = mTaskDependencies;
  taskDependencies.detach();
  mDependenciesMutex->unlock();

  QMap< long, QgsTaskList >::const_iterator it = taskDependencies.constBegin();
  for ( ; it != taskDependencies.constEnd(); ++it )
  {
    if ( it.value().contains( cancelledTask ) )
    {
      // found task with this dependancy

      // cancel it - note that this will be recursive, so any tasks dependant
      // on this one will also be cancelled
      QgsTask* dependentTask = task( it.key() );
      if ( dependentTask )
        dependentTask->cancel();
    }
  }
}
