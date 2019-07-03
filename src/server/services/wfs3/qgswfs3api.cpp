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
#include "qgsvectorlayer.h"

#include "nlohmann/json.hpp"
#include "inja/inja.hpp"

using json = nlohmann::json;
using namespace inja;


namespace QgsWfs3
{

  QMap<contentType, QString> sContentTypeMime = [ ]() -> QMap<contentType, QString>
  {
    QMap<contentType, QString> map;
    map[contentType::JSON] = QStringLiteral( "application/json" );
    map[contentType::GEOJSON] = QStringLiteral( "application/vnd.geo+json" );
    map[contentType::HTML] = QStringLiteral( "text/html" );
    map[contentType::XML] = QStringLiteral( "application/xml" );
    map[contentType::GML] = QStringLiteral( "application/gml+xml" );
    map[contentType::OPENAPI3] = QStringLiteral( "application/openapi+json;version=3.0" );
    Q_ASSERT( map.count() == QMetaEnum::fromType<QgsWfs3::contentType>().keyCount() );
    return map;
  }();

  template<class T>
  void Api::registerHandler()
  {
    auto handler { qgis::make_unique<T>() };
    mHandlers.emplace_back( std::move( handler ) );
  }

  Api::Api( QgsServerInterface *serverIface ):
    QgsServerApi( serverIface )
  {
    registerHandler<CollectionsItemsHandler>();
    registerHandler<CollectionsFeatureHandler>();
    registerHandler<CollectionsHandler>();
    registerHandler<DescribeCollectionHandler>();
    registerHandler<ConformanceHandler>();
    registerHandler<StaticHandler>();
    registerHandler<APIHandler>();
    registerHandler<LandingPageHandler>();
  }

