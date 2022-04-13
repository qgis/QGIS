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

class CORE_EXPORT QgsRangeRequestCache
{
  public:
    QgsRangeRequestCache();

    bool hasEntry( const QUrl &url, QPair<qint64, qint64> range );
    QByteArray entry( const QUrl &url, QPair<qint64, qint64> range );
    void registerEntry( const QUrl &url, QPair<qint64, qint64> range, QByteArray data );

    void clear();
    void setCacheDirectory( const QString &path );
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
