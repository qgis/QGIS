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

#include <qgsnetworkaccessmanager.h>

#include <qgsapplication.h>
#include <qgsmessagelog.h>
#include <qgslogger.h>
#include <qgis.h>

#include <QUrl>
#include <QSettings>
#include <QTimer>
#include <QNetworkReply>
#include <QThreadStorage>

#ifndef QT_NO_OPENSSL
#include <QSslConfiguration>
#endif

#include "qgsnetworkdiskcache.h"
#include "qgsauthmanager.h"

QgsNetworkAccessManager *QgsNetworkAccessManager::smMainNAM = 0;

/// @cond PRIVATE
class QgsNetworkProxyFactory : public QNetworkProxyFactory
{
  public:
    QgsNetworkProxyFactory() {}
    virtual ~QgsNetworkProxyFactory() {}

    virtual QList<QNetworkProxy> queryProxy( const QNetworkProxyQuery & query = QNetworkProxyQuery() ) override
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

      // no proxies from the proxy factor list check for excludes
      if ( query.queryType() != QNetworkProxyQuery::UrlRequest )
        return QList<QNetworkProxy>() << nam->fallbackProxy();

      QString url = query.url().toString();

      Q_FOREACH ( const QString& exclude, nam->excludeList() )
      {
        if ( url.startsWith( exclude ) )
        {
          QgsDebugMsg( QString( "using default proxy for %1 [exclude %2]" ).arg( url, exclude ) );
          return QList<QNetworkProxy>() << QNetworkProxy();
        }
      }

      if ( nam->useSystemProxy() )
      {
        QgsDebugMsg( QString( "requesting system proxy for query %1" ).arg( url ) );
        QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery( query );
        if ( !proxies.isEmpty() )
        {
          QgsDebugMsg( QString( "using system proxy %1:%2 for query" )
                       .arg( proxies.first().hostName() ).arg( proxies.first().port() ) );
          return proxies;
        }
      }

      QgsDebugMsg( QString( "using fallback proxy for %1" ).arg( url ) );
      return QList<QNetworkProxy>() << nam->fallbackProxy();
    }
};
///@endcond

//
// Static calls to enforce singleton behaviour
//
QgsNetworkAccessManager* QgsNetworkAccessManager::instance()
{
  static QThreadStorage<QgsNetworkAccessManager> sInstances;
  QgsNetworkAccessManager *nam = &sInstances.localData();

  if ( nam->thread() == qApp->thread() )
    smMainNAM = nam;

  if ( !nam->mInitialized )
    nam->setupDefaultProxyAndCache();

  return nam;
}

QgsNetworkAccessManager::QgsNetworkAccessManager( QObject *parent )
    : QNetworkAccessManager( parent )
    , mUseSystemProxy( false )
    , mInitialized( false )
{
  setProxyFactory( new QgsNetworkProxyFactory() );
}

QgsNetworkAccessManager::~QgsNetworkAccessManager()
{
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

const QStringList &QgsNetworkAccessManager::excludeList() const
{
  return mExcludedURLs;
}

const QNetworkProxy &QgsNetworkAccessManager::fallbackProxy() const
{
  return mFallbackProxy;
}

void QgsNetworkAccessManager::setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes )
{
  QgsDebugMsg( QString( "proxy settings: (type:%1 host: %2:%3, user:%4, password:%5" )
               .arg( proxy.type() == QNetworkProxy::DefaultProxy ? "DefaultProxy" :
                     proxy.type() == QNetworkProxy::Socks5Proxy ? "Socks5Proxy" :
                     proxy.type() == QNetworkProxy::NoProxy ? "NoProxy" :
                     proxy.type() == QNetworkProxy::HttpProxy ? "HttpProxy" :
                     proxy.type() == QNetworkProxy::HttpCachingProxy ? "HttpCachingProxy" :
                     proxy.type() == QNetworkProxy::FtpCachingProxy ? "FtpCachingProxy" :
                     "Undefined",
                     proxy.hostName() )
               .arg( proxy.port() )
               .arg( proxy.user(),
                     proxy.password().isEmpty() ? "not set" : "set" ) );

  mFallbackProxy = proxy;
  mExcludedURLs = excludes;
}

