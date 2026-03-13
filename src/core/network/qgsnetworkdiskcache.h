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


#include "qgis_core.h"

#include <QMutex>
#include <QNetworkDiskCache>

#define SIP_NO_FILE

class QNetworkDiskCache;

///@cond PRIVATE

class ExpirableNetworkDiskCache : public QNetworkDiskCache
{
    Q_OBJECT

  public:
    explicit ExpirableNetworkDiskCache( QObject *parent = nullptr )
      : QNetworkDiskCache( parent )
    {}
    qint64 runExpire() { return QNetworkDiskCache::expire(); }
};

///@endcond

/**
 * \ingroup core
 * \brief Wrapper implementation of QNetworkDiskCache with all methods guarded by a
 * mutex solely for internal use of QgsNetworkAccessManagers
 *
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsNetworkDiskCache : public QNetworkDiskCache
{
    Q_OBJECT

  public:
    /**
     * Registers the original request headers for a pending request to the specified \a url.
     *
     * This method is thread-safe.
     *
     * \see hasPendingRequestForUrl()
     * \see removePendingRequestForUrl()
     * \since QGIS 4.0
     */
    void insertPendingRequestHeaders( const QUrl &url, const QVariantMap &headers );

    /**
     * Returns TRUE if there is a pending (ongoing) request in place for the specified \a url.
     *
      * This method is thread-safe.
     *
     * \see insertPendingRequestHeaders()
     * \see removePendingRequestForUrl()
     *
     * \since QGIS 4.0
     */
    bool hasPendingRequestForUrl( const QUrl &url ) const;

    /**
     * Removes a pending (ongoing) request in place for the specified \a url.
     *
     * This method is thread-safe.
     *
     * \see insertPendingRequestHeaders()
     * \see hasPendingRequestForUrl()
     *
     * \since QGIS 4.0
     */
    void removePendingRequestForUrl( const QUrl &url ) const;

    /**
     * Returns TRUE if the cache has a matching but invalid entry for a \a request.
     *
     * Since QNetworkDiskCache entirely basis cache storage on URLs alone, we may
     * get situations where there's a match for a URL in the cache but it should
     * NOT be used for the specified \a request (e.g. when the previous response
     * had a non-matching "Vary" attribute).
     *
     * If this method returns FALSE, then the request should be amended to
     * prevent potential false-positive retrieval from cache.
     *
     * This method is thread-safe.
     */
    bool hasInvalidMatchForRequest( const QNetworkRequest &request );

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

    /**
     * Returns a smart cache size, in bytes, based on available free space
     * \since QGIS 3.40
     */
    static qint64 smartCacheSize( const QString &path );

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

    static QHash<QUrl, QVariantMap> sPendingRequestHeaders;

    friend class QgsNetworkAccessManager;
};

#endif // QGSNETWORKDISKCACHE_H
