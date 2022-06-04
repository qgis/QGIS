/***************************************************************************
                         qgsrangerequestcache.h
                         --------------------
    begin                : April 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRANGEREQUESTCACHE_H
#define QGSRANGEREQUESTCACHE_H

#include <QtGlobal>
#include <QMap>
#include <QByteArray>
#include <QUrl>
#include <QFileInfoList>
#include <QDir>
#include <QNetworkRequest>

#include "qgis_core.h"

#define SIP_NO_FILE


/**
 * \ingroup core
 *
 * \brief A custom cache for handling the storage and retrieval of HTTP range requests on disk
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsRangeRequestCache
{
  public:
    //! Constructor
    QgsRangeRequestCache();

    //! Checks whether the range request exists in the cache
    bool hasEntry( const QNetworkRequest &request );
    //! Returns the range request data stored in the cache and an empty byte array if the data is not in the cache
    QByteArray entry( const QNetworkRequest &request );
    //! Adds the range request data into the cache
    void registerEntry( const QNetworkRequest &request, QByteArray data );

    //! Clears the cache removing all of the files
    void clear();

    /**
     * Set the Cache Directory object.
     * Returns whether the cache directory was set properly
     * \see error()
     */
    bool setCacheDirectory( const QString &path );
    //! Sets the cache size
    void setCacheSize( qint64 maxBytes );

    //! Returns the last error that occurred when manipulating the cache.
    QString error() const { return mError; }

    friend class TestQgsCopcProvider;
  private:
    QString mError;
    QString mCacheDir;
    qint64 mMaxDataSize = 256 * 1024 * 1024;

    QString rangeFileName( const QNetworkRequest &request ) const;

    QByteArray readFile( const QString &fileName );
    bool writeFile( const QString &fileName, QByteArray data );
    bool removeFile( const QString &fileName );

    void expire();

    QFileInfoList cacheEntries();
};

#endif // QGSRANGEREQUESTCACHE_H
