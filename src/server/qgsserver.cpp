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
#include "qgsversion.h"
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
#include "qgsserverapi.h"
#include "qgsserverapicontext.h"
#include "qgsserverparameters.h"
#include "qgsapplication.h"
#include "qgsruntimeprofiler.h"
#include "qgscoordinatetransform.h"

#include <QDomDocument>
#include <QNetworkDiskCache>
#include <QSettings>
#include <QElapsedTimer>

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

QgsServiceRegistry *QgsServer::sServiceRegistry = nullptr;

Q_GLOBAL_STATIC( QgsServerSettings, sSettings );

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

QFileInfo QgsServer::defaultAdminSLD()
{
  return QFileInfo( QStringLiteral( "admin.sld" ) );
}

void QgsServer::setupNetworkAccessManager()
{
  const QSettings settings;
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
  QNetworkDiskCache *cache = new QNetworkDiskCache( nullptr );
  const qint64 cacheSize = sSettings()->cacheSize();
  const QString cacheDirectory = sSettings()->cacheDirectory();
  cache->setCacheDirectory( cacheDirectory );
  cache->setMaximumCacheSize( cacheSize );
  QgsMessageLog::logMessage( QStringLiteral( "cacheDirectory: %1" ).arg( cache->cacheDirectory() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  QgsMessageLog::logMessage( QStringLiteral( "maximumCacheSize: %1" ).arg( cache->maximumCacheSize() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  nam->setCache( cache );
}

QFileInfo QgsServer::defaultProjectFile()
{
  const QDir currentDir;
  fprintf( FCGI_stderr, "current directory: %s\n", currentDir.absolutePath().toUtf8().constData() );
  QStringList nameFilterList;
  nameFilterList << QStringLiteral( "*.qgs" )
                 << QStringLiteral( "*.qgz" );
  const QFileInfoList projectFiles = currentDir.entryInfoList( nameFilterList, QDir::Files, QDir::Name );
  for ( int x = 0; x < projectFiles.size(); x++ )
  {
    QgsMessageLog::logMessage( projectFiles.at( x ).absoluteFilePath(), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }
  if ( projectFiles.isEmpty() )
  {
    return QFileInfo();
  }
  return projectFiles.at( 0 );
}

void QgsServer::printRequestParameters( const QMap< QString, QString> &parameterMap, Qgis::MessageLevel logLevel )
{
  if ( logLevel > Qgis::MessageLevel::Info )
  {
    return;
  }

  QMap< QString, QString>::const_iterator pIt = parameterMap.constBegin();
  for ( ; pIt != parameterMap.constEnd(); ++pIt )
  {
    QgsMessageLog::logMessage( pIt.key() + ":" + pIt.value(), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }
}

QString QgsServer::configPath( const QString &defaultConfigPath, const QString &configPath )
{
  QString cfPath( defaultConfigPath );
  const QString projectFile = sSettings()->projectFile();
  if ( !projectFile.isEmpty() )
  {
    cfPath = projectFile;
    QgsDebugMsg( QStringLiteral( "QGIS_PROJECT_FILE:%1" ).arg( cfPath ) );
  }
  else
  {
    if ( configPath.isEmpty() )
    {
      // Read it from the environment, because a rewrite rule may have rewritten it
      if ( getenv( "QGIS_PROJECT_FILE" ) )
      {
        cfPath = getenv( "QGIS_PROJECT_FILE" );
        QgsMessageLog::logMessage( QStringLiteral( "Using configuration file path from environment: %1" ).arg( cfPath ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
      }
      else  if ( ! defaultConfigPath.isEmpty() )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Using default configuration file path: %1" ).arg( defaultConfigPath ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
      }
    }
    else
    {
      cfPath = configPath;
      QgsDebugMsg( QStringLiteral( "MAP:%1" ).arg( cfPath ) );
    }
  }
  return cfPath;
}

void QgsServer::initLocale()
{
  // System locale override
  if ( ! sSettings()->overrideSystemLocale().isEmpty() )
  {
    QLocale::setDefault( QLocale( sSettings()->overrideSystemLocale() ) );
  }
  // Number group separator settings
  QLocale currentLocale;
  if ( sSettings()->showGroupSeparator() )
  {
    currentLocale.setNumberOptions( currentLocale.numberOptions() &= ~QLocale::NumberOption::OmitGroupSeparator );
  }
  else
  {
    currentLocale.setNumberOptions( currentLocale.numberOptions() |= QLocale::NumberOption::OmitGroupSeparator );
  }
  QLocale::setDefault( currentLocale );
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

  // TODO: remove QGIS_OPTIONS_PATH from settings and rely on QgsApplication's env var QGIS_CUSTOM_CONFIG_PATH
  //       Note that QGIS_CUSTOM_CONFIG_PATH gives /tmp/qt_temp-rUpsId/profiles/default/QGIS/QGIS3.ini
  //       while     QGIS_OPTIONS_PATH gives       /tmp/qt_temp-rUpsId/QGIS/QGIS3.ini
  QgsApplication::init( qgetenv( "QGIS_OPTIONS_PATH" ) );

#if defined(SERVER_SKIP_ECW)
  QgsMessageLog::logMessage( "Skipping GDAL ECW drivers in server.", "Server", Qgis::MessageLevel::Info );
  QgsApplication::skipGdalDriver( "ECW" );
  QgsApplication::skipGdalDriver( "JP2ECW" );
#endif

  // reload settings to take into account QCoreApplication and QgsApplication
  // configuration
  sSettings()->load();

  // init and configure logger
  QgsServerLogger::instance();
  QgsServerLogger::instance()->setLogLevel( sSettings()->logLevel() );
  if ( ! sSettings()->logFile().isEmpty() )
  {
    QgsServerLogger::instance()->setLogFile( sSettings()->logFile() );
  }
  else if ( sSettings()->logStderr() )
  {
    QgsServerLogger::instance()->setLogStderr();
  }

  // Logging handlers for CRS grid issues
  QgsCoordinateTransform::setCustomMissingRequiredGridHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs,
      const QgsDatumTransform::GridDetails & grid )
  {
    QgsServerLogger::instance()->logMessage( QStringLiteral( "Cannot use project transform between %1 and %2 - missing grid %3" )
        .arg( sourceCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ),
              destinationCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ),
              grid.shortName ),
        QStringLiteral( "QGIS Server" ), Qgis::MessageLevel::Warning );
  } );


  QgsCoordinateTransform::setCustomMissingGridUsedByContextHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs,
      const QgsDatumTransform::TransformDetails & details )
  {
    QString gridMessage;
    for ( const QgsDatumTransform::GridDetails &grid : details.grids )
    {
      if ( !grid.isAvailable )
      {
        gridMessage.append( QStringLiteral( "This transformation requires the grid file '%1', which is not available for use on the system.\n" ).arg( grid.shortName ) );
      }
    }
    QgsServerLogger::instance()->logMessage( QStringLiteral( "Cannot use project transform between %1 and %2 - %3.\n%4" )
        .arg( sourceCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ),
              destinationCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ),
              details.name,
              gridMessage ),
        QStringLiteral( "QGIS Server" ), Qgis::MessageLevel::Warning );
  } );


  QgsCoordinateTransform::setCustomMissingPreferredGridHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs,
      const QgsCoordinateReferenceSystem & destinationCrs,
      const QgsDatumTransform::TransformDetails & preferredOperation,
      const QgsDatumTransform::TransformDetails & availableOperation )
  {

    QString gridMessage;
    for ( const QgsDatumTransform::GridDetails &grid : preferredOperation.grids )
    {
      if ( !grid.isAvailable )
      {
        gridMessage.append( QStringLiteral( "This transformation requires the grid file '%1', which is not available for use on the system.\n" ).arg( grid.shortName ) );
        if ( !grid.url.isEmpty() )
        {
          if ( !grid.packageName.isEmpty() )
          {
            gridMessage.append( QStringLiteral( "This grid is part of the '%1' package, available for download from %2.\n" ).arg( grid.packageName, grid.url ) );
          }
          else
          {
            gridMessage.append( QStringLiteral( "This grid is available for download from %1.\n" ).arg( grid.url ) );
          }
        }
      }
    }

    QString accuracyMessage;
    if ( availableOperation.accuracy >= 0 && preferredOperation.accuracy >= 0 )
      accuracyMessage = QStringLiteral( "Current transform '%1' has an accuracy of %2 meters, while the preferred transformation '%3' has accuracy %4 meters.\n" ).arg( availableOperation.name )
                        .arg( availableOperation.accuracy ).arg( preferredOperation.name ).arg( preferredOperation.accuracy );
    else if ( preferredOperation.accuracy >= 0 )
      accuracyMessage = QStringLiteral( "Current transform '%1' has an unknown accuracy, while the preferred transformation '%2' has accuracy %3 meters.\n" )
                        .arg( availableOperation.name, preferredOperation.name )
                        .arg( preferredOperation.accuracy );

    const QString longMessage = QStringLiteral( "The preferred transform between '%1' and '%2' is not available for use on the system.\n" ).arg( sourceCrs.userFriendlyIdentifier(),
                                destinationCrs.userFriendlyIdentifier() )
                                + gridMessage + accuracyMessage;

    QgsServerLogger::instance()->logMessage( longMessage, QStringLiteral( "QGIS Server" ), Qgis::MessageLevel::Warning );

  } );

  QgsCoordinateTransform::setCustomCoordinateOperationCreationErrorHandler( [ = ]( const QgsCoordinateReferenceSystem & sourceCrs, const QgsCoordinateReferenceSystem & destinationCrs, const QString & error )
  {
    const QString longMessage = QStringLiteral( "No transform is available between %1 and %2: %3" )
                                .arg( sourceCrs.userFriendlyIdentifier(), destinationCrs.userFriendlyIdentifier(), error );
    QgsServerLogger::instance()->logMessage( longMessage, QStringLiteral( "QGIS Server" ), Qgis::MessageLevel::Warning );
  } );

  // Configure locale
  initLocale();

  QgsMessageLog::logMessage( QStringLiteral( "QGIS Server Starting : %1 (%2)" ).arg( _QGIS_VERSION, QGSVERSION ), "Server", Qgis::MessageLevel::Info );

  // log settings currently used
  sSettings()->logSummary();

  setupNetworkAccessManager();
  QDomImplementation::setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );
  QgsMessageLog::logMessage( "Prefix  PATH: " + QgsApplication::prefixPath(), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  QgsMessageLog::logMessage( "Plugin  PATH: " + QgsApplication::pluginPath(), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  QgsMessageLog::logMessage( "PkgData PATH: " + QgsApplication::pkgDataPath(), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  QgsMessageLog::logMessage( "User DB PATH: " + QgsApplication::qgisUserDatabaseFilePath(), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  QgsMessageLog::logMessage( "Auth DB PATH: " + QgsApplication::qgisAuthDatabaseFilePath(), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  QgsMessageLog::logMessage( "SVG PATHS: " + QgsApplication::svgPaths().join( QDir::listSeparator() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );

  QgsApplication::createDatabase(); //init qgis.db (e.g. necessary for user crs)

  // Initialize the authentication system
  //   creates or uses qgis-auth.db in ~/.qgis3/ or directory defined by QGIS_AUTH_DB_DIR_PATH env variable
  //   set the master password as first line of file defined by QGIS_AUTH_PASSWORD_FILE env variable
  //   (QGIS_AUTH_PASSWORD_FILE variable removed from environment after accessing)
  QgsApplication::authManager()->init( QgsApplication::pluginPath(), QgsApplication::qgisAuthDatabaseFilePath() );

  QString defaultConfigFilePath;
  const QFileInfo projectFileInfo = defaultProjectFile(); //try to find a .qgs/.qgz file in the server directory
  if ( projectFileInfo.exists() )
  {
    defaultConfigFilePath = projectFileInfo.absoluteFilePath();
    QgsMessageLog::logMessage( "Using default project file: " + defaultConfigFilePath, QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }
  else
  {
    const QFileInfo adminSLDFileInfo = defaultAdminSLD();
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

  sServerInterface = new QgsServerInterfaceImpl( sCapabilitiesCache, sServiceRegistry, sSettings() );

  // Load service module
  const QString modulePath = QgsApplication::libexecPath() + "server";
  // qDebug() << QStringLiteral( "Initializing server modules from: %1" ).arg( modulePath );
  sServiceRegistry->init( modulePath,  sServerInterface );

  sInitialized = true;
  QgsMessageLog::logMessage( QStringLiteral( "Server initialized" ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  return true;
}



void QgsServer::putenv( const QString &var, const QString &val )
{
  if ( val.isEmpty() )
  {
    qunsetenv( var.toUtf8().data() );
  }
  else
  {
    qputenv( var.toUtf8().data(), val.toUtf8() );
  }
  sSettings()->load( var );
}

void QgsServer::handleRequest( QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project )
{
  const Qgis::MessageLevel logLevel = QgsServerLogger::instance()->logLevel();
  {

    const QgsScopedRuntimeProfile profiler { QStringLiteral( "handleRequest" ), QStringLiteral( "server" ) };

    qApp->processEvents();

    response.clear();

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
      QgsMessageLog::logMessage( "Parse input exception: " + e.message(), QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
      requestHandler.setServiceException( e );
    }

    // Set the request handler into the interface for plugins to manipulate it
    sServerInterface->setRequestHandler( &requestHandler );

    // Initialize configfilepath so that is is available
    // before calling plugin methods
    // Note that plugins may still change that value using
    // setConfigFilePath() interface method
    if ( ! project )
    {
      const QString configFilePath = configPath( *sConfigFilePath, request.serverParameters().map() );
      sServerInterface->setConfigFilePath( configFilePath );
    }
    else
    {
      sServerInterface->setConfigFilePath( project->fileName() );
    }

    // Call  requestReady() method (if enabled)
    // This may also throw exceptions if there are errors in python plugins code
    try
    {
      responseDecorator.start();
    }
    catch ( QgsException &ex )
    {
      // Internal server error
      response.sendError( 500, QStringLiteral( "Internal Server Error" ) );
      QgsMessageLog::logMessage( ex.what(), QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    }

    // Plugins may have set exceptions
    if ( !requestHandler.exceptionRaised() )
    {
      try
      {
        const QgsServerParameters params = request.serverParameters();
        printRequestParameters( params.toMap(), logLevel );

        // Setup project (config file path)
        if ( ! project )
        {
          const QString configFilePath = configPath( *sConfigFilePath, params.map() );

          // load the project if needed and not empty
          if ( ! configFilePath.isEmpty() )
          {
            project = mConfigCache->project( configFilePath, sServerInterface->serverSettings() );
          }
        }

        // Set the current project instance
        QgsProject::setInstance( const_cast<QgsProject *>( project ) );

        if ( project )
        {
          sServerInterface->setConfigFilePath( project->fileName() );
        }
        else
        {
          sServerInterface->setConfigFilePath( QString() );
        }

        // Note that at this point we still might not have set a valid project.
        // There are APIs that work without a project (e.g. the landing page catalog API that
        // lists the available projects metadata).

        // Dispatcher: if SERVICE is set, we assume a OWS service, if not, let's try an API
        // TODO: QGIS 4 fix the OWS services and treat them as APIs
        QgsServerApi *api = nullptr;
        if ( params.service().isEmpty() && ( api = sServiceRegistry->apiForRequest( request ) ) )
        {
          const QgsServerApiContext context { api->rootPath(), &request, &responseDecorator, project, sServerInterface };
          api->executeRequest( context );
        }
        else
        {

          // Project is mandatory for OWS at this point
          if ( ! project )
          {
            throw QgsServerException( QStringLiteral( "Project file error. For OWS services: please provide a SERVICE and a MAP parameter pointing to a valid QGIS project file" ) );
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
                                          QStringLiteral( "Service unknown or unsupported. Current supported services (case-sensitive): WMS WFS WCS WMTS SampleService, or use a WFS3 (OGC API Features) endpoint" ) );
          }
        }
      }
      catch ( QgsServerException &ex )
      {
        responseDecorator.write( ex );
        QString format;
        QgsMessageLog::logMessage( ex.formatResponse( format ), QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
      }
      catch ( QgsException &ex )
      {
        // Internal server error
        response.sendError( 500, QStringLiteral( "Internal Server Error" ) );
        QgsMessageLog::logMessage( ex.what(), QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
      }
    }

    // Terminate the response
    // This may also throw exceptions if there are errors in python plugins code
    try
    {
      responseDecorator.finish();
    }
    catch ( QgsException &ex )
    {
      // Internal server error
      response.sendError( 500, QStringLiteral( "Internal Server Error" ) );
      QgsMessageLog::logMessage( ex.what(), QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    }

    // We are done using requestHandler in plugins, make sure we don't access
    // to a deleted request handler from Python bindings
    sServerInterface->clearRequestHandler();
  }

  if ( logLevel == Qgis::MessageLevel::Info )
  {
    QgsMessageLog::logMessage( "Request finished in " + QString::number( QgsApplication::profiler()->profileTime( QStringLiteral( "handleRequest" ), QStringLiteral( "server" ) ) * 1000.0 ) + " ms", QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
    if ( sSettings->logProfile() )
    {
      std::function <void( const QModelIndex &, int )> profileFormatter;
      profileFormatter = [ &profileFormatter ]( const QModelIndex & idx, int level )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Profile: %1%2, %3 : %4 ms" )
                                   .arg( level > 0 ? QString().fill( '-', level ) + ' ' : QString() )
                                   .arg( QgsApplication::profiler()->data( idx, QgsRuntimeProfilerNode::Roles::Group ).toString() )
                                   .arg( QgsApplication::profiler()->data( idx, QgsRuntimeProfilerNode::Roles::Name ).toString() )
                                   .arg( QString::number( QgsApplication::profiler()->data( idx, QgsRuntimeProfilerNode::Roles::Elapsed ).toDouble() * 1000.0 ) ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );

        for ( int subRow = 0; subRow < QgsApplication::profiler()->rowCount( idx ); subRow++ )
        {
          const auto subIdx { QgsApplication::profiler()->index( subRow, 0, idx ) };
          profileFormatter( subIdx, level + 1 );
        }

      };

      for ( int row = 0; row < QgsApplication::profiler()->rowCount( ); row++ )
      {
        const auto idx { QgsApplication::profiler()->index( row, 0 ) };
        profileFormatter( idx, 0 );
      }
    }
  }


  // Clear the profiler server section after each request
  QgsApplication::profiler()->clear( QStringLiteral( "server" ) );

}


#ifdef HAVE_SERVER_PYTHON_PLUGINS
void QgsServer::initPython()
{
  // Init plugins
  if ( ! QgsServerPlugins::initPlugins( sServerInterface ) )
  {
    QgsMessageLog::logMessage( QStringLiteral( "No server python plugins are available" ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "Server python plugins loaded" ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }
}
#endif

