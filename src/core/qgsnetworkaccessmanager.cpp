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

#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgis.h"
#include "qgssettings.h"
#include "qgsnetworkdiskcache.h"
#include "qgsauthmanager.h"
#include "qgsnetworkreply.h"
#include "qgsblockingnetworkrequest.h"

#include <QUrl>
#include <QTimer>
#include <QBuffer>
#include <QNetworkReply>
#include <QThreadStorage>
#include <QAuthenticator>

#ifndef QT_NO_SSL
#include <QSslConfiguration>
#endif

#include "qgsnetworkdiskcache.h"
#include "qgsauthmanager.h"

QgsNetworkAccessManager *QgsNetworkAccessManager::sMainNAM = nullptr;

/// @cond PRIVATE
class QgsNetworkProxyFactory : public QNetworkProxyFactory
{
  public:
    QgsNetworkProxyFactory() = default;

    QList<QNetworkProxy> queryProxy( const QNetworkProxyQuery &query = QNetworkProxyQuery() ) override
    {
      QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

      // iterate proxies factories and take first non empty list
      Q_FOREACH ( QNetworkProxyFactory *f, nam->proxyFactories() )
      {
        QList<QNetworkProxy> systemproxies = f->systemProxyForQuery( query );
        if ( !systemproxies.isEmpty() )
          return systemproxies;

        QList<QNetworkProxy> proxies = f->queryProxy( query );
        if ( !proxies.isEmpty() )
          return proxies;
      }

      // no proxies from the proxy factory list check for excludes
      if ( query.queryType() != QNetworkProxyQuery::UrlRequest )
        return QList<QNetworkProxy>() << nam->fallbackProxy();

      QString url = query.url().toString();

      Q_FOREACH ( const QString &exclude, nam->excludeList() )
      {
        if ( !exclude.trimmed().isEmpty() && url.startsWith( exclude ) )
        {
          QgsDebugMsgLevel( QStringLiteral( "using default proxy for %1 [exclude %2]" ).arg( url, exclude ), 4 );
          return QList<QNetworkProxy>() << QNetworkProxy();
        }
      }

      if ( nam->useSystemProxy() )
      {
        QgsDebugMsgLevel( QStringLiteral( "requesting system proxy for query %1" ).arg( url ), 4 );
        QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery( query );
        if ( !proxies.isEmpty() )
        {
          QgsDebugMsgLevel( QStringLiteral( "using system proxy %1:%2 for query" )
                            .arg( proxies.first().hostName() ).arg( proxies.first().port() ), 4 );
          return proxies;
        }
      }

      QgsDebugMsgLevel( QStringLiteral( "using fallback proxy for %1" ).arg( url ), 4 );
      return QList<QNetworkProxy>() << nam->fallbackProxy();
    }
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
    nam->setupDefaultProxyAndCache( connectionType );

  return nam;
}

QgsNetworkAccessManager::QgsNetworkAccessManager( QObject *parent )
  : QNetworkAccessManager( parent )
{
  setProxyFactory( new QgsNetworkProxyFactory() );
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

const QNetworkProxy &QgsNetworkAccessManager::fallbackProxy() const
{
  return mFallbackProxy;
}

void QgsNetworkAccessManager::setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes )
{
  QgsDebugMsgLevel( QStringLiteral( "proxy settings: (type:%1 host: %2:%3, user:%4, password:%5" )
                    .arg( proxy.type() == QNetworkProxy::DefaultProxy ? QStringLiteral( "DefaultProxy" ) :
                          proxy.type() == QNetworkProxy::Socks5Proxy ? QStringLiteral( "Socks5Proxy" ) :
                          proxy.type() == QNetworkProxy::NoProxy ? QStringLiteral( "NoProxy" ) :
                          proxy.type() == QNetworkProxy::HttpProxy ? QStringLiteral( "HttpProxy" ) :
                          proxy.type() == QNetworkProxy::HttpCachingProxy ? QStringLiteral( "HttpCachingProxy" ) :
                          proxy.type() == QNetworkProxy::FtpCachingProxy ? QStringLiteral( "FtpCachingProxy" ) :
                          QStringLiteral( "Undefined" ),
                          proxy.hostName() )
                    .arg( proxy.port() )
                    .arg( proxy.user(),
                          proxy.password().isEmpty() ? QStringLiteral( "not set" ) : QStringLiteral( "set" ) ), 4 );

  mFallbackProxy = proxy;
  mExcludedURLs = excludes;
  // remove empty records from excludes list -- these would otherwise match ANY url, so the proxy would always be skipped!
  mExcludedURLs.erase( std::remove_if( mExcludedURLs.begin(), mExcludedURLs.end(), // clazy:exclude=detaching-member
                                       []( const QString & url )
  {
    return url.trimmed().isEmpty();
  } ), mExcludedURLs.end() ); // clazy:exclude=detaching-member

}

