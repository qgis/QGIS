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
#include "qgsaccesscontrol.h"
#include "qgsproject.h"

#include <QFile>

QgsConfigCache* QgsConfigCache::instance()
{
  static QgsConfigCache *instance = nullptr;

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
  QgsMessageLog::logMessage(
    QString( "Open the project file '%1'." )
    .arg( filePath ),
    "Server", QgsMessageLog::INFO
  );

  QDomDocument* doc = xmlDocument( filePath );
  if ( !doc )
  {
    return nullptr;
  }

  QgsProjectVersion fileVersion = getVersion( *doc );
  QgsProjectVersion thisVersion( QGis::QGIS_VERSION );

  if ( thisVersion != fileVersion )
  {
    QgsMessageLog::logMessage(
      QString(
        "\n========================================================================"
        "\n= WARNING: This project file was saved by a different version of QGIS. ="
        "\n========================================================================"
      ), "Server", QgsMessageLog::WARNING
    );
  }
  QgsMessageLog::logMessage(
    QString( "QGIS server version %1, project version %2" )
    .arg( thisVersion.text() )
    .arg( fileVersion.text() ),
    "Server", QgsMessageLog::INFO
  );
  return new QgsServerProjectParser( doc, filePath );
}

QgsWCSProjectParser *QgsConfigCache::wcsConfiguration(
  const QString& filePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  , const QgsAccessControl* accessControl
#endif
)
{
  QgsWCSProjectParser *p = mWCSConfigCache.object( filePath );
  if ( !p )
  {
    QDomDocument* doc = xmlDocument( filePath );
    if ( !doc )
    {
      return nullptr;
    }
    p = new QgsWCSProjectParser(
      filePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      , accessControl
#endif
    );
    mWCSConfigCache.insert( filePath, p );
    p = mWCSConfigCache.object( filePath );
    Q_ASSERT( p );
  }

  QgsMSLayerCache::instance()->setProjectMaxLayers( p->wcsLayers().size() );
  return p;
}

QgsWFSProjectParser *QgsConfigCache::wfsConfiguration(
  const QString& filePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  , const QgsAccessControl* accessControl
#endif
)
{
  QgsWFSProjectParser *p = mWFSConfigCache.object( filePath );
  if ( !p )
  {
    QDomDocument* doc = xmlDocument( filePath );
    if ( !doc )
    {
      return nullptr;
    }
    p = new QgsWFSProjectParser(
      filePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      , accessControl
#endif
    );
    mWFSConfigCache.insert( filePath, p );
    p = mWFSConfigCache.object( filePath );
    Q_ASSERT( p );
  }

  QgsMSLayerCache::instance()->setProjectMaxLayers( p->wfsLayers().size() );
  return p;
}

QgsWMSConfigParser *QgsConfigCache::wmsConfiguration(
  const QString& filePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  , const QgsAccessControl* accessControl
#endif
  , const QMap<QString, QString>& parameterMap
)
{
  QgsWMSConfigParser *p = mWMSConfigCache.object( filePath );
  if ( !p )
  {
    QDomDocument* doc = xmlDocument( filePath );
    if ( !doc )
    {
      return nullptr;
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
      p = new QgsWMSProjectParser(
        filePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        , accessControl
#endif
      );
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
    return nullptr;
  }

  if ( !configFile.open( QIODevice::ReadOnly ) )
  {
    QgsMessageLog::logMessage( "Error, cannot open configuration file '" + filePath + "'", "Server", QgsMessageLog::CRITICAL );
    return nullptr;
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
      return nullptr;
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
  mWMSConfigCache.remove( path );
  mWFSConfigCache.remove( path );
  mWCSConfigCache.remove( path );

  //xml document must be removed last, as other config cache destructors may require it
  mXmlDocumentCache.remove( path );

  mFileSystemWatcher.removePath( path );
}


void QgsConfigCache::removeEntry( const QString& path )
{
  removeChangedEntry( path );
}

