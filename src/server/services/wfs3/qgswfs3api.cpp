/***************************************************************************
                              qgswfs3.cpp
                              -------------------------
  begin                : April 15, 2019
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

#include "qgsmodule.h"
#include "qgsproject.h"
#include "qgsserverexception.h"
#include "qgswfs3api.h"
#include "qgswfs3handlers.h"


#include "nlohmann/json.hpp"
#include "inja/inja.hpp"

using json = nlohmann::json;

namespace QgsWfs3
{

  template<class T>
  void Api::registerHandler()
  {
    auto handler { qgis::make_unique<T>() };
    mHandlers.emplace_back( std::move( handler ) );
  }

  Api::Api( QgsServerInterface *serverIface )
    : mServerIface( serverIface )
  {
    registerHandler<CollectionsHandler>();
    registerHandler<APIHandler>();
    registerHandler<ConformanceHandler>();
    registerHandler<LandingPageHandler>();
  }

  QUrl Api::normalizedUrl( QUrl url ) const
  {
    if ( url.path().endsWith( '/' ) )
    {
      url.setPath( url.path().chopped( 1 ) );
    }
    return url;
  }

  std::string Api::extension( QUrl url )
  {
    const auto parts { url.fileName().split( '.', QString::SplitBehavior::SkipEmptyParts ) };
    if ( parts.length() > 1 )
    {
      auto res { parts.last().toStdString() };
      std::transform( res.begin(), res.end(), res.begin(), ::toupper );
      return res;
    }
    return "";
  }

  const std::vector<std::unique_ptr<Handler>> &Api::handlers() const
  {
    return mHandlers;
  }

  void Handler::write( const json &data,  const QgsServerRequest &request, QgsServerResponse &response, const QString &contentType ) const
  {
    const auto extension { QgsWfs3::Api::extension( request.url() ) };
    if ( extension == "" || extension == QgsWfs3::Api::contentTypeToString( QgsWfs3::contentType::JSON ) )
    {
      jsonDump( data, response, contentType == "" ? QStringLiteral( "application/json" ) : contentType );
    }
    else if ( extension == QgsWfs3::Api::contentTypeToString( QgsWfs3::contentType::HTML ) )
    {
      htmlDump( data, response );
    }
    else
    {
      throw QgsServerApiBadRequestError( "Content type could not be determined from the request!" );
    }
  }

  void Handler::jsonDump( const json &data, QgsServerResponse &response, const QString &contentType ) const
  {
    response.setHeader( QStringLiteral( "Content-Type" ), contentType );
#ifdef QGISDEBUG
    response.write( data.dump( 2 ) );
#else
    response.write( data.dump( ) );
#endif
  }

  void Handler::htmlDump( const json &data, QgsServerResponse &response, const std::string &templatePath ) const
  {
    // TODO: template loader and parser
    auto path { templatePath };
    if ( path == "" )
    {
      path = staticMetaObject.className();
      path += ".html";
    }
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/html" ) );
    response.write( "<h1>" + path + "</h1>" );
    response.write( "<pre>" + data.dump( 2 ) + "</pre>" );
  }

  void Handler::xmlDump( const json &data, QgsServerResponse &response ) const
  {
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "application/xml" ) );
    response.write( "<pre>" + data.dump( 2 ) + "</pre>" );
  }

  void Api::executeRequest( const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) const
  {
    // Get url
    const auto url { normalizedUrl( request.url() ) };
    // Find matching handler
    auto hasMatch { false };
    for ( const auto &h : handlers() )
    {
      //qDebug() << "Checking Path "  << url.path( ) << " starts with " << rootPath() + h->path ;
      if ( url.path( ).startsWith( rootPath() + h->path ) )
      {
        hasMatch = true;
        // Execute handler
        h->handleRequest( this, request, response, project );
        break;
      }
    }
    // Throw
    if ( ! hasMatch )
    {
      throw QgsServerApiNotFoundError( QStringLiteral( "Requested URI does not match with any API handler" ) );
    }
  }

  void Handler::handleRequest( const QgsWfs3::Api *api, const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) const
  {
    const json data
    {
      { "operationId", operationId },
      { "path", path.toStdString() },
      { "summary", summary },
      { "description", description },
      { "linkTitle", linkTitle },
      { "linkType", QgsWfs3::Api::relToString( linkType ) },
      { "mimeType", mimeType }
    };
    jsonDump( data, response );
  }

  std::string Handler::href( const Api *api, const QgsServerRequest &request ) const
  {
    auto url { request.url() };
    url.setPath( api->rootPath() + path );
    return url.toString().toStdString();
  }


} // namespace QgsWfs3
