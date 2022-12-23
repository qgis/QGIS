/***************************************************************************
                              qgsconfigcache.h
                              ----------------
  begin                : July 24th, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCONFIGCACHE_H
#define QGSCONFIGCACHE_H

#include "qgsconfig.h"

#include <QCache>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QObject>
#include <QDomDocument>

#include "qgis_server.h"
#include "qgis_sip.h"
#include "qgsproject.h"
#include "qgsserversettings.h"

#ifndef SIP_RUN

class QgsConfigCache;

/**
 * \ingroup server
 * \brief Abstract base class for implementing
 * cache invalidation strategy
 *
 * \since QGIS 3.26
 */
class SERVER_EXPORT QgsAbstractCacheStrategy
{
  public:
    //! The name of the strategy
    virtual QString name() const = 0;

    /**
     * Called when an entry is removed from cache.
     * \param path The path of the project
     */
    virtual void entryRemoved( const QString &path ) = 0;

    /**
     * Called when an entry is removed from cache.
     * \param path The path of the project
     */
    virtual void entryInserted( const QString &path ) = 0;

    //! Attache cache to this strategy
    virtual void attach( QgsConfigCache *cache ) = 0;

    virtual ~QgsAbstractCacheStrategy() = default;

};

#endif // SIP_RUN

/**
 * \ingroup server
 * \brief Cache for server configuration.
 * \since QGIS 2.8
 */
class SERVER_EXPORT QgsConfigCache : public QObject
{
    Q_OBJECT
  public:

    /**
     * Initialize from settings.
     *
     * This method must be called prior any call to `QgsConfigCache::instance`
     */
    static void initialize( QgsServerSettings *settings );

    /**
     * Returns the current instance.
     */
    static QgsConfigCache *instance();

    /**
     * Removes an entry from cache.
     * \param path The path of the project
     */
    void removeEntry( const QString &path );

    /**
     * If the project is not cached yet, then the project is read from the
     * path. If the project is not available, then NULLPTR is returned.
     * If the project contains any bad layer it is considered unavailable
     * unless the server configuration variable QGIS_SERVER_IGNORE_BAD_LAYERS
     * passed in the optional settings argument is set to TRUE (the default
     * value is FALSE).
     * \param path the filename of the QGIS project
     * \param settings QGIS server settings
     * \returns the project or NULLPTR if an error happened
     * \since QGIS 3.0
     */
    const QgsProject *project( const QString &path, const QgsServerSettings *settings = nullptr );

    /**
     * Returns the name of the current strategy
     * \since QGIS 3.26
     */
    QString strategyName() const { return mStrategy->name(); }

  public:
    //! Initialize from settings
    QgsConfigCache( QgsServerSettings *settings );

    //! Initialize with a strategy implementation.
    QgsConfigCache( QgsAbstractCacheStrategy *strategy ) SIP_SKIP;

  private:
    // SIP require this
    QgsConfigCache() SIP_FORCE;

  private:
    //! Returns xml document for project file / sld or 0 in case of errors
    QDomDocument *xmlDocument( const QString &filePath );

    QCache<QString, QDomDocument> mXmlDocumentCache;
    QCache<QString, std::pair<QDateTime, std::unique_ptr<QgsProject> > > mProjectCache;

    std::unique_ptr<QgsAbstractCacheStrategy> mStrategy;

  private:
    //! Insert project in cache
    void cacheProject( const QString &path, QgsProject *project );

    static QgsConfigCache *sInstance;

  public slots:
    //! Remove cache entry
    void removeChangedEntry( const QString &path );

    //! Remove all changed cache entries
    void removeChangedEntries();
};


#ifndef SIP_RUN

/**
 * \ingroup server
 * \brief File system cache strategy for server configuration
 * \since QGIS 3.26
 */
class SERVER_EXPORT QgsFileSystemCacheStrategy : public QgsAbstractCacheStrategy
{
  public:
    //! Creates a new filesystem strategy
    QgsFileSystemCacheStrategy();

    //! The name of the strategy
    QString name() const override { return QStringLiteral( "filesystem" ); };

    //! Attach cache to this strategy
    void attach( QgsConfigCache *cache ) override;

    /**
     * Called when an entry is removed from cache.
     * \param path The path of the project
     */
    void entryRemoved( const QString &path ) override;

    /**
     * Called when an entry is inserted
     * \param path The path of the project
     */
    void entryInserted( const QString &path ) override;

  private:
    //! For invalidating files
    QFileSystemWatcher mFileSystemWatcher;
};


/**
 * \ingroup server
 * \brief Periodic system cache strategy for server configuration
 * \since QGIS 3.26
 */
class SERVER_EXPORT QgsPeriodicCacheStrategy: public QgsAbstractCacheStrategy
{
  public:

    /**
     *  Creates a new periodic strategy
     *  \param interval The invalidation check interval in milliseconds
     */
    QgsPeriodicCacheStrategy( int interval = 3000 );

    //! The name of the strategy
    QString name() const override { return QStringLiteral( "periodic" ); };

    /**
     * Sets the invalidation check interval for PeriodicStrategy
     *
     * If \a msec is set to a positive value then the cache will check for invalidation
     * every \a msec milliseconds
     */
    void setCheckInterval( int msec );

    //! Returns the invalidation check interval
    int checkInterval() const { return mInterval; }

    //! Attaches cache to this strategy
    void attach( QgsConfigCache *owner ) override;

    /**
     * Called when an entry is removed from cache.
     * \param path The path of the project
     */
    void entryRemoved( const QString &path ) override;

    /**
     * Called when an entry is inserted
     * \param path The path of the project
     */
    void entryInserted( const QString &path ) override;

  private:
    //! Timer for checking cache invalidation
    QTimer mTimer;

    //! Interval in ms between two invalidation check
    int mInterval;
};

/**
 * \ingroup server
 * \brief Null system cache strategy for server configuration,
 * completely disable cache invalidation
 * invalidation
 * \since QGIS 3.26
 */
class SERVER_EXPORT QgsNullCacheStrategy: public QgsAbstractCacheStrategy
{
  public:
    //! Creates a new null strategy
    QgsNullCacheStrategy() = default;

    //! The name of the strategy
    QString name() const override { return QStringLiteral( "off" ); };

    //! Attaches cache to this strategy
    void attach( QgsConfigCache *owner ) override;

    /**
     * Called when an entry is removed from cache.
     * \param path The path of the project
     */
    void entryRemoved( const QString &path ) override;

    /**
     * Called when an entry is inserted
     * \param path The path of the project
     */
    void entryInserted( const QString &path ) override;
};

#endif // SIP_RUN

#endif // QGSCONFIGCACHE_H

