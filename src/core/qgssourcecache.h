/***************************************************************************
                         qgssourcecache.h
                         ---------------
    begin                : July 2020
    copyright            : (C) 2020 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSOURCECACHE_H
#define QGSSOURCECACHE_H

#include "qgsabstractcontentcache.h"
#include "qgis_sip.h"
#include "qgis_core.h"

#ifndef SIP_RUN

///@cond PRIVATE

/**
 * \ingroup core
 * \class QgsSourceCacheEntry
 * An entry for a QgsSourceCache, representing a given source's file path
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsSourceCacheEntry : public QgsAbstractContentCacheEntry
{
  public:

    /**
     * Constructor for QgsSourceCacheEntry, corresponding to the specified \a path.
     */
    QgsSourceCacheEntry( const QString &path ) ;

    //! The local file path of the source string
    QString filePath;

    int dataSize() const override;
    void dump() const override;
    bool isEqual( const QgsAbstractContentCacheEntry *other ) const override;

};

///@endcond
#endif

/**
 * \class QgsSourceCache
 * \ingroup core
 * A cache for source strings that returns a local file path containing the source content.
 *
 * QgsSourceCache is not usually directly created, but rather accessed through
 * QgsApplication::sourceCache().
 *
 * \since QGIS 3.16
*/
#ifdef SIP_RUN
class CORE_EXPORT QgsSourceCache : public QgsAbstractContentCacheBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsSourceCache : public QgsAbstractContentCache< QgsSourceCacheEntry >
{
#endif
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSourceCache, with the specified \a parent object.
     */
    QgsSourceCache( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a local file path reflecting the content of a specified source \a path
     *
     * \a path may be a local file, remote (HTTP) url, or a base 64 encoded string (with a "base64:" prefix).
     */
    QString localFilePath( const QString &path, bool blocking = false );

  signals:

    /**
     * Emitted when the cache has finished retrieving a 3D model from a remote \a url.
     */
    void remoteSourceFetched( const QString &url );

  private:

    QString fetchSource( const QString &path, bool &isBroken, bool blocking = false ) const;

    std::unique_ptr< QTemporaryDir > temporaryDir;
};

#endif // QGSSOURCECACHE_H