  QUrl Api::normalizedUrl( QUrl url )
  {
    return url.adjusted( QUrl::StripTrailingSlash | QUrl::NormalizePathSegments );
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
    QString result { metaEnum.valueToKey( ct ) };
    return result.replace( '_', '-' );
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

  void Handler::write( const json &data, const Api *api, const QgsServerRequest *request, QgsServerResponse *response, const json &metadata ) const
  {
    // TODO: accept GML and XML?
    const auto contentType { contentTypeFromRequest( request ) };
    json _data { data };
    switch ( contentType )
    {
      case QgsWfs3::contentType::HTML:
        _data["handler"] = handlerData( );
        if ( ! metadata.is_null() )
        {
          _data["metadata"] = metadata;
        }
        htmlDump( _data, api, request, response );
        break;
      case QgsWfs3::contentType::GEOJSON:
      case QgsWfs3::contentType::JSON:
      case QgsWfs3::contentType::OPENAPI3:
        jsonDump( _data, response, sContentTypeMime.value( contentType ) );
        break;
      case QgsWfs3::contentType::GML:
      case QgsWfs3::contentType::XML:
        throw QgsServerApiNotImplementedError( QStringLiteral( "Requested content type is not yet implemented" ) );
    }
  }

  void Handler::jsonDump( const json &data, QgsServerResponse *response, const QString &contentType ) const
  {
    response->setHeader( QStringLiteral( "Content-Type" ), contentType );
#ifdef QGISDEBUG
    response->write( data.dump( 2 ) );
#else
    response->write( data.dump( ) );
#endif
  }

  void Handler::htmlDump( const json &data, const Api *api, const QgsServerRequest *request, QgsServerResponse *response ) const
  {
    response->setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/html" ) );
    auto path { templatePath() };
    if ( ! QFile::exists( path ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Template not found error: %1" ).arg( path ), QStringLiteral( "Server" ), Qgis::Critical );
      throw QgsServerApiBadRequestError( QStringLiteral( "Template not found: %1" ).arg( QFileInfo( path ).fileName() ) );
    }

    QFile f( path );
    if ( ! f.open( QFile::ReadOnly | QFile::Text ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Could not open template file: %1" ).arg( path ), QStringLiteral( "Server" ), Qgis::Critical );
      throw QgsServerApiInternalServerError( QStringLiteral( "Could not open template file: %1" ).arg( QFileInfo( path ).fileName() ) );
    }

    try
    {
      // Get the template directory and the file name
      QFileInfo pathInfo { path };
      Environment env { ( pathInfo.dir().path() + QDir::separator() ).toStdString() };

      // For template debugging:
      env.add_callback( "json_dump", 0, [ = ]( Arguments & )
      {
        return data.dump();
      } );

      // Path manipulation
      env.add_callback( "append_path", 1, [ = ]( Arguments & args )
      {
        auto url { request->url() };
        QFileInfo fi{ url.path() };
        auto suffix { fi.suffix() };
        auto fName { fi.filePath()};
        fName.chop( suffix.length() + 1 );
        fName += '/' + QString::number( args.at( 0 )->get<QgsFeatureId>( ) );
        if ( !suffix.isEmpty() )
        {
          fName += '.' + suffix;
        }
        fi.setFile( fName );
        url.setPath( fi.filePath() );
        return url.toString().toStdString();
      } );

      // Path manipulation
      env.add_callback( "chomp_path", 1, [ = ]( Arguments & args )
      {
        QUrl url { QString::fromStdString( args.at( 0 )->get<std::string>( ) ) };
        QFileInfo fi{ url.path() };
        auto suffix { fi.suffix() };
        auto fName { fi.filePath()};
        fName.chop( suffix.length() + 1 );
        // Chomp last segment
        fName = fName.replace( QRegularExpression( R"raw(\/[^/]+$)raw" ), QString() );
        if ( !suffix.isEmpty() )
        {
          fName += '.' + suffix;
        }
        fi.setFile( fName );
        url.setPath( fi.filePath() );
        return url.toString().toStdString();
      } );

      // Static
      env.add_callback( "static", 1, [ = ]( Arguments & args )
      {
        auto asset( args.at( 0 )->get<std::string>( ) );
        return api->rootPath().toStdString() + "/static/" + asset;
      } );

      response->write( env.render_file( pathInfo.fileName().toStdString(), data ) );
    }
    catch ( std::exception &e )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error parsing template file: %1 - %2" ).arg( path, e.what() ), QStringLiteral( "Server" ), Qgis::Critical );
      throw QgsServerApiInternalServerError( QStringLiteral( "Error parsing template file: %1" ).arg( e.what() ) );
    }
  }

  json Handler::handlerData() const
  {
    // TODO: add navigation for breadcrumbs
    json _data;
    _data["linkTitle"] = linkTitle;
    _data["operationId"] = operationId;
    _data["description"] = description;
    _data["summary"] = summary;
    return _data;
  }

  void Handler::xmlDump( const json &data, QgsServerResponse *response ) const
  {
    response->setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "application/xml" ) );
    response->write( "<pre>" + data.dump( 2 ) + "</pre>" );
  }

  const QString Handler::templatePath() const
  {
    auto path { resourcesPath() };
    path += QString::fromStdString( operationId );
    path += QStringLiteral( ".html" );
    return path;
  }

  const QString Handler::resourcesPath() const
  {
    return QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/server/api/wfs3/" ) );
  }

  const QString Handler::staticPath() const
  {
    return resourcesPath() + QDir().separator() + QStringLiteral( "static" );
  }

