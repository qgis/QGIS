/***************************************************************************
    qgsauthmanager.cpp
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

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QObject>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QVariant>
#include <QSqlDriver>
#include <QDomElement>
#include <QDomDocument>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QRandomGenerator>

#include <QtCrypto>

#ifndef QT_NO_SSL
#include <QSslConfiguration>
#endif

// QGIS includes
#include "qgsauthcertutils.h"
#include "qgsauthcrypto.h"
#include "qgsauthmethod.h"
#include "qgsauthmethodmetadata.h"
#include "qgsauthmethodregistry.h"
#include "qgscredentials.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsauthmanager.h"
#include "moc_qgsauthmanager.cpp"
#include "qgsauthconfigurationstorageregistry.h"
#include "qgsauthconfigurationstoragesqlite.h"
#include "qgsvariantutils.h"
#include "qgssettings.h"
#include "qgsruntimeprofiler.h"

QgsAuthManager *QgsAuthManager::sInstance = nullptr;

const QString QgsAuthManager::AUTH_CONFIG_TABLE = QStringLiteral( "auth_configs" );
const QString QgsAuthManager::AUTH_SERVERS_TABLE = QStringLiteral( "auth_servers" );
const QString QgsAuthManager::AUTH_MAN_TAG = QObject::tr( "Authentication Manager" );
const QString QgsAuthManager::AUTH_CFG_REGEX = QStringLiteral( "authcfg=([a-z]|[A-Z]|[0-9]){7}" );


const QLatin1String QgsAuthManager::AUTH_PASSWORD_HELPER_KEY_NAME_BASE( "QGIS-Master-Password" );
const QLatin1String QgsAuthManager::AUTH_PASSWORD_HELPER_FOLDER_NAME( "QGIS" );



#if defined(Q_OS_MAC)
const QString QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME( "Keychain" );
#elif defined(Q_OS_WIN)
const QString QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME( "Password Manager" );
#elif defined(Q_OS_LINUX)
const QString QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME( QStringLiteral( "Wallet/KeyRing" ) );
#else
const QString QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME( "Password Manager" );
#endif

QgsAuthManager *QgsAuthManager::instance()
{
  static QMutex sMutex;
  QMutexLocker locker( &sMutex );
  if ( !sInstance )
  {
    sInstance = new QgsAuthManager( );
  }
  return sInstance;
}


QgsAuthManager::QgsAuthManager()
{
  mMutex = std::make_unique<QRecursiveMutex>();
  mMasterPasswordMutex = std::make_unique<QRecursiveMutex>();
  connect( this, &QgsAuthManager::messageLog,
           this, &QgsAuthManager::writeToConsole );
}

QSqlDatabase QgsAuthManager::authDatabaseConnection() const
{
  ensureInitialized();

  QSqlDatabase authdb;

  if ( isDisabled() )
    return authdb;

  // while everything we use from QSqlDatabase here is thread safe, we need to ensure
  // that the connection cleanup on thread finalization happens in a predictable order
  QMutexLocker locker( mMutex.get() );

  // Get the first enabled DB storage from the registry
  if ( QgsAuthConfigurationStorageDb *storage = defaultDbStorage() )
  {
    return storage->authDatabaseConnection();
  }

  return authdb;
}

const QString QgsAuthManager::methodConfigTableName() const
{
  if ( ! isDisabled() )
  {
    ensureInitialized();

    // Returns the first enabled and ready "DB" storage
    QgsAuthConfigurationStorageRegistry *storageRegistry = authConfigurationStorageRegistry();
    const QList<QgsAuthConfigurationStorage *> storages { storageRegistry->readyStorages() };
    for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
    {
      if ( auto dbStorage = qobject_cast<QgsAuthConfigurationStorageDb *>( storage ) )
      {
        if ( dbStorage->capabilities() & Qgis::AuthConfigurationStorageCapability::ReadConfiguration )
        {
          return dbStorage->quotedQualifiedIdentifier( dbStorage->methodConfigTableName() );
        }
      }
    }
  }

  return QString();
}

bool QgsAuthManager::isFilesystemBasedDatabase( const QString &uri )
{
  // Loop through all registered SQL drivers and return false if
  // the URI starts with one of them except the SQLite based drivers
  const auto drivers { QSqlDatabase::drivers() };
  for ( const QString &driver : std::as_const( drivers ) )
  {
    if ( driver != ( QStringLiteral( "QSQLITE" ) ) && driver != ( QStringLiteral( "QSPATIALITE" ) ) && uri.startsWith( driver ) )
    {
      return false;
    }
  }
  return true;
}

const QString QgsAuthManager::authenticationDatabaseUri() const
{
  return mAuthDatabaseConnectionUri;
}

const QString QgsAuthManager::authenticationDatabaseUriStripped() const
{
  QRegularExpression re( QStringLiteral( "password=(.*)" ) );
  QString uri = mAuthDatabaseConnectionUri;
  return uri.replace( re, QStringLiteral( "password=*****" ) );
}


bool QgsAuthManager::init( const QString &pluginPath, const QString &authDatabasePath )
{
  mAuthDatabaseConnectionUri = authDatabasePath.startsWith( QLatin1String( "QSQLITE://" ) ) ? authDatabasePath : QStringLiteral( "QSQLITE://" ) + authDatabasePath;
  return initPrivate( pluginPath );
}

bool QgsAuthManager::ensureInitialized() const
{
  static QRecursiveMutex sInitializationMutex;
  static bool sInitialized = false;

  sInitializationMutex.lock();
  if ( sInitialized )
  {
    sInitializationMutex.unlock();
    return mLazyInitResult;
  }

  mLazyInitResult = const_cast< QgsAuthManager * >( this )->initPrivate( mPluginPath );
  sInitialized = true;
  sInitializationMutex.unlock();

  return mLazyInitResult;
}

static char *sPassFileEnv = nullptr;

bool QgsAuthManager::initPrivate( const QString &pluginPath )
{
  if ( mAuthInit )
    return true;

  mAuthInit = true;
  QgsScopedRuntimeProfile profile( tr( "Initializing authentication manager" ) );

  QgsDebugMsgLevel( QStringLiteral( "Initializing QCA..." ), 2 );
  mQcaInitializer = std::make_unique<QCA::Initializer>( QCA::Practical, 256 );

  QgsDebugMsgLevel( QStringLiteral( "QCA initialized." ), 2 );
  QCA::scanForPlugins();

  QgsDebugMsgLevel( QStringLiteral( "QCA Plugin Diagnostics Context: %1" ).arg( QCA::pluginDiagnosticText() ), 2 );
  QStringList capabilities;

  capabilities = QCA::supportedFeatures();
  QgsDebugMsgLevel( QStringLiteral( "QCA supports: %1" ).arg( capabilities.join( "," ) ), 2 );

  // do run-time check for qca-ossl plugin
  if ( !QCA::isSupported( "cert", QStringLiteral( "qca-ossl" ) ) )
  {
    mAuthDisabled = true;
    mAuthDisabledMessage = tr( "QCA's OpenSSL plugin (qca-ossl) is missing" );
    return isDisabled();
  }

  QgsDebugMsgLevel( QStringLiteral( "Prioritizing qca-ossl over all other QCA providers..." ), 2 );
  const QCA::ProviderList provds = QCA::providers();
  QStringList prlist;
  for ( QCA::Provider *p : provds )
  {
    QString pn = p->name();
    int pr = 0;
    if ( pn != QLatin1String( "qca-ossl" ) )
    {
      pr = QCA::providerPriority( pn ) + 1;
    }
    QCA::setProviderPriority( pn, pr );
    prlist << QStringLiteral( "%1:%2" ).arg( pn ).arg( QCA::providerPriority( pn ) );
  }
  QgsDebugMsgLevel( QStringLiteral( "QCA provider priorities: %1" ).arg( prlist.join( ", " ) ), 2 );

  QgsDebugMsgLevel( QStringLiteral( "Populating auth method registry" ), 3 );
  QgsAuthMethodRegistry *authreg = QgsAuthMethodRegistry::instance( pluginPath );

  QStringList methods = authreg->authMethodList();

  QgsDebugMsgLevel( QStringLiteral( "Authentication methods found: %1" ).arg( methods.join( ", " ) ), 2 );

  if ( methods.isEmpty() )
  {
    mAuthDisabled = true;
    mAuthDisabledMessage = tr( "No authentication method plugins found" );
    return isDisabled();
  }

  if ( !registerCoreAuthMethods() )
  {
    mAuthDisabled = true;
    mAuthDisabledMessage = tr( "No authentication method plugins could be loaded" );
    return isDisabled();
  }

  QgsDebugMsgLevel( QStringLiteral( "Auth database URI: %1" ).arg( mAuthDatabaseConnectionUri ), 2 );

  // Add the default configuration storage
  const QString sqliteDbPath { sqliteDatabasePath() };
  if ( ! sqliteDbPath.isEmpty() )
  {
    authConfigurationStorageRegistry()->addStorage( new QgsAuthConfigurationStorageSqlite( sqliteDbPath ) );
  }
  else if ( ! mAuthDatabaseConnectionUri.isEmpty() )
  {
    // For safety reasons we don't allow writing on potentially shared storages by default, plugins may override
    // this behavior by registering their own storage subclass or by explicitly setting read-only to false.
    QgsAuthConfigurationStorageDb *storage = new QgsAuthConfigurationStorageDb( mAuthDatabaseConnectionUri );
    if ( !QgsAuthManager::isFilesystemBasedDatabase( mAuthDatabaseConnectionUri ) )
    {
      storage->setReadOnly( true );
    }
    authConfigurationStorageRegistry()->addStorage( storage );
  }

  // Loop through all registered storages and call initialize
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->storages() };
  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( ! storage->isEnabled() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Storage %1 is disabled" ).arg( storage->name() ), 2 );
      continue;
    }
    if ( !storage->initialize() )
    {
      const QString err = tr( "Failed to initialize storage %1: %2" ).arg( storage->name(), storage->lastError() );
      QgsDebugError( err );
      emit messageLog( err, authManTag(), Qgis::MessageLevel::Warning );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Storage %1 initialized" ).arg( storage->name() ), 2 );
    }
    connect( storage, &QgsAuthConfigurationStorage::methodConfigChanged, this, [this] { updateConfigAuthMethods(); } );
    connect( storage, &QgsAuthConfigurationStorage::messageLog, this, &QgsAuthManager::messageLog );
  }

  updateConfigAuthMethods();

#ifndef QT_NO_SSL
  initSslCaches();
#endif
  // set the master password from first line of file defined by QGIS_AUTH_PASSWORD_FILE env variable
  if ( sPassFileEnv && masterPasswordHashInDatabase() )
  {
    QString passpath( sPassFileEnv );
    free( sPassFileEnv );
    sPassFileEnv = nullptr;

    QString masterpass;
    QFile passfile( passpath );
    if ( passfile.exists() && passfile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QTextStream passin( &passfile );
      while ( !passin.atEnd() )
      {
        masterpass = passin.readLine();
        break;
      }
      passfile.close();
    }
    if ( !masterpass.isEmpty() )
    {
      if ( setMasterPassword( masterpass, true ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "Authentication master password set from QGIS_AUTH_PASSWORD_FILE" ), 2 );
      }
      else
      {
        QgsDebugError( "QGIS_AUTH_PASSWORD_FILE set, but FAILED to set password using: " + passpath );
        return false;
      }
    }
    else
    {
      QgsDebugError( "QGIS_AUTH_PASSWORD_FILE set, but FAILED to read password from: " + passpath );
      return false;
    }
  }

#ifndef QT_NO_SSL
  initSslCaches();
#endif

  return true;
}

void QgsAuthManager::setup( const QString &pluginPath, const QString &authDatabasePath )
{
  mPluginPath = pluginPath;
  mAuthDatabaseConnectionUri = authDatabasePath;

  const char *p = getenv( "QGIS_AUTH_PASSWORD_FILE" );
  if ( p )
  {
    sPassFileEnv = qstrdup( p );

    // clear the env variable, so it can not be accessed from plugins, etc.
    // (note: stored QgsApplication::systemEnvVars() skips this env variable as well)
#ifdef Q_OS_WIN
    putenv( "QGIS_AUTH_PASSWORD_FILE" );
#else
    unsetenv( "QGIS_AUTH_PASSWORD_FILE" );
#endif
  }
}

bool QgsAuthManager::isDisabled() const
{
  ensureInitialized();

  if ( mAuthDisabled )
  {
    QgsDebugError( QStringLiteral( "Authentication system DISABLED: QCA's qca-ossl (OpenSSL) plugin is missing" ) );
  }
  return mAuthDisabled;
}

const QString QgsAuthManager::disabledMessage() const
{
  ensureInitialized();

  return tr( "Authentication system is DISABLED:\n%1" ).arg( mAuthDisabledMessage );
}


const QString QgsAuthManager::sqliteDatabasePath() const
{
  if ( !QgsAuthManager::isFilesystemBasedDatabase( mAuthDatabaseConnectionUri ) )
  {
    return QString();
  }

  // Remove the driver:// prefix if present
  QString path = mAuthDatabaseConnectionUri;
  if ( path.startsWith( QStringLiteral( "QSQLITE://" ), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    path = path.mid( 10 );
  }
  else if ( path.startsWith( QStringLiteral( "QSPATIALITE://" ), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    path = path.mid( 14 );
  }

  return QDir::cleanPath( path );
}

const QString QgsAuthManager::authenticationDatabasePath() const
{
  return sqliteDatabasePath();
}

bool QgsAuthManager::setMasterPassword( bool verify )
{
  ensureInitialized();

  QMutexLocker locker( mMasterPasswordMutex.get() );
  if ( isDisabled() )
    return false;

  if ( mScheduledDbErase )
    return false;

  if ( mMasterPass.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Master password is not yet set by user" ), 2 );
    if ( !masterPasswordInput() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Master password input canceled by user" ), 2 );
      return false;
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Master password is set" ), 2 );
    if ( !verify )
      return true;
  }

  if ( !verifyMasterPassword() )
    return false;

  QgsDebugMsgLevel( QStringLiteral( "Master password is set and verified" ), 2 );
  return true;
}

bool QgsAuthManager::setMasterPassword( const QString &pass, bool verify )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( isDisabled() )
    return false;

  if ( mScheduledDbErase )
    return false;

  // since this is generally for automation, we don't care if passed-in is same as existing
  QString prevpass = QString( mMasterPass );
  mMasterPass = pass;
  if ( verify && !verifyMasterPassword() )
  {
    mMasterPass = prevpass;
    const char *err = QT_TR_NOOP( "Master password set: FAILED to verify, reset to previous" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "Master password set: SUCCESS%1" ).arg( verify ? " and verified" : "" ), 2 );
  return true;
}

bool QgsAuthManager::verifyMasterPassword( const QString &compare )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  int rows = 0;
  if ( !masterPasswordRowsInDb( &rows ) )
  {
    const char *err = QT_TR_NOOP( "Master password: FAILED to access database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Critical );

    clearMasterPassword();
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "Master password: %1 rows in database" ).arg( rows ), 2 );

  if ( rows > 1 )
  {
    const char *err = QT_TR_NOOP( "Master password: FAILED to find just one master password record in database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );

    clearMasterPassword();
    return false;
  }
  else if ( rows == 1 )
  {
    if ( !masterPasswordCheckAgainstDb( compare ) )
    {
      if ( compare.isNull() ) // don't complain when comparing, since it could be an incomplete comparison string
      {
        const char *err = QT_TR_NOOP( "Master password: FAILED to verify against hash in database" );
        QgsDebugError( err );
        emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );

        clearMasterPassword();

        emit masterPasswordVerified( false );
      }
      ++mPassTries;
      if ( mPassTries >= 5 )
      {
        mAuthDisabled = true;
        const char *err = QT_TR_NOOP( "Master password: failed 5 times authentication system DISABLED" );
        QgsDebugError( err );
        emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
      }
      return false;
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Master password: verified against hash in database" ), 2 );
      if ( compare.isNull() )
        emit masterPasswordVerified( true );
    }
  }
  else if ( compare.isNull() ) // compares should never be stored
  {
    if ( !masterPasswordStoreInDb() )
    {
      const char *err = QT_TR_NOOP( "Master password: hash FAILED to be stored in database" );
      QgsDebugError( err );
      emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Critical );

      clearMasterPassword();
      return false;
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Master password: hash stored in database" ), 2 );
    }
    // double-check storing
    if ( !masterPasswordCheckAgainstDb() )
    {
      const char *err = QT_TR_NOOP( "Master password: FAILED to verify against hash in database" );
      QgsDebugError( err );
      emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );

      clearMasterPassword();
      emit masterPasswordVerified( false );
      return false;
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Master password: verified against hash in database" ), 2 );
      emit masterPasswordVerified( true );
    }
  }

  return true;
}

bool QgsAuthManager::masterPasswordIsSet() const
{
  ensureInitialized();

  return !mMasterPass.isEmpty();
}

bool QgsAuthManager::masterPasswordSame( const QString &pass ) const
{
  ensureInitialized();

  return mMasterPass == pass;
}

bool QgsAuthManager::resetMasterPassword( const QString &newpass, const QString &oldpass,
    bool keepbackup, QString *backuppath )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  // verify caller knows the current master password
  // this means that the user will have had to already set the master password as well
  if ( !masterPasswordSame( oldpass ) )
    return false;

  QString dbbackup;
  if ( !backupAuthenticationDatabase( &dbbackup ) )
    return false;

  QgsDebugMsgLevel( QStringLiteral( "Master password reset: backed up current database" ), 2 );

  // store current password and civ
  QString prevpass = QString( mMasterPass );
  QString prevciv = QString( masterPasswordCiv() );

  // on ANY FAILURE from this point, reinstate previous password and database
  bool ok = true;

  // clear password hash table (also clears mMasterPass)
  if ( ok && !masterPasswordClearDb() )
  {
    ok = false;
    const char *err = QT_TR_NOOP( "Master password reset FAILED: could not clear current password from database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
  }
  if ( ok )
  {
    QgsDebugMsgLevel( QStringLiteral( "Master password reset: cleared current password from database" ), 2 );
  }

  // mMasterPass empty, set new password (don't verify, since not stored yet)
  setMasterPassword( newpass, false );

  // store new password hash
  if ( ok && !masterPasswordStoreInDb() )
  {
    ok = false;
    const char *err = QT_TR_NOOP( "Master password reset FAILED: could not store new password in database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
  }
  if ( ok )
  {
    QgsDebugMsgLevel( QStringLiteral( "Master password reset: stored new password in database" ), 2 );
  }

  // verify it stored password properly
  if ( ok && !verifyMasterPassword() )
  {
    ok = false;
    const char *err = QT_TR_NOOP( "Master password reset FAILED: could not verify new password in database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
  }

  // re-encrypt everything with new password
  if ( ok && !reencryptAllAuthenticationConfigs( prevpass, prevciv ) )
  {
    ok = false;
    const char *err = QT_TR_NOOP( "Master password reset FAILED: could not re-encrypt configs in database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
  }
  if ( ok )
  {
    QgsDebugMsgLevel( QStringLiteral( "Master password reset: re-encrypted configs in database" ), 2 );
  }

  // verify it all worked
  if ( ok && !verifyPasswordCanDecryptConfigs() )
  {
    ok = false;
    const char *err = QT_TR_NOOP( "Master password reset FAILED: could not verify password can decrypt re-encrypted configs" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
  }

  if ( ok && !reencryptAllAuthenticationSettings( prevpass, prevciv ) )
  {
    ok = false;
    const char *err = QT_TR_NOOP( "Master password reset FAILED: could not re-encrypt settings in database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
  }

  if ( ok && !reencryptAllAuthenticationIdentities( prevpass, prevciv ) )
  {
    ok = false;
    const char *err = QT_TR_NOOP( "Master password reset FAILED: could not re-encrypt identities in database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
  }

  // something went wrong, reinstate previous password and database
  if ( !ok )
  {
    // backup database of failed attempt, for inspection
    QString errdbbackup( dbbackup );
    errdbbackup.replace( QLatin1String( ".db" ), QLatin1String( "_ERROR.db" ) );
    QFile::rename( sqliteDatabasePath(), errdbbackup );
    QgsDebugError( QStringLiteral( "Master password reset FAILED: backed up failed db at %1" ).arg( errdbbackup ) );
    // reinstate previous database and password
    QFile::rename( dbbackup, sqliteDatabasePath() );
    mMasterPass = prevpass;
    QgsDebugError( QStringLiteral( "Master password reset FAILED: reinstated previous password and database" ) );

    // assign error db backup
    if ( backuppath )
      *backuppath = errdbbackup;

    return false;
  }


  if ( !keepbackup && !QFile::remove( dbbackup ) )
  {
    const char *err = QT_TR_NOOP( "Master password reset: could not remove old database backup" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    // a non-blocking error, continue
  }

  if ( keepbackup )
  {
    QgsDebugMsgLevel( QStringLiteral( "Master password reset: backed up previous db at %1" ).arg( dbbackup ), 2 );
    if ( backuppath )
      *backuppath = dbbackup;
  }

  QgsDebugMsgLevel( QStringLiteral( "Master password reset: SUCCESS" ), 2 );
  emit authDatabaseChanged();
  return true;
}

void QgsAuthManager::setScheduledAuthDatabaseErase( bool scheduleErase )
{
  ensureInitialized();

  mScheduledDbErase = scheduleErase;
  // any call (start or stop) should reset these
  mScheduledDbEraseRequestEmitted = false;
  mScheduledDbEraseRequestCount = 0;

  if ( scheduleErase )
  {
    if ( !mScheduledDbEraseTimer )
    {
      mScheduledDbEraseTimer = new QTimer( this );
      connect( mScheduledDbEraseTimer, &QTimer::timeout, this, &QgsAuthManager::tryToStartDbErase );
      mScheduledDbEraseTimer->start( mScheduledDbEraseRequestWait * 1000 );
    }
    else if ( !mScheduledDbEraseTimer->isActive() )
    {
      mScheduledDbEraseTimer->start();
    }
  }
  else
  {
    if ( mScheduledDbEraseTimer && mScheduledDbEraseTimer->isActive() )
      mScheduledDbEraseTimer->stop();
  }
}

bool QgsAuthManager::registerCoreAuthMethods()
{
  if ( isDisabled() )
    return false;

  qDeleteAll( mAuthMethods );
  mAuthMethods.clear();
  const QStringList methods = QgsAuthMethodRegistry::instance()->authMethodList();
  for ( const auto &authMethodKey : methods )
  {
    mAuthMethods.insert( authMethodKey, QgsAuthMethodRegistry::instance()->createAuthMethod( authMethodKey ) );
  }

  return !mAuthMethods.isEmpty();
}

const QString QgsAuthManager::uniqueConfigId() const
{
  ensureInitialized();

  QStringList configids = configIds();
  QString id;
  int len = 7;

  // Suppress warning: Potential leak of memory in qtimer.h [clang-analyzer-cplusplus.NewDeleteLeaks]
#ifndef __clang_analyzer__
  // sleep just a bit to make sure the current time has changed
  QEventLoop loop;
  QTimer::singleShot( 3, &loop, &QEventLoop::quit );
  loop.exec();
#endif

  while ( true )
  {
    id.clear();
    for ( int i = 0; i < len; i++ )
    {
      switch ( QRandomGenerator::system()->generate() % 2 )
      {
        case 0:
          id += static_cast<char>( '0' + QRandomGenerator::system()->generate() % 10 );
          break;
        case 1:
          id += static_cast<char>( 'a' + QRandomGenerator::system()->generate() % 26 );
          break;
      }
    }
    if ( !configids.contains( id ) )
    {
      break;
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "Generated unique ID: %1" ).arg( id ), 2 );
  return id;
}

bool QgsAuthManager::configIdUnique( const QString &id ) const
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  if ( id.isEmpty() )
  {
    const char *err = QT_TR_NOOP( "Config ID is empty" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }
  QStringList configids = configIds();
  return !configids.contains( id );
}

bool QgsAuthManager::hasConfigId( const QString &txt )
{
  const thread_local QRegularExpression authCfgRegExp( AUTH_CFG_REGEX );
  return txt.indexOf( authCfgRegExp ) != -1;
}

QgsAuthMethodConfigsMap QgsAuthManager::availableAuthMethodConfigs( const QString &dataprovider )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QStringList providerAuthMethodsKeys;
  if ( !dataprovider.isEmpty() )
  {
    providerAuthMethodsKeys = authMethodsKeys( dataprovider.toLower() );
  }

  QgsAuthMethodConfigsMap baseConfigs;

  if ( isDisabled() )
    return baseConfigs;

  // Loop through all storages with capability ReadConfiguration and get the auth methods
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration ) };
  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    QgsAuthMethodConfigsMap configs = storage->authMethodConfigs();
    for ( const QgsAuthMethodConfig &config : std::as_const( configs ) )
    {
      if ( providerAuthMethodsKeys.isEmpty() || providerAuthMethodsKeys.contains( config.method() ) )
      {
        // Check if the config with that id is already in the list and warn if it is
        if ( baseConfigs.contains( config.id() ) )
        {
          // This may not be an error, since the same config may be stored in multiple storages.
          emit messageLog( tr( "A config with same id %1 was already added, skipping from %2" ).arg( config.id(), storage->name() ), authManTag(), Qgis::MessageLevel::Warning );
        }
        else
        {
          baseConfigs.insert( config.id(), config );
        }
      }
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
    QgsDebugError( QStringLiteral( "No credentials storages found" ) );
  }

  return baseConfigs;

}

void QgsAuthManager::updateConfigAuthMethods()
{
  ensureInitialized();

  // Loop through all registered storages and get the auth methods
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration ) };
  QStringList configIds;
  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    const QgsAuthMethodConfigsMap configs = storage->authMethodConfigs();
    for ( const QgsAuthMethodConfig &config : std::as_const( configs ) )
    {
      if ( ! configIds.contains( config.id() ) )
      {
        mConfigAuthMethods.insert( config.id(), config.method() );
        QgsDebugMsgLevel( QStringLiteral( "Stored auth config/methods:\n%1 %2" ).arg( config.id(), config.method() ), 2 );
      }
      else
      {
        // This may not be an error, since the same config may be stored in multiple storages.
        // A warning is issued when creating the list initially from availableAuthMethodConfigs()
        QgsDebugMsgLevel( QStringLiteral( "A config with same id %1 was already added, skipping from %2" ).arg( config.id(), storage->name() ), 2 );
      }
    }
  }
}

QgsAuthMethod *QgsAuthManager::configAuthMethod( const QString &authcfg )
{
  ensureInitialized();

  if ( isDisabled() )
    return nullptr;

  if ( !mConfigAuthMethods.contains( authcfg ) )
  {
    QgsDebugError( QStringLiteral( "No config auth method found in database for authcfg: %1" ).arg( authcfg ) );
    return nullptr;
  }

  QString authMethodKey = mConfigAuthMethods.value( authcfg );

  return authMethod( authMethodKey );
}

QString QgsAuthManager::configAuthMethodKey( const QString &authcfg ) const
{
  ensureInitialized();

  if ( isDisabled() )
    return QString();

  return mConfigAuthMethods.value( authcfg, QString() );
}


QStringList QgsAuthManager::authMethodsKeys( const QString &dataprovider )
{
  ensureInitialized();

  return authMethodsMap( dataprovider.toLower() ).keys();
}

QgsAuthMethod *QgsAuthManager::authMethod( const QString &authMethodKey )
{
  ensureInitialized();

  if ( !mAuthMethods.contains( authMethodKey ) )
  {
    QgsDebugError( QStringLiteral( "No auth method registered for auth method key: %1" ).arg( authMethodKey ) );
    return nullptr;
  }

  return mAuthMethods.value( authMethodKey );
}

const QgsAuthMethodMetadata *QgsAuthManager::authMethodMetadata( const QString &authMethodKey )
{
  ensureInitialized();

  if ( !mAuthMethods.contains( authMethodKey ) )
  {
    QgsDebugError( QStringLiteral( "No auth method registered for auth method key: %1" ).arg( authMethodKey ) );
    return nullptr;
  }

  return QgsAuthMethodRegistry::instance()->authMethodMetadata( authMethodKey );
}


QgsAuthMethodsMap QgsAuthManager::authMethodsMap( const QString &dataprovider )
{
  ensureInitialized();

  if ( dataprovider.isEmpty() )
  {
    return mAuthMethods;
  }

  QgsAuthMethodsMap filteredmap;
  QgsAuthMethodsMap::const_iterator i = mAuthMethods.constBegin();
  while ( i != mAuthMethods.constEnd() )
  {
    if ( i.value()
         && ( i.value()->supportedDataProviders().contains( QStringLiteral( "all" ) )
              || i.value()->supportedDataProviders().contains( dataprovider ) ) )
    {
      filteredmap.insert( i.key(), i.value() );
    }
    ++i;
  }
  return filteredmap;
}

#ifdef HAVE_GUI
QWidget *QgsAuthManager::authMethodEditWidget( const QString &authMethodKey, QWidget *parent )
{
  ensureInitialized();

  QgsAuthMethod *method = authMethod( authMethodKey );
  if ( method )
    return method->editWidget( parent );
  else
    return nullptr;
}
#endif

QgsAuthMethod::Expansions QgsAuthManager::supportedAuthMethodExpansions( const QString &authcfg )
{
  ensureInitialized();

  if ( isDisabled() )
    return QgsAuthMethod::Expansions();

  QgsAuthMethod *authmethod = configAuthMethod( authcfg );
  if ( authmethod )
  {
    return authmethod->supportedExpansions();
  }
  return QgsAuthMethod::Expansions();
}

bool QgsAuthManager::storeAuthenticationConfig( QgsAuthMethodConfig &config, bool overwrite )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( !setMasterPassword( true ) )
    return false;

  // don't need to validate id, since it has not be defined yet
  if ( !config.isValid() )
  {
    const char *err = QT_TR_NOOP( "Store config: FAILED because config is invalid" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  QString uid = config.id();
  bool passedinID = !uid.isEmpty();
  if ( uid.isEmpty() )
  {
    uid = uniqueConfigId();
  }
  else if ( configIds().contains( uid ) )
  {
    if ( !overwrite )
    {
      const char *err = QT_TR_NOOP( "Store config: FAILED because pre-defined config ID %1 is not unique" );
      QgsDebugError( err );
      emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
      return false;
    }
    locker.unlock();
    if ( ! removeAuthenticationConfig( uid ) )
    {
      const char *err = QT_TR_NOOP( "Store config: FAILED because pre-defined config ID %1 could not be removed" );
      QgsDebugError( err );
      emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
      return false;
    }
    locker.relock();
  }

  QString configstring = config.configString();
  if ( configstring.isEmpty() )
  {
    const char *err = QT_TR_NOOP( "Store config: FAILED because config string is empty" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::CreateConfiguration ) )
  {
    if ( defaultStorage->isEncrypted() )
    {
      configstring = QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), configstring );
    }

    // Make a copy to not alter the original config
    QgsAuthMethodConfig configCopy { config };
    configCopy.setId( uid );
    if ( !defaultStorage->storeMethodConfig( configCopy, configstring ) )
    {
      emit messageLog( tr( "Store config: FAILED to store config in default storage: %1" ).arg( defaultStorage->lastError() ), authManTag(), Qgis::MessageLevel::Warning );
      return false;
    }
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }

  // passed-in config should now be like as if it was just loaded from db
  if ( !passedinID )
    config.setId( uid );

  updateConfigAuthMethods();

  QgsDebugMsgLevel( QStringLiteral( "Store config SUCCESS for authcfg: %1" ).arg( uid ), 2 );
  return true;
}

bool QgsAuthManager::updateAuthenticationConfig( const QgsAuthMethodConfig &config )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( !setMasterPassword( true ) )
    return false;

  // validate id
  if ( !config.isValid( true ) )
  {
    const char *err = QT_TR_NOOP( "Update config: FAILED because config is invalid" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  QString configstring = config.configString();
  if ( configstring.isEmpty() )
  {
    const char *err = QT_TR_NOOP( "Update config: FAILED because config is empty" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  // Loop through all storages with capability ReadConfiguration and update the first one that has the config
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->methodConfigExists( config.id() ) )
    {
      if ( !storage->capabilities().testFlag( Qgis::AuthConfigurationStorageCapability::UpdateConfiguration ) )
      {
        emit messageLog( tr( "Update config: FAILED because storage %1 does not support updating" ).arg( storage->name( ) ), authManTag(), Qgis::MessageLevel::Warning );
        return false;
      }
      if ( storage->isEncrypted() )
      {
        configstring = QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), configstring );
      }
      if ( !storage->storeMethodConfig( config, configstring ) )
      {
        emit messageLog( tr( "Store config: FAILED to store config in the storage: %1" ).arg( storage->lastError() ), authManTag(), Qgis::MessageLevel::Critical );
        return false;
      }
      break;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }

  // should come before updating auth methods, in case user switched auth methods in config
  clearCachedConfig( config.id() );

  updateConfigAuthMethods();

  QgsDebugMsgLevel( QStringLiteral( "Update config SUCCESS for authcfg: %1" ).arg( config.id() ), 2 );

  return true;
}

bool QgsAuthManager::loadAuthenticationConfig( const QString &authcfg, QgsAuthMethodConfig &config, bool full )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  if ( full && !setMasterPassword( true ) )
    return false;

  QMutexLocker locker( mMutex.get() );

  // Loop through all storages with capability ReadConfiguration and get the config from the first one that has the config
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->methodConfigExists( authcfg ) )
    {
      QString payload;
      config = storage->loadMethodConfig( authcfg, payload, full );

      if ( ! config.isValid( true ) || ( full && payload.isEmpty() ) )
      {
        emit messageLog( tr( "Load config: FAILED to load config %1 from default storage: %2" ).arg( authcfg, storage->lastError() ), authManTag(), Qgis::MessageLevel::Critical );
        return false;
      }

      if ( full )
      {
        if ( storage->isEncrypted() )
        {
          payload = QgsAuthCrypto::decrypt( mMasterPass, masterPasswordCiv(), payload );
        }
        config.loadConfigString( payload );
      }

      QString authMethodKey = configAuthMethodKey( authcfg );
      QgsAuthMethod *authmethod = authMethod( authMethodKey );
      if ( authmethod )
      {
        authmethod->updateMethodConfig( config );
      }
      else
      {
        QgsDebugError( QStringLiteral( "Update of authcfg %1 FAILED for auth method %2" ).arg( authcfg, authMethodKey ) );
      }

      QgsDebugMsgLevel( QStringLiteral( "Load %1 config SUCCESS for authcfg: %2" ).arg( full ? "full" : "base", authcfg ), 2 );
      return true;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }
  else
  {
    emit messageLog( tr( "Load config: FAILED to load config %1 from any storage" ).arg( authcfg ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

bool QgsAuthManager::removeAuthenticationConfig( const QString &authcfg )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( isDisabled() )
    return false;

  if ( authcfg.isEmpty() )
    return false;

  // Loop through all storages with capability DeleteConfiguration and delete the first one that has the config
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::DeleteConfiguration ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->methodConfigExists( authcfg ) )
    {
      if ( !storage->removeMethodConfig( authcfg ) )
      {
        emit messageLog( tr( "Remove config: FAILED to remove config from the storage: %1" ).arg( storage->lastError() ), authManTag(), Qgis::MessageLevel::Critical );
        return false;
      }
      else
      {
        clearCachedConfig( authcfg );
        updateConfigAuthMethods();
        QgsDebugMsgLevel( QStringLiteral( "REMOVED config for authcfg: %1" ).arg( authcfg ), 2 );
        return true;
      }
      break;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }
  else
  {
    emit messageLog( tr( "Remove config: FAILED to remove config %1 from any storage" ).arg( authcfg ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;

}

bool QgsAuthManager::exportAuthenticationConfigsToXml( const QString &filename, const QStringList &authcfgs, const QString &password )
{
  ensureInitialized();

  if ( filename.isEmpty() )
    return false;

  QDomDocument document( QStringLiteral( "qgis_authentication" ) );
  QDomElement root = document.createElement( QStringLiteral( "qgis_authentication" ) );
  document.appendChild( root );

  QString civ;
  if ( !password.isEmpty() )
  {
    QString salt;
    QString hash;
    QgsAuthCrypto::passwordKeyHash( password, &salt, &hash, &civ );
    root.setAttribute( QStringLiteral( "salt" ), salt );
    root.setAttribute( QStringLiteral( "hash" ), hash );
    root.setAttribute( QStringLiteral( "civ" ), civ );
  }

  QDomElement configurations = document.createElement( QStringLiteral( "configurations" ) );
  for ( const QString &authcfg : authcfgs )
  {
    QgsAuthMethodConfig authMethodConfig;

    bool ok = loadAuthenticationConfig( authcfg, authMethodConfig, true );
    if ( ok )
    {
      authMethodConfig.writeXml( configurations, document );
    }
  }
  if ( !password.isEmpty() )
  {
    QString configurationsString;
    QTextStream ts( &configurationsString );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ts.setCodec( "UTF-8" );
#endif
    configurations.save( ts, 2 );
    root.appendChild( document.createTextNode( QgsAuthCrypto::encrypt( password, civ, configurationsString ) ) );
  }
  else
  {
    root.appendChild( configurations );
  }

  QFile file( filename );
  if ( !file.open( QFile::WriteOnly | QIODevice::Truncate ) )
    return false;

  QTextStream ts( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  ts.setCodec( "UTF-8" );
#endif
  document.save( ts, 2 );
  file.close();
  return true;
}

bool QgsAuthManager::importAuthenticationConfigsFromXml( const QString &filename, const QString &password, bool overwrite )
{
  ensureInitialized();

  QFile file( filename );
  if ( !file.open( QFile::ReadOnly ) )
  {
    return false;
  }

  QDomDocument document( QStringLiteral( "qgis_authentication" ) );
  if ( !document.setContent( &file ) )
  {
    file.close();
    return false;
  }
  file.close();

  QDomElement root = document.documentElement();
  if ( root.tagName() != QLatin1String( "qgis_authentication" ) )
  {
    return false;
  }

  QDomElement configurations;
  if ( root.hasAttribute( QStringLiteral( "salt" ) ) )
  {
    QString salt = root.attribute( QStringLiteral( "salt" ) );
    QString hash = root.attribute( QStringLiteral( "hash" ) );
    QString civ = root.attribute( QStringLiteral( "civ" ) );
    if ( !QgsAuthCrypto::verifyPasswordKeyHash( password, salt, hash ) )
      return false;

    document.setContent( QgsAuthCrypto::decrypt( password, civ, root.text() ) );
    configurations = document.firstChild().toElement();
  }
  else
  {
    configurations = root.firstChildElement( QStringLiteral( "configurations" ) );
  }

  QDomElement configuration = configurations.firstChildElement();
  while ( !configuration.isNull() )
  {
    QgsAuthMethodConfig authMethodConfig;
    authMethodConfig.readXml( configuration );
    storeAuthenticationConfig( authMethodConfig, overwrite );

    configuration = configuration.nextSiblingElement();
  }
  return true;
}

bool QgsAuthManager::removeAllAuthenticationConfigs()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( isDisabled() )
    return false;

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::DeleteConfiguration ) )
  {
    if ( defaultStorage->clearMethodConfigs() )
    {
      clearAllCachedConfigs();
      updateConfigAuthMethods();
      QgsDebugMsgLevel( QStringLiteral( "REMOVED all configs from the default storage" ), 2 );
      return true;
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "FAILED to remove all configs from the default storage" ), 2 );
      return false;
    }
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }
}


bool QgsAuthManager::backupAuthenticationDatabase( QString *backuppath )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );

  if ( sqliteDatabasePath().isEmpty() )
  {
    const char *err = QT_TR_NOOP( "The authentication database is not filesystem-based" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  if ( !QFile::exists( sqliteDatabasePath() ) )
  {
    const char *err = QT_TR_NOOP( "No authentication database found" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  // close any connection to current db
  Q_NOWARN_DEPRECATED_PUSH
  QSqlDatabase authConn = authDatabaseConnection();
  Q_NOWARN_DEPRECATED_POP
  if ( authConn.isValid() && authConn.isOpen() )
    authConn.close();

  // duplicate current db file to 'qgis-auth_YYYY-MM-DD-HHMMSS.db' backup
  QString datestamp( QDateTime::currentDateTime().toString( QStringLiteral( "yyyy-MM-dd-hhmmss" ) ) );
  QString dbbackup( sqliteDatabasePath() );
  dbbackup.replace( QLatin1String( ".db" ), QStringLiteral( "_%1.db" ).arg( datestamp ) );

  if ( !QFile::copy( sqliteDatabasePath(), dbbackup ) )
  {
    const char *err = QT_TR_NOOP( "Could not back up authentication database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  if ( backuppath )
    *backuppath = dbbackup;

  QgsDebugMsgLevel( QStringLiteral( "Backed up auth database at %1" ).arg( dbbackup ), 2 );
  return true;
}

bool QgsAuthManager::eraseAuthenticationDatabase( bool backup, QString *backuppath )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( isDisabled() )
    return false;

  QString dbbackup;
  if ( backup && !backupAuthenticationDatabase( &dbbackup ) )
  {
    emit messageLog( tr( "Failed to backup authentication database" ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  if ( backuppath && !dbbackup.isEmpty() )
    *backuppath = dbbackup;

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::ClearStorage ) )
  {
    if ( defaultStorage->erase() )
    {
      mMasterPass = QString();
      clearAllCachedConfigs();
      updateConfigAuthMethods();
      QgsDebugMsgLevel( QStringLiteral( "ERASED all configs" ), 2 );
      return true;
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "FAILED to erase all configs" ), 2 );
      return false;
    }
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }

#ifndef QT_NO_SSL
  initSslCaches();
#endif

  emit authDatabaseChanged();

  return true;
}

bool QgsAuthManager::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  QgsAuthMethod *authmethod = configAuthMethod( authcfg );
  if ( authmethod )
  {
    if ( !( authmethod->supportedExpansions() & QgsAuthMethod::NetworkRequest ) )
    {
      QgsDebugError( QStringLiteral( "Network request updating not supported by authcfg: %1" ).arg( authcfg ) );
      return true;
    }

    if ( !authmethod->updateNetworkRequest( request, authcfg, dataprovider.toLower() ) )
    {
      authmethod->clearCachedConfig( authcfg );
      return false;
    }
    return true;
  }
  return false;
}

bool QgsAuthManager::updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
    const QString &dataprovider )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  QgsAuthMethod *authmethod = configAuthMethod( authcfg );
  if ( authmethod )
  {
    if ( !( authmethod->supportedExpansions() & QgsAuthMethod::NetworkReply ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Network reply updating not supported by authcfg: %1" ).arg( authcfg ), 3 );
      return true;
    }

    if ( !authmethod->updateNetworkReply( reply, authcfg, dataprovider.toLower() ) )
    {
      authmethod->clearCachedConfig( authcfg );
      return false;
    }
    return true;
  }

  return false;
}

bool QgsAuthManager::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  QgsAuthMethod *authmethod = configAuthMethod( authcfg );
  if ( authmethod )
  {
    if ( !( authmethod->supportedExpansions() & QgsAuthMethod::DataSourceUri ) )
    {
      QgsDebugError( QStringLiteral( "Data source URI updating not supported by authcfg: %1" ).arg( authcfg ) );
      return true;
    }

    if ( !authmethod->updateDataSourceUriItems( connectionItems, authcfg, dataprovider.toLower() ) )
    {
      authmethod->clearCachedConfig( authcfg );
      return false;
    }
    return true;
  }

  return false;
}

bool QgsAuthManager::updateNetworkProxy( QNetworkProxy &proxy, const QString &authcfg, const QString &dataprovider )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  QgsAuthMethod *authmethod = configAuthMethod( authcfg );
  if ( authmethod )
  {
    if ( !( authmethod->supportedExpansions() & QgsAuthMethod::NetworkProxy ) )
    {
      QgsDebugError( QStringLiteral( "Proxy updating not supported by authcfg: %1" ).arg( authcfg ) );
      return true;
    }

    if ( !authmethod->updateNetworkProxy( proxy, authcfg, dataprovider.toLower() ) )
    {
      authmethod->clearCachedConfig( authcfg );
      return false;
    }
    QgsDebugMsgLevel( QStringLiteral( "Proxy updated successfully from authcfg: %1" ).arg( authcfg ), 2 );
    return true;
  }

  return false;
}

bool QgsAuthManager::storeAuthSetting( const QString &key, const QVariant &value, bool encrypt )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( key.isEmpty() )
    return false;

  QString storeval( value.toString() );
  if ( encrypt )
  {
    if ( !setMasterPassword( true ) )
    {
      return false;
    }
    else
    {
      storeval = QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), value.toString() );
    }
  }

  if ( existsAuthSetting( key ) && ! removeAuthSetting( key ) )
  {
    emit messageLog( tr( "Store setting: FAILED to remove pre-existing setting %1" ).arg( key ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  // Set the setting in the first storage that has the capability to store it

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::CreateSetting ) )
  {
    if ( !defaultStorage->storeAuthSetting( key, storeval ) )
    {
      emit messageLog( tr( "Store setting: FAILED to store setting in default storage" ), authManTag(), Qgis::MessageLevel::Warning );
      return false;
    }
    return true;
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }
}

QVariant QgsAuthManager::authSetting( const QString &key, const QVariant &defaultValue, bool decrypt )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( key.isEmpty() )
    return QVariant();

  if ( decrypt && !setMasterPassword( true ) )
    return QVariant();

  QVariant value = defaultValue;

  // Loop through all storages with capability ReadSetting and get the setting from the first one that has the setting
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadSetting ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    QString storeval = storage->loadAuthSetting( key );
    if ( !storeval.isEmpty() )
    {
      if ( decrypt )
      {
        storeval = QgsAuthCrypto::decrypt( mMasterPass, masterPasswordCiv(), storeval );
      }
      value = storeval;
      break;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return value;
}

bool QgsAuthManager::existsAuthSetting( const QString &key )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( key.isEmpty() )
    return false;

  // Loop through all storages with capability ReadSetting and get the setting from the first one that has the setting
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadSetting ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {

    if ( storage->authSettingExists( key ) )
    { return true; }

  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

bool QgsAuthManager::removeAuthSetting( const QString &key )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( key.isEmpty() )
    return false;

  // Loop through all storages with capability ReadSetting and delete from the first one that has the setting, fail if it has no capability
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadSetting ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->authSettingExists( key ) )
    {
      if ( storage->capabilities().testFlag( Qgis::AuthConfigurationStorageCapability::DeleteSetting ) )
      {
        if ( !storage->removeAuthSetting( key ) )
        {
          emit messageLog( tr( "Remove setting: FAILED to remove setting from storage: %1" ).arg( storage->lastError() ), authManTag(), Qgis::MessageLevel::Warning );
          return false;
        }
        return true;
      }
      else
      {
        emit messageLog( tr( "Remove setting: FAILED to remove setting from storage %1: storage is read only" ).arg( storage->name() ), authManTag(), Qgis::MessageLevel::Warning );
        return false;
      }
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }
  return false;
}

#ifndef QT_NO_SSL

////////////////// Certificate calls ///////////////////////

bool QgsAuthManager::initSslCaches()
{
  QgsScopedRuntimeProfile profile( "Initialize SSL cache" );

  QMutexLocker locker( mMutex.get() );
  bool res = true;
  res = res && rebuildCaCertsCache();
  res = res && rebuildCertTrustCache();
  res = res && rebuildTrustedCaCertsCache();
  res = res && rebuildIgnoredSslErrorCache();
  mCustomConfigByHostCache.clear();
  mHasCheckedIfCustomConfigByHostExists = false;

  if ( !res )
    QgsDebugError( QStringLiteral( "Init of SSL caches FAILED" ) );
  return res;
}

bool QgsAuthManager::storeCertIdentity( const QSslCertificate &cert, const QSslKey &key )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }
  if ( key.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed private key is null" ) );
    return false;
  }

  if ( !setMasterPassword( true ) )
    return false;

  QString id( QgsAuthCertUtils::shaHexForCert( cert ) );


  if ( existsCertIdentity( id ) && ! removeCertIdentity( id ) )
  {
    QgsDebugError( QStringLiteral( "Store certificate identity: FAILED to remove pre-existing certificate identity %1" ).arg( id ) );
    return false;
  }

  QString keypem( QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), key.toPem() ) );

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::CreateCertificateIdentity ) )
  {
    if ( !defaultStorage->storeCertIdentity( cert, keypem ) )
    {
      emit messageLog( tr( "Store certificate identity: FAILED to store certificate identity in default storage" ), authManTag(), Qgis::MessageLevel::Warning );
      return false;
    }
    return true;
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }
}

const QSslCertificate QgsAuthManager::certIdentity( const QString &id )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );

  QSslCertificate cert;

  if ( id.isEmpty() )
    return cert;

  // Loop through all storages with capability ReadCertificateIdentity and get the certificate from the first one that has the certificate
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    cert = storage->loadCertIdentity( id );
    if ( !cert.isNull() )
    {
      return cert;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return cert;
}

const QPair<QSslCertificate, QSslKey> QgsAuthManager::certIdentityBundle( const QString &id )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QPair<QSslCertificate, QSslKey> bundle;
  if ( id.isEmpty() )
    return bundle;

  if ( !setMasterPassword( true ) )
    return bundle;

  // Loop through all storages with capability ReadCertificateIdentity and get the certificate from the first one that has the certificate
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->certIdentityExists( id ) )
    {
      QPair<QSslCertificate, QString> encryptedBundle { storage->loadCertIdentityBundle( id ) };
      if ( encryptedBundle.first.isNull() )
      {
        QgsDebugError( QStringLiteral( "Certificate identity bundle is null for id: %1" ).arg( id ) );
        return bundle;
      }
      QSslKey key( QgsAuthCrypto::decrypt( mMasterPass, masterPasswordCiv(), encryptedBundle.second ).toLatin1(),
                   QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey );
      if ( key.isNull() )
      {
        QgsDebugError( QStringLiteral( "Certificate identity bundle: FAILED to create private key" ) );
        return bundle;
      }
      bundle = qMakePair( encryptedBundle.first, key );
      break;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return bundle;
  }

  return bundle;
}

const QStringList QgsAuthManager::certIdentityBundleToPem( const QString &id )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QPair<QSslCertificate, QSslKey> bundle( certIdentityBundle( id ) );
  if ( QgsAuthCertUtils::certIsViable( bundle.first ) && !bundle.second.isNull() )
  {
    return QStringList() << QString( bundle.first.toPem() ) << QString( bundle.second.toPem() );
  }
  return QStringList();
}

const QList<QSslCertificate> QgsAuthManager::certIdentities()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QList<QSslCertificate> certs;

  // Loop through all storages with capability ReadCertificateIdentity and collect the certificates from all storages
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    const QList<QSslCertificate> storageCerts = storage->certIdentities();
    // Add if not already in the list, warn otherwise
    for ( const QSslCertificate &cert : std::as_const( storageCerts ) )
    {
      if ( !certs.contains( cert ) )
      {
        certs.append( cert );
      }
      else
      {
        emit messageLog( tr( "Certificate already in the list: %1" ).arg( cert.issuerDisplayName() ), authManTag(), Qgis::MessageLevel::Warning );
      }
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return certs;
}

QStringList QgsAuthManager::certIdentityIds() const
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );

  if ( isDisabled() )
    return {};

  // Loop through all storages with capability ReadCertificateIdentity and collect the certificate ids from all storages
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity ) };

  QStringList ids;

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    const QStringList storageIds = storage->certIdentityIds();
    // Add if not already in the list, warn otherwise
    for ( const QString &id : std::as_const( storageIds ) )
    {
      if ( !ids.contains( id ) )
      {
        ids.append( id );
      }
      else
      {
        emit messageLog( tr( "Certificate identity id already in the list: %1" ).arg( id ), authManTag(), Qgis::MessageLevel::Warning );
      }
    }
  }

  return ids;
}

bool QgsAuthManager::existsCertIdentity( const QString &id )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( id.isEmpty() )
    return false;

  // Loop through all storages with capability ReadCertificateIdentity and check if the certificate exists in any storage
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->certIdentityExists( id ) )
    {
      return true;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

bool QgsAuthManager::removeCertIdentity( const QString &id )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( id.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed bundle ID is empty" ) );
    return false;
  }

  // Loop through all storages with capability ReadCertificateIdentity and delete from the first one that has the bundle, fail if it has no capability
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->certIdentityExists( id ) )
    {
      if ( !storage->removeCertIdentity( id ) )
      {
        emit messageLog( tr( "Remove certificate identity: FAILED to remove certificate identity from storage: %1" ).arg( storage->lastError() ), authManTag(), Qgis::MessageLevel::Warning );
        return false;
      }
      return true;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;

}

bool QgsAuthManager::storeSslCertCustomConfig( const QgsAuthConfigSslServer &config )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( config.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed config is null" ) );
    return false;
  }

  const QSslCertificate cert( config.sslCertificate() );
  const QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

  if ( existsSslCertCustomConfig( id, config.sslHostPort() ) && !removeSslCertCustomConfig( id, config.sslHostPort() ) )
  {
    QgsDebugError( QStringLiteral( "Store SSL certificate custom config: FAILED to remove pre-existing config %1" ).arg( id ) );
    return false;
  }

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::CreateSslCertificateCustomConfig ) )
  {
    if ( !defaultStorage->storeSslCertCustomConfig( config ) )
    {
      emit messageLog( tr( "Store SSL certificate custom config: FAILED to store config in default storage" ), authManTag(), Qgis::MessageLevel::Warning );
      return false;
    }
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }

  updateIgnoredSslErrorsCacheFromConfig( config );
  mCustomConfigByHostCache.clear();

  return true;
}

const QgsAuthConfigSslServer QgsAuthManager::sslCertCustomConfig( const QString &id, const QString &hostport )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QgsAuthConfigSslServer config;

  if ( id.isEmpty() || hostport.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed config ID or host:port is empty" ) );
    return config;
  }

  // Loop through all storages with capability ReadSslCertificateCustomConfig and get the config from the first one that has the config
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->sslCertCustomConfigExists( id, hostport ) )
    {
      config = storage->loadSslCertCustomConfig( id, hostport );
      if ( !config.isNull() )
      {
        return config;
      }
      else
      {
        emit messageLog( tr( "Could not load SSL custom config %1 %2 from the storage." ).arg( id, hostport ), authManTag(), Qgis::MessageLevel::Critical );
        return config;
      }
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return config;

}

const QgsAuthConfigSslServer QgsAuthManager::sslCertCustomConfigByHost( const QString &hostport )
{
  ensureInitialized();

  QgsAuthConfigSslServer config;
  if ( hostport.isEmpty() )
  {
    return config;
  }

  QMutexLocker locker( mMutex.get() );

  if ( mCustomConfigByHostCache.contains( hostport ) )
    return mCustomConfigByHostCache.value( hostport );

  // Loop through all storages with capability ReadSslCertificateCustomConfig and get the config from the first one that has the config
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    config = storage->loadSslCertCustomConfigByHost( hostport );
    if ( !config.isNull() )
    {
      mCustomConfigByHostCache.insert( hostport, config );
    }

  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return config;
}

const QList<QgsAuthConfigSslServer> QgsAuthManager::sslCertCustomConfigs()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QList<QgsAuthConfigSslServer> configs;

  // Loop through all storages with capability ReadSslCertificateCustomConfig
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig ) };

  QStringList ids;

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    const QList<QgsAuthConfigSslServer> storageConfigs = storage->sslCertCustomConfigs();
    // Check if id + hostPort is not already in the list, warn otherwise
    for ( const auto &config : std::as_const( storageConfigs ) )
    {
      const QString id( QgsAuthCertUtils::shaHexForCert( config.sslCertificate() ) );
      const QString hostPort = config.sslHostPort();
      const QString shaHostPort( QStringLiteral( "%1:%2" ).arg( id, hostPort ) );
      if ( ! ids.contains( shaHostPort ) )
      {
        ids.append( shaHostPort );
        configs.append( config );
      }
      else
      {
        emit messageLog( tr( "SSL custom config already in the list: %1" ).arg( hostPort ), authManTag(), Qgis::MessageLevel::Warning );
      }
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return configs;
}

bool QgsAuthManager::existsSslCertCustomConfig( const QString &id, const QString &hostPort )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( id.isEmpty() || hostPort.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed config ID or host:port is empty" ) );
    return false;
  }

  // Loop through all storages with capability ReadSslCertificateCustomConfig
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->sslCertCustomConfigExists( id, hostPort ) )
    {
      return true;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

bool QgsAuthManager::removeSslCertCustomConfig( const QString &id, const QString &hostport )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( id.isEmpty() || hostport.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed config ID or host:port is empty" ) );
    return false;
  }

  mCustomConfigByHostCache.clear();

  // Loop through all storages with capability DeleteSslCertificateCustomConfig
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::DeleteSslCertificateCustomConfig ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->sslCertCustomConfigExists( id, hostport ) )
    {
      if ( !storage->removeSslCertCustomConfig( id, hostport ) )
      {
        emit messageLog( tr( "FAILED to remove SSL cert custom config for host:port, id: %1, %2: %3" ).arg( hostport, id, storage->lastError() ), authManTag(), Qgis::MessageLevel::Warning );
        return false;
      }
      const QString shaHostPort( QStringLiteral( "%1:%2" ).arg( id, hostport ) );
      if ( mIgnoredSslErrorsCache.contains( shaHostPort ) )
      {
        mIgnoredSslErrorsCache.remove( shaHostPort );
      }
      return true;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}


void QgsAuthManager::dumpIgnoredSslErrorsCache_()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( !mIgnoredSslErrorsCache.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Ignored SSL errors cache items:" ), 1 );
    QHash<QString, QSet<QSslError::SslError> >::const_iterator i = mIgnoredSslErrorsCache.constBegin();
    while ( i != mIgnoredSslErrorsCache.constEnd() )
    {
      QStringList errs;
      for ( auto err : i.value() )
      {
        errs << QgsAuthCertUtils::sslErrorEnumString( err );
      }
      QgsDebugMsgLevel( QStringLiteral( "%1 = %2" ).arg( i.key(), errs.join( ", " ) ), 1 );
      ++i;
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Ignored SSL errors cache EMPTY" ), 2 );
  }
}

bool QgsAuthManager::updateIgnoredSslErrorsCacheFromConfig( const QgsAuthConfigSslServer &config )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( config.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed config is null" ) );
    return false;
  }

  QString shahostport( QStringLiteral( "%1:%2" )
                       .arg( QgsAuthCertUtils::shaHexForCert( config.sslCertificate() ).trimmed(),
                             config.sslHostPort().trimmed() ) );
  if ( mIgnoredSslErrorsCache.contains( shahostport ) )
  {
    mIgnoredSslErrorsCache.remove( shahostport );
  }
  const QList<QSslError::SslError> errenums( config.sslIgnoredErrorEnums() );
  if ( !errenums.isEmpty() )
  {
    mIgnoredSslErrorsCache.insert( shahostport, QSet<QSslError::SslError>( errenums.begin(), errenums.end() ) );
    QgsDebugMsgLevel( QStringLiteral( "Update of ignored SSL errors cache SUCCEEDED for sha:host:port = %1" ).arg( shahostport ), 2 );
    dumpIgnoredSslErrorsCache_();
    return true;
  }

  QgsDebugMsgLevel( QStringLiteral( "No ignored SSL errors to cache for sha:host:port = %1" ).arg( shahostport ), 2 );
  return true;
}

bool QgsAuthManager::updateIgnoredSslErrorsCache( const QString &shahostport, const QList<QSslError> &errors )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  const thread_local QRegularExpression rx( QRegularExpression::anchoredPattern( "\\S+:\\S+:\\d+" ) );
  if ( !rx.match( shahostport ).hasMatch() )
  {
    QgsDebugError( "Passed shahostport does not match \\S+:\\S+:\\d+, "
                   "e.g. 74a4ef5ea94512a43769b744cda0ca5049a72491:www.example.com:443" );
    return false;
  }

  if ( mIgnoredSslErrorsCache.contains( shahostport ) )
  {
    mIgnoredSslErrorsCache.remove( shahostport );
  }

  if ( errors.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed errors list empty" ) );
    return false;
  }

  QSet<QSslError::SslError> errs;
  for ( const auto &error : errors )
  {
    if ( error.error() == QSslError::NoError )
      continue;

    errs.insert( error.error() );
  }

  if ( errs.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed errors list does not contain errors" ) );
    return false;
  }

  mIgnoredSslErrorsCache.insert( shahostport, errs );

  QgsDebugMsgLevel( QStringLiteral( "Update of ignored SSL errors cache SUCCEEDED for sha:host:port = %1" ).arg( shahostport ), 2 );
  dumpIgnoredSslErrorsCache_();
  return true;
}

bool QgsAuthManager::rebuildIgnoredSslErrorCache()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QHash<QString, QSet<QSslError::SslError> > prevcache( mIgnoredSslErrorsCache );
  QHash<QString, QSet<QSslError::SslError> > nextcache;

  // Loop through all storages with capability ReadSslCertificateCustomConfig
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig ) };

  QStringList ids;

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    const auto customConfigs { storage->sslCertCustomConfigs() };
    for ( const auto &config : std::as_const( customConfigs ) )
    {
      const QString shaHostPort( QStringLiteral( "%1:%2" ).arg( QgsAuthCertUtils::shaHexForCert( config.sslCertificate() ), config.sslHostPort() ) );
      if ( ! ids.contains( shaHostPort ) )
      {
        ids.append( shaHostPort );
        if ( !config.sslIgnoredErrorEnums().isEmpty() )
        {
          nextcache.insert( shaHostPort, QSet<QSslError::SslError>( config.sslIgnoredErrorEnums().cbegin(), config.sslIgnoredErrorEnums().cend() ) );
        }
        if ( prevcache.contains( shaHostPort ) )
        {
          prevcache.remove( shaHostPort );
        }
      }
      else
      {
        emit messageLog( tr( "SSL custom config already in the list: %1" ).arg( config.sslHostPort() ), authManTag(), Qgis::MessageLevel::Warning );
      }
    }
  }

  if ( !prevcache.isEmpty() )
  {
    // preserve any existing per-session ignored errors for hosts
    QHash<QString, QSet<QSslError::SslError> >::const_iterator i = prevcache.constBegin();
    while ( i != prevcache.constEnd() )
    {
      nextcache.insert( i.key(), i.value() );
      ++i;
    }
  }

  if ( nextcache != mIgnoredSslErrorsCache )
  {
    mIgnoredSslErrorsCache.clear();
    mIgnoredSslErrorsCache = nextcache;
    QgsDebugMsgLevel( QStringLiteral( "Rebuild of ignored SSL errors cache SUCCEEDED" ), 2 );
    dumpIgnoredSslErrorsCache_();
    return true;
  }

  QgsDebugMsgLevel( QStringLiteral( "Rebuild of ignored SSL errors cache SAME AS BEFORE" ), 2 );
  dumpIgnoredSslErrorsCache_();
  return true;
}

bool QgsAuthManager::storeCertAuthorities( const QList<QSslCertificate> &certs )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( certs.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate list has no certs" ) );
    return false;
  }

  for ( const auto &cert : certs )
  {
    if ( !storeCertAuthority( cert ) )
      return false;
  }
  return true;
}

bool QgsAuthManager::storeCertAuthority( const QSslCertificate &cert )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  // don't refuse !cert.isValid() (actually just expired) CAs,
  // as user may want to ignore that SSL connection error
  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }

  if ( existsCertAuthority( cert ) && !removeCertAuthority( cert ) )
  {
    QgsDebugError( QStringLiteral( "Store certificate authority: FAILED to remove pre-existing certificate authority" ) );
    return false;
  }

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::CreateCertificateAuthority ) )
  {
    return defaultStorage->storeCertAuthority( cert );
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }

  return false;
}

const QSslCertificate QgsAuthManager::certAuthority( const QString &id )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QSslCertificate emptycert;
  QSslCertificate cert;
  if ( id.isEmpty() )
    return emptycert;

  // Loop through all storages with capability ReadCertificateAuthority and get the certificate from the first one that has the certificate
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    cert = storage->loadCertAuthority( id );
    if ( !cert.isNull() )
    {
      return cert;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return emptycert;
  }

  return cert;
}

bool QgsAuthManager::existsCertAuthority( const QSslCertificate &cert )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }

  // Loop through all storages with capability ReadCertificateAuthority and get the certificate from the first one that has the certificate
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->certAuthorityExists( cert ) )
    {
      return true;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

bool QgsAuthManager::removeCertAuthority( const QSslCertificate &cert )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }

  // Loop through all storages with capability ReadCertificateAuthority and delete from the first one that has the certificate, fail if it has no capability
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->certAuthorityExists( cert ) )
    {

      if ( !storage->capabilities().testFlag( Qgis::AuthConfigurationStorageCapability::DeleteCertificateAuthority ) )
      {
        emit messageLog( tr( "Remove certificate: FAILED to remove setting from storage %1: storage is read only" ).arg( storage->name() ), authManTag(), Qgis::MessageLevel::Warning );
        return false;
      }

      if ( !storage->removeCertAuthority( cert ) )
      {
        emit messageLog( tr( "Remove certificate authority: FAILED to remove certificate authority from storage: %1" ).arg( storage->lastError() ), authManTag(), Qgis::MessageLevel::Warning );
        return false;
      }
      return true;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

const QList<QSslCertificate> QgsAuthManager::systemRootCAs()
{
  return QSslConfiguration::systemCaCertificates();
}

const QList<QSslCertificate> QgsAuthManager::extraFileCAs()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QList<QSslCertificate> certs;
  QList<QSslCertificate> filecerts;
  QVariant cafileval = QgsAuthManager::instance()->authSetting( QStringLiteral( "cafile" ) );
  if ( QgsVariantUtils::isNull( cafileval ) )
    return certs;

  QVariant allowinvalid = QgsAuthManager::instance()->authSetting( QStringLiteral( "cafileallowinvalid" ), QVariant( false ) );
  if ( QgsVariantUtils::isNull( allowinvalid ) )
    return certs;

  QString cafile( cafileval.toString() );
  if ( !cafile.isEmpty() && QFile::exists( cafile ) )
  {
    filecerts = QgsAuthCertUtils::certsFromFile( cafile );
  }
  // only CAs or certs capable of signing other certs are allowed
  for ( const auto &cert : std::as_const( filecerts ) )
  {
    if ( !allowinvalid.toBool() && ( cert.isBlacklisted()
                                     || cert.isNull()
                                     || cert.expiryDate() <= QDateTime::currentDateTime()
                                     || cert.effectiveDate() > QDateTime::currentDateTime() ) )
    {
      continue;
    }

    if ( QgsAuthCertUtils::certificateIsAuthorityOrIssuer( cert ) )
    {
      certs << cert;
    }
  }
  return certs;
}

const QList<QSslCertificate> QgsAuthManager::databaseCAs()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );

  // Loop through all storages with capability ReadCertificateAuthority and collect the certificates from all storages
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority ) };

  QList<QSslCertificate> certs;

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    const QList<QSslCertificate> storageCerts = storage->caCerts();
    // Add if not already in the list, warn otherwise
    for ( const QSslCertificate &cert : std::as_const( storageCerts ) )
    {
      if ( !certs.contains( cert ) )
      {
        certs.append( cert );
      }
      else
      {
        emit messageLog( tr( "Certificate already in the list: %1" ).arg( cert.issuerDisplayName() ), authManTag(), Qgis::MessageLevel::Warning );
      }
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return certs;
}

const QMap<QString, QSslCertificate> QgsAuthManager::mappedDatabaseCAs()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  return QgsAuthCertUtils::mapDigestToCerts( databaseCAs() );
}

bool QgsAuthManager::rebuildCaCertsCache()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  mCaCertsCache.clear();
  // in reverse order of precedence, with regards to duplicates, so QMap inserts overwrite
  insertCaCertInCache( QgsAuthCertUtils::SystemRoot, systemRootCAs() );
  insertCaCertInCache( QgsAuthCertUtils::FromFile, extraFileCAs() );
  insertCaCertInCache( QgsAuthCertUtils::InDatabase, databaseCAs() );

  bool res = !mCaCertsCache.isEmpty(); // should at least contain system root CAs
  if ( !res )
    QgsDebugError( QStringLiteral( "Rebuild of CA certs cache FAILED" ) );
  return res;
}

bool QgsAuthManager::storeCertTrustPolicy( const QSslCertificate &cert, QgsAuthCertUtils::CertTrustPolicy policy )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null." ) );
    return false;
  }

  if ( certTrustPolicy( cert ) == policy )
  {
    return true;
  }

  if ( certificateTrustPolicy( cert ) != QgsAuthCertUtils::DefaultTrust && ! removeCertTrustPolicy( cert ) )
  {
    emit messageLog( tr( "Could not delete pre-existing certificate trust policy." ), authManTag(), Qgis::MessageLevel::Warning );
    return false;
  }

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::CreateCertificateTrustPolicy ) )
  {
    return defaultStorage->storeCertTrustPolicy( cert, policy );
  }
  else
  {
    emit messageLog( tr( "Could not connect to any authentication configuration storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }
}

QgsAuthCertUtils::CertTrustPolicy QgsAuthManager::certTrustPolicy( const QSslCertificate &cert )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return QgsAuthCertUtils::DefaultTrust;
  }

  // Loop through all storages with capability ReadCertificateTrustPolicy and get the policy from the first one that has the policy
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateTrustPolicy ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    QgsAuthCertUtils::CertTrustPolicy policy = storage->loadCertTrustPolicy( cert );
    if ( policy != QgsAuthCertUtils::DefaultTrust )
    {
      return policy;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any credentials storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return QgsAuthCertUtils::DefaultTrust;
}

bool QgsAuthManager::removeCertTrustPolicies( const QList<QSslCertificate> &certs )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( certs.empty() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate list has no certs" ) );
    return false;
  }

  for ( const auto &cert : certs )
  {
    if ( !removeCertTrustPolicy( cert ) )
      return false;
  }
  return true;
}

bool QgsAuthManager::removeCertTrustPolicy( const QSslCertificate &cert )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }

  // Loop through all storages with capability ReadCertificateTrustPolicy and delete from the first one that has the policy, fail if it has no capability
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateTrustPolicy ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( storage->certTrustPolicyExists( cert ) )
    {
      if ( !storage->capabilities().testFlag( Qgis::AuthConfigurationStorageCapability::DeleteCertificateTrustPolicy ) )
      {
        emit messageLog( tr( "Remove certificate trust policy: FAILED to remove setting from storage %1: storage is read only" ).arg( storage->name() ), authManTag(), Qgis::MessageLevel::Warning );
        return false;
      }

      if ( !storage->removeCertTrustPolicy( cert ) )
      {
        emit messageLog( tr( "Remove certificate trust policy: FAILED to remove certificate trust policy from storage: %1" ).arg( storage->lastError() ), authManTag(), Qgis::MessageLevel::Warning );
        return false;
      }
      return true;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any authentication configuration storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

QgsAuthCertUtils::CertTrustPolicy QgsAuthManager::certificateTrustPolicy( const QSslCertificate &cert )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( cert.isNull() )
  {
    return QgsAuthCertUtils::NoPolicy;
  }

  QString id( QgsAuthCertUtils::shaHexForCert( cert ) );
  const QStringList &trustedids = mCertTrustCache.value( QgsAuthCertUtils::Trusted );
  const QStringList &untrustedids = mCertTrustCache.value( QgsAuthCertUtils::Untrusted );

  QgsAuthCertUtils::CertTrustPolicy policy( QgsAuthCertUtils::DefaultTrust );
  if ( trustedids.contains( id ) )
  {
    policy = QgsAuthCertUtils::Trusted;
  }
  else if ( untrustedids.contains( id ) )
  {
    policy = QgsAuthCertUtils::Untrusted;
  }
  return policy;
}

bool QgsAuthManager::setDefaultCertTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy )
{
  ensureInitialized();

  if ( policy == QgsAuthCertUtils::DefaultTrust )
  {
    // set default trust policy to Trusted by removing setting
    return removeAuthSetting( QStringLiteral( "certdefaulttrust" ) );
  }
  return storeAuthSetting( QStringLiteral( "certdefaulttrust" ), static_cast< int >( policy ) );
}

QgsAuthCertUtils::CertTrustPolicy QgsAuthManager::defaultCertTrustPolicy()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QVariant policy( authSetting( QStringLiteral( "certdefaulttrust" ) ) );
  if ( QgsVariantUtils::isNull( policy ) )
  {
    return QgsAuthCertUtils::Trusted;
  }
  return static_cast< QgsAuthCertUtils::CertTrustPolicy >( policy.toInt() );
}

bool QgsAuthManager::rebuildCertTrustCache()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  mCertTrustCache.clear();

  // Loop through all storages with capability ReadCertificateTrustPolicy
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateTrustPolicy ) };

  QStringList ids;

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {

    const auto trustedCerts { storage->caCertsPolicy() };
    for ( auto it = trustedCerts.cbegin(); it != trustedCerts.cend(); ++it )
    {
      const QString id { it.key( )};
      if ( ! ids.contains( id ) )
      {
        ids.append( id );
        const QgsAuthCertUtils::CertTrustPolicy policy( it.value() );
        if ( policy == QgsAuthCertUtils::CertTrustPolicy::Trusted )
        {
          QStringList ids;
          if ( mCertTrustCache.contains( QgsAuthCertUtils::Trusted ) )
          {
            ids = mCertTrustCache.value( QgsAuthCertUtils::Trusted );
          }
          mCertTrustCache.insert( QgsAuthCertUtils::Trusted, ids << it.key() );
        }
      }
      else
      {
        emit messageLog( tr( "Certificate already in the list: %1" ).arg( it.key() ), authManTag(), Qgis::MessageLevel::Warning );
      }
    }
  }

  if ( ! storages.empty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Rebuild of cert trust policy cache SUCCEEDED" ), 2 );
    return true;
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }
}

const QList<QSslCertificate> QgsAuthManager::trustedCaCerts( bool includeinvalid )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  QgsAuthCertUtils::CertTrustPolicy defaultpolicy( defaultCertTrustPolicy() );
  QStringList trustedids = mCertTrustCache.value( QgsAuthCertUtils::Trusted );
  QStringList untrustedids = mCertTrustCache.value( QgsAuthCertUtils::Untrusted );
  const QList<QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate> > &certpairs( mCaCertsCache.values() );

  QList<QSslCertificate> trustedcerts;
  for ( int i = 0; i < certpairs.size(); ++i )
  {
    QSslCertificate cert( certpairs.at( i ).second );
    QString certid( QgsAuthCertUtils::shaHexForCert( cert ) );
    if ( trustedids.contains( certid ) )
    {
      // trusted certs are always added regardless of their validity
      trustedcerts.append( cert );
    }
    else if ( defaultpolicy == QgsAuthCertUtils::Trusted && !untrustedids.contains( certid ) )
    {
      if ( !includeinvalid && !QgsAuthCertUtils::certIsViable( cert ) )
        continue;
      trustedcerts.append( cert );
    }
  }

  // update application default SSL config for new requests
  QSslConfiguration sslconfig( QSslConfiguration::defaultConfiguration() );
  sslconfig.setCaCertificates( trustedcerts );
  QSslConfiguration::setDefaultConfiguration( sslconfig );

  return trustedcerts;
}

const QList<QSslCertificate> QgsAuthManager::untrustedCaCerts( QList<QSslCertificate> trustedCAs )
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( trustedCAs.isEmpty() )
  {
    if ( mTrustedCaCertsCache.isEmpty() )
    {
      rebuildTrustedCaCertsCache();
    }
    trustedCAs = trustedCaCertsCache();
  }

  const QList<QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate> > &certpairs( mCaCertsCache.values() );

  QList<QSslCertificate> untrustedCAs;
  for ( int i = 0; i < certpairs.size(); ++i )
  {
    QSslCertificate cert( certpairs.at( i ).second );
    if ( !trustedCAs.contains( cert ) )
    {
      untrustedCAs.append( cert );
    }
  }
  return untrustedCAs;
}

bool QgsAuthManager::rebuildTrustedCaCertsCache()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  mTrustedCaCertsCache = trustedCaCerts();
  QgsDebugMsgLevel( QStringLiteral( "Rebuilt trusted cert authorities cache" ), 2 );
  // TODO: add some error trapping for the operation
  return true;
}

const QByteArray QgsAuthManager::trustedCaCertsPemText()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  return QgsAuthCertUtils::certsToPemText( trustedCaCertsCache() );
}

bool QgsAuthManager::passwordHelperSync()
{
  ensureInitialized();

  QMutexLocker locker( mMutex.get() );
  if ( masterPasswordIsSet() )
  {
    return passwordHelperWrite( mMasterPass );
  }
  return false;
}


////////////////// Certificate calls - end ///////////////////////

#endif

void QgsAuthManager::clearAllCachedConfigs()
{
  ensureInitialized();

  if ( isDisabled() )
    return;

  const QStringList ids = configIds();
  for ( const auto &authcfg : ids )
  {
    clearCachedConfig( authcfg );
  }
}

void QgsAuthManager::clearCachedConfig( const QString &authcfg )
{
  ensureInitialized();

  if ( isDisabled() )
    return;

  QgsAuthMethod *authmethod = configAuthMethod( authcfg );
  if ( authmethod )
  {
    authmethod->clearCachedConfig( authcfg );
  }
}

void QgsAuthManager::writeToConsole( const QString &message,
                                     const QString &tag,
                                     Qgis::MessageLevel level )
{
  Q_UNUSED( tag )

  ensureInitialized();

  // only output WARNING and CRITICAL messages
  if ( level == Qgis::MessageLevel::Info )
    return;

  QString msg;
  switch ( level )
  {
    case Qgis::MessageLevel::Warning:
      msg += QLatin1String( "WARNING: " );
      break;
    case Qgis::MessageLevel::Critical:
      msg += QLatin1String( "ERROR: " );
      break;
    default:
      break;
  }
  msg += message;

  QTextStream out( stdout, QIODevice::WriteOnly );
  out << msg << Qt::endl;
}

void QgsAuthManager::tryToStartDbErase()
{
  ensureInitialized();

  ++mScheduledDbEraseRequestCount;
  // wait a total of 90 seconds for GUI availiability or user interaction, then cancel schedule
  int trycutoff = 90 / ( mScheduledDbEraseRequestWait ? mScheduledDbEraseRequestWait : 3 );
  if ( mScheduledDbEraseRequestCount >= trycutoff )
  {
    setScheduledAuthDatabaseErase( false );
    QgsDebugMsgLevel( QStringLiteral( "authDatabaseEraseRequest emitting/scheduling canceled" ), 2 );
    return;
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "authDatabaseEraseRequest attempt (%1 of %2)" )
                      .arg( mScheduledDbEraseRequestCount ).arg( trycutoff ), 2 );
  }

  if ( scheduledAuthDatabaseErase() && !mScheduledDbEraseRequestEmitted && mMutex->tryLock() )
  {
    // see note in header about this signal's use
    mScheduledDbEraseRequestEmitted = true;
    emit authDatabaseEraseRequested();

    mMutex->unlock();

    QgsDebugMsgLevel( QStringLiteral( "authDatabaseEraseRequest emitted" ), 2 );
    return;
  }
  QgsDebugMsgLevel( QStringLiteral( "authDatabaseEraseRequest emit skipped" ), 2 );
}


QgsAuthManager::~QgsAuthManager()
{
  QMutexLocker locker( mMutex.get() );

  QMapIterator<QThread *, QMetaObject::Connection> iterator( mConnectedThreads );
  while ( iterator.hasNext() )
  {
    iterator.next();
    QThread::disconnect( iterator.value() );
  }

  if ( !mAuthInit )
    return;

  locker.unlock();

  if ( !isDisabled() )
  {
    delete QgsAuthMethodRegistry::instance();
    qDeleteAll( mAuthMethods );

    Q_NOWARN_DEPRECATED_PUSH
    QSqlDatabase authConn = authDatabaseConnection();
    Q_NOWARN_DEPRECATED_POP
    if ( authConn.isValid() && authConn.isOpen() )
      authConn.close();
  }
  delete mScheduledDbEraseTimer;
  mScheduledDbEraseTimer = nullptr;
  QSqlDatabase::removeDatabase( QStringLiteral( "authentication.configs" ) );
}

QgsAuthConfigurationStorageRegistry *QgsAuthManager::authConfigurationStorageRegistry() const
{
  QMutexLocker locker( mMutex.get() );
  if ( ! mAuthConfigurationStorageRegistry )
  {
    mAuthConfigurationStorageRegistry = std::make_unique<QgsAuthConfigurationStorageRegistry>();
  }
  return mAuthConfigurationStorageRegistry.get();
}


QString QgsAuthManager::passwordHelperName() const
{
  return tr( "Password Helper" );
}


void QgsAuthManager::passwordHelperLog( const QString &msg ) const
{
  ensureInitialized();

  if ( passwordHelperLoggingEnabled() )
  {
    QgsMessageLog::logMessage( msg, passwordHelperName() );
  }
}

bool QgsAuthManager::passwordHelperDelete()
{
  ensureInitialized();

  passwordHelperLog( tr( "Opening %1 for DELETE…" ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ) );
  bool result;
  QKeychain::DeletePasswordJob job( AUTH_PASSWORD_HELPER_FOLDER_NAME );
  QgsSettings settings;
  job.setInsecureFallback( settings.value( QStringLiteral( "password_helper_insecure_fallback" ), false, QgsSettings::Section::Auth ).toBool() );
  job.setAutoDelete( false );
  job.setKey( authPasswordHelperKeyName() );
  QEventLoop loop;
  connect( &job, &QKeychain::Job::finished, &loop, &QEventLoop::quit );
  job.start();
  loop.exec();
  if ( job.error() )
  {
    mPasswordHelperErrorCode = job.error();
    mPasswordHelperErrorMessage = tr( "Delete password failed: %1." ).arg( job.errorString() );
    // Signals used in the tests to exit main application loop
    emit passwordHelperFailure();
    result = false;
  }
  else
  {
    // Signals used in the tests to exit main application loop
    emit passwordHelperSuccess();
    result = true;
  }
  passwordHelperProcessError();
  return result;
}

QString QgsAuthManager::passwordHelperRead()
{
  ensureInitialized();

  // Retrieve it!
  QString password;
  passwordHelperLog( tr( "Opening %1 for READ…" ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ) );
  QKeychain::ReadPasswordJob job( AUTH_PASSWORD_HELPER_FOLDER_NAME );
  QgsSettings settings;
  job.setInsecureFallback( settings.value( QStringLiteral( "password_helper_insecure_fallback" ), false, QgsSettings::Section::Auth ).toBool() );
  job.setAutoDelete( false );
  job.setKey( authPasswordHelperKeyName() );
  QEventLoop loop;
  connect( &job, &QKeychain::Job::finished, &loop, &QEventLoop::quit );
  job.start();
  loop.exec();
  if ( job.error() )
  {
    mPasswordHelperErrorCode = job.error();
    mPasswordHelperErrorMessage = tr( "Retrieving password from your %1 failed: %2." ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME, job.errorString() );
    // Signals used in the tests to exit main application loop
    emit passwordHelperFailure();
  }
  else
  {
    password = job.textData();
    // Password is there but it is empty, treat it like if it was not found
    if ( password.isEmpty() )
    {
      mPasswordHelperErrorCode = QKeychain::EntryNotFound;
      mPasswordHelperErrorMessage = tr( "Empty password retrieved from your %1." ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME );
      // Signals used in the tests to exit main application loop
      emit passwordHelperFailure();
    }
    else
    {
      // Signals used in the tests to exit main application loop
      emit passwordHelperSuccess();
    }
  }
  passwordHelperProcessError();
  return password;
}

bool QgsAuthManager::passwordHelperWrite( const QString &password )
{
  ensureInitialized();

  Q_ASSERT( !password.isEmpty() );
  bool result;
  passwordHelperLog( tr( "Opening %1 for WRITE…" ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ) );
  QKeychain::WritePasswordJob job( AUTH_PASSWORD_HELPER_FOLDER_NAME );
  QgsSettings settings;
  job.setInsecureFallback( settings.value( QStringLiteral( "password_helper_insecure_fallback" ), false, QgsSettings::Section::Auth ).toBool() );
  job.setAutoDelete( false );
  job.setKey( authPasswordHelperKeyName() );
  job.setTextData( password );
  QEventLoop loop;
  connect( &job, &QKeychain::Job::finished, &loop, &QEventLoop::quit );
  job.start();
  loop.exec();
  if ( job.error() )
  {
    mPasswordHelperErrorCode = job.error();
    mPasswordHelperErrorMessage = tr( "Storing password in your %1 failed: %2." ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME, job.errorString() );
    // Signals used in the tests to exit main application loop
    emit passwordHelperFailure();
    result = false;
  }
  else
  {
    passwordHelperClearErrors();
    // Signals used in the tests to exit main application loop
    emit passwordHelperSuccess();
    result = true;
  }
  passwordHelperProcessError();
  return result;
}

bool QgsAuthManager::passwordHelperEnabled()
{
  // Does the user want to store the password in the wallet?
  QgsSettings settings;
  return settings.value( QStringLiteral( "use_password_helper" ), true, QgsSettings::Section::Auth ).toBool();
}

void QgsAuthManager::setPasswordHelperEnabled( const bool enabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "use_password_helper" ),  enabled, QgsSettings::Section::Auth );
  emit messageLog( enabled ? tr( "Your %1 will be <b>used from now</b> on to store and retrieve the master password." )
                   .arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ) :
                   tr( "Your %1 will <b>not be used anymore</b> to store and retrieve the master password." )
                   .arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ) );
}

bool QgsAuthManager::passwordHelperLoggingEnabled()
{
  // Does the user want to store the password in the wallet?
  QgsSettings settings;
  return settings.value( QStringLiteral( "password_helper_logging" ), false, QgsSettings::Section::Auth ).toBool();
}

void QgsAuthManager::setPasswordHelperLoggingEnabled( const bool enabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "password_helper_logging" ),  enabled, QgsSettings::Section::Auth );
}

void QgsAuthManager::passwordHelperClearErrors()
{
  mPasswordHelperErrorCode = QKeychain::NoError;
  mPasswordHelperErrorMessage.clear();
}

void QgsAuthManager::passwordHelperProcessError()
{
  ensureInitialized();

  if ( mPasswordHelperErrorCode == QKeychain::AccessDenied ||
       mPasswordHelperErrorCode == QKeychain::AccessDeniedByUser ||
       mPasswordHelperErrorCode == QKeychain::NoBackendAvailable ||
       mPasswordHelperErrorCode == QKeychain::NotImplemented )
  {
    // If the error is permanent or the user denied access to the wallet
    // we also want to disable the wallet system to prevent annoying
    // notification on each subsequent access.
    setPasswordHelperEnabled( false );
    mPasswordHelperErrorMessage = tr( "There was an error and integration with your %1 system has been disabled. "
                                      "You can re-enable it at any time through the \"Utilities\" menu "
                                      "in the Authentication pane of the options dialog. %2" )
                                  .arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME, mPasswordHelperErrorMessage );
  }
  if ( mPasswordHelperErrorCode != QKeychain::NoError )
  {
    // We've got an error from the wallet
    passwordHelperLog( tr( "Error in %1: %2" ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME, mPasswordHelperErrorMessage ) );
    emit passwordHelperMessageLog( mPasswordHelperErrorMessage, authManTag(), Qgis::MessageLevel::Critical );
  }
  passwordHelperClearErrors();
}


bool QgsAuthManager::masterPasswordInput()
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  QString pass;
  bool storedPasswordIsValid = false;
  bool ok = false;

  // Read the password from the wallet
  if ( passwordHelperEnabled() )
  {
    pass = passwordHelperRead();
    if ( ! pass.isEmpty() && ( mPasswordHelperErrorCode == QKeychain::NoError ) )
    {
      // Let's check the password!
      if ( verifyMasterPassword( pass ) )
      {
        ok = true;
        storedPasswordIsValid = true;
        emit passwordHelperMessageLog( tr( "Master password has been successfully read from your %1" ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ), authManTag(), Qgis::MessageLevel::Info );
      }
      else
      {
        emit passwordHelperMessageLog( tr( "Master password stored in your %1 is not valid" ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ), authManTag(), Qgis::MessageLevel::Warning );
      }
    }
  }

  if ( ! ok )
  {
    pass.clear();
    ok = QgsCredentials::instance()->getMasterPassword( pass, masterPasswordHashInDatabase() );
  }

  if ( ok && !pass.isEmpty() && mMasterPass != pass )
  {
    mMasterPass = pass;
    if ( passwordHelperEnabled() && ! storedPasswordIsValid )
    {
      if ( passwordHelperWrite( pass ) )
      {
        emit passwordHelperMessageLog( tr( "Master password has been successfully written to your %1" ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ), authManTag(), Qgis::MessageLevel::Info );
      }
      else
      {
        emit passwordHelperMessageLog( tr( "Master password could not be written to your %1" ).arg( AUTH_PASSWORD_HELPER_DISPLAY_NAME ), authManTag(), Qgis::MessageLevel::Warning );
      }
    }
    return true;
  }
  return false;
}

bool QgsAuthManager::masterPasswordRowsInDb( int *rows ) const
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  *rows = 0;

  QMutexLocker locker( mMutex.get() );

  // Loop through all storages with capability ReadMasterPassword and count the number of master passwords
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadMasterPassword ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    try
    {
      *rows += storage->masterPasswords( ).count();
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any authentication configuration storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return rows != 0;

}

bool QgsAuthManager::masterPasswordHashInDatabase() const
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  int rows = 0;
  if ( !masterPasswordRowsInDb( &rows ) )
  {
    const char *err = QT_TR_NOOP( "Master password: FAILED to access database" );
    QgsDebugError( err );
    emit messageLog( tr( err ), authManTag(), Qgis::MessageLevel::Critical );

    return false;
  }
  return ( rows == 1 );
}

bool QgsAuthManager::masterPasswordCheckAgainstDb( const QString &compare ) const
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  // Only check the default DB
  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::ReadMasterPassword ) )
  {
    try
    {
      const QList<QgsAuthConfigurationStorage::MasterPasswordConfig> passwords { defaultStorage->masterPasswords( ) };
      if ( passwords.size() == 0 )
      {
        emit messageLog( tr( "Master password: FAILED to access database" ), authManTag(), Qgis::MessageLevel::Critical );
        return false;
      }
      const QgsAuthConfigurationStorage::MasterPasswordConfig storedPassword { passwords.first() };
      return QgsAuthCrypto::verifyPasswordKeyHash( compare.isNull() ? mMasterPass : compare, storedPassword.salt, storedPassword.hash );
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
      return false;
    }

  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }
}

bool QgsAuthManager::masterPasswordStoreInDb() const
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  QString salt, hash, civ;
  QgsAuthCrypto::passwordKeyHash( mMasterPass, &salt, &hash, &civ );

  // Only store in the default DB
  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::CreateMasterPassword ) )
  {
    try
    {
      return defaultStorage->storeMasterPassword( { salt, civ, hash } );
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
      return false;
    }
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }
}

bool QgsAuthManager::masterPasswordClearDb()
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::DeleteMasterPassword ) )
  {

    try
    {
      return defaultStorage->clearMasterPasswords();
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
      return false;
    }

  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }
}

const QString QgsAuthManager::masterPasswordCiv() const
{
  ensureInitialized();

  if ( isDisabled() )
    return QString();

  if ( QgsAuthConfigurationStorage *defaultStorage = firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability::ReadMasterPassword ) )
  {
    try
    {
      const QList<QgsAuthConfigurationStorage::MasterPasswordConfig> passwords { defaultStorage->masterPasswords( ) };
      if ( passwords.size() == 0 )
      {
        emit messageLog( tr( "Master password: FAILED to access database" ), authManTag(), Qgis::MessageLevel::Critical );
        return  QString();
      }
      return passwords.first().civ;
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
      return QString();
    }
  }
  else
  {
    emit messageLog( tr( "Could not connect to the default storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return QString();
  }
}

QStringList QgsAuthManager::configIds() const
{
  ensureInitialized();

  QStringList configKeys = QStringList();

  if ( isDisabled() )
    return configKeys;

  // Loop through all storages with capability ReadConfiguration and get the config ids
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    try
    {
      const QgsAuthMethodConfigsMap configs = storage->authMethodConfigs();
      // Check if the config ids are already in the list
      for ( auto it = configs.cbegin(); it != configs.cend(); ++it )
      {
        if ( !configKeys.contains( it.key() ) )
        {
          configKeys.append( it.key() );
        }
        else
        {
          emit messageLog( tr( "Config id %1 is already in the list" ).arg( it.key() ), authManTag(), Qgis::MessageLevel::Warning );
        }
      }
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
    }
  }

  return configKeys;
}

bool QgsAuthManager::verifyPasswordCanDecryptConfigs() const
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  // no need to check for setMasterPassword, since this is private and it will be set

  // Loop through all storages with capability ReadConfiguration and check if the password can decrypt the configs
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration ) };

  for ( const QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {

    if ( ! storage->isEncrypted() )
    {
      continue;
    }

    try
    {
      const QgsAuthMethodConfigsMap configs = storage->authMethodConfigsWithPayload();
      for ( auto it = configs.cbegin(); it != configs.cend(); ++it )
      {
        QString configstring( QgsAuthCrypto::decrypt( mMasterPass, masterPasswordCiv(), it.value().config( QStringLiteral( "encrypted_payload" ) ) ) );
        if ( configstring.isEmpty() )
        {
          QgsDebugError( QStringLiteral( "Verify password can decrypt configs FAILED, could not decrypt a config (id: %1) from storage %2" )
                         .arg( it.key(), storage->name() ) );
          return false;
        }
      }
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
      return false;
    }

  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any authentication configuration storage." ), authManTag(), Qgis::MessageLevel::Critical );
    return false;
  }

  return true;
}

bool QgsAuthManager::reencryptAllAuthenticationConfigs( const QString &prevpass, const QString &prevciv )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  bool res = true;
  const QStringList ids = configIds();
  for ( const auto &configid : ids )
  {
    res = res && reencryptAuthenticationConfig( configid, prevpass, prevciv );
  }
  return res;
}

bool QgsAuthManager::reencryptAuthenticationConfig( const QString &authcfg, const QString &prevpass, const QString &prevciv )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  // no need to check for setMasterPassword, since this is private and it will be set

  // Loop through all storages with capability ReadConfiguration and reencrypt the config
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration ) };

  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    try
    {
      if ( storage->methodConfigExists( authcfg ) )
      {
        if ( ! storage->isEncrypted() )
        {
          return true;
        }

        QString payload;
        const QgsAuthMethodConfig config = storage->loadMethodConfig( authcfg, payload, true );
        if ( payload.isEmpty() || ! config.isValid( true ) )
        {
          QgsDebugError( QStringLiteral( "Reencrypt FAILED, could not find config (id: %1)" ).arg( authcfg ) );
          return false;
        }

        QString configstring( QgsAuthCrypto::decrypt( prevpass, prevciv, payload ) );
        if ( configstring.isEmpty() )
        {
          QgsDebugError( QStringLiteral( "Reencrypt FAILED, could not decrypt config (id: %1)" ).arg( authcfg ) );
          return false;
        }

        configstring = QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), configstring );

        if ( !storage->storeMethodConfig( config, configstring ) )
        {
          emit messageLog( tr( "Store config: FAILED to store config in default storage: %1" ).arg( storage->lastError() ), authManTag(), Qgis::MessageLevel::Warning );
          return false;
        }
        return true;
      }
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
      return false;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any authentication configuration storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }
  else
  {
    emit messageLog( tr( "Reencrypt FAILED, could not find config (id: %1)" ).arg( authcfg ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

bool QgsAuthManager::reencryptAllAuthenticationSettings( const QString &prevpass, const QString &prevciv )
{
  ensureInitialized();

  // TODO: start remove (when function is actually used)
  Q_UNUSED( prevpass )
  Q_UNUSED( prevciv )
  return true;
  // end remove

#if 0
  if ( isDisabled() )
    return false;

  ///////////////////////////////////////////////////////////////
  // When adding settings that require encryption, add to list //
  ///////////////////////////////////////////////////////////////

  QStringList encryptedsettings;
  encryptedsettings << "";

  for ( const auto & sett, std::as_const( encryptedsettings ) )
  {
    if ( sett.isEmpty() || !existsAuthSetting( sett ) )
      continue;

    // no need to check for setMasterPassword, since this is private and it will be set

    QSqlQuery query( authDbConnection() );

    query.prepare( QStringLiteral( "SELECT value FROM %1 "
                                   "WHERE setting = :setting" ).arg( authDbSettingsTable() ) );

    query.bindValue( ":setting", sett );

    if ( !authDbQuery( &query ) )
      return false;

    if ( !query.isActive() || !query.isSelect() )
    {
      QgsDebugError( QStringLiteral( "Reencrypt FAILED, query not active or a select operation for setting: %2" ).arg( sett ) );
      return false;
    }

    if ( query.first() )
    {
      QString settvalue( QgsAuthCrypto::decrypt( prevpass, prevciv, query.value( 0 ).toString() ) );

      query.clear();

      query.prepare( QStringLiteral( "UPDATE %1 "
                                     "SET value = :value "
                                     "WHERE setting = :setting" ).arg( authDbSettingsTable() ) );

      query.bindValue( ":setting", sett );
      query.bindValue( ":value", QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), settvalue ) );

      if ( !authDbStartTransaction() )
        return false;

      if ( !authDbQuery( &query ) )
        return false;

      if ( !authDbCommit() )
        return false;

      QgsDebugMsgLevel( QStringLiteral( "Reencrypt SUCCESS for setting: %2" ).arg( sett ), 2 );
      return true;
    }
    else
    {
      QgsDebugError( QStringLiteral( "Reencrypt FAILED, could not find in db setting: %2" ).arg( sett ) );
      return false;
    }

    if ( query.next() )
    {
      QgsDebugError( QStringLiteral( "Select contains more than one for setting: %1" ).arg( sett ) );
      emit messageOut( tr( "Authentication database contains duplicate setting keys" ), authManTag(), WARNING );
    }

    return false;
  }

  return true;
#endif
}

bool QgsAuthManager::reencryptAllAuthenticationIdentities( const QString &prevpass, const QString &prevciv )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  bool res = true;
  const QStringList ids = certIdentityIds();
  for ( const auto &identid : ids )
  {
    res = res && reencryptAuthenticationIdentity( identid, prevpass, prevciv );
  }
  return res;
}

bool QgsAuthManager::reencryptAuthenticationIdentity(
  const QString &identid,
  const QString &prevpass,
  const QString &prevciv )
{
  ensureInitialized();

  if ( isDisabled() )
    return false;

  // no need to check for setMasterPassword, since this is private and it will be set

  // Loop through all storages with capability ReadCertificateIdentity and reencrypt the identity
  const QList<QgsAuthConfigurationStorage *> storages { authConfigurationStorageRegistry()->readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity ) };


  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {

    try
    {

      if ( storage->certIdentityExists( identid ) )
      {
        if ( ! storage->isEncrypted() )
        {
          return true;
        }

        const QPair<QSslCertificate, QString> identityBundle = storage->loadCertIdentityBundle( identid );
        QString keystring( QgsAuthCrypto::decrypt( prevpass, prevciv, identityBundle.second ) );
        if ( keystring.isEmpty() )
        {
          QgsDebugError( QStringLiteral( "Reencrypt FAILED, could not decrypt identity id: %1" ).arg( identid ) );
          return false;
        }

        keystring = QgsAuthCrypto::encrypt( mMasterPass, masterPasswordCiv(), keystring );
        return storage->storeCertIdentity( identityBundle.first, keystring );
      }
    }
    catch ( const QgsNotSupportedException &e )
    {
      // It should not happen because we are checking the capability in advance
      emit messageLog( e.what(), authManTag(), Qgis::MessageLevel::Critical );
      return false;
    }
  }

  if ( storages.empty() )
  {
    emit messageLog( tr( "Could not connect to any authentication configuration storage." ), authManTag(), Qgis::MessageLevel::Critical );
  }
  else
  {
    emit messageLog( tr( "Reencrypt FAILED, could not find identity (id: %1)" ).arg( identid ), authManTag(), Qgis::MessageLevel::Critical );
  }

  return false;
}

void QgsAuthManager::insertCaCertInCache( QgsAuthCertUtils::CaCertSource source, const QList<QSslCertificate> &certs )
{
  ensureInitialized();

  for ( const auto &cert : certs )
  {
    mCaCertsCache.insert( QgsAuthCertUtils::shaHexForCert( cert ),
                          QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate>( source, cert ) );
  }
}

QString QgsAuthManager::authPasswordHelperKeyName() const
{
  ensureInitialized();

  QString dbProfilePath;

  // TODO: get the current profile name from the application

  if ( isFilesystemBasedDatabase( mAuthDatabaseConnectionUri ) )
  {
    const QFileInfo info( mAuthDatabaseConnectionUri );
    dbProfilePath = info.dir().dirName();
  }
  else
  {
    dbProfilePath = QCryptographicHash::hash( ( mAuthDatabaseConnectionUri.toUtf8() ), QCryptographicHash::Md5 ).toHex();
  }

  // if not running from the default profile, ensure that a different key is used
  return AUTH_PASSWORD_HELPER_KEY_NAME_BASE + ( dbProfilePath.compare( QLatin1String( "default" ), Qt::CaseInsensitive ) == 0 ? QString() : dbProfilePath );
}

QgsAuthConfigurationStorageDb *QgsAuthManager::defaultDbStorage() const
{
  QgsAuthConfigurationStorageRegistry *storageRegistry = authConfigurationStorageRegistry();
  const auto storages = storageRegistry->readyStorages( );
  for ( QgsAuthConfigurationStorage *storage : std::as_const( storages ) )
  {
    if ( qobject_cast<QgsAuthConfigurationStorageDb *>( storage ) )
    {
      return static_cast<QgsAuthConfigurationStorageDb *>( storage );
    }
  }
  return nullptr;
}

QgsAuthConfigurationStorage *QgsAuthManager::firstStorageWithCapability( Qgis::AuthConfigurationStorageCapability capability ) const
{
  QgsAuthConfigurationStorageRegistry *storageRegistry = authConfigurationStorageRegistry();
  return storageRegistry->firstReadyStorageWithCapability( capability );
}

