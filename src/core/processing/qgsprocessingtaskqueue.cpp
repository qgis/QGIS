/***************************************************************************
                         qgsprocessingtaskqueue.cpp
                         --------------------------
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

#include "qgsprocessingalgorithm.h"

QgsProcessingTaskQueue *QgsProcessingTaskQueue::sInstance = nullptr;

QgsProcessingQueuedTask::QgsProcessingQueuedTask( const QString &algorithmId,
    const QVariantMap &parameters,
    const QString &description )
  : mAlgorithmId( algorithmId )
  , mParameters( parameters )
  , mDescription( description )
{
}

QgsProcessingTaskQueue::QgsProcessingTaskQueue()
  : QObject( nullptr )
{
}

QgsProcessingTaskQueue::~QgsProcessingTaskQueue()
{
}

QgsProcessingTaskQueue *QgsProcessingTaskQueue::instance()
{
  if ( !sInstance )
  {
    sInstance = new QgsProcessingTaskQueue();
  }
  return sInstance;
}

void QgsProcessingTaskQueue::addTask( const QString &algorithmId, const QVariantMap &parameters, const QString &description )
{
  mQueue.append( QgsProcessingQueuedTask( algorithmId, parameters, description ) );
  emit queueChanged();
}

bool QgsProcessingTaskQueue::removeTask( int index )
{
  if ( index >= 0 && index < mQueue.count() )
  {
    mQueue.removeAt( index );
    emit queueChanged();
    return true;
  }
  return false;
}

bool QgsProcessingTaskQueue::moveTaskUp( int index )
{
  if ( index > 0 && index < mQueue.count() )
  {
    mQueue.swapItemsAt( index, index - 1 );
    emit queueChanged();
    return true;
  }
  return false;
}

bool QgsProcessingTaskQueue::moveTaskDown( int index )
{
  if ( index >= 0 && index < mQueue.count() - 1 )
  {
    mQueue.swapItemsAt( index, index + 1 );
    emit queueChanged();
    return true;
  }
  return false;
}

void QgsProcessingTaskQueue::clear()
{
  mQueue.clear();
  emit queueChanged();
}

#include "moc_qgsprocessingtaskqueue.cpp"
