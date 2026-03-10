/***************************************************************************
                           qgsapplicationthemeregistry.cpp
                             -------------------
    begin                : January 2026
    copyright            : (C) 2026 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplicationthemeregistry.h"

#include "qgsapplication.h"

#include <QDir>
#include <QString>

using namespace Qt::StringLiterals;

QgsApplicationThemeRegistry::QgsApplicationThemeRegistry()
{
  addApplicationThemes();
}

void QgsApplicationThemeRegistry::addApplicationThemes()
{
  mThemes.insert( "default"_L1, QString() );

  const QStringList paths = QStringList() << QgsApplication::instance()->userThemesFolder() << QgsApplication::instance()->defaultThemesFolder();
  for ( const QString &path : paths )
  {
    QDir folder( path );
    const QFileInfoList folderInfos = folder.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );
    for ( const QFileInfo &folderInfo : folderInfos )
    {
      const QString name = folderInfo.baseName();
      const QString folder = folderInfo.absoluteFilePath();
      addTheme( name, folder );
    }
  }
}

bool QgsApplicationThemeRegistry::addTheme( const QString &name, const QString &folder )
{
  if ( mThemes.contains( name ) )
  {
    return false;
  }

  const QFileInfo folderInfo( folder );
  if ( !folderInfo.exists() )
  {
    return false;
  }

  const QFileInfo styleInfo( folderInfo.absoluteFilePath() + "/style.qss" );
  if ( !styleInfo.exists() )
  {
    return false;
  }

  mThemes.insert( name, folder );
  return true;
}

bool QgsApplicationThemeRegistry::removeTheme( const QString &name )
{
  if ( !mThemes.contains( name ) )
  {
    return false;
  }

  mThemes.remove( name );
  return true;
}

QStringList QgsApplicationThemeRegistry::themes() const
{
  return mThemes.keys();
}

QString QgsApplicationThemeRegistry::themeFolder( const QString &name ) const
{
  return mThemes.value( name );
}

QHash<QString, QString> QgsApplicationThemeRegistry::themeFolders() const
{
  return mThemes;
}
