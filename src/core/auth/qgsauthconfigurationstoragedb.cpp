/***************************************************************************
  qgsauthconfigurationstoragedb.cpp - QgsAuthConfigurationStorageDb

 ---------------------
 begin                : 20.6.2024
 copyright            : (C) 2024 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsauthconfigurationstoragedb.h"
#include "moc_qgsauthconfigurationstoragedb.cpp"

#include "qgslogger.h"
#include "qgsauthcertutils.h"
#include "qurl.h"

#include <QSqlError>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QThread>
#include <QCoreApplication>
#include <QUrlQuery>


QgsAuthConfigurationStorageDb::QgsAuthConfigurationStorageDb( const QMap<QString, QVariant> &settings )
  : QgsAuthConfigurationStorage( settings )
{
  // Parse settings
  mDriver = mConfiguration.value( QStringLiteral( "driver" ), QStringLiteral( "QSQLITE" ) ).toString();
  mDatabase = mConfiguration.value( QStringLiteral( "database" ) ).toString();
  mHost = mConfiguration.value( QStringLiteral( "host" ) ).toString();
  mPort = mConfiguration.value( QStringLiteral( "port" ) ).toInt();
  mUser = mConfiguration.value( QStringLiteral( "user" ) ).toString();
  mPassword = mConfiguration.value( QStringLiteral( "password" ) ).toString();
  mConnectOptions = mConfiguration.value( QStringLiteral( "options" ) ).toString();
  // Debug print all connection settings
  QgsDebugMsgLevel( QStringLiteral( "Auth db connection settings: driver=%1, database='%2', host=%3, port=%4, user='%5', schema=%6, options=%7" )
                    .arg( mDriver, mDatabase, mHost, QString::number( mPort ), mUser, mConnectOptions, mConfiguration.value( QStringLiteral( "schema" ) ).toString() ), 2 );

}

QgsAuthConfigurationStorageDb::QgsAuthConfigurationStorageDb( const QString &uri )
  : QgsAuthConfigurationStorageDb( uriToSettings( uri ) )
{
}

QgsAuthConfigurationStorageDb::~QgsAuthConfigurationStorageDb()
{
  QMutexLocker locker( &mMutex );

  QMapIterator<QThread *, QMetaObject::Connection> iterator( mConnectedThreads );
  while ( iterator.hasNext() )
  {
    iterator.next();
    QThread::disconnect( iterator.value() );
  }
}

QSqlDatabase QgsAuthConfigurationStorageDb::authDatabaseConnection() const
{
  QSqlDatabase authdb;

  QMutexLocker locker( &mMutex );

  const QString connectionName = QStringLiteral( "authentication.configs:0x%1" ).arg( reinterpret_cast<quintptr>( QThread::currentThread() ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) );
  QgsDebugMsgLevel( QStringLiteral( "Using auth db connection name: %1 " ).arg( connectionName ), 3 );
  if ( !QSqlDatabase::contains( connectionName ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "No existing connection, creating a new one" ), 3 );
    authdb = QSqlDatabase::addDatabase( mDriver, connectionName );


    // Check that the driver is available
    if ( !QSqlDatabase::isDriverAvailable( mDriver ) )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Driver '%1' is not available" ).arg( mDriver ) );
      return authdb;
    }

    authdb.setDatabaseName( mDatabase );
    authdb.setHostName( mHost );
    authdb.setPort( mPort );
    authdb.setUserName( mUser );
    authdb.setPassword( mPassword );
    authdb.setConnectOptions( mConnectOptions );

    // Return if not valid
    if ( !authdb.isValid() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db connection is not valid" ) );
      return authdb;
    }

    // for background threads, remove database when current thread finishes
    if ( QCoreApplication::instance() && QThread::currentThread() != QCoreApplication::instance()->thread() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Scheduled auth db remove on thread close" ), 4 );

      // IMPORTANT - we use a direct connection here, because the database removal must happen immediately
      // when the thread finishes, and we cannot let this get queued on the main thread's event loop (where
      // QgsAuthManager lives).
      // Otherwise, the QSqlDatabase's private data's thread gets reset immediately the QThread::finished,
      // and a subsequent call to QSqlDatabase::database with the same thread address (yep it happens, actually a lot)
      // triggers a condition in QSqlDatabase which detects the nullptr private thread data and returns an invalid database instead.
      // QSqlDatabase::removeDatabase is thread safe, so this is ok to do.
      // Right about now is a good time to re-evaluate your selected career ;)
      // I've done that and I decided to become a musician. I'll probably be a better musician than a software developer.
      QMetaObject::Connection connection = connect( QThread::currentThread(), &QThread::finished, QThread::currentThread(), [connectionName, this ]
      {
        QMutexLocker locker( &mMutex );
        QSqlDatabase::removeDatabase( connectionName );
        mConnectedThreads.remove( QThread::currentThread() ); // NOLINT(clang-analyzer-core.CallAndMessage)
      }, Qt::DirectConnection );

      mConnectedThreads.insert( QThread::currentThread(), connection );
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Reusing existing connection" ), 4 );
    authdb = QSqlDatabase::database( connectionName, false );
  }

  locker.unlock();

  if ( !authdb.isOpen() )
  {
    if ( !authdb.open() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Opening of authentication db FAILED : %1" ).arg( authdb.lastError().text() ) );
    }
  }

  return authdb;
}

bool QgsAuthConfigurationStorageDb::authDbOpen() const
{
  QMutexLocker locker( &mMutex );
  QSqlDatabase authdb = authDatabaseConnection();

  if ( !authdb.isOpen() )
  {
    if ( !authdb.open() )
    {
      const QString err = tr( "Unable to establish database connection\nDatabase: %1\nDriver error: %2\nDatabase error: %3" )
                          .arg( mDatabase,
                                authdb.lastError().driverText(),
                                authdb.lastError().databaseText() );

      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( err );
      return false;
    }
  }
  return true;
}


bool QgsAuthConfigurationStorageDb::authDbQuery( QSqlQuery *query, const QString &sql ) const
{
  QMutexLocker locker( &mMutex );
  query->setForwardOnly( true );
  const bool result { sql.isEmpty() ? query->exec() : query->exec( sql ) };

  auto boundQuery = []( const QSqlQuery * query ) -> QString
  {
    QString str = query->lastQuery();
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    QMapIterator<QString, QVariant> it( query->boundValues() );
#else
    const QStringList keys = query->boundValueNames();
    const QVariantList values = query->boundValues();
    QMap<QString, QVariant> boundValues;
    for ( int i = 0; i < keys.count(); i++ )
    {
      boundValues.insert( keys.at( i ), values.at( i ).toString() );
    }
    QMapIterator<QString, QVariant> it = QMapIterator<QString, QVariant>( boundValues );
#endif
    while ( it.hasNext() )
    {
      it.next();
      str.replace( it.key(), it.value().toString() );
    }
    return str;
  };

  if ( !result )
  {
    if ( query->lastError().isValid() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db query FAILED: %1\nError: %2" )
          .arg( sql.isEmpty() ? boundQuery( query ) : sql,
                query->lastError().text() ), Qgis::MessageLevel::Warning );
      QgsDebugMsgLevel( QStringLiteral( "Auth db query FAILED: %1" ).arg( sql.isEmpty() ? boundQuery( query ) : sql ), 2 );
      return false;
    }
    else
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db query exec() FAILED: %1" ).arg( sql.isEmpty() ? boundQuery( query ) : sql ), Qgis::MessageLevel::Warning );
      QgsDebugMsgLevel( QStringLiteral( "Auth db query FAILED: %1" ).arg( sql.isEmpty() ? boundQuery( query ) : sql ), 2 );
      return false;
    }
  }
  return true;
}


bool QgsAuthConfigurationStorageDb::authDbTransactionQuery( QSqlQuery *query )
{
  QMutexLocker locker( &mMutex );
  if ( !authDatabaseConnection().transaction() )
  {
    setError( tr( "Auth db FAILED to start transaction" ), Qgis::MessageLevel::Warning );
    return false;
  }

  bool ok = authDbQuery( query );

  if ( ok && !authDatabaseConnection().commit() )
  {
    setError( tr( "Auth db FAILED to rollback changes" ), Qgis::MessageLevel::Warning );
    ( void )authDatabaseConnection().rollback();
    return false;
  }

  return ok;
}

bool QgsAuthConfigurationStorageDb::initialize()
{
  QMutexLocker locker( &mMutex );
  QSqlDatabase authdb = authDatabaseConnection();
  if ( !authdb.isValid() || !authdb.isOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }
  else
  {
    if ( !isReadOnly() && ( !createCertTables() || !createConfigTables() ) )
    {
      setError( tr( "Auth db initialization FAILED: %1" ).arg( lastError() ), Qgis::MessageLevel::Critical );
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
}

QList<QgsAuthConfigurationStorage::SettingParameter> QgsAuthConfigurationStorageDb::settingsParameters() const
{
  return
  {
    { QStringLiteral( "driver" ), tr( "SQL Driver (see https://doc.qt.io/qt/sql-driver.html)" ), QVariant::String },
    { QStringLiteral( "database" ), tr( "Database" ), QVariant::String },
    { QStringLiteral( "schema" ), tr( "Schema for all tables" ), QVariant::String },
    { QStringLiteral( "host" ), tr( "Host" ), QVariant::String },
    { QStringLiteral( "port" ), tr( "Port" ), QVariant::Int },
    { QStringLiteral( "user" ), tr( "User" ), QVariant::String },
    { QStringLiteral( "password" ), tr( "Password" ), QVariant::String },
    { QStringLiteral( "options" ), tr( "Connection options" ), QVariant::String },
  };
}

#ifndef QT_NO_SSL

bool QgsAuthConfigurationStorageDb::storeCertIdentity( const QSslCertificate &cert, const QString &keyPem )
{
  QMutexLocker locker( &mMutex );

  const QString id{ QgsAuthCertUtils::shaHexForCert( cert ) };

  if ( certIdentityExists( id ) )
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::UpdateCertificateIdentity );
    removeCertIdentity( id );
  }
  else
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::CreateCertificateIdentity );
  }

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  if ( cert.isNull() )
  {
    setError( tr( "Certificate is NULL" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );
  const QString certPem{ cert.toPem() };

  query.prepare( QStringLiteral( "INSERT INTO %1 (id, key, cert) VALUES (:id, :key, :cert)" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );
  query.bindValue( QStringLiteral( ":key" ), keyPem );
  query.bindValue( QStringLiteral( ":cert" ), certPem );

  if ( !authDbQuery( &query ) )
    return false;

  emit certIdentityChanged();

  return true;
}

bool QgsAuthConfigurationStorageDb::removeCertIdentity( const QSslCertificate &cert )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteCertificateIdentity );

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "DELETE FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), cert.digest().toHex() );

  if ( !authDbQuery( &query ) )
  {
    setError( tr( "Failed to remove cert identity '%1'" ).arg( QString( cert.digest().toHex() ) ), Qgis::MessageLevel::Critical );
    return false;
  }

  emit certIdentityChanged();

  return true;
}

const QSslCertificate QgsAuthConfigurationStorageDb::loadCertIdentity( const QString &id ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity );

  QSslCertificate emptycert;

  if ( id.isEmpty() )
    return emptycert;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return emptycert;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT cert FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
    return emptycert;

  QSslCertificate cert;

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      cert = QSslCertificate( query.value( 0 ).toByteArray(), QSsl::Pem );
      QgsDebugMsgLevel( QStringLiteral( "Certificate identity retrieved for id: %1" ).arg( id ), 2 );
      if ( cert.isNull() )
      {
        const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Failed to retrieve certificate identity for id: %1: certificate is NULL" ).arg( id ), Qgis::MessageLevel::Warning );
        return emptycert;
      }
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Authentication database contains more than one certificate identity for id: %1" ).arg( id ), Qgis::MessageLevel::Warning );
      return emptycert;
    }
  }
  return cert;
}

const QPair<QSslCertificate, QString> QgsAuthConfigurationStorageDb::loadCertIdentityBundle( const QString &id ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity );

  QPair<QSslCertificate, QString> bundle;

  if ( id.isEmpty() )
    return bundle;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return bundle;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT key, cert FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) ) );

  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
    return bundle;

  if ( query.isActive() && query.isSelect() )
  {
    QSslCertificate cert;
    QString key;
    if ( query.first() )
    {
      key = query.value( 0 ).toString();
      if ( key.isEmpty() )
      {
        const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Retrieve certificate identity bundle: FAILED to create private key" ), Qgis::MessageLevel::Warning );
        return bundle;
      }
      cert = QSslCertificate( query.value( 1 ).toByteArray(), QSsl::Pem );
      if ( cert.isNull() )
      {
        const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Retrieve certificate identity bundle: FAILED to create certificate" ), Qgis::MessageLevel::Warning );
        return bundle;
      }
      QgsDebugMsgLevel( QStringLiteral( "Certificate identity bundle retrieved for id: %1" ).arg( id ), 2 );
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Retrieved more than one certificate identity for id: %1" ).arg( id ),  Qgis::MessageLevel::Warning );
      return bundle;
    }
    bundle = qMakePair( cert, key );
  }
  return bundle;
}

const QList<QSslCertificate> QgsAuthConfigurationStorageDb::certIdentities() const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity );

  QList<QSslCertificate> certs;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return certs;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT cert FROM %1" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) ) );

  if ( !authDbQuery( &query ) )
    return certs;

  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      QSslCertificate cert( query.value( 0 ).toByteArray(), QSsl::Pem );
      if ( !cert.isNull() )
      {
        certs.append( cert );
      }
    }
  }
  return certs;
}

QStringList QgsAuthConfigurationStorageDb::certIdentityIds() const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return {};
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id FROM %1" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) ) );

  if ( !authDbQuery( &query ) )
    return {};

  QStringList ids;
  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      ids.append( query.value( 0 ).toString() );
    }
  }
  return ids;
}

bool QgsAuthConfigurationStorageDb::certIdentityExists( const QString &id ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity );

  if ( id.isEmpty() )
    return false;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return {};
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT cert FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  bool ret { false };

  if ( !authDbQuery( &query ) )
    return false;

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      ret = true;
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Authentication database contains more than one certificate bundles for id: %1" ).arg( id ),  Qgis::MessageLevel::Warning );
      return false;
    }
  }
  return ret;
}

bool QgsAuthConfigurationStorageDb::removeCertIdentity( const QString &id )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteCertificateIdentity );

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return {};
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "DELETE FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
  {
    setError( tr( "Failed to remove certificate identity '%1'" ).arg( id ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( query.numRowsAffected() == 0 )
  {
    setError( tr( "No certificate identity found for id: %1" ).arg( id ), Qgis::MessageLevel::Warning );
    return false;
  }

  emit certIdentityChanged();

  return true;
}

bool QgsAuthConfigurationStorageDb::storeSslCertCustomConfig( const QgsAuthConfigSslServer &config )
{
  QMutexLocker locker( &mMutex );

  QSslCertificate cert( config.sslCertificate() );
  QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

  if ( sslCertCustomConfigExists( id, config.sslHostPort() ) )
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::UpdateSslCertificateCustomConfig );
    removeSslCertCustomConfig( id, config.sslHostPort().trimmed() );
  }
  else
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::CreateSslCertificateCustomConfig );
  }

  if ( ! capabilities().testFlag( Qgis::AuthConfigurationStorageCapability::CreateSslCertificateCustomConfig ) )
  {
    setError( tr( "Storage does not support creating SSL certificate custom configs" ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QString certpem( cert.toPem() );
  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "INSERT INTO %1 (id, host, cert, config) VALUES (:id, :host, :cert, :config)" ).arg( quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) ) );

  query.bindValue( QStringLiteral( ":id" ), id );
  query.bindValue( QStringLiteral( ":host" ), config.sslHostPort().trimmed() );
  query.bindValue( QStringLiteral( ":cert" ), certpem );
  query.bindValue( QStringLiteral( ":config" ), config.configString() );

  if ( !authDbQuery( &query ) )
    return false;

  QgsDebugMsgLevel( QStringLiteral( "Store SSL cert custom config SUCCESS for host:port, id: %1, %2" )
                    .arg( config.sslHostPort().trimmed(), id ), 2 );

  emit sslCertCustomConfigChanged();

  return true;
}

QStringList QgsAuthConfigurationStorageDb::sslCertCustomConfigIds() const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return {};
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id FROM %1" ).arg( quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) ) );

  if ( !authDbQuery( &query ) )
    return {};

  QStringList ids;
  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      ids.append( query.value( 0 ).toString() );
    }
  }
  return ids;

}

const QgsAuthConfigSslServer QgsAuthConfigurationStorageDb::loadSslCertCustomConfig( const QString &id, const QString &hostport ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig );

  QgsAuthConfigSslServer config;

  if ( id.isEmpty() || hostport.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed config ID or host:port is empty" ) );
    return config;
  }

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return config;
  }

  QSqlQuery query( authDatabaseConnection() );
  query.prepare( QStringLiteral( "SELECT host, cert, config FROM %1 WHERE id = :id AND host = :host" ).arg( quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );
  query.bindValue( QStringLiteral( ":host" ), hostport.trimmed() );

  if ( !authDbQuery( &query ) )
    return config;

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      config.setSslCertificate( QSslCertificate( query.value( 1 ).toByteArray(), QSsl::Pem ) );
      config.setSslHostPort( query.value( 0 ).toString().trimmed() );
      config.loadConfigString( query.value( 2 ).toString() );
      QgsDebugMsgLevel( QStringLiteral( "SSL cert custom config retrieved for host:port, id: %1, %2" ).arg( hostport, id ), 2 );
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Retrieved more than one SSL cert custom config for id: %1" ).arg( id ), Qgis::MessageLevel::Warning );
      return QgsAuthConfigSslServer();
    }
  }
  return config;
}

const QgsAuthConfigSslServer QgsAuthConfigurationStorageDb::loadSslCertCustomConfigByHost( const QString &hostport ) const
{
  // TODO: implement a flag to skip the query in case there are no custom
  // certs in the storage, to avoid the overhead of checking the db
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig );

  QgsAuthConfigSslServer config;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return config;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id, cert, config FROM %1 WHERE host = :host" ).arg( quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) ) );

  query.bindValue( QStringLiteral( ":host" ), hostport.trimmed() );

  if ( !authDbQuery( &query ) )
    return config;

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      config.setSslCertificate( QSslCertificate( query.value( 1 ).toByteArray(), QSsl::Pem ) );
      config.setSslHostPort( hostport );
      config.loadConfigString( query.value( 2 ).toString() );
      QgsDebugMsgLevel( QStringLiteral( "SSL cert custom config retrieved for host:port %1" ).arg( hostport ), 2 );
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Authentication database contains more than one SSL cert custom config" ), Qgis::MessageLevel::Warning );
      return QgsAuthConfigSslServer();
    }
  }

  return config;
}

const QList<QgsAuthConfigSslServer> QgsAuthConfigurationStorageDb::sslCertCustomConfigs() const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig );

  QList<QgsAuthConfigSslServer> configs;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return configs;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id, host, cert, config FROM %1" ).arg( quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) ) );

  if ( !authDbQuery( &query ) )
    return configs;

  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      QgsAuthConfigSslServer config;
      config.setSslCertificate( QSslCertificate( query.value( 2 ).toByteArray(), QSsl::Pem ) );
      config.setSslHostPort( query.value( 1 ).toString().trimmed() );
      config.loadConfigString( query.value( 3 ).toString() );
      configs.append( config );
    }
  }
  return configs;
}

bool QgsAuthConfigurationStorageDb::sslCertCustomConfigExists( const QString &id, const QString &hostport )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig );

  if ( id.isEmpty() || hostport.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed config ID or host:port is empty" ) );
    return false;
  }

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id FROM %1 WHERE id = :id AND host = :host" ).arg( quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );
  query.bindValue( QStringLiteral( ":host" ), hostport.trimmed() );

  if ( !authDbQuery( &query ) )
    return false;

  bool res = false;
  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      QgsDebugMsgLevel( QStringLiteral( "SSL cert custom config exists for host:port, id: %1, %2" ).arg( hostport, id ), 2 );
      res = true;
    }
    if ( query.next() )
    {
      QgsDebugError( QStringLiteral( "Retrieved more than one SSL cert custom config for host:port, id: %1, %2" ).arg( hostport, id ) );
      emit messageLog( tr( "Authentication database contains more than one SSL cert custom configs for host:port, id: %1, %2" )
                       .arg( hostport, id ), loggerTag(), Qgis::MessageLevel::Warning );
      return false;
    }
  }
  return res;
}

bool QgsAuthConfigurationStorageDb::removeSslCertCustomConfig( const QString &id, const QString &hostport )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteSslCertificateCustomConfig );

  if ( id.isEmpty() || hostport.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Passed config ID or host:port is empty" ) );
    return false;
  }

  // TODO: check the flag to skip the query in case there are no custom certs config by host in the storage

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "DELETE FROM %1 WHERE id = :id AND host = :host" ).arg( quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );
  query.bindValue( QStringLiteral( ":host" ), hostport.trimmed() );

  if ( !authDbQuery( &query ) )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Failed to remove SSL certificate custom config for host:port, id: %1, %2" ).arg( hostport, id ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( query.numRowsAffected() == 0 )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "No SSL certificate custom config found for host:port, id: %1, %2" ).arg( hostport, id ), Qgis::MessageLevel::Warning );
    return false;
  }

  emit sslCertCustomConfigChanged();

  return true;
}

QStringList QgsAuthConfigurationStorageDb::certAuthorityIds() const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return {};
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id FROM %1" ).arg( quotedQualifiedIdentifier( certAuthorityTableName() ) ) );

  if ( !authDbQuery( &query ) )
    return {};

  QStringList ids;
  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      ids.append( query.value( 0 ).toString() );
    }
  }
  return ids;
}

bool QgsAuthConfigurationStorageDb::storeCertAuthority( const QSslCertificate &cert )
{
  QMutexLocker locker( &mMutex );

  if ( certAuthorityExists( cert ) )
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::UpdateCertificateAuthority );
    removeCertAuthority( cert );
  }
  else
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::CreateCertificateAuthority );
  }

  // don't refuse !cert.isValid() (actually just expired) CAs,
  // as user may want to ignore that SSL connection error
  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  const QString id( QgsAuthCertUtils::shaHexForCert( cert ) );
  const QString pem( cert.toPem() );

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "INSERT INTO %1 (id, cert) VALUES (:id, :cert)" ).arg( quotedQualifiedIdentifier( certAuthorityTableName() ) ) );

  query.bindValue( QStringLiteral( ":id" ), id );
  query.bindValue( QStringLiteral( ":cert" ), pem );

  if ( !authDbQuery( &query ) )
    return false;

  QgsDebugMsgLevel( QStringLiteral( "Store certificate authority SUCCESS for id: %1" ).arg( id ), 2 );
  emit certAuthorityChanged();

  return true;
}

const QSslCertificate QgsAuthConfigurationStorageDb::loadCertAuthority( const QString &id ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority );

  QSslCertificate emptycert;

  if ( id.isEmpty() )
    return emptycert;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return emptycert;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT cert FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certAuthorityTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
    return emptycert;

  QSslCertificate cert;

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      cert = QSslCertificate( query.value( 0 ).toByteArray(), QSsl::Pem );
      QgsDebugMsgLevel( QStringLiteral( "Certificate authority retrieved for id: %1" ).arg( id ), 2 );
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Retrieved more than one certificate authority for id: %1" ).arg( id ), Qgis::MessageLevel::Warning );
      return emptycert;
    }
  }
  return cert;
}

bool QgsAuthConfigurationStorageDb::certAuthorityExists( const QSslCertificate &cert ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority );

  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  const QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certAuthorityTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
    return false;

  bool res = false;
  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Certificate authority exists for id: %1" ).arg( id ), 2 );
      res = true;
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Retrieved more than one certificate authority for id: %1" ).arg( id ), Qgis::MessageLevel::Warning );
      // TODO: check whether it makes sense to return false here (and in other similar cases)
      return false;
    }
  }
  return res;
}

bool QgsAuthConfigurationStorageDb::removeCertAuthority( const QSslCertificate &cert )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteCertificateAuthority );

  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  const QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "DELETE FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certAuthorityTableName() ) ) );

  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
  {
    setError( tr( "Failed to remove certificate authority '%1'" ).arg( id ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( query.numRowsAffected() == 0 )
  {
    setError( tr( "No certificate authority found for id: %1" ).arg( id ), Qgis::MessageLevel::Warning );
    return false;
  }

  emit certAuthorityChanged();

  return true;
}

const  QMap<QString, QgsAuthCertUtils::CertTrustPolicy> QgsAuthConfigurationStorageDb::caCertsPolicy() const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateTrustPolicy );

  QMap<QString, QgsAuthCertUtils::CertTrustPolicy> trustedCerts;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return trustedCerts;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id, policy FROM %1" ).arg( quotedQualifiedIdentifier( certTrustPolicyTableName() ) ) );

  if ( !authDbQuery( &query ) )
    return trustedCerts;

  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      QString id( query.value( 0 ).toString() );
      int policy = query.value( 1 ).toInt();
      QgsAuthCertUtils::CertTrustPolicy trustPolicy = static_cast< QgsAuthCertUtils::CertTrustPolicy >( policy );
      trustedCerts[ id ] = trustPolicy;
    }
  }

  return trustedCerts;
}

const QList<QSslCertificate> QgsAuthConfigurationStorageDb::caCerts() const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority );

  QList<QSslCertificate> authorities;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return authorities;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT id, cert FROM %1" ).arg( quotedQualifiedIdentifier( certAuthorityTableName() ) ) );

  if ( !authDbQuery( &query ) )
    return authorities;

  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      const QSslCertificate cert( query.value( 1 ).toByteArray(), QSsl::Pem );
      if ( !cert.isNull() )
      {
        authorities.append( cert );
      }
      else
      {
        const QString id { query.value( 0 ).toString() };
        const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Invalid CA found: %1" ).arg( id ), Qgis::MessageLevel::Warning );
      }
    }
  }
  return authorities;
}

bool QgsAuthConfigurationStorageDb::storeCertTrustPolicy( const QSslCertificate &cert, QgsAuthCertUtils::CertTrustPolicy policy )
{
  QMutexLocker locker( &mMutex );

  const bool policyExisted = certTrustPolicyExists( cert );

  if ( policyExisted )
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::UpdateCertificateTrustPolicy );
  }
  else
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::CreateCertificateTrustPolicy );
  }

  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return false;
  }

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  const QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

  // Handle default trust case
  if ( policy == QgsAuthCertUtils::DefaultTrust )
  {
    if ( !policyExisted )
    {
      QgsDebugMsgLevel( QStringLiteral( "Passed policy was default, no cert records in database for id: %1" ).arg( id ), 2 );
      return true;
    }

    if ( !removeCertTrustPolicy( cert ) )
    {
      setError( tr( "Failed to remove certificate trust policy for id: %1" ).arg( id ) );
      return false;
    }

    QgsDebugMsgLevel( QStringLiteral( "Passed policy was default, all cert records in database were removed for id: %1" ).arg( id ), 2 );

    emit certAuthorityChanged();

    return true;
  }

  // Handle other policies
  if ( policyExisted && !removeCertTrustPolicy( cert ) )
  {
    setError( tr( "Failed to remove certificate trust policy for id: %1" ).arg( id ) );
    return false;
  }

  // Insert new policy
  QSqlQuery query( authDatabaseConnection() );
  query.prepare( QStringLiteral( "INSERT INTO %1 (id, policy) VALUES (:id, :policy)" ).arg( quotedQualifiedIdentifier( certTrustPolicyTableName() ) ) );

  query.bindValue( QStringLiteral( ":id" ), id );
  query.bindValue( QStringLiteral( ":policy" ), static_cast< int >( policy ) );

  if ( !authDbQuery( &query ) )
    return false;

  QgsDebugMsgLevel( QStringLiteral( "Store certificate trust policy SUCCESS for id: %1" ).arg( id ), 2 );
  emit certAuthorityChanged();

  return true;
}

QgsAuthCertUtils::CertTrustPolicy QgsAuthConfigurationStorageDb::loadCertTrustPolicy( const QSslCertificate &cert ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateTrustPolicy );

  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return QgsAuthCertUtils::DefaultTrust;
  }

  QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return QgsAuthCertUtils::DefaultTrust;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT policy FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certTrustPolicyTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
    return QgsAuthCertUtils::DefaultTrust;

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      int policy = query.value( 0 ).toInt();
      QgsDebugMsgLevel( QStringLiteral( "Certificate trust policy retrieved for id: %1" ).arg( id ), 2 );
      return static_cast< QgsAuthCertUtils::CertTrustPolicy >( policy );
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Retrieved more than one certificate trust policy for id: %1" ).arg( id ), Qgis::MessageLevel::Warning );
      return QgsAuthCertUtils::DefaultTrust;
    }
  }
  return QgsAuthCertUtils::DefaultTrust;
}

bool QgsAuthConfigurationStorageDb::removeCertTrustPolicy( const QSslCertificate &cert )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteCertificateTrustPolicy );

  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return QgsAuthCertUtils::DefaultTrust;
  }

  QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return QgsAuthCertUtils::DefaultTrust;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "DELETE FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certTrustPolicyTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
  {
    setError( tr( "Failed to remove certificate trust policy '%1'" ).arg( id ) );
    return false;
  }

  if ( query.numRowsAffected() == 0 )
  {
    setError( tr( "No certificate trust policy found for id: %1" ).arg( id ) );
    return false;
  }

  emit sslCertTrustPolicyChanged();

  return true;
}

bool QgsAuthConfigurationStorageDb::certTrustPolicyExists( const QSslCertificate &cert ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadCertificateTrustPolicy );

  if ( cert.isNull() )
  {
    QgsDebugError( QStringLiteral( "Passed certificate is null" ) );
    return QgsAuthCertUtils::DefaultTrust;
  }

  QString id( QgsAuthCertUtils::shaHexForCert( cert ) );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return QgsAuthCertUtils::DefaultTrust;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT COUNT(id) FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( certTrustPolicyTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
    return false;

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      return query.value( 0 ).toInt() > 0;
    }
  }
  return false;
}

#endif

const QList<QgsAuthConfigurationStorage::MasterPasswordConfig> QgsAuthConfigurationStorageDb::masterPasswords() const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadMasterPassword );

  QList<QgsAuthConfigurationStorage::MasterPasswordConfig> passwords;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return passwords;
  }

  QSqlQuery query( authDatabaseConnection() );
  query.prepare( QStringLiteral( "SELECT salt, civ, hash FROM %1" ).arg( quotedQualifiedIdentifier( masterPasswordTableName() ) ) );

  if ( !authDbQuery( &query ) )
    return passwords;

  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      const QString salt = query.value( 0 ).toString();
      const QString civ = query.value( 1 ).toString();
      const QString hash = query.value( 2 ).toString();
      passwords.append( { salt, civ, hash } );
    }
  }
  return passwords;
}

bool QgsAuthConfigurationStorageDb::storeMasterPassword( const QgsAuthConfigurationStorage::MasterPasswordConfig &config )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::CreateMasterPassword );

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );
  query.prepare( QStringLiteral( "INSERT INTO %1 (salt, civ, hash) VALUES (:salt, :civ, :hash)" ).arg( quotedQualifiedIdentifier( masterPasswordTableName() ) ) );

  query.bindValue( QStringLiteral( ":salt" ), config.salt );
  query.bindValue( QStringLiteral( ":civ" ), config.civ );
  query.bindValue( QStringLiteral( ":hash" ), config.hash );

  if ( !authDbQuery( &query ) )
    return false;

  emit masterPasswordChanged();

  return true;
}

bool QgsAuthConfigurationStorageDb::clearMasterPasswords()
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteMasterPassword );

  const bool ret { clearTables( { masterPasswordTableName() } ) };

  if ( ret )
  {
    emit masterPasswordChanged();
  }

  return ret;
}

QString QgsAuthConfigurationStorageDb::methodConfigTableName() const
{
  return QStringLiteral( "auth_configs" );
}

QString QgsAuthConfigurationStorageDb::authSettingsTableName() const
{
  return QStringLiteral( "auth_settings" );
}

QString QgsAuthConfigurationStorageDb::certIdentityTableName() const
{
  return QStringLiteral( "auth_identities" );
}

QString QgsAuthConfigurationStorageDb::sslCertCustomConfigTableName() const
{
  return QStringLiteral( "auth_servers" );
}

QString QgsAuthConfigurationStorageDb::certAuthorityTableName() const
{
  return QStringLiteral( "auth_authorities" );
}

QString QgsAuthConfigurationStorageDb::certTrustPolicyTableName() const
{
  return QStringLiteral( "auth_trust" );
}

QString QgsAuthConfigurationStorageDb::masterPasswordTableName() const
{
  return QStringLiteral( "auth_pass" );
}

QString QgsAuthConfigurationStorageDb::quotedQualifiedIdentifier( const QString &name, bool isIndex ) const
{
  const QString schema { mConfiguration.value( QStringLiteral( "schema" ) ).toString() };
  if ( schema.isEmpty() )
  {
    return authDatabaseConnection().driver()->escapeIdentifier( name, QSqlDriver::TableName );
  }
  else
  {
    if ( isIndex )
    {
      return authDatabaseConnection().driver()->escapeIdentifier( schema + QStringLiteral( "_" ) + name, QSqlDriver::TableName );
    }
    else
    {
      return authDatabaseConnection().driver()->escapeIdentifier( schema, QSqlDriver::TableName ) + QStringLiteral( "." ) + authDatabaseConnection().driver()->escapeIdentifier( name, QSqlDriver::TableName );
    }
  }
}

QString QgsAuthConfigurationStorageDb::name() const
{
  return QStringLiteral( "%1:%2" ).arg( mDriver, mDatabase );
}

QString QgsAuthConfigurationStorageDb::type() const
{
  return QStringLiteral( "DB-%2" ).arg( mDriver );
}

QString QgsAuthConfigurationStorageDb::description() const
{
  return tr( "Store credentials in a %1 database" ).arg( name() );
}

QString QgsAuthConfigurationStorageDb::id() const
{
  QMutexLocker locker( &mMutex );

  if ( mId.isEmpty() )
  {
    // Create a hash from the driver name, database name, port, hostname and username
    QCryptographicHash hash( QCryptographicHash::Sha256 );
    hash.addData( mDriver.toUtf8() );
    hash.addData( mDatabase.toUtf8() );
    hash.addData( QString::number( mPort ).toUtf8() );
    hash.addData( mHost.toUtf8() );
    hash.addData( mUser.toUtf8() );
    mId = QString( hash.result().toHex() );
  }
  return mId;
}

bool QgsAuthConfigurationStorageDb::createConfigTables()
{
  QMutexLocker locker( &mMutex );

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  // create the tables
  QString qstr;

  qstr = QStringLiteral( "CREATE TABLE IF NOT EXISTS %1 (\n"
                         "    salt TEXT NOT NULL,\n"
                         "    civ TEXT NOT NULL\n"
                         ", hash TEXT NOT NULL);" ).arg( quotedQualifiedIdentifier( masterPasswordTableName() ) );

  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE TABLE IF NOT EXISTS %1 (\n"
                         "    id TEXT NOT NULL,\n"
                         "    name TEXT NOT NULL,\n"
                         "    uri TEXT,\n"
                         "    type TEXT NOT NULL,\n"
                         "    version INTEGER NOT NULL\n"
                         ", config TEXT NOT NULL);" ).arg( quotedQualifiedIdentifier( methodConfigTableName() ) );
  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE UNIQUE INDEX IF NOT EXISTS %1 ON %2 (id ASC);" ).arg( quotedQualifiedIdentifier( QStringLiteral( "config_id_index" ), true ), quotedQualifiedIdentifier( methodConfigTableName() ) );

  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE INDEX IF NOT EXISTS %1 ON %2 (uri ASC);" )
         .arg( quotedQualifiedIdentifier( QStringLiteral( "uri_index" ), true ),
               quotedQualifiedIdentifier( methodConfigTableName() ) );

  if ( !authDbQuery( &query, qstr ) )
    return false;

  return true;
}

bool QgsAuthConfigurationStorageDb::createCertTables()
{
  QMutexLocker locker( &mMutex );

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "Creating cert tables in auth db" ), 2 );

  QSqlQuery query( authDatabaseConnection() );

  // create the tables
  QString qstr;

  qstr = QStringLiteral( "CREATE TABLE IF NOT EXISTS %1 (\n"
                         "   setting TEXT NOT NULL\n"
                         ", value TEXT);" ).arg( quotedQualifiedIdentifier( authSettingsTableName() ) );

  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE TABLE IF NOT EXISTS %1 (\n"
                         "    id TEXT NOT NULL,\n"
                         "    key TEXT NOT NULL\n"
                         ", cert TEXT NOT NULL);" ).arg( quotedQualifiedIdentifier( certIdentityTableName() ) );


  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE UNIQUE INDEX IF NOT EXISTS %1 ON %2 (id ASC);" )
         .arg( quotedQualifiedIdentifier( QStringLiteral( "cert_ident_id_index" ), true ), quotedQualifiedIdentifier( certIdentityTableName() ) );

  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();


  qstr = QStringLiteral( "CREATE TABLE IF NOT EXISTS %1 (\n"
                         "    id TEXT NOT NULL,\n"
                         "    host TEXT NOT NULL,\n"
                         "    cert TEXT\n"
                         ", config TEXT NOT NULL);" ).arg( quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) );

  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE UNIQUE INDEX IF NOT EXISTS %1 ON %2 (host ASC);" ).arg( quotedQualifiedIdentifier( QStringLiteral( "host_index" ), true ), quotedQualifiedIdentifier( sslCertCustomConfigTableName() ) );


  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();


  qstr = QStringLiteral( "CREATE TABLE IF NOT EXISTS %1 (\n"
                         "    id TEXT NOT NULL\n"
                         ", cert TEXT  NOT NULL);" ).arg( quotedQualifiedIdentifier( certAuthorityTableName() ) );


  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE UNIQUE INDEX IF NOT EXISTS %1 ON %2 (id ASC);" )
         .arg( quotedQualifiedIdentifier( QStringLiteral( "ca_id_index" ), true ), quotedQualifiedIdentifier( certAuthorityTableName() ) );


  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE TABLE IF NOT EXISTS %1 (\n"
                         "    id TEXT NOT NULL\n"
                         ", policy TEXT  NOT NULL);" ).arg( quotedQualifiedIdentifier( certTrustPolicyTableName() ) );

  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  qstr = QStringLiteral( "CREATE UNIQUE INDEX IF NOT EXISTS %1 ON %2 (id ASC);" )
         .arg( quotedQualifiedIdentifier( QStringLiteral( "trust_id_index" ), true ), quotedQualifiedIdentifier( certTrustPolicyTableName() ) );


  if ( !authDbQuery( &query, qstr ) )
    return false;

  query.clear();

  return true;
}

QgsAuthMethodConfigsMap QgsAuthConfigurationStorageDb::authMethodConfigs( const QStringList &allowedMethods ) const
{

  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration );

  QgsAuthMethodConfigsMap baseConfigs;

  if ( ! isEnabled() || !isReady() )
    return baseConfigs;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return baseConfigs;
  }

  QSqlQuery query( authDatabaseConnection() );
  query.prepare( QStringLiteral( "SELECT id, name, uri, type, version FROM %1" ).arg( quotedQualifiedIdentifier( methodConfigTableName() ) ) );

  if ( !authDbQuery( &query ) )
  {
    return baseConfigs;
  }

  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      QString authcfg = query.value( 0 ).toString();
      QgsAuthMethodConfig config;
      config.setId( authcfg );
      config.setName( query.value( 1 ).toString() );
      config.setUri( query.value( 2 ).toString() );
      config.setMethod( query.value( 3 ).toString() );
      config.setVersion( query.value( 4 ).toInt() );

      if ( !allowedMethods.isEmpty() && !allowedMethods.contains( config.method() ) )
      {
        continue;
      }

      baseConfigs.insert( authcfg, config );
    }
  }
  return baseConfigs;

}

QgsAuthMethodConfigsMap QgsAuthConfigurationStorageDb::authMethodConfigsWithPayload( ) const
{

  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration );

  QgsAuthMethodConfigsMap baseConfigs;

  if ( ! isEnabled() || !isReady() )
    return baseConfigs;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return baseConfigs;
  }

  QSqlQuery query( authDatabaseConnection() );
  query.prepare( QStringLiteral( "SELECT id, name, uri, type, version, config FROM %1" ).arg( quotedQualifiedIdentifier( methodConfigTableName() ) ) );

  if ( !authDbQuery( &query ) )
  {
    return baseConfigs;
  }

  if ( query.isActive() && query.isSelect() )
  {
    while ( query.next() )
    {
      QString authcfg = query.value( 0 ).toString();
      QgsAuthMethodConfig config;
      config.setId( authcfg );
      config.setName( query.value( 1 ).toString() );
      config.setUri( query.value( 2 ).toString() );
      config.setMethod( query.value( 3 ).toString() );
      config.setVersion( query.value( 4 ).toInt() );
      config.setConfig( QStringLiteral( "encrypted_payload" ), query.value( 5 ).toString() );
      baseConfigs.insert( authcfg, config );
    }
  }
  return baseConfigs;
}

void QgsAuthConfigurationStorageDb::checkCapabilities()
{
  QMutexLocker locker( &mMutex );

  mCapabilities = Qgis::AuthConfigurationStorageCapabilities();

  if ( ! isEnabled() || !isReady() )
    return;

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return;
  }

  // Check if each table exist and set capabilities

  static const QStringList existingTables = authDatabaseConnection().tables();
  QString schema { mConfiguration.value( QStringLiteral( "schema" ) ).toString() };
  if ( ! schema.isEmpty() )
  {
    schema += '.';
  }

  if ( existingTables.contains( schema.isEmpty() ? masterPasswordTableName( ) : schema + masterPasswordTableName( ) ) )
  {
    mCapabilities |= Qgis::AuthConfigurationStorageCapability::ReadMasterPassword;
    if ( !isReadOnly() )
    {
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::CreateMasterPassword;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::UpdateMasterPassword;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::DeleteMasterPassword;
    }
  }

  if ( existingTables.contains( schema.isEmpty() ? methodConfigTableName( ) : schema + methodConfigTableName( ) ) )
  {
    mCapabilities |= Qgis::AuthConfigurationStorageCapability::ReadConfiguration;
    if ( !isReadOnly() )
    {
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::CreateConfiguration;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::UpdateConfiguration;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::DeleteConfiguration;
    }
  }

  if ( existingTables.contains( schema.isEmpty() ? authSettingsTableName( ) : schema + authSettingsTableName( ) ) )
  {
    mCapabilities |= Qgis::AuthConfigurationStorageCapability::ReadSetting;
    if ( !isReadOnly() )
    {
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::CreateSetting;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::UpdateSetting;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::DeleteSetting;
    }
  }

  if ( existingTables.contains( schema.isEmpty() ? certIdentityTableName( ) : schema + certIdentityTableName( ) ) )
  {
    mCapabilities |= Qgis::AuthConfigurationStorageCapability::ReadCertificateIdentity;
    if ( !isReadOnly() )
    {
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::CreateCertificateIdentity;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::UpdateCertificateIdentity;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::DeleteCertificateIdentity;
    }
  }

  if ( existingTables.contains( schema.isEmpty() ? sslCertCustomConfigTableName( ) : schema +  sslCertCustomConfigTableName( ) ) )
  {
    mCapabilities |= Qgis::AuthConfigurationStorageCapability::ReadSslCertificateCustomConfig;
    if ( !isReadOnly() )
    {
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::CreateSslCertificateCustomConfig;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::UpdateSslCertificateCustomConfig;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::DeleteSslCertificateCustomConfig;
    }
  }

  if ( existingTables.contains( schema.isEmpty() ? certAuthorityTableName( ) : schema + certAuthorityTableName( ) ) )
  {
    mCapabilities |= Qgis::AuthConfigurationStorageCapability::ReadCertificateAuthority;
    if ( !isReadOnly() )
    {
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::CreateCertificateAuthority;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::UpdateCertificateAuthority;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::DeleteCertificateAuthority;
    }
  }

  if ( existingTables.contains( schema.isEmpty() ? certTrustPolicyTableName( ) : schema + certTrustPolicyTableName( ) ) )
  {
    mCapabilities |= Qgis::AuthConfigurationStorageCapability::ReadCertificateTrustPolicy;
    if ( !isReadOnly() )
    {
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::CreateCertificateTrustPolicy;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::UpdateCertificateTrustPolicy;
      mCapabilities |= Qgis::AuthConfigurationStorageCapability::DeleteCertificateTrustPolicy;
    }
  }

  // Any delete capability will set ClearStorage
  if ( ( mCapabilities & Qgis::AuthConfigurationStorageCapability::DeleteMasterPassword ) ||
       ( mCapabilities & Qgis::AuthConfigurationStorageCapability::DeleteConfiguration ) ||
       ( mCapabilities & Qgis::AuthConfigurationStorageCapability::DeleteSetting ) ||
       ( mCapabilities & Qgis::AuthConfigurationStorageCapability::DeleteCertificateIdentity ) ||
       ( mCapabilities & Qgis::AuthConfigurationStorageCapability::DeleteSslCertificateCustomConfig ) ||
       ( mCapabilities & Qgis::AuthConfigurationStorageCapability::DeleteCertificateAuthority ) ||
       ( mCapabilities & Qgis::AuthConfigurationStorageCapability::DeleteCertificateTrustPolicy ) )
  {
    mCapabilities |= Qgis::AuthConfigurationStorageCapability::ClearStorage;
  }

}

QgsAuthMethodConfig QgsAuthConfigurationStorageDb::loadMethodConfig( const QString &id, QString &payload, bool full ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration );

  QgsAuthMethodConfig config;

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ), Qgis::MessageLevel::Critical );
    return config;
  }

  QSqlQuery query( authDatabaseConnection() );

  if ( full )
  {
    query.prepare( QStringLiteral( "SELECT name, uri, type, version, config FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( methodConfigTableName() ) ) );
  }
  else
  {
    query.prepare( QStringLiteral( "SELECT name, uri, type, version FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( methodConfigTableName() ) ) );
  }

  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
  {
    return config;
  }

  if ( query.isActive() && query.isSelect() )
  {
    if ( query.first() )
    {
      config.setId( id );
      config.setName( query.value( 0 ).toString() );
      config.setUri( query.value( 1 ).toString() );
      config.setMethod( query.value( 2 ).toString() );
      config.setVersion( query.value( 3 ).toInt() );
      if ( full )
      {
        payload = query.value( 4 ).toString();
      }
    }
    if ( query.next() )
    {
      const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Authentication database contains more than one configuration IDs for '%1'" ).arg( id ), Qgis::MessageLevel::Warning );
    }
  }

  return config;
}

bool QgsAuthConfigurationStorageDb::storeMethodConfig( const QgsAuthMethodConfig &config, const QString &payload )
{
  QMutexLocker locker( &mMutex );

  if ( methodConfigExists( config.id() ) )
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::UpdateConfiguration );
  }
  else
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::CreateConfiguration );
  }

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  if ( payload.isEmpty() )
  {
    setError( tr( "Store config: FAILED because config string is empty" ), Qgis::MessageLevel::Warning );
    return false;
  }

  if ( ! config.isValid( true ) )
  {
    setError( tr( "Store config: FAILED because config is invalid" ), Qgis::MessageLevel::Warning );
    return false;
  }

  if ( methodConfigExists( config.id() ) )
  {
    removeMethodConfig( config.id() );
  }

  QSqlQuery query( authDatabaseConnection() );
  query.prepare( QStringLiteral( "INSERT INTO %1 (id, name, uri, type, version, config) VALUES (:id, :name, :uri, :type, :version, :config)" ).arg( quotedQualifiedIdentifier( methodConfigTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), config.id() );
  query.bindValue( QStringLiteral( ":name" ), config.name() );
  query.bindValue( QStringLiteral( ":uri" ), config.uri() );
  query.bindValue( QStringLiteral( ":type" ), config.method() );
  query.bindValue( QStringLiteral( ":version" ), config.version() );
  query.bindValue( QStringLiteral( ":config" ), payload );

  if ( !authDbQuery( &query ) )
  {
    setError( tr( "Failed to store config '%1'" ).arg( config.id() ), Qgis::MessageLevel::Critical );
    return false;
  }

  emit methodConfigChanged();

  return true;
}

bool QgsAuthConfigurationStorageDb::removeMethodConfig( const QString &id )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteConfiguration );

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "DELETE FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( methodConfigTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
  {
    setError( tr( "Failed to remove config '%1'" ).arg( id ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( query.numRowsAffected() == 0 )
  {
    setError( tr( "Config '%1' does not exist" ).arg( id ), Qgis::MessageLevel::Warning );
    return false;
  }

  emit methodConfigChanged();

  return true;
}

bool QgsAuthConfigurationStorageDb::methodConfigExists( const QString &id ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadConfiguration );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT COUNT( id ) FROM %1 WHERE id = :id" ).arg( quotedQualifiedIdentifier( methodConfigTableName() ) ) );
  query.bindValue( QStringLiteral( ":id" ), id );

  if ( !authDbQuery( &query ) )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Failed to query for config '%1'" ).arg( id ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( query.next() )
  {
    return query.value( 0 ).toInt() > 0;
  }

  return false;
}

bool QgsAuthConfigurationStorageDb::storeAuthSetting( const QString &key, const QString &value )
{
  QMutexLocker locker( &mMutex );

  if ( authSettingExists( key ) )
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::UpdateSetting );
    removeAuthSetting( key );
  }
  else
  {
    checkCapability( Qgis::AuthConfigurationStorageCapability::CreateSetting );
  }

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "INSERT INTO %1 (setting, value) VALUES (:setting, :value)" )
                 .arg( quotedQualifiedIdentifier( authSettingsTableName() ) ) );
  query.bindValue( QStringLiteral( ":setting" ), key );
  query.bindValue( QStringLiteral( ":value" ), value );

  if ( !authDbQuery( &query ) )
  {
    setError( tr( "Failed to set setting '%1'" ).arg( key ), Qgis::MessageLevel::Critical );
    return false;
  }
  else
  {
    emit authSettingsChanged();
    return true;
  }
}

QString QgsAuthConfigurationStorageDb::loadAuthSetting( const QString &key ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadSetting );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return QString();
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT value FROM %1 WHERE setting = :setting" ).arg( quotedQualifiedIdentifier( authSettingsTableName() ) ) );
  query.bindValue( QStringLiteral( ":setting" ), key );

  if ( !authDbQuery( &query ) )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Failed to query for setting '%1'" ).arg( key ), Qgis::MessageLevel::Critical );
    return QString();
  }

  if ( query.next() )
  {
    return query.value( 0 ).toString();
  }

  // Not sure we need this warning, as it's not necessarily an error if the setting doesn't exist
  // const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Setting '%1' does not exist" ).arg( key ), Qgis::MessageLevel::Warning );
  QgsDebugMsgLevel( QStringLiteral( "Setting '%1' does not exist" ).arg( key ), 2 );

  return QString();
}


bool QgsAuthConfigurationStorageDb::removeAuthSetting( const QString &key )
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteSetting );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "DELETE FROM %1 WHERE setting = :setting" ).arg( quotedQualifiedIdentifier( authSettingsTableName() ) ) );
  query.bindValue( QStringLiteral( ":setting" ), key );

  if ( !authDbQuery( &query ) )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Failed to remove setting '%1'" ).arg( key ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( query.numRowsAffected() == 0 )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Setting '%1' does not exist" ).arg( key ), Qgis::MessageLevel::Warning );
    return false;
  }

  emit authSettingsChanged();

  return true;
}

bool QgsAuthConfigurationStorageDb::authSettingExists( const QString &key ) const
{
  QMutexLocker locker( &mMutex );

  checkCapability( Qgis::AuthConfigurationStorageCapability::ReadSetting );

  if ( !authDbOpen() )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  query.prepare( QStringLiteral( "SELECT COUNT(value) FROM %1 WHERE setting = :setting" ).arg( quotedQualifiedIdentifier( authSettingsTableName() ) ) );
  query.bindValue( QStringLiteral( ":setting" ), key );

  if ( !authDbQuery( &query ) )
  {
    const_cast< QgsAuthConfigurationStorageDb * >( this )->setError( tr( "Failed to query for setting '%1'" ).arg( key ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( query.next() )
  {
    return query.value( 0 ).toInt() > 0;
  }

  return false;
}



bool QgsAuthConfigurationStorageDb::clearTables( const QStringList &tables )
{
  QMutexLocker locker( &mMutex );

  if ( !authDbOpen() )
  {
    setError( tr( "Auth db could not be opened" ) );
    return false;
  }

  QSqlQuery query( authDatabaseConnection() );

  for ( const auto &table : std::as_const( tables ) )
  {

    // Check if the table exists
    if ( ! tableExists( table ) )
    {
      setError( tr( "Failed to empty table '%1': table does not exist" ).arg( table ), Qgis::MessageLevel::Warning );
      continue;
    }

    // Check whether the table supports deletion
    if ( table.compare( methodConfigTableName(), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteConfiguration );
    }
    else if ( table.compare( authSettingsTableName(), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteSetting );
    }
    else if ( table.compare( certIdentityTableName(), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteCertificateIdentity );
    }
    else if ( table.compare( sslCertCustomConfigTableName(), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteSslCertificateCustomConfig );
    }
    else if ( table.compare( certAuthorityTableName(), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteCertificateAuthority );
    }
    else if ( table.compare( certTrustPolicyTableName(), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteCertificateTrustPolicy );
    }
    else if ( table.compare( masterPasswordTableName(), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      checkCapability( Qgis::AuthConfigurationStorageCapability::DeleteMasterPassword );
    }
    else
    {
      // Unsupported table: it should not happen!
      throw QgsNotSupportedException( tr( "Failed to empty table '%1': unsupported table" ).arg( table ) );
    }

    query.prepare( QStringLiteral( "DELETE FROM %1" ).arg( quotedQualifiedIdentifier( table ) ) );

    if ( !authDbQuery( &query ) )
    {
      setError( tr( "Failed to empty table '%1'" ).arg( quotedQualifiedIdentifier( table ) ) );
      return false;
    }
  }
  return true;
}

bool QgsAuthConfigurationStorageDb::tableExists( const QString &table ) const
{
  QString schema { mConfiguration.value( QStringLiteral( "schema" ) ).toString() };
  if ( ! schema.isEmpty() )
  {
    schema += '.';
  }
  return authDatabaseConnection().tables().contains( schema + table );
}

const QMap<QString, QVariant> QgsAuthConfigurationStorageDb::uriToSettings( const QString &uri )
{
  QUrl url( uri );
  QMap<QString, QVariant> settings;

  if ( url.isValid() )
  {
    settings.insert( QStringLiteral( "driver" ), url.scheme().toUpper() );
    settings.insert( QStringLiteral( "host" ), url.host() );
    settings.insert( QStringLiteral( "port" ), QString::number( url.port() ) );
    QString path { url.path() };
    // Remove leading slash from the path unless the driver is QSQLITE or QSPATIALITE
    if ( path.startsWith( QLatin1Char( '/' ) ) &&
         !( settings.value( QStringLiteral( "driver" ) ) == QLatin1String( "QSQLITE" ) ||
            settings.value( QStringLiteral( "driver" ) ) == QLatin1String( "QSPATIALITE" ) ) )
    {
      path = path.mid( 1 );
    }
    settings.insert( QStringLiteral( "database" ), path );
    settings.insert( QStringLiteral( "user" ), url.userName() );
    settings.insert( QStringLiteral( "password" ), url.password() );
    QUrlQuery query{ url };

    // Extract the schema from the query string
    QString schemaName { query.queryItemValue( QStringLiteral( "schema" ) ) };
    if ( schemaName.isEmpty() )
    {
      schemaName = query.queryItemValue( QStringLiteral( "SCHEMA" ) );
    }

    if ( ! schemaName.isEmpty() )
    {
      settings.insert( QStringLiteral( "schema" ), schemaName );
      query.removeAllQueryItems( QStringLiteral( "schema" ) );
      query.removeAllQueryItems( QStringLiteral( "SCHEMA" ) );
    }

    settings.insert( QStringLiteral( "options" ), query.toString() );
  }
  return settings;
}

bool QgsAuthConfigurationStorageDb::clearMethodConfigs()
{
  if ( clearTables( {{ methodConfigTableName( ) }} ) )
  {
    emit methodConfigChanged();
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsAuthConfigurationStorageDb::erase()
{

  checkCapability( Qgis::AuthConfigurationStorageCapability::ClearStorage );

  if ( clearTables( {{
    methodConfigTableName(),
      authSettingsTableName(),
      certIdentityTableName(),
      sslCertCustomConfigTableName(),
      certAuthorityTableName(),
      certTrustPolicyTableName(),
      masterPasswordTableName()
    }} ) )
  {
    emit storageChanged( id( ) );
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsAuthConfigurationStorageDb::isReady() const
{
  QMutexLocker locker( &mMutex );
  return mIsReady;
}
