/***************************************************************************
                          qgsabout.h  -  description
                             -------------------
    begin                : Sat, 20 Mar 2010
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
/* $Id:$ */
#ifndef QGSNETWORKPROXYFACTORY_H
#define QGSNETWORKPROXYFACTORY_H

#include <QNetworkProxyFactory>
#include <QStringList>

class QgsNetworkProxyFactory : public QObject, public QNetworkProxyFactory
{
  public:
    QgsNetworkProxyFactory();
    virtual ~QgsNetworkProxyFactory();
    virtual QList<QNetworkProxy> queryProxy( const QNetworkProxyQuery & query = QNetworkProxyQuery() );

    void setProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes );

  private:
    QStringList mExcludedURLs;
    QNetworkProxy mProxy;
};

#endif
