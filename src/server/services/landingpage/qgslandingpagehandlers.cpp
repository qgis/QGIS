/***************************************************************************
                              qgsLandingPagehandlers.cpp
                              -------------------------
  begin                : May 3, 2019
  copyright            : (C) 2019 by Alessandro Pasotti
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

#include "qgslandingpagehandlers.h"
#include "qgslandingpageutils.h"
#include "qgsserverinterface.h"
#include "qgsserverresponse.h"
#include "qgsserverprojectutils.h"
#include "qgsvectorlayer.h"
#include "qgslayertreenode.h"
#include "qgslayertree.h"

#include <QDir>
#include <QCryptographicHash>

QgsLandingPageHandler::QgsLandingPageHandler( const QgsServerSettings *settings )
  : mSettings( settings )
{
  setContentTypes( { QgsServerOgcApi::ContentType::JSON, QgsServerOgcApi::ContentType::HTML } );
}

void QgsLandingPageHandler::handleRequest( const QgsServerApiContext &context ) const
{
  const QString requestPrefix { prefix( context.serverInterface()->serverSettings() ) };
  auto urlPath { context.request()->url().path() };

  while ( urlPath.endsWith( '/' ) )
  {
    urlPath.chop( 1 );
  }

  if ( urlPath == requestPrefix )
  {
    QUrl url { context.request()->url() };
    url.setPath( QStringLiteral( "%1/index.%2" )
                   .arg( requestPrefix, QgsServerOgcApi::contentTypeToExtension( contentTypeFromRequest( context.request() ) ) ) );
    context.response()->setStatusCode( 302 );
    context.response()->setHeader( QStringLiteral( "Location" ), url.toString() );
  }
  else
  {
    const json projects = projectsData( *context.request() );
    json data {
      { "links", links( context ) },
      { "projects", projects },
      { "projects_count", projects.size() }
    };
    write( data, context, { { "pageTitle", linkTitle() }, { "navigation", json::array() } } );
  }
}

const QString QgsLandingPageHandler::templatePath( const QgsServerApiContext &context ) const
{
  QString path { context.serverInterface()->serverSettings()->apiResourcesDirectory() };
  path += QLatin1String( "/ogc/static/landingpage/index.html" );
  return path;
}

QString QgsLandingPageHandler::prefix( const QgsServerSettings *settings )
{
  QString prefix { settings->landingPageBaseUrlPrefix() };

  while ( prefix.endsWith( '/' ) )
  {
    prefix.chop( 1 );
  }

  if ( !prefix.isEmpty() && !prefix.startsWith( '/' ) )
  {
    prefix.prepend( '/' );
  }
  return prefix;
}

json QgsLandingPageHandler::projectsData( const QgsServerRequest &request ) const
{
  json j = json::array();
  const QMap<QString, QString> availableProjects = QgsLandingPageUtils::projects( *mSettings );
  for ( auto it = availableProjects.constBegin(); it != availableProjects.constEnd(); ++it )
  {
    try
    {
      j.push_back( QgsLandingPageUtils::projectInfo( it.value(), mSettings, request ) );
    }
    catch ( QgsServerException & )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Could not open project '%1': skipping." ).arg( it.value() ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Critical );
    }
  }
  return j;
}


QgsLandingPageMapHandler::QgsLandingPageMapHandler( const QgsServerSettings *settings )
  : mSettings( settings )
{
  setContentTypes( { QgsServerOgcApi::ContentType::JSON } );
}

void QgsLandingPageMapHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json data;
  data["links"] = json::array();
  const QString projectPath { QgsLandingPageUtils::projectUriFromUrl( context.request()->url().path(), *mSettings ) };
  if ( projectPath.isEmpty() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Requested project hash not found!" ) );
  }
  data["project"] = QgsLandingPageUtils::projectInfo( projectPath, mSettings, *context.request() );
  write( data, context, { { "pageTitle", linkTitle() }, { "navigation", json::array() } } );
}

QRegularExpression QgsLandingPageMapHandler::path() const
{
  return QRegularExpression( QStringLiteral( R"re(^%1/map/([a-f0-9]{32}).*$)re" ).arg( QgsLandingPageHandler::prefix( mSettings ) ) );
}


QRegularExpression QgsLandingPageHandler::path() const
{
  return QRegularExpression( QStringLiteral( R"re(^%1(/index.html|/index.json|/)?$)re" ).arg( prefix( mSettings ) ) );
}
