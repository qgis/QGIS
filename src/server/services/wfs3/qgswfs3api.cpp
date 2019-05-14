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
#include "qgsserverapicontext.h"
#include "qgswfs3api.h"
#include "qgswfs3handlers.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"


#include "nlohmann/json.hpp"
#include "inja/inja.hpp"

using json = nlohmann::json;
using namespace inja;

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
    registerHandler<CollectionsItemsHandler>();
    registerHandler<CollectionsFeatureHandler>();
    registerHandler<CollectionsHandler>();
    registerHandler<ConformanceHandler>();
    registerHandler<APIHandler>();
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

  std::string Api::relToString( const rel &rel )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<QgsWfs3::rel>();
    return metaEnum.valueToKey( rel );
  }

  QString Api::contentTypeToString( const contentType &ct )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<contentType>();
    return metaEnum.valueToKey( ct );
  }

  std::string Api::contentTypeToStdString( const contentType &ct )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<contentType>();
    return metaEnum.valueToKey( ct );
  }

  QString Api::contentTypeToExtension( const contentType &ct )
  {
    return contentTypeToString( ct ).toLower();
  }

  void Handler::write( const json &data,  const QgsServerRequest &request, QgsServerResponse &response, const QString &contentType ) const
  {
    // TODO: accept GML and get mime type from ACCEPT HTTP header
    const auto extension { QgsWfs3::Api::extension( request.url() ) };
    if ( extension == "" || extension == QgsWfs3::Api::contentTypeToStdString( QgsWfs3::contentType::JSON ) )
    {
      jsonDump( data, response, contentType == "" ? QStringLiteral( "application/json" ) : contentType );
    }
    else if ( extension == QgsWfs3::Api::contentTypeToStdString( QgsWfs3::contentType::HTML ) )
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

  void Handler::htmlDump( const json &data, QgsServerResponse &response ) const
  {
    auto path { templatePath() };
    if ( ! QFile::exists( path ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Template not found error: %1" ).arg( path ), QStringLiteral( "Server" ), Qgis::Critical );
      throw QgsServerApiBadRequestError( QStringLiteral( "Template not found" ) );
    }

    QFile f( path );
    if ( ! f.open( QFile::ReadOnly | QFile::Text ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Could not open template file: %1" ).arg( path ), QStringLiteral( "Server" ), Qgis::Critical );
      throw QgsServerApiInternalServerError( QStringLiteral( "Could not open template file" ) );
    }

    QTextStream in( &f );
    const auto tplSource { in.readAll().toStdString() };
    f.close();

    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/html" ) );
    Environment env;
    auto tpl { env.parse( tplSource ) };
    try
    {
      response.write( env.render( tpl, data ) );
    }
    catch ( std::exception &e )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error parsing template file: %1 - %2" ).arg( path, e.what() ), QStringLiteral( "Server" ), Qgis::Critical );
      throw QgsServerApiInternalServerError( QStringLiteral( "Error parsing template file" ) );
    }
  }

  void Handler::xmlDump( const json &data, QgsServerResponse &response ) const
  {
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "application/xml" ) );
    response.write( "<pre>" + data.dump( 2 ) + "</pre>" );
  }

  const QString Handler::templatePath() const
  {
    // TODO: make this path configurable
    auto path { QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/server/api/wfs3/" ) )};
    path += staticMetaObject.className();
    path += QStringLiteral( ".html" );
    return path;
  }

  void Api::executeRequest( QgsServerApiContext *context ) const
  {
    // Get url
    const auto url { normalizedUrl( context->request()->url() ) };
    // Find matching handler
    auto hasMatch { false };
    for ( const auto &h : handlers() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Checking Path %1 for %2 " ).arg( url.path( ), rootPath() + h->path.pattern() ), QStringLiteral( "Server" ), Qgis::Info );
      if ( h->path.match( url.path( ) ).hasMatch() )
      {
        hasMatch = true;
        // Execute handler
        QgsMessageLog::logMessage( QStringLiteral( "Found handler %1" ).arg( QString::fromStdString( h->operationId ) ), QStringLiteral( "Server" ), Qgis::Info );
        h->handleRequest( this, context );
        break;
      }
    }
    // Throw
    if ( ! hasMatch )
    {
      throw QgsServerApiBadRequestError( QStringLiteral( "Requested URI does not match with any API handler" ) );
    }
  }

  void Handler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
  {
    Q_UNUSED( api );
    const json data
    {
      { "operationId", operationId },
      { "path", path.pattern().toStdString() },
      { "summary", summary },
      { "description", description },
      { "linkTitle", linkTitle },
      { "linkType", QgsWfs3::Api::relToString( linkType ) },
      { "mimeType", mimeType }
    };
    jsonDump( data, *context->response() );
  }

  std::string Handler::href( const Api *api, const QgsServerRequest &request, const QString &extraPath, const QString &extension ) const
  {
    auto url { request.url() };
    Q_ASSERT( path.match( url.path() ).captured().count() > 0 );
    url.setPath( api->rootPath() + path.match( url.path() ).captured()[0] + extraPath );
    if ( ! extension.isEmpty() )
      url.setPath( url.path() + '.' + extension );
    return url.toString( QUrl::FullyEncoded ).toStdString();
  }


} // namespace QgsWfs3
