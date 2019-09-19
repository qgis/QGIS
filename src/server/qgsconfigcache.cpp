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

const QgsProject *QgsConfigCache::project( const QString &path )
{
  if ( ! mProjectCache[ path ] )
  {
    std::unique_ptr<QgsProject> prj( new QgsProject() );
    QgsStoreBadLayerInfo *badLayerHandler = new QgsStoreBadLayerInfo();
    prj->setBadLayerHandler( badLayerHandler );
    if ( prj->read( path ) )
    {
      if ( !badLayerHandler->badLayers().isEmpty() )
      {
        QString errorMsg = QStringLiteral( "Layer(s) %1 not valid" ).arg( badLayerHandler->badLayers().join( ',' ) );
        QgsMessageLog::logMessage( errorMsg, QStringLiteral( "Server" ), Qgis::Critical );
        throw QgsServerException( QStringLiteral( "Layer(s) not valid" ) );
      }
      mProjectCache.insert( path, prj.release() );
      mFileSystemWatcher.addPath( path );
    }
    else
    {
      QgsMessageLog::logMessage(
        tr( "Error when loading project file '%1': %2 " ).arg( path, prj->error() ),
        QStringLiteral( "Server" ), Qgis::Critical );
    }
  }
  QgsProject::setInstance( mProjectCache[ path ] );
  return mProjectCache[ path ];
}

QDomDocument *QgsConfigCache::xmlDocument( const QString &filePath )
{
  //first open file
  QFile configFile( filePath );
  if ( !configFile.exists() )
  {
    QgsMessageLog::logMessage( "Error, configuration file '" + filePath + "' does not exist", QStringLiteral( "Server" ), Qgis::Critical );
    return nullptr;
  }

  if ( !configFile.open( QIODevice::ReadOnly ) )
  {
    QgsMessageLog::logMessage( "Error, cannot open configuration file '" + filePath + "'", QStringLiteral( "Server" ), Qgis::Critical );
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
                                 QStringLiteral( "': parse error %1 at row %2, column %3" ).arg( errorMsg ).arg( line ).arg( column ), QStringLiteral( "Server" ), Qgis::Critical );
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

