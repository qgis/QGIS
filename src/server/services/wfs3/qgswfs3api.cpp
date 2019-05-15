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

  QMap<contentTypes, QString> sContentTypeMime = [ ]() -> QMap<contentTypes, QString>
  {
    QMap<contentTypes, QString> map;
    map[contentTypes::JSON] = QStringLiteral( "application/json" );
    map[contentTypes::GEOJSON] = QStringLiteral( "application/vnd.geo+json" );
    map[contentTypes::HTML] = QStringLiteral( "text/html" );
    map[contentTypes::XML] = QStringLiteral( "application/xml" );
    map[contentTypes::GML] = QStringLiteral( "application/gml+xml" );
    map[contentTypes::OPENAPI3] = QStringLiteral( "application/openapi+json;version=3.0" );
    Q_ASSERT( map.count() == QMetaEnum::fromType<QgsWfs3::contentTypes>().keyCount() );
    return map;
  }();

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

  QString Api::contentTypeToString( const contentTypes &ct )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<contentTypes>();
    return metaEnum.valueToKey( ct );
  }

  std::string Api::contentTypeToStdString( const contentTypes &ct )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<contentTypes>();
    return metaEnum.valueToKey( ct );
  }

  QString Api::contentTypeToExtension( const contentTypes &ct )
  {
    return contentTypeToString( ct ).toLower();
  }


  void Handler::write( const json &data,  const QgsServerRequest *request, QgsServerResponse *response ) const
  {
    // TODO: accept GML
    const auto contentType { contentTypeFromRequest( request ) };

    switch ( contentType )
    {
      case QgsWfs3::contentTypes::HTML:
        htmlDump( data, response );
        break;
      case QgsWfs3::contentTypes::GEOJSON:
      case QgsWfs3::contentTypes::JSON:
      case QgsWfs3::contentTypes::OPENAPI3:
        jsonDump( data, response, sContentTypeMime.value( contentType ) );
        break;
      case QgsWfs3::contentTypes::GML:
      case QgsWfs3::contentTypes::XML:
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

  void Handler::htmlDump( const json &data, QgsServerResponse *response ) const
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

    try
    {
      // Get the template directory and the file name
      QFileInfo pathInfo { path };
      Environment env { ( pathInfo.dir().path() + QDir::separator() ).toStdString() };
      response->write( env.render_file( pathInfo.fileName().toStdString(), data ) );
    }
    catch ( std::exception &e )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error parsing template file: %1 - %2" ).arg( path, e.what() ), QStringLiteral( "Server" ), Qgis::Critical );
      throw QgsServerApiInternalServerError( QStringLiteral( "Error parsing template file" ) );
    }
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

  contentTypes Handler::contentTypeFromRequest( const QgsServerRequest *request ) const
  {
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
    const auto extension { QFileInfo( request->url().path() ).completeSuffix().toUpper() };
    if ( ! extension.isEmpty() )
    {
      static QMetaEnum metaEnum { QMetaEnum::fromType<contentTypes>() };
      auto ok { false };
      const auto ct  { metaEnum.keyToValue( extension.toLocal8Bit().constData(), &ok ) };
      if ( ok )
      {
        return static_cast<contentTypes>( ct );
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "The client requested an unsupported extension: %1" ).arg( extension ), QStringLiteral( "Server" ), Qgis::Warning );
      }
    }
    // Fallback to default
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

    // (re-)add extention
    // JSON is the default anyway, we don'n need to add it
    if ( ! extension.isEmpty() &&  extension != QStringLiteral( "json" ) )
    {
      url.setPath( url.path() + '.' + extension );
    }
    return QgsWfs3::Api::normalizedUrl( url ).toString( QUrl::FullyEncoded ).toStdString();
  }

  QString contentTypeForAccept( const QString &accept )
  {
    for ( const auto &ct : sContentTypeMime )
    {
      if ( accept.contains( ct ) )
      {
        return ct;
      }
    }
    return QString();
  }


} // namespace QgsWfs3
