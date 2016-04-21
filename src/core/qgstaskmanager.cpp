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
  run();
}

void QgsTask::cancel()
{
  mShouldTerminate = true;
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

void QgsTask::stopped()
{
  mStatus = Terminated;
  emit statusChanged( Terminated );
  emit taskStopped();
}


//
// QgsTaskManager
//

// Static calls to enforce singleton behaviour
QgsTaskManager *QgsTaskManager::sInstance = nullptr;
QgsTaskManager *QgsTaskManager::instance()
{
  if ( !sInstance )
  {
    sInstance = new QgsTaskManager();
  }

  return sInstance;
}

QgsTaskManager::QgsTaskManager( QObject* parent )
    : QObject( parent )
    , mNextTaskId( 0 )
{}

QgsTaskManager::~QgsTaskManager()
{
  //first tell all tasks to cancel
  cancelAll();

  //then clean them up, including waiting for them to terminate
  QMap< long, TaskInfo >::const_iterator it = mTasks.constBegin();
  for ( ; it != mTasks.constEnd(); ++it )
  {
    cleanupAndDeleteTask( it.value().task );
  }
}

long QgsTaskManager::addTask( QgsTask* task )
{
  mTasks.insert( mNextTaskId, task );

  connect( task, SIGNAL( progressChanged( double ) ), this, SLOT( taskProgressChanged( double ) ) );
  connect( task, SIGNAL( statusChanged( int ) ), this, SLOT( taskStatusChanged( int ) ) );

  emit taskAdded( mNextTaskId );
  processQueue();

  return mNextTaskId++;
}

bool QgsTaskManager::deleteTask( long id )
{
  QgsTask* task = mTasks.value( id ).task;
  return deleteTask( task );
}

bool QgsTaskManager::deleteTask( QgsTask *task )
{
  if ( !task )
    return false;

  bool result = cleanupAndDeleteTask( task );

  // remove from internal task list
  for ( QMap< long, TaskInfo >::iterator it = mTasks.begin(); it != mTasks.end(); )
  {
    if ( it.value().task == task )
      it = mTasks.erase( it );
    else
      ++it;
  }

  return result;
}

QgsTask*QgsTaskManager::task( long id ) const
{
  return mTasks.value( id ).task;
}

QList<QgsTask*> QgsTaskManager::tasks() const
{
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

  QMap< long, TaskInfo >::const_iterator it = mTasks.constBegin();
  for ( ; it != mTasks.constEnd(); ++it )
  {
    if ( it.value().task == task )
      return it.key();
  }
  return -1;
}

void QgsTaskManager::cancelAll()
{
  QMap< long, TaskInfo >::iterator it = mTasks.begin();
  for ( ; it != mTasks.end(); ++it )
  {
    QgsTask* task = it.value().task;
    if ( task->isActive() )
    {
      task->cancel();
    }
  }
}

void QgsTaskManager::taskProgressChanged( double progress )
{
  QgsTask* task = qobject_cast< QgsTask* >( sender() );

  //find ID of task
  long id = taskId( task );
  if ( id < 0 )
    return;

  emit progressChanged( id, progress );
}

void QgsTaskManager::taskStatusChanged( int status )
{
  QgsTask* task = qobject_cast< QgsTask* >( sender() );

  //find ID of task
  long id = taskId( task );
  if ( id < 0 )
    return;

  emit statusChanged( id, status );
  processQueue();
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
    connect( task, SIGNAL( taskCompleted() ), task, SLOT( deleteLater() ) );
    connect( task, SIGNAL( taskStopped() ), task, SLOT( deleteLater() ) );
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
  for ( QMap< long, TaskInfo >::iterator it = mTasks.begin(); it != mTasks.end(); ++it )
  {
    QgsTask* task = it.value().task;
    if ( task && task->status() == QgsTask::Queued )
    {
      mTasks[ it.key()].future = QtConcurrent::run( task, &QgsTask::start );
    }
  }
}
