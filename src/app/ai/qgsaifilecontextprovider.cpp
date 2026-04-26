#include "qgsaifilecontextprovider.h"

#include <algorithm>
#include <QDir>
#include <QFile>
#include <QFileInfo>

QgsAiFileContextProvider::QgsAiFileContextProvider( const QString &workspaceRoot, QObject *parent )
  : QObject( parent )
  , mWorkspaceRoot( QDir( workspaceRoot ).absolutePath() )
{
}

QString QgsAiFileContextProvider::normalizePath( const QString &filePath ) const
{
  if ( filePath.isEmpty() )
    return QString();

  QFileInfo info( filePath );
  const QString absolutePath = info.isAbsolute() ? info.absoluteFilePath() : QDir( mWorkspaceRoot ).absoluteFilePath( filePath );
  const QString cleanPath = QDir::cleanPath( absolutePath );

  if ( !cleanPath.startsWith( mWorkspaceRoot ) )
    return QString();

  return cleanPath;
}

QgsAiFileContext QgsAiFileContextProvider::buildContext( const QString &filePath, const QString &selectedText, int maxBytes ) const
{
  QgsAiFileContext context;
  context.selectedText = selectedText;

  const QString normalizedPath = normalizePath( filePath );
  if ( normalizedPath.isEmpty() )
    return context;

  context.filePath = normalizedPath;
  QFile file( normalizedPath );
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    return context;

  QByteArray content = file.read( std::max( 0, maxBytes ) + 1 );
  context.truncated = content.size() > maxBytes;
  if ( context.truncated )
    content.truncate( maxBytes );
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
