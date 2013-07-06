/***************************************************************************
                              qgs_map_serv.cpp
 A server application supporting WMS/SLD syntax for HTTP GET and Orchestra
map service syntax for SOAP/HTTP POST
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
#include "qgsgetrequesthandler.h"
#include "qgspostrequesthandler.h"
#include "qgssoaprequesthandler.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgswmsserver.h"
#include "qgswfsserver.h"
#include "qgsmaprenderer.h"
#include "qgsmapserviceexception.h"
#include "qgspallabeling.h"
#include "qgsprojectparser.h"
#include "qgssldparser.h"
#include <QDomDocument>
#include <QImage>
#include <QSettings>
#include <QDateTime>

//for CMAKE_INSTALL_PREFIX
#include "qgsconfig.h"

#include <fcgi_stdio.h>


void dummyMessageHandler( QtMsgType type, const char *msg )
{
  Q_UNUSED( type );
  Q_UNUSED( msg );
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
  QgsDebugMsg( "current directory: " + currentDir.absolutePath() );
  fprintf( FCGI_stderr, "current directory: %s\n", currentDir.absolutePath().toUtf8().constData() );
  QStringList nameFilterList;
  nameFilterList << "*.qgs";
  QFileInfoList projectFiles = currentDir.entryInfoList( nameFilterList, QDir::Files, QDir::Name );
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

int main( int argc, char * argv[] )
{
#ifndef _MSC_VER
  qInstallMsgHandler( dummyMessageHandler );
#endif

  QgsApplication qgsapp( argc, argv, getenv( "DISPLAY" ) );

  //Default prefix path may be altered by environment variable
  char* prefixPath = getenv( "QGIS_PREFIX_PATH" );
  if ( prefixPath )
  {
    QgsApplication::setPrefixPath( prefixPath, TRUE );
  }
#if !defined(Q_OS_WIN)
  else
  {
    // init QGIS's paths - true means that all path will be inited from prefix
    QgsApplication::setPrefixPath( CMAKE_INSTALL_PREFIX, TRUE );
  }
#endif

#if defined(MAPSERVER_SKIP_ECW)
  QgsDebugMsg( "Skipping GDAL ECW drivers in server." );
  QgsApplication::skipGdalDriver( "ECW" );
  QgsApplication::skipGdalDriver( "JP2ECW" );
#endif

  QDomImplementation::setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );
  QgsDebugMsg( "Prefix  PATH: " + QgsApplication::prefixPath() );
  QgsDebugMsg( "Plugin  PATH: " + QgsApplication::pluginPath() );
  QgsDebugMsg( "PkgData PATH: " + QgsApplication::pkgDataPath() );
  QgsDebugMsg( "User DB PATH: " + QgsApplication::qgisUserDbFilePath() );

  QgsDebugMsg( qgsapp.applicationDirPath() + "/qgis_wms_server.log" );
  QgsApplication::createDB(); //init qgis.db (e.g. necessary for user crs)

  //create config cache and search for config files in the current directory.
  //These configurations are used if no mapfile parameter is present in the request
  QString defaultConfigFilePath;
  QFileInfo projectFileInfo = defaultProjectFile(); //try to find a .qgs file in the server directory
  if ( projectFileInfo.exists() )
  {
    defaultConfigFilePath = projectFileInfo.absoluteFilePath();
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

  while ( fcgi_accept() >= 0 )
  {
    printRequestInfos(); //print request infos if in debug mode

    //use QgsGetRequestHandler in case of HTTP GET and QgsSOAPRequestHandler in case of HTTP POST
    QgsRequestHandler* theRequestHandler = 0;
    char* requestMethod = getenv( "REQUEST_METHOD" );
    if ( requestMethod != NULL )
    {
      if ( strcmp( requestMethod, "POST" ) == 0 )
      {
        //QgsDebugMsg( "Creating QgsSOAPRequestHandler" );
        //theRequestHandler = new QgsSOAPRequestHandler();
        theRequestHandler = new QgsPostRequestHandler();
      }
      else
      {
        QgsDebugMsg( "Creating QgsGetRequestHandler" );
        theRequestHandler = new QgsGetRequestHandler();
      }
    }
    else
    {
      QgsDebugMsg( "Creating QgsGetRequestHandler" );
      theRequestHandler = new QgsGetRequestHandler();
    }

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

    //set admin config file to wms server object
    QString configFilePath( defaultConfigFilePath );

    paramIt = parameterMap.find( "MAP" );
    if ( paramIt == parameterMap.constEnd() )
    {
      QgsDebugMsg( QString( "Using default configuration file path: %1" ).arg( defaultConfigFilePath ) );
    }
    else
    {
      configFilePath = paramIt.value();
    }

    QgsConfigParser* adminConfigParser = QgsConfigCache::instance()->searchConfiguration( configFilePath );
    if ( !adminConfigParser )
    {
      QgsDebugMsg( "parse error on config file " + configFilePath );
      theRequestHandler->sendServiceException( QgsMapServiceException( "", "Configuration file problem : perhaps you left off the .qgs extension?" ) );
      continue;
    }

    //sld parser might need information about request parameters
    adminConfigParser->setParameterMap( parameterMap );

    //request to WMS?
    QString serviceString;
    paramIt = parameterMap.find( "SERVICE" );
    if ( paramIt == parameterMap.constEnd() )
    {
#ifndef QGISDEBUG
      serviceString = parameterMap.value( "SERVICE", "WMS" );
#else
      QgsDebugMsg( "unable to find 'SERVICE' parameter, exiting..." );
      theRequestHandler->sendServiceException( QgsMapServiceException( "ServiceNotSpecified", "Service not specified. The SERVICE parameter is mandatory" ) );
      delete theRequestHandler;
      continue;
#endif
    }
    else
    {
      serviceString = paramIt.value();
    }

    QgsWMSServer* theServer = 0;
    if ( serviceString == "WFS" )
    {
      delete theServer;
      QgsWFSServer* theServer = 0;
      try
      {
        theServer = new QgsWFSServer( parameterMap );
      }
      catch ( QgsMapServiceException e ) //admin.sld may be invalid
      {
        theRequestHandler->sendServiceException( e );
        continue;
      }

      theServer->setAdminConfigParser( adminConfigParser );


      //request type
      QString request = parameterMap.value( "REQUEST" );
      if ( request.isEmpty() )
      {
        //do some error handling
        QgsDebugMsg( "unable to find 'REQUEST' parameter, exiting..." );
        theRequestHandler->sendServiceException( QgsMapServiceException( "OperationNotSupported", "Please check the value of the REQUEST parameter" ) );
        delete theRequestHandler;
        delete theServer;
        continue;
      }

      if ( request.compare( "GetCapabilities", Qt::CaseInsensitive ) == 0 )
      {
        QDomDocument capabilitiesDocument;
        try
        {
          capabilitiesDocument = theServer->getCapabilities();
        }
        catch ( QgsMapServiceException& ex )
        {
          theRequestHandler->sendServiceException( ex );
          delete theRequestHandler;
          delete theServer;
          continue;
        }
        QgsDebugMsg( "sending GetCapabilities response" );
        theRequestHandler->sendGetCapabilitiesResponse( capabilitiesDocument );
        delete theRequestHandler;
        delete theServer;
        continue;
      }
      else if ( request.compare( "DescribeFeatureType", Qt::CaseInsensitive ) == 0 )
      {
        QDomDocument describeDocument;
        try
        {
          describeDocument = theServer->describeFeatureType();
        }
        catch ( QgsMapServiceException& ex )
        {
          theRequestHandler->sendServiceException( ex );
          delete theRequestHandler;
          delete theServer;
          continue;
        }
        QgsDebugMsg( "sending GetCapabilities response" );
        theRequestHandler->sendGetCapabilitiesResponse( describeDocument );
        delete theRequestHandler;
        delete theServer;
        continue;
      }
      else if ( request.compare( "GetFeature", Qt::CaseInsensitive ) == 0 )
      {
        //output format for GetFeature
        QString outputFormat = parameterMap.value( "OUTPUTFORMAT" );
        try
        {
          if ( theServer->getFeature( *theRequestHandler, outputFormat ) != 0 )
          {
            delete theRequestHandler;
            delete theServer;
            continue;
          }
          else
          {
            delete theRequestHandler;
            delete theServer;
            continue;
          }
        }
        catch ( QgsMapServiceException& ex )
        {
          theRequestHandler->sendServiceException( ex );
          delete theRequestHandler;
          delete theServer;
          continue;
        }
      }
      else if ( request.compare( "Transaction", Qt::CaseInsensitive ) == 0 )
      {
        QDomDocument transactionDocument;
        try
        {
          transactionDocument = theServer->transaction( parameterMap.value( "REQUEST_BODY" ) );
        }
        catch ( QgsMapServiceException& ex )
        {
          theRequestHandler->sendServiceException( ex );
          delete theRequestHandler;
          delete theServer;
          continue;
        }
        QgsDebugMsg( "sending Transaction response" );
        theRequestHandler->sendGetCapabilitiesResponse( transactionDocument );
        delete theRequestHandler;
        delete theServer;
        continue;
      }

      return 0;
    }

    try
    {
      theServer = new QgsWMSServer( parameterMap, theMapRenderer );
    }
    catch ( QgsMapServiceException e ) //admin.sld may be invalid
    {
      theRequestHandler->sendServiceException( e );
      continue;
    }

    theServer->setAdminConfigParser( adminConfigParser );


    //request type
    QString request = parameterMap.value( "REQUEST" );
    if ( request.isEmpty() )
    {
      //do some error handling
      QgsDebugMsg( "unable to find 'REQUEST' parameter, exiting..." );
      theRequestHandler->sendServiceException( QgsMapServiceException( "OperationNotSupported", "Please check the value of the REQUEST parameter" ) );
      delete theRequestHandler;
      delete theServer;
      continue;
    }

    QString version = parameterMap.value( "VERSION", "1.3.0" );
    bool getProjectSettings = ( request.compare( "GetProjectSettings", Qt::CaseInsensitive ) == 0 );
    if ( getProjectSettings )
    {
      version = "1.3.0"; //getProjectSettings extends WMS 1.3.0 capabilities
    }

    if ( request.compare( "GetCapabilities", Qt::CaseInsensitive ) == 0 || getProjectSettings )
    {
      const QDomDocument* capabilitiesDocument = capabilitiesCache.searchCapabilitiesDocument( configFilePath, getProjectSettings ? "projectSettings" : version );
      if ( !capabilitiesDocument ) //capabilities xml not in cache. Create a new one
      {
        QgsDebugMsg( "Capabilities document not found in cache" );
        QDomDocument doc;
        try
        {
          doc = theServer->getCapabilities( version, getProjectSettings );
        }
        catch ( QgsMapServiceException& ex )
        {
          theRequestHandler->sendServiceException( ex );
          delete theRequestHandler;
          delete theServer;
          continue;
        }
        capabilitiesCache.insertCapabilitiesDocument( configFilePath, getProjectSettings ? "projectSettings" : version, &doc );
        capabilitiesDocument = capabilitiesCache.searchCapabilitiesDocument( configFilePath, getProjectSettings ? "projectSettings" : version );
      }
      else
      {
        QgsDebugMsg( "Found capabilities document in cache" );
      }

      if ( capabilitiesDocument )
      {
        theRequestHandler->sendGetCapabilitiesResponse( *capabilitiesDocument );
      }
      delete theRequestHandler;
      delete theServer;
      continue;
    }
    else if ( request.compare( "GetMap", Qt::CaseInsensitive ) == 0 )
    {
      QImage* result = 0;
      try
      {
        result = theServer->getMap();
      }
      catch ( QgsMapServiceException& ex )
      {
        QgsDebugMsg( "Caught exception during GetMap request" );
        theRequestHandler->sendServiceException( ex );
        delete theRequestHandler;
        delete theServer;
        continue;
      }

      if ( result )
      {
        QgsDebugMsg( "Sending GetMap response" );
        theRequestHandler->sendGetMapResponse( serviceString, result );
        QgsDebugMsg( "Response sent" );
      }
      else
      {
        //do some error handling
        QgsDebugMsg( "result image is 0" );
      }
      delete result;
      delete theRequestHandler;
      delete theServer;
      continue;
    }
    else if ( request.compare( "GetFeatureInfo", Qt::CaseInsensitive ) == 0 )
    {
      QDomDocument featureInfoDoc;
      try
      {
        if ( theServer->getFeatureInfo( featureInfoDoc, version ) != 0 )
        {
          delete theRequestHandler;
          delete theServer;
          continue;
        }
      }
      catch ( QgsMapServiceException& ex )
      {
        theRequestHandler->sendServiceException( ex );
        delete theRequestHandler;
        delete theServer;
        continue;
      }

      QString infoFormat = parameterMap.value( "INFO_FORMAT" );
      theRequestHandler->sendGetFeatureInfoResponse( featureInfoDoc, infoFormat );
      delete theRequestHandler;
      delete theServer;
      continue;
    }
    else if ( request.compare( "GetContext", Qt::CaseInsensitive ) == 0 )
    {
      try
      {
        QDomDocument doc = theServer->getContext();
        theRequestHandler->sendGetStyleResponse( doc );
      }
      catch ( QgsMapServiceException& ex )
      {
        theRequestHandler->sendServiceException( ex );
      }

      delete theRequestHandler;
      delete theServer;
      continue;
    }
    else if ( request.compare( "GetStyles", Qt::CaseInsensitive ) == 0 || request.compare( "GetStyle", Qt::CaseInsensitive ) == 0 ) // GetStyle for compatibility with earlier QGIS versions
    {
      try
      {
        QDomDocument doc = theServer->getStyle();
        theRequestHandler->sendGetStyleResponse( doc );
      }
      catch ( QgsMapServiceException& ex )
      {
        theRequestHandler->sendServiceException( ex );
      }

      delete theRequestHandler;
      delete theServer;
      continue;
    }
    else if ( request.compare( "GetLegendGraphic", Qt::CaseInsensitive ) == 0 || request.compare( "GetLegendGraphics", Qt::CaseInsensitive ) == 0 ) // GetLegendGraphics for compatibility with earlier QGIS versions
    {
      QImage* result = 0;
      try
      {
        result = theServer->getLegendGraphics();
      }
      catch ( QgsMapServiceException& ex )
      {
        theRequestHandler->sendServiceException( ex );
      }

      if ( result )
      {
        QgsDebugMsg( "Sending GetLegendGraphic response" );
        //sending is the same for GetMap and GetLegendGraphic
        theRequestHandler->sendGetMapResponse( serviceString, result );
      }
      else
      {
        //do some error handling
        QgsDebugMsg( "result image is 0" );
      }
      delete result;
      delete theRequestHandler;
      delete theServer;
      continue;

    }
    else if ( request.compare( "GetPrint", Qt::CaseInsensitive ) == 0 )
    {
      QByteArray* printOutput = 0;
      try
      {
        printOutput = theServer->getPrint( theRequestHandler->format() );
      }
      catch ( QgsMapServiceException& ex )
      {
        theRequestHandler->sendServiceException( ex );
      }

      if ( printOutput )
      {
        theRequestHandler->sendGetPrintResponse( printOutput );
      }
      delete printOutput;
      delete theRequestHandler;
      delete theServer;
      continue;
    }
    else//unknown request
    {
      QgsMapServiceException e( "OperationNotSupported", "Operation " + request + " not supported" );
      theRequestHandler->sendServiceException( e );
      delete theRequestHandler;
      delete theServer;
    }
  }

  delete theMapRenderer;
  QgsDebugMsg( "************* all done ***************" );
  return 0;
}
