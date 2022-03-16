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
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsentryimpl.h"

class QgsFeedback;

#ifndef SIP_RUN
#include "qgsconfig.h"
constexpr int sFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );

#define QgsSetRequestInitiatorClass(request, _class) request.setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorClass ), _class ); request.setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorRequestId ), QString(QString( __FILE__ ).mid( sFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")") );
#define QgsSetRequestInitiatorId(request, str) request.setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorRequestId ), QString(QString( __FILE__ ).mid( sFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + "): " + str) );
#endif

/**
 * \class QgsNetworkRequestParameters
 * \ingroup core
 * \brief Encapsulates parameters and properties of a network request.
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
 * \brief QgsSslErrorHandler responds to SSL errors encountered during network requests. The
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

/**
 * \class QgsNetworkAuthenticationHandler
 * \brief Network authentication handler, used for responding to network authentication requests during network requests.
 * \ingroup core
 *
 * \brief QgsNetworkAuthenticationHandler responds to authentication requests encountered during network requests. The
 * base QgsNetworkAuthenticationHandler class responds to requests only by logging the request,
 * but does not provide any username or password to allow the request to proceed.
 *
 * Subclasses can override this behavior by implementing their own handleAuthRequest()
 * method. QgsNetworkAuthenticationHandler are ONLY ever called from the main thread, so it
 * is safe to utilize gui widgets and dialogs during handleAuthRequest (e.g. to
 * present prompts to users requesting the username and password).
 *
 * If a reply is coming from background thread, that thread is blocked while handleAuthRequest()
 * is running.
 *
 * An application instance can only have a single network authentication handler. The current
 * authentication handler is set by calling QgsNetworkAccessManager::setAuthHandler().
 * By default an instance of the logging-only QgsNetworkAuthenticationHandler base class is used.
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsNetworkAuthenticationHandler
{

  public:

    virtual ~QgsNetworkAuthenticationHandler() = default;

    /**
     * Called whenever network authentication requests are encountered during a network \a reply.
     *
     * Subclasses should reimplement this method to implement their own logic
     * regarding how to handle the requests and whether they should be presented to users.
     *
     * The base class method just logs the request but does not provide any username/password resolution.
     */
    virtual void handleAuthRequest( QNetworkReply *reply, QAuthenticator *auth );

    /**
     * Called to initiate a network authentication through external browser \a url.
     *
     * \since QGIS 3.20
     */
    virtual void handleAuthRequestOpenBrowser( const QUrl &url );

    /**
     * Called to terminate a network authentication through external browser.
     *
     * \since QGIS 3.20
     */
    virtual void handleAuthRequestCloseBrowser();

};
#endif


/**
 * \class QgsNetworkAccessManager
 * \brief network access manager for QGIS
 * \ingroup core
 *
 * \brief This class implements the QGIS network access manager.  It's a singleton
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

    /**
     * Sets the application network authentication \a handler, which is used to respond to network
     * authentication prompts during network requests.
     *
     * Ownership of \a handler is transferred to the main thread QgsNetworkAccessManager instance.
     *
     * This method must ONLY be called on the main thread QgsNetworkAccessManager. It is not
     * necessary to set handlers for background threads -- the main thread QgsNetworkAuthenticationHandler will
     * automatically be used in a thread-safe manner for any authentication requests encountered on background threads.
     *
     * The default QgsNetworkAuthenticationHandler responds to request only by logging the request,
     * but does not provide any username or password resolution.
     *
     * \note Not available in Python bindings.
     * \since QGIS 3.6
     */
    void setAuthHandler( std::unique_ptr< QgsNetworkAuthenticationHandler > handler );
