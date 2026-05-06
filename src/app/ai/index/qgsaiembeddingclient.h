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
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

using namespace Qt::StringLiterals;

/**
 * Synchronous client for the OpenAI Embeddings API (`/v1/embeddings`).
 * Reuses the same QgsSettings/env-var conventions as QgsAiModelRouter for the
 * OpenAI API key. Each call to embed() blocks the calling thread via a local
 * QEventLoop until the response arrives or a timeout triggers.
 *
 * The default model is `text-embedding-3-small` (1536 dimensions). Pass an
 * explicit override to setModel() to use another embedding model.
 *
 * Used only by the workspace index for retrieval — *not* by the chat router.
 */
class APP_EXPORT QgsAiEmbeddingClient : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiEmbeddingClient( QObject *parent = nullptr );

    void setModel( const QString &model ) { mModel = model; }
    QString model() const { return mModel; }

    /**
     * Returns true when an OpenAI API key is configured (settings or env).
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
    QString openAiApiKey() const;
    bool embedBatch( const QStringList &batch, QList<QVector<float>> &out, QString *errorMessage );

    QString mModel = u"text-embedding-3-small"_s;
    int mTimeoutMs = 60000;
};

#endif // QGSAIEMBEDDINGCLIENT_H
