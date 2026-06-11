/***************************************************************************
    qgsaiopenroutermodelcatalog.h
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

#ifndef QGSAIOPENROUTERMODELCATALOG_H
#define QGSAIOPENROUTERMODELCATALOG_H

#include "qgis_app.h"

#include <QList>
#include <QObject>
#include <QString>

/**
 * Async catalog of tool-capable OpenRouter models for the model picker UI.
 *
 * Fetches `GET /api/v1/models?supported_parameters=tools` (no auth required),
 * caches the parsed list as JSON in the QGIS settings directory with a 24h TTL,
 * and falls back to a static curated list when offline. Also exposes
 * `GET /api/v1/key` for the "Test connection" button (credits summary).
 */
class APP_EXPORT QgsAiOpenRouterModelCatalog : public QObject
{
    Q_OBJECT

  public:
    struct ModelInfo
    {
        QString id;
        QString name;
        int contextLength = 0;
        double promptUsdPerMTok = 0.0;     // USD per million prompt tokens
        double completionUsdPerMTok = 0.0; // USD per million completion tokens

        //! One-line label for combo boxes: name, context size and pricing.
        QString displayLabel() const;
    };

    explicit QgsAiOpenRouterModelCatalog( QObject *parent = nullptr );

    //! Overrides the API base URL (e.g. a loopback server in tests). Default: https://openrouter.ai/api/v1
    void setApiBaseOverride( const QString &base ) { mApiBaseOverride = base.trimmed(); }
    //! Overrides the cache file path (tests). Default: <qgisSettingsDir>/openrouter_models_cache.json
    void setCacheFilePathOverride( const QString &path ) { mCacheFilePathOverride = path.trimmed(); }

    //! Cache freshness window.
    static constexpr qint64 CACHE_TTL_SECONDS = 24 * 3600;

    /**
     * Emits modelsReady() with the cached list when it is fresh (unless \a force),
     * otherwise fetches the catalog from the network. On network failure, the
     * stale cache or the static curated list is emitted instead — modelsReady()
     * always fires exactly once per refresh() call.
     */
    void refresh( bool force = false );

    /**
     * Fetches the API key info (label, usage, remaining credits) for the
     * "Test connection" button. Emits keyInfoReady() or keyInfoFailed().
     */
    void fetchKeyInfo( const QString &apiKey );

    //! Parses a /models response body into tool-capable model infos. Public for unit tests.
    static QList<ModelInfo> parseModelsJson( const QByteArray &body );

    //! Static fallback list used when both the network and the cache are unavailable.
    static QList<ModelInfo> curatedFallback();

    //! Resolved cache file path (honors the override).
    QString cacheFilePath() const;

  signals:
    void modelsReady( const QList<QgsAiOpenRouterModelCatalog::ModelInfo> &models, bool fromCache );
    void keyInfoReady( const QString &summary );
    void keyInfoFailed( const QString &errorMessage );

  private:
    QString apiBase() const;
    bool readCache( QList<ModelInfo> &models, qint64 &fetchedAtSecs ) const;
    void writeCache( const QList<ModelInfo> &models ) const;

    QString mApiBaseOverride;
    QString mCacheFilePathOverride;
};

#endif // QGSAIOPENROUTERMODELCATALOG_H
