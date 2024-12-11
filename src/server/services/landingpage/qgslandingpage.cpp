/***************************************************************************
                              qgslandingpage.cpp
                              -------------------------
  begin                : August 3, 2020
  copyright            : (C) 2020 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodule.h"
#include "qgsserverogcapi.h"
#include "qgsserverfilter.h"
#include "qgslandingpagehandlers.h"
#include "qgslandingpageutils.h"
#include "qgsserverstatichandler.h"
#include "qgsmessagelog.h"


/**
 * Landing page API
 * \since QGIS 3.16
 */
class QgsLandingPageApi : public QgsServerOgcApi
{
  public:
    QgsLandingPageApi( QgsServerInterface *serverIface, const QString &rootPath, const QString &name, const QString &description = QString(), const QString &version = QString() )
      : QgsServerOgcApi( serverIface, rootPath, name, description, version )
    {
    }

    bool accept( const QUrl &url ) const override
    {
      QString baseUrlPrefix { serverIface()->serverSettings()->landingPageBaseUrlPrefix() };

      // Make sure non empty prefix always starts with /
      if ( !baseUrlPrefix.isEmpty() && !baseUrlPrefix.startsWith( '/' ) )
      {
        baseUrlPrefix.prepend( '/' );
      }

      // Make sure path always starts with '/'
      QString path { url.path().startsWith( '/' ) ? url.path() : url.path().prepend( '/' ) };

      // Mainly for CI testing of legacy OGC XML responses, we offer a way to disable landingpage API.
      // The plugin installation is optional so this won't be an issue in production.
      if ( qgetenv( "QGIS_SERVER_DISABLED_APIS" ).contains( name().toUtf8() ) || ( !path.startsWith( baseUrlPrefix ) ) )
      {
        return false;
      }

      path = path.mid( baseUrlPrefix.length() );

      // Valid paths
      return path.isEmpty()
             || path == '/'
             || path.startsWith( QLatin1String( "/map/" ) )
             || path.startsWith( QLatin1String( "/index." ) )
             // Static
             || path.startsWith( QLatin1String( "/css/" ) )
             || path.startsWith( QLatin1String( "/js/" ) )
             || path == QLatin1String( "favicon.ico" );
    }
};

/**
 * Sets QGIS_PROJECT_FILE from /project/<hash>/ URL fragment
 * This is used to set the QGIS_PROJECT_FILE environment variable for legacy SERVICEs (WFS, WMS etc.)
 * \since QGIS 3.16
 */
class QgsProjectLoaderFilter : public QgsServerFilter
{
  public:
    QgsProjectLoaderFilter( QgsServerInterface *serverIface )
      : QgsServerFilter( serverIface )
    {
    }

    /**
     * Read the project hash and set the QGIS_PROJECT_FILE environment variable
     */
    void requestReady() override
    {
      mEnvWasChanged = false;
      const auto handler { serverInterface()->requestHandler() };
      if ( handler->path().startsWith( QStringLiteral( "%1/project/" ).arg( QgsLandingPageHandler::prefix( serverInterface()->serverSettings() ) ) ) )
      {
        const QString projectPath { QgsLandingPageUtils::projectUriFromUrl( handler->url(), *serverInterface()->serverSettings() ) };
        if ( !projectPath.isEmpty() )
        {
          mEnvWasChanged = true;
          mOriginalProjectFromEnv = qgetenv( "QGIS_PROJECT_FILE" );
          qputenv( "QGIS_PROJECT_FILE", projectPath.toUtf8() );
          serverInterface()->setConfigFilePath( projectPath.toUtf8() );
          QgsMessageLog::logMessage( QStringLiteral( "Project from URL set to: %1" ).arg( projectPath ), QStringLiteral( "Landing Page Plugin" ), Qgis::MessageLevel::Info );
        }
        else
        {
          QgsMessageLog::logMessage( QStringLiteral( "Could not get project from URL: %1" ).arg( handler->url() ), QStringLiteral( "Landing Page Plugin" ), Qgis::MessageLevel::Info );
        }
      }
    };

    /**
     * Restore original QGIS_PROJECT_FILE environment variable value
     */
    void responseComplete() override
    {
      if ( mEnvWasChanged )
        qputenv( "QGIS_PROJECT_FILE", mOriginalProjectFromEnv.toUtf8() );
    };


  private:
    QString mOriginalProjectFromEnv;
    bool mEnvWasChanged = false;
};


/**
 * \class QgsLandingPageModule
 * \brief Landing page module for QGIS Server
 * \since QGIS 3.16
 */
class QgsLandingPageModule : public QgsServiceModule
{
  public:
    void registerSelf( QgsServiceRegistry &registry, QgsServerInterface *serverIface ) override
    {
      QgsLandingPageApi *landingPageApi = new QgsLandingPageApi { serverIface, QStringLiteral( "/" ), QStringLiteral( "Landing Page" ), QStringLiteral( "1.0.0" ) };
      // Register handlers
      landingPageApi->registerHandler<QgsServerStaticHandler>(
        QStringLiteral( "%1/(?<staticFilePath>((css|js)/.*)|favicon.ico)$" )
          .arg( QgsLandingPageHandler::prefix( serverIface->serverSettings() ) ),
        QStringLiteral( "landingpage" )
      );
      landingPageApi->registerHandler<QgsLandingPageHandler>( serverIface->serverSettings() );
      landingPageApi->registerHandler<QgsLandingPageMapHandler>( serverIface->serverSettings() );

      // Register API
      registry.registerApi( landingPageApi );

      // Register filters
      serverIface->registerFilter( new QgsProjectLoaderFilter( serverIface ) );
    }
};


// Entry points
QGISEXTERN QgsServiceModule *QGS_ServiceModule_Init()
{
  static QgsLandingPageModule module;
  return &module;
}
QGISEXTERN void QGS_ServiceModule_Exit( QgsServiceModule * )
{
  // Nothing to do
}
