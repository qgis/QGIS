/***************************************************************************
    qgsaiembeddingclient.h
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

#ifndef QGSAIEMBEDDINGCLIENT_H
#define QGSAIEMBEDDINGCLIENT_H

#include "qgis_app.h"

#include <QList>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

using namespace Qt::StringLiterals;

/**
 * Legacy synchronous client for remote OpenAI/OpenRouter embeddings.
 * Reuses the same QgsSettings/env-var conventions as QgsAiModelRouter for API
 * keys. Each call to embed() blocks the calling thread via a local QEventLoop
 * until the response arrives or a timeout triggers.
 *
 * The default OpenAI model is `text-embedding-3-small`; OpenRouter defaults to
 * `openai/text-embedding-3-small`. Pass an explicit override to setModel() to
 * use another embedding model.
 *
 * Not used by the default workspace index path. Local/on-device embeddings are
 * represented by QgsAiEmbeddingProvider implementations.
 */
class APP_EXPORT QgsAiEmbeddingClient : public QObject
{
    Q_OBJECT

  public:
    enum class Provider
    {
      OpenAi,
      OpenRouter
    };

    explicit QgsAiEmbeddingClient( QObject *parent = nullptr );

    void setProvider( Provider provider )
    {
      mProviderOverride = provider;
      mHasProviderOverride = true;
      resetCircuitBreaker();
    }
    void setModel( const QString &model ) { mModelOverride = model.trimmed(); }

    /**
     * Overrides the embeddings endpoint URL (e.g. a self-hosted gateway, or a
     * loopback server in tests). Pass an empty string to restore the default
     * provider endpoint.
     */
    void setEndpointOverride( const QString &endpoint ) { mEndpointOverride = endpoint.trimmed(); }
    QString model() const;
    Provider provider() const;
    QString endpoint() const;
    QJsonObject openRouterProviderPreferences() const;
    bool authenticationFailed() const { return mAuthenticationFailed; }
    void resetCircuitBreaker()
    {
      mAuthenticationFailed = false;
      mAuthFailureLogged = false;
    }

    /**
     * Returns true when the configured embeddings provider has an API key (settings or env).
     * Lets callers fail fast with a clear message before walking the workspace.
     */
    bool hasApiKey() const;

    /**
     * Synchronous embed of \a texts. On success, \a out has size == texts.size()
     * and each entry has the model's embedding dimension. On failure, \a errorMessage
     * is filled and the function returns false.
     *
     * The implementation batches inputs into chunks of \a maxBatch entries to
     * keep payloads under 1 MB. Default batch size is 64.
     */
    bool embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage = nullptr, int maxBatch = 64 );

  private:
    QString apiKey() const;
    bool embedBatch( const QStringList &batch, QList<QVector<float>> &out, QString *errorMessage );

    QString mModelOverride;
    QString mEndpointOverride;
    int mTimeoutMs = 60000;
    Provider mProviderOverride = Provider::OpenAi;
    bool mHasProviderOverride = false;
    bool mAuthenticationFailed = false;
    bool mAuthFailureLogged = false;
};

#endif // QGSAIEMBEDDINGCLIENT_H
