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
#include "qgssldconfigparser.h"

#include <QFile>

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
  mFileSystemWatcher.addPath( filePath );
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
  mFileSystemWatcher.addPath( filePath );
  return p;
}

QgsWMSConfigParser* QgsConfigCache::wmsConfiguration( const QString& filePath, const QMap<QString, QString>& parameterMap )
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
    p = new QgsSLDConfigParser( doc, parameterMap );
  }
  else
  {
    p = new QgsWMSProjectParser( doc, filePath );
  }

  int numberOfLayers = p->nLayers();
  if ( numberOfLayers > 100 )
  {
    mWMSConfigCache.setMaxCost( numberOfLayers );
  }

  mWMSConfigCache.insert( filePath, p );
  mFileSystemWatcher.addPath( filePath );
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

void QgsConfigCache::removeChangedEntry( const QString& path )
{
  mWMSConfigCache.remove( path );
  mWFSConfigCache.remove( path );
  mWCSConfigCache.remove( path );
  mFileSystemWatcher.removePath( path );
}
