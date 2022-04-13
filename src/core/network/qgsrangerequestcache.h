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

#include "qgis_core.h"

#define SIP_NO_FILE


/**
 * \ingroup core
 * \brief A custom cache for handling the storage and retrieval of HTTP range requests on disk
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsRangeRequestCache
{
  public:
    //! Constructor
    QgsRangeRequestCache();

    //! Checks whether the range request exists in the cache
    bool hasEntry( const QUrl &url, QPair<qint64, qint64> range );
    //! Returns the range request data stored in the cache and an empty byte array if the data is not in the cache
    QByteArray entry( const QUrl &url, QPair<qint64, qint64> range );
    //! Adds the range request data into the cache
    void registerEntry( const QUrl &url, QPair<qint64, qint64> range, QByteArray data );

    //! Clears the cache removing all of the files
    void clear();
    //! Sets the directory where data will be stored
    void setCacheDirectory( const QString &path );
    //! Sets the cache size
    void setCacheSize( qint64 maxBytes );

    friend class TestQgsCopcProvider;
  private:
    QString mCacheDir;
    qint64 mMaxDataSize = 256 * 1024 * 1024;

    QString rangeFileName( const QUrl &url, QPair<qint64, qint64> range )
    {
      return mCacheDir + QStringLiteral( "%1?%2-%3" ).arg( qHash( url.toString() ) ).arg( range.first ).arg( range.second );
    }

    QByteArray readFile( const QString &fileName );
    void writeFile( const QString &fileName, QByteArray data );
    void removeFile( const QString &fileName );

    QFileInfoList cacheFiles();

    void expire();

    QStringList cacheEntries();
};

#endif // QGSRANGEREQUESTCACHE_H
