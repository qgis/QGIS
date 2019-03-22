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
#include "qgsrequesthandler.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgsmapserviceexception.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsserverlogger.h"
#include "qgsserverrequest.h"
#include "qgsfilterresponsedecorator.h"
#include "qgsservice.h"
#include "qgsserverparameters.h"
#include "qgsapplication.h"

#include <QDomDocument>
#include <QNetworkDiskCache>
#include <QSettings>
#include <QDateTime>

// TODO: remove, it's only needed by a single debug message
#include <fcgi_stdio.h>
#include <cstdlib>



// Server status static initializers.
// Default values are for C++, SIP bindings will override their
// options in in init()

QString *QgsServer::sConfigFilePath = nullptr;
QgsCapabilitiesCache *QgsServer::sCapabilitiesCache = nullptr;
QgsServerInterfaceImpl *QgsServer::sServerInterface = nullptr;
// Initialization must run once for all servers
bool QgsServer::sInitialized = false;
QgsServerSettings QgsServer::sSettings;

QgsServiceRegistry *QgsServer::sServiceRegistry = nullptr;

QgsServer::QgsServer()
{
  // QgsApplication must exist
  if ( qobject_cast<QgsApplication *>( qApp ) == nullptr )
  {
    qFatal( "A QgsApplication must exist before a QgsServer instance can be created." );
    abort();
  }
  init();
  mConfigCache = QgsConfigCache::instance();
}

QString &QgsServer::serverName()
{
  static QString *name = new QString( QStringLiteral( "qgis_server" ) );
  return *name;
}


QFileInfo QgsServer::defaultAdminSLD()
{
  return QFileInfo( QStringLiteral( "admin.sld" ) );
}

void QgsServer::setupNetworkAccessManager()
{
  QSettings settings;
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
  QNetworkDiskCache *cache = new QNetworkDiskCache( nullptr );
  qint64 cacheSize = sSettings.cacheSize();
  QString cacheDirectory = sSettings.cacheDirectory();
  cache->setCacheDirectory( cacheDirectory );
  cache->setMaximumCacheSize( cacheSize );
  QgsMessageLog::logMessage( QStringLiteral( "cacheDirectory: %1" ).arg( cache->cacheDirectory() ), QStringLiteral( "Server" ), Qgis::Info );
  QgsMessageLog::logMessage( QStringLiteral( "maximumCacheSize: %1" ).arg( cache->maximumCacheSize() ), QStringLiteral( "Server" ), Qgis::Info );
  nam->setCache( cache );
}

QFileInfo QgsServer::defaultProjectFile()
{
  QDir currentDir;
  fprintf( FCGI_stderr, "current directory: %s\n", currentDir.absolutePath().toUtf8().constData() );
  QStringList nameFilterList;
  nameFilterList << QStringLiteral( "*.qgs" )
                 << QStringLiteral( "*.qgz" );
  QFileInfoList projectFiles = currentDir.entryInfoList( nameFilterList, QDir::Files, QDir::Name );
  for ( int x = 0; x < projectFiles.size(); x++ )
  {
    QgsMessageLog::logMessage( projectFiles.at( x ).absoluteFilePath(), QStringLiteral( "Server" ), Qgis::Info );
  }
  if ( projectFiles.isEmpty() )
  {
    return QFileInfo();
  }
  return projectFiles.at( 0 );
}

void QgsServer::printRequestParameters( const QMap< QString, QString> &parameterMap, Qgis::MessageLevel logLevel )
{
  if ( logLevel > Qgis::Info )
  {
    return;
  }

  QMap< QString, QString>::const_iterator pIt = parameterMap.constBegin();
  for ( ; pIt != parameterMap.constEnd(); ++pIt )
  {
    QgsMessageLog::logMessage( pIt.key() + ":" + pIt.value(), QStringLiteral( "Server" ), Qgis::Info );
  }
}

