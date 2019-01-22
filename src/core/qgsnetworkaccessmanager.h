/***************************************************************************
                          qgsnetworkaccessmanager.h  -  description
                             -------------------
    begin                : 2010-05-08
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNETWORKACCESSMANAGER_H
#define QGSNETWORKACCESSMANAGER_H

#include <QList>
#include "qgis.h"
#include "qgsnetworkreply.h"
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkRequest>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \class QgsNetworkRequestParameters
 * \ingroup core
 * Encapsulates parameters and properties of a network request.
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsNetworkRequestParameters
{
  public:

    /**
     * Default constructor.
     */
    QgsNetworkRequestParameters() = default;

    /**
     * Constructor for QgsNetworkRequestParameters, with the specified network
     * \a operation and original \a request.
     */
    QgsNetworkRequestParameters( QNetworkAccessManager::Operation operation,
                                 const QNetworkRequest &request,
                                 int requestId );

    /**
     * Returns the request operation, e.g. GET or POST.
     */
    QNetworkAccessManager::Operation operation() const { return mOperation; }

    /**
     * Returns the network request.
     *
     * This is the original network request sent to QgsNetworkAccessManager, but with QGIS specific
     * configuration options such as proxy handling and SSL exceptions applied.
     */
    QNetworkRequest request() const { return mRequest; }

    /**
     * Returns a string identifying the thread which the request originated from.
     */
    QString originatingThreadId() const { return mOriginatingThreadId; }

    /**
     * Returns a unique ID identifying the request.
     */
    int requestId() const { return mRequestId; }

  private:

    QNetworkAccessManager::Operation mOperation;
    QNetworkRequest mRequest;
    QString mOriginatingThreadId;
    int mRequestId = 0;
};

/**
 * \class QgsNetworkAccessManager
 * \brief network access manager for QGIS
 * \ingroup core
 *
 * This class implements the QGIS network access manager.  It's a singleton
 * that can be used across QGIS.
 *
 * Plugins can insert proxy factories and thereby redirect requests to
 * individual proxies.
 *
 * If no proxy factories are there or none returns a proxy for an URL a
 * fallback proxy can be set.  There's also a exclude list that defines URLs
 * that the fallback proxy should not be used for, then no proxy will be used.
 *
 * \since 1.5
 */
class CORE_EXPORT QgsNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

  public:

    /**
     * Returns a pointer to the active QgsNetworkAccessManager
     * for the current thread.
     *
     * With the \a connectionType parameter it is possible to setup the default connection
     * type that is used to handle signals that might require user interaction and therefore
     * need to be handled on the main thread. See in-depth discussion below.
     *
     * \param connectionType In most cases the default of using a ``Qt::BlockingQueuedConnection``
     * is ok, to make a background thread wait for the main thread to answer such a request is
     * fine and anything else is dangerous.
     * However, in case the request was started on the main thread, one should execute a
     * local event loop in a helper thread and freeze the main thread for the duration of the
     * download. In this case, if an authentication request is sent from the background thread
     * network access manager, the background thread should be blocked, the main thread be woken
     * up, processEvents() executed once, the main thread frozen again and the background thread
     * continued.
     */
    static QgsNetworkAccessManager *instance( Qt::ConnectionType connectionType = Qt::BlockingQueuedConnection );

    QgsNetworkAccessManager( QObject *parent = nullptr );

    //! insert a factory into the proxy factories list
    void insertProxyFactory( QNetworkProxyFactory *factory SIP_TRANSFER );

    //! remove a factory from the proxy factories list
    void removeProxyFactory( QNetworkProxyFactory *factory SIP_TRANSFERBACK );

    //! retrieve proxy factory list
    const QList<QNetworkProxyFactory *> proxyFactories() const;

    //! retrieve fall back proxy (for urls that no factory returned proxies for)
    const QNetworkProxy &fallbackProxy() const;

    //! retrieve exclude list (urls shouldn't use the fallback proxy)
    QStringList excludeList() const;

    //! Sets fallback proxy and URL that shouldn't use it.
    void setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes );

    //! Gets name for QNetworkRequest::CacheLoadControl
    static QString cacheLoadControlName( QNetworkRequest::CacheLoadControl control );

    //! Gets QNetworkRequest::CacheLoadControl from name
    static QNetworkRequest::CacheLoadControl cacheLoadControlFromName( const QString &name );

    /**
     * Setup the QgsNetworkAccessManager (NAM) according to the user's settings.
     * The \a connectionType sets up the default connection type that is used to
     * handle signals that might require user interaction and therefore
     * need to be handled on the main thread. See in-depth discussion in the documentation
     * for the constructor of this class.
     */
    void setupDefaultProxyAndCache( Qt::ConnectionType connectionType = Qt::BlockingQueuedConnection );

    //! Returns whether the system proxy should be used
    bool useSystemProxy() const { return mUseSystemProxy; }

  signals:

    /**
     * \deprecated Use the thread-safe requestAboutToBeCreated( QgsNetworkRequestParameters ) signal instead.
     */
    Q_DECL_DEPRECATED void requestAboutToBeCreated( QNetworkAccessManager::Operation, const QNetworkRequest &, QIODevice * ) SIP_DEPRECATED;

    /**
     * Emitted when a network request is about to be created.
     *
     * This signal is propagated to the main thread QgsNetworkAccessManager instance, so it is necessary
     * only to connect to the main thread's signal in order to receive notifications about requests
     * created in any thread.
     *
     * \see finished( QgsNetworkReplyContent )
     * \see requestTimedOut( QgsNetworkRequestParameters )
     * \since QGIS 3.6
     */
    void requestAboutToBeCreated( QgsNetworkRequestParameters request );

    /**
     * This signal is emitted whenever a pending network reply is finished.
     *
     * The \a reply parameter will contain a QgsNetworkReplyContent object, containing all the useful
     * information relating to the reply, including headers and reply content.
     *
     * This signal is propagated to the main thread QgsNetworkAccessManager instance, so it is necessary
     * only to connect to the main thread's signal in order to receive notifications about requests
     * created in any thread.
     *
     * \see requestAboutToBeCreated( QgsNetworkRequestParameters )
     * \see requestTimedOut( QgsNetworkRequestParameters )
     * \since QGIS 3.6
     */
    void finished( QgsNetworkReplyContent reply );

    /**
     * Emitted when a network request has timed out.
     *
     * This signal is propagated to the main thread QgsNetworkAccessManager instance, so it is necessary
     * only to connect to the main thread's signal in order to receive notifications about requests
     * created in any thread.
     *
     * \see requestAboutToBeCreated( QgsNetworkRequestParameters )
     * \see finished( QgsNetworkReplyContent )
     * \since QGIS 3.6
     */
    void requestTimedOut( QgsNetworkRequestParameters request );

    /**
     * \deprecated Use the thread-safe requestAboutToBeCreated( QgsNetworkRequestParameters ) signal instead.
     */
    Q_DECL_DEPRECATED void requestCreated( QNetworkReply * ) SIP_DEPRECATED;

    void requestTimedOut( QNetworkReply * );

  private slots:
    void abortRequest();

    void onReplyFinished( QNetworkReply *reply );

  protected:
    QNetworkReply *createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData = nullptr ) override;

  private:
    QList<QNetworkProxyFactory *> mProxyFactories;
    QNetworkProxy mFallbackProxy;
    QStringList mExcludedURLs;
    bool mUseSystemProxy = false;
    bool mInitialized = false;
    static QgsNetworkAccessManager *sMainNAM;
};

#endif // QGSNETWORKACCESSMANAGER_H

