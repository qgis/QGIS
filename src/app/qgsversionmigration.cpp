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
#include "qgssettings.h"
#include "qgslogger.h"
#include "qsettings.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include "qgssymbol.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgsreadwritecontext.h"
#include "qgsuserprofilemanager.h"

#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDomDocument>

std::unique_ptr<QgsVersionMigration> QgsVersionMigration::canMigrate( int fromVersion, int toVersion )
{
  if ( fromVersion == 20000 && toVersion >= 29900 )
  {
    return std::make_unique< Qgs2To3Migration >();
  }
  return nullptr;
}

QgsError Qgs2To3Migration::runMigration()
{
  QgsError errors;
  const QgsError settingsErrors = migrateSettings();
  if ( !settingsErrors.isEmpty() )
  {
    const QList<QgsErrorMessage> errorList( settingsErrors.messageList( ) );
    for ( const auto &err : errorList )
    {
      errors.append( err );
    }
  }
  const QgsError stylesErrors = migrateStyles();
  if ( !stylesErrors.isEmpty() )
  {
    const QList<QgsErrorMessage> errorList( stylesErrors.messageList( ) );
    for ( const auto &err : errorList )
    {
      errors.append( err );
    }
  }
  const QgsError authDbErrors = migrateAuthDb();
  if ( !authDbErrors.isEmpty() )
  {
    const QList<QgsErrorMessage> errorList( authDbErrors.messageList( ) );
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
  const bool alreadyMigrated = settings.value( QStringLiteral( "migration/settings" ), false ).toBool();
  const int  settingsMigrationVersion = settings.value( QStringLiteral( "migration/fileVersion" ), 0 ).toInt();
  QFile migrationFile( migrationFilePath() );
  if ( migrationFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QTextStream in( &migrationFile );
    const QString line = in.readLine();
    if ( line.startsWith( "#" ) && line.contains( QStringLiteral( "version=" ) ) )
    {
      const QStringList parts = line.split( '=' );
      mMigrationFileVersion = parts.at( 1 ).toInt();
      QgsDebugMsgLevel( QStringLiteral( "File version is=%1" ).arg( mMigrationFileVersion ), 2 );
    }
    migrationFile.close();
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Can not open %1" ).arg( migrationFile.fileName() ) );
    mMigrationFileVersion = settingsMigrationVersion;
  }

  return ( !alreadyMigrated || settingsMigrationVersion != mMigrationFileVersion );
}

QgsError Qgs2To3Migration::migrateStyles()
{
  QgsError error;
  const QString oldHome = QStringLiteral( "%1/.qgis2" ).arg( QDir::homePath() );
  const QString oldStyleFile = QStringLiteral( "%1/symbology-ng-style.db" ).arg( oldHome );
  QgsDebugMsgLevel( QStringLiteral( "OLD STYLE FILE %1" ).arg( oldStyleFile ), 2 );
  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "migration" );
  db.setDatabaseName( oldStyleFile );
  if ( !db.open() )
  {
    error.append( db.lastError().text() );
    QgsDebugMsg( db.lastError().text() );
    return error;
  }

  QSqlQuery query( db );
  QSqlQuery tagQuery( "SELECT name FROM tag"
                      "JOIN tagmap ON tagmap.tag_id = tag.id"
                      "WHERE tagmap.symbol_id = :symbol_id", db );

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
        QgsDebugMsg( "Cannot open symbol " + name );
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
      QgsDebugMsgLevel( QStringLiteral( "MIGRATION: Importing %1" ).arg( name ), 2 );
      QgsSymbol *symbol = QgsSymbolLayerUtils::loadSymbol( symElement, QgsReadWriteContext() );
      tags << "QGIS 2";
      if ( style->symbolId( name ) == 0 )
      {
        style->saveSymbol( name, symbol, false, tags );
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
    newSettings.setValue( QStringLiteral( "migration/settings" ), true );
    // Set the dev gen so we can force a migration.
    newSettings.setValue( QStringLiteral( "migration/fileVersion" ), mMigrationFileVersion );
  }
  else
  {
    const QString msg = QString( "Can not open %1" ).arg( inputFile.fileName() );
    QgsDebugMsg( msg );
    error.append( msg );
  }

  if ( keys.count() > 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "MIGRATION: Translating settings keys" ), 2 );
    QList<QPair<QString, QString>>::iterator i;
    for ( i = keys.begin(); i != keys.end(); ++i )
    {
      const QPair<QString, QString> pair = *i;

      const QString oldKey = pair.first;
      const QString newKey = pair.second;

      if ( oldKey.contains( oldKey ) )
      {
        QgsDebugMsgLevel( QStringLiteral( " -> %1 -> %2" ).arg( oldKey, newKey ), 2 );
        newSettings.setValue( newKey, mOldSettings->value( oldKey ) );
      }
    }
  }
  return error;
}

QgsError Qgs2To3Migration::migrateAuthDb()
{
  QgsError error;
  const QString oldHome = QStringLiteral( "%1/.qgis2" ).arg( QDir::homePath() );
  const QString oldAuthDbFilePath = QStringLiteral( "%1/qgis-auth.db" ).arg( oldHome );
  // Try to retrieve the current profile folder (I didn't find an QgsApplication API for it)
  QDir settingsDir = QFileInfo( QgsSettings().fileName() ).absoluteDir();
  settingsDir.cdUp();
  const QString newAuthDbFilePath = QStringLiteral( "%1/qgis-auth.db" ).arg( settingsDir.absolutePath() );
  // Do not overwrite!
  if ( QFile( newAuthDbFilePath ).exists( ) )
  {
    const QString msg = QStringLiteral( "Could not copy old auth DB to %1: file already exists!" ).arg( newAuthDbFilePath );
    QgsDebugMsg( msg );
    error.append( msg );
  }
  else
  {
    QFile oldDbFile( oldAuthDbFilePath );
    if ( oldDbFile.exists( ) )
    {
      if ( oldDbFile.copy( newAuthDbFilePath ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "Old auth DB successfully copied to %1" ).arg( newAuthDbFilePath ), 2 );
      }
      else
      {
        const QString msg = QStringLiteral( "Could not copy auth DB %1 to %2" ).arg( oldAuthDbFilePath, newAuthDbFilePath );
        QgsDebugMsg( msg );
        error.append( msg );
      }
    }
    else
    {
      const QString msg = QStringLiteral( "Could not copy auth DB %1 to %2: old DB does not exists!" ).arg( oldAuthDbFilePath, newAuthDbFilePath );
      QgsDebugMsg( msg );
      error.append( msg );
    }
  }
  return error;
}

QList<QPair<QString, QString> > Qgs2To3Migration::walk( QString group, QString newkey )
{
  mOldSettings->beginGroup( group );
  QList<QPair<QString, QString> > foundKeys;
  const auto constChildGroups = mOldSettings->childGroups();
  for ( const QString &group : constChildGroups )
  {
    const QList<QPair<QString, QString> > data = walk( group, newkey );
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

  if ( newKeyPart == QLatin1String( "*" ) )
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
  return QgsApplication::resolvePkgPath() +  "/resources/2to3migration.txt";
}
