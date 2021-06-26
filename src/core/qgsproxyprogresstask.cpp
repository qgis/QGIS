/***************************************************************************
                             qgsproxyprogresstask.cpp
                             ------------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsproxyprogresstask.h"
#include "qgsapplication.h"

QgsProxyProgressTask::QgsProxyProgressTask( const QString &description )
  : QgsTask( description, QgsTask::Flags() )
{
}

void QgsProxyProgressTask::finalize( bool result )
{
  QMutexLocker lock( &mNotFinishedMutex );
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