QNetworkReply *QgsNetworkAccessManager::createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData )
{
  QgsSettings s;

  QNetworkRequest *pReq( const_cast< QNetworkRequest * >( &req ) ); // hack user agent

  QString userAgent = s.value( QStringLiteral( "/qgis/networkAndProxy/userAgent" ), "Mozilla/5.0" ).toString();
  if ( !userAgent.isEmpty() )
    userAgent += ' ';
  userAgent += QStringLiteral( "QGIS/%1" ).arg( Qgis::QGIS_VERSION );
  pReq->setRawHeader( "User-Agent", userAgent.toUtf8() );

#ifndef QT_NO_SSL
  bool ishttps = pReq->url().scheme().compare( QLatin1String( "https" ), Qt::CaseInsensitive ) == 0;
  if ( ishttps && !QgsApplication::authManager()->isDisabled() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Adding trusted CA certs to request" ), 3 );
    QSslConfiguration sslconfig( pReq->sslConfiguration() );
    // Merge trusted CAs with any additional CAs added by the authentication methods
    sslconfig.setCaCertificates( QgsAuthCertUtils::casMerge( QgsApplication::authManager()->trustedCaCertsCache(), sslconfig.caCertificates( ) ) );
    // check for SSL cert custom config
    QString hostport( QStringLiteral( "%1:%2" )
                      .arg( pReq->url().host().trimmed() )
                      .arg( pReq->url().port() != -1 ? pReq->url().port() : 443 ) );
    QgsAuthConfigSslServer servconfig = QgsApplication::authManager()->sslCertCustomConfigByHost( hostport.trimmed() );
    if ( !servconfig.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "Adding SSL custom config to request for %1" ).arg( hostport ) );
      sslconfig.setProtocol( servconfig.sslProtocol() );
      sslconfig.setPeerVerifyMode( servconfig.sslPeerVerifyMode() );
      sslconfig.setPeerVerifyDepth( servconfig.sslPeerVerifyDepth() );
    }

    pReq->setSslConfiguration( sslconfig );
  }
#endif

  static QAtomicInt sRequestId = 0;
  const int requestId = ++sRequestId;
  QByteArray content;
  if ( QBuffer *buffer = qobject_cast<QBuffer *>( outgoingData ) )
  {
    content = buffer->buffer();
  }

  emit requestAboutToBeCreated( QgsNetworkRequestParameters( op, req, requestId, content ) );
  Q_NOWARN_DEPRECATED_PUSH
  emit requestAboutToBeCreated( op, req, outgoingData );
  Q_NOWARN_DEPRECATED_POP
  QNetworkReply *reply = QNetworkAccessManager::createRequest( op, req, outgoingData );
  reply->setProperty( "requestId", requestId );

  Q_NOWARN_DEPRECATED_PUSH
  emit requestCreated( reply );
  Q_NOWARN_DEPRECATED_POP

  connect( reply, &QNetworkReply::downloadProgress, this, &QgsNetworkAccessManager::onReplyDownloadProgress );
#ifndef QT_NO_SSL
  connect( reply, &QNetworkReply::sslErrors, this, &QgsNetworkAccessManager::onReplySslErrors );
#endif

  // The timer will call abortRequest slot to abort the connection if needed.
  // The timer is stopped by the finished signal and is restarted on downloadProgress and
  // uploadProgress.
  QTimer *timer = new QTimer( reply );
  timer->setObjectName( QStringLiteral( "timeoutTimer" ) );
  connect( timer, &QTimer::timeout, this, &QgsNetworkAccessManager::abortRequest );
  timer->setSingleShot( true );
  timer->start( timeout() );

  connect( reply, &QNetworkReply::downloadProgress, timer, [timer] { timer->start(); } );
  connect( reply, &QNetworkReply::uploadProgress, timer, [timer] { timer->start(); } );
  connect( reply, &QNetworkReply::finished, timer, &QTimer::stop );
  QgsDebugMsgLevel( QStringLiteral( "Created [reply:%1]" ).arg( reinterpret_cast< qint64 >( reply ), 0, 16 ), 3 );

  return reply;
}

#ifndef QT_NO_SSL
void QgsNetworkAccessManager::unlockAfterSslErrorHandled()
{
  Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
  mSslErrorWaitCondition.wakeOne();
}
#endif

