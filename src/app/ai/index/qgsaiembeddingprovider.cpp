/***************************************************************************
    qgsaiembeddingprovider.cpp
    --------------------------
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

#include "qgsaiembeddingprovider.h"

#include "qgsaiembeddingclient.h"
#include "qgssettings.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include <QByteArray>
#include <QChar>
#include <QCryptographicHash>
#include <QObject>

using namespace Qt::StringLiterals;

namespace
{
  constexpr int LOCAL_DIMENSION = 384;
  constexpr const char *INDEX_PROVIDER_SETTING = "strata/index/embedding_provider";
  constexpr const char *LEGACY_EMBEDDINGS_PROVIDER_SETTING = "ai/embeddings/provider";

  quint64 stableHash64( const QString &value )
  {
    const QByteArray digest = QCryptographicHash::hash( value.toUtf8(), QCryptographicHash::Sha1 );
    quint64 out = 0;
    const int bytes = std::min<int>( 8, digest.size() );
    for ( int i = 0; i < bytes; ++i )
      out = ( out << 8 ) | static_cast<unsigned char>( digest.at( i ) );
    return out;
  }

  void addFeature( QVector<float> &vector, const QString &feature, float weight )
  {
    const quint64 h = stableHash64( feature );
    const int idx = static_cast<int>( h % static_cast<quint64>( vector.size() ) );
    const float sign = ( h & 0x100000000ULL ) ? -1.0f : 1.0f;
    vector[idx] += sign * weight;
  }

  QStringList tokenize( const QString &text )
  {
    QString normalized = text.toLower().normalized( QString::NormalizationForm_KD );
    QStringList tokens;
    QString current;
    for ( const QChar ch : normalized )
    {
      if ( ch.isLetterOrNumber() )
      {
        current.append( ch );
      }
      else if ( !current.isEmpty() )
      {
        tokens.append( current );
        current.clear();
      }
    }
    if ( !current.isEmpty() )
      tokens.append( current );
    return tokens;
  }

  class RemoteEmbeddingProvider final : public QgsAiEmbeddingProvider
  {
    public:
      explicit RemoteEmbeddingProvider( QgsAiEmbeddingClient::Provider provider, QObject *parent )
        : mProvider( provider )
      {
        Q_UNUSED( parent )
        mClient.setProvider( provider );
      }

      QString providerId() const override
      {
        return mProvider == QgsAiEmbeddingClient::Provider::OpenRouter ? u"openrouter"_s : u"openai"_s;
      }

      QString displayName() const override
      {
        return mProvider == QgsAiEmbeddingClient::Provider::OpenRouter ? u"OpenRouter"_s : u"OpenAI"_s;
      }

      QString modelId() const override { return mClient.model(); }
      QString modelRevision() const override { return u"remote"_s; }
      bool isRemote() const override { return true; }

      bool isAvailable( QString *errorMessage = nullptr ) const override
      {
        if ( mClient.hasApiKey() )
          return true;

        if ( errorMessage )
        {
          *errorMessage = mProvider == QgsAiEmbeddingClient::Provider::OpenRouter ? u"OpenRouter embedding provider is selected, but no OpenRouter API key is configured."_s
                                                                                  : u"OpenAI embedding provider is selected, but no OpenAI API key is configured."_s;
        }
        return false;
      }

      bool embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage = nullptr, int maxBatch = 64 ) override
      {
        return mClient.embed( texts, out, errorMessage, maxBatch );
      }

      bool embed( const QStringList &texts, QgsAiEmbeddingRole role, QList<QVector<float>> &out, QString *errorMessage = nullptr, const QgsAiEmbeddingOptions &options = QgsAiEmbeddingOptions() ) override
      {
        Q_UNUSED( role )
        return embed( texts, out, errorMessage, options.maxBatch );
      }

    private:
      mutable QgsAiEmbeddingClient mClient;
      QgsAiEmbeddingClient::Provider mProvider = QgsAiEmbeddingClient::Provider::OpenAi;
  };
} // namespace

bool QgsAiLocalEmbeddingProvider::isAvailable( QString *errorMessage ) const
{
  Q_UNUSED( errorMessage )
  return true;
}

bool QgsAiLocalEmbeddingProvider::embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage, int maxBatch )
{
  QgsAiEmbeddingOptions options;
  options.maxBatch = maxBatch;
  return embed( texts, QgsAiEmbeddingRole::Passage, out, errorMessage, options );
}

bool QgsAiLocalEmbeddingProvider::embed( const QStringList &texts, QgsAiEmbeddingRole role, QList<QVector<float>> &out, QString *errorMessage, const QgsAiEmbeddingOptions &options )
{
  Q_UNUSED( errorMessage )
  Q_UNUSED( options )
  out.clear();
  out.reserve( texts.size() );
  for ( const QString &text : texts )
    out.append( embedOne( text, role ) );
  return true;
}

QVector<float> QgsAiLocalEmbeddingProvider::embedOne( const QString &text, QgsAiEmbeddingRole role ) const
{
  QVector<float> vector( LOCAL_DIMENSION );
  const QStringList tokens = tokenize( text );

  addFeature( vector, role == QgsAiEmbeddingRole::Query ? u"role:query"_s : u"role:passage"_s, 0.05f );

  for ( int i = 0; i < tokens.size(); ++i )
  {
    const QString &token = tokens.at( i );
    addFeature( vector, u"w:"_s + token, 1.0f );

    if ( i + 1 < tokens.size() )
      addFeature( vector, u"b:"_s + token + u" "_s + tokens.at( i + 1 ), 0.8f );

    if ( token.size() >= 3 )
    {
      for ( int j = 0; j <= token.size() - 3; ++j )
        addFeature( vector, u"c:"_s + token.mid( j, 3 ), 0.25f );
    }
  }

  if ( tokens.isEmpty() )
  {
    const QString trimmed = text.trimmed().toLower();
    for ( int i = 0; i < trimmed.size(); ++i )
      addFeature( vector, u"u:"_s + trimmed.mid( i, 1 ), 0.2f );
  }

  double norm = 0.0;
  for ( float v : vector )
    norm += static_cast<double>( v ) * static_cast<double>( v );
  if ( norm > 0.0 )
  {
    const float inv = static_cast<float>( 1.0 / std::sqrt( norm ) );
    for ( float &v : vector )
      v *= inv;
  }
  return vector;
}

QString QgsAiEmbeddingProviderRegistry::defaultProviderId()
{
  return u"local:e5-small-int8"_s;
}

QString QgsAiEmbeddingProviderRegistry::configuredProviderId()
{
  const QgsSettings settings;
  const QString configured = settings.value( QString::fromLatin1( INDEX_PROVIDER_SETTING ), defaultProviderId() ).toString().trimmed().toLower();
  if ( providerIds().contains( configured ) )
    return configured;

  // Legacy remote embedding settings must not implicitly enable remote
  // indexing. They are ignored unless copied to the new Strata index key.
  Q_UNUSED( LEGACY_EMBEDDINGS_PROVIDER_SETTING )
  return defaultProviderId();
}

void QgsAiEmbeddingProviderRegistry::setConfiguredProviderId( const QString &providerId )
{
  QgsSettings settings;
  const QString normalized = providerId.trimmed().toLower();
  settings.setValue( QString::fromLatin1( INDEX_PROVIDER_SETTING ), providerIds().contains( normalized ) ? normalized : defaultProviderId() );
}

QStringList QgsAiEmbeddingProviderRegistry::providerIds()
{
  return { defaultProviderId(), u"openai"_s, u"openrouter"_s };
}

QString QgsAiEmbeddingProviderRegistry::displayNameForProviderId( const QString &providerId )
{
  const QString normalized = providerId.trimmed().toLower();
  if ( normalized == "openai"_L1 )
    return u"OpenAI"_s;
  if ( normalized == "openrouter"_L1 )
    return u"OpenRouter"_s;
  return u"Local small model (recommended)"_s;
}

bool QgsAiEmbeddingProviderRegistry::isRemoteProviderId( const QString &providerId )
{
  const QString normalized = providerId.trimmed().toLower();
  return normalized == "openai"_L1 || normalized == "openrouter"_L1;
}

std::unique_ptr<QgsAiEmbeddingProvider> QgsAiEmbeddingProviderRegistry::createProviderFromSettings( QObject *parent )
{
  return createProvider( configuredProviderId(), parent );
}

std::unique_ptr<QgsAiEmbeddingProvider> QgsAiEmbeddingProviderRegistry::createProvider( const QString &providerId, QObject *parent )
{
  const QString normalized = providerId.trimmed().toLower();
  if ( normalized == "openai"_L1 )
    return std::make_unique<RemoteEmbeddingProvider>( QgsAiEmbeddingClient::Provider::OpenAi, parent );
  if ( normalized == "openrouter"_L1 )
    return std::make_unique<RemoteEmbeddingProvider>( QgsAiEmbeddingClient::Provider::OpenRouter, parent );
  return std::make_unique<QgsAiLocalEmbeddingProvider>();
}