#endif

    /**
     * Inserts a \a factory into the proxy factories list.
     *
     * Ownership of \a factory is transferred to the manager.
     *
     * \see removeProxyFactory()
     * \see proxyFactories()
     */
    void insertProxyFactory( QNetworkProxyFactory *factory SIP_TRANSFER );

    /**
     * Removes a \a factory from the proxy factories list.
     *
     * \see insertProxyFactory()
     * \see proxyFactories()
     */
    void removeProxyFactory( QNetworkProxyFactory *factory SIP_TRANSFERBACK );

    /**
     * Returns a list of proxy factories used by the manager.
     *
     * \see insertProxyFactory()
     * \see proxyFactories()
     */
    const QList<QNetworkProxyFactory *> proxyFactories() const;

    /**
     * Returns the fallback proxy used by the manager.
     *
     * The fallback proxy is used for URLs which no other proxy factory returned proxies for.
     *
     * \see proxyFactories()
     * \see setFallbackProxyAndExcludes()
     */
    const QNetworkProxy &fallbackProxy() const;

    /**
     * Returns the proxy exclude list.
     *
     * This list consists of the beginning of URL strings which will not use the fallback proxy.
     *
     * \see noProxyList()
     * \see fallbackProxy()
     * \see setFallbackProxyAndExcludes()
     */
    QStringList excludeList() const;

    /**
     * Returns the no proxy list.
     *
     * This list consists of the beginning of URL strings which will not use any proxy at all
     *
     * \see excludeList()
     * \see fallbackProxy()
     * \see setFallbackProxyAndExcludes()
     */
    QStringList noProxyList() const;

    /**
     * Sets the fallback \a proxy and URLs which shouldn't use it.
     *
     * The fallback proxy is used for URLs which no other proxy factory returned proxies for.
     * The \a excludes list specifies the beginning of URL strings which will not use this fallback proxy.
     * The \a noProxyURLs list specifies the beginning of URL strings which will not use any proxy at all
     *
     * \see fallbackProxy()
     * \see excludeList()
     * \see noProxyList()
     */
    void setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes, const QStringList &noProxyURLs );

    /**
     * Returns the name for QNetworkRequest::CacheLoadControl.
     *
     * \see cacheLoadControlFromName()
     */
    static QString cacheLoadControlName( QNetworkRequest::CacheLoadControl control );

    /**
     * Returns QNetworkRequest::CacheLoadControl from a \a name.
     *
     * \see cacheLoadControlName()
     */
    static QNetworkRequest::CacheLoadControl cacheLoadControlFromName( const QString &name );

    /**
     * Setup the QgsNetworkAccessManager (NAM) according to the user's settings.
     * The \a connectionType sets up the default connection type that is used to
     * handle signals that might require user interaction and therefore
     * need to be handled on the main thread. See in-depth discussion in the documentation
     * for the constructor of this class.
     */
    void setupDefaultProxyAndCache( Qt::ConnectionType connectionType = Qt::BlockingQueuedConnection );

#ifndef SIP_RUN

    /**
     * Returns TRUE if all network caching is disabled.
     *
     * \see setCacheDisabled()
     * \note Not available in Python bindings.
     * \since QGIS 3.18
     */
    bool cacheDisabled() const { return mCacheDisabled; }

    /**
     * Sets whether all network caching should be disabled.
     *
     * If \a disabled is TRUE then all caching will be disabled, causing all requests
     * to be retrieved from the network regardless of the request's attributes.
     *
     * \see cacheDisabled()
     * \note Not available in Python bindings.
     * \since QGIS 3.18
     */
    void setCacheDisabled( bool disabled ) { mCacheDisabled = disabled; }
#endif

    /**
     * Returns whether the system proxy should be used.
     */
    bool useSystemProxy() const { return mUseSystemProxy; }

    /**
     * Returns the network timeout length, in milliseconds.
     *
     * \see setTimeout()
     * \since QGIS 3.6
     */
    static int timeout();

    /**
     * Sets the maximum timeout \a time for network requests, in milliseconds.
     * If set to 0, no timeout is set.
     *
     * \see timeout()
     * \since QGIS 3.6
     */
    static void setTimeout( int time );

    /**
     * Posts a GET request to obtain the contents of the target request and returns a new QgsNetworkReplyContent object for reading.
     * The current thread will be blocked until the request is returned.
     *
     * This method is safe to call in either the main thread or a worker thread.
     *
     * If \a forceRefresh is FALSE then previously cached replies may be used for the request. If
     * it is set to TRUE then a new query is always performed.
     *
     * If an \a authCfg has been specified, then that authentication configuration required will automatically be applied to
     * \a request. There is no need to manually apply the authentication to the request prior to calling
     * this method.
     *
     * The optional \a feedback argument can be used to abort ongoing requests.
     *
     * The contents of the reply will be returned after the request is completed or an error occurs.
     *
     * \see blockingPost()
     * \since QGIS 3.6
     */
    static QgsNetworkReplyContent blockingGet( QNetworkRequest &request, const QString &authCfg = QString(), bool forceRefresh = false, QgsFeedback *feedback = nullptr );

    /**
     * Posts a POST request to obtain the contents of the target \a request, using the given \a data, and returns a new
     * QgsNetworkReplyContent object for reading. The current thread will be blocked until the request is returned.
     *
     * This method is safe to call in either the main thread or a worker thread.
     *
     * If \a forceRefresh is FALSE then previously cached replies may be used for the request. If
     * it is set to TRUE then a new query is always performed.
     *
     * If an \a authCfg has been specified, then that authentication configuration required will automatically be applied to
     * \a request. There is no need to manually apply the authentication to the request prior to calling
     * this method.
     *
     * The optional \a feedback argument can be used to abort ongoing requests.
     *
     * The contents of the reply will be returned after the request is completed or an error occurs.
     *
     * \see blockingGet()
     * \since QGIS 3.6
     */
    static QgsNetworkReplyContent blockingPost( QNetworkRequest &request, const QByteArray &data, const QString &authCfg = QString(), bool forceRefresh = false, QgsFeedback *feedback = nullptr );

    /**
     * Sets a request pre-processor function, which allows manipulation of a network request before it is processed.
     *
     * The \a processor function takes the QNetworkRequest as its argument, and can mutate the request if necessary.
     *
     * \returns An auto-generated string uniquely identifying the preprocessor, which can later be
     * used to remove the preprocessor (via a call to removeRequestPreprocessor()).
     *
     * \see removeRequestPreprocessor()
     * \since QGIS 3.22
     */
