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
#include "qgsnetworkreply.h"
#include "qgis_sip.h"
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QMutex>
#include <QWaitCondition>
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"

#ifndef SIP_RUN
#include "qgsconfig.h"
constexpr int sFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );

#define QgsSetRequestInitiatorClass(request, _class) request.setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorClass ), _class ); request.setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorRequestId ), QString( __FILE__ ).mid( sFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")" );
#define QgsSetRequestInitiatorId(request, str) request.setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorRequestId ), QString( __FILE__ ).mid( sFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + "): " + str );
#endif

/**
 * \class QgsNetworkRequestParameters
 * \ingroup core
 * Encapsulates parameters and properties of a network request.
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsNetworkRequestParameters
{
  public:

    //! Custom request attributes
    enum RequestAttributes
    {
      AttributeInitiatorClass = QNetworkRequest::User + 3000, //!< Class name of original object which created the request
      AttributeInitiatorRequestId, //!< Internal ID used by originator object to identify requests
    };

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
                                 int requestId,
                                 const QByteArray &content = QByteArray() );

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

    /**
     * Returns the request's content. This is only used for POST or PUT operation
     * requests.
     */
    QByteArray content() const { return mContent; }

    /**
     * Returns the class name of the object which initiated this request.
     *
     * This is only available for QNetworkRequests which have had the
     * QgsNetworkRequestParameters::AttributeInitiatorClass attribute set.
     *
     * \see initiatorRequestId()
     */
    QString initiatorClassName() const { return mInitiatorClass; }

    /**
     * Returns the internal ID used by the object which initiated this request to identify
     * individual requests.
     *
     * This is only available for QNetworkRequests which have had the
     * QgsNetworkRequestParameters::AttributeInitiatorRequestId attribute set.
     *
     * \see initiatorClassName()
     */
    QVariant initiatorRequestId() const { return mInitiatorRequestId; }

  private:

    QNetworkAccessManager::Operation mOperation;
    QNetworkRequest mRequest;
    QString mOriginatingThreadId;
    int mRequestId = 0;
    QByteArray mContent;
    QString mInitiatorClass;
    QVariant mInitiatorRequestId;
};

class QgsNetworkAccessManager;

#ifndef SIP_RUN

/**
 * \class QgsSslErrorHandler
 * \brief SSL error handler, used for responding to SSL errors encountered during network requests.
 * \ingroup core
 *
 * QgsSslErrorHandler responds to SSL errors encountered during network requests. The
 * base QgsSslErrorHandler class responds to SSL errors only by logging the errors,
 * and uses the default Qt response, which is to abort the request.
 *
 * Subclasses can override this behavior by implementing their own handleSslErrors()
 * method. QgsSslErrorHandlers are ONLY ever called from the main thread, so it
 * is safe to utilize gui widgets and dialogs during handleSslErrors (e.g. to
 * present prompts to users notifying them of the errors and asking them
 * to choose the appropriate response.).
 *
 * If a reply is coming from background thread, that thread is blocked while handleSslErrors()
 * is running.
 *
 * If the errors should be ignored and the request allowed to proceed, the subclasses'
 * handleSslErrors() method MUST call QNetworkReply::ignoreSslErrors() on the specified
 * QNetworkReply object.
 *
 * An application instance can only have a single SSL error handler. The current
 * SSL error handler is set by calling QgsNetworkAccessManager::setSslErrorHandler().
 * By default an instance of the logging-only QgsSslErrorHandler base class is used.
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsSslErrorHandler
{

  public:

    virtual ~QgsSslErrorHandler() = default;

    /**
     * Called whenever SSL \a errors are encountered during a network \a reply.
     *
     * Subclasses should reimplement this method to implement their own logic
     * regarding whether or not these SSL errors should be ignored, and how
     * to present them to users.
     *
     * The base class method just logs errors and leaves the default Qt response
     * to SSL errors, which is to abort the network request on any errors.
     */
    virtual void handleSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );

};
#endif


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

