/***************************************************************************
                         qgstiledownloadmanager.cpp
                         --------------------------
    begin                : January 2021
    copyright            : (C) 2021 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledownloadmanager.h"

#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"

#include <QElapsedTimer>
#include <QNetworkReply>


/// @cond PRIVATE

QgsTileDownloadManagerWorker::QgsTileDownloadManagerWorker( QgsTileDownloadManager *manager, QObject *parent )
  : QObject( parent )
  , mManager( manager )
  , mIdleTimer( this )
{
  connect( &mIdleTimer, &QTimer::timeout, this, &QgsTileDownloadManagerWorker::idleTimerTimeout );
}

void QgsTileDownloadManagerWorker::startIdleTimer()
{
  if ( !mIdleTimer.isActive() )
  {
    mIdleTimer.start( mManager->mIdleThreadTimeoutMs );
  }
}

void QgsTileDownloadManagerWorker::queueUpdated()
{
  const QMutexLocker locker( &mManager->mMutex );

  if ( mManager->mShuttingDown )
  {
    for ( auto it = mManager->mQueue.begin(); it != mManager->mQueue.end(); ++it )
    {
      it->networkReply->abort();
    }

    quitThread();
    return;
  }

  if ( mIdleTimer.isActive() && !mManager->mQueue.isEmpty() )
  {
    // if timer to kill thread is running: stop the timer, we have work to do
    mIdleTimer.stop();
  }

  for ( auto it = mManager->mQueue.begin(); it != mManager->mQueue.end(); ++it )
  {
    if ( !it->networkReply )
    {
      QgsDebugMsgLevel( QStringLiteral( "Tile download manager: starting request: " ) + it->request.url().toString(), 2 );
      // start entries which are not in progress

      it->networkReply = QgsNetworkAccessManager::instance()->get( it->request );
      connect( it->networkReply, &QNetworkReply::finished, it->objWorker, &QgsTileDownloadManagerReplyWorkerObject::replyFinished );

      ++mManager->mStats.networkRequestsStarted;
    }
  }
}

void QgsTileDownloadManagerWorker::quitThread()
{
  QgsDebugMsgLevel( QStringLiteral( "Tile download manager: stopping worker thread" ), 2 );

  mManager->mWorker->deleteLater();
  mManager->mWorker = nullptr;
  // we signal to our worker thread it's time to go. Its finished() signal is connected
  // to deleteLater() call, so it will get deleted automatically
  mManager->mWorkerThread->quit();
  mManager->mWorkerThread = nullptr;
  mManager->mShuttingDown = false;
}

void QgsTileDownloadManagerWorker::idleTimerTimeout()
{
  const QMutexLocker locker( &mManager->mMutex );
  Q_ASSERT( mManager->mQueue.isEmpty() );
  quitThread();
}


///


void QgsTileDownloadManagerReplyWorkerObject::replyFinished()
{
  const QMutexLocker locker( &mManager->mMutex );

  QgsDebugMsgLevel( QStringLiteral( "Tile download manager: internal reply finished: " ) + mRequest.url().toString(), 2 );

  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  QByteArray data;

  if ( reply->error() == QNetworkReply::NoError )
  {
    ++mManager->mStats.networkRequestsOk;
    data = reply->readAll();
  }
  else
  {
    ++mManager->mStats.networkRequestsFailed;
    const QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
    if ( contentType.startsWith( QLatin1String( "text/plain" ) ) )
      data = reply->readAll();
  }

  emit finished( data, reply->error(), reply->errorString() );

  reply->deleteLater();

  // kill the worker obj
  deleteLater();

  mManager->removeEntry( mRequest );

  if ( mManager->mQueue.isEmpty() )
  {
    // if this was the last thing in the queue, start a timer to kill thread after X seconds
    mManager->mWorker->startIdleTimer();
  }
}

/// @endcond

///


QgsTileDownloadManager::QgsTileDownloadManager()
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  : mMutex( QMutex::Recursive )
#endif
{
}

QgsTileDownloadManager::~QgsTileDownloadManager()
{
  // make sure the worker thread is gone and any pending requests are canceled
  shutdown();
}

QgsTileDownloadManagerReply *QgsTileDownloadManager::get( const QNetworkRequest &request )
{
  const QMutexLocker locker( &mMutex );

  if ( !mWorker )
  {
    QgsDebugMsgLevel( QStringLiteral( "Tile download manager: starting worker thread" ), 2 );
    mWorkerThread = new QThread;
    mWorker = new QgsTileDownloadManagerWorker( this );
    mWorker->moveToThread( mWorkerThread );
    QObject::connect( mWorkerThread, &QThread::finished, mWorker, &QObject::deleteLater );
    mWorkerThread->start();
  }

  QgsTileDownloadManagerReply *reply = new QgsTileDownloadManagerReply( this, request ); // lives in the same thread as the caller

  ++mStats.requestsTotal;

  QgsTileDownloadManager::QueueEntry entry = findEntryForRequest( request );
  if ( !entry.isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Tile download manager: get (new entry): " ) + request.url().toString(), 2 );
    // create a new entry and add it to queue
    entry.request = request;
    entry.objWorker = new QgsTileDownloadManagerReplyWorkerObject( this, request );
    entry.objWorker->moveToThread( mWorkerThread );

    QObject::connect( entry.objWorker, &QgsTileDownloadManagerReplyWorkerObject::finished, reply, &QgsTileDownloadManagerReply::requestFinished );  // should be queued connection

    addEntry( entry );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Tile download manager: get (existing entry): " ) + request.url().toString(), 2 );

    QObject::connect( entry.objWorker, &QgsTileDownloadManagerReplyWorkerObject::finished, reply, &QgsTileDownloadManagerReply::requestFinished );  // should be queued connection

    ++mStats.requestsMerged;
  }

  signalQueueModified();

  return reply;
}

bool QgsTileDownloadManager::hasPendingRequests() const
{
  const QMutexLocker locker( &mMutex );

  return !mQueue.isEmpty();
}

bool QgsTileDownloadManager::waitForPendingRequests( int msec )
{
  QElapsedTimer t;
  t.start();

  while ( msec == -1 || t.elapsed() < msec )
  {
    {
      const QMutexLocker locker( &mMutex );
      if ( mQueue.isEmpty() )
        return true;
    }
    QThread::usleep( 1000 );
  }

  return false;
}

void QgsTileDownloadManager::shutdown()
{
  {
    const QMutexLocker locker( &mMutex );
    if ( !mWorkerThread )
      return;  // nothing to stop

    // let's signal to the thread
    mShuttingDown = true;
    signalQueueModified();
  }

  // wait until the thread is gone
  while ( 1 )
  {
    {
      const QMutexLocker locker( &mMutex );
      if ( !mWorkerThread )
        return;  // the thread has stopped
    }

    QThread::usleep( 1000 );
  }
}

bool QgsTileDownloadManager::hasWorkerThreadRunning() const
{
  return mWorkerThread && mWorkerThread->isRunning();
}

void QgsTileDownloadManager::resetStatistics()
{
  const QMutexLocker locker( &mMutex );
  mStats = QgsTileDownloadManager::Stats();
}

QgsTileDownloadManager::QueueEntry QgsTileDownloadManager::findEntryForRequest( const QNetworkRequest &request )
{
  for ( auto it = mQueue.constBegin(); it != mQueue.constEnd(); ++it )
  {
    if ( it->request.url() == request.url() )
      return *it;
  }
  return QgsTileDownloadManager::QueueEntry();
}

void QgsTileDownloadManager::addEntry( const QgsTileDownloadManager::QueueEntry &entry )
{
  for ( auto it = mQueue.constBegin(); it != mQueue.constEnd(); ++it )
  {
    Q_ASSERT( entry.request.url() != it->request.url() );
  }

  mQueue.append( entry );
}

void QgsTileDownloadManager::updateEntry( const QgsTileDownloadManager::QueueEntry &entry )
{
  for ( auto it = mQueue.begin(); it != mQueue.end(); ++it )
  {
    if ( entry.request.url() == it->request.url() )
    {
      *it = entry;
      return;
    }
  }
  Q_ASSERT( false );
}

void QgsTileDownloadManager::removeEntry( const QNetworkRequest &request )
{
  int i = 0;
  for ( auto it = mQueue.constBegin(); it != mQueue.constEnd(); ++it, ++i )
  {
    if ( it->request.url() == request.url() )
    {
      mQueue.removeAt( i );
      return;
    }
  }
  Q_ASSERT( false );
}

void QgsTileDownloadManager::signalQueueModified()
{
  QMetaObject::invokeMethod( mWorker, &QgsTileDownloadManagerWorker::queueUpdated, Qt::QueuedConnection );
}


///


QgsTileDownloadManagerReply::QgsTileDownloadManagerReply( QgsTileDownloadManager *manager, const QNetworkRequest &request )
  : mManager( manager )
  , mRequest( request )
{
}

QgsTileDownloadManagerReply::~QgsTileDownloadManagerReply()
{
  const QMutexLocker locker( &mManager->mMutex );

  if ( !mHasFinished )
  {
    QgsDebugMsgLevel( QStringLiteral( "Tile download manager: reply deleted before finished: " ) + mRequest.url().toString(), 2 );

    ++mManager->mStats.requestsEarlyDeleted;
  }
}

void QgsTileDownloadManagerReply::requestFinished( QByteArray data, QNetworkReply::NetworkError error, const QString &errorString )
{
  QgsDebugMsgLevel( QStringLiteral( "Tile download manager: reply finished: " ) + mRequest.url().toString(), 2 );

  mHasFinished = true;
  mData = data;
  mError = error;
  mErrorString = errorString;
  emit finished();
}
