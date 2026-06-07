/***************************************************************************
    qgsaifilecontextprovider.h
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

#ifndef QGSAIFILECONTEXTPROVIDER_H
#define QGSAIFILECONTEXTPROVIDER_H

#include "qgis_app.h"

#include <QObject>
#include <QString>
#include <QStringList>

struct APP_EXPORT QgsAiFileContext
{
    QString filePath;
    QString selectedText;
    QString fileSnippet;
    qint64 fileSize = 0;
    bool truncated = false;
    bool binary = false;

    bool isValid() const { return !filePath.isEmpty() || !selectedText.isEmpty(); }
};

class APP_EXPORT QgsAiFileContextProvider : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiFileContextProvider( const QString &workspaceRoot, QObject *parent = nullptr );

    /**
     * Resolves the AI workspace root: project home path, then strata/workspace/root,
     * then legacy geoai/qgis_ai settings, then a profile-local default.
     */
    static QString resolveWorkspaceRoot();

    QgsAiFileContext buildContext( const QString &filePath, const QString &selectedText = QString(), int maxBytes = 16384, bool allowExternal = false ) const;
    QString resolveWorkspaceFile( const QString &filePath ) const;
    QStringList workspaceFileCandidates( const QString &query, int maxResults = 25 ) const;
    QStringList searchInFile( const QString &filePath, const QString &needle, int maxMatches = 25 ) const;
    QString diffPreview( const QString &beforeText, const QString &afterText ) const;
    QString workspaceRoot() const { return mWorkspaceRoot; }
    void setWorkspaceRoot( const QString &workspaceRoot );

    /**
     * Returns the absolute, cleaned path for \a filePath. If \a allowExternal is false (default)
     * and the resolved path is not inside the workspace root, returns an empty string.
     * Public so tools that write/download into the workspace (e.g. download_file) can validate
     * destination paths against the same boundary used by read tools.
     */
    QString normalizePath( const QString &filePath, bool allowExternal = false ) const;

    /**
     * Returns true iff \a absolutePath is inside the workspace root (or equal to it).
     * Public for the same reason as normalizePath.
     */
    bool isInWorkspace( const QString &absolutePath ) const;

  signals:
    void workspaceRootChanged( const QString &workspaceRoot );

  private:
    QString mWorkspaceRoot;
};

#endif // QGSAIFILECONTEXTPROVIDER_H
