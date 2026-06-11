/***************************************************************************
    qgsaiembeddingclient.cpp
    ---------------------
    begin                : April 2026
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

#include "qgsaiembeddingclient.h"

#include "ai/qgsaisecretstore.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

#include <algorithm>

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTimer>
#include <QUrl>

#include "moc_qgsaiembeddingclient.cpp"

using namespace Qt::StringLiterals;

namespace
{
  constexpr const char *OPENAI_EMBEDDINGS_ENDPOINT = "https://api.openai.com/v1/embeddings";
  constexpr const char *OPENROUTER_EMBEDDINGS_ENDPOINT = "https://openrouter.ai/api/v1/embeddings";
  constexpr const char *OPENAI_KEY_SETTING = "ai/provider/openai/apiKey";
  constexpr const char *OPENROUTER_KEY_SETTING = "ai/provider/openrouter/apiKey";
  constexpr const char *EMBEDDINGS_PROVIDER_SETTING = "ai/embeddings/provider";
  constexpr const char *OPENAI_EMBEDDING_MODEL_SETTING = "ai/embeddings/openai/model";
  constexpr const char *OPENROUTER_EMBEDDING_MODEL_SETTING = "ai/embeddings/openrouter/model";
} //namespace

QgsAiEmbeddingClient::QgsAiEmbeddingClient( QObject *parent )
  : QObject( parent )
{}

QgsAiEmbeddingClient::Provider QgsAiEmbeddingClient::provider() const
{
  if ( mHasProviderOverride )
    return mProviderOverride;

  const QgsSettings settings;
  const QString configured = settings.value( QString::fromLatin1( EMBEDDINGS_PROVIDER_SETTING ), u"openai"_s ).toString().trimmed();
  return configured.compare( u"openrouter"_s, Qt::CaseInsensitive ) == 0 ? Provider::OpenRouter : Provider::OpenAi;
}

QString QgsAiEmbeddingClient::endpoint() const
{
  if ( !mEndpointOverride.isEmpty() )
    return mEndpointOverride;
  return provider() == Provider::OpenRouter ? QString::fromLatin1( OPENROUTER_EMBEDDINGS_ENDPOINT ) : QString::fromLatin1( OPENAI_EMBEDDINGS_ENDPOINT );
}

QString QgsAiEmbeddingClient::model() const
{
  if ( !mModelOverride.isEmpty() )
    return mModelOverride;

  const QgsSettings settings;
  if ( provider() == Provider::OpenRouter )
  {
    const QString configured = settings.value( QString::fromLatin1( OPENROUTER_EMBEDDING_MODEL_SETTING ), u"openai/text-embedding-3-small"_s ).toString().trimmed();
    return configured.isEmpty() ? u"openai/text-embedding-3-small"_s : configured;
  }

  const QString configured = settings.value( QString::fromLatin1( OPENAI_EMBEDDING_MODEL_SETTING ), u"text-embedding-3-small"_s ).toString().trimmed();
  return configured.isEmpty() ? u"text-embedding-3-small"_s : configured;
}

QJsonObject QgsAiEmbeddingClient::openRouterProviderPreferences() const
{
  QJsonObject providerPreferences;
  providerPreferences.insert( u"sort"_s, u"price"_s );
  providerPreferences.insert( u"data_collection"_s, u"deny"_s );
  providerPreferences.insert( u"allow_fallbacks"_s, true );
  return providerPreferences;
}

QString QgsAiEmbeddingClient::apiKey() const
{
  const bool useOpenRouter = provider() == Provider::OpenRouter;
  return QgsAiSecretStore::readSecret(
    QString::fromLatin1( useOpenRouter ? OPENROUTER_KEY_SETTING : OPENAI_KEY_SETTING ),
    { useOpenRouter ? u"OPENROUTER_API_KEY"_s : u"OPENAI_API_KEY"_s }
  );
}

bool QgsAiEmbeddingClient::hasApiKey() const
{
  return !mAuthenticationFailed && !apiKey().isEmpty();
}

bool QgsAiEmbeddingClient::embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage, int maxBatch )
{
  out.clear();
  if ( texts.isEmpty() )
    return true;

  const int batchSize = std::clamp( maxBatch, 1, 256 );
  for ( int i = 0; i < texts.size(); i += batchSize )
  {
    const QStringList batch = texts.mid( i, batchSize );
    QList<QVector<float>> batchOut;
    if ( !embedBatch( batch, batchOut, errorMessage ) )
      return false;
    if ( batchOut.size() != batch.size() )
    {
      if ( errorMessage )
        *errorMessage = u"Embedding response shape mismatch: expected %1 vectors, got %2."_s.arg( batch.size() ).arg( batchOut.size() );
      return false;
    }
    out.append( batchOut );
  }
  return true;
}

bool QgsAiEmbeddingClient::performRequest( const QByteArray &payload, const QString &key, int &httpStatus, QByteArray &body, int &networkError, int &retryAfterSeconds, QString *errorMessage )
{
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
  if ( !nam )
  {
    if ( errorMessage )
      *errorMessage = u"Network manager is not available."_s;
    return false;
  }

  QNetworkRequest request { QUrl( endpoint() ) };
  request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( key ) ).toUtf8() );
  request.setTransferTimeout( mTimeoutMs );
  if ( provider() == Provider::OpenRouter )
  {
    // App attribution headers recommended by OpenRouter.
    request.setRawHeader( "HTTP-Referer", "https://github.com/francemazzi/strata" );
    request.setRawHeader( "X-Title", "Strata" );
  }

  QNetworkReply *reply = nam->post( request, payload );
  if ( !reply )
  {
    if ( errorMessage )
      *errorMessage = u"Failed to start embeddings request."_s;
    return false;
  }

  // Block until finished. setTransferTimeout above guards against hangs.
  QEventLoop loop;
  connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
  loop.exec();

  httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  body = reply->readAll();
  networkError = static_cast<int>( reply->error() );
  retryAfterSeconds = -1;
  if ( reply->hasRawHeader( "Retry-After" ) )
  {
    bool parsedOk = false;
    const int parsed = QString::fromLatin1( reply->rawHeader( "Retry-After" ) ).trimmed().toInt( &parsedOk );
    if ( parsedOk && parsed >= 0 )
      retryAfterSeconds = parsed;
  }
  reply->deleteLater();
  return true;
}

bool QgsAiEmbeddingClient::embedBatch( const QStringList &batch, QList<QVector<float>> &out, QString *errorMessage )
{
  const QString providerName = provider() == Provider::OpenRouter ? u"OpenRouter"_s : u"OpenAI"_s;

  if ( mAuthenticationFailed )
  {
    if ( errorMessage )
    {
      *errorMessage = mCreditsExhausted
                        ? u"%1 embeddings are disabled after running out of credits. Top up the account and retry."_s.arg( providerName )
                        : u"%1 embeddings are disabled after an authentication failure. Check the API key and retry."_s.arg( providerName );
    }
    return false;
  }

  const QString key = apiKey();
  if ( key.isEmpty() )
  {
    if ( errorMessage )
    {
      *errorMessage = provider() == Provider::OpenRouter ? u"OpenRouter API key is not configured (ai/provider/openrouter/apiKey or OPENROUTER_API_KEY)."_s
                                                         : u"OpenAI API key is not configured (ai/provider/openai/apiKey or OPENAI_API_KEY)."_s;
    }
    return false;
  }

  QJsonObject payload;
  payload.insert( u"model"_s, model() );
  QJsonArray input;
  for ( const QString &t : batch )
    input.append( t );
  payload.insert( u"input"_s, input );
  if ( provider() == Provider::OpenRouter )
    payload.insert( u"provider"_s, openRouterProviderPreferences() );
  const QByteArray payloadBytes = QJsonDocument( payload ).toJson( QJsonDocument::Compact );

  int httpStatus = 0;
  QByteArray body;
  int networkError = 0;
  int retryAfterSeconds = -1;
  if ( !performRequest( payloadBytes, key, httpStatus, body, networkError, retryAfterSeconds, errorMessage ) )
    return false;

  // One retry for transient failures (408/429/5xx), honoring Retry-After when present.
  const bool transientFailure = httpStatus == 408 || httpStatus == 429 || ( httpStatus >= 500 && httpStatus <= 599 );
  if ( transientFailure )
  {
    const int delayMs = retryAfterSeconds >= 0 ? std::min( retryAfterSeconds, 10 ) * 1000 : 1000;
    QgsMessageLog::logMessage( u"Embeddings request hit HTTP %1; retrying once in %2 ms."_s.arg( httpStatus ).arg( delayMs ), u"AI/Index"_s, Qgis::MessageLevel::Info, false );
    QEventLoop waitLoop;
    QTimer::singleShot( delayMs, &waitLoop, &QEventLoop::quit );
    waitLoop.exec();
    if ( !performRequest( payloadBytes, key, httpStatus, body, networkError, retryAfterSeconds, errorMessage ) )
      return false;
  }

  if ( networkError != 0 || httpStatus < 200 || httpStatus >= 300 )
  {
    QString detail;
    const QJsonDocument doc = QJsonDocument::fromJson( body );
    if ( doc.isObject() )
    {
      const QJsonObject err = doc.object().value( u"error"_s ).toObject();
      detail = err.value( u"message"_s ).toString();
    }
    if ( httpStatus == 401 || httpStatus == 403 )
    {
      mAuthenticationFailed = true;
      if ( !mAuthFailureLogged )
      {
        QgsMessageLog::logMessage( u"Embeddings authentication failed for %1; disabling this remote embedding provider until settings are changed."_s.arg( providerName ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
        mAuthFailureLogged = true;
      }
      if ( errorMessage )
        *errorMessage = u"%1 embeddings authentication failed (HTTP %2)%3"_s.arg( providerName ).arg( httpStatus ).arg( detail.isEmpty() ? u"."_s : u": %1"_s.arg( detail ) );
      return false;
    }
    if ( httpStatus == 402 )
    {
      // Out of credits: trip the breaker so indexing doesn't hammer a paid API.
      mAuthenticationFailed = true;
      mCreditsExhausted = true;
      if ( !mAuthFailureLogged )
      {
        QgsMessageLog::logMessage( u"%1 account has insufficient credits for embeddings; disabling this remote embedding provider until settings are changed."_s.arg( providerName ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
        mAuthFailureLogged = true;
      }
      if ( errorMessage )
        *errorMessage = u"%1 account has insufficient credits for embeddings%2"_s.arg( providerName ).arg( provider() == Provider::OpenRouter ? u" — top up at openrouter.ai/credits."_s : u"."_s );
      return false;
    }

    QgsMessageLog::logMessage(
      u"Embeddings request failed httpStatus=%1 networkError=%2 detail=%3"_s.arg( httpStatus ).arg( networkError ).arg( detail.left( 300 ) ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false
    );
    if ( errorMessage )
      *errorMessage = detail.isEmpty() ? u"Embeddings request failed (HTTP %1)."_s.arg( httpStatus ) : u"Embeddings request failed: %1"_s.arg( detail );
    return false;
  }

  const QJsonDocument doc = QJsonDocument::fromJson( body );
  if ( !doc.isObject() )
  {
    if ( errorMessage )
      *errorMessage = u"Embeddings response is not a JSON object."_s;
    return false;
  }

  // Response: { "data": [{ "embedding": [...], "index": N }, ...], "model": "...", "usage": {...} }
  // OpenAI guarantees ordering by index; we sort defensively.
  const QJsonArray data = doc.object().value( u"data"_s ).toArray();
  out.resize( batch.size() );
  for ( const QJsonValue &item : data )
  {
    const QJsonObject obj = item.toObject();
    const int idx = obj.value( u"index"_s ).toInt( -1 );
    if ( idx < 0 || idx >= batch.size() )
      continue;
    const QJsonArray emb = obj.value( u"embedding"_s ).toArray();
    QVector<float> vec;
    vec.reserve( emb.size() );
    for ( const QJsonValue &v : emb )
      vec.append( static_cast<float>( v.toDouble() ) );
    out[idx] = vec;
  }

  // Sanity: every slot must be populated.
  for ( int i = 0; i < out.size(); ++i )
  {
    if ( out.at( i ).isEmpty() )
    {
      if ( errorMessage )
        *errorMessage = u"Embedding for input %1 was missing in the response."_s.arg( i );
      return false;
    }
  }
  return true;
}
