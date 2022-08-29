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

#define SIP_NO_FILE

#include <QTimer>
#include <QThread>
#include <QMutex>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "qgis_core.h"

class QgsTileDownloadManager;
class QgsRangeRequestCache;

/**
 * \ingroup core
 * \brief Reply object for tile download manager requests returned from calls to QgsTileDownloadManager::get().
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
    ~QgsTileDownloadManagerReply();

    //! Returns whether the reply has already finished (with success/failure)
    bool hasFinished() const { return mHasFinished; }
    //! Returns binary data returned in the reply (only valid when already finished)
    QByteArray data() const { return mData; }
    //! Returns the reply URL
    QUrl url() const { return mUrl; }
    //! Returns the attribute associated with the \a code
    QVariant attribute( QNetworkRequest::Attribute code );
    //! Returns the value of the known header \a header.
    QVariant header( QNetworkRequest::KnownHeaders header );
    //! Returns a list of raw header pairs
    const QList<QNetworkReply::RawHeaderPair> rawHeaderPairs() const { return mRawHeaderPairs; }
    //! Returns error code (only valid when already finished)
    QNetworkReply::NetworkError error() const { return mError; }
    //! Returns error string (only valid when already finished)
    QString errorString() const { return mErrorString; }
    //! Returns the original request for this reply object
    QNetworkRequest request() const { return mRequest; }

  signals:
    //! Emitted when the reply has finished (either with a success or with a failure)
    void finished();

  private slots:
    void requestFinished( QByteArray data, QUrl url, const QMap<QNetworkRequest::Attribute, QVariant> &attributes, const QMap<QNetworkRequest::KnownHeaders, QVariant> &headers, const QList<QNetworkReply::RawHeaderPair> rawHeaderPairs, QNetworkReply::NetworkError error, const QString &errorString );
    void cachedRangeRequestFinished();

  private:
    QgsTileDownloadManagerReply( QgsTileDownloadManager *manager, const QNetworkRequest &request );

    friend class QgsTileDownloadManager;  // allows creation of new instances from the manager

  private:
    //! "parent" download manager of this reply
    QgsTileDownloadManager *mManager = nullptr;
    QNetworkRequest mRequest;
    bool mHasFinished = false;
    QByteArray mData;
    QNetworkReply::NetworkError mError = QNetworkReply::NoError;
    QString mErrorString;
    QUrl mUrl;
    QMap<QNetworkRequest::Attribute, QVariant> mAttributes;
    QMap<QNetworkRequest::KnownHeaders, QVariant> mHeaders;
    QList<QNetworkReply::RawHeaderPair> mRawHeaderPairs;
};


/// @cond PRIVATE

/**
 * \ingroup core
 * \brief Reply object that is used in the worker thread of the tile download manager.
 * \note This class is not a part of public API
 */
class QgsTileDownloadManagerReplyWorkerObject : public QObject
{
    Q_OBJECT
  public:
    QgsTileDownloadManagerReplyWorkerObject( QgsTileDownloadManager *manager, const QNetworkRequest &request )
      : mManager( manager ), mRequest( request ) {}

  public slots:
    void replyFinished();

  signals:
    void finished( QByteArray data, QUrl url, const QMap<QNetworkRequest::Attribute, QVariant> &attributes, const QMap<QNetworkRequest::KnownHeaders, QVariant> &headers, const QList<QNetworkReply::RawHeaderPair> rawHeaderPairs, QNetworkReply::NetworkError error, const QString &errorString );

  private:
    //! "parent" download manager of this worker object
    QgsTileDownloadManager *mManager = nullptr;
    QNetworkRequest mRequest;
};


/**
 * \ingroup core
 * \brief Worker object that is used in the worker thread of the tile download manager.
 * \note This class is not a part of public API
 */
class QgsTileDownloadManagerWorker : public QObject
{
    Q_OBJECT

  public:
    //! Creates the worker
    QgsTileDownloadManagerWorker( QgsTileDownloadManager *manager, QObject *parent = nullptr );

    void startIdleTimer();

  public slots:
    void queueUpdated();
    void idleTimerTimeout();

  signals:
    void requestFinished( QString url, QByteArray data );

  private:
    void quitThread();

