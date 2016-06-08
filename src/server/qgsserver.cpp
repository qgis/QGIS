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

#include "qgsauthmanager.h"
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
#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsaccesscontrolfilter.h"
#endif

#include <QDomDocument>
#include <QNetworkDiskCache>
#include <QImage>
#include <QSettings>
#include <QDateTime>
#include <QScopedPointer>
// TODO: remove, it's only needed by a single debug message
#include <fcgi_stdio.h>
#include <stdlib.h>



// Server status static initialisers.
// Default values are for C++, SIP bindings will override their
// options in in init()

QString* QgsServer::sConfigFilePath = nullptr;
QgsCapabilitiesCache* QgsServer::sCapabilitiesCache = nullptr;
QgsMapRenderer* QgsServer::sMapRenderer = nullptr;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
QgsServerInterfaceImpl*QgsServer::sServerInterface = nullptr;
bool QgsServer::sInitPython = true;
#endif
// Initialization must run once for all servers
bool QgsServer::sInitialised =  false;
char* QgsServer::sArgv[1];
int QgsServer::sArgc;
QgsApplication* QgsServer::sQgsApplication = nullptr;
bool QgsServer::sCaptureOutput = false;



QgsServer::QgsServer( int &argc, char **argv )
{
  init( argc, argv );
  saveEnvVars();
}


QgsServer::QgsServer()
{
  init();
  saveEnvVars();
}


QgsServer::~QgsServer()
{
}


QString& QgsServer::serverName()
{
  static QString* name = new QString( "qgis_server" );
  return *name;
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
  QNetworkDiskCache *cache = new QNetworkDiskCache( nullptr );
  QString cacheDirectory = settings.value( "cache/directory", QgsApplication::qgisSettingsDirPath() + "cache" ).toString();
  qint64 cacheSize = settings.value( "cache/size", 50 * 1024 * 1024 ).toULongLong();
  QgsMessageLog::logMessage( QString( "setCacheDirectory: %1" ).arg( cacheDirectory ), "Server", QgsMessageLog::INFO );
  QgsMessageLog::logMessage( QString( "setMaximumCacheSize: %1" ).arg( cacheSize ), "Server", QgsMessageLog::INFO );
  cache->setCacheDirectory( cacheDirectory );
  cache->setMaximumCacheSize( cacheSize );
  QgsMessageLog::logMessage( QString( "cacheDirectory: %1" ).arg( cache->cacheDirectory() ), "Server", QgsMessageLog::INFO );
  QgsMessageLog::logMessage( QString( "maximumCacheSize: %1" ).arg( cache->maximumCacheSize() ), "Server", QgsMessageLog::INFO );
  nam->setCache( cache );
}

/**
 * @brief QgsServer::createRequestHandler factory, creates a request instance
 * @param captureOutput
 * @return request instance
 */
