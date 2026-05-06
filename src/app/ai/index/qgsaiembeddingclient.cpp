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
#include "moc_qgsaiembeddingclient.cpp"

#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTimer>
#include <QUrl>

using namespace Qt::StringLiterals;

namespace
{
  constexpr const char *OPENAI_EMBEDDINGS_ENDPOINT = "https://api.openai.com/v1/embeddings";
  constexpr const char *OPENAI_KEY_SETTING = "ai/provider/openai/apiKey";
} //namespace

QgsAiEmbeddingClient::QgsAiEmbeddingClient( QObject *parent )
  : QObject( parent )
{}

QString QgsAiEmbeddingClient::openAiApiKey() const
{
  const QgsSettings settings;
  const QString stored = settings.value( OPENAI_KEY_SETTING ).toString().trimmed();
  if ( !stored.isEmpty() )
    return stored;

  const QString envValue = qEnvironmentVariable( "OPENAI_API_KEY" ).trimmed();
  return envValue;
}

bool QgsAiEmbeddingClient::hasApiKey() const
{
  return !openAiApiKey().isEmpty();
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

bool QgsAiEmbeddingClient::embedBatch( const QStringList &batch, QList<QVector<float>> &out, QString *errorMessage )
{
  const QString apiKey = openAiApiKey();
  if ( apiKey.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"OpenAI API key is not configured (ai/provider/openai/apiKey or OPENAI_API_KEY)."_s;
    return false;
  }

  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
  if ( !nam )
  {
    if ( errorMessage )
      *errorMessage = u"Network manager is not available."_s;
    return false;
  }

  QJsonObject payload;
  payload.insert( u"model"_s, mModel );
  QJsonArray input;
  for ( const QString &t : batch )
    input.append( t );
  payload.insert( u"input"_s, input );

  QNetworkRequest request( QUrl( QString::fromUtf8( OPENAI_EMBEDDINGS_ENDPOINT ) ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( apiKey ) ).toUtf8() );
  request.setTransferTimeout( mTimeoutMs );

  QNetworkReply *reply = nam->post( request, QJsonDocument( payload ).toJson( QJsonDocument::Compact ) );
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

  const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QByteArray body = reply->readAll();
  const QNetworkReply::NetworkError netError = reply->error();
  reply->deleteLater();

  if ( netError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
  {
    QString detail;
    const QJsonDocument doc = QJsonDocument::fromJson( body );
    if ( doc.isObject() )
    {
      const QJsonObject err = doc.object().value( u"error"_s ).toObject();
      detail = err.value( u"message"_s ).toString();
    }
    QgsMessageLog::
      logMessage( u"Embeddings request failed httpStatus=%1 networkError=%2 detail=%3"_s.arg( httpStatus ).arg( static_cast<int>( netError ) ).arg( detail.left( 300 ) ), u"AI/Index"_s, Qgis::MessageLevel::Warning, false );
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
