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
#include <QMutex>


//
// QgsTask
//

QgsTask::QgsTask( const QString &name )
    : QObject()
    , mDescription( name )
    , mStatus( Running )
    , mProgress( 0.0 )
{}

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
  QMap< long, QgsTask* >::const_iterator it = mTasks.constBegin();
  for ( ; it != mTasks.constEnd(); ++it )
  {
    cleanupAndDeleteTask( it.value() );
  }
}

long QgsTaskManager::addTask( QgsTask* task )
{
  static QMutex sAddMutex( QMutex::Recursive );
  QMutexLocker locker( &sAddMutex );

  mTasks.insert( mNextTaskId, task );
  connect( task, SIGNAL( progressChanged( double ) ), this, SLOT( taskProgressChanged( double ) ) );
  connect( task, SIGNAL( statusChanged( int ) ), this, SLOT( taskStatusChanged( int ) ) );
  emit taskAdded( mNextTaskId );
  return mNextTaskId++;
}

bool QgsTaskManager::deleteTask( long id )
{
  QgsTask* task = mTasks.value( id );
  return deleteTask( task );
}

bool QgsTaskManager::deleteTask( QgsTask *task )
{
  if ( !task )
    return false;

  bool result = cleanupAndDeleteTask( task );

  // remove from internal task list
  for ( QMap< long, QgsTask* >::iterator it = mTasks.begin(); it != mTasks.end(); )
  {
    if ( it.value() == task )
      it = mTasks.erase( it );
    else
      ++it;
  }

  return result;
}

QgsTask*QgsTaskManager::task( long id ) const
{
  return mTasks.value( id );
}

QList<QgsTask*> QgsTaskManager::tasks() const
{
  return mTasks.values();
}

long QgsTaskManager::taskId( QgsTask *task ) const
{
  if ( !task )
    return -1;

  QMap< long, QgsTask* >::const_iterator it = mTasks.constBegin();
  for ( ; it != mTasks.constEnd(); ++it )
  {
    if ( it.value() == task )
      return it.key();
  }
  return -1;
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
}

bool QgsTaskManager::cleanupAndDeleteTask( QgsTask *task )
{
  if ( !task )
    return false;

  if ( task->isActive() )
    task->terminate();

  emit taskAboutToBeDeleted( taskId( task ) );

  task->deleteLater();
  return true;
}
