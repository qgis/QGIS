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

    QgsAiFileContext buildContext( const QString &filePath, const QString &selectedText = QString(), int maxBytes = 16384, bool allowExternal = false ) const;
    QString resolveWorkspaceFile( const QString &filePath ) const;
    QStringList workspaceFileCandidates( const QString &query, int maxResults = 25 ) const;
    QStringList searchInFile( const QString &filePath, const QString &needle, int maxMatches = 25 ) const;
    QString diffPreview( const QString &beforeText, const QString &afterText ) const;
    QString workspaceRoot() const { return mWorkspaceRoot; }

  private:
    QString normalizePath( const QString &filePath, bool allowExternal = false ) const;
    bool isInWorkspace( const QString &absolutePath ) const;
    QString mWorkspaceRoot;
};

#endif // QGSAIFILECONTEXTPROVIDER_H