  contentType Handler::contentTypeFromRequest( const QgsServerRequest *request ) const
  {
    // First file extension ...
    const auto extension { QFileInfo( request->url().path() ).completeSuffix().toUpper() };
    if ( ! extension.isEmpty() )
    {
      static QMetaEnum metaEnum { QMetaEnum::fromType<contentType>() };
      auto ok { false };
      const auto ct  { metaEnum.keyToValue( extension.toLocal8Bit().constData(), &ok ) };
      if ( ok )
      {
        return static_cast<contentType>( ct );
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "The client requested an unsupported extension: %1" ).arg( extension ), QStringLiteral( "Server" ), Qgis::Warning );
      }
    }
    // ... then "Accept" ...
    const auto accept { request->header( QStringLiteral( "Accept" ) ) };
    if ( ! accept.isEmpty() )
    {
      const auto ctFromAccept { contentTypeForAccept( accept ) };
      if ( ! ctFromAccept.isEmpty() )
      {
        return sContentTypeMime.key( ctFromAccept );
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "The client requested an unsupported content type in Accept header: %1" ).arg( accept ), QStringLiteral( "Server" ), Qgis::Warning );
      }
    }
    // ... fallback to default
    return mimeType;
  }

  QgsVectorLayer *Handler::layerFromCollection( QgsServerApiContext *context, const QString &collectionId ) const
  {
    const auto mapLayers { context->project()->mapLayersByShortName<QgsVectorLayer *>( collectionId ) };
    if ( mapLayers.count() != 1 )
    {
      throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Collection with given id (%1) was not found or multiple matches were found" ).arg( collectionId ) );
    }
    return mapLayers.first();
  }

  std::string Api::parentLink( const QUrl &url, int levels )
  {
    auto path { url.path() };
    const QFileInfo fi { path };
    const auto suffix { fi.suffix() };
    if ( ! suffix.isEmpty() )
    {
      path.chop( suffix.length() + 1 );
    }
    while ( path.endsWith( '/' ) )
    {
      path.chop( 1 );
    }
    QRegularExpression re( R"raw(\/[^/]+$)raw" );
    for ( int i = 0; i < levels ; i++ )
    {
      path = path.replace( re, QString() );
    }
    QUrl result( url );
    QList<QPair<QString, QString> > qi;
    const auto constItems { result.queryItems( ) };
    for ( const auto &i : constItems )
    {
      if ( i.first.compare( QStringLiteral( "MAP" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 )
      {
        qi.push_back( i );
      }
    }
    result.setQueryItems( qi );
    result.setPath( path );
    return result.toString().toStdString();
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
    jsonDump( data, context->response() );
  }

  std::string Handler::href( const Api *api, const QgsServerRequest *request, const QString &extraPath, const QString &extension ) const
  {
    auto url { request->url() };
    const auto match { path.match( url.path() ) };
    if ( match.captured().count() > 0 )
    {
      url.setPath( api->rootPath() + match.captured( 0 ) );
    }
    // Remove any existing extension
    const auto suffixLength { QFileInfo( url.path() ).completeSuffix().length() };
    if ( suffixLength > 0 )
    {
      auto path {url.path()};
      path.truncate( path.length() - ( suffixLength + 1 ) );
      url.setPath( path );
    }
    // Add extra path
    url.setPath( url.path() + extraPath );

    // (re-)add extension
    // JSON is the default anyway, we don'n need to add it
    if ( ! extension.isEmpty() )
    {
      // Remove trailing slashes if any.
      QString path { url.path() };
      while ( path.endsWith( '/' ) )
      {
        path.chop( 1 );
      }
      url.setPath( path + '.' + extension );
    }
    return QgsWfs3::Api::normalizedUrl( url ).toString( QUrl::FullyEncoded ).toStdString();
  }

  json Handler::link( const Api *api, const QgsServerApiContext *context,
                      const rel &linkType, const contentType contentType,
                      const std::string &title ) const
  {
    const auto currentCt { contentTypeFromRequest( context->request() ) };
    json l
    {
      {
        "href", href( api, context->request(), "/" + landingPageRootLink,
                      QgsWfs3::Api::contentTypeToExtension( contentType ) )
      },
      { "rel", QgsWfs3::Api::relToString( currentCt == contentType ? rel::self : linkType ) },
      { "type", QgsWfs3::Api::mimeType( contentType ) },
      { "title", title != "" ? title : linkTitle },
    };
    return l;
  }

  json Handler::links( const Api *api, const QgsServerApiContext *context ) const
  {
    json links = json::array();
    for ( const auto &ct : qgis::as_const( contentTypes ) )
    {
      links.push_back( link( api, context, rel::alternate, ct, linkTitle  + " as " + QgsWfs3::Api::contentTypeToStdString( ct ) ) );
    }
    return links;
  }


  QString contentTypeForAccept( const QString &accept )
  {
    QString result;
    for ( const auto &ct : qgis::as_const( sContentTypeMime ) )
    {
      if ( accept.contains( ct ) )
      {
        result = ct;
        break;
      }
    }
    return result;
  }

  std::string Api::mimeType( const contentType &contentType )
  {
    return sContentTypeMime.value( contentType, QString() ).toStdString();

  }

} // namespace QgsWfs3
