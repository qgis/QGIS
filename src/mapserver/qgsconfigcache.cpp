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
#include "qgswcsprojectparser.h"
#include "qgswfsprojectparser.h"
#include "qgswmsprojectparser.h"

QgsConfigCache* QgsConfigCache::instance()
{
  static QgsConfigCache mInstance;
  return &mInstance;
}

QgsConfigCache::~QgsConfigCache()
{
}

QgsWCSProjectParser* QgsConfigCache::wcsConfiguration( const QString& filePath )
{
  QgsWCSProjectParser* p = mWCSConfigCache.object( filePath );
  if ( p )
  {
    return p;
  }

  QDomDocument* doc = xmlDocument( filePath );
  if ( !doc )
  {
    return 0;
  }
  p = new QgsWCSProjectParser( doc, filePath );
  mWCSConfigCache.insert( filePath, p );
  return p;
}

QgsWFSProjectParser* QgsConfigCache::wfsConfiguration( const QString& filePath )
{
  QgsWFSProjectParser* p = mWFSConfigCache.object( filePath );
  if ( p )
  {
    return p;
  }

  QDomDocument* doc = xmlDocument( filePath );
  if ( !doc )
  {
    return 0;
  }

  p = new QgsWFSProjectParser( doc, filePath );
  mWFSConfigCache.insert( filePath, p );
  return p;
}

QgsWMSConfigParser* QgsConfigCache::wmsConfiguration( const QString& filePath )
{
  QgsWMSConfigParser* p = mWMSConfigCache.object( filePath );
  if ( p )
  {
    return p;
  }

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
  }
  else
  {
    p = new QgsWMSProjectParser( doc, filePath );
  }

  mWMSConfigCache.insert( filePath, p );
  return p;
}

QDomDocument* QgsConfigCache::xmlDocument( const QString& filePath )
{
  //first open file
  QFile configFile( filePath );
  if ( !configFile.exists() || !configFile.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( "File unreadable: " + filePath );
    return 0;
  }

  //then create xml document
  QDomDocument* xmlDoc = new QDomDocument();
  QString errorMsg;
  int line, column;
  if ( !xmlDoc->setContent( &configFile, true, &errorMsg, &line, &column ) )
  {
    QgsDebugMsg( QString( "Parse error %1 at row %2, column %3 in %4 " )
                 .arg( errorMsg ).arg( line ).arg( column ).arg( filePath ) );
    delete xmlDoc;
    return 0;
  }
  return xmlDoc;
}

#if 0

#include "qgslogger.h"
#include "qgsmslayercache.h"
#include "qgsprojectfiletransform.h"
#include "qgsprojectparser.h"
#include "qgssldparser.h"
#include <QCoreApplication>


QgsConfigCache* QgsConfigCache::instance()
{
  static QgsConfigCache mInstance;
  return &mInstance;
}

QgsConfigCache::QgsConfigCache()
{
  QObject::connect( &mFileSystemWatcher, SIGNAL( fileChanged( const QString& ) ), this, SLOT( removeChangedEntry( const QString& ) ) );
}

QgsConfigCache::~QgsConfigCache()
{
  foreach ( QgsConfigParser *parser, mCachedConfigurations.values() )
  {
    delete parser;
  }
}

QgsConfigParser* QgsConfigCache::searchConfiguration( const QString& filePath )
{
  QCoreApplication::processEvents(); //check for updates from file system watcher
  QgsConfigParser* p = mCachedConfigurations.value( filePath, 0 );

  if ( p )
  {
    QgsDebugMsg( "Return configuration from cache" );
  }
  else
  {
    QgsDebugMsg( "Create new configuration" );
    p = insertConfiguration( filePath );
  }

  if ( p )
  {
    //there could be more layers in a project than allowed by the cache per default
    QgsMSLayerCache::instance()->setProjectMaxLayers( p->numberOfLayers() );
  }

  return p;
}

QgsConfigParser* QgsConfigCache::insertConfiguration( const QString& filePath )
{
  if ( mCachedConfigurations.size() > 40 )
  {
    //remove a cache entry to avoid memory problems
    QHash<QString, QgsConfigParser*>::iterator configIt = mCachedConfigurations.begin();
    if ( configIt != mCachedConfigurations.end() )
    {
      mFileSystemWatcher.removePath( configIt.key() );
      delete configIt.value();
      mCachedConfigurations.erase( configIt );
    }
  }

  //first open file
  QFile* configFile = new QFile( filePath );
  if ( !configFile->exists() || !configFile->open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( "File unreadable: " + filePath );
    delete configFile;
    return 0;
  }

  //then create xml document
  QDomDocument* configDoc = new QDomDocument();
  QString errorMsg;
  int line, column;
  if ( !configDoc->setContent( configFile, true, &errorMsg, &line, &column ) )
  {
    QgsDebugMsg( QString( "Parse error %1 at row %2, column %3 in %4 " )
                 .arg( errorMsg ).arg( line ).arg( column ).arg( filePath ) );
    delete configFile;
    delete configDoc;
    return 0;
  }

  //is it an sld document or a qgis project file?
  QDomElement documentElem = configDoc->documentElement();
  QgsConfigParser* configParser = 0;
  if ( documentElem.tagName() == "StyledLayerDescriptor" )
  {
    configParser = new QgsSLDParser( configDoc );
  }
  else if ( documentElem.tagName() == "qgis" )
  {
    //convert project file to current version first
    QgsProjectFileTransform pt( *configDoc, QgsProjectVersion( documentElem.attribute( "version" ) ) );
    pt.updateRevision( QgsProjectVersion( QGis::QGIS_VERSION ) );
    configParser = new QgsProjectParser( configDoc, filePath );
  }
  else
  {
    QgsDebugMsg( "SLD or qgis expected in " + filePath );
    delete configDoc;
    return 0;
  }

  mCachedConfigurations.insert( filePath, configParser );
  mFileSystemWatcher.addPath( filePath );
  delete configFile;
  return configParser;
}

void QgsConfigCache::removeChangedEntry( const QString& path )
{
  QgsDebugMsg( "Remove config cache entry because file changed" );
  QHash<QString, QgsConfigParser*>::iterator configIt = mCachedConfigurations.find( path );
  if ( configIt != mCachedConfigurations.end() )
  {
    delete configIt.value();
    mCachedConfigurations.erase( configIt );
  }
  mFileSystemWatcher.removePath( path );
}

#endif //0
