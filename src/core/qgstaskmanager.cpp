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
#include "qgsproject.h"
#include "qgsmaplayerlistutils_p.h"
#include <mutex>
#include <QStack>
#include <QtConcurrentRun>


//
// QgsTask
//

QgsTask::QgsTask( const QString &name, Flags flags )
  : mFlags( flags )
  , mDescription( name )
  , mNotStartedMutex( 1 )
{
  mNotStartedMutex.acquire();
}

QgsTask::~QgsTask()
{
  Q_ASSERT_X( mStatus != Running, "delete", QStringLiteral( "status was %1" ).arg( mStatus ).toLatin1() );
  // even here we are not sure that task start method has ended
  mNotFinishedMutex.lock();
  const auto constMSubTasks = mSubTasks;
  for ( const SubTask &subTask : constMSubTasks )
  {
    delete subTask.task;
  }
  mNotFinishedMutex.unlock();
  mNotStartedMutex.release();
}

void QgsTask::setDescription( const QString &description )
{
  mDescription = description;
}

qint64 QgsTask::elapsedTime() const
{
  return mElapsedTime.elapsed();
}

void QgsTask::start()
{
  QMutexLocker locker( &mNotFinishedMutex );
  mNotStartedMutex.release();
  mStartCount++;
  Q_ASSERT( mStartCount == 1 );

  if ( mStatus != Queued )
    return;

  mStatus = Running;
  mOverallStatus = Running;
  mElapsedTime.start();

  emit statusChanged( Running );
  emit begun();

  // force initial emission of progressChanged, but respect if task has had initial progress manually set
  setProgress( mProgress );

  if ( run() )
  {
    completed();
  }
  else
  {
    terminated();
  }
}

void QgsTask::cancel()
{
  if ( mOverallStatus == Complete || mOverallStatus == Terminated )
    return;

  mShouldTerminateMutex.lock();
  mShouldTerminate = true;
  mShouldTerminateMutex.unlock();
  if ( mStatus == Queued || mStatus == OnHold )
  {
    // immediately terminate unstarted jobs
    terminated();
    mNotStartedMutex.release();
  }

  if ( mStatus == Terminated )
  {
    processSubTasksForTermination();
  }

  const auto constMSubTasks = mSubTasks;
  for ( const SubTask &subTask : constMSubTasks )
  {
    subTask.task->cancel();
  }
}

bool QgsTask::isCanceled() const
{
  QMutexLocker locker( &mShouldTerminateMutex );
  return mShouldTerminate;
}

void QgsTask::hold()
{
  if ( mStatus == Queued )
  {
    mStatus = OnHold;
    processSubTasksForHold();
  }

  const auto constMSubTasks = mSubTasks;
  for ( const SubTask &subTask : constMSubTasks )
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

  const auto constMSubTasks = mSubTasks;
  for ( const SubTask &subTask : constMSubTasks )
  {
    subTask.task->unhold();
  }
}

void QgsTask::addSubTask( QgsTask *subTask, const QgsTaskList &dependencies,
                          SubTaskDependency subTaskDependency )
{
  mSubTasks << SubTask( subTask, dependencies, subTaskDependency );
  connect( subTask, &QgsTask::progressChanged, this, [this] { setProgress( mProgress ); } );
  connect( subTask, &QgsTask::statusChanged, this, &QgsTask::subTaskStatusChanged );
}

QList<QgsMapLayer *> QgsTask::dependentLayers() const
{
  return _qgis_listQPointerToRaw( mDependentLayers );
}

bool QgsTask::waitForFinished( int timeout )
{
  // We wait the task to be started
  mNotStartedMutex.acquire();

  bool rv = true;
  if ( mOverallStatus == Complete || mOverallStatus == Terminated )
  {
    rv = true;
  }
  else
  {
    if ( timeout == 0 )
      timeout = std::numeric_limits< int >::max();
    if ( mNotFinishedMutex.tryLock( timeout ) )
    {
      mNotFinishedMutex.unlock();
      QCoreApplication::sendPostedEvents( this );
      rv = true;
    }
    else
    {
      rv = false;
    }
  }
  return rv;
}

void QgsTask::setDependentLayers( const QList< QgsMapLayer * > &dependentLayers )
{
  mDependentLayers = _qgis_listRawToQPointer( dependentLayers );
}