void QgsNetworkAccessManager::abortRequest()
{
  QTimer *timer = qobject_cast<QTimer *>( sender() );
  Q_ASSERT( timer );

  QNetworkReply *reply = qobject_cast<QNetworkReply *>( timer->parent() );
  Q_ASSERT( reply );

  reply->abort();
  QgsDebugMsgLevel( QStringLiteral( "Abort [reply:%1] %2" ).arg( reinterpret_cast< qint64 >( reply ), 0, 16 ).arg( reply->url().toString() ), 3 );
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

  QgsDebugMsg( QStringLiteral( "Stopping network reply timeout whilst SSL error is handled" ) );
  pauseTimeout( reply );

  emit requestEncounteredSslErrors( getRequestId( reply ), errors );

  // in main thread this will trigger SSL error handler immediately and return once the errors are handled,
  // while in worker thread the signal will be queued (and return immediately) -- hence the need to lock the thread in the next block
  emit sslErrorsOccurred( reply, errors );
  if ( this != sMainNAM )
  {
    // lock thread and wait till error is handled. If we return from this slot now, then the reply will resume
    // without actually giving the main thread the chance to act on the ssl error and possibly ignore it.
    mSslErrorHandlerMutex.lock();
    mSslErrorWaitCondition.wait( &mSslErrorHandlerMutex );
    mSslErrorHandlerMutex.unlock();
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
  else if ( this == sMainNAM )
  {
    // notify other threads to allow them to handle the reply
    qobject_cast< QgsNetworkAccessManager *>( reply->manager() )->unlockAfterSslErrorHandled(); // safe to call directly - the other thread will be stuck waiting for us
  }
}

void QgsNetworkAccessManager::unlockAfterAuthRequestHandled()
{
  Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
  mAuthRequestWaitCondition.wakeOne();
}

void QgsNetworkAccessManager::afterAuthRequestHandled( QNetworkReply *reply )
{
  if ( reply->manager() == this )
  {
    restartTimeout( reply );
    emit authRequestHandled( reply );
  }
  else if ( this == sMainNAM )
  {
    // notify other threads to allow them to handle the reply
    qobject_cast< QgsNetworkAccessManager *>( reply->manager() )->unlockAfterAuthRequestHandled(); // safe to call directly - the other thread will be stuck waiting for us
  }
}

void QgsNetworkAccessManager::pauseTimeout( QNetworkReply *reply )
{
  Q_ASSERT( reply->manager() == this );

  QTimer *timer = reply->findChild<QTimer *>( QStringLiteral( "timeoutTimer" ) );
  if ( timer && timer->isActive() )
  {
    timer->stop();
  }
}

void QgsNetworkAccessManager::restartTimeout( QNetworkReply *reply )
{
  Q_ASSERT( reply->manager() == this );
  // restart reply timeout
  QTimer *timer = reply->findChild<QTimer *>( QStringLiteral( "timeoutTimer" ) );
  if ( timer )
  {
    Q_ASSERT( !timer->isActive() );
    QgsDebugMsg( QStringLiteral( "Restarting network reply timeout" ) );
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
}

#endif

void QgsNetworkAccessManager::onAuthRequired( QNetworkReply *reply, QAuthenticator *auth )
{
  Q_ASSERT( reply );
  Q_ASSERT( reply->manager() == this );

  QgsDebugMsg( QStringLiteral( "Stopping network reply timeout whilst auth request is handled" ) );
  pauseTimeout( reply );

  emit requestRequiresAuth( getRequestId( reply ), auth->realm() );

  // in main thread this will trigger auth handler immediately and return once the request is satisfied,
  // while in worker thread the signal will be queued (and return immediately) -- hence the need to lock the thread in the next block
  emit authRequestOccurred( reply, auth );
  if ( this != sMainNAM )
  {
    // lock thread and wait till error is handled. If we return from this slot now, then the reply will resume
    // without actually giving the main thread the chance to act on the ssl error and possibly ignore it.
    mAuthRequestHandlerMutex.lock();
    mAuthRequestWaitCondition.wait( &mAuthRequestHandlerMutex );
    mAuthRequestHandlerMutex.unlock();
    afterAuthRequestHandled( reply );
  }
}

void QgsNetworkAccessManager::handleAuthRequest( QNetworkReply *reply, QAuthenticator *auth )
{
  mAuthHandler->handleAuthRequest( reply, auth );

  emit requestAuthDetailsAdded( getRequestId( reply ), auth->realm(), auth->user(), auth->password() );

  afterAuthRequestHandled( reply );
}

QString QgsNetworkAccessManager::cacheLoadControlName( QNetworkRequest::CacheLoadControl control )
{
  switch ( control )
  {
    case QNetworkRequest::AlwaysNetwork:
      return QStringLiteral( "AlwaysNetwork" );
    case QNetworkRequest::PreferNetwork:
      return QStringLiteral( "PreferNetwork" );
    case QNetworkRequest::PreferCache:
      return QStringLiteral( "PreferCache" );
    case QNetworkRequest::AlwaysCache:
      return QStringLiteral( "AlwaysCache" );
  }
  return QStringLiteral( "PreferNetwork" );
}

QNetworkRequest::CacheLoadControl QgsNetworkAccessManager::cacheLoadControlFromName( const QString &name )
{
  if ( name == QLatin1String( "AlwaysNetwork" ) )
  {
    return QNetworkRequest::AlwaysNetwork;
  }
  else if ( name == QLatin1String( "PreferNetwork" ) )
  {
    return QNetworkRequest::PreferNetwork;
  }
  else if ( name == QLatin1String( "PreferCache" ) )
  {
    return QNetworkRequest::PreferCache;
  }
  else if ( name == QLatin1String( "AlwaysCache" ) )
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

    connect( this, qgis::overload< QNetworkReply *>::of( &QgsNetworkAccessManager::requestTimedOut ),
             sMainNAM, qgis::overload< QNetworkReply *>::of( &QgsNetworkAccessManager::requestTimedOut ) );

    connect( this, qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestTimedOut ),
             sMainNAM, qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestTimedOut ) );

    connect( this, qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestAboutToBeCreated ),
             sMainNAM, qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestAboutToBeCreated ) );

    connect( this, qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ),
             sMainNAM, qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ) );

    connect( this, &QgsNetworkAccessManager::downloadProgress, sMainNAM, &QgsNetworkAccessManager::downloadProgress );

