#include "qgsaiembeddingclient.h"

#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace
{
  constexpr const char *OPENAI_EMBEDDINGS_ENDPOINT = "https://api.openai.com/v1/embeddings";
  constexpr const char *OPENAI_KEY_SETTING = "ai/provider/openai/apiKey";
}

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
        *errorMessage = QStringLiteral( "Embedding response shape mismatch: expected %1 vectors, got %2." )
                          .arg( batch.size() ).arg( batchOut.size() );
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
      *errorMessage = QStringLiteral( "OpenAI API key is not configured (ai/provider/openai/apiKey or OPENAI_API_KEY)." );
    return false;
  }

  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
  if ( !nam )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Network manager is not available." );
    return false;
  }

  QJsonObject payload;
  payload.insert( QStringLiteral( "model" ), mModel );
  QJsonArray input;
  for ( const QString &t : batch )
    input.append( t );
  payload.insert( QStringLiteral( "input" ), input );

  QNetworkRequest request( QUrl( QString::fromUtf8( OPENAI_EMBEDDINGS_ENDPOINT ) ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/json" ) );
  request.setRawHeader( "Authorization", ( QStringLiteral( "Bearer %1" ).arg( apiKey ) ).toUtf8() );
  request.setTransferTimeout( mTimeoutMs );

  QNetworkReply *reply = nam->post( request, QJsonDocument( payload ).toJson( QJsonDocument::Compact ) );
  if ( !reply )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Failed to start embeddings request." );
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
      const QJsonObject err = doc.object().value( QStringLiteral( "error" ) ).toObject();
      detail = err.value( QStringLiteral( "message" ) ).toString();
    }
    QgsMessageLog::logMessage(
      QStringLiteral( "Embeddings request failed httpStatus=%1 networkError=%2 detail=%3" )
        .arg( httpStatus ).arg( static_cast<int>( netError ) ).arg( detail.left( 300 ) ),
      QStringLiteral( "AI/Index" ), Qgis::MessageLevel::Warning, false );
    if ( errorMessage )
      *errorMessage = detail.isEmpty()
                        ? QStringLiteral( "Embeddings request failed (HTTP %1)." ).arg( httpStatus )
                        : QStringLiteral( "Embeddings request failed: %1" ).arg( detail );
    return false;
  }

  const QJsonDocument doc = QJsonDocument::fromJson( body );
  if ( !doc.isObject() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Embeddings response is not a JSON object." );
    return false;
  }

  // Response: { "data": [{ "embedding": [...], "index": N }, ...], "model": "...", "usage": {...} }
  // OpenAI guarantees ordering by index; we sort defensively.
  const QJsonArray data = doc.object().value( QStringLiteral( "data" ) ).toArray();
  out.resize( batch.size() );
  for ( const QJsonValue &item : data )
  {
    const QJsonObject obj = item.toObject();
    const int idx = obj.value( QStringLiteral( "index" ) ).toInt( -1 );
    if ( idx < 0 || idx >= batch.size() )
      continue;
    const QJsonArray emb = obj.value( QStringLiteral( "embedding" ) ).toArray();
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
        *errorMessage = QStringLiteral( "Embedding for input %1 was missing in the response." ).arg( i );
      return false;
    }
  }
  return true;
}