void QgsTask::subTaskStatusChanged( int status )
{
  QgsTask *subTask = qobject_cast< QgsTask * >( sender() );
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
  else if ( ( status == Complete || status == Terminated ) && mStatus == Terminated )
  {
    //check again if all subtasks are terminated
    processSubTasksForTermination();
  }
  else if ( ( status == Complete || status == Terminated || status == OnHold ) && mStatus == OnHold )
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
    const auto constMSubTasks = mSubTasks;
    for ( const SubTask &subTask : constMSubTasks )
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

  // avoid flooding with too many events
  double prevProgress = mTotalProgress;
  mTotalProgress = progress;

  // avoid spamming with too many progressChanged reports
  if ( static_cast< int >( prevProgress * 10 ) != static_cast< int >( mTotalProgress * 10 ) )
    emit progressChanged( progress );
}

void QgsTask::completed()
{
  mStatus = Complete;
  QMetaObject::invokeMethod( this, "processSubTasksForCompletion" );
}

void QgsTask::processSubTasksForCompletion()
{
  bool subTasksCompleted = true;
  const auto constMSubTasks = mSubTasks;
  for ( const SubTask &subTask : constMSubTasks )
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
  const auto constMSubTasks = mSubTasks;
  for ( const SubTask &subTask : constMSubTasks )
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
  const auto constMSubTasks = mSubTasks;
  for ( const SubTask &subTask : constMSubTasks )
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
  QMetaObject::invokeMethod( this, "processSubTasksForTermination" );
}


///@cond PRIVATE

class QgsTaskRunnableWrapper : public QRunnable
{
  public:

    explicit QgsTaskRunnableWrapper( QgsTask *task )
      : mTask( task )
    {
      setAutoDelete( true );
    }

    void run() override
    {
      Q_ASSERT( mTask );
      mTask->start();
    }

  private:

    QgsTask *mTask = nullptr;

};

///@endcond



//
// QgsTaskManager
//

QgsTaskManager::QgsTaskManager( QObject *parent )
  : QObject( parent )
  , mThreadPool( new QThreadPool( this ) )
  , mTaskMutex( new QRecursiveMutex() )
{

}

QgsTaskManager::~QgsTaskManager()
{
  //first tell all tasks to cancel
  cancelAll();

  //then clean them up, including waiting for them to terminate
  mTaskMutex->lock();
  QMap< long, TaskInfo > tasks = mTasks;
  mTasks.detach();
  mTaskMutex->unlock();
  QMap< long, TaskInfo >::const_iterator it = tasks.constBegin();
  for ( ; it != tasks.constEnd(); ++it )
  {
    cleanupAndDeleteTask( it.value().task );
  }

  delete mTaskMutex;
  mThreadPool->waitForDone();
}

QThreadPool *QgsTaskManager::threadPool()
{
  return mThreadPool;
}

long QgsTaskManager::addTask( QgsTask *task, int priority )
{
  return addTaskPrivate( task, QgsTaskList(), false, priority );
}

long QgsTaskManager::addTask( const QgsTaskManager::TaskDefinition &definition, int priority )
{
  return addTaskPrivate( definition.task,
                         definition.dependentTasks,
                         false,
                         priority );
}


long QgsTaskManager::addTaskPrivate( QgsTask *task, QgsTaskList dependencies, bool isSubTask, int priority )
{
  if ( !task )
    return 0;

  if ( !mInitialized )
  {
    mInitialized = true;
    // defer connection to project until we actually need it -- we don't want to connect to the project instance in the constructor,
    // cos that forces early creation of QgsProject
    connect( QgsProject::instance(), static_cast < void ( QgsProject::* )( const QList< QgsMapLayer * >& ) > ( &QgsProject::layersWillBeRemoved ),
             this, &QgsTaskManager::layersWillBeRemoved );
  }

  long taskId = mNextTaskId++;

  mTaskMutex->lock();
  mTasks.insert( taskId, TaskInfo( task, priority ) );
  mMapTaskPtrToId[task] = taskId;
  if ( isSubTask )
  {
    mSubTasks << task;
  }
  else
  {
    mParentTasks << task;
  }
  if ( !task->dependentLayers().isEmpty() )
    mLayerDependencies.insert( taskId, _qgis_listRawToQPointer( task->dependentLayers() ) );
  mTaskMutex->unlock();

  connect( task, &QgsTask::statusChanged, this, &QgsTaskManager::taskStatusChanged );
  if ( !isSubTask )
  {
    connect( task, &QgsTask::progressChanged, this, &QgsTaskManager::taskProgressChanged );
  }

  // add all subtasks, must be done before dependency resolution
  for ( const QgsTask::SubTask &subTask : std::as_const( task->mSubTasks ) )
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
    if ( !( task->flags() & QgsTask::Hidden ) )
      emit taskAdded( taskId );

    processQueue();
  }

  return taskId;
}

