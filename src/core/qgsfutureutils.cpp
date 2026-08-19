/***************************************************************************
               qgsthreadingutils.h
                     --------------------------------------
               Date                 : August 2026
               Copyright            : (C) 2026 by David Koňařík
               email                : dvdkon at konarici dot cz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfutureutils.h"

#include "moc_qgsfutureutils.cpp"

/// @cond private

QgsCombinedFutureHelper::QgsCombinedFutureHelper( QList<QFuture<void>> futures, std::function<void()> allFinishedCallback )
  : mAllFinishedCallback( std::move( allFinishedCallback ) )
{
  mWatchers.resize( futures.size() );
  for ( qsizetype i = 0; i < futures.size(); i++ )
  {
    mWatchers[i] = new QFutureWatcher<void>( this );
    connect( mWatchers[i], &QFutureWatcherBase::finished, this, &QgsCombinedFutureHelper::onOneFinished );
    mWatchers[i]->setFuture( futures[i] );
  }
}

void QgsCombinedFutureHelper::onOneFinished()
{
  if ( mFired )
    return;
  // Check all futures are finished
  for ( auto &watcher : std::as_const( mWatchers ) )
  {
    if ( !watcher->isFinished() )
      return;
  }
  mFired = true;
  mAllFinishedCallback();
  deleteLater();
}

/// @endcond

void QgsFutureUtils::waitForFinished( QFuture<void> future )
{
  if ( QThread::currentThread() == QCoreApplication::instance()->thread() )
  {
    // We can't block on the main thread (the future finishing may depend on
    // some signal being delivered), so instead we manually spin the event loop
    // until the future is finished.
    // Running main loop events here is not without its dangers. If those
    // events mutate something our calling code is working on (like with
    // deleteLater()), we could get a nasty crash. Sadly QEventLoop doesn't
    // allow us to filter on event types.
    QEventLoop loop;
    QFutureWatcher<void> watcher;
    QObject::connect( &watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit );
#ifdef QGISDEBUG
    // Helps expose the issues talked about above
    QCoreApplication::processEvents();
#endif
    watcher.setFuture( future );
    if ( !watcher.isFinished() )
      loop.exec();
  }
  else
  {
    // On worker threads we can safely block
    future.waitForFinished();
  }
}

QFuture<void> QgsFutureUtils::clone( QFuture<void> base )
{
  auto watcher = new QFutureWatcher<void>;

  QFuture<void> cloned = QtFuture::connect( watcher, &QFutureWatcherBase::finished ) //
                           .then( [watcher]() { watcher->deleteLater(); } );
  watcher->setFuture( base );

  return cloned;
}
