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
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>

#include "qgis_core.h"

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

    /**
     * Pauses the timeout on a network \a reply.
     *
     * This must be called from the same thread as \a reply was created from. Calling from a different
     * thread will have no effect on the timeout.
     *
     * \see restartReplyTimeout()
     * \since QGIS 3.6
     */
    static void pauseReplyTimeout( QNetworkReply *reply );

    /**
     * Restarts the timeout on a network \a reply.
     *
     * This must be called from the same thread as \a reply was created from. Calling from a different
     * thread will have no effect on the timeout.
     *
     * \see pauseReplyTimeout()
     * \since QGIS 3.6
     */
    static void restartReplyTimeout( QNetworkReply *reply );

  signals:
    void requestAboutToBeCreated( QNetworkAccessManager::Operation, const QNetworkRequest &, QIODevice * );
    void requestCreated( QNetworkReply * );
    void requestTimedOut( QNetworkReply * );

  private slots:
    void abortRequest();

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


/**
 * Temporarily blocks QNetworkReply timeouts for the lifetime of the object.
 *
 * On object destruction the reply timeout will be restarted.
 *
 * This must be created from the same thread as \a reply was created from. Creating from a different
 * thread will have no effect on the timeout.
 *
 * \ingroup core
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsNetworkReplyTimeoutBlocker
{
  public:

    /**
     * Constructor for QgsNetworkReplyTimeoutBlocker.
     *
     * This will block network reply timeouts for the specified \a reply for the lifetime of this object.
     *
     * This must be created from the same thread as \a reply was created from. Creating from a different
     * thread will have no effect on the timeout.
     */
    QgsNetworkReplyTimeoutBlocker( QNetworkReply *reply )
      : mReply( reply )
    {
      if ( mReply )
        QgsNetworkAccessManager::pauseReplyTimeout( mReply );
    }

    //! QgsNetworkReplyTimeoutBlocker cannot be copied
    QgsNetworkReplyTimeoutBlocker( const QgsNetworkReplyTimeoutBlocker &other ) = delete;

    //! QgsNetworkReplyTimeoutBlocker cannot be copied
    QgsNetworkReplyTimeoutBlocker &operator=( const QgsNetworkReplyTimeoutBlocker &other ) = delete;

    ~QgsNetworkReplyTimeoutBlocker()
    {
      if ( mReply )
        QgsNetworkAccessManager::restartReplyTimeout( mReply );
    }

  private:
    QPointer< QNetworkReply > mReply;

#ifdef SIP_RUN
    QgsNetworkReplyTimeoutBlocker( const QgsNetworkReplyTimeoutBlocker &other );
#endif
};


#endif // QGSNETWORKACCESSMANAGER_H