QgsTask *QgsTaskManager::task( long id ) const
{
  QMutexLocker ml( mTaskMutex );
  QgsTask *t = nullptr;
  if ( mTasks.contains( id ) )
    t = mTasks.value( id ).task;
  return t;
}

QList<QgsTask *> QgsTaskManager::tasks() const
{
  QMutexLocker ml( mTaskMutex );
  return QList<QgsTask *>( mParentTasks.begin(), mParentTasks.end() );
}

int QgsTaskManager::count() const
{
  QMutexLocker ml( mTaskMutex );
  return mParentTasks.count();
}

long QgsTaskManager::taskId( QgsTask *task ) const
{
  if ( !task )
    return -1;

  QMutexLocker ml( mTaskMutex );
  const auto iter = mMapTaskPtrToId.constFind( task );
  if ( iter != mMapTaskPtrToId.constEnd() )
    return *iter;
  return -1;
}

void QgsTaskManager::cancelAll()
{
  mTaskMutex->lock();
  QSet< QgsTask * > parents = mParentTasks;
  parents.detach();
  mTaskMutex->unlock();

  const auto constParents = parents;
  for ( QgsTask *task : constParents )
  {
    task->cancel();
  }
}

bool QgsTaskManager::dependenciesSatisfied( long taskId ) const
{
  mTaskMutex->lock();
  QMap< long, QgsTaskList > dependencies = mTaskDependencies;
  dependencies.detach();
  mTaskMutex->unlock();

  if ( !dependencies.contains( taskId ) )
    return true;

  const auto constValue = dependencies.value( taskId );
  for ( QgsTask *task : constValue )
  {
    if ( task->status() != QgsTask::Complete )
      return false;
  }

  return true;
}

QSet<long> QgsTaskManager::dependencies( long taskId ) const
{
  QSet<long> results;
  if ( resolveDependencies( taskId, results ) )
    return results;
  else
    return QSet<long>();
}

bool QgsTaskManager::resolveDependencies( long thisTaskId, QSet<long> &results ) const
{
  mTaskMutex->lock();
  QMap< long, QgsTaskList > dependencies = mTaskDependencies;
  dependencies.detach();
  mTaskMutex->unlock();

  QSet<long> alreadyExploredTaskIds;
  QStack<long> stackTaskIds;
  stackTaskIds.push( thisTaskId );
  while ( !stackTaskIds.isEmpty() )
  {
    const long currentTaskId = stackTaskIds.pop();
    alreadyExploredTaskIds.insert( currentTaskId );

    auto iter = dependencies.constFind( currentTaskId );
    if ( iter == dependencies.constEnd() )
      continue;

    const auto &constValue = *iter;
    for ( QgsTask *task : constValue )
    {
      const long dependentTaskId = taskId( task );
      if ( dependentTaskId >= 0 )
      {
        if ( thisTaskId == dependentTaskId )
        {
          // circular dependencies
          return false;
        }

        //add task as dependent
        results.insert( dependentTaskId );

        // and add it to the stack of tasks whose dependencies must be resolved
        if ( !alreadyExploredTaskIds.contains( dependentTaskId ) )
        {
          stackTaskIds.push( dependentTaskId );
        }
      }
    }
  }

  return true;
}

bool QgsTaskManager::hasCircularDependencies( long taskId ) const
{
  QSet< long > d;
  return !resolveDependencies( taskId, d );
}

QList<QgsMapLayer *> QgsTaskManager::dependentLayers( long taskId ) const
{
  QMutexLocker ml( mTaskMutex );
  return _qgis_listQPointerToRaw( mLayerDependencies.value( taskId, QgsWeakMapLayerPointerList() ) );
}

