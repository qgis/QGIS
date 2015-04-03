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
#include "qgsmslayercache.h"
#include "qgswcsprojectparser.h"
#include "qgswfsprojectparser.h"
#include "qgswmsprojectparser.h"
#include "qgssldconfigparser.h"

#include <QFile>

QgsConfigCache* QgsConfigCache::instance()
{
  static QgsConfigCache *instance = 0;

  if ( !instance )
    instance = new QgsConfigCache();

  return instance;
}

QgsConfigCache::QgsConfigCache()
{
  QObject::connect( &mFileSystemWatcher, SIGNAL( fileChanged( const QString& ) ), this, SLOT( removeChangedEntry( const QString& ) ) );
}

QgsConfigCache::~QgsConfigCache()
{
}

QgsServerProjectParser* QgsConfigCache::serverConfiguration( const QString& filePath )
{
  QDomDocument* doc = xmlDocument( filePath );
  if ( !doc )
  {
    return 0;
  }
  return new QgsServerProjectParser( doc, filePath );
}

QgsWCSProjectParser *QgsConfigCache::wcsConfiguration( const QString& filePath )
{
  QgsWCSProjectParser *p = mWCSConfigCache.object( filePath );
  if ( !p )
  {
    QDomDocument* doc = xmlDocument( filePath );
    if ( !doc )
    {
      return 0;
    }
    p = new QgsWCSProjectParser( filePath );
    mWCSConfigCache.insert( filePath, p );
    p = mWCSConfigCache.object( filePath );
    Q_ASSERT( p );
  }

  QgsMSLayerCache::instance()->setProjectMaxLayers( p->wcsLayers().size() );
  return p;
}

QgsWFSProjectParser *QgsConfigCache::wfsConfiguration( const QString& filePath )
{
  QgsWFSProjectParser *p = mWFSConfigCache.object( filePath );
  if ( !p )
  {
    QDomDocument* doc = xmlDocument( filePath );
    if ( !doc )
    {
      return 0;
    }
    p = new QgsWFSProjectParser( filePath );
    mWFSConfigCache.insert( filePath, p );
    p = mWFSConfigCache.object( filePath );
    Q_ASSERT( p );
  }

  QgsMSLayerCache::instance()->setProjectMaxLayers( p->wfsLayers().size() );
  return p;
}

QgsWMSConfigParser *QgsConfigCache::wmsConfiguration( const QString& filePath, const QMap<QString, QString>& parameterMap )
{
  QgsWMSConfigParser *p = mWMSConfigCache.object( filePath );
  if ( !p )
  {
    QDomDocument* doc = xmlDocument( filePath );
    if ( !doc )
    {
      return 0;
    }

    //sld or QGIS project file?
    //is it an sld document or a qgis project file?
    QDomElement documentElem = doc->documentElement();
    if ( documentElem.tagName() == "StyledLayerDescriptor" )
    {
      p = new QgsSLDConfigParser( doc, parameterMap );
    }
    else
    {
      p = new QgsWMSProjectParser( filePath );
    }
    mWMSConfigCache.insert( filePath, p );
    p = mWMSConfigCache.object( filePath );
    Q_ASSERT( p );
  }

  QgsMSLayerCache::instance()->setProjectMaxLayers( p->nLayers() );
  return p;
}

QDomDocument* QgsConfigCache::xmlDocument( const QString& filePath )
{
  //first open file
  QFile configFile( filePath );
  if ( !configFile.exists() )
  {
    QgsMessageLog::logMessage( "Error, configuration file '" + filePath + "' does not exist", "Server", QgsMessageLog::CRITICAL );
    return 0;
  }

  if ( !configFile.open( QIODevice::ReadOnly ) )
  {
    QgsMessageLog::logMessage( "Error, cannot open configuration file '" + filePath + "'", "Server", QgsMessageLog::CRITICAL );
    return 0;
  }

  // first get cache
  QDomDocument* xmlDoc = mXmlDocumentCache.object( filePath );
  if ( !xmlDoc )
  {
    //then create xml document
    xmlDoc = new QDomDocument();
    QString errorMsg;
    int line, column;
    if ( !xmlDoc->setContent( &configFile, true, &errorMsg, &line, &column ) )
    {
      QgsMessageLog::logMessage( "Error parsing file '" + filePath +
                                 QString( "': parse error %1 at row %2, column %3" ).arg( errorMsg ).arg( line ).arg( column ), "Server", QgsMessageLog::CRITICAL );
      delete xmlDoc;
      return 0;
    }
    mXmlDocumentCache.insert( filePath, xmlDoc );
    mFileSystemWatcher.addPath( filePath );
    xmlDoc = mXmlDocumentCache.object( filePath );
    Q_ASSERT( xmlDoc );
  }
  return xmlDoc;
}

void QgsConfigCache::removeChangedEntry( const QString& path )
{
  mXmlDocumentCache.remove( path );
  mWMSConfigCache.remove( path );
  mWFSConfigCache.remove( path );
  mWCSConfigCache.remove( path );
  mFileSystemWatcher.removePath( path );
}
