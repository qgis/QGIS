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
#include <QNetworkDiskCache>

class QgsNetworkProxyFactory : public QNetworkProxyFactory
{
  public:
    QgsNetworkProxyFactory() {}
    virtual ~QgsNetworkProxyFactory() {}

    virtual QList<QNetworkProxy> queryProxy( const QNetworkProxyQuery & query = QNetworkProxyQuery() ) override
    {
      QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

      // iterate proxies factories and take first non empty list
      foreach ( QNetworkProxyFactory *f, nam->proxyFactories() )
      {
        QList<QNetworkProxy> systemproxies = f->systemProxyForQuery( query );
        if ( systemproxies.size() > 0 )
          return systemproxies;

        QList<QNetworkProxy> proxies = f->queryProxy( query );
        if ( proxies.size() > 0 )
          return proxies;
      }

      // no proxies from the proxy factor list check for excludes
      if ( query.queryType() != QNetworkProxyQuery::UrlRequest )
        return QList<QNetworkProxy>() << nam->fallbackProxy();

      QString url = query.url().toString();

      foreach ( QString exclude, nam->excludeList() )
      {
        if ( url.startsWith( exclude ) )
        {
          QgsDebugMsg( QString( "using default proxy for %1 [exclude %2]" ).arg( url ).arg( exclude ) );
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

//
// Static calls to enforce singleton behaviour
//
QgsNetworkAccessManager* QgsNetworkAccessManager::instance()
{
  static QgsNetworkAccessManager* sInstance( new QgsNetworkAccessManager( QApplication::instance() ) );
  return sInstance;
}

QgsNetworkAccessManager::QgsNetworkAccessManager( QObject *parent )
    : QNetworkAccessManager( parent )
    , mUseSystemProxy( false )
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
                     "Undefined" )
               .arg( proxy.hostName() )
               .arg( proxy.port() )
               .arg( proxy.user() )
               .arg( proxy.password().isEmpty() ? "not set" : "set" ) );

  mFallbackProxy = proxy;
  mExcludedURLs = excludes;
}

QNetworkReply *QgsNetworkAccessManager::createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData )
{
  QSettings s;

  QNetworkRequest *pReq(( QNetworkRequest * ) &req ); // hack user agent

  QString userAgent = s.value( "/qgis/networkAndProxy/userAgent", "Mozilla/5.0" ).toString();
  if ( !userAgent.isEmpty() )
    userAgent += " ";
  userAgent += QString( "QGIS/%1" ).arg( QGis::QGIS_VERSION );
  pReq->setRawHeader( "User-Agent", userAgent.toUtf8() );

  emit requestAboutToBeCreated( op, req, outgoingData );
  QNetworkReply *reply = QNetworkAccessManager::createRequest( op, req, outgoingData );

  emit requestCreated( reply );

  // abort request, when network timeout happens
  QTimer *timer = new QTimer( reply );
  connect( timer, SIGNAL( timeout() ), this, SLOT( abortRequest() ) );
  timer->setSingleShot( true );
  timer->start( s.value( "/qgis/networkAndProxy/networkTimeout", "20000" ).toInt() );

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
      break;
    case QNetworkRequest::PreferNetwork:
      return "PreferNetwork";
      break;
    case QNetworkRequest::PreferCache:
      return "PreferCache";
      break;
    case QNetworkRequest::AlwaysCache:
      return "AlwaysCache";
      break;
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
  QNetworkProxy proxy;
  QStringList excludes;

  QSettings settings;

  mUseSystemProxy = false;

  if ( this != instance() )
  {
    Qt::ConnectionType connectionType = thread() == instance()->thread() ? Qt::AutoConnection : Qt::BlockingQueuedConnection;

    connect( this, SIGNAL( authenticationRequired( QNetworkReply *, QAuthenticator * ) ),
             instance(), SIGNAL( authenticationRequired( QNetworkReply *, QAuthenticator * ) ),
             connectionType );

    connect( this, SIGNAL( proxyAuthenticationRequired( const QNetworkProxy &, QAuthenticator * ) ),
             instance(), SIGNAL( proxyAuthenticationRequired( const QNetworkProxy &, QAuthenticator * ) ),
             connectionType );

    connect( this, SIGNAL( requestTimedOut( QNetworkReply* ) ),
             instance(), SIGNAL( requestTimedOut( QNetworkReply* ) ) );

#ifndef QT_NO_OPENSSL
    connect( this, SIGNAL( sslErrors( QNetworkReply *, const QList<QSslError> & ) ),
             instance(), SIGNAL( sslErrors( QNetworkReply *, const QList<QSslError> & ) ),
             connectionType );
#endif
  }

  // check if proxy is enabled
  bool proxyEnabled = settings.value( "proxy/proxyEnabled", false ).toBool();
  if ( proxyEnabled )
  {
    excludes = settings.value( "proxy/proxyExcludedUrls", "" ).toString().split( "|", QString::SkipEmptyParts );

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
                   .arg( proxyUser ).arg( proxyPassword )
                 );
      proxy = QNetworkProxy( proxyType, proxyHost, proxyPort, proxyUser, proxyPassword );
    }
  }

#if QT_VERSION >= 0x40500
  setFallbackProxyAndExcludes( proxy, excludes );

  QNetworkDiskCache *newcache = qobject_cast<QNetworkDiskCache*>( cache() );
  if ( !newcache )
    newcache = new QNetworkDiskCache( this );

  QString cacheDirectory = settings.value( "cache/directory", QgsApplication::qgisSettingsDirPath() + "cache" ).toString();
  qint64 cacheSize = settings.value( "cache/size", 50 * 1024 * 1024 ).toULongLong();
  QgsDebugMsg( QString( "setCacheDirectory: %1" ).arg( cacheDirectory ) );
  QgsDebugMsg( QString( "setMaximumCacheSize: %1" ).arg( cacheSize ) );
  newcache->setCacheDirectory( cacheDirectory );
  newcache->setMaximumCacheSize( cacheSize );
  QgsDebugMsg( QString( "cacheDirectory: %1" ).arg( newcache->cacheDirectory() ) );
  QgsDebugMsg( QString( "maximumCacheSize: %1" ).arg( newcache->maximumCacheSize() ) );

  if ( cache() != newcache )
    setCache( newcache );
#else
  setProxy( proxy );
#endif
}

