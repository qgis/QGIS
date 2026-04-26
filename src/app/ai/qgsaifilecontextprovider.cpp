#include "qgsaifilecontextprovider.h"

#include <algorithm>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

QgsAiFileContextProvider::QgsAiFileContextProvider( const QString &workspaceRoot, QObject *parent )
  : QObject( parent )
  , mWorkspaceRoot( QDir( workspaceRoot ).absolutePath() )
{
}

bool QgsAiFileContextProvider::isInWorkspace( const QString &absolutePath ) const
{
  if ( absolutePath.isEmpty() || mWorkspaceRoot.isEmpty() )
    return false;

  const QString relativePath = QDir( mWorkspaceRoot ).relativeFilePath( absolutePath );
  return relativePath == QLatin1String( "." )
         || ( !relativePath.startsWith( QStringLiteral( "../" ) )
              && relativePath != QLatin1String( ".." )
              && !QDir::isAbsolutePath( relativePath ) );
}

QString QgsAiFileContextProvider::normalizePath( const QString &filePath, bool allowExternal ) const
{
  if ( filePath.isEmpty() )
    return QString();

  QFileInfo info( filePath );
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
    if ( relativePath.startsWith( QStringLiteral( ".git/" ) )
         || relativePath.startsWith( QStringLiteral( "build/" ) )
         || relativePath.startsWith( QStringLiteral( "external/" ) )
         || relativePath.startsWith( QStringLiteral( "i18n/" ) )
         || relativePath.startsWith( QStringLiteral( "tests/testdata/" ) )
         || relativePath.startsWith( QStringLiteral( "vcpkg/" ) ) )
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
      matches << QStringLiteral( "%1:%2" ).arg( lineNumber + 1 ).arg( line.trimmed() );
  }

  return matches;
}

QString QgsAiFileContextProvider::diffPreview( const QString &beforeText, const QString &afterText ) const
{
  if ( beforeText == afterText )
    return QStringLiteral( "No changes." );

  QString preview;
  preview += QStringLiteral( "--- before\n" );
  preview += beforeText.left( 2000 );
  preview += QStringLiteral( "\n+++ after\n" );
  preview += afterText.left( 2000 );
  return preview;
}
