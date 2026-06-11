/***************************************************************************
    qgsaichathistorystore.h
    ---------------------
    begin                : May 2026
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

#ifndef QGSAICHATHISTORYSTORE_H
#define QGSAICHATHISTORYSTORE_H

#include "qgis_app.h"
#include "qgsaimodels.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>

class QSqlDatabase;

class QgsAiFileContextProvider;

/**
 * Persists chat sessions in a SQLite database. By default the store keeps the
 * legacy per-workspace behavior, mirroring the layout used by QgsAiWorkspaceIndex.
 * Call setHistoryScopeKey() to switch to an explicit per-project history scope.
 * An explicit empty scope disables persistence, which is used for unsaved QGIS
 * projects so they never show history from other projects.
 *
 * Two tables: \c sessions (one row per chat) and \c messages (one row per
 * message, with a session_id foreign key and a per-session ordering column).
 */
class APP_EXPORT QgsAiChatHistoryStore : public QObject
{
    Q_OBJECT

  public:
    struct SessionInfo
    {
        QString id;
        QString title;
        QString agent;
        QDateTime createdAt;
        QDateTime updatedAt;
        int messageCount = 0;
    };

    // v2: values (title, content, metadata_json) encrypted at rest via QgsAiSecretStore.
    static constexpr int SCHEMA_VERSION = 2;

    explicit QgsAiChatHistoryStore( QgsAiFileContextProvider *contextProvider, QObject *parent = nullptr );
    ~QgsAiChatHistoryStore() override;

    //! Sets an explicit history scope. Empty scope disables persistence instead of falling back to the workspace.
    void setHistoryScopeKey( const QString &scopeKey );

    //! Clears the explicit history scope and returns to legacy workspace-scoped persistence.
    void clearHistoryScopeKey();

    //! Returns true when an explicit history scope has been set, including an empty scope.
    bool hasExplicitHistoryScopeKey() const { return mHasExplicitHistoryScopeKey; }

    //! Returns the explicit history scope key, or an empty string when none is set.
    QString historyScopeKey() const { return mHistoryScopeKey; }

    //! Returns true when the current scope can persist chat history to SQLite.
    bool hasPersistentHistoryScope() const;

    //! Opens the SQLite database for the current history scope and creates the schema if missing.
    bool ensureReady( QString *errorMessage = nullptr );

    //! Returns the sessions for the current history scope ordered by updatedAt descending.
    QList<SessionInfo> listSessions() const;

    //! Returns all messages of the given session ordered by insertion.
    QList<QgsAiChatMessage> loadMessages( const QString &sessionId ) const;

    //! Inserts a new session row. \a id is supplied by the caller (typically a UUID).
    bool createSession( const QString &id, const QString &title, const QString &agent );

    //! Appends a message and bumps the session updated_at. Returns false on SQL error.
    bool appendMessage( const QString &sessionId, const QgsAiChatMessage &msg, int ordering );

    //! Updates metadata for an existing message. Used for UI state such as accepted/rejected plans.
    bool updateMessageMetadata( const QString &sessionId, const QString &messageId, const QVariantMap &metadata );

    //! Updates the session title. Returns false on SQL error or unknown id.
    bool renameSession( const QString &sessionId, const QString &newTitle );

    //! Removes the session and all its messages (cascade).
    bool deleteSession( const QString &sessionId );

    //! Bumps the updated_at timestamp without touching messages.
    bool touchSession( const QString &sessionId );

    //! Returns the largest ordering value stored for \a sessionId, or -1 if none.
    int lastOrdering( const QString &sessionId ) const;

  signals:
    //! Emitted when the active history scope changes and the session list may differ.
    void sessionListChanged();

  private slots:
    void onWorkspaceRootChanged();

  private:
    void resetDatabaseConnection();

    QString connectionName() const;
    QString dbPath() const;
    bool openDatabase( QString *errorMessage = nullptr ) const;

    /**
     * Encrypts every plaintext value (no `enc1:` prefix) in place, inside a
     * transaction (rollback on failure keeps the plaintext readable). No-op
     * when storage encryption is unavailable.
     */
    void migratePlaintextRows( QSqlDatabase &db );

    QgsAiFileContextProvider *mContextProvider = nullptr;
    QString mHistoryScopeKey;
    bool mHasExplicitHistoryScopeKey = false;
    mutable bool mReady = false;
};

#endif // QGSAICHATHISTORYSTORE_H
