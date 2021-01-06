/***************************************************************************
 qgsnetworkdiskcache.h  -  Thread-safe interface for QNetworkDiskCache
    -------------------
    begin                : 2016-03-05
    copyright            : (C) 2016 by Juergen E. Fischer
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

#ifndef QGSNETWORKDISKCACHE_H
#define QGSNETWORKDISKCACHE_H

#define SIP_NO_FILE

#include <QNetworkDiskCache>
#include <QMutex>

class QNetworkDiskCache;

///@cond PRIVATE

class ExpirableNetworkDiskCache : public QNetworkDiskCache
{
    Q_OBJECT

  public:
    explicit ExpirableNetworkDiskCache( QObject *parent = nullptr ) : QNetworkDiskCache( parent ) {}
    qint64 runExpire() { return QNetworkDiskCache::expire(); }
};

///@endcond

/**
 * \ingroup core
 * Wrapper implementation of QNetworkDiskCache with all methods guarded by a
 * mutex soly for internal use of QgsNetworkAccessManagers
 *
 * \note not available in Python bindings
 */
class QgsNetworkDiskCache : public QNetworkDiskCache
{
    Q_OBJECT

  public:

    //! \see QNetworkDiskCache::cacheDirectory
    QString cacheDirectory() const;

    //! \see QNetworkDiskCache::setCacheDirectory
    void setCacheDirectory( const QString &cacheDir );

    //! \see QNetworkDiskCache::maximumCacheSize()
    qint64 maximumCacheSize() const;

    //! \see QNetworkDiskCache::setMaximumCacheSize()
    void setMaximumCacheSize( qint64 size );

    //! \see QNetworkDiskCache::metaData()
    QNetworkCacheMetaData metaData( const QUrl &url ) override;

    //! \see QNetworkDiskCache::updateMetaData()
    void updateMetaData( const QNetworkCacheMetaData &metaData ) override;

    //! \see QNetworkDiskCache::data()
    QIODevice *data( const QUrl &url ) override;

    //! \see QNetworkDiskCache::remove()
    bool remove( const QUrl &url ) override;

    //! \see QNetworkDiskCache::cacheSize()
    qint64 cacheSize() const override;

    //! \see QNetworkDiskCache::prepare()
    QIODevice *prepare( const QNetworkCacheMetaData &metaData ) override;

    //! \see QNetworkDiskCache::insert()
    void insert( QIODevice *device ) override;

    //! \see QNetworkDiskCache::fileMetaData()
    QNetworkCacheMetaData fileMetaData( const QString &fileName ) const;

  public slots:
    //! \see QNetworkDiskCache::clear()
    void clear() override;

  protected:
    //! \see QNetworkDiskCache::expire()
    qint64 expire() override;

  private:
    explicit QgsNetworkDiskCache( QObject *parent );

    static ExpirableNetworkDiskCache sDiskCache;
    static QMutex sDiskCacheMutex;

    friend class QgsNetworkAccessManager;
};

#endif // QGSNETWORKDISKCACHE_H
