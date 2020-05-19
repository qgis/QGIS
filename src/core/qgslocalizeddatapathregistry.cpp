/***************************************************************************
  qgslocalizeddatapathregistry.cpp
  --------------------------------------
  Date                 : May 2020
  Copyright            : (C) 2020 by Denis Rouzaud
  Email                : denis.rouzaud
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDir>

#include "qgslocalizeddatapathregistry.h"
#include "qgssettings.h"
#include "qgis.h"
#include "qgsreadwritelocker.h"


QgsLocalizedDataPathRegistry::QgsLocalizedDataPathRegistry()
{
  readFromSettings();
}

QString QgsLocalizedDataPathRegistry::globalPath( const QString &relativePath ) const
{
  for ( const QDir &basePath : qgis::as_const( mPaths ) )
    if ( basePath.exists( relativePath ) )
      return basePath.absoluteFilePath( relativePath );

  return QString();
}

QString QgsLocalizedDataPathRegistry::localizedPath( const QString &fullPath ) const
{
  for ( const QDir &basePath : qgis::as_const( mPaths ) )
    if ( fullPath.startsWith( basePath.absolutePath() ) )
      return basePath.relativeFilePath( fullPath );

  return QString();

}

QStringList QgsLocalizedDataPathRegistry::paths() const
{
  QStringList paths;
  for ( const QDir &dir : mPaths )
    paths << dir.absolutePath();
  return paths;
}

void QgsLocalizedDataPathRegistry::setPaths( const QStringList &paths )
{
  mPaths.clear();
  for ( const QString &path : paths )
  {
    QDir dir( path );
    if ( !mPaths.contains( dir ) )
      mPaths << dir;
  }

  writeToSettings();
}

void QgsLocalizedDataPathRegistry::registerPath( const QString &path, int position )
{
  QDir dir( path );
  if ( mPaths.contains( dir ) )
    return;

  if ( position >= 0 && position < mPaths.count() )
    mPaths.insert( position, dir );
  else
    mPaths.append( dir );

  writeToSettings();
}

void QgsLocalizedDataPathRegistry::unregisterPath( const QString &path )
{
  mPaths.removeAll( QDir( path ) );
  writeToSettings();
}

void QgsLocalizedDataPathRegistry::readFromSettings()
{
  setPaths( QgsSettings().value( QStringLiteral( "/qgis/localized_data_paths" ) ).toStringList() );
}

void QgsLocalizedDataPathRegistry::writeToSettings()
{
  QgsSettings().setValue( QStringLiteral( "/qgis/localized_data_paths" ), paths() );
}
