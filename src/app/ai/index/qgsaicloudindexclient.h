/***************************************************************************
    qgsaicloudindexclient.h
    -----------------------
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

#ifndef QGSAICLOUDINDEXCLIENT_H
#define QGSAICLOUDINDEXCLIENT_H

#include "qgis_app.h"

#include "qgsaiworkspaceindex.h"

#include <QList>
#include <QObject>
#include <QString>

/**
 * Strata Cloud context indexing client.
 *
 * The client upserts the current desktop workspace first, then sends explicit
 * content-opt-in context items to `/v1/index/:workspaceId/context`.
 */
class APP_EXPORT QgsAiCloudIndexClient : public QObject
{
    Q_OBJECT

  public:
    struct ContextItem
    {
        QString sourceType;
        QString path;
        QString layerId;
        QString title;
        QString mimeType;
        QString text;
        QString ocrText;
        QString caption;
        int chunkIndex = -1;
    };

    struct SyncResult
    {
        QString workspaceId;
        int upserted = 0;
        int queued = 0;
    };

    explicit QgsAiCloudIndexClient( QObject *parent = nullptr );

    static QString apiBaseForChatEndpoint( const QString &chatEndpoint );
    static QString workspaceFingerprint( const QString &workspaceRoot );
    static QList<ContextItem> contextItemsFromChunks( const QList<QgsAiWorkspaceIndex::Chunk> &chunks );
    static QList<ContextItem> contextItemsFromWorkspaceFolders( const QString &workspaceRoot, const QString &rulesPath, const QString &skillsPath );
    static QList<ContextItem> deduplicateContextItems( const QList<ContextItem> &items );
    static bool containsForbiddenPayload( const ContextItem &item );
    static bool validateContextItems( const QList<ContextItem> &items, QString *errorMessage = nullptr );

    void syncWorkspaceContext(
      const QString &chatEndpoint,
      const QString &sessionToken,
      const QString &workspaceRoot,
      const QString &workspaceName,
      const QList<ContextItem> &items,
      bool contentOptIn
    );

  signals:
    void contextSynced( const QgsAiCloudIndexClient::SyncResult &result );
    void requestFailed( const QString &message );

  private:
    void postContextBatch(
      const QString &apiBase,
      const QString &sessionToken,
      const QString &workspaceId,
      const QList<ContextItem> &items,
      int offset,
      int accumulatedUpserted,
      int accumulatedQueued,
      bool contentOptIn
    );
};

Q_DECLARE_METATYPE( QgsAiCloudIndexClient::ContextItem )
Q_DECLARE_METATYPE( QgsAiCloudIndexClient::SyncResult )

#endif // QGSAICLOUDINDEXCLIENT_H