QList<QgsTask *> QgsTaskManager::tasksDependentOnLayer( QgsMapLayer *layer ) const
{
  QMutexLocker ml( mTaskMutex );
  QList< QgsTask * > tasks;
  QMap< long, QgsWeakMapLayerPointerList >::const_iterator layerIt = mLayerDependencies.constBegin();
  for ( ; layerIt != mLayerDependencies.constEnd(); ++layerIt )
  {
    if ( _qgis_listQPointerToRaw( layerIt.value() ).contains( layer ) )
    {
      QgsTask *layerTask = task( layerIt.key() );
      if ( layerTask )
        tasks << layerTask;
    }
  }
  return tasks;
}

QList<QgsTask *> QgsTaskManager::activeTasks() const
{
  QMutexLocker ml( mTaskMutex );
  QSet< QgsTask * > activeTasks = mActiveTasks;
  activeTasks.intersect( mParentTasks );
  return QList<QgsTask *>( activeTasks.constBegin(), activeTasks.constEnd() );
}

int QgsTaskManager::countActiveTasks( bool includeHidden ) const
{
  QMutexLocker ml( mTaskMutex );
  QSet< QgsTask * > tasks = mActiveTasks;

  if ( !includeHidden )
  {
    QSet< QgsTask * > filteredTasks;
    filteredTasks.reserve( tasks.size() );
    for ( QgsTask *task : tasks )
    {
      if ( !( task->flags() & QgsTask::Hidden ) )
        filteredTasks.insert( task );
    }
    tasks = filteredTasks;
  }

  return tasks.intersect( mParentTasks ).count();
}

void QgsTaskManager::triggerTask( QgsTask *task )
{
  if ( task )
    emit taskTriggered( task );
}

void QgsTaskManager::taskProgressChanged( double progress )
{
  QgsTask *task = qobject_cast< QgsTask * >( sender() );
  if ( task && task->flags() & QgsTask::Hidden )
    return;

  //find ID of task
  long id = taskId( task );
  if ( id < 0 )
    return;

  emit progressChanged( id, progress );

  if ( countActiveTasks( false ) == 1 )
  {
    emit finalTaskProgressChanged( progress );
  }
}