#ifndef QT_NO_SSL
    connect( this, &QNetworkAccessManager::sslErrors,
             sMainNAM, &QNetworkAccessManager::sslErrors,
             connectionType );

    connect( this, &QgsNetworkAccessManager::requestEncounteredSslErrors, sMainNAM, &QgsNetworkAccessManager::requestEncounteredSslErrors );
#endif

    connect( this, &QgsNetworkAccessManager::requestRequiresAuth, sMainNAM, &QgsNetworkAccessManager::requestRequiresAuth );
  }
  else
  {
#ifndef QT_NO_SSL
    setSslErrorHandler( qgis::make_unique< QgsSslErrorHandler >() );
#endif
    setAuthHandler( qgis::make_unique< QgsNetworkAuthenticationHandler>() );
  }
#ifndef QT_NO_SSL
  connect( this, &QgsNetworkAccessManager::sslErrorsOccurred, sMainNAM, &QgsNetworkAccessManager::handleSslErrors );
#endif
  connect( this, &QNetworkAccessManager::authenticationRequired, this, &QgsNetworkAccessManager::onAuthRequired );
  connect( this, &QgsNetworkAccessManager::authRequestOccurred, sMainNAM, &QgsNetworkAccessManager::handleAuthRequest );

  connect( this, &QNetworkAccessManager::finished, this, &QgsNetworkAccessManager::onReplyFinished );

  // check if proxy is enabled
  QgsSettings settings;
  QNetworkProxy proxy;
  QStringList excludes;

  bool proxyEnabled = settings.value( QStringLiteral( "proxy/proxyEnabled" ), false ).toBool();
  if ( proxyEnabled )
  {
    excludes = settings.value( QStringLiteral( "proxy/proxyExcludedUrls" ), QStringList() ).toStringList();

    //read type, host, port, user, passw from settings
    QString proxyHost = settings.value( QStringLiteral( "proxy/proxyHost" ), "" ).toString();
    int proxyPort = settings.value( QStringLiteral( "proxy/proxyPort" ), "" ).toString().toInt();

    QString proxyUser = settings.value( QStringLiteral( "proxy/proxyUser" ), "" ).toString();
    QString proxyPassword = settings.value( QStringLiteral( "proxy/proxyPassword" ), "" ).toString();

    QString proxyTypeString = settings.value( QStringLiteral( "proxy/proxyType" ), "" ).toString();

    if ( proxyTypeString == QLatin1String( "DefaultProxy" ) )
    {
      mUseSystemProxy = true;
      QNetworkProxyFactory::setUseSystemConfiguration( true );
      QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery();
      if ( !proxies.isEmpty() )
      {
        proxy = proxies.first();
      }
      QgsDebugMsgLevel( QStringLiteral( "setting default proxy" ), 4 );
    }
    else
    {
      QNetworkProxy::ProxyType proxyType = QNetworkProxy::DefaultProxy;
      if ( proxyTypeString == QLatin1String( "Socks5Proxy" ) )
      {
        proxyType = QNetworkProxy::Socks5Proxy;
      }
      else if ( proxyTypeString == QLatin1String( "HttpProxy" ) )
      {
        proxyType = QNetworkProxy::HttpProxy;
      }
      else if ( proxyTypeString == QLatin1String( "HttpCachingProxy" ) )
      {
        proxyType = QNetworkProxy::HttpCachingProxy;
      }
      else if ( proxyTypeString == QLatin1String( "FtpCachingProxy" ) )
      {
        proxyType = QNetworkProxy::FtpCachingProxy;
      }
      QgsDebugMsg( QStringLiteral( "setting proxy %1 %2:%3 %4/%5" )
                   .arg( proxyType )
                   .arg( proxyHost ).arg( proxyPort )
                   .arg( proxyUser, proxyPassword )
                 );
      proxy = QNetworkProxy( proxyType, proxyHost, proxyPort, proxyUser, proxyPassword );
    }
    // Setup network proxy authentication configuration
    QString authcfg = settings.value( QStringLiteral( "proxy/authcfg" ), "" ).toString();
    if ( !authcfg.isEmpty( ) )
    {
      QgsDebugMsg( QStringLiteral( "setting proxy from stored authentication configuration %1" ).arg( authcfg ) );
      // Never crash! Never.
      if ( QgsApplication::authManager() )
        QgsApplication::authManager()->updateNetworkProxy( proxy, authcfg );
    }
  }

  setFallbackProxyAndExcludes( proxy, excludes );

  QgsNetworkDiskCache *newcache = qobject_cast<QgsNetworkDiskCache *>( cache() );
  if ( !newcache )
    newcache = new QgsNetworkDiskCache( this );

  QString cacheDirectory = settings.value( QStringLiteral( "cache/directory" ) ).toString();
  if ( cacheDirectory.isEmpty() )
    cacheDirectory = QgsApplication::qgisSettingsDirPath() + "cache";
  qint64 cacheSize = settings.value( QStringLiteral( "cache/size" ), 50 * 1024 * 1024 ).toULongLong();
  newcache->setCacheDirectory( cacheDirectory );
  newcache->setMaximumCacheSize( cacheSize );
  QgsDebugMsgLevel( QStringLiteral( "cacheDirectory: %1" ).arg( newcache->cacheDirectory() ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "maximumCacheSize: %1" ).arg( newcache->maximumCacheSize() ), 4 );

  if ( cache() != newcache )
    setCache( newcache );
}