QNetworkReply *QgsNetworkAccessManager::createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData )
{
  QSettings s;

  QNetworkRequest *pReq( const_cast< QNetworkRequest * >( &req ) ); // hack user agent

  QString userAgent = s.value( "/qgis/networkAndProxy/userAgent", "Mozilla/5.0" ).toString();
  if ( !userAgent.isEmpty() )
    userAgent += ' ';
  userAgent += QString( "QGIS/%1" ).arg( QGis::QGIS_VERSION );
  pReq->setRawHeader( "User-Agent", userAgent.toUtf8() );

#ifndef QT_NO_OPENSSL
  bool ishttps = pReq->url().scheme().toLower() == "https";
  if ( ishttps && !QgsAuthManager::instance()->isDisabled() )
  {
    QgsDebugMsg( "Adding trusted CA certs to request" );
    QSslConfiguration sslconfig( pReq->sslConfiguration() );
    sslconfig.setCaCertificates( QgsAuthManager::instance()->getTrustedCaCertsCache() );

    // check for SSL cert custom config
    QString hostport( QString( "%1:%2" )
                      .arg( pReq->url().host().trimmed() )
                      .arg( pReq->url().port() != -1 ? pReq->url().port() : 443 ) );
    QgsAuthConfigSslServer servconfig = QgsAuthManager::instance()->getSslCertCustomConfigByHost( hostport.trimmed() );
    if ( !servconfig.isNull() )
    {
      QgsDebugMsg( QString( "Adding SSL custom config to request for %1" ).arg( hostport ) );
      sslconfig.setProtocol( servconfig.sslProtocol() );
      sslconfig.setPeerVerifyMode( servconfig.sslPeerVerifyMode() );
      sslconfig.setPeerVerifyDepth( servconfig.sslPeerVerifyDepth() );
    }

    pReq->setSslConfiguration( sslconfig );
  }
#endif

  emit requestAboutToBeCreated( op, req, outgoingData );
  QNetworkReply *reply = QNetworkAccessManager::createRequest( op, req, outgoingData );

  emit requestCreated( reply );

  // abort request, when network timeout happens
  QTimer *timer = new QTimer( reply );
  timer->setObjectName( "timeoutTimer" );
  connect( timer, SIGNAL( timeout() ), this, SLOT( abortRequest() ) );
  timer->setSingleShot( true );
  timer->start( s.value( "/qgis/networkAndProxy/networkTimeout", "60000" ).toInt() );

  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), timer, SLOT( start() ) );
  connect( reply, SIGNAL( uploadProgress( qint64, qint64 ) ), timer, SLOT( start() ) );

  return reply;
}

void QgsNetworkAccessManager::abortRequest()
{
  QTimer *timer = qobject_cast<QTimer *>( sender() );
  Q_ASSERT( timer );

  QNetworkReply *reply = qobject_cast<QNetworkReply *>( timer->parent() );
  Q_ASSERT( reply );

  QgsDebugMsg( QString( "Abort [reply:%1]" ).arg(( qint64 ) reply, 0, 16 ) );

  QgsMessageLog::logMessage( tr( "Network request %1 timed out" ).arg( reply->url().toString() ), tr( "Network" ) );

  if ( reply->isRunning() )
    reply->close();

  emit requestTimedOut( reply );
}

QString QgsNetworkAccessManager::cacheLoadControlName( QNetworkRequest::CacheLoadControl theControl )
{
  switch ( theControl )
  {
    case QNetworkRequest::AlwaysNetwork:
      return "AlwaysNetwork";
    case QNetworkRequest::PreferNetwork:
      return "PreferNetwork";
    case QNetworkRequest::PreferCache:
      return "PreferCache";
    case QNetworkRequest::AlwaysCache:
      return "AlwaysCache";
    default:
      break;
  }
  return "PreferNetwork";
}

QNetworkRequest::CacheLoadControl QgsNetworkAccessManager::cacheLoadControlFromName( const QString &theName )
{
  if ( theName == "AlwaysNetwork" )
  {
    return QNetworkRequest::AlwaysNetwork;
  }
  else if ( theName == "PreferNetwork" )
  {
    return QNetworkRequest::PreferNetwork;
  }
  else if ( theName == "PreferCache" )
  {
    return QNetworkRequest::PreferCache;
  }
  else if ( theName == "AlwaysCache" )
  {
    return QNetworkRequest::AlwaysCache;
  }
  return QNetworkRequest::PreferNetwork;
}

