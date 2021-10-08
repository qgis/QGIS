/***************************************************************************
  qgsserverogcapi.cpp - QgsServerOgcApi

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

#include <QDir>
#include <QDebug>
#include <QtGlobal>

#include "qgsserverogcapi.h"
#include "qgsserverogcapihandler.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"

QMap<QgsServerOgcApi::ContentType, QStringList> QgsServerOgcApi::sContentTypeMime = [ ]() -> QMap<QgsServerOgcApi::ContentType, QStringList>
{
  QMap<QgsServerOgcApi::ContentType, QStringList> map;
  map[QgsServerOgcApi::ContentType::JSON] = QStringList { QStringLiteral( "application/json" ) };
  map[QgsServerOgcApi::ContentType::GEOJSON] = QStringList {
    QStringLiteral( "application/geo+json" ),
    QStringLiteral( "application/vnd.geo+json" ),
    QStringLiteral( "application/geojson" )
  };
  map[QgsServerOgcApi::ContentType::HTML] = QStringList { QStringLiteral( "text/html" ) };
  map[QgsServerOgcApi::ContentType::OPENAPI3] = QStringList { QStringLiteral( "application/vnd.oai.openapi+json;version=3.0" ) };
  map[QgsServerOgcApi::ContentType::XML] = QStringList { QStringLiteral( "application/xml" ) };
  return map;
}();

QHash<QgsServerOgcApi::ContentType, QList<QgsServerOgcApi::ContentType>> QgsServerOgcApi::sContentTypeAliases = [ ]() -> QHash<ContentType, QList<ContentType>>
{
  QHash<QgsServerOgcApi::ContentType, QList<QgsServerOgcApi::ContentType>> map;
  map[ContentType::JSON] = { QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::ContentType::OPENAPI3 };
  return map;
}();


QgsServerOgcApi::QgsServerOgcApi( QgsServerInterface *serverIface, const QString &rootPath, const QString &name, const QString &description, const QString &version ):
  QgsServerApi( serverIface ),
  mRootPath( rootPath ),
  mName( name ),
  mDescription( description ),
  mVersion( version )
{

}

QgsServerOgcApi::~QgsServerOgcApi()
{
  //qDebug() << "API destroyed: " << name();
}

void QgsServerOgcApi::registerHandler( QgsServerOgcApiHandler *handler )
{
  std::shared_ptr<QgsServerOgcApiHandler> hp( handler );
  mHandlers.emplace_back( std::move( hp ) );
}

QUrl QgsServerOgcApi::sanitizeUrl( const QUrl &url )
{
  // Since QT 5.12 NormalizePathSegments does not collapse double slashes
  QUrl u { url.adjusted( QUrl::StripTrailingSlash | QUrl::NormalizePathSegments ) };
  if ( u.path().contains( QLatin1String( "//" ) ) )
  {
    u.setPath( u.path().replace( QLatin1String( "//" ), QChar( '/' ) ) );
  }
  // Make sure the path starts with '/'
  if ( !u.path().startsWith( '/' ) )
  {
    u.setPath( u.path().prepend( '/' ) );
  }
  return u;
}

void QgsServerOgcApi::executeRequest( const QgsServerApiContext &context ) const
{
  // Get url
  const auto path { sanitizeUrl( context.handlerPath( ) ).path() };
  // Find matching handler
  auto hasMatch { false };
  for ( const auto &handler : mHandlers )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Checking API path %1 for %2 " ).arg( path, handler->path().pattern() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
    if ( handler->path().match( path ).hasMatch() )
    {
      hasMatch = true;
      // Execute handler
      QgsMessageLog::logMessage( QStringLiteral( "API %1: found handler %2" ).arg( name(), QString::fromStdString( handler->operationId() ) ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
      // May throw QgsServerApiBadRequestException or JSON exceptions on serializing
      try
      {
        handler->handleRequest( context );
      }
      catch ( json::exception &ex )
      {
        throw QgsServerApiInternalServerError( QStringLiteral( "The API handler returned an error: %1" ).arg( ex.what() ) );
      }
      break;
    }
  }
  // Throw
  if ( ! hasMatch )
  {
    throw QgsServerApiBadRequestException( QStringLiteral( "Requested URI does not match any registered API handler" ) );
  }
}

const QMap<QgsServerOgcApi::ContentType, QStringList> QgsServerOgcApi::contentTypeMimes()
{
  return sContentTypeMime;
}

const QHash<QgsServerOgcApi::ContentType, QList<QgsServerOgcApi::ContentType> > QgsServerOgcApi::contentTypeAliases()
{
  return sContentTypeAliases;
}

std::string QgsServerOgcApi::relToString( const Rel &rel )
{
  static const QMetaEnum metaEnum = QMetaEnum::fromType<QgsServerOgcApi::Rel>();
  std::string val { metaEnum.valueToKey( rel ) };
  std::replace( val.begin(), val.end(), '_', '-' );
  return val;
}

QString QgsServerOgcApi::contentTypeToString( const ContentType &ct )
{
  static const QMetaEnum metaEnum = QMetaEnum::fromType<ContentType>();
  QString result { metaEnum.valueToKey( ct ) };
  return result.replace( '_', '-' );
}

std::string QgsServerOgcApi::contentTypeToStdString( const ContentType &ct )
{
  static const QMetaEnum metaEnum = QMetaEnum::fromType<ContentType>();
  return metaEnum.valueToKey( ct );
}

QString QgsServerOgcApi::contentTypeToExtension( const ContentType &ct )
{
  return contentTypeToString( ct ).toLower();
}

QgsServerOgcApi::ContentType QgsServerOgcApi::contenTypeFromExtension( const std::string &extension )
{
  const QString exts = QString::fromStdString( extension );
  const auto constMimeTypes( QgsServerOgcApi::contentTypeMimes() );
  for ( auto it = constMimeTypes.constBegin();
        it != constMimeTypes.constEnd();
        ++it )
  {
    const auto constValues = it.value();
    for ( const auto &value : constValues )
    {
      if ( value.contains( exts, Qt::CaseSensitivity::CaseInsensitive ) )
      {
        return it.key();
      }
    }
  }
  // Default to JSON, but log a warning!
  QgsMessageLog::logMessage( QStringLiteral( "Content type for extension %1 not found! Returning default (JSON)" ).arg( exts ),
                             QStringLiteral( "Server" ),
                             Qgis::MessageLevel::Warning );
  return QgsServerOgcApi::ContentType::JSON;
}

std::string QgsServerOgcApi::mimeType( const QgsServerOgcApi::ContentType &contentType )
{
  if ( ! sContentTypeMime.contains( contentType ) )
  {
    return "";
  }
  return sContentTypeMime.value( contentType ).first().toStdString();
}

const std::vector<std::shared_ptr<QgsServerOgcApiHandler> > QgsServerOgcApi::handlers() const
{
  return mHandlers;
}


