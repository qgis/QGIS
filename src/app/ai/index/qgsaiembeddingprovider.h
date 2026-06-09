/***************************************************************************
    qgsaiembeddingprovider.h
    ------------------------
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

#ifndef QGSAIEMBEDDINGPROVIDER_H
#define QGSAIEMBEDDINGPROVIDER_H

#include "qgis_app.h"

#include <QList>
#include <memory>
#include <QString>
#include <QStringList>
#include <QVector>

using namespace Qt::StringLiterals;

class QObject;

enum class QgsAiEmbeddingRole
{
  Query,
  Passage,
};

struct QgsAiEmbeddingOptions
{
    int maxBatch = 64;
};

/**
 * Local embedding provider interface used by the workspace index.
 *
 * Implementations must not call external APIs unless the user has explicitly
 * selected a remote provider. The default product path is local/on-device.
 */
class APP_EXPORT QgsAiEmbeddingProvider
{
  public:
    virtual ~QgsAiEmbeddingProvider() = default;

    virtual QString providerId() const = 0;
    virtual QString displayName() const = 0;
    virtual QString modelId() const { return providerId(); }
    virtual QString modelRevision() const { return QString(); }
    virtual int embeddingDimension() const { return 0; }
    virtual bool isRemote() const { return false; }
    virtual bool isAvailable( QString *errorMessage = nullptr ) const = 0;
    virtual bool embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage = nullptr, int maxBatch = 64 ) = 0;
    virtual bool embed( const QStringList &texts, QgsAiEmbeddingRole role, QList<QVector<float>> &out, QString *errorMessage = nullptr, const QgsAiEmbeddingOptions &options = QgsAiEmbeddingOptions() )
    {
      Q_UNUSED( role )
      return embed( texts, out, errorMessage, options.maxBatch );
    }
};

/**
 * Placeholder provider used until a local embedding model is packaged.
 */
class APP_EXPORT QgsAiUnavailableLocalEmbeddingProvider final : public QgsAiEmbeddingProvider
{
  public:
    QString providerId() const override { return u"local"_s; }
    QString displayName() const override { return u"Local embedding model"_s; }
    QString modelId() const override { return u"unavailable"_s; }

    bool isAvailable( QString *errorMessage = nullptr ) const override
    {
      if ( errorMessage )
        *errorMessage = u"Local embedding model is not installed."_s;
      return false;
    }

    bool embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage = nullptr, int maxBatch = 64 ) override
    {
      Q_UNUSED( texts )
      Q_UNUSED( maxBatch )
      out.clear();
      if ( errorMessage )
        *errorMessage = u"Local embedding model is not installed."_s;
      return false;
    }
};

/**
 * Zero-setup local embedding provider.
 *
 * This is intentionally small and CPU-only. It provides deterministic 384
 * dimensional embeddings from lexical tokens/ngrams so indexing works on
 * low-memory machines without external APIs. A future ONNX/SentencePiece E5
 * implementation can keep the same provider registry slot and force a schema
 * rebuild through a different modelId/modelRevision.
 */
class APP_EXPORT QgsAiLocalEmbeddingProvider final : public QgsAiEmbeddingProvider
{
  public:
    QString providerId() const override { return u"local:e5-small-int8"_s; }
    QString displayName() const override { return u"Local small model"_s; }
    QString modelId() const override { return u"strata-local-minihash-384"_s; }
    QString modelRevision() const override { return u"2026-06-09"_s; }
    int embeddingDimension() const override { return 384; }
    bool isAvailable( QString *errorMessage = nullptr ) const override;
    bool embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage = nullptr, int maxBatch = 64 ) override;
    bool embed( const QStringList &texts, QgsAiEmbeddingRole role, QList<QVector<float>> &out, QString *errorMessage = nullptr, const QgsAiEmbeddingOptions &options = QgsAiEmbeddingOptions() ) override;

  private:
    QVector<float> embedOne( const QString &text, QgsAiEmbeddingRole role ) const;
};

class APP_EXPORT QgsAiEmbeddingProviderRegistry
{
  public:
    static QString defaultProviderId();
    static QString configuredProviderId();
    static void setConfiguredProviderId( const QString &providerId );
    static QStringList providerIds();
    static QString displayNameForProviderId( const QString &providerId );
    static bool isRemoteProviderId( const QString &providerId );
    static std::unique_ptr<QgsAiEmbeddingProvider> createProviderFromSettings( QObject *parent = nullptr );
    static std::unique_ptr<QgsAiEmbeddingProvider> createProvider( const QString &providerId, QObject *parent = nullptr );
};

#endif // QGSAIEMBEDDINGPROVIDER_H
