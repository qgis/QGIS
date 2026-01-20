/***************************************************************************
                              qgsconfigcache.cpp
                              ------------------
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
#include "qgsconfigcache.h"

#include "qgsmessagelog.h"
#include "qgsserverexception.h"
#include "qgsserverprojectutils.h"
#include "qgsstorebadlayerinfo.h"
#include "qgsvectorlayer.h"

#include <QFile>

#include "moc_qgsconfigcache.cpp"

QgsConfigCache *QgsConfigCache::sInstance = nullptr;


QgsAbstractCacheStrategy *getStrategyFromSettings( QgsServerSettings *settings )
{
  QgsAbstractCacheStrategy *strategy;
  if ( settings && settings->projectCacheStrategy() == "periodic"_L1 )
  {
    strategy = new QgsPeriodicCacheStrategy( settings->projectCacheCheckInterval() );
    QgsMessageLog::logMessage(
      u"Initializing 'periodic' cache strategy"_s,
      u"Server"_s, Qgis::MessageLevel::Info
    );
  }
  else if ( settings && settings->projectCacheStrategy() == "off"_L1 )
  {
    strategy = new QgsNullCacheStrategy();
    QgsMessageLog::logMessage(
      u"Initializing 'off' cache strategy"_s,
      u"Server"_s, Qgis::MessageLevel::Info
    );
  }
  else
  {
    strategy = new QgsFileSystemCacheStrategy();
    QgsMessageLog::logMessage(
      u"Initializing 'filesystem' cache strategy"_s,
      u"Server"_s, Qgis::MessageLevel::Info
    );
  }

  return strategy;
}


void QgsConfigCache::initialize( QgsServerSettings *settings )
{
  if ( sInstance )
  {
    QgsMessageLog::logMessage(
      u"Project's cache is already initialized"_s,
      u"Server"_s, Qgis::MessageLevel::Warning
    );
    return;
  }

  sInstance = new QgsConfigCache( getStrategyFromSettings( settings ) );
}

QgsConfigCache *QgsConfigCache::instance()
{
  if ( !sInstance )
  {
    qFatal( "QgsConfigCache must be initialized before accessing QgsConfigCache instance." );
    Q_ASSERT( false );
  }
  return sInstance;
}

QgsConfigCache::QgsConfigCache( QgsServerSettings *settings )
  : QgsConfigCache( getStrategyFromSettings( settings ) )
{
  mXmlDocumentCache.setMaxCost( settings->projectCacheSize() );
  mProjectCache.setMaxCost( settings->projectCacheSize() );
}

QgsConfigCache::QgsConfigCache( QgsAbstractCacheStrategy *strategy )
  : mStrategy( strategy )
{
  mStrategy->attach( this );
}

QgsConfigCache::QgsConfigCache()
  : QgsConfigCache( new QgsFileSystemCacheStrategy() )
{
}

const QgsProject *QgsConfigCache::project( const QString &path, const QgsServerSettings *settings )
{
  if ( !mProjectCache[path] )
  {
    // disable the project style database -- this incurs unwanted cost and is not required
    auto prj = std::make_unique<QgsProject>( nullptr, Qgis::ProjectCapabilities() );

    // This is required by virtual layers that call QgsProject::instance() inside the constructor :(
    QgsProject::setInstance( prj.get() );

    QgsStoreBadLayerInfo *badLayerHandler = new QgsStoreBadLayerInfo();
    prj->setBadLayerHandler( badLayerHandler );

    // Always skip original styles storage
    Qgis::ProjectReadFlags readFlags = Qgis::ProjectReadFlag::DontStoreOriginalStyles
                                       | Qgis::ProjectReadFlag::DontLoad3DViews
                                       | Qgis::ProjectReadFlag::DontUpgradeAnnotations;
    if ( settings )
    {
      // Activate trust layer metadata flag
      if ( settings->trustLayerMetadata() )
      {
        readFlags |= Qgis::ProjectReadFlag::TrustLayerMetadata;
      }
      // Activate force layer read only flag
      if ( settings->forceReadOnlyLayers() )
      {
        readFlags |= Qgis::ProjectReadFlag::ForceReadOnlyLayers;
      }
      // Activate don't load layouts flag
      if ( settings->getPrintDisabled() )
      {
        readFlags |= Qgis::ProjectReadFlag::DontLoadLayouts;
      }
    }

    if ( prj->read( path, readFlags ) )
    {
      if ( !badLayerHandler->badLayers().isEmpty() )
      {
        // if bad layers are not restricted layers so service failed
        QStringList unrestrictedBadLayers;
        // test bad layers through restrictedlayers
        const QStringList badLayerIds = badLayerHandler->badLayers();
        const QMap<QString, QString> badLayerNames = badLayerHandler->badLayerNames();
        const QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *prj );
        for ( const QString &badLayerId : badLayerIds )
        {
          // if this bad layer is in restricted layers
          // it doesn't need to be added to unrestricted bad layers
          if ( badLayerNames.contains( badLayerId ) && restrictedLayers.contains( badLayerNames.value( badLayerId ) ) )
          {
            continue;
          }
          unrestrictedBadLayers.append( badLayerId );
        }
        if ( !unrestrictedBadLayers.isEmpty() )
        {
          // This is a critical error unless QGIS_SERVER_IGNORE_BAD_LAYERS is set to TRUE
          if ( !settings || !settings->ignoreBadLayers() )
          {
            QgsMessageLog::logMessage(
              u"Error, Layer(s) %1 not valid in project %2"_s.arg( unrestrictedBadLayers.join( ", "_L1 ), path ),
              u"Server"_s, Qgis::MessageLevel::Critical
            );
            throw QgsServerException( u"Layer(s) not valid"_s );
          }
          else
          {
            QgsMessageLog::logMessage(
              u"Warning, Layer(s) %1 not valid in project %2"_s.arg( unrestrictedBadLayers.join( ", "_L1 ), path ),
              u"Server"_s, Qgis::MessageLevel::Warning
            );
          }
        }
      }
      cacheProject( path, prj.release() );
    }
    else
    {
      QgsMessageLog::logMessage(
        u"Error when loading project file '%1': %2 "_s.arg( path, prj->error() ),
        u"Server"_s, Qgis::MessageLevel::Critical
      );
    }
  }

  auto entry = mProjectCache[path];
  if ( !entry )
  {
    return nullptr;
  }

  //Try to reload data sources of invalid layers
  if ( ( settings && settings->retryBadLayers() ) && ( entry->second->validCount() != entry->second->count() ) )
  {
    for ( const auto &l : entry->second->mapLayers() )
    {
      if ( !l->isValid() )
      {
        QString subsetString;
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( l );
        if ( vlayer )
        {
          subsetString = vlayer->subsetString();
        }
        QgsDataProvider::ProviderOptions options;
        l->setDataSource( l->source(), l->name(), l->providerType(), options );
        if ( vlayer && !subsetString.isEmpty() )
        {
          vlayer->setSubsetString( subsetString );
        }
      }
    }
  }

  return entry->second.get();
}

QList<QgsProject *> QgsConfigCache::projects() const
{
  QList<QgsProject *> projects;

  const auto constKeys { mProjectCache.keys() };
  for ( const auto &path : std::as_const( constKeys ) )
  {
    projects << mProjectCache[path]->second.get();
  }

  return projects;
}

QDomDocument *QgsConfigCache::xmlDocument( const QString &filePath )
{
  //first open file
  QFile configFile( filePath );
  if ( !configFile.exists() )
  {
    QgsMessageLog::logMessage( "Error, configuration file '" + filePath + "' does not exist", u"Server"_s, Qgis::MessageLevel::Critical );
    return nullptr;
  }

  if ( !configFile.open( QIODevice::ReadOnly ) )
  {
    QgsMessageLog::logMessage( "Error, cannot open configuration file '" + filePath + "'", u"Server"_s, Qgis::MessageLevel::Critical );
    return nullptr;
  }

  // first get cache
  QDomDocument *xmlDoc = mXmlDocumentCache.object( filePath );
  if ( !xmlDoc )
  {
    //then create xml document
    xmlDoc = new QDomDocument();
    QString errorMsg;
    int line, column;
    if ( !xmlDoc->setContent( &configFile, true, &errorMsg, &line, &column ) )
    {
      QgsMessageLog::logMessage( "Error parsing file '" + filePath + u"': parse error %1 at row %2, column %3"_s.arg( errorMsg ).arg( line ).arg( column ), u"Server"_s, Qgis::MessageLevel::Critical );
      delete xmlDoc;
      return nullptr;
    }
    mXmlDocumentCache.insert( filePath, xmlDoc );
    xmlDoc = mXmlDocumentCache.object( filePath );
    Q_ASSERT( xmlDoc );
  }
  return xmlDoc;
}


void QgsConfigCache::cacheProject( const QString &path, QgsProject *project )
{
  mProjectCache.insert( path, new std::pair<QDateTime, std::unique_ptr<QgsProject>>( project->lastModified(), std::unique_ptr<QgsProject>( project ) ) );

  mStrategy->entryInserted( path );
}

void QgsConfigCache::removeEntry( const QString &path )
{
  mProjectCache.remove( path );

  //xml document must be removed last, as other config cache destructors may require it
  mXmlDocumentCache.remove( path );

  mStrategy->entryRemoved( path );

  emit projectRemovedFromCache( path );
}

// slots

void QgsConfigCache::removeChangedEntry( const QString &path )
{
  removeEntry( path );
}


void QgsConfigCache::removeChangedEntries()
{
  // QCache::keys returns a QList so it is safe
  // to mutate while iterating
  const auto constKeys { mProjectCache.keys() };
  for ( const auto &path : std::as_const( constKeys ) )
  {
    const auto entry = mProjectCache[path];
    if ( entry && entry->first < entry->second->lastModified() )
    {
      removeEntry( path );
    }
  }
}

// File system invalidation strategy


QgsFileSystemCacheStrategy::QgsFileSystemCacheStrategy()
{
}

void QgsFileSystemCacheStrategy::attach( QgsConfigCache *cache )
{
  QObject::connect( &mFileSystemWatcher, &QFileSystemWatcher::fileChanged, cache, &QgsConfigCache::removeChangedEntry );
}

void QgsFileSystemCacheStrategy::entryRemoved( const QString &path )
{
  mFileSystemWatcher.removePath( path );
}

void QgsFileSystemCacheStrategy::entryInserted( const QString &path )
{
  mFileSystemWatcher.addPath( path );
}

// Periodic invalidation strategy

QgsPeriodicCacheStrategy::QgsPeriodicCacheStrategy( int interval )
  : mInterval( interval )
{
}

void QgsPeriodicCacheStrategy::attach( QgsConfigCache *cache )
{
  QObject::connect( &mTimer, &QTimer::timeout, cache, &QgsConfigCache::removeChangedEntries );
}


void QgsPeriodicCacheStrategy::entryRemoved( const QString &path )
{
  Q_UNUSED( path )
  // No-op
}

void QgsPeriodicCacheStrategy::entryInserted( const QString &path )
{
  Q_UNUSED( path )
  if ( !mTimer.isActive() )
  {
    mTimer.start( mInterval );
  }
}

void QgsPeriodicCacheStrategy::setCheckInterval( int msec )
{
  if ( mTimer.isActive() )
  {
    // Restart timer
    mTimer.start( msec );
  }
}


// Null strategy

void QgsNullCacheStrategy::attach( QgsConfigCache *cache )
{
  Q_UNUSED( cache )
}

void QgsNullCacheStrategy::entryRemoved( const QString &path )
{
  Q_UNUSED( path )
}

void QgsNullCacheStrategy::entryInserted( const QString &path )
{
  Q_UNUSED( path )
}
