/***************************************************************************
    qgsaiopenroutermodelcatalog.cpp
    ---------------------
    begin                : June 2026
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

#include "qgsaiopenroutermodelcatalog.h"

#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

#include "moc_qgsaiopenroutermodelcatalog.cpp"

using namespace Qt::StringLiterals;

namespace
{
  constexpr const char *OPENROUTER_API_BASE = "https://openrouter.ai/api/v1";
  constexpr int FETCH_TIMEOUT_MS = 20000;
} //namespace

QString QgsAiOpenRouterModelCatalog::ModelInfo::displayLabel() const
{
  const QString displayName = name.isEmpty() ? id : name;
  QString label = displayName;
  if ( contextLength > 0 )
    label += u" — %1k ctx"_s.arg( contextLength / 1000 );
  if ( promptUsdPerMTok > 0 || completionUsdPerMTok > 0 )
    label += u" — $%1/M in, $%2/M out"_s.arg( promptUsdPerMTok, 0, 'g', 3 ).arg( completionUsdPerMTok, 0, 'g', 3 );
  return label;
}

QgsAiOpenRouterModelCatalog::QgsAiOpenRouterModelCatalog( QObject *parent )
  : QObject( parent )
{}

QString QgsAiOpenRouterModelCatalog::apiBase() const
{
  return mApiBaseOverride.isEmpty() ? QString::fromUtf8( OPENROUTER_API_BASE ) : mApiBaseOverride;
}

QString QgsAiOpenRouterModelCatalog::cacheFilePath() const
{
  if ( !mCacheFilePathOverride.isEmpty() )
    return mCacheFilePathOverride;
  return QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"openrouter_models_cache.json"_s );
}

QList<QgsAiOpenRouterModelCatalog::ModelInfo> QgsAiOpenRouterModelCatalog::parseModelsJson( const QByteArray &body )
{
  QList<ModelInfo> models;
  const QJsonDocument doc = QJsonDocument::fromJson( body );
  if ( !doc.isObject() )
    return models;

  const QJsonArray data = doc.object().value( u"data"_s ).toArray();
  models.reserve( data.size() );
  for ( const QJsonValue &item : data )
  {
    const QJsonObject obj = item.toObject();

    // The endpoint is queried with ?supported_parameters=tools, but re-filter
    // client-side: a model without tool support breaks the agent loop.
    const QJsonArray supportedParameters = obj.value( u"supported_parameters"_s ).toArray();
    bool supportsTools = supportedParameters.isEmpty(); // tolerate responses that omit the field
    for ( const QJsonValue &parameter : supportedParameters )
    {
      if ( parameter.toString() == "tools"_L1 )
      {
        supportsTools = true;
        break;
      }
    }
    if ( !supportsTools )
      continue;

    ModelInfo info;
    info.id = obj.value( u"id"_s ).toString();
    if ( info.id.isEmpty() )
      continue;
    info.name = obj.value( u"name"_s ).toString();
    info.contextLength = obj.value( u"context_length"_s ).toInt();

    // Pricing comes as USD-per-token strings (e.g. "0.000003").
    const QJsonObject pricing = obj.value( u"pricing"_s ).toObject();
    info.promptUsdPerMTok = pricing.value( u"prompt"_s ).toString().toDouble() * 1e6;
    info.completionUsdPerMTok = pricing.value( u"completion"_s ).toString().toDouble() * 1e6;
    models.append( info );
  }
  return models;
}

QList<QgsAiOpenRouterModelCatalog::ModelInfo> QgsAiOpenRouterModelCatalog::curatedFallback()
{
  // Strong tool-calling models, refreshed manually when defaults change.
  const auto entry = []( const QString &id, const QString &name, int contextLength ) {
    ModelInfo info;
    info.id = id;
    info.name = name;
    info.contextLength = contextLength;
    return info;
  };
  return {
    entry( u"anthropic/claude-sonnet-4.6"_s, u"Claude Sonnet 4.6"_s, 200000 ),
    entry( u"openrouter/auto"_s, u"OpenRouter Auto"_s, 0 ),
    entry( u"anthropic/claude-opus-4.7"_s, u"Claude Opus 4.7"_s, 200000 ),
    entry( u"openai/gpt-5.1"_s, u"GPT-5.1"_s, 400000 ),
    entry( u"google/gemini-3.5-flash"_s, u"Gemini 3.5 Flash"_s, 1000000 ),
    entry( u"deepseek/deepseek-v4-flash"_s, u"DeepSeek V4 Flash"_s, 1000000 ),
  };
}

bool QgsAiOpenRouterModelCatalog::readCache( QList<ModelInfo> &models, qint64 &fetchedAtSecs ) const
{
  QFile file( cacheFilePath() );
  if ( !file.open( QIODevice::ReadOnly ) )
    return false;

  const QJsonDocument doc = QJsonDocument::fromJson( file.readAll() );
  if ( !doc.isObject() )
    return false;

  fetchedAtSecs = doc.object().value( u"fetchedAt"_s ).toVariant().toLongLong();
  models.clear();
  const QJsonArray cached = doc.object().value( u"models"_s ).toArray();
  models.reserve( cached.size() );
  for ( const QJsonValue &item : cached )
  {
    const QJsonObject obj = item.toObject();
    ModelInfo info;
    info.id = obj.value( u"id"_s ).toString();
    if ( info.id.isEmpty() )
      continue;
    info.name = obj.value( u"name"_s ).toString();
    info.contextLength = obj.value( u"contextLength"_s ).toInt();
    info.promptUsdPerMTok = obj.value( u"promptUsdPerMTok"_s ).toDouble();
    info.completionUsdPerMTok = obj.value( u"completionUsdPerMTok"_s ).toDouble();
    models.append( info );
  }
  return !models.isEmpty();
}

void QgsAiOpenRouterModelCatalog::writeCache( const QList<ModelInfo> &models ) const
{
  QJsonArray serialized;
  for ( const ModelInfo &info : models )
  {
    QJsonObject obj;
    obj.insert( u"id"_s, info.id );
    obj.insert( u"name"_s, info.name );
    obj.insert( u"contextLength"_s, info.contextLength );
    obj.insert( u"promptUsdPerMTok"_s, info.promptUsdPerMTok );
    obj.insert( u"completionUsdPerMTok"_s, info.completionUsdPerMTok );
    serialized.append( obj );
  }
  QJsonObject root;
  root.insert( u"fetchedAt"_s, QDateTime::currentSecsSinceEpoch() );
  root.insert( u"models"_s, serialized );

  QFile file( cacheFilePath() );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    file.write( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
}

void QgsAiOpenRouterModelCatalog::refresh( bool force )
{
  QList<ModelInfo> cached;
  qint64 fetchedAtSecs = 0;
  const bool hasCache = readCache( cached, fetchedAtSecs );
  const bool cacheFresh = hasCache && ( QDateTime::currentSecsSinceEpoch() - fetchedAtSecs ) < CACHE_TTL_SECONDS;

  if ( cacheFresh && !force )
  {
    emit modelsReady( cached, true );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit modelsReady( hasCache ? cached : curatedFallback(), true );
    return;
  }

  QNetworkRequest request( QUrl( apiBase() + u"/models?supported_parameters=tools"_s ) );
  request.setTransferTimeout( FETCH_TIMEOUT_MS );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit modelsReady( hasCache ? cached : curatedFallback(), true );
    return;
  }

  // The reply must clean itself up even when this catalog (dialog-owned) is
  // destroyed before the response arrives.
  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply, cached, hasCache]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    if ( reply->error() == QNetworkReply::NoError && httpStatus >= 200 && httpStatus < 300 )
    {
      const QList<ModelInfo> models = parseModelsJson( reply->readAll() );
      if ( !models.isEmpty() )
      {
        writeCache( models );
        emit modelsReady( models, false );
        return;
      }
    }
    QgsMessageLog::
      logMessage( u"OpenRouter model catalog fetch failed (HTTP %1); using %2."_s.arg( httpStatus ).arg( hasCache ? u"stale cache"_s : u"curated fallback list"_s ), u"AI"_s, Qgis::MessageLevel::Warning, false );
    emit modelsReady( hasCache ? cached : curatedFallback(), true );
  } );
}

void QgsAiOpenRouterModelCatalog::fetchKeyInfo( const QString &apiKey )
{
  const QString key = apiKey.trimmed();
  if ( key.isEmpty() )
  {
    emit keyInfoFailed( tr( "No API key configured." ) );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit keyInfoFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( QUrl( apiBase() + u"/key"_s ) );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( key ) ).toUtf8() );
  request.setTransferTimeout( FETCH_TIMEOUT_MS );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit keyInfoFailed( tr( "Unable to start the request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray body = reply->readAll();

    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      if ( httpStatus == 401 || httpStatus == 403 )
        emit keyInfoFailed( tr( "The API key is invalid or disabled (HTTP %1)." ).arg( httpStatus ) );
      else
        emit keyInfoFailed( tr( "Connection test failed (HTTP %1)." ).arg( httpStatus ) );
      return;
    }

    const QJsonObject data = QJsonDocument::fromJson( body ).object().value( u"data"_s ).toObject();
    if ( data.isEmpty() )
    {
      emit keyInfoFailed( tr( "Unexpected response from OpenRouter." ) );
      return;
    }

    const QString label = data.value( u"label"_s ).toString();
    const double usage = data.value( u"usage"_s ).toDouble();
    QString summary = label.isEmpty() ? tr( "Connected." ) : tr( "Connected as \"%1\"." ).arg( label );
    summary += u" "_s + tr( "Usage: $%1." ).arg( usage, 0, 'f', 2 );
    if ( !data.value( u"limit"_s ).isNull() )
    {
      const double limit = data.value( u"limit"_s ).toDouble();
      const double remaining = data.value( u"limit_remaining"_s ).toDouble();
      summary += u" "_s + tr( "Credits remaining: $%1 of $%2." ).arg( remaining, 0, 'f', 2 ).arg( limit, 0, 'f', 2 );
    }
    emit keyInfoReady( summary );
  } );
}
