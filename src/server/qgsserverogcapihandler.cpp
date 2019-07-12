/***************************************************************************
  qgsserverogcapihandler.cpp - QgsServerOgcApiHandler

 ---------------------
 begin                : 10.7.2019
 copyright            : (C) 2019 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsjsonutils.h"
#include "qgsvectorlayer.h"

#include "qgsserverogcapihandler.h"
#include "qgsserverresponse.h"


#include "nlohmann/json.hpp"
#include "inja/inja.hpp"

using json = nlohmann::json;
using namespace inja;



QVariantMap QgsServerOgcApiHandler::validate( const QgsServerApiContext &context ) const
{
  QVariantMap result ;
  QVariantList positional;
  const auto constParameters { parameters() };
  for ( const auto &p : constParameters )
  {
    // value() calls the validators and throw an exception if validation fails
    result[p.name()] = p.value( context );
  }
  const auto match { path().match( context.request()->url().toString() ) };
  if ( match.hasMatch() )
  {
    const auto constNamed { path().namedCaptureGroups() };
    // Get named path parameters
    for ( const auto &name : constNamed )
    {
      if ( ! name.isEmpty() )
        result[name] = match.captured( name );
    }
    // Get unnamed (positional) path parameters
    for ( int i = 1; i < path().captureCount(); i++ )
    {
      positional.append( match.captured( i ) );
    }
  }
  result["path_arguments"] = positional;
  return result;
}

QgsServerOgcApiHandler::~QgsServerOgcApiHandler()
{
  //qDebug() << "handler destroyed";
}

QString QgsServerOgcApiHandler::contentTypeForAccept( const QString &accept ) const
{

  QString result;
  const auto constMimes { QgsServerOgcApi::contentTypeMimes() };
  for ( const auto &ct : constMimes )
  {
    if ( accept.contains( ct ) )
    {
      result = ct;
      break;
    }
  }
  return result;

}

void QgsServerOgcApiHandler::write( json &data, const QgsServerApiContext &context, const json &htmlMetadata ) const
{
  // TODO: accept GML and XML?
  const auto contentType { contentTypeFromRequest( context.request() ) };
  switch ( contentType )
  {
    case QgsServerOgcApi::ContentType::HTML:
      data["handler"] = handlerData( );
      if ( ! htmlMetadata.is_null() )
      {
        data["metadata"] = htmlMetadata;
      }
      htmlDump( data, context );
      break;
    case QgsServerOgcApi::ContentType::GEOJSON:
    case QgsServerOgcApi::ContentType::JSON:
    case QgsServerOgcApi::ContentType::OPENAPI3:
      jsonDump( data, context, QgsServerOgcApi::contentTypeMimes().value( contentType ) );
      break;
    case QgsServerOgcApi::ContentType::GML:
    case QgsServerOgcApi::ContentType::XML:
      throw QgsServerApiNotImplementedException( QStringLiteral( "Requested content type is not yet implemented" ) );
  }
}
void QgsServerOgcApiHandler::write( QVariant &data, const QgsServerApiContext &context, const QVariantMap &htmlMetadata ) const
{
  json j { QgsJsonUtils::jsonFromVariant( data ) };
  json jm { QgsJsonUtils::jsonFromVariant( htmlMetadata ) };
  QgsServerOgcApiHandler::write( j, context, jm );
}

std::string QgsServerOgcApiHandler::href( const QgsServerApiContext &context, const QString &extraPath, const QString &extension ) const
{
  QUrl url { context.request()->url() };
  QString urlBasePath { context.matchedPath() };
  const auto match { path().match( url.path() ) };
  if ( match.captured().count() > 0 )
  {
    url.setPath( urlBasePath + match.captured( 0 ) );
  }
  else
  {
    url.setPath( urlBasePath );
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
  return QgsServerOgcApi::sanitizeUrl( url ).toString( QUrl::FullyEncoded ).toStdString();

}

void QgsServerOgcApiHandler::jsonDump( json &data, const QgsServerApiContext &context, const QString &contentType ) const
{
  QDateTime time { QDateTime::currentDateTime() };
  time.setTimeSpec( Qt::TimeSpec::UTC );
  data["timeStamp"] = time.toString( Qt::DateFormat::ISODate ).toStdString() ;
  context.response()->setHeader( QStringLiteral( "Content-Type" ), contentType );
#ifdef QGISDEBUG
  context.response()->write( data.dump( 2 ) );
#else
  context.response()->write( data.dump( ) );
#endif
}

json QgsServerOgcApiHandler::handlerData() const
{
  json data;
  data["linkTitle"] = linkTitle();
  data["operationId"] = operationId();
  data["description"] = description();
  data["summary"] = summary();
  return data;
}

json QgsServerOgcApiHandler::link( const QgsServerApiContext &context, const QgsServerOgcApi::Rel &linkType, const QgsServerOgcApi::ContentType contentType, const std::string &title ) const
{
  json l
  {
    {
      "href", href( context, "/",
                    QgsServerOgcApi::contentTypeToExtension( contentType ) )
    },
    { "rel", QgsServerOgcApi::relToString( linkType ) },
    { "type", QgsServerOgcApi::mimeType( contentType ) },
    { "title", title != "" ? title : linkTitle() },
  };
  return l;
}

json QgsServerOgcApiHandler::links( const QgsServerApiContext &context ) const
{
  const auto currentCt { contentTypeFromRequest( context.request() ) };
  json links = json::array();
  const auto constCts { contentTypes() };
  for ( const auto &ct : constCts )
  {
    links.push_back( link( context, ( ct == currentCt ? QgsServerOgcApi::Rel::self :
                                      QgsServerOgcApi::Rel::alternate ), ct,
                           linkTitle()  + " as " + QgsServerOgcApi::contentTypeToStdString( ct ) ) );
  }
  return links;
}

const QString QgsServerOgcApiHandler::staticPath( ) const
{
  // resources/server/api + /ogc/static
  return QgsServerOgcApi::resourcesPath() + QStringLiteral( "/ogc/static" );
}

const QString QgsServerOgcApiHandler::templatePath( const QgsServerApiContext &context ) const
{
  // resources/server/api + /ogc/templates/ + operationId + .html
  auto path { QgsServerOgcApi::resourcesPath() };
  path += QStringLiteral( "/ogc/templates" );
  path += context.apiRootPath();
  path += '/';
  path += QString::fromStdString( operationId() );
  path += QStringLiteral( ".html" );
  return path;
}


void QgsServerOgcApiHandler::htmlDump( const json &data, const QgsServerApiContext &context ) const
{
  context.response()->setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/html" ) );
  auto path { templatePath( context ) };
  if ( ! QFile::exists( path ) )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Template not found error: %1" ).arg( path ), QStringLiteral( "Server" ), Qgis::Critical );
    throw QgsServerApiBadRequestException( QStringLiteral( "Template not found: %1" ).arg( QFileInfo( path ).fileName() ) );
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

    // Path manipulation: appends a directory path to the current url
    env.add_callback( "path_append", 1, [ = ]( Arguments & args )
    {
      auto url { context.request()->url() };
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

    // Path manipulation: removes the specified number of directory components from the current url path
    env.add_callback( "path_chomp", 1, [ = ]( Arguments & args )
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

    // Returns filtered links from a link list
    // links_filter( <links>, <key>, <value> )
    env.add_callback( "links_filter", 3, [ = ]( Arguments & args )
    {
      json links { args.at( 0 )->get<json>( ) };
      std::string key { args.at( 1 )->get<std::string>( ) };
      std::string value { args.at( 2 )->get<std::string>( ) };
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
    env.add_callback( "content_type_name", 1, [ = ]( Arguments & args )
    {
      const QgsServerOgcApi::ContentType ct { QgsServerOgcApi::contenTypeFromExtension( args.at( 0 )->get<std::string>( ) ) };
      return QgsServerOgcApi::contentTypeToStdString( ct );
    } );


    // Static: returns the full URL to the specified static <path>
    env.add_callback( "static", 1, [ = ]( Arguments & args )
    {
      auto asset( args.at( 0 )->get<std::string>( ) );
      return context.matchedPath().toStdString() + "/static/" + asset;
    } );

    context.response()->write( env.render_file( pathInfo.fileName().toStdString(), data ) );
  }
  catch ( std::exception &e )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Error parsing template file: %1 - %2" ).arg( path, e.what() ), QStringLiteral( "Server" ), Qgis::Critical );
    throw QgsServerApiInternalServerError( QStringLiteral( "Error parsing template file: %1" ).arg( e.what() ) );
  }
}
QgsServerOgcApi::ContentType QgsServerOgcApiHandler::contentTypeFromRequest( const QgsServerRequest *request ) const
{
  // Fallback to default
  QgsServerOgcApi::ContentType result { defaultContentType() };
  bool found { false };
  // First file extension ...
  const auto extension { QFileInfo( request->url().path() ).completeSuffix().toUpper() };
  if ( ! extension.isEmpty() )
  {
    static QMetaEnum metaEnum { QMetaEnum::fromType<QgsServerOgcApi::ContentType>() };
    auto ok { false };
    const auto ct  { metaEnum.keyToValue( extension.toLocal8Bit().constData(), &ok ) };
    if ( ok )
    {
      result = static_cast<QgsServerOgcApi::ContentType>( ct );
      found = true;
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "The client requested an unsupported extension: %1" ).arg( extension ), QStringLiteral( "Server" ), Qgis::Warning );
    }
  }
  // ... then "Accept"
  const auto accept { request->header( QStringLiteral( "Accept" ) ) };
  if ( ! found && ! accept.isEmpty() )
  {
    const auto ctFromAccept { contentTypeForAccept( accept ) };
    if ( ! ctFromAccept.isEmpty() )
    {
      result = QgsServerOgcApi::contentTypeMimes().key( ctFromAccept );
      found = true;
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "The client requested an unsupported content type in Accept header: %1" ).arg( accept ), QStringLiteral( "Server" ), Qgis::Warning );
    }
  }
  // Validation: check if the requested content type (or an alias) is supported by the handler
  if ( ! contentTypes().contains( result ) )
  {
    // Check aliases
    bool found { false };
    if ( QgsServerOgcApi::contentTypeAliases().keys().contains( result ) )
    {
      const auto constCt { contentTypes() };
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

    if ( ! found )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Unsupported Content-Type: %1" ).arg( QgsServerOgcApi::contentTypeToString( result ) ), QStringLiteral( "Server" ), Qgis::Info );
      throw QgsServerApiBadRequestException( QStringLiteral( "Unsupported Content-Type: %1" ).arg( QgsServerOgcApi::contentTypeToString( result ) ) );
    }
  }
  return result;
}

std::string QgsServerOgcApiHandler::parentLink( const QUrl &url, int levels )
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

QgsVectorLayer *QgsServerOgcApiHandler::layerFromCollection( const QgsServerApiContext &context, const QString &collectionId )
{
  const auto mapLayers { context.project()->mapLayersByShortName<QgsVectorLayer *>( collectionId ) };
  if ( mapLayers.count() != 1 )
  {
    throw QgsServerApiImproperlyConfiguredException( QStringLiteral( "Collection with given id (%1) was not found or multiple matches were found" ).arg( collectionId ) );
  }
  return mapLayers.first();
}
