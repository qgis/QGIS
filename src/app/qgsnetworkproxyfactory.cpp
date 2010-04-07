/***************************************************************************
                          qgsnetworkproxyfactory.cpp - description
                             -------------------
    begin                : Sat Mar 20 2010
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
/* $Id$ */

#include <QtGlobal>

#if QT_VERSION >= 0x40500

#include <QSettings>
#include <QUrl>

#include "qgsnetworkproxyfactory.h"
#include "qgslogger.h"

QgsNetworkProxyFactory::QgsNetworkProxyFactory()
{
}

void QgsNetworkProxyFactory::setProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes )
{
  mProxy = proxy;
  mExcludedURLs = excludes;
}

QgsNetworkProxyFactory::~QgsNetworkProxyFactory()
{
}

QList<QNetworkProxy> QgsNetworkProxyFactory::queryProxy( const QNetworkProxyQuery &query )
{
  if( query.queryType() != QNetworkProxyQuery::UrlRequest )
    return QList<QNetworkProxy>() << mProxy;

  QString url = query.url().toString();

  foreach( QString exclude, mExcludedURLs )
  {
    if ( url.startsWith( exclude ) )
    {
      QgsDebugMsg( QString("using default proxy for %1 [exclude %2]").arg( url ).arg( exclude ) );
      return QList<QNetworkProxy>() << QNetworkProxy();
    }
  }

  QgsDebugMsg( QString("using user proxy for %1").arg( url ) );
  return QList<QNetworkProxy>() << mProxy; 
}

#endif // QT_VERSION >= 0x40500
