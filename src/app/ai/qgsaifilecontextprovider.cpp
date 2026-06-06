/***************************************************************************
    qgsaifilecontextprovider.cpp
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

#include "qgsaifilecontextprovider.h"

#include <algorithm>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QString>

#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgssettings.h"

#include "moc_qgsaifilecontextprovider.cpp"

using namespace Qt::StringLiterals;

QString QgsAiFileContextProvider::resolveWorkspaceRoot()
{
  const QString projectHome = QgsProject::instance()->homePath().trimmed();
  if ( !projectHome.isEmpty() )
    return QDir( projectHome ).absolutePath();

  QgsSettings settings;
  const QString settingsKey = u"geoai/workspace/root"_s;
  const QString legacyKey = u"qgis_ai/workspace/root"_s;
  QString configured = settings.value( settingsKey, settings.value( legacyKey, QString() ) ).toString().trimmed();
  if ( !configured.isEmpty() )
    return QDir( configured ).absolutePath();

  const QString defaultRoot = QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"ai_workspace"_s );
  QDir().mkpath( defaultRoot );
  const QString absoluteDefaultRoot = QDir( defaultRoot ).absolutePath();
  settings.setValue( settingsKey, absoluteDefaultRoot );
  settings.remove( legacyKey );
  return absoluteDefaultRoot;
}

QgsAiFileContextProvider::QgsAiFileContextProvider( const QString &workspaceRoot, QObject *parent )
  : QObject( parent )
  , mWorkspaceRoot( workspaceRoot.trimmed().isEmpty() ? QString() : QDir( workspaceRoot ).absolutePath() )
{}

void QgsAiFileContextProvider::setWorkspaceRoot( const QString &workspaceRoot )
{
  const QString normalizedRoot = workspaceRoot.trimmed().isEmpty() ? QString() : QDir( workspaceRoot ).absolutePath();
  if ( mWorkspaceRoot == normalizedRoot )
    return;

  mWorkspaceRoot = normalizedRoot;
  emit workspaceRootChanged( mWorkspaceRoot );
}

bool QgsAiFileContextProvider::isInWorkspace( const QString &absolutePath ) const
{
  if ( absolutePath.isEmpty() || mWorkspaceRoot.isEmpty() )
    return false;

  const QString relativePath = QDir( mWorkspaceRoot ).relativeFilePath( absolutePath );
  return relativePath == "."_L1 || ( !relativePath.startsWith( "../"_L1 ) && relativePath != ".."_L1 && !QDir::isAbsolutePath( relativePath ) );
}

QString QgsAiFileContextProvider::normalizePath( const QString &filePath, bool allowExternal ) const
{
  if ( filePath.isEmpty() )
    return QString();

  QFileInfo info( filePath );
  if ( mWorkspaceRoot.isEmpty() && ( !allowExternal || !info.isAbsolute() ) )
    return QString();

  const QString absolutePath = info.isAbsolute() ? info.absoluteFilePath() : QDir( mWorkspaceRoot ).absoluteFilePath( filePath );
  const QString cleanPath = QDir::cleanPath( absolutePath );

  if ( !allowExternal && !isInWorkspace( cleanPath ) )
    return QString();

  return cleanPath;
}

QString QgsAiFileContextProvider::resolveWorkspaceFile( const QString &filePath ) const
{
  const QString normalizedPath = normalizePath( filePath );
  if ( normalizedPath.isEmpty() )
    return QString();

  QFileInfo info( normalizedPath );
  if ( !info.exists() || !info.isFile() )
    return QString();

  return normalizedPath;
}

QStringList QgsAiFileContextProvider::workspaceFileCandidates( const QString &query, int maxResults ) const
{
  QStringList candidates;
  if ( maxResults <= 0 || mWorkspaceRoot.isEmpty() )
    return candidates;

  const QString normalizedQuery = query.trimmed();
  QDir rootDir( mWorkspaceRoot );
  QDirIterator iterator( mWorkspaceRoot, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories );
  int visited = 0;
  while ( iterator.hasNext() && candidates.size() < maxResults && visited < 50000 )
  {
    const QString absolutePath = QDir::cleanPath( iterator.next() );
    ++visited;

    const QString relativePath = rootDir.relativeFilePath( absolutePath );
    if ( relativePath.startsWith( ".git/"_L1 )
         || relativePath.startsWith( "build/"_L1 )
         || relativePath.startsWith( "external/"_L1 )
         || relativePath.startsWith( "i18n/"_L1 )
         || relativePath.startsWith( "tests/testdata/"_L1 )
         || relativePath.startsWith( "vcpkg/"_L1 ) )
    {
      continue;
    }

    if ( normalizedQuery.isEmpty() || relativePath.contains( normalizedQuery, Qt::CaseInsensitive ) )
      candidates << relativePath;
  }

  candidates.sort( Qt::CaseInsensitive );
  return candidates;
}

QgsAiFileContext QgsAiFileContextProvider::buildContext( const QString &filePath, const QString &selectedText, int maxBytes, bool allowExternal ) const
{
  QgsAiFileContext context;
  context.selectedText = selectedText;

  const QString normalizedPath = normalizePath( filePath, allowExternal );
  if ( normalizedPath.isEmpty() )
    return context;

  context.filePath = normalizedPath;
  context.fileSize = QFileInfo( normalizedPath ).size();
  QFile file( normalizedPath );
  if ( !file.open( QIODevice::ReadOnly ) )
    return context;

  QByteArray content = file.read( std::max( 0, maxBytes ) + 1 );
  context.truncated = content.size() > maxBytes;
  if ( context.truncated )
    content.truncate( maxBytes );

  context.binary = content.contains( '\0' );
  if ( context.binary )
    return context;

  context.fileSnippet = QString::fromUtf8( content );
  return context;
}

QStringList QgsAiFileContextProvider::searchInFile( const QString &filePath, const QString &needle, int maxMatches ) const
{
  QStringList matches;
  if ( needle.isEmpty() || maxMatches <= 0 )
    return matches;

  const QgsAiFileContext context = buildContext( filePath, QString(), 512 * 1024 );
  if ( context.filePath.isEmpty() || context.fileSnippet.isEmpty() )
    return matches;

  const QStringList lines = context.fileSnippet.split( '\n' );
  for ( int lineNumber = 0; lineNumber < lines.size() && matches.size() < maxMatches; ++lineNumber )
  {
    const QString line = lines.at( lineNumber );
    if ( line.contains( needle, Qt::CaseInsensitive ) )
      matches << u"%1:%2"_s.arg( lineNumber + 1 ).arg( line.trimmed() );
  }

  return matches;
}

QString QgsAiFileContextProvider::diffPreview( const QString &beforeText, const QString &afterText ) const
{
  if ( beforeText == afterText )
    return u"No changes."_s;

  QString preview;
  preview += "--- before\n"_L1;
  preview += beforeText.left( 2000 );
  preview += "\n+++ after\n"_L1;
  preview += afterText.left( 2000 );
  return preview;
}
