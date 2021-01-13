/***************************************************************************
                         qgstiledownloadmanager.h
                         ------------------------
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

#ifndef QGSTILEDOWNLOADMANAGER_H
#define QGSTILEDOWNLOADMANAGER_H

#include <QThread>
#include <QMutex>

#include <QNetworkAccessManager>

#include "qgis_core.h"

/**
 * \ingroup core
 * Reply object for tile download manager requests returned from calls to QgsTileDownloadManager::get().
 *
 * When the underlying network request has finished (with success or failure), the finished() signal
 * gets emitted.
 *
 * It is OK to delete this object before the request has finished - the request will not be aborted,
 * the download manager will finish the download (as it may be needed soon afterwards).
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsTileDownloadManagerReply : public QObject
{
    Q_OBJECT
  public:
    QgsTileDownloadManagerReply( const QNetworkRequest &request ): mRequest( request ) {}
    ~QgsTileDownloadManagerReply();

    QByteArray data() const { return mData; }

    QNetworkRequest request() const { return mRequest; }

  public slots:
    void requestFinished( QByteArray data );

  signals:
    void finished();

  private:
    QNetworkRequest mRequest;
    QByteArray mData;
};


/// @cond PRIVATE

/**
 * \ingroup core
 * Reply object that is used in the worker thread of the tile download manager.
 * \note This class is not a part of public API
 */
class QgsTileDownloadManagerReplyWorkerObject : public QObject
{
    Q_OBJECT
  public:
    QgsTileDownloadManagerReplyWorkerObject( const QNetworkRequest &request ): mRequest( request ) {}

  public slots:
    void replyFinished();

  signals:
    void finished( QByteArray data );

  private:
    QNetworkRequest mRequest;
};


/**
 * \ingroup core
 * Worker object that is used in the worker thread of the tile download manager.
 * \note This class is not a part of public API
 */
class QgsTileDownloadManagerWorker : public QObject
{
    Q_OBJECT

  public:
    QgsTileDownloadManagerWorker( QObject *parent = nullptr ): QObject( parent ) {}
    //~QgsTileDownloadManagerWorker();

  public slots:
    void queueUpdated();

  signals:
    void requestFinished( QString url, QByteArray data );

  private:
};

/// @endcond



/**
 * \ingroup core
 *
 * Tile download manager handles downloads of map tiles for the purpose of map rendering.
 * The purpose of this class is to handle a couple of situations that may happen:
 *
 * - a map rendering job starts which requests tiles from a remote server, then in a short
 *   while user zooms/pans map, which would normally mean that all pending requests get
 *   aborted and then restarted soon afterwards. The download manager lets the requests
 *   to finish to avoid excessive load on servers and needless aborts and repeated requests.
 * - multiple map rendering jobs start at a similar time, requesting map the same map tiles.
 *   Normally they could be requested multiple times from the server - the download manager
 *   groups these requests and only does a single request at a time.
 *
 * At this point, it is not recommended to use this class for other scenarios than map
 * rendering: using it elsewhere could slow down map rendering or have some unexpected
 * negative effects.
 *
 * How do things work:
 *
 * - Upon a request, a QgsTileDownloadManagerReply object (based on QObject) is returned,
 *   encapsulating the pending reply. Client can wait for its finished() signal - when
 *   it gets emitted, the request has finished with success or failure. Client can delete
 *   the reply object before the request is processed - download manager will finish its
 *   download (and it will get cached for a future use).
 * - All requests are done by QgsNetworkAccessManager
 * - A worker thread responsible for all network tile requests is started first time a tile
 *   is requested. Having a dedicated thread rather than reusing main thread makes things
 *   more predictable as we won't get stuck in case the main thread is doing some work or
 *   it is waiting for map rendering to finish.
 * - There is a shared download queue (protected by a mutex) with a list of active requests
 *   and requests waiting to be processed.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsTileDownloadManager
{
  public:


    class QueueEntry
    {
      public:
        bool isValid() const { return !request.url().isEmpty(); }

        //! The actual original Qt network request
        QNetworkRequest request;
        //! Helper QObject that lives in worker thread that emits signals
        QgsTileDownloadManagerReplyWorkerObject *objWorker = nullptr;
        //! Sources waiting for this data
        QList<QgsTileDownloadManagerReply *> listeners;
        //! Internal network reply - only to be touched by the worker thread
        QNetworkReply *networkReply = nullptr;
    };


    class Stats
    {
      public:
        //! How many requests were done through the download manager
        int requestsTotal = 0;
        //! How many requests were same as some other pending request and got "merged"
        int requestsMerged = 0;
        //! How many requests were deleted early by the client (i.e. lost interest)
        int requestsEarlyDeleted = 0;

        //! How many actual network requests were started
        int networkRequestsStarted = 0;
        //! How many network requests have been successful
        int networkRequestsOk = 0;
        //! How many network requests have failed
        int networkRequestsFailed = 0;
    };

    QgsTileDownloadManager();

    static QgsTileDownloadManagerReply *get( const QNetworkRequest &request );

    static bool hasPendingRequests();

    static bool waitForPendingRequests( int msec = -1 );

    static void shutdown();

    static Stats statistics() { return stats; }

    static void resetStatistics();

    friend class QgsTileDownloadManagerWorker;
    friend class QgsTileDownloadManagerReply;
    friend class QgsTileDownloadManagerReplyWorkerObject;

  private:

    // these can be only used with mutex locked!
    static QueueEntry findEntryForRequest( const QNetworkRequest &request );
    static void addEntry( const QueueEntry &entry );
    static void updateEntry( const QueueEntry &entry );
    static void removeEntry( const QNetworkRequest &request );

    static void signalQueueModified();

  private:

    static QList<QueueEntry> queue;
    static bool shuttingDown;
    static QMutex mutex;
    static QThread *workerThread;
    static QObject *worker;
    static Stats stats;
};

#endif // QGSTILEDOWNLOADMANAGER_H
