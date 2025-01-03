/***************************************************************************
  qgsauthconfigurationstoragesqlite.cpp - QgsAuthConfigurationStorageSqlite

 ---------------------
 begin                : 20.6.2024
 copyright            : (C) 2024 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsauthconfigurationstoragesqlite.h"
#include "moc_qgsauthconfigurationstoragesqlite.cpp"
#include "qgslogger.h"
#include "qgsauthcertutils.h"

#include <QFileInfo>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QCoreApplication>

///@cond PRIVATE

QgsAuthConfigurationStorageSqlite::QgsAuthConfigurationStorageSqlite( const QString &databasePath )
  : QgsAuthConfigurationStorageDb( {{ QStringLiteral( "driver" ), QStringLiteral( "QSQLITE" ) }, { QStringLiteral( "database" ), databasePath }} )
{
}

bool QgsAuthConfigurationStorageSqlite::initialize()
{
  QMutexLocker locker( &mMutex );

  if ( !QFileInfo::exists( mDatabase ) )
  {
    // Check if the parent path exists
    QFileInfo parentInfo( QFileInfo( mDatabase ).path() );
    if ( ! parentInfo.exists() )
    {
      // Try to create the directory
      QDir dir;
      if ( !dir.mkpath( parentInfo.absolutePath() ) )
      {
        setError( tr( "Auth db directory path '%1' could not be created" ).arg( mDatabase ) );
        return false;
      }
    }

    // Try to create the database
    QSqlDatabase db = authDatabaseConnection();
    if ( !db.open() )
    {
      setError( tr( "Auth db file '%1' could not be created" ).arg( mDatabase ) );
      return false;
    }
  }

  // Check if the file is readable
  const QFileInfo fileInfo( mDatabase );
  if ( !fileInfo.permission( QFile::ReadOwner ) )
  {
    setError( tr( "Auth db file '%1' is not readable" ).arg( mDatabase ) );
    return false;
  }

  // Check if the file is writable
  if ( !fileInfo.permission( QFile::WriteOwner ) )
  {
    setError( tr( "Auth db file '%1' is not writable" ).arg( mDatabase ), Qgis::MessageLevel::Warning );
  }

  const bool ok { createConfigTables() &&createCertTables() };
  if ( !ok )
  {
    setError( tr( "Auth db initialization FAILED" ), Qgis::MessageLevel::Critical );
    mIsReady = false;
    return false;
  }

  mIsReady = true;

  checkCapabilities();

  // Recompute capabilities if needed
  connect( this, &QgsAuthConfigurationStorageDb::readOnlyChanged, this, [this]( bool )
  {
    checkCapabilities();
  } );

  return true;
}

QList<QgsAuthConfigurationStorage::SettingParameter> QgsAuthConfigurationStorageSqlite::settingsParameters() const
{
  return {{ QStringLiteral( "database" ), tr( "Path to the SQLite database file" ), QVariant::String }};
}

QString QgsAuthConfigurationStorageSqlite::description() const
{
  return tr( "Store credentials in a local SQLite database" );
}

QString QgsAuthConfigurationStorageSqlite::type() const
{
  return QStringLiteral( "SQLITE" );
}

bool QgsAuthConfigurationStorageSqlite::tableExists( const QString &table ) const
{
  QMutexLocker locker( &mMutex );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageSqlite * >( this )->setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );
  query.prepare( QStringLiteral( "SELECT name FROM sqlite_master WHERE type='table' AND name=:name" ) );
  query.bindValue( QStringLiteral( ":name" ), table );

  if ( !authDbQuery( &query ) )
  {
    const_cast< QgsAuthConfigurationStorageSqlite * >( this )->setError( tr( "Failed to check if table '%1' exists" ).arg( table ) );
    return false;
  }

  if ( ! query.next() )
  {
    return false;
  }

  return true;
}

void QgsAuthConfigurationStorageSqlite::checkCapabilities()
{

  QMutexLocker locker( &mMutex );
  QFileInfo fileInfo( mDatabase );
  if ( ! fileInfo.exists() )
  {
    mCapabilities = Qgis::AuthConfigurationStorageCapabilities();
    return;
  }

  const bool readOnly { isReadOnly() };

  mIsReadOnly = mIsReadOnly && fileInfo.isWritable();
  QgsAuthConfigurationStorageDb::checkCapabilities();

  if ( ! fileInfo.isReadable() )
  {
    mCapabilities.setFlag( Qgis::AuthConfigurationStorageCapability::ReadConfiguration, false );
    mCapabilities.setFlag( Qgis::AuthConfigurationStorageCapability::ReadMasterPassword, false );
    mCapabilities.setFlag( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority, false );
    mCapabilities.setFlag( Qgis::AuthConfigurationStorageCapability::ReadCertificateTrustPolicy, false );
    mCapabilities.setFlag( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity, false );
    mCapabilities.setFlag( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig, false );
  }

  // We need to emit the signal without repeating the check
  if ( mIsReadOnly != readOnly )
  {
    mIsReadOnly = readOnly;
    whileBlocking( this )->setReadOnly( !readOnly );
  }

}

/// @endcond