  private:
    //! "parent" download manager of this worker
    QgsTileDownloadManager *mManager = nullptr;
    //! Timer used to delete the worker thread if thread is not doing anything for some time
    QTimer mIdleTimer;
};

/// @endcond



/**
 * \ingroup core
 *
 * \brief Tile download manager handles downloads of map tiles for the purpose of map rendering.
 * The purpose of this class is to handle a couple of situations that may happen:
 *
 * - a map rendering job starts which requests tiles from a remote server, then in a short
 *   while user zooms/pans map, which would normally mean that all pending requests get
 *   aborted and then restarted soon afterwards. The download manager allows the requests
 *   to finish to avoid excessive load on servers and needless aborts and repeated requests.
 * - multiple map rendering jobs start at a similar time, requesting the same map tiles.
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
 * - Added in QGIS 3.26: If the request is an HTTP range request, the data may be cached
 *   into a local directory using QgsRangeRequestCache to avoid requesting the same data
 *   excessively. Caching will be disabled for requests with CacheLoadControlAttribute set
 *   to AlwaysNetwork or CacheSaveControlAttribute set to false
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsTileDownloadManager
{

    //! An entry in the queue of requests to be handled by this class
    class QueueEntry
    {
      public:
        bool isValid() const { return !request.url().isEmpty(); }

        //! The actual original Qt network request
        QNetworkRequest request;
        //! Helper QObject that lives in worker thread that emits signals
        QgsTileDownloadManagerReplyWorkerObject *objWorker = nullptr;
        //! Internal network reply - only to be touched by the worker thread
        QNetworkReply *networkReply = nullptr;
    };

  public:

    /**
     * \ingroup core
     * \brief Encapsulates any statistics we would like to keep about requests
     * \since QGIS 3.18
     */
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
    ~QgsTileDownloadManager();

    /**
     * Starts a request. Returns a new object that should be deleted by the caller
     * when not needed anymore.
     */
    QgsTileDownloadManagerReply *get( const QNetworkRequest &request );

    //! Returns whether there are any pending requests in the queue
    bool hasPendingRequests() const;

    /**
     * Blocks the current thread until the queue is empty. This should not be used
     * in production code, it is however useful for auto tests
     */
    bool waitForPendingRequests( int msec = -1 ) const;

    //! Asks the worker thread to stop and blocks until it is not stopped.
    void shutdown();

    /**
     * Returns whether the worker thread is running currently (it may be stopped
     * if there were no requests recently
     */
    bool hasWorkerThreadRunning() const;

    /**
     * Sets after how many milliseconds the idle worker therad should terminate.
     * This function is meant mainly for unit testing.
     */
    void setIdleThreadTimeout( int timeoutMs ) { mIdleThreadTimeoutMs = timeoutMs; }

    //! Returns basic statistics of the queries handled by this class
    Stats statistics() const { return mStats; }

    //! Resets statistics of numbers of queries handled by this class
    void resetStatistics();

    friend class QgsTileDownloadManagerWorker;
    friend class QgsTileDownloadManagerReply;
    friend class QgsTileDownloadManagerReplyWorkerObject;

  private:

    // these can be only used with mutex locked!
    QueueEntry findEntryForRequest( const QNetworkRequest &request );
    void addEntry( const QueueEntry &entry );
    void updateEntry( const QueueEntry &entry );
    void removeEntry( const QNetworkRequest &request );
    void processStagedEntryRemovals();

    void signalQueueModified();

    bool isRangeRequest( const QNetworkRequest &request );
    bool isCachedRangeRequest( const QNetworkRequest &request );

  private:

    std::vector<QueueEntry> mQueue;

    bool mStageQueueRemovals = false;
    std::vector< QNetworkRequest > mStagedQueueRemovals;

    bool mShuttingDown = false;
    mutable QRecursiveMutex mMutex;

    QThread *mWorkerThread = nullptr;
    QgsTileDownloadManagerWorker *mWorker = nullptr;
    Stats mStats;

    int mIdleThreadTimeoutMs = 10000;

    std::unique_ptr<QgsRangeRequestCache> mRangesCache;
};

#endif // QGSTILEDOWNLOADMANAGER_H
