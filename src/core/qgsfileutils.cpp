/***************************************************************************
    qgsfileutils.cpp
    ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne.trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfileutils.h"
#include <QObject>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QSet>

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
  return QStringLiteral( "%1 %2" ).arg( QString::number( bytes ), unit );
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
  QRegularExpression rx( QStringLiteral( "[/\\\\\\?%\\*\\:\\|\"<>]" ) );
  QString s = string;
  s.replace( rx, QStringLiteral( "_" ) );
  return s;
}

QString QgsFileUtils::findClosestExistingPath( const QString &path )
{
  if ( path.isEmpty() )
    return QString();

  QDir currentPath;
  QFileInfo fi( path );
  if ( fi.isFile() )
    currentPath = fi.dir();
  else
    currentPath = QDir( path );

  QSet< QString > visited;
  while ( !currentPath.exists() )
  {
    const QString parentPath = QDir::cleanPath( currentPath.absolutePath() + QStringLiteral( "/.." ) );
    if ( visited.contains( parentPath ) )
      return QString(); // break circular links

    if ( parentPath.isEmpty() || parentPath == QStringLiteral( "." ) )
      return QString();
    currentPath = QDir( parentPath );
    visited << parentPath;
  }

  const QString res = QDir::cleanPath( currentPath.absolutePath() );

  if ( res == QDir::currentPath() )
    return QString(); // avoid default to binary folder if a filename alone is specified

  return res == QStringLiteral( "." ) ? QString() : res;
}
