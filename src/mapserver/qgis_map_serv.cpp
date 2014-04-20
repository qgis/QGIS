/***************************************************************************
                              qgs_map_serv.cpp
 A server application supporting WMS / WFS / WCS
                              -------------------
  begin                : July 04, 2006
  copyright            : (C) 2006 by Marco Hugentobler & Ionut Iosifescu Enescu
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscapabilitiescache.h"
#include "qgsconfigcache.h"
#include "qgsfontutils.h"
#include "qgsgetrequesthandler.h"
#include "qgspostrequesthandler.h"
#include "qgssoaprequesthandler.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgswmsserver.h"
#include "qgswfsserver.h"
#include "qgswcsserver.h"
#include "qgsmaprenderer.h"
#include "qgsmapserviceexception.h"
#include "qgspallabeling.h"
#include "qgsnetworkaccessmanager.h"

#include <QDomDocument>
#include <QNetworkDiskCache>
#include <QImage>
#include <QSettings>
#include <QDateTime>

//for CMAKE_INSTALL_PREFIX
#include "qgsconfig.h"

#include <fcgi_stdio.h>


void dummyMessageHandler( QtMsgType type, const char *msg )
{
#ifdef QGSMSDEBUG
  QString output;

  switch ( type )
  {
    case QtDebugMsg:
      output += "Debug: ";
      break;
    case QtCriticalMsg:
      output += "Critical: ";
      break;
    case QtWarningMsg:
      output += "Warning: ";
      break;
    case QtFatalMsg:
      output += "Fatal: ";
  }

  output += msg;

  QgsLogger::logMessageToFile( output );

  if ( type == QtFatalMsg )
    abort();
#else
  Q_UNUSED( type );
  Q_UNUSED( msg );
#endif
}

void printRequestInfos()
{
#ifdef QGSMSDEBUG
  //print out some infos about the request
  QgsDebugMsg( "************************new request**********************" );
  QgsDebugMsg( QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );

  if ( getenv( "REMOTE_ADDR" ) != NULL )
  {
    QgsDebugMsg( "remote ip: " + QString( getenv( "REMOTE_ADDR" ) ) );
  }
  if ( getenv( "REMOTE_HOST" ) != NULL )
  {
    QgsDebugMsg( "remote host: " + QString( getenv( "REMOTE_HOST" ) ) );
  }
  if ( getenv( "REMOTE_USER" ) != NULL )
  {
    QgsDebugMsg( "remote user: " + QString( getenv( "REMOTE_USER" ) ) );
  }
  if ( getenv( "REMOTE_IDENT" ) != NULL )
  {
    QgsDebugMsg( "REMOTE_IDENT: " + QString( getenv( "REMOTE_IDENT" ) ) );
  }
  if ( getenv( "CONTENT_TYPE" ) != NULL )
  {
    QgsDebugMsg( "CONTENT_TYPE: " + QString( getenv( "CONTENT_TYPE" ) ) );
  }
  if ( getenv( "AUTH_TYPE" ) != NULL )
  {
    QgsDebugMsg( "AUTH_TYPE: " + QString( getenv( "AUTH_TYPE" ) ) );
  }
  if ( getenv( "HTTP_USER_AGENT" ) != NULL )
  {
    QgsDebugMsg( "HTTP_USER_AGENT: " + QString( getenv( "HTTP_USER_AGENT" ) ) );
  }
  if ( getenv( "HTTP_PROXY" ) != NULL )
  {
    QgsDebugMsg( "HTTP_PROXY: " + QString( getenv( "HTTP_PROXY" ) ) );
  }
  if ( getenv( "HTTPS_PROXY" ) != NULL )
  {
    QgsDebugMsg( "HTTPS_PROXY: " + QString( getenv( "HTTPS_PROXY" ) ) );
  }
  if ( getenv( "NO_PROXY" ) != NULL )
  {
    QgsDebugMsg( "NO_PROXY: " + QString( getenv( "NO_PROXY" ) ) );
  }

#endif //QGSMSDEBUG
}

QFileInfo defaultProjectFile()
{
  QDir currentDir;
  fprintf( FCGI_stderr, "current directory: %s\n", currentDir.absolutePath().toUtf8().constData() );
  QStringList nameFilterList;
  nameFilterList << "*.qgs";
  QFileInfoList projectFiles = currentDir.entryInfoList( nameFilterList, QDir::Files, QDir::Name );
  for ( int x = 0; x < projectFiles.size(); x++ )
  {
    QgsDebugMsg( projectFiles.at( x ).absoluteFilePath() );
  }
  if ( projectFiles.size() < 1 )
  {
    return QFileInfo();
  }
  return projectFiles.at( 0 );
}

QFileInfo defaultAdminSLD()
{
  return QFileInfo( "admin.sld" );
}

int fcgi_accept()
{
#ifdef Q_OS_WIN
  if ( FCGX_IsCGI() )
    return FCGI_Accept();
  else
    return FCGX_Accept( &FCGI_stdin->fcgx_stream, &FCGI_stdout->fcgx_stream, &FCGI_stderr->fcgx_stream, &environ );
#else
  return FCGI_Accept();
#endif
}

void setupNetworkAccessManager()
{
  QSettings settings;
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
  QNetworkDiskCache *cache = new QNetworkDiskCache( 0 );
  QString cacheDirectory = settings.value( "cache/directory", QgsApplication::qgisSettingsDirPath() + "cache" ).toString();
  qint64 cacheSize = settings.value( "cache/size", 50 * 1024 * 1024 ).toULongLong();
  QgsDebugMsg( QString( "setCacheDirectory: %1" ).arg( cacheDirectory ) );
  QgsDebugMsg( QString( "setMaximumCacheSize: %1" ).arg( cacheSize ) );
  cache->setCacheDirectory( cacheDirectory );
  cache->setMaximumCacheSize( cacheSize );
  QgsDebugMsg( QString( "cacheDirectory: %1" ).arg( cache->cacheDirectory() ) );
  QgsDebugMsg( QString( "maximumCacheSize: %1" ).arg( cache->maximumCacheSize() ) );
  nam->setCache( cache );
}

QgsRequestHandler* createRequestHandler()
{
  QgsRequestHandler* requestHandler = 0;
  char* requestMethod = getenv( "REQUEST_METHOD" );
  if ( requestMethod != NULL )
  {
    if ( strcmp( requestMethod, "POST" ) == 0 )
    {
      //requestHandler = new QgsSOAPRequestHandler();
      requestHandler = new QgsPostRequestHandler();
    }
    else
    {
      requestHandler = new QgsGetRequestHandler();
    }
  }
  else
  {
    requestHandler = new QgsGetRequestHandler();
  }
  return requestHandler;
}

QString configPath( const QString& defaultConfigPath, const QMap<QString, QString>& parameters )
{
  QString cfPath( defaultConfigPath );
  QString projectFile = getenv( "QGIS_PROJECT_FILE" );
  if ( !projectFile.isEmpty() )
  {
    cfPath = projectFile;
  }
  else
  {
    QMap<QString, QString>::const_iterator paramIt = parameters.find( "MAP" );
    if ( paramIt == parameters.constEnd() )
    {
      QgsDebugMsg( QString( "Using default configuration file path: %1" ).arg( defaultConfigPath ) );
    }
    else
    {
      cfPath = paramIt.value();
    }
  }
  return cfPath;
}


int main( int argc, char * argv[] )
{
#ifndef _MSC_VER
  qInstallMsgHandler( dummyMessageHandler );
#endif

  QgsApplication qgsapp( argc, argv, getenv( "DISPLAY" ) );

  //Default prefix path may be altered by environment variable
  QgsApplication::init();
#if !defined(Q_OS_WIN)
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::setPrefixPath( CMAKE_INSTALL_PREFIX, TRUE );
#endif

#if defined(MAPSERVER_SKIP_ECW)
  QgsDebugMsg( "Skipping GDAL ECW drivers in server." );
  QgsApplication::skipGdalDriver( "ECW" );
  QgsApplication::skipGdalDriver( "JP2ECW" );
#endif

  setupNetworkAccessManager();
  QDomImplementation::setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );
  QgsDebugMsg( "Prefix  PATH: " + QgsApplication::prefixPath() );
  QgsDebugMsg( "Plugin  PATH: " + QgsApplication::pluginPath() );
  QgsDebugMsg( "PkgData PATH: " + QgsApplication::pkgDataPath() );
  QgsDebugMsg( "User DB PATH: " + QgsApplication::qgisUserDbFilePath() );

  QgsDebugMsg( qgsapp.applicationDirPath() + "/qgis_wms_server.log" );
  QgsApplication::createDB(); //init qgis.db (e.g. necessary for user crs)

  QString defaultConfigFilePath;
  QFileInfo projectFileInfo = defaultProjectFile(); //try to find a .qgs file in the server directory
  if ( projectFileInfo.exists() )
  {
    defaultConfigFilePath = projectFileInfo.absoluteFilePath();
    QgsDebugMsg( "Using default project file: " + defaultConfigFilePath );
  }
  else
  {
    QFileInfo adminSLDFileInfo = defaultAdminSLD();
    if ( adminSLDFileInfo.exists() )
    {
      defaultConfigFilePath = adminSLDFileInfo.absoluteFilePath();
    }
  }

  //create cache for capabilities XML
  QgsCapabilitiesCache capabilitiesCache;

  //creating QgsMapRenderer is expensive (access to srs.db), so we do it here before the fcgi loop
  QgsMapRenderer* theMapRenderer = new QgsMapRenderer();
  theMapRenderer->setLabelingEngine( new QgsPalLabeling() );

  printRequestInfos();

#ifdef QGSMSDEBUG
  QgsFontUtils::loadStandardTestFonts( QStringList() << "Roman" << "Bold" );
#endif

  while ( fcgi_accept() >= 0 )
  {
    printRequestInfos(); //print request infos if in debug mode

    //Request handler
    QgsRequestHandler* theRequestHandler = createRequestHandler();
    QMap<QString, QString> parameterMap;
    try
    {
      parameterMap = theRequestHandler->parseInput();
    }
    catch ( QgsMapServiceException& e )
    {
      QgsDebugMsg( "An exception was thrown during input parsing" );
      theRequestHandler->sendServiceException( e );
      continue;
    }

    QMap<QString, QString>::const_iterator paramIt;

    //Config file path
    QString configFilePath = configPath( defaultConfigFilePath, parameterMap );

    //Service parameter
    QString serviceString;
    paramIt = parameterMap.find( "SERVICE" );
    if ( paramIt == parameterMap.constEnd() )
    {
      theRequestHandler->sendServiceException( QgsMapServiceException( "ServiceNotSpecified", "Service not specified. The SERVICE parameter is mandatory" ) );
      delete theRequestHandler;
      continue;
    }
    else
    {
      serviceString = paramIt.value();
    }

    if ( serviceString == "WCS" )
    {
      QgsWCSProjectParser* p = QgsConfigCache::instance()->wcsConfiguration( configFilePath );
      if ( !p )
      {
        //error handling
      }
      QgsWCSServer wcsServer( configFilePath, parameterMap, p, theRequestHandler );
      wcsServer.executeRequest();
    }
    else if ( serviceString == "WFS" )
    {
      QgsWFSProjectParser* p = QgsConfigCache::instance()->wfsConfiguration( configFilePath );
      if ( !p )
      {
        //error handling
      }
      QgsWFSServer wfsServer( configFilePath, parameterMap, p, theRequestHandler );
      wfsServer.executeRequest();
    }
    else    //WMS else
    {
      QgsWMSConfigParser* p = QgsConfigCache::instance()->wmsConfiguration( configFilePath, parameterMap );
      if ( !p )
      {
        //error handling
      }
      //adminConfigParser->loadLabelSettings( theMapRenderer->labelingEngine() );
      QgsWMSServer wmsServer( configFilePath, parameterMap, p, theRequestHandler, theMapRenderer, &capabilitiesCache );
      wmsServer.executeRequest();
    }
  }

  delete theMapRenderer;
  return 0;
}

