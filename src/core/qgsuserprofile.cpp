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

#include <QDir>
#include <QTextStream>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

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
  QDir dir( mProfileFolder );
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
  QFile qgisPrivateDbFile( qgisDB() );

  // Looks for qgis.db
  // If it's not there we can just return name.
  if ( !qgisPrivateDbFile.exists() )
  {
    return name();
  }

  QSqlDatabase db = QSqlDatabase::addDatabase( QStringLiteral( "QSQLITE" ) );
  db.setDatabaseName( qgisDB() );
  if ( !db.open() )
    return name();

  QSqlQuery query;
  query.prepare( QStringLiteral( "SELECT value FROM tbl_config_variables WHERE variable = 'ALIAS'" ) );
  QString profileAlias = name();
  if ( query.exec() )
  {
    if ( query.next() )
    {
      QString alias = query.value( 0 ).toString();
      if ( !alias.isEmpty() )
        profileAlias = alias;
    }
  }
  db.close();
  return profileAlias;
}

QgsError QgsUserProfile::setAlias( const QString &alias )
{
  QgsError error;
  QFile qgisPrivateDbFile( qgisDB() );

  if ( !qgisPrivateDbFile.exists() )
  {
    error.append( QObject::tr( "qgis.db doesn't exist in the user's profile folder" ) );
    return error;
  }

  QSqlDatabase db = QSqlDatabase::addDatabase( QStringLiteral( "QSQLITE" ), QStringLiteral( "userprofile" ) );
  db.setDatabaseName( qgisDB() );
  if ( !db.open() )
  {
    error.append( QObject::tr( "Unable to open qgis.db for update." ) );
    return error;
  }

  QSqlQuery query;
  QString sql = QStringLiteral( "INSERT OR REPLACE INTO tbl_config_variables VALUES ('ALIAS', :alias);" );
  query.prepare( sql );
  query.bindValue( QStringLiteral( ":alias" ), alias );
  if ( !query.exec() )
  {
    error.append( QObject::tr( "Could not save alias to database: %1" ).arg( query.lastError().text() ) );
  }
  db.close();
  return error;
}

const QIcon QgsUserProfile::icon() const
{
  QString path = mProfileFolder + QDir::separator() + "icon.svg";
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
