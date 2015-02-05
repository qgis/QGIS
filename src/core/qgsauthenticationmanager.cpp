/***************************************************************************
    qgsauthenticationmanager.cpp
    ---------------------
    begin                : October 5, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthenticationmanager.h"

#include <QEventLoop>
#include <QFileInfo>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QVariant>

#include <QtCrypto>

#include "qgsapplication.h"
#include "qgsauthenticationcrypto.h"
#include "qgsauthenticationprovider.h"
#include "qgscredentials.h"
#include "qgslogger.h"


const QString QgsAuthManager::smAuthConfigTable = "auth_configs";
const QString QgsAuthManager::smAuthPassTable = "auth_pass";
const QString QgsAuthManager::smAuthManTag = QObject::tr( "Authentication Manager" );

QSqlDatabase QgsAuthManager::authDbConnection() const
{
  QSqlDatabase authdb;
  if ( isDisabled() )
    return authdb;

  QString connectionname = "authentication.configs";
  if ( !QSqlDatabase::contains( connectionname ) )
  {
    authdb = QSqlDatabase::addDatabase( "QSQLITE", connectionname );
    authdb.setDatabaseName( authenticationDbPath() );
  }
  else
  {
    authdb = QSqlDatabase::database( connectionname );
  }
  if ( !authdb.isOpen() )
    authdb.open();

  return authdb;
}

bool QgsAuthManager::init()
{
  QgsDebugMsg( "Initializing QCA..." );
  mQcaInitializer = new QCA::Initializer( QCA::Practical, 256 );

  QgsDebugMsg( "QCA initialized." );
  QCA::scanForPlugins();

  QgsDebugMsg( QString( "QCA Plugin Diagnostics Context: %1" ).arg( QCA::pluginDiagnosticText() ) );
  QStringList capabilities;

  capabilities = QCA::supportedFeatures();
  QgsDebugMsg( QString( "QCA supports: %1" ).arg( capabilities.join( "," ) ) );

  // do run-time check for qca-ossl plugin
  if ( !QCA::isSupported( "cert", "qca-ossl" ) )
  {
    mAuthDisabled = true;
    return isDisabled();
  }

  QgsDebugMsg( "Prioritizing qca-ossl over all other QCA providers..." );
  QCA::ProviderList provds = QCA::providers();
  QStringList prlist;
  foreach ( QCA::Provider* p, provds )
  {
    QString pn = p->name();
    int pr = 0;
    if ( pn != QString( "qca-ossl" ) )
    {
      pr = QCA::providerPriority( pn ) + 1;
    }
    QCA::setProviderPriority( pn, pr );
    prlist << QString( "%1:%2" ).arg( pn ).arg( QCA::providerPriority( pn ) );
  }
  QgsDebugMsg( QString( "QCA provider priorities: %1" ).arg( prlist.join( ", " ) ) );

  registerProviders();

  mAuthDbPath = QgsApplication::qgisAuthDbFilePath();
  QgsDebugMsg( QString( "Authorization database path: %1" ).arg( authenticationDbPath() ) );

  QFileInfo dbinfo( authenticationDbPath() );
  if ( dbinfo.exists() )
  {
    if ( !dbinfo.permission( QFile::ReadOwner | QFile::WriteOwner ) )
    {
      const char* err = QT_TR_NOOP( "Auth db is not readable or writable by user" );
      QgsDebugMsg( err );
      emit messageOut( tr( err ), authManTag(), CRITICAL );
      return false;
    }
    if ( dbinfo.size() > 0 )
    {
      QgsDebugMsg( "Auth db exists and has data" );
      updateConfigProviderTypes();
      return true;
    }
  }

  // create and open the db
  if ( !authDbOpen() )
  {
    const char* err = QT_TR_NOOP( "Auth db could not be created and opened" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), CRITICAL );
    return false;
  }

  QSqlQuery query( authDbConnection() );

  // create the tables
  QString qstr;

  qstr = QString( "CREATE TABLE %1 (\n"
                  "    'salt' TEXT NOT NULL,\n"
                  "    'civ' TEXT NOT NULL\n"
                  ", 'hash' TEXT  NOT NULL);" ).arg( authDbPassTable() );
  query.prepare( qstr );
  if ( !authDbQuery( &query ) )
    return false;
  query.clear();

  qstr = QString( "CREATE TABLE %1 (\n"
                  "    'id' TEXT NOT NULL,\n"
                  "    'name' TEXT NOT NULL,\n"
                  "    'uri' TEXT,\n"
                  "    'type' TEXT NOT NULL,\n"
                  "    'version' INTEGER NOT NULL\n"
                  ", 'config' TEXT  NOT NULL);" ).arg( authDbConfigTable() );
  query.prepare( qstr );
  if ( !authDbQuery( &query ) )
    return false;
  query.clear();

  qstr = QString( "CREATE UNIQUE INDEX 'id_index' on %1 (id ASC);" ).arg( authDbConfigTable() );
  query.prepare( qstr );
  if ( !authDbQuery( &query ) )
    return false;
  query.clear();

  qstr = QString( "CREATE INDEX 'uri_index' on %1 (uri ASC);" ).arg( authDbConfigTable() );
  query.prepare( qstr );
  if ( !authDbQuery( &query ) )
    return false;
  query.clear();

  return true;
}

bool QgsAuthManager::isDisabled() const
{
  if ( mAuthDisabled )
  {
    QgsDebugMsg( "Authentication system DISABLED: QCA's qca-ossl (OpenSSL) plugin is missing" );
  }
  return mAuthDisabled;
}

const QString QgsAuthManager::disabledMessage() const
{
  return tr( "Authentication system is DISABLED:\n"
             "QCA's OpenSSL plugin (qca-ossl) is missing" );
}

bool QgsAuthManager::setMasterPassword( bool verify )
{
  if ( isDisabled() )
    return false;

  if ( mMasterPass.isEmpty() )
  {
    QgsDebugMsg( "Master password is not yet set by user" );
    if ( !masterPasswordInput() )
    {
      QgsDebugMsg( "Master password input canceled by user" );
      return false;
    }
  }
  else
  {
    QgsDebugMsg( "Master password is set" );
    if ( !verify )
      return true;
  }

  if ( !verifyMasterPassword() )
    return false;

  QgsDebugMsg( "Master password is set and verified" );
  return true;
}

bool QgsAuthManager::setMasterPassword( const QString& pass, bool verify )
{
  if ( isDisabled() )
    return false;

  // since this is generally for automation, we don't care if passed-in is same as existing
  QString prevpass = QString( mMasterPass );
  mMasterPass = pass;
  if ( verify && !verifyMasterPassword() )
  {
    mMasterPass = prevpass;
    const char* err = QT_TR_NOOP( "Master password set: FAILED to verify, reset to previous" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }

  QgsDebugMsg( QString( "Master password set: SUCCESS%1" ).arg( verify ? " and verified" : "" ) );
  return true;
}

bool QgsAuthManager::verifyMasterPassword()
{
  if ( isDisabled() )
    return false;

  int rows = 0;
  if ( !masterPasswordRowsInDb( &rows ) )
  {
    const char* err = QT_TR_NOOP( "Master password: FAILED to access database" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), CRITICAL );

    clearMasterPassword();
    return false;
  }

  QgsDebugMsg( QString( "Master password: %1 rows in database" ).arg( rows ) );

  if ( rows > 1 )
  {
    const char* err = QT_TR_NOOP( "Master password: FAILED to find just one master password record in database" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );

    clearMasterPassword();
    return false;
  }
  else if ( rows == 1 )
  {
    if ( !masterPasswordCheckAgainstDb() )
    {
      const char* err = QT_TR_NOOP( "Master password: FAILED to verify against hash in database" );
      QgsDebugMsg( err );
      emit messageOut( tr( err ), authManTag(), WARNING );

      clearMasterPassword();
      emit masterPasswordVerified( false );
      return false;
    }
    else
    {
      QgsDebugMsg( "Master password: verified against hash in database" );
      emit masterPasswordVerified( true );
    }
  }
  else
  {
    if ( !masterPasswordStoreInDb() )
    {
      const char* err = QT_TR_NOOP( "Master password: hash FAILED to be stored in database" );
      QgsDebugMsg( err );
      emit messageOut( tr( err ), authManTag(), CRITICAL );

      clearMasterPassword();
      return false;
    }
    else
    {
      QgsDebugMsg( "Master password: hash stored in database" );
    }
    // double-check storing
    if ( !masterPasswordCheckAgainstDb() )
    {
      const char* err = QT_TR_NOOP( "Master password: FAILED to verify against hash in database" );
      QgsDebugMsg( err );
      emit messageOut( tr( err ), authManTag(), WARNING );

      clearMasterPassword();
      emit masterPasswordVerified( false );
      return false;
    }
    else
    {
      QgsDebugMsg( "Master password: verified against hash in database" );
      emit masterPasswordVerified( true );
    }
  }

  return true;
}

bool QgsAuthManager::masterPasswordIsSet() const
{
  return !mMasterPass.isEmpty();
}

bool QgsAuthManager::masterPasswordSame( const QString &pass ) const
{
  return mMasterPass == pass;
}

bool QgsAuthManager::resetMasterPassword( const QString& newpass, const QString &oldpass,
                                          bool keepbackup, QString *backuppath )
{
  if ( isDisabled() )
    return false;

  // verify caller knows the current master password
  // this means that the user will have had to already set the master password as well
  if ( !masterPasswordSame( oldpass ) )
    return false;

  // close any connection to current db
  authDbConnection().close();

  // duplicate current db file to 'qgis-auth_YYYY-MM-DD-HHMMSS.db' backup
  QString datestamp( QDateTime::currentDateTime().toString( "yyyy-MM-dd-hhmmss" ) );
  QString dbbackup( authenticationDbPath() );
  dbbackup.replace( QString( ".db" ), QString( "_%1.db" ).arg( datestamp ) );

  if ( !QFile::copy( authenticationDbPath(), dbbackup ) )
  {
    const char* err = QT_TR_NOOP( "Master password reset FAILED: could not backup current database" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }
  QgsDebugMsg( "Master password reset: backed up current database" );

  // create new database and connection
  authDbConnection();

  // store current password and civ
  QString prevpass = QString( mMasterPass );
  QString prevciv = QString( masterPasswordCiv() );

  // on ANY FAILURE from this point, reinstate previous password and database
  bool ok = true;

  // clear password hash table (also clears mMasterPass)
  if ( ok && !masterPasswordClearDb() )
  {
    ok = false;
    const char* err = QT_TR_NOOP( "Master password reset FAILED: could not clear current password from database" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
  }
  if ( ok )
    QgsDebugMsg( "Master password reset: cleared current password from database" );

  // mMasterPass empty, set new password (don't verify, since not stored yet)
  setMasterPassword( newpass, false );

  // store new password hash
  if ( ok && !masterPasswordStoreInDb() )
  {
    ok = false;
    const char* err = QT_TR_NOOP( "Master password reset FAILED: could not store new password in database" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
  }
  if ( ok )
    QgsDebugMsg( "Master password reset: stored new password in database" );

  // verify it stored password properly
  if ( ok && !verifyMasterPassword() )
  {
    ok = false;
    const char* err = QT_TR_NOOP( "Master password reset FAILED: could not verify new password in database" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
  }

  // re-encrypt everything with new password
  if ( ok && !reencryptAllAuthenticationConfigs( prevpass, prevciv ) )
  {
    ok = false;
    const char* err = QT_TR_NOOP( "Master password reset FAILED: could not re-encrypt configs in database" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
  }
  if ( ok )
    QgsDebugMsg( "Master password reset: re-encrypted configs in database" );

  // verify it all worked
  if ( ok && !verifyPasswordCanDecryptConfigs() )
  {
    ok = false;
    const char* err = QT_TR_NOOP( "Master password reset FAILED: could not verify password can decrypt re-encrypted configs" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
  }

  // something went wrong, reinstate previous password and database
  if ( !ok )
  {
    // backup database of failed attempt, for inspection
    authDbConnection().close();
    QString errdbbackup( dbbackup );
    errdbbackup.replace( QString( ".db" ), QString( "_ERROR.db" ) );
    QFile::rename( authenticationDbPath(), errdbbackup );
    QgsDebugMsg( QString( "Master password reset FAILED: backed up failed db at %1" ).arg( errdbbackup ) );

    // reinstate previous database and password
    QFile::rename( dbbackup, authenticationDbPath() );
    mMasterPass = prevpass;
    authDbConnection();
    QgsDebugMsg( "Master password reset FAILED: reinstated previous password and database" );

    // assign error db backup
    if ( backuppath )
      *backuppath = errdbbackup;

    return false;
  }


  if ( !keepbackup && !QFile::remove( dbbackup ) )
  {
    const char* err = QT_TR_NOOP( "Master password reset: could not remove old database backup" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    // a non-blocking error, continue
  }

  if ( keepbackup )
  {
    QgsDebugMsg( QString( "Master password reset: backed up previous db at %1" ).arg( dbbackup ) );
    if ( backuppath )
      *backuppath = dbbackup;
  }

  QgsDebugMsg( "Master password reset: SUCCESS" );
  return true;
}

void QgsAuthManager::registerProviders()
{
  if ( isDisabled() )
    return;

  if ( !mProvidersRegistered )
  {
    mProviders.insert( QgsAuthType::Basic, new QgsAuthProviderBasic() );
#ifndef QT_NO_OPENSSL
    mProviders.insert( QgsAuthType::PkiPaths, new QgsAuthProviderPkiPaths() );
    mProviders.insert( QgsAuthType::PkiPkcs12, new QgsAuthProviderPkiPkcs12() );
#endif
  }
  mProvidersRegistered = true;
}

const QString QgsAuthManager::uniqueConfigId() const
{
  QStringList configids = configIds();
  QString id;
  int len = 7;
  // sleep just a bit to make sure the current time has changed
  QEventLoop loop;
  QTimer::singleShot( 3, &loop, SLOT( quit() ) );
  loop.exec();

  uint seed = ( uint ) QTime::currentTime().msec();
  qsrand( seed );

  while ( true )
  {
    id = "";
    for ( int i = 0; i < len; i++ )
    {
      switch ( qrand() % 2 )
      {
        case 0:
          id += ( '0' + qrand() % 10 );
          break;
        case 1:
          id += ( 'a' + qrand() % 26 );
          break;
      }
    }
    if ( !configids.contains( id ) )
    {
      break;
    }
  }
  QgsDebugMsg( QString( "Generated unique ID: %1" ).arg( id ) );
  return id;
}

bool QgsAuthManager::configIdUnique( const QString& id ) const
{
  if ( isDisabled() )
    return false;

  if ( id.isEmpty() )
  {
    const char* err = QT_TR_NOOP( "Config ID is empty" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }
  QStringList configids = configIds();
  return !configids.contains( id );
}

QHash<QString, QgsAuthConfigBase> QgsAuthManager::availableConfigs()
{
  QHash<QString, QgsAuthConfigBase> baseConfigs;

  if ( isDisabled() )
    return baseConfigs;

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "SELECT id, name, uri, type, version FROM %1" ).arg( authDbConfigTable() ) );

  if ( !authDbQuery( &query ) )
  {
    return baseConfigs;
  }

  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      QString authid = query.value( 0 ).toString();
      QgsAuthConfigBase config;
      config.setId( authid );
      config.setName( query.value( 1 ).toString() );
      config.setUri( query.value( 2 ).toString() );
      config.setType( QgsAuthType::stringToType( query.value( 3 ).toString() ) );
      config.setVersion( query.value( 4 ).toInt() );

      baseConfigs.insert( authid, config );
    }
  }
  return baseConfigs;
}

void QgsAuthManager::updateConfigProviderTypes()
{
  if ( isDisabled() )
    return;

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "SELECT id, type FROM %1" ).arg( authDbConfigTable() ) );

  if ( !authDbQuery( &query ) )
  {
    return;
  }

  if ( query.isActive() )
  {
    QgsDebugMsg( "Synching existing auth config provider types" );
    mConfigProviders.clear();
    while ( query.next() )
    {
      mConfigProviders.insert( query.value( 0 ).toString(),
                               QgsAuthType::stringToType( query.value( 1 ).toString() ) );
    }
  }
}

QgsAuthProvider* QgsAuthManager::configProvider( const QString& authid )
{
  if ( isDisabled() )
    return 0;

  if ( !mConfigProviders.contains( authid ) )
  {
    QgsDebugMsg( QString( "No config provider found for authid: %1" ).arg( authid ) );
    return 0;
  }

  QgsAuthType::ProviderType ptype = mConfigProviders.value( authid );

  if ( ptype == QgsAuthType::None || ptype == QgsAuthType::Unknown )
  {
    QgsDebugMsg( QString( "Provider type None or Unknown for authid: %1" ).arg( authid ) );
    return 0;
  }

  return mProviders.value( ptype );
}

QgsAuthType::ProviderType QgsAuthManager::configProviderType( const QString& authid )
{
  if ( isDisabled() )
    return QgsAuthType::Unknown;

  if ( !mConfigProviders.contains( authid ) )
    return QgsAuthType::Unknown;

  return mConfigProviders.value( authid );
}

bool QgsAuthManager::storeAuthenticationConfig( QgsAuthConfigBase &config )
{
  if ( !setMasterPassword( true ) )
    return false;

  // don't need to validate id, since it has not be defined yet
  if ( !config.isValid() )
  {
    const char* err = QT_TR_NOOP( "Store config: FAILED because config is invalid" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }

  QString configstring = config.configString();
  if ( configstring.isEmpty() )
  {
    const char* err = QT_TR_NOOP( "Store config: FAILED because config is empty" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }
#if( 0 )
  QgsDebugMsg( QString( "authDbConfigTable(): %1" ).arg( authDbConfigTable() ) );
  QgsDebugMsg( QString( "name: %1" ).arg( config.name() ) );
  QgsDebugMsg( QString( "uri: %1" ).arg( config.uri() ) );
  QgsDebugMsg( QString( "type: %1" ).arg( config.typeToString() ) );
  QgsDebugMsg( QString( "version: %1" ).arg( config.version() ) );
  QgsDebugMsg( QString( "config: %1" ).arg( configstring ) ); // DO NOT LEAVE THIS LINE UNCOMMENTED !
#endif

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "INSERT INTO %1 (id, name, uri, type, version, config) "
                          "VALUES (:id, :name, :uri, :type, :version, :config)" ).arg( authDbConfigTable() ) );

  QString uid = uniqueConfigId();

  query.bindValue( ":id", uid );
  query.bindValue( ":name", config.name() );
  query.bindValue( ":uri", config.uri() );
  query.bindValue( ":type", config.typeToString() );
  query.bindValue( ":version", config.version() );
  query.bindValue( ":config", QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), configstring ) );

  if ( !authDbStartTransaction() )
    return false;

  if ( !authDbQuery( &query ) )
    return false;

  if ( !authDbCommit() )
    return false;

  // passed-in config should now be like as if it was just loaded from db
  config.setId( uid );

  updateConfigProviderTypes();

  QgsDebugMsg( QString( "Store config SUCCESS for authid: %1" ).arg( uid ) );
  return true;
}

bool QgsAuthManager::updateAuthenticationConfig( const QgsAuthConfigBase& config )
{
  if ( !setMasterPassword( true ) )
    return false;

  // validate id
  if ( !config.isValid( true ) )
  {
    const char* err = QT_TR_NOOP( "Update config: FAILED because config is invalid" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }

  QString configstring = config.configString();
  if ( configstring.isEmpty() )
  {
    const char* err = QT_TR_NOOP( "Update config: FAILED because config is empty" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }

#if( 0 )
  QgsDebugMsg( QString( "authDbConfigTable(): %1" ).arg( authDbConfigTable() ) );
  QgsDebugMsg( QString( "id: %1" ).arg( config.id() ) );
  QgsDebugMsg( QString( "name: %1" ).arg( config.name() ) );
  QgsDebugMsg( QString( "uri: %1" ).arg( config.uri() ) );
  QgsDebugMsg( QString( "type: %1" ).arg( config.typeToString() ) );
  QgsDebugMsg( QString( "version: %1" ).arg( config.version() ) );
  QgsDebugMsg( QString( "config: %1" ).arg( configstring ) ); // DO NOT LEAVE THIS LINE UNCOMMENTED !
#endif

  QSqlQuery query( authDbConnection() );
  if ( !query.prepare( QString( "UPDATE %1 "
                                "SET name = :name, uri = :uri, type = :type, version = :version, config = :config "
                                "WHERE id = :id" ).arg( authDbConfigTable() ) ) )
  {
    const char* err = QT_TR_NOOP( "Update config: FAILED to prepare query" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }

  query.bindValue( ":id", config.id() );
  query.bindValue( ":name", config.name() );
  query.bindValue( ":uri", config.uri() );
  query.bindValue( ":type", config.typeToString() );
  query.bindValue( ":version", config.version() );
  query.bindValue( ":config", QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), configstring ) );

  if ( !authDbStartTransaction() )
    return false;

  if ( !authDbQuery( &query ) )
    return false;

  if ( !authDbCommit() )
    return false;

  // should come before updating provider types, in case user switched providers in config
  clearCachedConfig( config.id() );

  updateConfigProviderTypes();

  QgsDebugMsg( QString( "Update config SUCCESS for authid: %1" ).arg( config.id() ) );

  return true;
}

bool QgsAuthManager::loadAuthenticationConfig( const QString& authid, QgsAuthConfigBase &config, bool full )
{
  if ( isDisabled() )
    return false;

  if ( full && !setMasterPassword( true ) )
    return false;

  QSqlQuery query( authDbConnection() );
  full = full && config.type() != QgsAuthType::None; // negates 'full' if loading into base class
  if ( full )
  {
    query.prepare( QString( "SELECT id, name, uri, type, version, config FROM %1 "
                            "WHERE id = :id" ).arg( authDbConfigTable() ) );
  }
  else
  {
    query.prepare( QString( "SELECT id, name, uri, type, version FROM %1 "
                            "WHERE id = :id" ).arg( authDbConfigTable() ) );
  }

  query.bindValue( ":id", authid );

  if ( !authDbQuery( &query ) )
  {
    return false;
  }

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      config.setId( query.value( 0 ).toString() );
      config.setName( query.value( 1 ).toString() );
      config.setUri( query.value( 2 ).toString() );
      config.setType( QgsAuthType::stringToType( query.value( 3 ).toString() ) );
      config.setVersion( query.value( 4 ).toInt() );

      if ( full )
      {
        config.loadConfigString( QgsAuthCrypto::decrypt( mMasterPass, masterPasswordCiv(), query.value( 5 ).toString() ) );
      }

      QgsDebugMsg( QString( "Load %1 config SUCCESS for authid: %2" ).arg( full ? "full" : "base" ) .arg( authid ) );
      return true;
    }
    if ( query.next() )
    {
      QgsDebugMsg( QString( "Select contains more than one for authid: %1" ).arg( authid ) );
      emit messageOut( tr( "Authentication database contains duplicate configuration IDs" ), authManTag(), WARNING );
    }
  }
  return false;
}

bool QgsAuthManager::removeAuthenticationConfig( const QString& authid )
{
  if ( isDisabled() )
    return false;

  if ( authid.isEmpty() )
    return false;

  QSqlQuery query( authDbConnection() );

  query.prepare( QString( "DELETE FROM %1 WHERE id = :id" ).arg( authDbConfigTable() ) );

  query.bindValue( ":id", authid );

  if ( !authDbStartTransaction() )
    return false;

  if ( !authDbQuery( &query ) )
    return false;

  if ( !authDbCommit() )
    return false;

  clearCachedConfig( authid );

  updateConfigProviderTypes();

  QgsDebugMsg( QString( "REMOVED config for authid: %1" ).arg( authid ) );

  return true;
}

bool QgsAuthManager::removeAllAuthenticationConfigs()
{
  if ( isDisabled() )
    return false;

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "DELETE FROM %1" ).arg( authDbConfigTable() ) );
  bool res = authDbTransactionQuery( &query );

  if ( res )
  {
    clearAllCachedConfigs();
    updateConfigProviderTypes();
  }

  QgsDebugMsg( QString( "Remove configs from database: %1" ).arg( res ? "SUCCEEDED" : "FAILED" ) );

  return res;
}

bool QgsAuthManager::eraseAuthenticationDatabase()
{
  if ( isDisabled() )
    return false;

  bool erased = ( removeAllAuthenticationConfigs() && masterPasswordClearDb() );
  QgsDebugMsg( QString( "Erase database: %1" ).arg( erased ? "SUCCEEDED" : "FAILED" ) );
  return erased;
}

bool QgsAuthManager::updateNetworkRequest( QNetworkRequest &request, const QString& authid )
{
  if ( isDisabled() )
    return false;

  QgsAuthProvider* provider = configProvider( authid );
  if ( provider )
  {
    if ( !provider->updateNetworkRequest( request, authid ) )
    {
      provider->clearCachedConfig( authid );
      return false;
    }
    return true;
  }
  QgsDebugMsg( QString( "No provider returned for authid: %1" ).arg( authid ) );
  return false;
}

bool QgsAuthManager::updateNetworkReply( QNetworkReply *reply, const QString& authid )
{
  if ( isDisabled() )
    return false;

  QgsAuthProvider* provider = configProvider( authid );
  if ( provider )
  {
    if ( !provider->updateNetworkReply( reply , authid ) )
    {
      provider->clearCachedConfig( authid );
      return false;
    }
    return true;
  }
  QgsDebugMsg( QString( "No provider returned for authid: %1" ).arg( authid ) );
  return false;
}

void QgsAuthManager::clearAllCachedConfigs()
{
  if ( isDisabled() )
    return;

  foreach ( QString configid, configIds() )
  {
    clearCachedConfig( configid );
  }
}

void QgsAuthManager::clearCachedConfig( const QString& authid )
{
  if ( isDisabled() )
    return;

  QgsAuthProvider* provider = configProvider( authid );
  if ( provider )
  {
    provider->clearCachedConfig( authid );
  }
}

void QgsAuthManager::writeToConsole( const QString &message,
                                     const QString &tag,
                                     QgsAuthManager::MessageLevel level )
{
  Q_UNUSED( tag );

  // only output WARNING and CRITICAL messages
  if ( level == QgsAuthManager::INFO )
    return;

  QString msg;
  switch ( level )
  {
    case QgsAuthManager::WARNING:
      msg += "WARNING: ";
      break;
    case QgsAuthManager::CRITICAL:
      msg += "ERROR: ";
      break;
    default:
      break;
  }
  msg += message;

  QTextStream out( stdout, QIODevice::WriteOnly );
  out << msg << endl;
}

QgsAuthManager::QgsAuthManager()
    : QObject()
    , mAuthDbPath( QString() )
    , mQcaInitializer( 0 )
    , mProvidersRegistered( false )
    , mMasterPass( QString() )
    , mAuthDisabled( false )
{
  connect( this, SIGNAL( messageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ),
           this, SLOT( writeToConsole( const QString&, const QString&, QgsAuthManager::MessageLevel ) ) );
}

QgsAuthManager::~QgsAuthManager()
{
  if ( !isDisabled() )
  {
    authDbConnection().close();
    qDeleteAll( mProviders.values() );
  }
  delete mQcaInitializer;
  mQcaInitializer = 0;
}

bool QgsAuthManager::masterPasswordInput()
{
  if ( isDisabled() )
    return false;

  QString pass;
  QgsCredentials * creds = QgsCredentials::instance();
  creds->lock();
  bool ok = creds->getMasterPassword( pass, masterPasswordHashInDb() );
  creds->unlock();

  if ( ok && !pass.isEmpty() && !masterPasswordSame( pass ) )
  {
    mMasterPass = pass;
    return true;
  }
  return false;
}

bool QgsAuthManager::masterPasswordRowsInDb( int *rows ) const
{
  if ( isDisabled() )
    return false;

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "SELECT Count(*) FROM %1" ).arg( authDbPassTable() ) );

  bool ok = authDbQuery( &query );
  if ( query.first() )
  {
    *rows = query.value( 0 ).toInt();
  }

  return ok;
}

bool QgsAuthManager::masterPasswordHashInDb() const
{
  if ( isDisabled() )
    return false;

  int rows = 0;
  if ( !masterPasswordRowsInDb( &rows ) )
  {
    const char* err = QT_TR_NOOP( "Master password: FAILED to access database" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), CRITICAL );

    return false;
  }
  return ( rows == 1 );
}

bool QgsAuthManager::masterPasswordCheckAgainstDb() const
{
  if ( isDisabled() )
    return false;

  // first verify there is only one row in auth db (uses first found)

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "SELECT salt, hash FROM %1" ).arg( authDbPassTable() ) );
  if ( !authDbQuery( &query ) )
    return false;

  if ( !query.first() )
    return false;

  QString salt = query.value( 0 ).toString();
  QString hash = query.value( 1 ).toString();

  return QgsAuthCrypto::verifyPasswordKeyHash( mMasterPass, salt, hash );
}

bool QgsAuthManager::masterPasswordStoreInDb() const
{
  if ( isDisabled() )
    return false;

  QString salt, hash, civ;
  QgsAuthCrypto::passwordKeyHash( mMasterPass, &salt, &hash, &civ );

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "INSERT INTO %1 (salt, hash, civ) VALUES (:salt, :hash, :civ)" ).arg( authDbPassTable() ) );

  query.bindValue( ":salt", salt );
  query.bindValue( ":hash", hash );
  query.bindValue( ":civ", civ );

  if ( !authDbStartTransaction() )
    return false;

  if ( !authDbQuery( &query ) )
    return false;

  if ( !authDbCommit() )
    return false;

  return true;
}

bool QgsAuthManager::masterPasswordClearDb()
{
  if ( isDisabled() )
    return false;

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "DELETE FROM %1" ).arg( authDbPassTable() ) );
  bool res = authDbTransactionQuery( &query );
  if ( res )
    clearMasterPassword();
  return res;
}

const QString QgsAuthManager::masterPasswordCiv() const
{
  if ( isDisabled() )
    return QString();

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "SELECT civ FROM %1" ).arg( authDbPassTable() ) );
  if ( !authDbQuery( &query ) )
    return QString();

  if ( !query.first() )
    return QString();

  return query.value( 0 ).toString();
}

QStringList QgsAuthManager::configIds() const
{
  QStringList configids = QStringList();

  if ( isDisabled() )
    return configids;

  QSqlQuery query( authDbConnection() );
  query.prepare( QString( "SELECT id FROM %1" ).arg( authDbConfigTable() ) );

  if ( !authDbQuery( &query ) )
  {
    return configids;
  }

  if ( query.isActive() )
  {
    while ( query.next() )
    {
      configids << query.value( 0 ).toString();
    }
  }
  return configids;
}

bool QgsAuthManager::verifyPasswordCanDecryptConfigs() const
{
  if ( isDisabled() )
    return false;

  // no need to check for setMasterPassword, since this is private and it will be set

  QSqlQuery query( authDbConnection() );

  query.prepare( QString( "SELECT id, config FROM %1" ).arg( authDbConfigTable() ) );

  if ( !authDbQuery( &query ) )
    return false;

  if ( !query.isActive() || !query.isSelect() )
  {
    QgsDebugMsg( QString( "Verify password can decrypt configs FAILED, query not active or a select operation" ) );
    return false;
  }

  int checked = 0;
  while ( query.next() )
  {
    ++checked;
    QString configstring( QgsAuthCrypto::decrypt( mMasterPass, masterPasswordCiv(), query.value( 1 ).toString() ) );
    if ( configstring.isEmpty() )
    {
      QgsDebugMsg( QString( "Verify password can decrypt configs FAILED, could not decrypt a config (id: %1)" )
                   .arg( query.value( 0 ).toString() ) );
      return false;
    }
  }

  QgsDebugMsg( QString( "Verify password can decrypt configs SUCCESS (checked %1 configs)" ).arg( checked ) );
  return true;
}

bool QgsAuthManager::reencryptAllAuthenticationConfigs( const QString &prevpass, const QString &prevciv )
{
  if ( isDisabled() )
    return false;

  bool res = true;
  foreach ( QString configid, configIds() )
  {
    res = res && reencryptAuthenticationConfig( configid, prevpass, prevciv );
  }
  return res;
}

bool QgsAuthManager::reencryptAuthenticationConfig( const QString &authid, const QString &prevpass, const QString &prevciv )
{
  if ( isDisabled() )
    return false;

  // no need to check for setMasterPassword, since this is private and it will be set

  QSqlQuery query( authDbConnection() );

  query.prepare( QString( "SELECT config FROM %1 "
                          "WHERE id = :id" ).arg( authDbConfigTable() ) );

  query.bindValue( ":id", authid );

  if ( !authDbQuery( &query ) )
    return false;

  if ( !query.isActive() || !query.isSelect() )
  {
    QgsDebugMsg( QString( "Reencrypt FAILED, query not active or a select operation for authid: %2" ).arg( authid ) );
    return false;
  }

  if ( query.first() )
  {
    QString configstring( QgsAuthCrypto::decrypt( prevpass, prevciv, query.value( 0 ).toString() ) );

    query.clear();

    query.prepare( QString( "UPDATE %1 "
                            "SET config = :config "
                            "WHERE id = :id" ).arg( authDbConfigTable() ) );

    query.bindValue( ":id", authid );
    query.bindValue( ":config", QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), configstring ) );

    if ( !authDbStartTransaction() )
      return false;

    if ( !authDbQuery( &query ) )
      return false;

    if ( !authDbCommit() )
      return false;

    QgsDebugMsg( QString( "Reencrypt SUCCESS for authid: %2" ).arg( authid ) );
    return true;
  }
  else
  {
    QgsDebugMsg( QString( "Reencrypt FAILED, could not find in db authid: %2" ).arg( authid ) );
    return false;
  }

  if ( query.next() )
  {
    QgsDebugMsg( QString( "Select contains more than one for authid: %1" ).arg( authid ) );
    emit messageOut( tr( "Authentication database contains duplicate configuration IDs" ), authManTag(), WARNING );
  }

  return false;
}

bool QgsAuthManager::authDbOpen() const
{
  if ( isDisabled() )
    return false;

  QSqlDatabase authdb = authDbConnection();
  if ( !authdb.isOpen() )
  {
    if ( !authdb.open() )
    {
      QgsDebugMsg( QString( "Unable to establish database connection\nDatabase: %1\nDriver error: %2\nDatabase error: %3" )
                   .arg( authenticationDbPath() )
                   .arg( authdb.lastError().driverText() )
                   .arg( authdb.lastError().databaseText() ) );
      emit messageOut( tr( "Unable to establish authentication database connection" ), authManTag(), CRITICAL );
      return false;
    }
  }
  return true;
}

bool QgsAuthManager::authDbQuery( QSqlQuery *query ) const
{
  if ( isDisabled() )
    return false;

  query->setForwardOnly( true );
  query->exec();

  if ( query->lastError().isValid() )
  {
    QgsDebugMsg( QString( "Auth db query FAILED: %1\nError: %2" )
                 .arg( query->executedQuery() )
                 .arg( query->lastError().text() ) );
    emit messageOut( tr( "Auth db query FAILED" ), authManTag(), WARNING );
    return false;
  }

  return true;
}

bool QgsAuthManager::authDbStartTransaction() const
{
  if ( isDisabled() )
    return false;

  if ( !authDbConnection().transaction() )
  {
    const char* err = QT_TR_NOOP( "Auth db FAILED to start transaction" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }

  return true;
}

bool QgsAuthManager::authDbCommit() const
{
  if ( isDisabled() )
    return false;

  if ( !authDbConnection().commit() )
  {
    const char* err = QT_TR_NOOP( "Auth db FAILED to rollback changes" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    authDbConnection().rollback();
    return false;
  }

  return true;
}

bool QgsAuthManager::authDbTransactionQuery( QSqlQuery *query ) const
{
  if ( isDisabled() )
    return false;

  if ( !authDbConnection().transaction() )
  {
    const char* err = QT_TR_NOOP( "Auth db FAILED to start transaction" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    return false;
  }

  bool ok = authDbQuery( query );

  if ( ok && !authDbConnection().commit() )
  {
    const char* err = QT_TR_NOOP( "Auth db FAILED to rollback changes" );
    QgsDebugMsg( err );
    emit messageOut( tr( err ), authManTag(), WARNING );
    authDbConnection().rollback();
    return false;
  }

  return ok;
}

