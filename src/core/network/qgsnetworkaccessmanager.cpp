/***************************************************************************
                       qgsnetworkaccessmanager.cpp
  This class implements a QNetworkManager with the ability to chain in
  own proxy factories.

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

#include "qgsnetworkaccessmanager.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkdiskcache.h"
#include "qgsnetworkreply.h"
#include "qgssettings.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"
#include "qgssettingstree.h"

#include <QAuthenticator>
#include <QBuffer>
#include <QNetworkReply>
#include <QRecursiveMutex>
#include <QStandardPaths>
#include <QThreadStorage>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include "moc_qgsnetworkaccessmanager.cpp"

const QgsSettingsEntryInteger *QgsNetworkAccessManager::settingsNetworkTimeout = new QgsSettingsEntryInteger( u"network-timeout"_s, QgsSettingsTree::sTreeNetwork, 60000, QObject::tr( "Network timeout" ) );

#ifndef QT_NO_SSL
#include <QSslConfiguration>
#endif

#include "qgsnetworkdiskcache.h"
#include "qgsauthmanager.h"

QgsNetworkAccessManager *QgsNetworkAccessManager::sMainNAM = nullptr;

static std::vector< std::pair< QString, std::function< void( QNetworkRequest * ) > > > sCustomPreprocessors;
static std::vector< std::pair< QString, std::function< void( QNetworkRequest *, int &op, QByteArray *data ) > > > sCustomAdvancedPreprocessors;
static std::vector< std::pair< QString, std::function< void( const QNetworkRequest &, QNetworkReply * ) > > > sCustomReplyPreprocessors;

/// @cond PRIVATE
class QgsNetworkProxyFactory : public QNetworkProxyFactory
{
  public:
    QgsNetworkProxyFactory() = default;

    QList<QNetworkProxy> queryProxy( const QNetworkProxyQuery &query = QNetworkProxyQuery() ) override
    {
      QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

      // iterate proxies factories and take first non empty list
      const auto constProxyFactories = nam->proxyFactories();
      for ( QNetworkProxyFactory *f : constProxyFactories )
      {
        QList<QNetworkProxy> systemproxies = QNetworkProxyFactory::systemProxyForQuery( query );
        if ( !systemproxies.isEmpty() )
          return systemproxies;

        QList<QNetworkProxy> proxies = f->queryProxy( query );
        if ( !proxies.isEmpty() )
          return proxies;
      }

      // no proxies from the proxy factory list check for excludes
      if ( query.queryType() != QNetworkProxyQuery::UrlRequest )
        return QList<QNetworkProxy>() << nam->fallbackProxy();

      const QString url = query.url().toString();

      const auto constNoProxyList = nam->noProxyList();
      for ( const QString &noProxy : constNoProxyList )
      {
        if ( !noProxy.trimmed().isEmpty() && url.startsWith( noProxy ) )
        {
          QgsDebugMsgLevel( u"don't using any proxy for %1 [exclude %2]"_s.arg( url, noProxy ), 4 );
          return QList<QNetworkProxy>() << QNetworkProxy( QNetworkProxy::NoProxy );
        }
      }

      const auto constExcludeList = nam->excludeList();
      for ( const QString &exclude : constExcludeList )
      {
        if ( !exclude.trimmed().isEmpty() && url.startsWith( exclude ) )
        {
          QgsDebugMsgLevel( u"using default proxy for %1 [exclude %2]"_s.arg( url, exclude ), 4 );
          return QList<QNetworkProxy>() << QNetworkProxy( QNetworkProxy::DefaultProxy );
        }
      }

      if ( nam->useSystemProxy() )
      {
        QgsDebugMsgLevel( u"requesting system proxy for query %1"_s.arg( url ), 4 );
        QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery( query );
        if ( !proxies.isEmpty() )
        {
          QgsDebugMsgLevel( u"using system proxy %1:%2 for query"_s
                            .arg( proxies.first().hostName() ).arg( proxies.first().port() ), 4 );
          return proxies;
        }
      }

      QgsDebugMsgLevel( u"using fallback proxy for %1"_s.arg( url ), 4 );
      return QList<QNetworkProxy>() << nam->fallbackProxy();
    }
};
///@endcond

/// @cond PRIVATE
class QgsNetworkCookieJar : public QNetworkCookieJar
{
    Q_OBJECT

  public:
    QgsNetworkCookieJar( QgsNetworkAccessManager *parent )
      : QNetworkCookieJar( parent )
      , mNam( parent )
    {}

    bool deleteCookie( const QNetworkCookie &cookie ) override
    {
      const QMutexLocker locker( &mMutex );
      if ( QNetworkCookieJar::deleteCookie( cookie ) )
      {
        emit mNam->cookiesChanged( allCookies() );
        return true;
      }
      return false;
    }
    bool insertCookie( const QNetworkCookie &cookie ) override
    {
      const QMutexLocker locker( &mMutex );
      if ( QNetworkCookieJar::insertCookie( cookie ) )
      {
        emit mNam->cookiesChanged( allCookies() );
        return true;
      }
      return false;
    }
    bool setCookiesFromUrl( const QList<QNetworkCookie> &cookieList, const QUrl &url ) override
    {
      const QMutexLocker locker( &mMutex );
      return QNetworkCookieJar::setCookiesFromUrl( cookieList, url );
    }
    bool updateCookie( const QNetworkCookie &cookie ) override
    {
      const QMutexLocker locker( &mMutex );
      if ( QNetworkCookieJar::updateCookie( cookie ) )
      {
        emit mNam->cookiesChanged( allCookies() );
        return true;
      }
      return false;
    }

    // Override these to make them public
    QList<QNetworkCookie> allCookies() const
    {
      const QMutexLocker locker( &mMutex );
      return QNetworkCookieJar::allCookies();
    }
    void setAllCookies( const QList<QNetworkCookie> &cookieList )
    {
      const QMutexLocker locker( &mMutex );
      QNetworkCookieJar::setAllCookies( cookieList );
    }

    QgsNetworkAccessManager *mNam = nullptr;
    mutable QRecursiveMutex mMutex;
};
///@endcond


//
// Static calls to enforce singleton behavior
//
QgsNetworkAccessManager *QgsNetworkAccessManager::instance( Qt::ConnectionType connectionType )
{
  static QThreadStorage<QgsNetworkAccessManager> sInstances;
  QgsNetworkAccessManager *nam = &sInstances.localData();

  if ( nam->thread() == qApp->thread() )
    sMainNAM = nam;

  if ( !nam->mInitialized )
  {
    nam->setupDefaultProxyAndCache( connectionType );
    nam->setCacheDisabled( sMainNAM->cacheDisabled() );
  }

  return nam;
}

QgsNetworkAccessManager::QgsNetworkAccessManager( QObject *parent )
  : QNetworkAccessManager( parent )
  , mSslErrorHandlerSemaphore( 1 )
  , mAuthRequestHandlerSemaphore( 1 )
{
  setRedirectPolicy( QNetworkRequest::NoLessSafeRedirectPolicy );
  setProxyFactory( new QgsNetworkProxyFactory() );
  setCookieJar( new QgsNetworkCookieJar( this ) );
  enableStrictTransportSecurityStore( true );
  setStrictTransportSecurityEnabled( true );
}

void QgsNetworkAccessManager::setSslErrorHandler( std::unique_ptr<QgsSslErrorHandler> handler )
{
  Q_ASSERT( sMainNAM == this );
  mSslErrorHandler = std::move( handler );
}

void QgsNetworkAccessManager::setAuthHandler( std::unique_ptr<QgsNetworkAuthenticationHandler> handler )
{
  Q_ASSERT( sMainNAM == this );
  mAuthHandler = std::move( handler );
}

void QgsNetworkAccessManager::insertProxyFactory( QNetworkProxyFactory *factory )
{
  mProxyFactories.insert( 0, factory );
}

void QgsNetworkAccessManager::removeProxyFactory( QNetworkProxyFactory *factory )
{
  mProxyFactories.removeAll( factory );
}

const QList<QNetworkProxyFactory *> QgsNetworkAccessManager::proxyFactories() const
{
  return mProxyFactories;
}

QStringList QgsNetworkAccessManager::excludeList() const
{
  return mExcludedURLs;
}

QStringList QgsNetworkAccessManager::noProxyList() const
{
  return mNoProxyURLs;
}

const QNetworkProxy &QgsNetworkAccessManager::fallbackProxy() const
{
  return mFallbackProxy;
}

void QgsNetworkAccessManager::setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes, const QStringList &noProxyURLs )
{
  QgsDebugMsgLevel( u"proxy settings: (type:%1 host: %2:%3, user:%4, password:%5"_s
                    .arg( proxy.type() == QNetworkProxy::DefaultProxy ? u"DefaultProxy"_s :
                          proxy.type() == QNetworkProxy::Socks5Proxy ? u"Socks5Proxy"_s :
                          proxy.type() == QNetworkProxy::NoProxy ? u"NoProxy"_s :
                          proxy.type() == QNetworkProxy::HttpProxy ? u"HttpProxy"_s :
                          proxy.type() == QNetworkProxy::HttpCachingProxy ? u"HttpCachingProxy"_s :
                          proxy.type() == QNetworkProxy::FtpCachingProxy ? u"FtpCachingProxy"_s :
                          u"Undefined"_s,
                          proxy.hostName() )
                    .arg( proxy.port() )
                    .arg( proxy.user(),
                          proxy.password().isEmpty() ? u"not set"_s : u"set"_s ), 4 );

  mFallbackProxy = proxy;
  mExcludedURLs = excludes;
  // remove empty records from excludes list -- these would otherwise match ANY url, so the proxy would always be skipped!
  mExcludedURLs.erase( std::remove_if( mExcludedURLs.begin(), mExcludedURLs.end(), // clazy:exclude=detaching-member
                                       []( const QString & url )
  {
    return url.trimmed().isEmpty();
  } ), mExcludedURLs.end() ); // clazy:exclude=detaching-member

  mNoProxyURLs = noProxyURLs;
  mNoProxyURLs.erase( std::remove_if( mNoProxyURLs.begin(), mNoProxyURLs.end(), // clazy:exclude=detaching-member
                                      []( const QString & url )
  {
    return url.trimmed().isEmpty();
  } ), mNoProxyURLs.end() ); // clazy:exclude=detaching-member
}

QNetworkReply *QgsNetworkAccessManager::createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData )
{
  const QgsSettings s;

  QNetworkRequest *pReq( const_cast< QNetworkRequest * >( &req ) ); // hack user agent

  QString userAgent = s.value( u"/qgis/networkAndProxy/userAgent"_s, "Mozilla/5.0" ).toString();
  if ( !userAgent.isEmpty() )
    userAgent += ' ';
  userAgent += u"QGIS/%1/%2"_s.arg( Qgis::versionInt() ).arg( QSysInfo::prettyProductName() );
  pReq->setRawHeader( "User-Agent", userAgent.toLatin1() );

#ifndef QT_NO_SSL
  const bool ishttps = pReq->url().scheme().compare( "https"_L1, Qt::CaseInsensitive ) == 0;
  if ( ishttps && !QgsApplication::authManager()->isDisabled() )
  {
    QgsDebugMsgLevel( u"Adding trusted CA certs to request"_s, 3 );
    QSslConfiguration sslconfig( pReq->sslConfiguration() );
    // Merge trusted CAs with any additional CAs added by the authentication methods
    sslconfig.setCaCertificates( QgsAuthCertUtils::casMerge( QgsApplication::authManager()->trustedCaCertsCache(), sslconfig.caCertificates( ) ) );
    // check for SSL cert custom config
    const QString hostport( u"%1:%2"_s
                            .arg( pReq->url().host().trimmed() )
                            .arg( pReq->url().port() != -1 ? pReq->url().port() : 443 ) );
    const QgsAuthConfigSslServer servconfig = QgsApplication::authManager()->sslCertCustomConfigByHost( hostport.trimmed() );
    if ( !servconfig.isNull() )
    {
      QgsDebugMsgLevel( u"Adding SSL custom config to request for %1"_s.arg( hostport ), 2 );
      sslconfig.setProtocol( servconfig.sslProtocol() );
      sslconfig.setPeerVerifyMode( servconfig.sslPeerVerifyMode() );
      sslconfig.setPeerVerifyDepth( servconfig.sslPeerVerifyDepth() );
    }

    pReq->setSslConfiguration( sslconfig );
  }
#endif

  if ( sMainNAM->mCacheDisabled )
  {
    // if caching is disabled then we override whatever the request actually has set!
    pReq->setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
    pReq->setAttribute( QNetworkRequest::CacheSaveControlAttribute, false );
  }

  for ( const auto &preprocessor :  sCustomPreprocessors )
  {
    preprocessor.second( pReq );
  }

  static QAtomicInt sRequestId = 0;
  const int requestId = ++sRequestId;
  QByteArray content;
  if ( QBuffer *buffer = qobject_cast<QBuffer *>( outgoingData ) )
  {
    content = buffer->buffer();
  }

  for ( const auto &preprocessor :  sCustomAdvancedPreprocessors )
  {
    int intOp = static_cast< int >( op );
    preprocessor.second( pReq, intOp, &content );
    op = static_cast< QNetworkAccessManager::Operation >( intOp );
  }

  emit requestAboutToBeCreated( QgsNetworkRequestParameters( op, req, requestId, content ) );
  Q_NOWARN_DEPRECATED_PUSH
  emit requestAboutToBeCreated( op, req, outgoingData );
  Q_NOWARN_DEPRECATED_POP
  QNetworkReply *reply = QNetworkAccessManager::createRequest( op, req, outgoingData );
  reply->setProperty( "requestId", requestId );

  emit requestCreated( QgsNetworkRequestParameters( op, reply->request(), requestId, content ) );
  Q_NOWARN_DEPRECATED_PUSH
  emit requestCreated( reply );
  Q_NOWARN_DEPRECATED_POP

  connect( reply, &QNetworkReply::downloadProgress, this, &QgsNetworkAccessManager::onReplyDownloadProgress );
#ifndef QT_NO_SSL
  connect( reply, &QNetworkReply::sslErrors, this, &QgsNetworkAccessManager::onReplySslErrors );
#endif

  for ( const auto &replyPreprocessor :  sCustomReplyPreprocessors )
  {
    replyPreprocessor.second( req, reply );
  }

  // The timer will call abortRequest slot to abort the connection if needed.
  // The timer is stopped by the finished signal and is restarted on downloadProgress and
  // uploadProgress.
  if ( timeout() )
  {
    QTimer *timer = new QTimer( reply );
    timer->setObjectName( u"timeoutTimer"_s );
    connect( timer, &QTimer::timeout, this, &QgsNetworkAccessManager::abortRequest );
    timer->setSingleShot( true );
    timer->start( timeout() );

    connect( reply, &QNetworkReply::downloadProgress, timer, [timer] { timer->start(); } );
    connect( reply, &QNetworkReply::uploadProgress, timer, [timer] { timer->start(); } );
    connect( reply, &QNetworkReply::finished, timer, &QTimer::stop );
  }
  QgsDebugMsgLevel( u"Created [reply:%1]"_s.arg( reinterpret_cast< qint64 >( reply ), 0, 16 ), 3 );

  return reply;
}

void QgsNetworkAccessManager::abortRequest()
{
  QTimer *timer = qobject_cast<QTimer *>( sender() );
  Q_ASSERT( timer );

  QNetworkReply *reply = qobject_cast<QNetworkReply *>( timer->parent() );
  Q_ASSERT( reply );

  reply->abort();
  QgsDebugMsgLevel( u"Abort [reply:%1] %2"_s.arg( reinterpret_cast< qint64 >( reply ), 0, 16 ).arg( reply->url().toString() ), 3 );
  QgsMessageLog::logMessage( tr( "Network request %1 timed out" ).arg( reply->url().toString() ), tr( "Network" ) );
  // Notify the application
  emit requestTimedOut( QgsNetworkRequestParameters( reply->operation(), reply->request(), getRequestId( reply ) ) );
  emit requestTimedOut( reply );
}

void QgsNetworkAccessManager::onReplyFinished( QNetworkReply *reply )
{
  emit finished( QgsNetworkReplyContent( reply ) );
}

void QgsNetworkAccessManager::onReplyDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  if ( QNetworkReply *reply = qobject_cast< QNetworkReply *>( sender() ) )
  {
    emit downloadProgress( getRequestId( reply ), bytesReceived, bytesTotal );
  }
}

#ifndef QT_NO_SSL
void QgsNetworkAccessManager::onReplySslErrors( const QList<QSslError> &errors )
{
  QNetworkReply *reply = qobject_cast< QNetworkReply *>( sender() );
  Q_ASSERT( reply );
  Q_ASSERT( reply->manager() == this );

  QgsDebugMsgLevel( u"Stopping network reply timeout whilst SSL error is handled"_s, 2 );
  pauseTimeout( reply );

  emit requestEncounteredSslErrors( getRequestId( reply ), errors );

  // acquire semaphore a first time, so we block next acquire until release is called
  mSslErrorHandlerSemaphore.acquire();

  // in main thread this will trigger SSL error handler immediately and return once the errors are handled,
  // while in worker thread the signal will be queued (and return immediately) -- hence the need to lock the thread in the next block
  emit sslErrorsOccurred( reply, errors );
  if ( this != sMainNAM )
  {
    // lock thread and wait till error is handled. If we return from this slot now, then the reply will resume
    // without actually giving the main thread the chance to act on the ssl error and possibly ignore it.
    mSslErrorHandlerSemaphore.acquire();
    mSslErrorHandlerSemaphore.release();
    afterSslErrorHandled( reply );
  }
}

void QgsNetworkAccessManager::afterSslErrorHandled( QNetworkReply *reply )
{
  if ( reply->manager() == this )
  {
    restartTimeout( reply );
    emit sslErrorsHandled( reply );
  }
}

void QgsNetworkAccessManager::afterAuthRequestHandled( QNetworkReply *reply )
{
  if ( reply->manager() == this )
  {
    restartTimeout( reply );
    emit authRequestHandled( reply );
  }
}

void QgsNetworkAccessManager::pauseTimeout( QNetworkReply *reply )
{
  Q_ASSERT( reply->manager() == this );

  QTimer *timer = reply->findChild<QTimer *>( u"timeoutTimer"_s );
  if ( timer && timer->isActive() )
  {
    timer->stop();
  }
}

void QgsNetworkAccessManager::restartTimeout( QNetworkReply *reply )
{
  Q_ASSERT( reply->manager() == this );
  // restart reply timeout
  QTimer *timer = reply->findChild<QTimer *>( u"timeoutTimer"_s );
  if ( timer )
  {
    Q_ASSERT( !timer->isActive() );
    QgsDebugMsgLevel( u"Restarting network reply timeout"_s, 2 );
    timer->setSingleShot( true );
    timer->start( timeout() );
  }
}

int QgsNetworkAccessManager::getRequestId( QNetworkReply *reply )
{
  return reply->property( "requestId" ).toInt();
}

void QgsNetworkAccessManager::handleSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  mSslErrorHandler->handleSslErrors( reply, errors );
  afterSslErrorHandled( reply );
  qobject_cast<QgsNetworkAccessManager *>( reply->manager() )->mSslErrorHandlerSemaphore.release();
}

#endif

void QgsNetworkAccessManager::onAuthRequired( QNetworkReply *reply, QAuthenticator *auth )
{
  Q_ASSERT( reply );
  Q_ASSERT( reply->manager() == this );

  QgsDebugMsgLevel( u"Stopping network reply timeout whilst auth request is handled"_s, 2 );
  pauseTimeout( reply );

  emit requestRequiresAuth( getRequestId( reply ), auth->realm() );

  // acquire semaphore a first time, so we block next acquire until release is called
  mAuthRequestHandlerSemaphore.acquire();

  // in main thread this will trigger auth handler immediately and return once the request is satisfied,
  // while in worker thread the signal will be queued (and return immediately) -- hence the need to lock the thread in the next block
  emit authRequestOccurred( reply, auth );

  if ( this != sMainNAM )
  {
    // lock thread and wait till error is handled. If we return from this slot now, then the reply will resume
    // without actually giving the main thread the chance to act on the ssl error and possibly ignore it.
    mAuthRequestHandlerSemaphore.acquire();
    mAuthRequestHandlerSemaphore.release();
    afterAuthRequestHandled( reply );
  }
}

void QgsNetworkAccessManager::requestAuthOpenBrowser( const QUrl &url ) const
{
  if ( this != sMainNAM )
  {
    sMainNAM->requestAuthOpenBrowser( url );
    connect( sMainNAM, &QgsNetworkAccessManager::authBrowserAborted, this, &QgsNetworkAccessManager::abortAuthBrowser );
    return;
  }
  mAuthHandler->handleAuthRequestOpenBrowser( url );
}

void QgsNetworkAccessManager::requestAuthCloseBrowser() const
{
  if ( this != sMainNAM )
  {
    sMainNAM->requestAuthCloseBrowser();
    disconnect( sMainNAM, &QgsNetworkAccessManager::authBrowserAborted, this, &QgsNetworkAccessManager::abortAuthBrowser );
    return;
  }
  mAuthHandler->handleAuthRequestCloseBrowser();
}

void QgsNetworkAccessManager::abortAuthBrowser()
{
  if ( this != sMainNAM )
  {
    disconnect( sMainNAM, &QgsNetworkAccessManager::authBrowserAborted, this, &QgsNetworkAccessManager::abortAuthBrowser );
  }
  emit authBrowserAborted();
}

void QgsNetworkAccessManager::handleAuthRequest( QNetworkReply *reply, QAuthenticator *auth )
{
  mAuthHandler->handleAuthRequest( reply, auth );

  emit requestAuthDetailsAdded( getRequestId( reply ), auth->realm(), auth->user(), auth->password() );

  afterAuthRequestHandled( reply );
  qobject_cast<QgsNetworkAccessManager *>( reply->manager() )->mAuthRequestHandlerSemaphore.release();
}

QString QgsNetworkAccessManager::cacheLoadControlName( QNetworkRequest::CacheLoadControl control )
{
  switch ( control )
  {
    case QNetworkRequest::AlwaysNetwork:
      return u"AlwaysNetwork"_s;
    case QNetworkRequest::PreferNetwork:
      return u"PreferNetwork"_s;
    case QNetworkRequest::PreferCache:
      return u"PreferCache"_s;
    case QNetworkRequest::AlwaysCache:
      return u"AlwaysCache"_s;
  }
  return u"PreferNetwork"_s;
}

QNetworkRequest::CacheLoadControl QgsNetworkAccessManager::cacheLoadControlFromName( const QString &name )
{
  if ( name == "AlwaysNetwork"_L1 )
  {
    return QNetworkRequest::AlwaysNetwork;
  }
  else if ( name == "PreferNetwork"_L1 )
  {
    return QNetworkRequest::PreferNetwork;
  }
  else if ( name == "PreferCache"_L1 )
  {
    return QNetworkRequest::PreferCache;
  }
  else if ( name == "AlwaysCache"_L1 )
  {
    return QNetworkRequest::AlwaysCache;
  }
  return QNetworkRequest::PreferNetwork;
}

void QgsNetworkAccessManager::setupDefaultProxyAndCache( Qt::ConnectionType connectionType )
{
  mInitialized = true;
  mUseSystemProxy = false;

  Q_ASSERT( sMainNAM );

  if ( sMainNAM != this )
  {
    connect( this, &QNetworkAccessManager::proxyAuthenticationRequired,
             sMainNAM, &QNetworkAccessManager::proxyAuthenticationRequired,
             connectionType );

    connect( this, qOverload< QNetworkReply *>( &QgsNetworkAccessManager::requestTimedOut ),
             sMainNAM, qOverload< QNetworkReply *>( &QgsNetworkAccessManager::requestTimedOut ) );

    connect( this, qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestTimedOut ),
             sMainNAM, qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestTimedOut ) );

    connect( this, qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ),
             sMainNAM, qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ) );

    connect( this, qOverload< const QgsNetworkRequestParameters & >( &QgsNetworkAccessManager::requestCreated ),
             sMainNAM, qOverload< const QgsNetworkRequestParameters & >( &QgsNetworkAccessManager::requestCreated ) );

    connect( this, qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ),
             sMainNAM, qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ) );

    connect( this, &QgsNetworkAccessManager::downloadProgress, sMainNAM, &QgsNetworkAccessManager::downloadProgress );

#ifndef QT_NO_SSL
    connect( this, &QNetworkAccessManager::sslErrors,
             sMainNAM, &QNetworkAccessManager::sslErrors,
             connectionType );

    connect( this, &QgsNetworkAccessManager::requestEncounteredSslErrors, sMainNAM, &QgsNetworkAccessManager::requestEncounteredSslErrors );
#endif

    connect( this, &QgsNetworkAccessManager::requestRequiresAuth, sMainNAM, &QgsNetworkAccessManager::requestRequiresAuth );
    connect( sMainNAM, &QgsNetworkAccessManager::cookiesChanged, this, &QgsNetworkAccessManager::syncCookies );
    connect( this, &QgsNetworkAccessManager::cookiesChanged, sMainNAM, &QgsNetworkAccessManager::syncCookies );
  }
  else
  {
#ifndef QT_NO_SSL
    if ( !mSslErrorHandler )
      setSslErrorHandler( std::make_unique< QgsSslErrorHandler >() );
#endif
    if ( !mAuthHandler )
      setAuthHandler( std::make_unique< QgsNetworkAuthenticationHandler>() );
  }
#ifndef QT_NO_SSL
  connect( this, &QgsNetworkAccessManager::sslErrorsOccurred, sMainNAM, &QgsNetworkAccessManager::handleSslErrors );
#endif
  connect( this, &QNetworkAccessManager::authenticationRequired, this, &QgsNetworkAccessManager::onAuthRequired );
  connect( this, &QgsNetworkAccessManager::authRequestOccurred, sMainNAM, &QgsNetworkAccessManager::handleAuthRequest );

  connect( this, &QNetworkAccessManager::finished, this, &QgsNetworkAccessManager::onReplyFinished );

  // check if proxy is enabled
  const QgsSettings settings;
  QNetworkProxy proxy;
  QStringList excludes;
  QStringList noProxyURLs;

  const bool proxyEnabled = settings.value( u"proxy/proxyEnabled"_s, false ).toBool();
  if ( proxyEnabled )
  {
    // This settings is keep for retrocompatibility, the returned proxy for these URL is the default one,
    // meaning the system one
    excludes = settings.value( u"proxy/proxyExcludedUrls"_s, QStringList() ).toStringList();

    noProxyURLs = settings.value( u"proxy/noProxyUrls"_s, QStringList() ).toStringList();

    //read type, host, port, user, passw from settings
    const QString proxyHost = settings.value( u"proxy/proxyHost"_s, "" ).toString();
    const int proxyPort = settings.value( u"proxy/proxyPort"_s, "" ).toString().toInt();

    const QString proxyUser = settings.value( u"proxy/proxyUser"_s, "" ).toString();
    const QString proxyPassword = settings.value( u"proxy/proxyPassword"_s, "" ).toString();

    const QString proxyTypeString = settings.value( u"proxy/proxyType"_s, "" ).toString();

    if ( proxyTypeString == "DefaultProxy"_L1 )
    {
      mUseSystemProxy = true;
      QNetworkProxyFactory::setUseSystemConfiguration( true );
      QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery();
      if ( !proxies.isEmpty() )
      {
        proxy = proxies.first();
      }
      QgsDebugMsgLevel( u"setting default proxy"_s, 4 );
    }
    else
    {
      QNetworkProxy::ProxyType proxyType = QNetworkProxy::DefaultProxy;
      if ( proxyTypeString == "Socks5Proxy"_L1 )
      {
        proxyType = QNetworkProxy::Socks5Proxy;
      }
      else if ( proxyTypeString == "HttpProxy"_L1 )
      {
        proxyType = QNetworkProxy::HttpProxy;
      }
      else if ( proxyTypeString == "HttpCachingProxy"_L1 )
      {
        proxyType = QNetworkProxy::HttpCachingProxy;
      }
      else if ( proxyTypeString == "FtpCachingProxy"_L1 )
      {
        proxyType = QNetworkProxy::FtpCachingProxy;
      }
      QgsDebugMsgLevel( u"setting proxy %1 %2:%3 %4/%5"_s
                        .arg( proxyType )
                        .arg( proxyHost ).arg( proxyPort )
                        .arg( proxyUser, proxyPassword ), 2
                      );
      proxy = QNetworkProxy( proxyType, proxyHost, proxyPort, proxyUser, proxyPassword );
    }
    // Setup network proxy authentication configuration
    const QString authcfg = settings.value( u"proxy/authcfg"_s, "" ).toString();
    if ( !authcfg.isEmpty( ) )
    {
      QgsDebugMsgLevel( u"setting proxy from stored authentication configuration %1"_s.arg( authcfg ), 2 );
      // Never crash! Never.
      if ( QgsAuthManager *authManager = QgsApplication::authManager() )
        authManager->updateNetworkProxy( proxy, authcfg );
    }
  }

  setFallbackProxyAndExcludes( proxy, excludes, noProxyURLs );

  QgsNetworkDiskCache *newcache = qobject_cast<QgsNetworkDiskCache *>( cache() );
  if ( !newcache )
    newcache = new QgsNetworkDiskCache( this );

  QString cacheDirectory = QgsSettingsRegistryCore::settingsNetworkCacheDirectory->value();
  if ( cacheDirectory.isEmpty() )
    cacheDirectory = QStandardPaths::writableLocation( QStandardPaths::CacheLocation );
  newcache->setCacheDirectory( cacheDirectory );
  qint64 cacheSize = QgsSettingsRegistryCore::settingsNetworkCacheSize->value();
  newcache->setMaximumCacheSize( cacheSize );

  QgsDebugMsgLevel( u"cacheDirectory: %1"_s.arg( newcache->cacheDirectory() ), 4 );
  QgsDebugMsgLevel( u"maximumCacheSize: %1"_s.arg( newcache->maximumCacheSize() ), 4 );

  if ( cache() != newcache )
    setCache( newcache );

  if ( this != sMainNAM )
  {
    static_cast<QgsNetworkCookieJar *>( cookieJar() )->setAllCookies( static_cast<QgsNetworkCookieJar *>( sMainNAM->cookieJar() )->allCookies() );
  }
}

void QgsNetworkAccessManager::syncCookies( const QList<QNetworkCookie> &cookies )
{
  if ( sender() != this )
  {
    static_cast<QgsNetworkCookieJar *>( cookieJar() )->setAllCookies( cookies );
    if ( this == sMainNAM )
    {
      emit cookiesChanged( cookies );
    }
  }
}

int QgsNetworkAccessManager::timeout()
{
  return settingsNetworkTimeout->value();
}

void QgsNetworkAccessManager::setTimeout( const int time )
{
  settingsNetworkTimeout->setValue( time );
}

QgsNetworkReplyContent QgsNetworkAccessManager::blockingGet( QNetworkRequest &request, const QString &authCfg, bool forceRefresh, QgsFeedback *feedback, Qgis::NetworkRequestFlags flags )
{
  QgsBlockingNetworkRequest br( flags );
  br.setAuthCfg( authCfg );
  br.get( request, forceRefresh, feedback );
  return br.reply();
}

QgsNetworkReplyContent QgsNetworkAccessManager::blockingPost( QNetworkRequest &request, const QByteArray &data, const QString &authCfg, bool forceRefresh, QgsFeedback *feedback, Qgis::NetworkRequestFlags flags )
{
  QgsBlockingNetworkRequest br( flags );
  br.setAuthCfg( authCfg );
  ( void )br.post( request, data, forceRefresh, feedback );
  return br.reply();
}

QString QgsNetworkAccessManager::setRequestPreprocessor( const std::function<void ( QNetworkRequest * )> &processor )
{
  QString id = QUuid::createUuid().toString();
  sCustomPreprocessors.emplace_back( std::make_pair( id, processor ) );
  return id;
}

bool QgsNetworkAccessManager::removeRequestPreprocessor( const QString &id )
{
  const size_t prevCount = sCustomPreprocessors.size();
  sCustomPreprocessors.erase( std::remove_if( sCustomPreprocessors.begin(), sCustomPreprocessors.end(), [id]( std::pair< QString, std::function< void( QNetworkRequest * ) > > &a )
  {
    return a.first == id;
  } ), sCustomPreprocessors.end() );
  return prevCount != sCustomPreprocessors.size();
}

bool QgsNetworkAccessManager::removeAdvancedRequestPreprocessor( const QString &id )
{
  const size_t prevCount = sCustomAdvancedPreprocessors.size();
  sCustomAdvancedPreprocessors.erase( std::remove_if( sCustomAdvancedPreprocessors.begin(), sCustomAdvancedPreprocessors.end(), [id]( std::pair< QString, std::function< void( QNetworkRequest *, int &, QByteArray * ) > > &a )
  {
    return a.first == id;
  } ), sCustomAdvancedPreprocessors.end() );
  return prevCount != sCustomAdvancedPreprocessors.size();
}

QString QgsNetworkAccessManager::setAdvancedRequestPreprocessor( const std::function<void ( QNetworkRequest *, int &, QByteArray * )> &processor )
{
  QString id = QUuid::createUuid().toString();
  sCustomAdvancedPreprocessors.emplace_back( std::make_pair( id, processor ) );
  return id;
}

QString QgsNetworkAccessManager::setReplyPreprocessor( const std::function<void ( const QNetworkRequest &, QNetworkReply * )> &processor )
{
  QString id = QUuid::createUuid().toString();
  sCustomReplyPreprocessors.emplace_back( std::make_pair( id, processor ) );
  return id;
}

bool QgsNetworkAccessManager::removeReplyPreprocessor( const QString &id )
{
  const size_t prevCount = sCustomReplyPreprocessors.size();
  sCustomReplyPreprocessors.erase( std::remove_if( sCustomReplyPreprocessors.begin(), sCustomReplyPreprocessors.end(), [id]( std::pair< QString, std::function< void( const QNetworkRequest &, QNetworkReply * ) > > &a )
  {
    return a.first == id;
  } ), sCustomReplyPreprocessors.end() );
  return prevCount != sCustomReplyPreprocessors.size();
}

void QgsNetworkAccessManager::preprocessRequest( QNetworkRequest *req ) const
{
  for ( const auto &preprocessor :  sCustomPreprocessors )
  {
    preprocessor.second( req );
  }
}


//
// QgsNetworkRequestParameters
//

QgsNetworkRequestParameters::QgsNetworkRequestParameters( QNetworkAccessManager::Operation operation, const QNetworkRequest &request, int requestId, const QByteArray &content )
  : mOperation( operation )
  , mRequest( request )
  , mOriginatingThreadId( u"0x%2"_s.arg( reinterpret_cast<quintptr>( QThread::currentThread() ), 2 * QT_POINTER_SIZE, 16, '0'_L1 ) )
  , mRequestId( requestId )
  , mContent( content )
  , mInitiatorClass( request.attribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorClass ) ).toString() )
  , mInitiatorRequestId( request.attribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorRequestId ) ) )
{
}


//
// QgsSslErrorHandler
//

void QgsSslErrorHandler::handleSslErrors( QNetworkReply *reply, const QList<QSslError> & )
{
  Q_UNUSED( reply )
  QgsDebugError( u"SSL errors occurred accessing URL:\n%1"_s.arg( reply->request().url().toString() ) );
}

//
// QgsNetworkAuthenticationHandler
//

void QgsNetworkAuthenticationHandler::handleAuthRequest( QNetworkReply *reply, QAuthenticator * )
{
  Q_UNUSED( reply )
  QgsDebugError( u"Network reply required authentication, but no handler was in place to provide this authentication request while accessing the URL:\n%1"_s.arg( reply->request().url().toString() ) );
}

void QgsNetworkAuthenticationHandler::handleAuthRequestOpenBrowser( const QUrl &url )
{
  Q_UNUSED( url )
  QgsDebugError( u"Network authentication required external browser to open URL %1, but no handler was in place"_s.arg( url.toString() ) );
}

void QgsNetworkAuthenticationHandler::handleAuthRequestCloseBrowser()
{
  QgsDebugError( u"Network authentication required external browser closed, but no handler was in place"_s );
}

// For QgsNetworkCookieJar
#include "qgsnetworkaccessmanager.moc"
