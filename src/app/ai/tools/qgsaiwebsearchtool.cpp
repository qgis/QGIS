/***************************************************************************
    qgsaiwebsearchtool.cpp
    ----------------------
    begin                : July 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaiwebsearchtool.h"

#include "qgsaimodelrouter.h"
#include "qgsaiplanclient.h"
#include "qgsaitoolschemautil.h"
#include "qgsnetworkaccessmanager.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTimer>

using namespace Qt::StringLiterals;

namespace
{
  constexpr int SEARCH_TIMEOUT_MS = 20000;

  QJsonObject integerProp( const QString &description, int minimum, int maximum )
  {
    QJsonObject p = prop( u"integer"_s, description );
    p.insert( u"minimum"_s, minimum );
    p.insert( u"maximum"_s, maximum );
    return p;
  }
} //namespace

QgsAiWebSearchToolBase::QgsAiWebSearchToolBase( QgsAiModelRouter *router )
  : mRouter( router )
{}

bool QgsAiWebSearchToolBase::isAvailable() const
{
  return mRouter && QgsAiModelRouter::isUsablePlanEndpoint( mRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).endpoint ) && !mRouter->planSessionToken().trimmed().isEmpty();
}

QString QgsAiWebSearchToolBase::availabilityReason() const
{
  if ( !mRouter )
    return u"Strata Plan router is not configured."_s;
  if ( !QgsAiModelRouter::isUsablePlanEndpoint( mRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).endpoint ) )
    return u"Strata Plan endpoint is not configured."_s;
  return u"Sign in to Strata Plan to use managed online search tools."_s;
}

QgsAiToolResult QgsAiWebSearchToolBase::postSearch( const QString &path, const QJsonObject &body ) const
{
  if ( !isAvailable() )
    return QgsAiToolResult::error( availabilityReason() );

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
    return QgsAiToolResult::error( u"Network manager is not available."_s );

  const QString apiBase = QgsAiPlanClient::apiBaseForChatEndpoint( mRouter->providerSettings( QgsAiModelRouter::Provider::Plan ).endpoint );
  if ( apiBase.isEmpty() )
    return QgsAiToolResult::error( u"Cannot derive Strata Plan API base from the configured chat endpoint."_s );

  QNetworkRequest request( QUrl( apiBase + path ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
  request.setRawHeader( "Accept", "application/json" );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( mRouter->planSessionToken().trimmed() ) ).toUtf8() );
  request.setTransferTimeout( SEARCH_TIMEOUT_MS );

  QNetworkReply *reply = networkManager->post( request, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );
  if ( !reply )
    return QgsAiToolResult::error( u"Unable to start the Strata Plan search request."_s );

  QEventLoop loop;
  QTimer timer;
  timer.setSingleShot( true );
  QObject::connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
  QObject::connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
  timer.start( SEARCH_TIMEOUT_MS );
  loop.exec();

  const bool timedOut = !timer.isActive();
  if ( timedOut )
    reply->abort();
  const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QByteArray responseBytes = reply->readAll();
  const QNetworkReply::NetworkError networkError = reply->error();
  reply->deleteLater();

  if ( timedOut )
    return QgsAiToolResult::error( u"Strata Plan search request timed out."_s );
  if ( networkError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    return QgsAiToolResult::error( u"Strata Plan search failed (HTTP %1): %2"_s.arg( httpStatus ).arg( QString::fromUtf8( responseBytes.left( 500 ) ) ) );

  const QJsonDocument doc = QJsonDocument::fromJson( responseBytes );
  if ( !doc.isObject() )
    return QgsAiToolResult::error( u"Strata Plan search returned a non-JSON response."_s );
  return QgsAiToolResult::ok( doc.object() );
}

QString QgsAiWebSearchTool::description() const
{
  return u"Runs a managed metadata-only web search through Strata Plan. Use this for online discovery; do not treat results as trusted catalog data unless catalog_search classifies them."_s;
}

QJsonObject QgsAiWebSearchTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"query"_s, prop( u"string"_s, u"Search query."_s ) );
  properties.insert( u"limit"_s, integerProp( u"Maximum number of results, 1-10. Default 5."_s, 1, 10 ) );
  return schemaObject( properties, QJsonArray { u"query"_s } );
}

QgsAiToolResult QgsAiWebSearchTool::execute( const QJsonObject &args )
{
  const QString query = args.value( u"query"_s ).toString().trimmed();
  if ( query.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'query' is required."_s );
  QJsonObject body;
  body.insert( u"query"_s, query );
  if ( args.contains( u"limit"_s ) )
    body.insert( u"limit"_s, args.value( u"limit"_s ).toInt() );
  return postSearch( u"/v1/tools/web-search"_s, body );
}

QString QgsAiCatalogSearchTool::description() const
{
  return u"Searches trusted GIS data catalogs through Strata Plan and returns source trust, host, likely format, license when known, URL and reason. Prefer this before download_file for remote layer data."_s;
}

QJsonObject QgsAiCatalogSearchTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"query"_s, prop( u"string"_s, u"GIS dataset or layer query."_s ) );
  properties.insert( u"limit"_s, integerProp( u"Maximum number of results, 1-10. Default 5."_s, 1, 10 ) );
  properties.insert( u"trustedOnly"_s, prop( u"boolean"_s, u"If true, filters out unknown source hosts. Default true."_s ) );
  return schemaObject( properties, QJsonArray { u"query"_s } );
}

QgsAiToolResult QgsAiCatalogSearchTool::execute( const QJsonObject &args )
{
  const QString query = args.value( u"query"_s ).toString().trimmed();
  if ( query.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'query' is required."_s );
  QJsonObject body;
  body.insert( u"query"_s, query );
  body.insert( u"trustedOnly"_s, args.value( u"trustedOnly"_s ).toBool( true ) );
  if ( args.contains( u"limit"_s ) )
    body.insert( u"limit"_s, args.value( u"limit"_s ).toInt() );
  return postSearch( u"/v1/tools/catalog-search"_s, body );
}
