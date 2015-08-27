/***************************************************************************
                              qgsserver.cpp
 A server application supporting WMS / WFS / WCS
                              -------------------
  begin                : July 04, 2006
  copyright            : (C) 2006 by Marco Hugentobler & Ionut Iosifescu Enescu
                       : (C) 2015 by Alessandro Pasotti
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                       : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//for CMAKE_INSTALL_PREFIX
#include "qgsconfig.h"
#include "qgsserver.h"

#include "qgscapabilitiescache.h"
#include "qgsfontutils.h"
#include "qgsgetrequesthandler.h"
#include "qgspostrequesthandler.h"
#include "qgssoaprequesthandler.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgswmsserver.h"
#include "qgswfsserver.h"
#include "qgswcsserver.h"
#include "qgsmapserviceexception.h"
#include "qgspallabeling.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmaplayerregistry.h"
#include "qgsserverlogger.h"
#include "qgseditorwidgetregistry.h"

#include <QDomDocument>
#include <QNetworkDiskCache>
#include <QImage>
#include <QSettings>
#include <QDateTime>
#include <QScopedPointer>
// TODO: remove, it's only needed by a single debug message
#include <fcgi_stdio.h>
#include <stdlib.h>


// Static initialisers, default values for fcgi server
QgsApplication* QgsServer::mQgsApplication = NULL;
bool QgsServer::mInitialised = FALSE;
QString QgsServer::mServerName( "qgis_server" );
bool QgsServer::mCaptureOutput = FALSE;
char* QgsServer::mArgv[1];
int QgsServer::mArgc = 1;
QString QgsServer::mConfigFilePath;
QgsMapRenderer* QgsServer::mMapRenderer = NULL;
QgsCapabilitiesCache* QgsServer::mCapabilitiesCache;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
bool QgsServer::mInitPython = TRUE;
QgsServerInterfaceImpl* QgsServer::mServerInterface = NULL;
#endif


QgsServer::QgsServer( )
{
}


QgsServer::~QgsServer( )
{
}


QFileInfo QgsServer::defaultAdminSLD()
{
  return QFileInfo( "admin.sld" );
}


/**
 * @brief QgsServer::setupNetworkAccessManager
 */
void QgsServer::setupNetworkAccessManager()
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

/**
 * @brief QgsServer::createRequestHandler factory, creates a request instance
 * @param captureOutput
 * @return request instance
 */
QgsRequestHandler* QgsServer::createRequestHandler( const bool captureOutput /*= FALSE*/ )
{
  QgsRequestHandler* requestHandler = 0;
  char* requestMethod = getenv( "REQUEST_METHOD" );
  if ( requestMethod != NULL )
  {
    if ( strcmp( requestMethod, "POST" ) == 0 )
    {
      //requestHandler = new QgsSOAPRequestHandler();
      requestHandler = new QgsPostRequestHandler( captureOutput );
    }
    else
    {
      requestHandler = new QgsGetRequestHandler( captureOutput );
    }
  }
  else
  {
    requestHandler = new QgsGetRequestHandler( captureOutput );
  }
  return requestHandler;
}

/**
 * @brief QgsServer::defaultProjectFile
 * @return the default project file
 */
QFileInfo QgsServer::defaultProjectFile()
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

/**
 * @brief QgsServer::printRequestParameters prints the request parameters
 * @param parameterMap
 * @param logLevel
 */
void QgsServer::printRequestParameters( const QMap< QString, QString>& parameterMap, int logLevel )
{
  if ( logLevel > 0 )
  {
    return;
  }

  QMap< QString, QString>::const_iterator pIt = parameterMap.constBegin();
  for ( ; pIt != parameterMap.constEnd(); ++pIt )
  {
    QgsMessageLog::logMessage( pIt.key() + ":" + pIt.value(), "Server", QgsMessageLog::INFO );
  }
}

/**
 * @brief QgsServer::printRequestInfos prints debug information about the request
 */
