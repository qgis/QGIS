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

#include "qgsnetworkaccessmanager.h"

#include <QNetworkReply>

QList<QgsTileDownloadManager::QueueEntry> QgsTileDownloadManager::queue;
bool QgsTileDownloadManager::shuttingDown = false;
QMutex QgsTileDownloadManager::mutex;
QThread *QgsTileDownloadManager::workerThread = nullptr;
QObject *QgsTileDownloadManager::worker = nullptr;
QgsTileDownloadManager::Stats QgsTileDownloadManager::stats;


void QgsTileDownloadManagerWorker::queueUpdated()
{
  qDebug() << "queueUpdated";

  // TODO: - if timer to kill thread is running: stop the timer

  QMutexLocker locker( &QgsTileDownloadManager::mutex );

  // TODO:  - if shutting down: abort all network requests, quit() thread
  if ( QgsTileDownloadManager::shuttingDown )
  {
    qDebug() << "shutting down!";
    for ( auto it = QgsTileDownloadManager::queue.begin(); it != QgsTileDownloadManager::queue.end(); ++it )
    {

    }
    QgsTileDownloadManager::shuttingDown = false;
    return;
  }

  for ( auto it = QgsTileDownloadManager::queue.begin(); it != QgsTileDownloadManager::queue.end(); ++it )
  {
    if ( !it->networkReply )
    {
      qDebug() << "starting request " << it->request.url();
      // start entries which are not in progress

      //QNetworkRequest request( it->url );
      it->networkReply = QgsNetworkAccessManager::instance()->get( it->request );
      connect( it->networkReply, &QNetworkReply::finished, it->objWorker, &QgsTileDownloadManagerReplyWorkerObject::replyFinished );

      ++QgsTileDownloadManager::stats.networkRequestsStarted;
    }
  }


}


///


void QgsTileDownloadManagerReplyWorkerObject::replyFinished()
{
  QMutexLocker locker( &QgsTileDownloadManager::mutex );

  qDebug() << "reply finished " << mRequest.url();

  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  QByteArray data;

  bool ok = reply->error() == QNetworkReply::NoError;
  if ( ok )
  {
    ++QgsTileDownloadManager::stats.networkRequestsOk;

    data = reply->readAll();
  }
  else
  {
    ++QgsTileDownloadManager::stats.networkRequestsFailed;

    // TODO: more handling of network errors?
  }

  emit finished( data );

  reply->deleteLater();

  // kill the worker obj
  deleteLater();

  QgsTileDownloadManager::removeEntry( mRequest );

  // TODO: - if this was the last thing in the queue, start a timer to kill thread after X seconds
}


///


QgsTileDownloadManager::QgsTileDownloadManager()
{

}

QgsTileDownloadManagerReply *QgsTileDownloadManager::get( const QNetworkRequest &request )
{
  QMutexLocker locker( &mutex );

  if ( !worker )
  {
    workerThread = new QThread;
    worker = new QgsTileDownloadManagerWorker;
    worker->moveToThread( workerThread );
    QObject::connect( workerThread, &QThread::finished, worker, &QObject::deleteLater );
    workerThread->start();
  }

  QgsTileDownloadManagerReply *reply = new QgsTileDownloadManagerReply( request ); // lives in the same thread as the caller

  ++stats.requestsTotal;

  QgsTileDownloadManager::QueueEntry entry = findEntryForRequest( request );
  if ( !entry.isValid() )
  {
    qDebug() << "new entry for " << request.url();
    // create a new entry and add it to queue
    entry.request = request;
    //entry.state = 0;
    entry.objWorker = new QgsTileDownloadManagerReplyWorkerObject( request );
    entry.objWorker->moveToThread( workerThread );

    entry.listeners.append( reply );
    QObject::connect( entry.objWorker, &QgsTileDownloadManagerReplyWorkerObject::finished, reply, &QgsTileDownloadManagerReply::requestFinished );  // should be queued connection

    addEntry( entry );
  }
  else
  {
    qDebug() << " adding listener for " << request.url();

    entry.listeners.append( reply );
    QObject::connect( entry.objWorker, &QgsTileDownloadManagerReplyWorkerObject::finished, reply, &QgsTileDownloadManagerReply::requestFinished );  // should be queued connection

    updateEntry( entry );
    ++stats.requestsMerged;
  }

  signalQueueModified();

  return reply;
}

bool QgsTileDownloadManager::hasPendingRequests()
{
  QMutexLocker locker( &mutex );

  return !queue.isEmpty();
}

bool QgsTileDownloadManager::waitForPendingRequests( int msec )
{
  QTime t;
  t.start();

  while ( msec == -1 || t.elapsed() < msec )
  {
    {
      QMutexLocker locker( &mutex );
      if ( queue.isEmpty() )
        return true;
    }
    QThread::currentThread()->usleep( 1000 );
  }

  return false;
}

void QgsTileDownloadManager::shutdown()
{
  {
    QMutexLocker locker( &mutex );
    shuttingDown = true;
    signalQueueModified();
  }
}

void QgsTileDownloadManager::resetStatistics()
{
  QMutexLocker locker( &mutex );
  stats = QgsTileDownloadManager::Stats();
}

QgsTileDownloadManager::QueueEntry QgsTileDownloadManager::findEntryForRequest( const QNetworkRequest &request )
{
  for ( auto it = queue.constBegin(); it != queue.constEnd(); ++it )
  {
    if ( it->request == request )
      return *it;
  }
  return QgsTileDownloadManager::QueueEntry();
}

void QgsTileDownloadManager::addEntry( const QgsTileDownloadManager::QueueEntry &entry )
{
  for ( auto it = queue.constBegin(); it != queue.constEnd(); ++it )
  {
    Q_ASSERT( entry.request != it->request );
  }

  queue.append( entry );
}

void QgsTileDownloadManager::updateEntry( const QgsTileDownloadManager::QueueEntry &entry )
{
  for ( auto it = queue.begin(); it != queue.end(); ++it )
  {
    if ( entry.request == it->request )
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
  for ( auto it = queue.constBegin(); it != queue.constEnd(); ++it, ++i )
  {
    if ( it->request == request )
    {
      queue.removeAt( i );
      return;
    }
  }
  Q_ASSERT( false );
}

void QgsTileDownloadManager::signalQueueModified()
{
  QMetaObject::invokeMethod( worker, "queueUpdated", Qt::QueuedConnection );
  //QMetaObject::invokeMethod( worker, &QgsTileDownloadManagerWorker::queueUpdated, Qt::QueuedConnection ); //&QgsTileDownloadManagerWorker::queueUpdated );
}


///


QgsTileDownloadManagerReply::~QgsTileDownloadManagerReply()
{
  QMutexLocker locker( &QgsTileDownloadManager::mutex );

  QgsTileDownloadManager::QueueEntry entry = QgsTileDownloadManager::findEntryForRequest( mRequest );
  if ( entry.isValid() )
  {
    // remove it from the list of listeners
    Q_ASSERT( entry.listeners.contains( this ) );
    entry.listeners.removeOne( this );

    ++QgsTileDownloadManager::stats.requestsEarlyDeleted;

    // TODO: we could also remove the whole entry if it has not started yet
  }

}

void QgsTileDownloadManagerReply::requestFinished( QByteArray data )
{
  qDebug() << QThread::currentThread() << mRequest.url();

  mData = data;
  emit finished();
}
