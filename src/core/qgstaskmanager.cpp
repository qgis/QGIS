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
    , mProgress( 0.0 )
    , mShouldTerminate( false )
{}

void QgsTask::start()
{
  mStatus = Running;
  emit statusChanged( Running );
  emit begun();
  TaskResult result = run();
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
  mShouldTerminate = true;
  if ( mStatus == Queued || mStatus == OnHold )
  {
    // immediately terminate unstarted jobs
    terminated();
  }
}

void QgsTask::hold()
{
  if ( mStatus == Queued )
  {
    mStatus = OnHold;
    emit statusChanged( OnHold );
  }
}

void QgsTask::unhold()
{
  if ( mStatus == OnHold )
  {
    mStatus = Queued;
    emit statusChanged( Queued );
  }
}

void QgsTask::setProgress( double progress )
{
  mProgress = progress;
  emit progressChanged( progress );
}

void QgsTask::completed()
{
  mStatus = Complete;
  emit statusChanged( Complete );
  emit taskCompleted();
}

void QgsTask::terminated()
{
  mStatus = Terminated;
  emit statusChanged( Terminated );
  emit taskTerminated();
}


//
// QgsTaskManager
//

QgsTaskManager::QgsTaskManager( QObject* parent )
    : QObject( parent )
    , mTaskMutex( new QMutex( QMutex::Recursive ) )
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
  mTaskMutex->lock();
  QMap< long, TaskInfo >::const_iterator it = mTasks.constBegin();
  for ( ; it != mTasks.constEnd(); ++it )
  {
    cleanupAndDeleteTask( it.value().task );
  }
  mTaskMutex->unlock();

  delete mTaskMutex;
}

long QgsTaskManager::addTask( QgsTask* task, const QgsTaskList& dependencies )
{
  QMutexLocker ml( mTaskMutex );
  mTasks.insert( mNextTaskId, task );

  connect( task, &QgsTask::progressChanged, this, &QgsTaskManager::taskProgressChanged );
  connect( task, &QgsTask::statusChanged, this, &QgsTaskManager::taskStatusChanged );

  if ( !dependencies.isEmpty() )
  {
    mTaskDependencies.insert( mNextTaskId, dependencies );
  }

  if ( hasCircularDependencies( mNextTaskId ) )
  {
    task->cancel();
  }

  emit taskAdded( mNextTaskId );
  processQueue();

  return mNextTaskId++;
}

QgsTask* QgsTaskManager::task( long id ) const
{
  QMutexLocker ml( mTaskMutex );
  return mTasks.value( id ).task;
}

QList<QgsTask*> QgsTaskManager::tasks() const
{
  QMutexLocker ml( mTaskMutex );
  QList< QgsTask* > list;
  for ( QMap< long, TaskInfo >::const_iterator it = mTasks.constBegin(); it != mTasks.constEnd(); ++it )
  {
    list << it.value().task;
  }
  return list;
}

long QgsTaskManager::taskId( QgsTask *task ) const
{
  if ( !task )
    return -1;

  QMutexLocker ml( mTaskMutex );
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
  QMutexLocker ml( mTaskMutex );
  QMap< long, TaskInfo >::const_iterator it = mTasks.constBegin();
  for ( ; it != mTasks.constEnd(); ++it )
  {
    QgsTask* task = it.value().task;
    if ( task->isActive() )
    {
      task->cancel();
    }
  }
}

bool QgsTaskManager::dependenciesSatisified( long taskId ) const
{
  QMutexLocker ml( mTaskMutex );

  if ( !mTaskDependencies.contains( taskId ) )
    return true;

  Q_FOREACH ( QgsTask* task, mTaskDependencies.value( taskId ) )
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
  QMutexLocker ml( mTaskMutex );

  if ( !mTaskDependencies.contains( currentTaskId ) )
    return true;

  Q_FOREACH ( QgsTask* task, mTaskDependencies.value( currentTaskId ) )
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
  QMutexLocker ml( mTaskMutex );
  mLayerDependencies.insert( taskId, layerIds );
}

QStringList QgsTaskManager::dependentLayers( long taskId ) const
{
  QMutexLocker ml( mTaskMutex );
  return mLayerDependencies.value( taskId, QStringList() );
}

QList<QgsTask*> QgsTaskManager::activeTasks() const
{
  QMutexLocker ml( mTaskMutex );
  QList< QgsTask* > taskList = mActiveTasks;
  taskList.detach();
  return taskList;
}

int QgsTaskManager::countActiveTasks() const
{
  QMutexLocker ml( mTaskMutex );
  return mActiveTasks.count();
}

void QgsTaskManager::taskProgressChanged( double progress )
{
  QMutexLocker ml( mTaskMutex );
  QgsTask* task = qobject_cast< QgsTask* >( sender() );

  //find ID of task
  long id = taskId( task );
  if ( id < 0 )
    return;

  emit progressChanged( id, progress );
  if ( mActiveTasks.count() == 1 )
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

  emit statusChanged( id, status );
  processQueue();
}

void QgsTaskManager::layersWillBeRemoved( const QStringList& layerIds )
{
  QMutexLocker ml( mTaskMutex );
  // scan through layers to be removed
  Q_FOREACH ( const QString& layerId, layerIds )
  {
    // scan through tasks with layer dependencies
    for ( QMap< long, QStringList >::const_iterator it = mLayerDependencies.constBegin();
          it != mLayerDependencies.constEnd(); ++it )
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

  emit taskAboutToBeDeleted( taskId( task ) );

  if ( task->isActive() )
  {
    task->cancel();
    // delete task when it's terminated
    connect( task, &QgsTask::taskCompleted, task, &QgsTask::deleteLater );
    connect( task, &QgsTask::taskTerminated, task, &QgsTask::deleteLater );
  }
  else
  {
    //task already finished, kill it
    task->deleteLater();
  }

  return true;
}

void QgsTaskManager::processQueue()
{
  QMutexLocker ml( mTaskMutex );
  int prevActiveCount = mActiveTasks.count();
  mActiveTasks.clear();
  for ( QMap< long, TaskInfo >::iterator it = mTasks.begin(); it != mTasks.end(); ++it )
  {
    QgsTask* task = it.value().task;
    if ( task && task->status() == QgsTask::Queued && dependenciesSatisified( taskId( task ) ) )
    {
      mTasks[ it.key()].future = QtConcurrent::run( task, &QgsTask::start );
    }

    if ( task && ( task->status() != QgsTask::Complete && task->status() != QgsTask::Terminated ) )
    {
      mActiveTasks << task;
    }
  }

  if ( mActiveTasks.isEmpty() )
  {
    emit allTasksFinished();
  }
  if ( prevActiveCount != mActiveTasks.count() )
  {
    emit countActiveTasksChanged( mActiveTasks.count() );
  }
}

void QgsTaskManager::cancelDependentTasks( long taskId )
{
  QMutexLocker ml( mTaskMutex );

  QgsTask* cancelledTask = task( taskId );
  for ( QMap< long, QgsTaskList >::iterator it = mTaskDependencies.begin(); it != mTaskDependencies.end(); ++it )
  {
    if ( it.value().contains( cancelledTask ) )
    {
      // found task with this dependancy

      // cancel it - note that this will be recursive, so any tasks dependant
      // on this one will also be cancelled
      task( it.key() )->cancel();
    }
  }
}