void QgsServer::printRequestInfos()
{
  QgsMessageLog::logMessage( "********************new request***************", "Server", QgsMessageLog::INFO );
  if ( getenv( "REMOTE_ADDR" ) != NULL )
  {
    QgsMessageLog::logMessage( "remote ip: " + QString( getenv( "REMOTE_ADDR" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_HOST" ) != NULL )
  {
    QgsMessageLog::logMessage( "remote ip: " + QString( getenv( "REMOTE_HOST" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_USER" ) != NULL )
  {
    QgsMessageLog::logMessage( "remote user: " + QString( getenv( "REMOTE_USER" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_IDENT" ) != NULL )
  {
    QgsMessageLog::logMessage( "REMOTE_IDENT: " + QString( getenv( "REMOTE_IDENT" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "CONTENT_TYPE" ) != NULL )
  {
    QgsMessageLog::logMessage( "CONTENT_TYPE: " + QString( getenv( "CONTENT_TYPE" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "AUTH_TYPE" ) != NULL )
  {
    QgsMessageLog::logMessage( "AUTH_TYPE: " + QString( getenv( "AUTH_TYPE" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "HTTP_USER_AGENT" ) != NULL )
  {
    QgsMessageLog::logMessage( "HTTP_USER_AGENT: " + QString( getenv( "HTTP_USER_AGENT" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "HTTP_PROXY" ) != NULL )
  {
    QgsMessageLog::logMessage( "HTTP_PROXY: " + QString( getenv( "HTTP_PROXY" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "HTTPS_PROXY" ) != NULL )
  {
    QgsMessageLog::logMessage( "HTTPS_PROXY: " + QString( getenv( "HTTPS_PROXY" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "NO_PROXY" ) != NULL )
  {
    QgsMessageLog::logMessage( "NO_PROXY: " + QString( getenv( "NO_PROXY" ) ), "Server", QgsMessageLog::INFO );
  }
}

void QgsServer::dummyMessageHandler( QtMsgType type, const char *msg )
{
#if 0 //def QGSMSDEBUG
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

/**
 * @brief QgsServer::configPath
 * @param defaultConfigPath
 * @param parameters
 * @return config file path
 */
QString QgsServer::configPath( const QString& defaultConfigPath, const QMap<QString, QString>& parameters )
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


/**
 * This is used in python bindings only
 */
bool QgsServer::init()
{
  if ( mInitialised )
  {
    return FALSE;
  }
  mArgv[0] = mServerName.toUtf8( ).data( );
  mArgc = 1;
  mCaptureOutput = TRUE;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  mInitPython = FALSE;
#endif
  return init( mArgc , mArgv );
}


/**
 * Server initialization
 */
bool QgsServer::init( int & argc, char ** argv )
{
  if ( mInitialised )
  {
    return FALSE;
  }

#ifndef _MSC_VER
  qInstallMsgHandler( dummyMessageHandler );
#endif

  QString optionsPath = getenv( "QGIS_OPTIONS_PATH" );
  if ( !optionsPath.isEmpty() )
  {
    QgsDebugMsg( "Options PATH: " + optionsPath );
    QSettings::setDefaultFormat( QSettings::IniFormat );
    QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, optionsPath );
  }

  mQgsApplication = new QgsApplication( argc, argv, getenv( "DISPLAY" ) );

  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( QgsApplication::QGIS_APPLICATION_NAME );

  //Default prefix path may be altered by environment variable
  QgsApplication::init();
#if !defined(Q_OS_WIN)
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::setPrefixPath( CMAKE_INSTALL_PREFIX, TRUE );
#endif

#if defined(SERVER_SKIP_ECW)
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
  QgsDebugMsg( "SVG PATHS: " + QgsApplication::svgPaths().join( ":" ) );

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
  mCapabilitiesCache = new QgsCapabilitiesCache();
  mMapRenderer =  new QgsMapRenderer;
  mMapRenderer->setLabelingEngine( new QgsPalLabeling() );

#ifdef ENABLE_MS_TESTS
  QgsFontUtils::loadStandardTestFonts( QStringList() << "Roman" << "Bold" );
#endif

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  mServerInterface = new QgsServerInterfaceImpl( mCapabilitiesCache );
  if ( mInitPython )
  {
    // Init plugins
    if ( ! QgsServerPlugins::initPlugins( mServerInterface ) )
    {
      QgsMessageLog::logMessage( "No server python plugins are available", "Server", QgsMessageLog::INFO );
    }
    else
    {
      QgsMessageLog::logMessage( "Server python plugins loaded", "Server", QgsMessageLog::INFO );
    }
  }
#endif

  QgsServerLogger::instance();

  QgsEditorWidgetRegistry::initEditors();
  mInitialised = TRUE;
  QgsMessageLog::logMessage( "Server intialised", "Server", QgsMessageLog::INFO );
  return TRUE;
}


/**
 * Handles the request
 */
QByteArray QgsServer::handleRequest( const QString queryString /*= QString( )*/ )
{
  return handleRequest( queryString, TRUE, TRUE );
}

/**
 * @brief Handles the request, returning only the body
 * @param queryString
 * @return response body if mCaptureOutput is set, empty QByteArray if not
 */
QByteArray QgsServer::handleRequestGetBody( const QString queryString /*= QString( )*/ )
{
  return handleRequest( queryString, FALSE, TRUE );
}

/**
 * @brief Handles the request, returning only the headers
 * @param queryString
 * @return response headers if mCaptureOutput is set, empty QByteArray if not
 */
QByteArray QgsServer::handleRequestGetHeaders( const QString queryString /*= QString( )*/ )
{
  return handleRequest( queryString, TRUE, FALSE );
}

/**
 * @brief Handles the request
 * @param queryString
 * @param returnBody
 * @param returnHeaders
 * @return response body and headers if mCaptureOutput is set and the
 * flags are set, empty QByteArray if not
 */
QByteArray QgsServer::handleRequest( const QString queryString ,
                                     bool returnHeaders,
                                     bool returnBody )
{

  // Run init if handleRequest was called without previously initialising
  // the server
  if ( ! mInitialised )
  {
    init( );
  }

  /*
   * This is mainly for python bindings, passing QUERY_STRING
   * to handleRequest without using os.environment
   */
  if ( ! queryString.isEmpty() )
  {
#ifdef _MSC_VER
    _putenv_s( "QUERY_STRING", queryString.toUtf8().data() );
#else
    setenv( "QUERY_STRING", queryString.toUtf8().data(), 1 );
#endif
  }

  int logLevel = QgsServerLogger::instance()->logLevel();
  QTime time; //used for measuring request time if loglevel < 1
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  mQgsApplication->processEvents();
  if ( logLevel < 1 )
  {
    time.start();
    printRequestInfos();
  }

  //Request handler
  QScopedPointer<QgsRequestHandler> theRequestHandler( createRequestHandler( mCaptureOutput ) );

  try
  {
    // TODO: split parse input into plain parse and processing from specific services
    theRequestHandler->parseInput();
  }
  catch ( QgsMapServiceException& e )
  {
    QgsMessageLog::logMessage( "Parse input exception: " + e.message(), "Server", QgsMessageLog::CRITICAL );
    theRequestHandler->setServiceException( e );
  }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  // Set the request handler into the interface for plugins to manipulate it
  mServerInterface->setRequestHandler( theRequestHandler.data() );
  // Iterate filters and call their requestReady() method
  QgsServerFiltersMap::const_iterator filtersIterator;
  for ( filtersIterator = mServerInterface->filters().constBegin(); filtersIterator != mServerInterface->filters().constEnd(); ++filtersIterator )
  {
    filtersIterator.value()->requestReady();
  }

  //Pass the filters to the requestHandler, this is needed for the following reasons:
  // 1. allow core services to access plugin filters and implement thir own plugin hooks
  // 2. allow requestHandler to call sendResponse plugin hook

  theRequestHandler->setPluginFilters( mServerInterface->filters() );
#endif

  // Copy the parameters map
  QMap<QString, QString> parameterMap( theRequestHandler->parameterMap() );

  printRequestParameters( parameterMap, logLevel );
  QMap<QString, QString>::const_iterator paramIt;
  //Config file path
  mConfigFilePath = configPath( mConfigFilePath, parameterMap );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  mServerInterface->setConfigFilePath( mConfigFilePath );
#endif
  //Service parameter
  QString serviceString = theRequestHandler->parameter( "SERVICE" );

  if ( serviceString.isEmpty() )
  {
    // SERVICE not mandatory for WMS 1.3.0 GetMap & GetFeatureInfo
    QString requestString = theRequestHandler->parameter( "REQUEST" );
    if ( requestString == "GetMap" || requestString == "GetFeatureInfo" )
    {
      serviceString = "WMS";
    }
  }

  //possibility for client to suggest a download filename
  QString outputFileName = theRequestHandler->parameter( "FILE_NAME" );
  if ( !outputFileName.isEmpty() )
  {
    theRequestHandler->setDefaultHeaders();
    theRequestHandler->setHeader( "Content-Disposition", "attachment; filename=\"" + outputFileName + "\"" );
  }

  // Enter core services main switch
  if ( !theRequestHandler->exceptionRaised() )
  {
    if ( serviceString == "WCS" )
    {
      QgsWCSProjectParser* p = QgsConfigCache::instance()->wcsConfiguration( mConfigFilePath );
      if ( !p )
      {
        theRequestHandler->setServiceException( QgsMapServiceException( "Project file error", "Error reading the project file" ) );
      }
      else
      {
        QgsWCSServer wcsServer( mConfigFilePath, parameterMap, p, theRequestHandler.data() );
        wcsServer.executeRequest();
      }
    }
    else if ( serviceString == "WFS" )
    {
      QgsWFSProjectParser* p = QgsConfigCache::instance()->wfsConfiguration( mConfigFilePath );
      if ( !p )
      {
        theRequestHandler->setServiceException( QgsMapServiceException( "Project file error", "Error reading the project file" ) );
      }
      else
      {
        QgsWFSServer wfsServer( mConfigFilePath, parameterMap, p, theRequestHandler.data() );
        wfsServer.executeRequest();
      }
    }
    else if ( serviceString == "WMS" )
    {
      QgsWMSConfigParser* p = QgsConfigCache::instance()->wmsConfiguration( mConfigFilePath, parameterMap );
      if ( !p )
      {
        theRequestHandler->setServiceException( QgsMapServiceException( "WMS configuration error", "There was an error reading the project file or the SLD configuration" ) );
      }
      else
      {
        QgsWMSServer wmsServer( mConfigFilePath, parameterMap, p, theRequestHandler.data(), mMapRenderer, mCapabilitiesCache );
        wmsServer.executeRequest();
      }
    }
    else
    {
      theRequestHandler->setServiceException( QgsMapServiceException( "Service configuration error", "Service unknown or unsupported" ) );
    } // end switch
  } // end if not exception raised

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  // Iterate filters and call their responseComplete() method
  for ( filtersIterator = mServerInterface->filters().constBegin(); filtersIterator != mServerInterface->filters().constEnd(); ++filtersIterator )
  {
    filtersIterator.value()->responseComplete();
  }
  // We are done using theRequestHandler in plugins, make sure we don't access
  // to a deleted request handler from Python bindings
  mServerInterface->clearRequestHandler( );
#endif

  theRequestHandler->sendResponse();

  if ( logLevel < 1 )
  {
    QgsMessageLog::logMessage( "Request finished in " + QString::number( time.elapsed() ) + " ms", "Server", QgsMessageLog::INFO );
  }
  // TODO: if HAVE_SERVER_PYTHON
  // Returns the response bytestream
  return theRequestHandler->getResponse( returnHeaders , returnBody );
}

