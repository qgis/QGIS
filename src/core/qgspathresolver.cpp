/***************************************************************************
  qgspathresolver.cpp
  --------------------------------------
  Date                 : February 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspathresolver.h"
#include "qgslocalizeddatapathregistry.h"

#include "qgis.h"
#include "qgsapplication.h"
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QUuid>


typedef std::vector< std::pair< QString, std::function< QString( const QString & ) > > > CustomResolvers;
Q_GLOBAL_STATIC( CustomResolvers, sCustomResolvers )
Q_GLOBAL_STATIC( CustomResolvers, sCustomWriters )

QgsPathResolver::QgsPathResolver( const QString &baseFileName, const QString &attachmentDir )
  : mBaseFileName( baseFileName ), mAttachmentDir( attachmentDir )
{
}


QString QgsPathResolver::readPath( const QString &f ) const
{
  QString filename = f;

  const CustomResolvers customResolvers = *sCustomResolvers();
  for ( const auto &resolver : customResolvers )
    filename = resolver.second( filename );

  if ( filename.isEmpty() )
    return QString();

  QString src = filename;
  if ( src.startsWith( QLatin1String( "inbuilt:" ) ) )
  {
    // strip away "inbuilt:" prefix, replace with actual  inbuilt data folder path
    return QgsApplication::pkgDataPath() + QStringLiteral( "/resources" ) + src.mid( 8 );
  }

  if ( src.startsWith( QLatin1String( "localized:" ) ) )
  {
    QStringList parts = src.split( "|" );
    // strip away "localized:" prefix, replace with actual  inbuilt data folder path
    parts[0] = QgsApplication::localizedDataPathRegistry()->globalPath( parts[0].mid( 10 ) ) ;
    if ( !parts[0].isEmpty() )
    {
      return parts.join( "|" );
    }
    else
    {
      return QString();
    }
  }
  if ( src.startsWith( QLatin1String( "attachment:" ) ) )
  {
    // resolve attachment w.r.t. temporary path where project archive is extracted
    return QDir( mAttachmentDir ).absoluteFilePath( src.mid( 11 ) );
  }

  if ( mBaseFileName.isNull() )
  {
    return src;
  }

  // if this is a VSIFILE, remove the VSI prefix and append to final result
  QString vsiPrefix = qgsVsiPrefix( src );
  if ( ! vsiPrefix.isEmpty() )
  {
    // unfortunately qgsVsiPrefix returns prefix also for files like "/x/y/z.gz"
    // so we need to check if we really have the prefix
    if ( src.startsWith( QLatin1String( "/vsi" ), Qt::CaseInsensitive ) )
      src.remove( 0, vsiPrefix.size() );
    else
      vsiPrefix.clear();
  }

  // relative path should always start with ./ or ../
  if ( !src.startsWith( QLatin1String( "./" ) ) && !src.startsWith( QLatin1String( "../" ) ) )
  {
#if defined(Q_OS_WIN)
    if ( src.startsWith( "\\\\" ) ||
         src.startsWith( "//" ) ||
         ( src[0].isLetter() && src[1] == ':' ) )
    {
      // UNC or absolute path
      return vsiPrefix + src;
    }
#else
    if ( src[0] == '/' )
    {
      // absolute path
      return vsiPrefix + src;
    }
#endif

    // so this one isn't absolute, but also doesn't start // with ./ or ../.
    // That means that it was saved with an earlier version of "relative path support",
    // where the source file had to exist and only the project directory was stripped
    // from the filename.

    const QFileInfo pfi( mBaseFileName );
    const QString home = pfi.absolutePath();
    if ( home.isEmpty() )
      return vsiPrefix + src;

    const QFileInfo fi( home + '/' + src );

    if ( !fi.exists() )
    {
      return vsiPrefix + src;
    }
    else
    {
      return vsiPrefix + fi.canonicalFilePath();
    }
  }

  QString srcPath = src;
  QString projPath = mBaseFileName;

  if ( projPath.isEmpty() )
  {
    return vsiPrefix + src;
  }

#if defined(Q_OS_WIN)
  srcPath.replace( '\\', '/' );
  projPath.replace( '\\', '/' );

  bool uncPath = projPath.startsWith( "//" );
#endif

  // Make sure the path is absolute (see GH #33200)
  projPath = QFileInfo( projPath ).absoluteFilePath();

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList srcElems = srcPath.split( '/', QString::SkipEmptyParts );
  QStringList projElems = projPath.split( '/', QString::SkipEmptyParts );
#else
  const QStringList srcElems = srcPath.split( '/', Qt::SkipEmptyParts );
  QStringList projElems = projPath.split( '/', Qt::SkipEmptyParts );
#endif

#if defined(Q_OS_WIN)
  if ( uncPath )
  {
    projElems.insert( 0, "" );
    projElems.insert( 0, "" );
  }
#endif

  // remove project file element
  projElems.removeLast();

  // append source path elements
  projElems << srcElems;
  projElems.removeAll( QStringLiteral( "." ) );

  // resolve ..
  int pos;
  while ( ( pos = projElems.indexOf( QLatin1String( ".." ) ) ) > 0 )
  {
    // remove preceding element and ..
    projElems.removeAt( pos - 1 );
    projElems.removeAt( pos - 1 );
  }

#if !defined(Q_OS_WIN)
  // make path absolute
  projElems.prepend( QString() );
#endif

  return vsiPrefix + projElems.join( QLatin1Char( '/' ) );
}

QString QgsPathResolver::setPathPreprocessor( const std::function<QString( const QString & )> &processor )
{
  QString id = QUuid::createUuid().toString();
  sCustomResolvers()->emplace_back( std::make_pair( id, processor ) );
  return id;
}

bool QgsPathResolver::removePathPreprocessor( const QString &id )
{
  const size_t prevCount = sCustomResolvers()->size();
  sCustomResolvers()->erase( std::remove_if( sCustomResolvers()->begin(), sCustomResolvers()->end(), [id]( std::pair< QString, std::function< QString( const QString & ) > > &a )
  {
    return a.first == id;
  } ), sCustomResolvers()->end() );
  return prevCount != sCustomResolvers()->size();
}

QString QgsPathResolver::setPathWriter( const std::function<QString( const QString & )> &writer )
{
  QString id = QUuid::createUuid().toString();
  sCustomWriters()->emplace_back( std::make_pair( id, writer ) );
  return id;
}

bool QgsPathResolver::removePathWriter( const QString &id )
{
  const size_t prevCount = sCustomWriters->size();
  sCustomWriters()->erase( std::remove_if( sCustomWriters->begin(), sCustomWriters->end(), [id]( std::pair< QString, std::function< QString( const QString & ) > > &a )
  {
    return a.first == id;
  } ), sCustomWriters->end() );
  return prevCount != sCustomWriters->size();
}

QString QgsPathResolver::writePath( const QString &s ) const
{
  QString src = s;
  if ( src.isEmpty() )
  {
    return src;
  }

  const QString localizedPath = QgsApplication::localizedDataPathRegistry()->localizedPath( src );
  if ( !localizedPath.isEmpty() )
    return QStringLiteral( "localized:" ) + localizedPath;

  const CustomResolvers customWriters = *sCustomWriters();
  for ( const auto &writer :  customWriters )
    src = writer.second( src );

  if ( src.startsWith( QgsApplication::pkgDataPath() + QStringLiteral( "/resources" ) ) )
  {
    // replace inbuilt data folder path with "inbuilt:" prefix
    return QStringLiteral( "inbuilt:" ) + src.mid( QgsApplication::pkgDataPath().length() + 10 );
  }

  if ( !mAttachmentDir.isEmpty() && src.startsWith( mAttachmentDir ) )
  {
    // Replace attachment dir with "attachment:" prefix
    return QStringLiteral( "attachment:" ) + QFileInfo( src ).fileName();
  }

  if ( mBaseFileName.isEmpty() )
  {
    return src;
  }

  // Get projPath even if project has not been created yet
  const QFileInfo pfi( QFileInfo( mBaseFileName ).path() );
  QString projPath = pfi.canonicalFilePath();

  // If project directory doesn't exit, fallback to absoluteFilePath : symbolic
  // links won't be handled correctly, but that's OK as the path is "virtual".
  if ( projPath.isEmpty() )
    projPath = pfi.absoluteFilePath();

  if ( projPath.isEmpty() )
  {
    return src;
  }

  // Check if it is a publicSource uri and clean it
  const QUrl url { src };
  QString srcPath { src };
  QString urlQuery;

  if ( url.isLocalFile( ) )
  {
    srcPath = url.path();
    urlQuery = url.query();
  }

  const QFileInfo srcFileInfo( srcPath );
  if ( srcFileInfo.exists() )
    srcPath = srcFileInfo.canonicalFilePath();

  // if this is a VSIFILE, remove the VSI prefix and append to final result
  const QString vsiPrefix = qgsVsiPrefix( src );
  if ( ! vsiPrefix.isEmpty() )
  {
    srcPath.remove( 0, vsiPrefix.size() );
  }

#if defined( Q_OS_WIN )
  const Qt::CaseSensitivity cs = Qt::CaseInsensitive;

  srcPath.replace( '\\', '/' );

  if ( srcPath.startsWith( "//" ) )
  {
    // keep UNC prefix
    srcPath = "\\\\" + srcPath.mid( 2 );
  }

  projPath.replace( '\\', '/' );
  if ( projPath.startsWith( "//" ) )
  {
    // keep UNC prefix
    projPath = "\\\\" + projPath.mid( 2 );
  }
#else
  const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList projElems = projPath.split( '/', QString::SkipEmptyParts );
  QStringList srcElems = srcPath.split( '/', QString::SkipEmptyParts );
#else
  QStringList projElems = projPath.split( '/', Qt::SkipEmptyParts );
  QStringList srcElems = srcPath.split( '/', Qt::SkipEmptyParts );
#endif

  projElems.removeAll( QStringLiteral( "." ) );
  srcElems.removeAll( QStringLiteral( "." ) );

  // remove common part
  int n = 0;
  while ( !srcElems.isEmpty() &&
          !projElems.isEmpty() &&
          srcElems[0].compare( projElems[0], cs ) == 0 )
  {
    srcElems.removeFirst();
    projElems.removeFirst();
    n++;
  }

  if ( n == 0 )
  {
    // no common parts; might not even be a file
    return src;
  }

  if ( !projElems.isEmpty() )
  {
    // go up to the common directory
    for ( int i = 0; i < projElems.size(); i++ )
    {
      srcElems.insert( 0, QStringLiteral( ".." ) );
    }
  }
  else
  {
    // let it start with . nevertheless,
    // so relative path always start with either ./ or ../
    srcElems.insert( 0, QStringLiteral( "." ) );
  }

  // Append url query if any
  QString returnPath { vsiPrefix + srcElems.join( QLatin1Char( '/' ) ) };
  if ( ! urlQuery.isEmpty() )
  {
    returnPath.append( '?' );
    returnPath.append( urlQuery );
  }
  return returnPath;
}