#ifndef SIP_RUN
    static QString setRequestPreprocessor( const std::function< void( QNetworkRequest *request )> &processor );
#else
    static QString setRequestPreprocessor( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    PyObject *s = 0;
    Py_BEGIN_ALLOW_THREADS
    Py_XINCREF( a0 );
    QString id = QgsNetworkAccessManager::setRequestPreprocessor( [a0]( QNetworkRequest *arg )->QString
    {
      QString res;
      SIP_BLOCK_THREADS
      PyObject *s = sipCallMethod( NULL, a0, "D", arg, sipType_QNetworkRequest, NULL );
      int state;
      int sipIsError = 0;
      QString *t1 = reinterpret_cast<QString *>( sipConvertToType( s, sipType_QString, 0, SIP_NOT_NONE, &state, &sipIsError ) );
      if ( sipIsError == 0 )
      {
        res = QString( *t1 );
      }
      sipReleaseType( t1, sipType_QString, state );
      SIP_UNBLOCK_THREADS
      return res;
    } );

    s = sipConvertFromNewType( new QString( id ), sipType_QString, 0 );
    Py_END_ALLOW_THREADS
    return s;
    % End
#endif

    /**
     * Removes the custom pre-processor function with matching \a id.
     *
     * The \a id must correspond to a pre-processor previously added via a call to setRequestPreprocessor().
     *
     * Returns TRUE if processor existed and was removed.
     *
     * \see setRequestPreprocessor()
     * \since QGIS 3.22
     */
#ifndef SIP_RUN
    static bool removeRequestPreprocessor( const QString &id );
#else
    static void removeRequestPreprocessor( const QString &id );
    % MethodCode
    if ( !QgsNetworkAccessManager::removeRequestPreprocessor( *a0 ) )
    {
      PyErr_SetString( PyExc_KeyError, QStringLiteral( "No processor with id %1 exists." ).arg( *a0 ).toUtf8().constData() );
      sipIsErr = 1;
    }
    % End
#endif

    /**
     * Forwards an external browser login \a url opening request to the authentication handler.
     *
     * \note If called by a background thread, the request will be forwarded to the network manager on the main thread.
     * \since QGIS 3.20
     */
    void requestAuthOpenBrowser( const QUrl &url ) const;

    /**
     * Forwards an external browser login closure request to the authentication handler.
     *
     * \note If called by a background thread, the request will be forwarded to the network manager on the main thread.
     * \since QGIS 3.20
     */
    void requestAuthCloseBrowser() const;

    /**
     * Abort any outstanding external browser login request.
     *
     * \note Background threads will listen to aborted browser request signals from the network manager on the main thread.
     * \since QGIS 3.20
     */
    void abortAuthBrowser();


#ifndef SIP_RUN
    //! Settings entry network timeout
    static const inline QgsSettingsEntryInteger settingsNetworkTimeout = QgsSettingsEntryInteger( QStringLiteral( "networkTimeout" ), QgsSettings::Prefix::QGIS_NETWORKANDPROXY, 60000, QObject::tr( "Network timeout" ) );
#endif

    /**
     * Preprocesses request
     * \param req the request to preprocess
     * \since QGIS 3.22
     */
    void preprocessRequest( QNetworkRequest *req ) const;

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
     * Emitted whenever a pending network reply is finished.
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

    /**
     * Emitted when a network request prompts an authentication request.
     *
     * The \a requestId argument reflects the unique ID identifying the original request which the authentication relates to.
     *
     * This signal is propagated to the main thread QgsNetworkAccessManager instance, so it is necessary
     * only to connect to the main thread's signal in order to receive notifications about authentication requests
     * from any thread.
     *
     * This signal is for debugging and logging purposes only, and cannot be used to respond to the
     * requests. See QgsNetworkAuthenticationHandler for details on how to handle authentication requests.
     *
     * \see requestAuthDetailsAdded()
     * \since QGIS 3.6
     */
    void requestRequiresAuth( int requestId, const QString &realm );

    /**
     * Emitted when network authentication details have been added to a request.
     *
     * The \a requestId argument reflects the unique ID identifying the original request which the authentication relates to.
     *
     * This signal is always sent from the main thread QgsNetworkAccessManager instance, so it is necessary
     * only to connect to the main thread's signal in order to receive notifications about authentication requests
     * from any thread.
     *
     * This signal is for debugging and logging purposes only, and should not be used to respond to the
     * requests. See QgsNetworkAuthenticationHandler for details on how to handle authentication requests.
     *
     * \see requestRequiresAuth()
     * \since QGIS 3.6
     */
    void requestAuthDetailsAdded( int requestId, const QString &realm, const QString &user, const QString &password );

#ifndef QT_NO_SSL

    /**
     * Emitted when a network request encounters SSL \a errors.
     *
     * The \a requestId argument reflects the unique ID identifying the original request which the SSL error relates to.
     *
     * This signal is propagated to the main thread QgsNetworkAccessManager instance, so it is necessary
     * only to connect to the main thread's signal in order to receive notifications about SSL errors
     * from any thread.
     *
     * This signal is for debugging and logging purposes only, and cannot be used to respond to the errors.
     * See QgsSslErrorHandler for details on how to handle SSL errors and potentially ignore them.
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

#ifndef SIP_RUN
///@cond PRIVATE
    // these signals are for internal use only - it's not safe to connect by external code
    void authRequestOccurred( QNetworkReply *, QAuthenticator *auth );
    void authRequestHandled( QNetworkReply *reply );
///@endcond
#endif

    /**
     * Emitted when external browser logins are to be aborted.
     *
     * \since QGIS 3.20
     */
    void authBrowserAborted();

    /**
     * Emitted when the cookies changed.
     * \since QGIS 3.22
     */

    void cookiesChanged( const QList<QNetworkCookie> &cookies );

  private slots:
    void abortRequest();

    void onReplyFinished( QNetworkReply *reply );

    void onReplyDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );
