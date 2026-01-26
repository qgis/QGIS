/***************************************************************************
  qgsversionmigration.cpp - QgsVersionMigration

 ---------------------
 begin                : 30.7.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsversionmigration.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsreadwritecontext.h"
#include "qgssettings.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsuserprofilemanager.h"

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <qsettings.h>

std::unique_ptr<QgsVersionMigration> QgsVersionMigration::canMigrate( int fromVersion, int toVersion )
{
  if ( fromVersion == 20000 && toVersion >= 29900 )
  {
    return std::make_unique<Qgs2To3Migration>();
  }
  return nullptr;
}

QgsError Qgs2To3Migration::runMigration()
{
  QgsError errors;
  const QgsError settingsErrors = migrateSettings();
  if ( !settingsErrors.isEmpty() )
  {
    const QList<QgsErrorMessage> errorList( settingsErrors.messageList() );
    for ( const auto &err : errorList )
    {
      errors.append( err );
    }
  }
  const QgsError stylesErrors = migrateStyles();
  if ( !stylesErrors.isEmpty() )
  {
    const QList<QgsErrorMessage> errorList( stylesErrors.messageList() );
    for ( const auto &err : errorList )
    {
      errors.append( err );
    }
  }
  const QgsError authDbErrors = migrateAuthDb();
  if ( !authDbErrors.isEmpty() )
  {
    const QList<QgsErrorMessage> errorList( authDbErrors.messageList() );
    for ( const auto &err : errorList )
    {
      errors.append( err );
    }
  }
  return errors;
}

bool Qgs2To3Migration::requiresMigration()
{
  const QgsSettings settings;
  const bool alreadyMigrated = settings.value( u"migration/settings"_s, false ).toBool();
  const int settingsMigrationVersion = settings.value( u"migration/fileVersion"_s, 0 ).toInt();
  QFile migrationFile( migrationFilePath() );
  if ( migrationFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QTextStream in( &migrationFile );
    const QString line = in.readLine();
    if ( line.startsWith( "#" ) && line.contains( u"version="_s ) )
    {
      const QStringList parts = line.split( '=' );
      mMigrationFileVersion = parts.at( 1 ).toInt();
      QgsDebugMsgLevel( u"File version is=%1"_s.arg( mMigrationFileVersion ), 2 );
    }
    migrationFile.close();
  }
  else
  {
    QgsDebugError( u"Can not open %1"_s.arg( migrationFile.fileName() ) );
    mMigrationFileVersion = settingsMigrationVersion;
  }

  return ( !alreadyMigrated || settingsMigrationVersion != mMigrationFileVersion );
}

QgsError Qgs2To3Migration::migrateStyles()
{
  QgsError error;
  const QString oldHome = u"%1/.qgis2"_s.arg( QDir::homePath() );
  const QString oldStyleFile = u"%1/symbology-ng-style.db"_s.arg( oldHome );
  QgsDebugMsgLevel( u"OLD STYLE FILE %1"_s.arg( oldStyleFile ), 2 );
  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "migration" );
  db.setDatabaseName( oldStyleFile );
  if ( !db.open() )
  {
    error.append( db.lastError().text() );
    QgsDebugError( db.lastError().text() );
    return error;
  }

  QSqlQuery query( db );
  QSqlQuery tagQuery( "SELECT name FROM tag"
                      "JOIN tagmap ON tagmap.tag_id = tag.id"
                      "WHERE tagmap.symbol_id = :symbol_id",
                      db );

  QgsStyle *style = QgsStyle::defaultStyle();
  if ( query.exec( "SELECT id, name, xml FROM symbol" ) )
  {
    while ( query.next() )
    {
      const QString symbol_id = query.value( 0 ).toString();
      const QString name = query.value( 1 ).toString();
      const QString xml = query.value( 2 ).toString();
      QDomDocument doc;
      if ( !doc.setContent( xml ) )
      {
        QgsDebugError( "Cannot open symbol " + name );
        continue;
      }

      tagQuery.bindValue( ":symbol_id", symbol_id );

      QStringList tags;
      if ( tagQuery.exec() )
      {
        while ( query.next() )
        {
          const QString tagname = query.value( 0 ).toString();
          tags << tagname;
        }
      }

      const QDomElement symElement = doc.documentElement();
      QgsDebugMsgLevel( u"MIGRATION: Importing %1"_s.arg( name ), 2 );
      std::unique_ptr< QgsSymbol > symbol = QgsSymbolLayerUtils::loadSymbol( symElement, QgsReadWriteContext() );
      tags << "QGIS 2";
      if ( style->symbolId( name ) == 0 )
      {
        style->saveSymbol( name, symbol.get(), false, tags );
      }
    }
  }

  QgsDebugMsgLevel( oldStyleFile, 2 );
  return error;
}

QgsError Qgs2To3Migration::migrateSettings()
{
  QgsError error;

  QgsSettings newSettings;

  // The platform default location for the settings from 2.x
  mOldSettings = new QSettings( "QGIS", "QGIS2" );

  QFile inputFile( migrationFilePath() );
  std::map<QString, QgsSettings::Section> sections;
  sections["none"] = QgsSettings::NoSection;
  sections["core"] = QgsSettings::Core;
  sections["gui"] = QgsSettings::Gui;
  sections["server"] = QgsSettings::Server;
  sections["plugins"] = QgsSettings::Plugins;
  sections["auth"] = QgsSettings::Auth;
  sections["app"] = QgsSettings::App;
  sections["providers"] = QgsSettings::Providers;
  sections["misc"] = QgsSettings::Misc;

  QList<QPair<QString, QString>> keys;

  if ( inputFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QTextStream in( &inputFile );
    while ( !in.atEnd() )
    {
      const QString line = in.readLine();

      if ( line.startsWith( "#" ) )
        continue;

      if ( line.isEmpty() )
        continue;

      const QStringList parts = line.split( ';' );

      Q_ASSERT_X( parts.count() == 2, "QgsVersionMigration::migrateSettings()", "Can't split line in 2 parts." );

      QString oldKey = parts.at( 0 );
      const QString newKey = parts.at( 1 );

      if ( oldKey.endsWith( "/*" ) )
      {
        oldKey = oldKey.replace( "/*", "" );
        const QList<QPair<QString, QString>> keyList = walk( oldKey, newKey );
        keys.append( keyList );
      }
      else
      {
        const QPair<QString, QString> key = transformKey( oldKey, newKey );
        keys.append( key );
      }
    }
    inputFile.close();
    newSettings.setValue( u"migration/settings"_s, true );
    // Set the dev gen so we can force a migration.
    newSettings.setValue( u"migration/fileVersion"_s, mMigrationFileVersion );
  }
  else
  {
    const QString msg = QString( "Can not open %1" ).arg( inputFile.fileName() );
    QgsDebugError( msg );
    error.append( msg );
  }

  if ( keys.count() > 0 )
  {
    QgsDebugMsgLevel( u"MIGRATION: Translating settings keys"_s, 2 );
    QList<QPair<QString, QString>>::iterator i;
    for ( i = keys.begin(); i != keys.end(); ++i )
    {
      const QPair<QString, QString> pair = *i;

      const QString oldKey = pair.first;
      const QString newKey = pair.second;

      if ( oldKey.contains( oldKey ) )
      {
        QgsDebugMsgLevel( u" -> %1 -> %2"_s.arg( oldKey, newKey ), 2 );
        newSettings.setValue( newKey, mOldSettings->value( oldKey ) );
      }
    }
  }
  return error;
}

QgsError Qgs2To3Migration::migrateAuthDb()
{
  QgsError error;
  const QString oldHome = u"%1/.qgis2"_s.arg( QDir::homePath() );
  const QString oldAuthDbFilePath = u"%1/qgis-auth.db"_s.arg( oldHome );
  // Try to retrieve the current profile folder (I didn't find an QgsApplication API for it)
  QDir settingsDir = QFileInfo( QgsSettings().fileName() ).absoluteDir();
  settingsDir.cdUp();
  const QString newAuthDbFilePath = u"%1/qgis-auth.db"_s.arg( settingsDir.absolutePath() );
  // Do not overwrite!
  if ( QFile( newAuthDbFilePath ).exists() )
  {
    const QString msg = u"Could not copy old auth DB to %1: file already exists!"_s.arg( newAuthDbFilePath );
    QgsDebugError( msg );
    error.append( msg );
  }
  else
  {
    QFile oldDbFile( oldAuthDbFilePath );
    if ( oldDbFile.exists() )
    {
      if ( oldDbFile.copy( newAuthDbFilePath ) )
      {
        QgsDebugMsgLevel( u"Old auth DB successfully copied to %1"_s.arg( newAuthDbFilePath ), 2 );
      }
      else
      {
        const QString msg = u"Could not copy auth DB %1 to %2"_s.arg( oldAuthDbFilePath, newAuthDbFilePath );
        QgsDebugError( msg );
        error.append( msg );
      }
    }
    else
    {
      const QString msg = u"Could not copy auth DB %1 to %2: old DB does not exists!"_s.arg( oldAuthDbFilePath, newAuthDbFilePath );
      QgsDebugError( msg );
      error.append( msg );
    }
  }
  return error;
}

QList<QPair<QString, QString>> Qgs2To3Migration::walk( QString group, QString newkey )
{
  mOldSettings->beginGroup( group );
  QList<QPair<QString, QString>> foundKeys;
  const auto constChildGroups = mOldSettings->childGroups();
  for ( const QString &group : constChildGroups )
  {
    const QList<QPair<QString, QString>> data = walk( group, newkey );
    foundKeys.append( data );
  }

  const auto constChildKeys = mOldSettings->childKeys();
  for ( const QString &key : constChildKeys )
  {
    const QString fullKey = mOldSettings->group() + "/" + key;
    foundKeys.append( transformKey( fullKey, newkey ) );
  }
  mOldSettings->endGroup();
  return foundKeys;
}

QPair<QString, QString> Qgs2To3Migration::transformKey( QString fullOldKey, QString newKeyPart )
{
  QString newKey = newKeyPart;
  const QString oldKey = fullOldKey;

  if ( newKeyPart == "*"_L1 )
  {
    newKey = fullOldKey;
  }

  if ( newKeyPart.endsWith( "/*" ) )
  {
    QStringList newKeyparts = newKeyPart.split( '/' );
    // Throw away the *
    newKeyparts.removeLast();
    QStringList oldKeyParts = fullOldKey.split( '/' );
    for ( int i = 0; i < newKeyparts.count(); ++i )
    {
      oldKeyParts.replace( i, newKeyparts.at( i ) );
    }
    newKey = oldKeyParts.join( "/" );
  }

  return qMakePair( oldKey, newKey );
}

QString Qgs2To3Migration::migrationFilePath()
{
  return QgsApplication::resolvePkgPath() + "/resources/2to3migration.txt";
}
