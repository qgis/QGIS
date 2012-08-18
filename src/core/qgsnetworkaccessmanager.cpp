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
#include <qgslogger.h>

#include <QUrl>
#include <QSettings>
#include <QTimer>
#include <QNetworkReply>

#if QT_VERSION >= 0x40500
class QgsNetworkProxyFactory : public QNetworkProxyFactory
{
  public:
    QgsNetworkProxyFactory() {}
    virtual ~QgsNetworkProxyFactory() {}

    virtual QList<QNetworkProxy> queryProxy( const QNetworkProxyQuery & query = QNetworkProxyQuery() )
    {
      QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

      // iterate proxies factories and take first non empty list
      foreach ( QNetworkProxyFactory *f, nam->proxyFactories() )
      {
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

      QgsDebugMsg( QString( "using user proxy for %1" ).arg( url ) );
      return QList<QNetworkProxy>() << nam->fallbackProxy();
    }
};
#endif

QgsNetworkAccessManager *QgsNetworkAccessManager::smNAM = 0;

QgsNetworkAccessManager *QgsNetworkAccessManager::instance()
{
  if ( smNAM )
    return smNAM;

  smNAM = new QgsNetworkAccessManager();

  return smNAM;
}

QgsNetworkAccessManager::QgsNetworkAccessManager( QObject *parent )
    : QNetworkAccessManager( parent )
{
#if QT_VERSION >= 0x40500
  setProxyFactory( new QgsNetworkProxyFactory() );
#endif
}

QgsNetworkAccessManager::~QgsNetworkAccessManager()
{
}

#if QT_VERSION >= 0x40500
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
#endif

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
  mFallbackProxy = proxy;
  mExcludedURLs = excludes;
}

QNetworkReply *QgsNetworkAccessManager::createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData )
{
  emit requestAboutToBeCreated( op, req, outgoingData );
  QNetworkReply *reply = QNetworkAccessManager::createRequest( op, req, outgoingData );
  emit requestCreated( reply );

  // abort request, when network timeout happens
  QTimer *timer = new QTimer( reply );

  QSettings s;
  timer->singleShot( s.value( "/qgis/networkAndProxy/networkTimeout", "20000" ).toInt(), reply, SLOT( abort() ) );

  return reply;
}
