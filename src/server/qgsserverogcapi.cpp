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

#include "qgsserverogcapi.h"
#include "qgsserverogcapihandler.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"

QMap<QgsServerOgcApi::ContentType, QString> QgsServerOgcApi::sContentTypeMime = [ ]() -> QMap<QgsServerOgcApi::ContentType, QString>
{
  QMap<QgsServerOgcApi::ContentType, QString> map;
  map[QgsServerOgcApi::ContentType::JSON] = QStringLiteral( "application/json" );
  map[QgsServerOgcApi::ContentType::GEOJSON] = QStringLiteral( "application/geo+json" );
  map[QgsServerOgcApi::ContentType::HTML] = QStringLiteral( "text/html" );
  map[QgsServerOgcApi::ContentType::OPENAPI3] = QStringLiteral( "application/openapi+json;version=3.0" );
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
  return url.adjusted( QUrl::StripTrailingSlash | QUrl::NormalizePathSegments );
}

void QgsServerOgcApi::executeRequest( const QgsServerApiContext &context ) const
{
  // Get url
  auto path { sanitizeUrl( context.request()->url() ).path() };
  // Find matching handler
  auto hasMatch { false };
  for ( const auto &h : mHandlers )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Checking API path %1 for %2 " ).arg( path, h->path().pattern() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
    if ( h->path().match( path ).hasMatch() )
    {
      hasMatch = true;
      // Execute handler
      QgsMessageLog::logMessage( QStringLiteral( "Found API handler %1" ).arg( QString::fromStdString( h->operationId() ) ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
      // May throw QgsServerApiBadRequestException or JSON exceptions on serializing
      try
      {
        h->handleRequest( context );
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

const QMap<QgsServerOgcApi::ContentType, QString> QgsServerOgcApi::contentTypeMimes()
{
  return sContentTypeMime;
}

const QHash<QgsServerOgcApi::ContentType, QList<QgsServerOgcApi::ContentType> > QgsServerOgcApi::contentTypeAliases()
{
  return sContentTypeAliases;
}

std::string QgsServerOgcApi::relToString( const Rel &rel )
{
  static QMetaEnum metaEnum = QMetaEnum::fromType<QgsServerOgcApi::Rel>();
  return metaEnum.valueToKey( rel );
}

QString QgsServerOgcApi::contentTypeToString( const ContentType &ct )
{
  static QMetaEnum metaEnum = QMetaEnum::fromType<ContentType>();
  QString result { metaEnum.valueToKey( ct ) };
  return result.replace( '_', '-' );
}

std::string QgsServerOgcApi::contentTypeToStdString( const ContentType &ct )
{
  static QMetaEnum metaEnum = QMetaEnum::fromType<ContentType>();
  return metaEnum.valueToKey( ct );
}

QString QgsServerOgcApi::contentTypeToExtension( const ContentType &ct )
{
  return contentTypeToString( ct ).toLower();
}

QgsServerOgcApi::ContentType QgsServerOgcApi::contenTypeFromExtension( const std::string &extension )
{
  return sContentTypeMime.key( QString::fromStdString( extension ) );
}

std::string QgsServerOgcApi::mimeType( const QgsServerOgcApi::ContentType &contentType )
{
  return sContentTypeMime.value( contentType, QString() ).toStdString();
}

const std::vector<std::shared_ptr<QgsServerOgcApiHandler> > QgsServerOgcApi::handlers() const
{
  return mHandlers;
}


