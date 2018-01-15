#include "qgsfileutils.h"
#include <QObject>
#include <QRegularExpression>

QString QgsFileUtils::representFileSize( qint64 bytes )
{
  QStringList list;
  list << QObject::tr( "KB" ) << QObject::tr( "MB" ) << QObject::tr( "GB" ) << QObject::tr( "TB" );

  QStringListIterator i( list );
  QString unit = QObject::tr( "bytes" );

  while ( bytes >= 1024.0 && i.hasNext() )
  {
    unit = i.next();
    bytes /= 1024.0;
  }
  return QString( "%1 %2" ).arg( QString::number( bytes ), unit );
}

QStringList QgsFileUtils::extensionsFromFilter( const QString &filter )
{
  const QRegularExpression rx( QStringLiteral( "\\*\\.([a-zA-Z0-9]+)" ) );
  QStringList extensions;
  QRegularExpressionMatchIterator matches = rx.globalMatch( filter );

  while ( matches.hasNext() )
  {
    const QRegularExpressionMatch match = matches.next();
    if ( match.hasMatch() )
    {
      QStringList newExtensions = match.capturedTexts();
      newExtensions.pop_front(); // remove whole match
      extensions.append( newExtensions );
    }
  }
  return extensions;
}

QString QgsFileUtils::ensureFileNameHasExtension( const QString &f, const QStringList &extensions )
{
  if ( extensions.empty() || f.isEmpty() )
    return f;

  QString fileName = f;
  bool hasExt = false;
  for ( const QString &extension : qgis::as_const( extensions ) )
  {
    const QString extWithDot = extension.startsWith( '.' ) ? extension : '.' + extension;
    if ( fileName.endsWith( extWithDot, Qt::CaseInsensitive ) )
    {
      hasExt = true;
      break;
    }
  }

  if ( !hasExt )
  {
    const QString extension = extensions.at( 0 );
    const QString extWithDot = extension.startsWith( '.' ) ? extension : '.' + extension;
    fileName += extWithDot;
  }

  return fileName;
}

QString QgsFileUtils::addExtensionFromFilter( const QString &fileName, const QString &filter )
{
  const QStringList extensions = extensionsFromFilter( filter );
  return ensureFileNameHasExtension( fileName, extensions );
}

QString QgsFileUtils::stringToSafeFilename( const QString &string )
{
  QRegularExpression rx( "[^\\w\\-. ]" );
  QString s = string;
  s.replace( rx, QStringLiteral( "_" ) );
  return s;
}
