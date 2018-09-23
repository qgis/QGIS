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
    return qgis::make_unique< Qgs2To3Migration >();
  }
  return nullptr;
}

QgsError Qgs2To3Migration::runMigration()
{
  QgsError errors;
  QgsError settingsErrors = migrateSettings();
  if ( !settingsErrors.isEmpty() )
  {
    const QList<QgsErrorMessage> errorList( settingsErrors.messageList( ) );
    for ( const auto &err : errorList )
    {
      errors.append( err );
    }
  }
  QgsError stylesErrors = migrateStyles();
  if ( !stylesErrors.isEmpty() )
  {
    const QList<QgsErrorMessage> errorList( stylesErrors.messageList( ) );
    for ( const auto &err : errorList )
    {
      errors.append( err );
    }
  }
  QgsError authDbErrors = migrateAuthDb();
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
  QgsSettings settings;
  bool alreadyMigrated = settings.value( QStringLiteral( "migration/settings" ), false ).toBool();
  int  settingsMigrationVersion = settings.value( QStringLiteral( "migration/fileVersion" ), 0 ).toInt();
  QFile migrationFile( migrationFilePath() );
  if ( migrationFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QTextStream in( &migrationFile );
    QString line = in.readLine();
    if ( line.startsWith( "#" ) && line.contains( QStringLiteral( "version=" ) ) )
    {
      QStringList parts = line.split( '=' );
      mMigrationFileVersion = parts.at( 1 ).toInt();
      QgsDebugMsg( QString( "File version is=%1" ).arg( mMigrationFileVersion ) );
    }
    migrationFile.close();
  }
  else
  {
    QgsDebugMsg( QString( "Can not open %1" ).arg( migrationFile.fileName() ) );
    mMigrationFileVersion = settingsMigrationVersion;
  }

  return ( !alreadyMigrated || settingsMigrationVersion != mMigrationFileVersion );
}

QgsError Qgs2To3Migration::migrateStyles()
{
  QgsError error;
  QString oldHome = QStringLiteral( "%1/.qgis2" ).arg( QDir::homePath() );
  QString oldStyleFile = QStringLiteral( "%1/symbology-ng-style.db" ).arg( oldHome );
  QgsDebugMsg( QString( "OLD STYLE FILE %1" ).arg( oldStyleFile ) );
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
      QString symbol_id = query.value( 0 ).toString();
      QString name = query.value( 1 ).toString();
      QString xml = query.value( 2 ).toString();
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
          QString tagname = query.value( 0 ).toString();
          tags << tagname;
        }
      }

      QDomElement symElement = doc.documentElement();
      QgsDebugMsg( QString( "MIGRATION: Importing %1" ).arg( name ) );
      QgsSymbol *symbol = QgsSymbolLayerUtils::loadSymbol( symElement, QgsReadWriteContext() );
      tags << "QGIS 2";
      if ( style->symbolId( name ) == 0 )
      {
        style->saveSymbol( name, symbol, false, tags );
      }
    }
  }

  QgsDebugMsg( oldStyleFile );
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
      QString line = in.readLine();

      if ( line.startsWith( "#" ) )
        continue;

      if ( line.isEmpty() )
        continue;

      const QStringList parts = line.split( ';' );

      Q_ASSERT_X( parts.count() == 2, "QgsVersionMigration::migrateSettings()", "Can't split line in 2 parts." );

      QString oldKey = parts.at( 0 );
      QString newKey = parts.at( 1 );

      if ( oldKey.endsWith( "/*" ) )
      {
        oldKey = oldKey.replace( "/*", "" );
        QList<QPair<QString, QString>> keyList = walk( oldKey, newKey );
        keys.append( keyList );
      }
      else
      {
        QPair<QString, QString> key = transformKey( oldKey, newKey );
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
    QString msg = QString( "Can not open %1" ).arg( inputFile.fileName() );
    QgsDebugMsg( msg );
    error.append( msg );
  }

  if ( keys.count() > 0 )
  {
    QgsDebugMsg( "MIGRATION: Translating settings keys" );
    QList<QPair<QString, QString>>::iterator i;
    for ( i = keys.begin(); i != keys.end(); ++i )
    {
      QPair<QString, QString> pair = *i;

      QString oldKey = pair.first;
      QString newKey = pair.second;

      if ( oldKey.contains( oldKey ) )
      {
        QgsDebugMsg( QString( " -> %1 -> %2" ).arg( oldKey, newKey ) );
        newSettings.setValue( newKey, mOldSettings->value( oldKey ) );
      }
    }
  }
  return error;
}

QgsError Qgs2To3Migration::migrateAuthDb()
{
  QgsError error;
  QString oldHome = QStringLiteral( "%1/.qgis2" ).arg( QDir::homePath() );
  QString oldAuthDbFilePath = QStringLiteral( "%1/qgis-auth.db" ).arg( oldHome );
  // Try to retrieve the current profile folder (I didn't find an QgsApplication API for it)
  QDir settingsDir = QFileInfo( QgsSettings().fileName() ).absoluteDir();
  settingsDir.cdUp();
  QString newAuthDbFilePath = QStringLiteral( "%1/qgis-auth.db" ).arg( settingsDir.absolutePath() );
  // Do not overwrite!
  if ( QFile( newAuthDbFilePath ).exists( ) )
  {
    QString msg = QStringLiteral( "Could not copy old auth DB to %1: file already exists!" ).arg( newAuthDbFilePath );
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
        QgsDebugMsg( QStringLiteral( "Old auth DB successfully copied to %1" ).arg( newAuthDbFilePath ) );
      }
      else
      {
        QString msg = QStringLiteral( "Could not copy auth DB %1 to %2" ).arg( oldAuthDbFilePath, newAuthDbFilePath );
        QgsDebugMsg( msg );
        error.append( msg );
      }
    }
    else
    {
      QString msg = QStringLiteral( "Could not copy auth DB %1 to %2: old DB does not exists!" ).arg( oldAuthDbFilePath, newAuthDbFilePath );
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
  Q_FOREACH ( const QString &group, mOldSettings->childGroups() )
  {
    QList<QPair<QString, QString> > data = walk( group, newkey );
    foundKeys.append( data );
  }

  Q_FOREACH ( const QString &key, mOldSettings->childKeys() )
  {
    QString fullKey = mOldSettings->group() + "/" + key;
    foundKeys.append( transformKey( fullKey, newkey ) );
  }
  mOldSettings->endGroup();
  return foundKeys;
}

QPair<QString, QString> Qgs2To3Migration::transformKey( QString fullOldKey, QString newKeyPart )
{
  QString newKey = newKeyPart;
  QString oldKey = fullOldKey;

  if ( newKeyPart == QStringLiteral( "*" ) )
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
