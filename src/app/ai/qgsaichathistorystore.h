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

class QgsAiFileContextProvider;

/**
 * Persists chat sessions per workspace in a SQLite database, mirroring the
 * layout used by QgsAiWorkspaceIndex. One file per workspace, stored under
 * QgsApplication::qgisSettingsDirPath()/ai_chat/ws_<hash>.sqlite where the
 * hash is derived from the workspace root path.
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

    static constexpr int SCHEMA_VERSION = 1;

    explicit QgsAiChatHistoryStore( QgsAiFileContextProvider *contextProvider, QObject *parent = nullptr );
    ~QgsAiChatHistoryStore() override;

    //! Opens the SQLite database for the current workspace and creates the schema if missing.
    bool ensureReady( QString *errorMessage = nullptr );

    //! Returns the sessions for the current workspace ordered by updatedAt descending.
    QList<SessionInfo> listSessions() const;

    //! Returns all messages of the given session ordered by insertion.
    QList<QgsAiChatMessage> loadMessages( const QString &sessionId ) const;

    //! Inserts a new session row. \a id is supplied by the caller (typically a UUID).
    bool createSession( const QString &id, const QString &title, const QString &agent );

    //! Appends a message and bumps the session updated_at. Returns false on SQL error.
    bool appendMessage( const QString &sessionId, const QgsAiChatMessage &msg, int ordering );

    //! Updates the session title. Returns false on SQL error or unknown id.
    bool renameSession( const QString &sessionId, const QString &newTitle );

    //! Removes the session and all its messages (cascade).
    bool deleteSession( const QString &sessionId );

    //! Bumps the updated_at timestamp without touching messages.
    bool touchSession( const QString &sessionId );

    //! Returns the largest ordering value stored for \a sessionId, or -1 if none.
    int lastOrdering( const QString &sessionId ) const;

  private:
    QString connectionName() const;
    QString dbPath() const;
    bool openDatabase( QString *errorMessage = nullptr ) const;

    QgsAiFileContextProvider *mContextProvider = nullptr;
    mutable bool mReady = false;
};

#endif // QGSAICHATHISTORYSTORE_H