#ifndef SIP_RUN

    /**
     * Sets the application SSL error \a handler, which is used to respond to SSL errors encountered
     * during network requests.
     *
     * Ownership of \a handler is transferred to the main thread QgsNetworkAccessManager instance.
     *
     * This method must ONLY be called on the main thread QgsNetworkAccessManager. It is not
     * necessary to set handlers for background threads -- the main thread QgsSslErrorHandler will
     * automatically be used in a thread-safe manner for any SSL errors encountered on background threads.
     *
     * The default QgsSslErrorHandler responds to SSL errors only by logging the errors,
     * and uses the default Qt response, which is to abort the request.
     *
     * \note Not available in Python bindings.
     * \since QGIS 3.6
     */
    void setSslErrorHandler( std::unique_ptr< QgsSslErrorHandler > handler );
#endif

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
     * Emitted when a network reply receives a progress report.
     *
     * The \a requestId argument reflects the unique ID identifying the original request which the progress report relates to.
     *
     * The \a bytesReceived parameter indicates the number of bytes received, while \a bytesTotal indicates the total number
     * of bytes expected to be downloaded. If the number of bytes to be downloaded is not known, \a bytesTotal will be -1.
     *
     * This signal is propagated to the main thread QgsNetworkAccessManager instance, so it is necessary
     * only to connect to the main thread's signal in order to receive notifications about requests
     * created in any thread.
     *
     * \since QGIS 3.6
     */
    void downloadProgress( int requestId, qint64 bytesReceived, qint64 bytesTotal );

#ifndef QT_NO_SSL

    /**
     * Emitted when a network request encounters SSL \a errors.
     *
     * The \a requestId argument reflects the unique ID identifying the original request which the progress report relates to.
     *
     * This signal is propagated to the main thread QgsNetworkAccessManager instance, so it is necessary
     * only to connect to the main thread's signal in order to receive notifications about SSL errors
     * from any thread.
     *
     * \since QGIS 3.6
     */
    void requestEncounteredSslErrors( int requestId, const QList<QSslError> &errors );

#ifndef SIP_RUN
///@cond PRIVATE
    // these signals are for internal use only - it's not safe to connect by external code
    void sslErrorsOccurred( QNetworkReply *, const QList<QSslError> &errors );
    void sslErrorsHandled( QNetworkReply *reply );
///@endcond
#endif

#endif

    /**
     * \deprecated Use the thread-safe requestAboutToBeCreated( QgsNetworkRequestParameters ) signal instead.
     */
    Q_DECL_DEPRECATED void requestCreated( QNetworkReply * ) SIP_DEPRECATED;

    void requestTimedOut( QNetworkReply * );


  private slots:
    void abortRequest();

    void onReplyFinished( QNetworkReply *reply );

    void onReplyDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );
#ifndef QT_NO_SSL
    void onReplySslErrors( const QList<QSslError> &errors );

    void handleSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif
  protected:
    QNetworkReply *createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData = nullptr ) override;

  private:
#ifndef QT_NO_SSL
    void unlockAfterSslErrorHandled();
    void afterSslErrorHandled( QNetworkReply *reply );
#endif
    QList<QNetworkProxyFactory *> mProxyFactories;
    QNetworkProxy mFallbackProxy;
    QStringList mExcludedURLs;
    bool mUseSystemProxy = false;
    bool mInitialized = false;
    static QgsNetworkAccessManager *sMainNAM;
    // ssl error handler, will be set for main thread ONLY
    std::unique_ptr< QgsSslErrorHandler > mSslErrorHandler;
    // only in use by work threads, unused in main thread
    QMutex mSslErrorHandlerMutex;
    // only in use by work threads, unused in main thread
    QWaitCondition mSslErrorWaitCondition;
};

#endif // QGSNETWORKACCESSMANAGER_H

