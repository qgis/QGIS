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

QgsConfigCache *QgsConfigCache::instance()
{
  static QgsConfigCache *sInstance = nullptr;

  if ( !sInstance )
    sInstance = new QgsConfigCache();

  return sInstance;
}

QgsConfigCache::QgsConfigCache()
{
  QObject::connect( &mFileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &QgsConfigCache::removeChangedEntry );
}


const QgsProject *QgsConfigCache::project( const QString &path, const QgsServerSettings *settings )
{
  if ( ! mProjectCache[ path ] )
  {

    std::unique_ptr<QgsProject> prj( new QgsProject() );

    // This is required by virtual layers that call QgsProject::instance() inside the constructor :(
    QgsProject::setInstance( prj.get() );

    QgsStoreBadLayerInfo *badLayerHandler = new QgsStoreBadLayerInfo();
    prj->setBadLayerHandler( badLayerHandler );

    // Always skip original styles storage
    QgsProject::ReadFlags readFlags = QgsProject::ReadFlag() | QgsProject::ReadFlag::FlagDontStoreOriginalStyles ;
    if ( settings )
    {
      // Activate trust layer metadata flag
      if ( settings->trustLayerMetadata() )
      {
        readFlags |= QgsProject::ReadFlag::FlagTrustLayerMetadata;
      }
      // Activate don't load layouts flag
      if ( settings->getPrintDisabled() )
      {
        readFlags |= QgsProject::ReadFlag::FlagDontLoadLayouts;
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
      mProjectCache.insert( path, prj.release() );
      mFileSystemWatcher.addPath( path );
    }
    else
    {
      QgsMessageLog::logMessage(
        QStringLiteral( "Error when loading project file '%1': %2 " ).arg( path, prj->error() ),
        QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    }
  }
  return mProjectCache[ path ];
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
    mFileSystemWatcher.addPath( filePath );
    xmlDoc = mXmlDocumentCache.object( filePath );
    Q_ASSERT( xmlDoc );
  }
  return xmlDoc;
}

void QgsConfigCache::removeChangedEntry( const QString &path )
{
  mProjectCache.remove( path );

  //xml document must be removed last, as other config cache destructors may require it
  mXmlDocumentCache.remove( path );

  mFileSystemWatcher.removePath( path );
}


void QgsConfigCache::removeEntry( const QString &path )
{
  removeChangedEntry( path );
}
