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
/* $Id$ */

#ifndef QGSNETWORKACCESSMANAGER_H
#define QGSNETWORKACCESSMANAGER_H

#include <QList>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>

/*
 * \class QgsNetworkAccessManager
 * \brief network access manager for QGIS
 * \ingroup core
 * \since 1.5
 *
 * This class implements the QGIS network access manager.  It's a singleton
 * that can be use across QGIS.
 *
 * Plugins can insert proxy factories and thereby redirect requests to
 * individual proxies.
 *
 * If no proxy factories are there or none returns a proxy for an URL a
 * fallback proxy can be set.  There's also a exclude list that defines URLs
 * that the fallback proxy should not be used for, then no proxy will be used.
 *
 */
class CORE_EXPORT QgsNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

  public:
    //! returns a pointer to the single instance
    // and creates that instance on the first call.
    static QgsNetworkAccessManager *instance();

    //! destructor
    ~QgsNetworkAccessManager();

#if QT_VERSION >= 0x40500
    //! insert a factory into the proxy factories list
    void insertProxyFactory( QNetworkProxyFactory *factory );

    //! remove a factory from the proxy factories list
    void removeProxyFactory( QNetworkProxyFactory *factory );

    //! retrieve proxy factory list
    const QList<QNetworkProxyFactory *> proxyFactories() const;
#endif

    //! retrieve fall back proxy (for urls that no factory returned proxies for)
    const QNetworkProxy &fallbackProxy() const;

    //! retrieve exclude list (urls shouldn't use the fallback proxy)
    const QStringList &excludeList() const;

    //! set fallback proxy and URL that shouldn't use it.
    void setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes );

  signals:
    void requestAboutToBeCreated( Operation, const QNetworkRequest &, QIODevice * );
    void requestCreated( QNetworkReply * );

  protected:
    virtual QNetworkReply *createRequest( Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0 );

  private:
    QgsNetworkAccessManager( QObject *parent = 0 );
#if QT_VERSION >= 0x40500
    QList<QNetworkProxyFactory*> mProxyFactories;
#endif
    QNetworkProxy mFallbackProxy;
    QStringList mExcludedURLs;

    static QgsNetworkAccessManager *smNAM;
};

#endif // QGSNETWORKACCESSMANAGER_H