QString QgsServer::configPath( const QString &defaultConfigPath, const QString &configPath )
{
  QString cfPath( defaultConfigPath );
  QString projectFile = sSettings.projectFile();
  if ( !projectFile.isEmpty() )
  {
    cfPath = projectFile;
    QgsDebugMsg( QStringLiteral( "QGIS_PROJECT_FILE:%1" ).arg( cfPath ) );
  }
  else
  {
    if ( configPath.isEmpty() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Using default configuration file path: %1" ).arg( defaultConfigPath ), QStringLiteral( "Server" ), Qgis::Info );
    }
    else
    {
      cfPath = configPath;
      QgsDebugMsg( QStringLiteral( "MAP:%1" ).arg( cfPath ) );
    }
  }
  return cfPath;
}

bool QgsServer::init()
{
  if ( sInitialized )
  {
    return false;
  }

  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( QgsApplication::QGIS_APPLICATION_NAME );

  QgsApplication::init();

#if defined(SERVER_SKIP_ECW)
  QgsMessageLog::logMessage( "Skipping GDAL ECW drivers in server.", "Server", Qgis::Info );
  QgsApplication::skipGdalDriver( "ECW" );
  QgsApplication::skipGdalDriver( "JP2ECW" );
#endif

  // reload settings to take into account QCoreApplication and QgsApplication
  // configuration
  sSettings.load();

  // init and configure logger
  QgsServerLogger::instance();
  QgsServerLogger::instance()->setLogLevel( sSettings.logLevel() );
  if ( ! sSettings.logFile().isEmpty() )
  {
    QgsServerLogger::instance()->setLogFile( sSettings.logFile() );
  }
  else if ( sSettings.logStderr() )
  {
    QgsServerLogger::instance()->setLogStderr();
  }

  // log settings currently used
  sSettings.logSummary();

  setupNetworkAccessManager();
  QDomImplementation::setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );
  QgsMessageLog::logMessage( "Prefix  PATH: " + QgsApplication::prefixPath(), QStringLiteral( "Server" ), Qgis::Info );
  QgsMessageLog::logMessage( "Plugin  PATH: " + QgsApplication::pluginPath(), QStringLiteral( "Server" ), Qgis::Info );
  QgsMessageLog::logMessage( "PkgData PATH: " + QgsApplication::pkgDataPath(), QStringLiteral( "Server" ), Qgis::Info );
  QgsMessageLog::logMessage( "User DB PATH: " + QgsApplication::qgisUserDatabaseFilePath(), QStringLiteral( "Server" ), Qgis::Info );
  QgsMessageLog::logMessage( "Auth DB PATH: " + QgsApplication::qgisAuthDatabaseFilePath(), QStringLiteral( "Server" ), Qgis::Info );
  QgsMessageLog::logMessage( "SVG PATHS: " + QgsApplication::svgPaths().join( QDir::separator() ), QStringLiteral( "Server" ), Qgis::Info );

  QgsApplication::createDatabase(); //init qgis.db (e.g. necessary for user crs)

  // Initialize the authentication system
  //   creates or uses qgis-auth.db in ~/.qgis3/ or directory defined by QGIS_AUTH_DB_DIR_PATH env variable
  //   set the master password as first line of file defined by QGIS_AUTH_PASSWORD_FILE env variable
  //   (QGIS_AUTH_PASSWORD_FILE variable removed from environment after accessing)
  QgsApplication::authManager()->init( QgsApplication::pluginPath(), QgsApplication::qgisAuthDatabaseFilePath() );

  QString defaultConfigFilePath;
  QFileInfo projectFileInfo = defaultProjectFile(); //try to find a .qgs/.qgz file in the server directory
  if ( projectFileInfo.exists() )
  {
    defaultConfigFilePath = projectFileInfo.absoluteFilePath();
    QgsMessageLog::logMessage( "Using default project file: " + defaultConfigFilePath, QStringLiteral( "Server" ), Qgis::Info );
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

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Roman" ) << QStringLiteral( "Bold" ) );

  sServiceRegistry = new QgsServiceRegistry();

  sServerInterface = new QgsServerInterfaceImpl( sCapabilitiesCache, sServiceRegistry, &sSettings );

  // Load service module
  QString modulePath = QgsApplication::libexecPath() + "server";
  qDebug() << "Initializing server modules from " << modulePath << endl;
  sServiceRegistry->init( modulePath,  sServerInterface );

  sInitialized = true;
  QgsMessageLog::logMessage( QStringLiteral( "Server initialized" ), QStringLiteral( "Server" ), Qgis::Info );
  return true;
}