#ifndef QT_NO_SSL
    void onReplySslErrors( const QList<QSslError> &errors );

    void handleSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif

    void onAuthRequired( QNetworkReply *reply, QAuthenticator *auth );
    void handleAuthRequest( QNetworkReply *reply, QAuthenticator *auth );

    void syncCookies( const QList<QNetworkCookie> &cookies );

  protected:
    QNetworkReply *createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData = nullptr ) override;

  private:
#ifndef QT_NO_SSL
    void unlockAfterSslErrorHandled();
    void afterSslErrorHandled( QNetworkReply *reply );
#endif

    void afterAuthRequestHandled( QNetworkReply *reply );

    void pauseTimeout( QNetworkReply *reply );
    void restartTimeout( QNetworkReply *reply );
    static int getRequestId( QNetworkReply *reply );

    QList<QNetworkProxyFactory *> mProxyFactories;
    QNetworkProxy mFallbackProxy;
    QStringList mExcludedURLs;
    QStringList mNoProxyURLs;
    bool mUseSystemProxy = false;
    bool mInitialized = false;
    bool mCacheDisabled = false;
    static QgsNetworkAccessManager *sMainNAM;
    // ssl error handler, will be set for main thread ONLY
    std::unique_ptr< QgsSslErrorHandler > mSslErrorHandler;
    // only in use by worker threads, unused in main thread
    QMutex mSslErrorHandlerMutex;
    // only in use by worker threads, unused in main thread
    QWaitCondition mSslErrorWaitCondition;

    // auth request handler, will be set for main thread ONLY
    std::unique_ptr< QgsNetworkAuthenticationHandler > mAuthHandler;
    // Used by worker threads to wait for authentication handler run in main thread
    QSemaphore mAuthRequestHandlerSemaphore;
};

#endif // QGSNETWORKACCESSMANAGER_H
