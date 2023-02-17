/***************************************************************************
                             qgsproxyprogresstask.cpp
                             ------------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsproxyprogresstask.h"
#include "qgsapplication.h"

QgsProxyProgressTask::QgsProxyProgressTask( const QString &description, bool canCancel )
  : QgsTask( description, canCancel ? QgsTask::CanCancel : QgsTask::Flags() )
{
}

void QgsProxyProgressTask::finalize( bool result )
{
  const QMutexLocker lock( &mNotFinishedMutex );
  mAlreadyFinished = true;

  mResult = result;
  mNotFinishedWaitCondition.wakeAll();
}

bool QgsProxyProgressTask::run()
{
  mNotFinishedMutex.lock();
  if ( !mAlreadyFinished )
  {
    mNotFinishedWaitCondition.wait( &mNotFinishedMutex );
  }
  mNotFinishedMutex.unlock();

  return mResult;
}

void QgsProxyProgressTask::setProxyProgress( double progress )
{
  QMetaObject::invokeMethod( this, "setProgress", Qt::AutoConnection, Q_ARG( double, progress ) );
}

void QgsProxyProgressTask::cancel()
{
  emit canceled();

  QgsTask::cancel();
}

//
// QgsScopedProxyProgressTask
//

QgsScopedProxyProgressTask::QgsScopedProxyProgressTask( const QString &description )
  : mTask( new QgsProxyProgressTask( description ) )
{
  QgsApplication::taskManager()->addTask( mTask );
}

QgsScopedProxyProgressTask::~QgsScopedProxyProgressTask()
{
  mTask->finalize( true );
}

void QgsScopedProxyProgressTask::setProgress( double progress )
{
  mTask->setProxyProgress( progress );
}