void QgsServer::putenv( const QString &var, const QString &val )
{
#ifdef _MSC_VER
  _putenv_s( var.toStdString().c_str(), val.toStdString().c_str() );
#else
  setenv( var.toStdString().c_str(), val.toStdString().c_str(), 1 );
#endif
  sSettings.load( var );
}

void QgsServer::handleRequest( QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project )
{
  Qgis::MessageLevel logLevel = QgsServerLogger::instance()->logLevel();
  QTime time; //used for measuring request time if loglevel < 1

  qApp->processEvents();

  if ( logLevel == Qgis::Info )
  {
    time.start();
  }

  // Pass the filters to the requestHandler, this is needed for the following reasons:
  // Allow server request to call sendResponse plugin hook if enabled
  QgsFilterResponseDecorator responseDecorator( sServerInterface->filters(), response );

  //Request handler
  QgsRequestHandler requestHandler( request, response );

  try
  {
    // TODO: split parse input into plain parse and processing from specific services
    requestHandler.parseInput();
  }
  catch ( QgsMapServiceException &e )
  {
    QgsMessageLog::logMessage( "Parse input exception: " + e.message(), QStringLiteral( "Server" ), Qgis::Critical );
    requestHandler.setServiceException( e );
  }

  // Set the request handler into the interface for plugins to manipulate it
  sServerInterface->setRequestHandler( &requestHandler );

  // Call  requestReady() method (if enabled)
  responseDecorator.start();

  // Plugins may have set exceptions
  if ( !requestHandler.exceptionRaised() )
  {
    try
    {
      const QgsServerParameters params = request.serverParameters();
      printRequestParameters( params.toMap(), logLevel );

      //Config file path
      if ( ! project )
      {
        QString configFilePath = configPath( *sConfigFilePath, params.map() );

        // load the project if needed and not empty
        project = mConfigCache->project( configFilePath );
        if ( ! project )
        {
          throw QgsServerException( QStringLiteral( "Project file error" ) );
        }

        sServerInterface->setConfigFilePath( configFilePath );
      }
      else
      {
        sServerInterface->setConfigFilePath( project->fileName() );
      }

      if ( ! params.fileName().isEmpty() )
      {
        const QString value = QString( "attachment; filename=\"%1\"" ).arg( params.fileName() );
        requestHandler.setResponseHeader( QStringLiteral( "Content-Disposition" ), value );
      }

      // Lookup for service
      QgsService *service = sServiceRegistry->getService( params.service(), params.version() );
      if ( service )
      {
        service->executeRequest( request, responseDecorator, project );
      }
      else
      {
        throw QgsOgcServiceException( QStringLiteral( "Service configuration error" ),
                                      QStringLiteral( "Service unknown or unsupported" ) );
      }
    }
    catch ( QgsServerException &ex )
    {
      responseDecorator.write( ex );
      QString format;
      QgsMessageLog::logMessage( ex.formatResponse( format ), QStringLiteral( "Server" ), Qgis::Info );
    }
    catch ( QgsException &ex )
    {
      // Internal server error
      response.sendError( 500, QStringLiteral( "Internal Server Error" ) );
      QgsMessageLog::logMessage( ex.what(), QStringLiteral( "Server" ), Qgis::Critical );
    }
  }
  // Terminate the response
  responseDecorator.finish();

  // We are done using requestHandler in plugins, make sure we don't access
  // to a deleted request handler from Python bindings
  sServerInterface->clearRequestHandler();

  if ( logLevel == Qgis::Info )
  {
    QgsMessageLog::logMessage( "Request finished in " + QString::number( time.elapsed() ) + " ms", QStringLiteral( "Server" ), Qgis::Info );
  }
}


#ifdef HAVE_SERVER_PYTHON_PLUGINS
void QgsServer::initPython()
{
  // Init plugins
  if ( ! QgsServerPlugins::initPlugins( sServerInterface ) )
  {
    QgsMessageLog::logMessage( QStringLiteral( "No server python plugins are available" ), QStringLiteral( "Server" ), Qgis::Info );
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "Server python plugins loaded" ), QStringLiteral( "Server" ), Qgis::Info );
  }
}
#endif