void QgsTaskManager::taskStatusChanged( int status )
{
  QgsTask *task = qobject_cast< QgsTask * >( sender() );
  const bool isHidden = task && task->flags() & QgsTask::Hidden;

  //find ID of task
  long id = taskId( task );
  if ( id < 0 )
    return;

  mTaskMutex->lock();
  QgsTaskRunnableWrapper *runnable = mTasks.value( id ).runnable;
  mTaskMutex->unlock();
  if ( runnable && mThreadPool->tryTake( runnable ) )
  {
    delete runnable;
    mTasks[ id ].runnable = nullptr;
  }

  if ( status == QgsTask::Terminated || status == QgsTask::Complete )
  {
    bool result = status == QgsTask::Complete;
    task->finished( result );
  }

  if ( status == QgsTask::Terminated )
  {
    //recursively cancel dependent tasks
    cancelDependentTasks( id );
  }

  mTaskMutex->lock();
  bool isParent = mParentTasks.contains( task );
  mTaskMutex->unlock();
  if ( isParent && !isHidden )
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

void QgsTaskManager::layersWillBeRemoved( const QList< QgsMapLayer * > &layers )
{
  mTaskMutex->lock();
  // scan through layers to be removed
  QMap< long, QgsWeakMapLayerPointerList > layerDependencies = mLayerDependencies;
  layerDependencies.detach();
  mTaskMutex->unlock();

  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    // scan through tasks with layer dependencies
    for ( QMap< long, QgsWeakMapLayerPointerList >::const_iterator it = layerDependencies.constBegin();
          it != layerDependencies.constEnd(); ++it )
    {
      if ( !( _qgis_listQPointerToRaw( it.value() ).contains( layer ) ) )
      {
        //task not dependent on this layer
        continue;
      }

      QgsTask *dependentTask = task( it.key() );
      if ( dependentTask && ( dependentTask->status() != QgsTask::Complete && dependentTask->status() != QgsTask::Terminated ) )
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

  QgsTaskRunnableWrapper *runnable = mTasks.value( id ).runnable;

  task->disconnect( this );

  mTaskMutex->lock();
  if ( mTaskDependencies.contains( id ) )
    mTaskDependencies.remove( id );
  mTaskMutex->unlock();

  emit taskAboutToBeDeleted( id );

  mTaskMutex->lock();
  bool isParent = mParentTasks.contains( task );
  mParentTasks.remove( task );
  mSubTasks.remove( task );
  mTasks.remove( id );
  mMapTaskPtrToId.remove( task );
  mLayerDependencies.remove( id );

  if ( task->status() != QgsTask::Complete && task->status() != QgsTask::Terminated )
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
    if ( runnable && mThreadPool->tryTake( runnable ) )
    {
      delete runnable;
      mTasks[ id ].runnable = nullptr;
    }

    if ( isParent )
    {
      //task already finished, kill it
      task->deleteLater();
    }
  }

  // at this stage (hopefully) dependent tasks have been canceled or queued
  for ( QMap< long, QgsTaskList >::iterator it = mTaskDependencies.begin(); it != mTaskDependencies.end(); ++it )
  {
    if ( it.value().contains( task ) )
    {
      it.value().removeAll( task );
    }
  }
  mTaskMutex->unlock();

  return true;
}

void QgsTaskManager::processQueue()
{
  int prevActiveCount = countActiveTasks( false );
  mTaskMutex->lock();
  mActiveTasks.clear();
  for ( QMap< long, TaskInfo >::iterator it = mTasks.begin(); it != mTasks.end(); ++it )
  {
    QgsTask *task = it.value().task;
    if ( task && task->mStatus == QgsTask::Queued && dependenciesSatisfied( it.key() ) && it.value().added.testAndSetRelaxed( 0, 1 ) )
    {
      it.value().createRunnable();
      mThreadPool->start( it.value().runnable, it.value().priority );
    }

    if ( task && ( task->mStatus != QgsTask::Complete && task->mStatus != QgsTask::Terminated ) )
    {
      mActiveTasks << task;
    }
  }

  bool allFinished = mActiveTasks.isEmpty();
  mTaskMutex->unlock();

  if ( allFinished )
  {
    emit allTasksFinished();
  }

  int newActiveCount = countActiveTasks( false );
  if ( prevActiveCount != newActiveCount )
  {
    emit countActiveTasksChanged( newActiveCount );
  }
}

void QgsTaskManager::cancelDependentTasks( long taskId )
{
  QgsTask *canceledTask = task( taskId );

  //deep copy
  mTaskMutex->lock();
  QMap< long, QgsTaskList > taskDependencies = mTaskDependencies;
  taskDependencies.detach();
  mTaskMutex->unlock();

  QMap< long, QgsTaskList >::const_iterator it = taskDependencies.constBegin();
  for ( ; it != taskDependencies.constEnd(); ++it )
  {
    if ( it.value().contains( canceledTask ) )
    {
      // found task with this dependency

      // cancel it - note that this will be recursive, so any tasks dependent
      // on this one will also be canceled
      QgsTask *dependentTask = task( it.key() );
      if ( dependentTask )
        dependentTask->cancel();
    }
  }
}

QgsTaskManager::TaskInfo::TaskInfo( QgsTask *task, int priority )
  : task( task )
  , added( 0 )
  , priority( priority )
{}

void QgsTaskManager::TaskInfo::createRunnable()
{
  Q_ASSERT( !runnable );
  runnable = new QgsTaskRunnableWrapper( task ); // auto deleted
}


QgsTaskWithSerialSubTasks::~QgsTaskWithSerialSubTasks()
{
  for ( QgsTask *subTask : mSubTasksSerial )
  {
    delete subTask;
  }
}

void QgsTaskWithSerialSubTasks::addSubTask( QgsTask *subTask )
{
  mSubTasksSerial << subTask;
}

bool QgsTaskWithSerialSubTasks::run()
{
  size_t i = 0;
  for ( QgsTask *subTask : mSubTasksSerial )
  {
    if ( mShouldTerminate )
      return false;
    connect( subTask, &QgsTask::progressChanged, this,
             [this, i]( double subTaskProgress )
    {
      mProgress = 100.0 * ( double( i ) + subTaskProgress / 100.0 ) / double( mSubTasksSerial.size() );
      setProgress( mProgress );
    } );
    if ( !subTask->run() )
      return false;
    subTask->completed();
    mProgress = 100.0 * double( i + 1 ) / double( mSubTasksSerial.size() );
    setProgress( mProgress );
    ++i;
  }
  return true;
}

void QgsTaskWithSerialSubTasks::cancel()
{
  if ( mOverallStatus == Complete || mOverallStatus == Terminated )
    return;

  QgsTask::cancel();

  for ( QgsTask *subTask : mSubTasksSerial )
  {
    subTask->cancel();
  }
}