QgsRequestHandler* QgsServer::createRequestHandler( const bool captureOutput )
{
  QgsRequestHandler* requestHandler = nullptr;
  char* requestMethod = getenv( "REQUEST_METHOD" );
  if ( requestMethod )
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
    QgsMessageLog::logMessage( projectFiles.at( x ).absoluteFilePath(), "Server", QgsMessageLog::INFO );
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
  if ( getenv( "REMOTE_ADDR" ) )
  {
    QgsMessageLog::logMessage( "remote ip: " + QString( getenv( "REMOTE_ADDR" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_HOST" ) )
  {
    QgsMessageLog::logMessage( "remote ip: " + QString( getenv( "REMOTE_HOST" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_USER" ) )
  {
    QgsMessageLog::logMessage( "remote user: " + QString( getenv( "REMOTE_USER" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_IDENT" ) )
  {
    QgsMessageLog::logMessage( "REMOTE_IDENT: " + QString( getenv( "REMOTE_IDENT" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "CONTENT_TYPE" ) )
  {
    QgsMessageLog::logMessage( "CONTENT_TYPE: " + QString( getenv( "CONTENT_TYPE" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "AUTH_TYPE" ) )
  {
    QgsMessageLog::logMessage( "AUTH_TYPE: " + QString( getenv( "AUTH_TYPE" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "HTTP_USER_AGENT" ) )
  {
    QgsMessageLog::logMessage( "HTTP_USER_AGENT: " + QString( getenv( "HTTP_USER_AGENT" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "HTTP_PROXY" ) )
  {
    QgsMessageLog::logMessage( "HTTP_PROXY: " + QString( getenv( "HTTP_PROXY" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "HTTPS_PROXY" ) )
  {
    QgsMessageLog::logMessage( "HTTPS_PROXY: " + QString( getenv( "HTTPS_PROXY" ) ), "Server", QgsMessageLog::INFO );
  }
  if ( getenv( "NO_PROXY" ) )
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
    QgsDebugMsg( QString( "QGIS_PROJECT_FILE:%1" ).arg( cfPath ) );
  }
  else
  {
    QMap<QString, QString>::const_iterator paramIt = parameters.find( "MAP" );
    if ( paramIt == parameters.constEnd() )
    {
      QgsMessageLog::logMessage( QString( "Using default configuration file path: %1" ).arg( defaultConfigPath ), "Server", QgsMessageLog::INFO );
    }
    else
    {
      cfPath = paramIt.value();
      QgsDebugMsg( QString( "MAP:%1" ).arg( cfPath ) );
    }
  }
  return cfPath;
}


/**
 * This is used in python bindings only
 */
bool QgsServer::init()
{
  if ( sInitialised )
  {
    return false;
  }

  sArgv[0] = serverName().toUtf8().data();
  sArgc = 1;
  sCaptureOutput = true;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  sInitPython = false;
#endif
  return init( sArgc , sArgv );
}


/**
 * Server initialization
 */
bool QgsServer::init( int & argc, char ** argv )
{
  if ( sInitialised )
  {
    return false;
  }

  QgsServerLogger::instance();

#ifndef _MSC_VER
  qInstallMsgHandler( dummyMessageHandler );
#endif

  QString optionsPath = getenv( "QGIS_OPTIONS_PATH" );
  if ( !optionsPath.isEmpty() )
  {
    QgsMessageLog::logMessage( "Options PATH: " + optionsPath, "Server", QgsMessageLog::INFO );
    QSettings::setDefaultFormat( QSettings::IniFormat );
    QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, optionsPath );
  }

  sQgsApplication = new QgsApplication( argc, argv, getenv( "DISPLAY" ), QString(), "server" );

  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( QgsApplication::QGIS_APPLICATION_NAME );

  //Default prefix path may be altered by environment variable
  QgsApplication::init();
#if !defined(Q_OS_WIN)
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::setPrefixPath( CMAKE_INSTALL_PREFIX, true );
#endif

#if defined(SERVER_SKIP_ECW)
  QgsMessageLog::logMessage( "Skipping GDAL ECW drivers in server.", "Server", QgsMessageLog::INFO );
  QgsApplication::skipGdalDriver( "ECW" );
  QgsApplication::skipGdalDriver( "JP2ECW" );
#endif

  setupNetworkAccessManager();
  QDomImplementation::setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );
  QgsMessageLog::logMessage( "Prefix  PATH: " + QgsApplication::prefixPath(), "Server", QgsMessageLog::INFO );
  QgsMessageLog::logMessage( "Plugin  PATH: " + QgsApplication::pluginPath(), "Server", QgsMessageLog::INFO );
  QgsMessageLog::logMessage( "PkgData PATH: " + QgsApplication::pkgDataPath(), "Server", QgsMessageLog::INFO );
  QgsMessageLog::logMessage( "User DB PATH: " + QgsApplication::qgisUserDbFilePath(), "Server", QgsMessageLog::INFO );
  QgsMessageLog::logMessage( "Auth DB PATH: " + QgsApplication::qgisAuthDbFilePath(), "Server", QgsMessageLog::INFO );
  QgsMessageLog::logMessage( "SVG PATHS: " + QgsApplication::svgPaths().join( QDir::separator() ), "Server", QgsMessageLog::INFO );

  QgsApplication::createDB(); //init qgis.db (e.g. necessary for user crs)

  // Instantiate authentication system
  //   creates or uses qgis-auth.db in ~/.qgis2/ or directory defined by QGIS_AUTH_DB_DIR_PATH env variable
  //   set the master password as first line of file defined by QGIS_AUTH_PASSWORD_FILE env variable
  //   (QGIS_AUTH_PASSWORD_FILE variable removed from environment after accessing)
  QgsAuthManager::instance()->init( QgsApplication::pluginPath() );

  QString defaultConfigFilePath;
  QFileInfo projectFileInfo = defaultProjectFile(); //try to find a .qgs file in the server directory
  if ( projectFileInfo.exists() )
  {
    defaultConfigFilePath = projectFileInfo.absoluteFilePath();
    QgsMessageLog::logMessage( "Using default project file: " + defaultConfigFilePath, "Server", QgsMessageLog::INFO );
  }
  else
  {
    QFileInfo adminSLDFileInfo = defaultAdminSLD();
    if ( adminSLDFileInfo.exists() )
    {
      defaultConfigFilePath = adminSLDFileInfo.absoluteFilePath();
    }
  }
  // Store the config file path
  sConfigFilePath = new QString( defaultConfigFilePath );


  //create cache for capabilities XML
  sCapabilitiesCache = new QgsCapabilitiesCache();
  sMapRenderer =  new QgsMapRenderer;
  sMapRenderer->setLabelingEngine( new QgsPalLabeling() );

#ifdef ENABLE_MS_TESTS
  QgsFontUtils::loadStandardTestFonts( QStringList() << "Roman" << "Bold" );
#endif

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  sServerInterface = new QgsServerInterfaceImpl( sCapabilitiesCache );
  if ( sInitPython )
  {
    // Init plugins
    if ( ! QgsServerPlugins::initPlugins( sServerInterface ) )
    {
      QgsMessageLog::logMessage( "No server python plugins are available", "Server", QgsMessageLog::INFO );
    }
    else
    {
      QgsMessageLog::logMessage( "Server python plugins loaded", "Server", QgsMessageLog::INFO );
    }
  }
#endif

  QgsEditorWidgetRegistry::initEditors();
  sInitialised = true;
  QgsMessageLog::logMessage( "Server initialized", "Server", QgsMessageLog::INFO );
  return true;
}

void QgsServer::putenv( const QString &var, const QString &val )
{
#ifdef _MSC_VER
  _putenv_s( var.toStdString().c_str(), val.toStdString().c_str() );
#else
  setenv( var.toStdString().c_str(), val.toStdString().c_str(), 1 );
#endif
}

/**
 * @brief Handles the request
 * @param queryString
 * @return response headers and body
 */
QPair<QByteArray, QByteArray> QgsServer::handleRequest( const QString& queryString )
{
  //apply environment variables
  QHash< QString, QString >::const_iterator envIt = mEnvironmentVariables.constBegin();
  for ( ; envIt != mEnvironmentVariables.constEnd(); ++envIt )
  {
    putenv( envIt.key(), envIt.value() );
  }

  /*
   * This is mainly for python bindings, passing QUERY_STRING
   * to handleRequest without using os.environment
   */
  if ( ! queryString.isEmpty() )
    putenv( "QUERY_STRING", queryString );

  int logLevel = QgsServerLogger::instance()->logLevel();
  QTime time; //used for measuring request time if loglevel < 1
  QgsMapLayerRegistry::instance()->removeAllMapLayers();

  // Clean up  Expression Context
  // because each call to QgsMapLayer::draw add items to QgsExpressionContext scope
  // list. This prevent the scope list to grow indefinitely and seriously deteriorate
  // performances and memory in the long run
  sMapRenderer->rendererContext()->setExpressionContext( QgsExpressionContext() );

  sQgsApplication->processEvents();
  if ( logLevel < 1 )
  {
    time.start();
    printRequestInfos();
  }

  //Request handler
  QScopedPointer<QgsRequestHandler> theRequestHandler( createRequestHandler( sCaptureOutput ) );

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
  sServerInterface->setRequestHandler( theRequestHandler.data() );
  // Iterate filters and call their requestReady() method
  QgsServerFiltersMap::const_iterator filtersIterator;
  QgsServerFiltersMap filters = sServerInterface->filters();
  for ( filtersIterator = filters.constBegin(); filtersIterator != filters.constEnd(); ++filtersIterator )
  {
    filtersIterator.value()->requestReady();
  }

  //Pass the filters to the requestHandler, this is needed for the following reasons:
  // 1. allow core services to access plugin filters and implement thir own plugin hooks
  // 2. allow requestHandler to call sendResponse plugin hook
  theRequestHandler->setPluginFilters( sServerInterface->filters() );
#endif

  // Copy the parameters map
  QMap<QString, QString> parameterMap( theRequestHandler->parameterMap() );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  const QgsAccessControl* accessControl = nullptr;
  accessControl = sServerInterface->accessControls();
#endif

  printRequestParameters( parameterMap, logLevel );
  QMap<QString, QString>::const_iterator paramIt;
  //Config file path
  QString configFilePath = configPath( *sConfigFilePath, parameterMap );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  sServerInterface->setConfigFilePath( configFilePath );
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
      QgsWCSProjectParser* p = QgsConfigCache::instance()->wcsConfiguration(
                                 configFilePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                                 , accessControl
#endif
                               );
      if ( !p )
      {
        theRequestHandler->setServiceException( QgsMapServiceException( "Project file error", "Error reading the project file" ) );
      }
      else
      {
        QgsWCSServer wcsServer(
          configFilePath
          , parameterMap
          , p
          , theRequestHandler.data()
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          , accessControl
#endif
        );
        wcsServer.executeRequest();
      }
    }
    else if ( serviceString == "WFS" )
    {
      QgsWFSProjectParser* p = QgsConfigCache::instance()->wfsConfiguration(
                                 configFilePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                                 , accessControl
#endif
                               );
      if ( !p )
      {
        theRequestHandler->setServiceException( QgsMapServiceException( "Project file error", "Error reading the project file" ) );
      }
      else
      {
        QgsWFSServer wfsServer(
          configFilePath
          , parameterMap
          , p
          , theRequestHandler.data()
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          , accessControl
#endif
        );
        wfsServer.executeRequest();
      }
    }
    else if ( serviceString == "WMS" )
    {
      QgsWMSConfigParser* p = QgsConfigCache::instance()->wmsConfiguration(
                                configFilePath
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                                , accessControl
#endif
                              );
      if ( !p )
      {
        theRequestHandler->setServiceException( QgsMapServiceException( "WMS configuration error", "There was an error reading the project file or the SLD configuration" ) );
      }
      else
      {
        QgsWMSServer wmsServer(
          configFilePath
          , parameterMap
          , p
          , theRequestHandler.data()
          , sMapRenderer
          , sCapabilitiesCache
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          , accessControl
#endif
        );
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
  filters = sServerInterface->filters();
  for ( filtersIterator = filters.constBegin(); filtersIterator != filters.constEnd(); ++filtersIterator )
  {
    filtersIterator.value()->responseComplete();
  }
  // We are done using theRequestHandler in plugins, make sure we don't access
  // to a deleted request handler from Python bindings
  sServerInterface->clearRequestHandler();
#endif

  theRequestHandler->sendResponse();

  if ( logLevel < 1 )
  {
    QgsMessageLog::logMessage( "Request finished in " + QString::number( time.elapsed() ) + " ms", "Server", QgsMessageLog::INFO );
  }
  // Returns the header and response bytestreams (to be used in Python bindings)
  return theRequestHandler->getResponse();
}

#if 0
// The following code was used to test type conversion in python bindings
QPair<QByteArray, QByteArray> QgsServer::testQPair( QPair<QByteArray, QByteArray> pair )
{
  return pair;
}
#endif

void QgsServer::saveEnvVars()
{
  saveEnvVar( "MAX_CACHE_LAYERS" );
  saveEnvVar( "DEFAULT_DATUM_TRANSFORM" );
}

void QgsServer::saveEnvVar( const QString& variableName )
{
  const char* env = getenv( variableName.toLocal8Bit() );
  if ( !env )
  {
    return;
  }

  mEnvironmentVariables.insert( variableName, QString::fromLocal8Bit( env ) );
}

