/***************************************************************************
  qgsserverogcapihandler.cpp - QgsServerOgcApiHandler

 ---------------------
 begin                : 10.7.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverogcapihandler.h"

#include <nlohmann/json.hpp>

#include "inja/inja.hpp"
#include "qgsjsonutils.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsserverapiutils.h"
#include "qgsserverinterface.h"
#include "qgsserverresponse.h"
#include "qgsvectorlayer.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

using namespace nlohmann;
using namespace inja;


QVariantMap QgsServerOgcApiHandler::values( const QgsServerApiContext &context ) const
{
  QVariantMap result;
  const auto constParameters { parameters( context ) };
  for ( const auto &p : constParameters )
  {
    // value() calls the validators and throws an exception if validation fails
    result[p.name()] = p.value( context );
  }
  const auto sanitizedPath { QgsServerOgcApi::sanitizeUrl( context.handlerPath() ).path() };
  const auto match { path().match( sanitizedPath ) };
  if ( match.hasMatch() )
  {
    const auto constNamed { path().namedCaptureGroups() };
    // Get named path parameters
    for ( const auto &name : constNamed )
    {
      if ( !name.isEmpty() )
        result[name] = QUrlQuery( match.captured( name ) ).toString();
    }
  }
  return result;
}

QgsServerOgcApiHandler::~QgsServerOgcApiHandler()
{
  //qDebug() << "handler destroyed";
}

QgsServerOgcApi::ContentType QgsServerOgcApiHandler::defaultContentType() const
{
  const auto constContentTypes( contentTypes() );
  return constContentTypes.size() > 0 ? constContentTypes.first() : QgsServerOgcApi::ContentType::JSON;
}

QList<QgsServerOgcApi::ContentType> QgsServerOgcApiHandler::contentTypes() const
{
  return mContentTypes;
}

void QgsServerOgcApiHandler::handleRequest( const QgsServerApiContext &context ) const
{
  Q_UNUSED( context )
  throw QgsServerApiNotImplementedException( u"Subclasses must implement handleRequest"_s );
}

QString QgsServerOgcApiHandler::contentTypeForAccept( const QString &accept ) const
{
  const auto constContentTypes( QgsServerOgcApi::contentTypeMimes() );
  for ( auto it = constContentTypes.constBegin();
        it != constContentTypes.constEnd(); ++it )
  {
    const auto constValues = it.value();
    for ( const auto &value : constValues )
    {
      if ( accept.contains( value, Qt::CaseSensitivity::CaseInsensitive ) )
      {
        return value;
      }
    }
  }
  // Log level info because this is not completely unexpected
  QgsMessageLog::logMessage( u"Content type for accept %1 not found!"_s.arg( accept ), u"Server"_s, Qgis::MessageLevel::Info );

  return QString();
}

void QgsServerOgcApiHandler::write( json &data, const QgsServerApiContext &context, const json &htmlMetadata ) const
{
  const QgsServerOgcApi::ContentType contentType { contentTypeFromRequest( context.request() ) };
  switch ( contentType )
  {
    case QgsServerOgcApi::ContentType::HTML:
      data["handler"] = schema( context );
      if ( !htmlMetadata.is_null() )
      {
        data["metadata"] = htmlMetadata;
      }
      htmlDump( data, context );
      break;
    case QgsServerOgcApi::ContentType::GEOJSON:
    case QgsServerOgcApi::ContentType::JSON:
    case QgsServerOgcApi::ContentType::OPENAPI3:
      jsonDump( data, context, QgsServerOgcApi::contentTypeMimes().value( contentType ).first() );
      break;
    case QgsServerOgcApi::ContentType::XML:
      // Not handled yet
      break;
  }
}

void QgsServerOgcApiHandler::write( QVariant &data, const QgsServerApiContext &context, const QVariantMap &htmlMetadata ) const
{
  json j = QgsJsonUtils::jsonFromVariant( data );
  json jm = QgsJsonUtils::jsonFromVariant( htmlMetadata );
  QgsServerOgcApiHandler::write( j, context, jm );
}

std::string QgsServerOgcApiHandler::href( const QgsServerApiContext &context, const QString &extraPath, const QString &extension ) const
{
  QUrl url { context.request()->url() };
  QString urlBasePath { context.matchedPath() };
  const auto match { path().match( QgsServerOgcApi::sanitizeUrl( context.handlerPath() ).path() ) };
  if ( match.captured().count() > 0 )
  {
    url.setPath( urlBasePath + match.captured( 0 ) );
  }
  else
  {
    url.setPath( urlBasePath );
  }

  // Remove any existing extension
  const auto suffixLength { QFileInfo( url.path() ).suffix().length() };
  if ( suffixLength > 0 )
  {
    auto path { url.path() };
    path.truncate( path.length() - ( suffixLength + 1 ) );
    url.setPath( path );
  }

  // Add extra path
  url.setPath( url.path() + extraPath );

  // (re-)add extension
  // JSON is the default anyway so we don't need to add it
  if ( !extension.isEmpty() )
  {
    // Remove trailing slashes if any.
    QString path { url.path() };
    while ( path.endsWith( '/' ) )
    {
      path.chop( 1 );
    }
    url.setPath( path + '.' + extension );
  }
  return QgsServerOgcApi::sanitizeUrl( url ).toString( QUrl::FullyEncoded ).toStdString();
}

void QgsServerOgcApiHandler::jsonDump( json &data, const QgsServerApiContext &context, const QString &contentType ) const
{
  // Do not append timestamp to openapi
  if ( !QgsServerOgcApi::contentTypeMimes().value( QgsServerOgcApi::ContentType::OPENAPI3 ).contains( contentType, Qt::CaseSensitivity::CaseInsensitive ) )
  {
    QDateTime time { QDateTime::currentDateTime() };
    time.setTimeSpec( Qt::TimeSpec::UTC );
    data["timeStamp"] = time.toString( Qt::DateFormat::ISODate ).toStdString();
  }
  context.response()->setStatusCode( 200 );
  context.response()->setHeader( u"Content-Type"_s, contentType );
#ifdef QGISDEBUG
  context.response()->write( data.dump( 2 ) );
#else
  context.response()->write( data.dump() );
#endif
}

json QgsServerOgcApiHandler::schema( const QgsServerApiContext &context ) const
{
  Q_UNUSED( context )
  return nullptr;
}

json QgsServerOgcApiHandler::link( const QgsServerApiContext &context, const QgsServerOgcApi::Rel &linkType, const QgsServerOgcApi::ContentType contentType, const std::string &title ) const
{
  json l {
    { "href", href( context, "/", QgsServerOgcApi::contentTypeToExtension( contentType ) )
    },
    { "rel", QgsServerOgcApi::relToString( linkType ) },
    { "type", QgsServerOgcApi::mimeType( contentType ) },
    { "title", title != "" ? title : linkTitle() },
  };
  return l;
}

json QgsServerOgcApiHandler::links( const QgsServerApiContext &context ) const
{
  const QgsServerOgcApi::ContentType currentCt { contentTypeFromRequest( context.request() ) };
  json links = json::array();
  const QList<QgsServerOgcApi::ContentType> constCts { contentTypes() };
  for ( const auto &ct : constCts )
  {
    links.push_back( link( context, ( ct == currentCt ? QgsServerOgcApi::Rel::self : QgsServerOgcApi::Rel::alternate ), ct, linkTitle() + " as " + QgsServerOgcApi::contentTypeToStdString( ct ) ) );
  }
  return links;
}

QgsVectorLayer *QgsServerOgcApiHandler::layerFromContext( const QgsServerApiContext &context ) const
{
  if ( !context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( u"Project is invalid or undefined"_s );
  }
  // Check collectionId
  const QRegularExpressionMatch match { path().match( context.request()->url().path() ) };
  if ( !match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( u"Collection was not found"_s );
  }
  const QString collectionId { match.captured( u"collectionId"_s ) };
  // May throw if not found
  return layerFromCollectionId( context, collectionId );
}

const QString QgsServerOgcApiHandler::staticPath( const QgsServerApiContext &context ) const
{
  // resources/server/api + /static
  return context.serverInterface()->serverSettings()->apiResourcesDirectory() + u"/ogc/static"_s;
}

const QString QgsServerOgcApiHandler::templatePath( const QgsServerApiContext &context ) const
{
  // resources/server/api + /ogc/templates/ + operationId + .html
  QString path { context.serverInterface()->serverSettings()->apiResourcesDirectory() };
  path += "/ogc/templates"_L1;
  path += context.apiRootPath();
  path += '/';
  path += QString::fromStdString( operationId() );
  path += ".html"_L1;
  return path;
}


void QgsServerOgcApiHandler::htmlDump( const json &data, const QgsServerApiContext &context ) const
{
  context.response()->setHeader( u"Content-Type"_s, u"text/html"_s );
  auto path { templatePath( context ) };
  if ( !QFile::exists( path ) )
  {
    QgsMessageLog::logMessage( u"Template not found error: %1"_s.arg( path ), u"Server"_s, Qgis::MessageLevel::Critical );
    throw QgsServerApiBadRequestException( u"Template not found: %1"_s.arg( QFileInfo( path ).fileName() ) );
  }

  QFile f( path );
  if ( !f.open( QFile::ReadOnly | QFile::Text ) )
  {
    QgsMessageLog::logMessage( u"Could not open template file: %1"_s.arg( path ), u"Server"_s, Qgis::MessageLevel::Critical );
    throw QgsServerApiInternalServerError( u"Could not open template file: %1"_s.arg( QFileInfo( path ).fileName() ) );
  }

  try
  {
    // Get the template directory and the file name
    QFileInfo pathInfo { path };
    Environment env { QString( pathInfo.dir().path() + QDir::separator() ).toStdString() };

    // For template debugging:
    env.add_callback( "json_dump", 0, [data]( Arguments & ) {
      return data.dump();
    } );

    // Path manipulation: appends a directory path to the current url
    env.add_callback( "path_append", 1, [context]( Arguments &args ) {
      auto url { context.request()->url() };
      QFileInfo fi { url.path() };
      auto suffix { fi.suffix() };
      auto fName { fi.filePath() };
      if ( !suffix.isEmpty() )
      {
        fName.chop( suffix.length() + 1 );
      }
      // Chop any ending slashes
      while ( fName.endsWith( '/' ) )
      {
        fName.chop( 1 );
      }
      fName += '/' + QString::fromStdString( args.at( 0 )->get<std::string>() );
      if ( !suffix.isEmpty() )
      {
        fName += '.' + suffix;
      }
      fi.setFile( fName );
      url.setPath( fi.filePath() );
      return url.toString().toStdString();
    } );

    // Path manipulation: removes the specified number of directory components from the current url path
    env.add_callback( "path_chomp", 1, []( Arguments &args ) {
      QUrl url { QString::fromStdString( args.at( 0 )->get<std::string>() ) };
      QFileInfo fi { url.path() };
      auto suffix { fi.suffix() };
      auto fName { fi.filePath() };
      fName.chop( suffix.length() + 1 );
      // Chomp last segment
      const thread_local QRegularExpression segmentRx( R"raw(\/[^/]+$)raw" );
      fName = fName.replace( segmentRx, QString() );
      if ( !suffix.isEmpty() )
      {
        fName += '.' + suffix;
      }
      fi.setFile( fName );
      url.setPath( fi.filePath() );
      return url.toString().toStdString();
    } );

    // Returns filtered links from a link list
    // links_filter( <links>, <key>, <value> )
    env.add_callback( "links_filter", 3, []( Arguments &args ) {
      json links = args.at( 0 )->get<json>();
      if ( !links.is_array() )
      {
        links = json::array();
      }
      std::string key { args.at( 1 )->get<std::string>() };
      std::string value { args.at( 2 )->get<std::string>() };
      json result = json::array();
      for ( const auto &l : links )
      {
        if ( l[key] == value )
        {
          result.push_back( l );
        }
      }
      return result;
    } );

    // Returns a short name from content types
    env.add_callback( "content_type_name", 1, []( Arguments &args ) {
      const QgsServerOgcApi::ContentType ct { QgsServerOgcApi::contentTypeFromExtension( args.at( 0 )->get<std::string>() ) };
      return QgsServerOgcApi::contentTypeToStdString( ct );
    } );

    // Replace newlines with <br>
    env.add_callback( "nl2br", 1, []( Arguments &args ) {
      QString text { QString::fromStdString( args.at( 0 )->get<std::string>() ) };
      return text.replace( '\n', "<br>"_L1 ).toStdString();
    } );


    // Returns a list of parameter component data from components -> parameters by ref name
    // parameter( <ref object> )
    env.add_callback( "component_parameter", 1, [data]( Arguments &args ) {
      json ret = json::array();
      json ref = args.at( 0 )->get<json>();
      if ( !ref.is_object() )
      {
        return ret;
      }
      try
      {
        QString name = QString::fromStdString( ref["$ref"] );
        name = name.split( '/' ).last();
        ret.push_back( data["components"]["parameters"][name.toStdString()] );
      }
      catch ( std::exception & )
      {
        // Do nothing
      }
      return ret;
    } );


    // Static: returns the full URL to the specified static <path>
    env.add_callback( "static", 1, [context]( Arguments &args ) {
      auto asset( args.at( 0 )->get<std::string>() );
      QString matchedPath { context.matchedPath() };
      // If its the root path '/' strip it!
      if ( matchedPath == '/' )
      {
        matchedPath.clear();
      }
      return matchedPath.toStdString() + "/static/" + asset;
    } );


    // Returns true if a string begins with the provided string prefix, false otherwise
    env.add_callback( "starts_with", 2, []( Arguments &args ) {
      return string_view::starts_with( args.at( 0 )->get<std::string_view>(), args.at( 1 )->get<std::string_view>() );
    } );

    // Returns "null" string if object is null else string object representation
    env.add_callback( "if_nullptr_null_str", 1, []( Arguments &args ) {
      json jsonValue = args.at( 0 )->get<json>();
      std::string out;
      switch ( jsonValue.type() )
      {
        // avoid escaping string value
        case json::value_t::string:
          out = jsonValue.get<std::string>();
          break;

        case json::value_t::array:
        case json::value_t::object:
          if ( jsonValue.is_null() )
          {
            out = "null";
          }
          else
          {
            out = jsonValue.dump();
          }

          break;

          // use dump() for all other value types
        default:
          out = jsonValue.dump();
      }
      return out;
    } );

    context.response()->write( env.render_file( pathInfo.fileName().toStdString(), data ) );
  }
  catch ( std::exception &e )
  {
    QgsMessageLog::logMessage( u"Error parsing template file: %1 - %2"_s.arg( path, e.what() ), u"Server"_s, Qgis::MessageLevel::Critical );
    throw QgsServerApiInternalServerError( u"Error parsing template file: %1"_s.arg( e.what() ) );
  }
}

QgsServerOgcApi::ContentType QgsServerOgcApiHandler::contentTypeFromRequest( const QgsServerRequest *request ) const
{
  // Fallback to default
  QgsServerOgcApi::ContentType result { defaultContentType() };
  bool found { false };
  // First file extension ...
  const QString extension { QFileInfo( request->url().path() ).suffix().toUpper() };
  if ( !extension.isEmpty() )
  {
    static QMetaEnum metaEnum { QMetaEnum::fromType<QgsServerOgcApi::ContentType>() };
    bool ok { false };
    const int ct { metaEnum.keyToValue( extension.toLocal8Bit().constData(), &ok ) };
    if ( ok )
    {
      result = static_cast<QgsServerOgcApi::ContentType>( ct );
      found = true;
    }
    else
    {
      QgsMessageLog::logMessage( u"The client requested an unsupported extension: %1"_s.arg( extension ), u"Server"_s, Qgis::MessageLevel::Warning );
    }
  }
  // ... then "Accept"
  const QString accept { request->header( u"Accept"_s ) };
  if ( !found && !accept.isEmpty() )
  {
    const QString ctFromAccept { contentTypeForAccept( accept ) };
    if ( !ctFromAccept.isEmpty() )
    {
      const auto constContentTypes( QgsServerOgcApi::contentTypeMimes() );
      auto it = constContentTypes.constBegin();
      while ( !found && it != constContentTypes.constEnd() )
      {
        int idx = it.value().indexOf( ctFromAccept );
        if ( idx >= 0 )
        {
          found = true;
          result = it.key();
        }
        it++;
      }
    }
    else
    {
      QgsMessageLog::logMessage( u"The client requested an unsupported content type in Accept header: %1"_s.arg( accept ), u"Server"_s, Qgis::MessageLevel::Warning );
    }
  }
  // Validation: check if the requested content type (or an alias) is supported by the handler
  if ( !contentTypes().contains( result ) )
  {
    // Check aliases
    bool found { false };
    if ( QgsServerOgcApi::contentTypeAliases().contains( result ) )
    {
      const QList<QgsServerOgcApi::ContentType> constCt { contentTypes() };
      for ( const auto &ct : constCt )
      {
        if ( QgsServerOgcApi::contentTypeAliases()[result].contains( ct ) )
        {
          result = ct;
          found = true;
          break;
        }
      }
    }

    if ( !found )
    {
      QgsMessageLog::logMessage( u"Unsupported Content-Type: %1"_s.arg( QgsServerOgcApi::contentTypeToString( result ) ), u"Server"_s, Qgis::MessageLevel::Info );
      throw QgsServerApiBadRequestException( u"Unsupported Content-Type: %1"_s.arg( QgsServerOgcApi::contentTypeToString( result ) ) );
    }
  }
  return result;
}

QString QgsServerOgcApiHandler::parentLink( const QUrl &url, int levels )
{
  QString path { url.path() };
  const QFileInfo fi { path };
  const QString suffix { fi.suffix() };
  if ( !suffix.isEmpty() )
  {
    path.chop( suffix.length() + 1 );
  }
  while ( path.endsWith( '/' ) )
  {
    path.chop( 1 );
  }
  const thread_local QRegularExpression re( R"raw(\/[^/]+$)raw" );
  for ( int i = 0; i < levels; i++ )
  {
    path = path.replace( re, QString() );
  }
  QUrl result( url );
  QUrlQuery query( result );
  QList<QPair<QString, QString>> qi;
  const auto constItems { query.queryItems() };
  for ( const auto &i : constItems )
  {
    if ( i.first.compare( u"MAP"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
    {
      qi.push_back( i );
    }
  }
  // Make sure the parent link ends with a slash
  if ( !path.endsWith( '/' ) )
  {
    path.append( '/' );
  }
  QUrlQuery resultQuery;
  resultQuery.setQueryItems( qi );
  result.setQuery( resultQuery );
  result.setPath( path );
  return result.toString();
}

QgsVectorLayer *QgsServerOgcApiHandler::layerFromCollectionId( const QgsServerApiContext &context, const QString &collectionId )
{
  const auto mapLayers { context.project()->mapLayersByShortName<QgsVectorLayer *>( collectionId ) };
  if ( mapLayers.count() != 1 )
  {
    throw QgsServerApiNotFoundError( u"Collection with given id (%1) was not found or multiple matches were found"_s.arg( collectionId ) );
  }
  return mapLayers.first();
}

json QgsServerOgcApiHandler::defaultResponse()
{
  static json defRes = {
    { "description", "An error occurred." },
    { "content", { { "application/json", { { "schema", { { "$ref", "#/components/schemas/exception" } } } } }, { "text/html", { { "schema", { { "type", "string" } } } } } }
    }
  };
  return defRes;
}

json QgsServerOgcApiHandler::jsonTags() const
{
  return QgsJsonUtils::jsonFromVariant( tags() );
}

void QgsServerOgcApiHandler::setContentTypesInt( const QList<int> &contentTypes )
{
  mContentTypes.clear();
  for ( const int &i : std::as_const( contentTypes ) )
  {
    mContentTypes.push_back( static_cast<QgsServerOgcApi::ContentType>( i ) );
  }
}

void QgsServerOgcApiHandler::setContentTypes( const QList<QgsServerOgcApi::ContentType> &contentTypes )
{
  mContentTypes = contentTypes;
}