int QgsNetworkAccessManager::timeout()
{
  return QgsSettings().value( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), 60000 ).toInt();
}

void QgsNetworkAccessManager::setTimeout( const int time )
{
  QgsSettings().setValue( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), time );
}

QgsNetworkReplyContent QgsNetworkAccessManager::blockingGet( QNetworkRequest &request, const QString &authCfg, bool forceRefresh, QgsFeedback *feedback )
{
  QgsBlockingNetworkRequest br;
  br.setAuthCfg( authCfg );
  br.get( request, forceRefresh, feedback );
  return br.reply();
}

QgsNetworkReplyContent QgsNetworkAccessManager::blockingPost( QNetworkRequest &request, const QByteArray &data, const QString &authCfg, bool forceRefresh, QgsFeedback *feedback )
{
  QgsBlockingNetworkRequest br;
  br.setAuthCfg( authCfg );
  br.post( request, data, forceRefresh, feedback );
  return br.reply();
}


//
// QgsNetworkRequestParameters
//

QgsNetworkRequestParameters::QgsNetworkRequestParameters( QNetworkAccessManager::Operation operation, const QNetworkRequest &request, int requestId, const QByteArray &content )
  : mOperation( operation )
  , mRequest( request )
  , mOriginatingThreadId( QStringLiteral( "0x%2" ).arg( reinterpret_cast<quintptr>( QThread::currentThread() ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) ) )
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
  Q_UNUSED( reply );
  QgsDebugMsg( QStringLiteral( "SSL errors occurred accessing URL:\n%1" ).arg( reply->request().url().toString() ) );
}

//
// QgsNetworkAuthenticationHandler
//

void QgsNetworkAuthenticationHandler::handleAuthRequest( QNetworkReply *reply, QAuthenticator * )
{
  Q_UNUSED( reply );
  QgsDebugMsg( QStringLiteral( "Network reply required authentication, but no handler was in place to provide this authentication request while accessing the URL:\n%1" ).arg( reply->request().url().toString() ) );
}
