/***************************************************************************
                        qgsuserprofile.h
     --------------------------------------
    Date                 :  Jul-2017
    Copyright            : (C) 2017 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsuserprofile.h"
#include "qgsapplication.h"
#include "qgssqliteutils.h"

#include <QDir>
#include <QTextStream>
#include <QSettings>
#include <sqlite3.h>

QgsUserProfile::QgsUserProfile( const QString &folder )
{
  mProfileFolder = folder;
}

const QString QgsUserProfile::folder() const
{
  return mProfileFolder;
}

QgsError QgsUserProfile::validate() const
{
  QgsError error;
  if ( !QDir( mProfileFolder ).exists() )
  {
    error.append( QObject::tr( "Profile folder doesn't exist" ) );
  }
  return error;
}

const QString QgsUserProfile::name() const
{
  const QDir dir( mProfileFolder );
  return dir.dirName();
}

void QgsUserProfile::initSettings() const
{
  // tell QSettings to use INI format and save the file in custom config path
  QSettings::setDefaultFormat( QSettings::IniFormat );
  QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, folder() );
}

const QString QgsUserProfile::alias() const
{
  const QString dbFile = qgisDB();
  QString profileAlias = name();

  // Looks for qgis.db
  // If it's not there we can just return name.
  if ( !QFile::exists( dbFile ) )
  {
    return profileAlias;
  }

  sqlite3_database_unique_ptr database;

  //check the db is available
  int result = database.open( dbFile );
  if ( result != SQLITE_OK )
  {
    return profileAlias;
  }

  sqlite3_statement_unique_ptr preparedStatement = database.prepare( QStringLiteral( "SELECT value FROM tbl_config_variables WHERE variable = 'ALIAS'" ), result );
  if ( result == SQLITE_OK )
  {
    if ( preparedStatement.step() == SQLITE_ROW )
    {
      const QString alias = preparedStatement.columnAsText( 0 );
      if ( !alias.isEmpty() )
        profileAlias = alias;
    }
  }
  return profileAlias;
}

QgsError QgsUserProfile::setAlias( const QString &alias ) const
{
  QgsError error;
  const QString dbFile = qgisDB();

  // Looks for qgis.db
  // If it's not there we can just return name.
  if ( !QFile::exists( dbFile ) )
  {
    error.append( QObject::tr( "qgis.db doesn't exist in the user's profile folder" ) );
    return error;
  }

  sqlite3_database_unique_ptr database;

  //check the db is available
  int result = database.open( dbFile );
  if ( result != SQLITE_OK )
  {
    error.append( QObject::tr( "Unable to open qgis.db for update." ) );
    return error;
  }

  const QString sql = QStringLiteral( "INSERT OR REPLACE INTO tbl_config_variables VALUES ('ALIAS', %1);" ).arg(
                        QgsSqliteUtils::quotedString( alias ) );

  sqlite3_statement_unique_ptr preparedStatement = database.prepare( sql, result );
  if ( result != SQLITE_OK || preparedStatement.step() != SQLITE_DONE )
  {
    error.append( QObject::tr( "Could not save alias to database: %1" ).arg( database.errorMessage() ) );
  }

  return error;
}

const QIcon QgsUserProfile::icon() const
{
  const QString path = mProfileFolder + QDir::separator() + "icon.svg";
  if ( !QDir( path ).exists() )
  {
    return QgsApplication::getThemeIcon( "user.svg" );
  }
  return QIcon( path );
}

QString QgsUserProfile::qgisDB() const
{
  return mProfileFolder + QDir::separator() + "qgis.db";
}