void QgsNetworkAccessManager::setupDefaultProxyAndCache()
{
  mInitialized = true;
  mUseSystemProxy = false;

  Q_ASSERT( smMainNAM );

  if ( smMainNAM != this )
  {
    connect( this, SIGNAL( authenticationRequired( QNetworkReply *, QAuthenticator * ) ),
             smMainNAM, SIGNAL( authenticationRequired( QNetworkReply *, QAuthenticator * ) ),
             Qt::BlockingQueuedConnection );

    connect( this, SIGNAL( proxyAuthenticationRequired( const QNetworkProxy &, QAuthenticator * ) ),
             smMainNAM, SIGNAL( proxyAuthenticationRequired( const QNetworkProxy &, QAuthenticator * ) ),
             Qt::BlockingQueuedConnection );

    connect( this, SIGNAL( requestTimedOut( QNetworkReply* ) ),
             smMainNAM, SIGNAL( requestTimedOut( QNetworkReply* ) ) );

#ifndef QT_NO_OPENSSL
    connect( this, SIGNAL( sslErrors( QNetworkReply *, const QList<QSslError> & ) ),
             smMainNAM, SIGNAL( sslErrors( QNetworkReply *, const QList<QSslError> & ) ),
             Qt::BlockingQueuedConnection );
#endif
  }

  // check if proxy is enabled
  QSettings settings;
  QNetworkProxy proxy;
  QStringList excludes;

  bool proxyEnabled = settings.value( "proxy/proxyEnabled", false ).toBool();
  if ( proxyEnabled )
  {
    excludes = settings.value( "proxy/proxyExcludedUrls", "" ).toString().split( '|', QString::SkipEmptyParts );

    //read type, host, port, user, passw from settings
    QString proxyHost = settings.value( "proxy/proxyHost", "" ).toString();
    int proxyPort = settings.value( "proxy/proxyPort", "" ).toString().toInt();
    QString proxyUser = settings.value( "proxy/proxyUser", "" ).toString();
    QString proxyPassword = settings.value( "proxy/proxyPassword", "" ).toString();

    QString proxyTypeString = settings.value( "proxy/proxyType", "" ).toString();

    if ( proxyTypeString == "DefaultProxy" )
    {
      mUseSystemProxy = true;
      QNetworkProxyFactory::setUseSystemConfiguration( true );
      QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery();
      if ( !proxies.isEmpty() )
      {
        proxy = proxies.first();
      }
      QgsDebugMsg( "setting default proxy" );
    }
    else
    {
      QNetworkProxy::ProxyType proxyType = QNetworkProxy::DefaultProxy;
      if ( proxyTypeString == "Socks5Proxy" )
      {
        proxyType = QNetworkProxy::Socks5Proxy;
      }
      else if ( proxyTypeString == "HttpProxy" )
      {
        proxyType = QNetworkProxy::HttpProxy;
      }
      else if ( proxyTypeString == "HttpCachingProxy" )
      {
        proxyType = QNetworkProxy::HttpCachingProxy;
      }
      else if ( proxyTypeString == "FtpCachingProxy" )
      {
        proxyType = QNetworkProxy::FtpCachingProxy;
      }
      QgsDebugMsg( QString( "setting proxy %1 %2:%3 %4/%5" )
                   .arg( proxyType )
                   .arg( proxyHost ).arg( proxyPort )
                   .arg( proxyUser, proxyPassword )
                 );
      proxy = QNetworkProxy( proxyType, proxyHost, proxyPort, proxyUser, proxyPassword );
    }
  }

  setFallbackProxyAndExcludes( proxy, excludes );

  QgsNetworkDiskCache *newcache = qobject_cast<QgsNetworkDiskCache*>( cache() );
  if ( !newcache )
    newcache = new QgsNetworkDiskCache( this );

  QString cacheDirectory = settings.value( "cache/directory" ).toString();
  if ( cacheDirectory.isEmpty() )
    cacheDirectory = QgsApplication::qgisSettingsDirPath() + "cache";
  qint64 cacheSize = settings.value( "cache/size", 50 * 1024 * 1024 ).toULongLong();
  newcache->setCacheDirectory( cacheDirectory );
  newcache->setMaximumCacheSize( cacheSize );
  QgsDebugMsg( QString( "cacheDirectory: %1" ).arg( newcache->cacheDirectory() ) );
  QgsDebugMsg( QString( "maximumCacheSize: %1" ).arg( newcache->maximumCacheSize() ) );

  if ( cache() != newcache )
    setCache( newcache );
}

void QgsNetworkAccessManager::sendGet( const QNetworkRequest & request )
{
  QNetworkReply * reply = get( request );
  emit requestSent( reply, QObject::sender() );
}

void QgsNetworkAccessManager::deleteReply( QNetworkReply * reply )
{
  if ( !reply )
  {
    return;
  }
  reply->abort();
  reply->deleteLater();
}
