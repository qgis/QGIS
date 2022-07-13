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
#include "qgsstorebadlayerinfo.h"
#include "qgsserverprojectutils.h"

#include <QFile>

QgsConfigCache *QgsConfigCache::sInstance = nullptr;


QgsAbstractCacheStrategy *getStrategyFromSettings( QgsServerSettings *settings )
{
  QgsAbstractCacheStrategy *strategy;
  if ( settings && settings->projectCacheStrategy() == QLatin1String( "periodic" ) )
  {
    strategy = new QgsPeriodicCacheStrategy( settings->projectCacheCheckInterval() );
    QgsMessageLog::logMessage(
      QStringLiteral( "Initializing 'periodic' cache strategy" ),
      QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }
  else if ( settings && settings->projectCacheStrategy() == QLatin1String( "off" ) )
  {
    strategy = new QgsNullCacheStrategy();
    QgsMessageLog::logMessage(
      QStringLiteral( "Initializing 'off' cache strategy" ),
      QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }
  else
  {
    strategy = new QgsFileSystemCacheStrategy();
    QgsMessageLog::logMessage(
      QStringLiteral( "Initializing 'filesystem' cache strategy" ),
      QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }

  return strategy;
}


void QgsConfigCache::initialize( QgsServerSettings *settings )
{
  if ( sInstance )
  {
    QgsMessageLog::logMessage(
      QStringLiteral( "Project's cache is already initialized" ),
      QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
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

}

QgsConfigCache::QgsConfigCache( QgsAbstractCacheStrategy *strategy )
  : mStrategy( strategy )
{
  mStrategy->attach( this );
}

QgsConfigCache::QgsConfigCache() : QgsConfigCache( new QgsFileSystemCacheStrategy() )
{

}

const QgsProject *QgsConfigCache::project( const QString &path, const QgsServerSettings *settings )
{
  if ( !mProjectCache[ path ] )
  {
    // disable the project style database -- this incurs unwanted cost and is not required
    std::unique_ptr<QgsProject> prj( new QgsProject( nullptr, Qgis::ProjectCapabilities() ) );

    // This is required by virtual layers that call QgsProject::instance() inside the constructor :(
    QgsProject::setInstance( prj.get() );

    QgsStoreBadLayerInfo *badLayerHandler = new QgsStoreBadLayerInfo();
    prj->setBadLayerHandler( badLayerHandler );

    // Always skip original styles storage
    Qgis::ProjectReadFlags readFlags = Qgis::ProjectReadFlag::DontStoreOriginalStyles
                                       | Qgis::ProjectReadFlag::DontLoad3DViews;
    if ( settings )
    {
      // Activate trust layer metadata flag
      if ( settings->trustLayerMetadata() )
      {
        readFlags |= Qgis::ProjectReadFlag::TrustLayerMetadata;
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
        const QStringList resctrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *prj );
        for ( const QString &badLayerId : badLayerIds )
        {
          // if this bad layer is in restricted layers
          // it doesn't need to be added to unrestricted bad layers
          if ( badLayerNames.contains( badLayerId ) &&
               resctrictedLayers.contains( badLayerNames.value( badLayerId ) ) )
          {
            continue;
          }
          unrestrictedBadLayers.append( badLayerId );
        }
        if ( !unrestrictedBadLayers.isEmpty() )
        {
          // This is a critical error unless QGIS_SERVER_IGNORE_BAD_LAYERS is set to TRUE
          if ( ! settings || ! settings->ignoreBadLayers() )
          {
            QgsMessageLog::logMessage(
              QStringLiteral( "Error, Layer(s) %1 not valid in project %2" ).arg( unrestrictedBadLayers.join( QLatin1String( ", " ) ), path ),
              QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
            throw QgsServerException( QStringLiteral( "Layer(s) not valid" ) );
          }
          else
          {
            QgsMessageLog::logMessage(
              QStringLiteral( "Warning, Layer(s) %1 not valid in project %2" ).arg( unrestrictedBadLayers.join( QLatin1String( ", " ) ), path ),
              QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
          }
        }
      }
      cacheProject( path, prj.release() );
    }
    else
    {
      QgsMessageLog::logMessage(
        QStringLiteral( "Error when loading project file '%1': %2 " ).arg( path, prj->error() ),
        QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    }
  }

  auto entry = mProjectCache[ path ];
  return entry ? entry->second.get() : nullptr;
}

QDomDocument *QgsConfigCache::xmlDocument( const QString &filePath )
{
  //first open file
  QFile configFile( filePath );
  if ( !configFile.exists() )
  {
    QgsMessageLog::logMessage( "Error, configuration file '" + filePath + "' does not exist", QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    return nullptr;
  }

  if ( !configFile.open( QIODevice::ReadOnly ) )
  {
    QgsMessageLog::logMessage( "Error, cannot open configuration file '" + filePath + "'", QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
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
      QgsMessageLog::logMessage( "Error parsing file '" + filePath +
                                 QStringLiteral( "': parse error %1 at row %2, column %3" ).arg( errorMsg ).arg( line ).arg( column ), QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
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
  mProjectCache.insert( path, new std::pair<QDateTime, std::unique_ptr<QgsProject> >( project->lastModified(),  std::unique_ptr<QgsProject>( project ) ) );

  mStrategy->entryInserted( path );
}

void QgsConfigCache::removeEntry( const QString &path )
{
  mProjectCache.remove( path );

  //xml document must be removed last, as other config cache destructors may require it
  mXmlDocumentCache.remove( path );

  mStrategy->entryRemoved( path );
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
  const auto constKeys {  mProjectCache.keys() };
  for ( const auto &path : std::as_const( constKeys ) )
  {
    const auto entry = mProjectCache[ path ];
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

