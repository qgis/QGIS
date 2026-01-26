/***************************************************************************
     testqgsauthmanager.cpp
     ----------------------
    Date                 : October 2015
    Copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    Author               : Larry Shaffer
    Email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsauthconfig.h"
#include "qgsauthmanager.h"
#include "qgsauthmethodmetadata.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QtTest/QSignalSpy>

/**
 * \ingroup UnitTests
 * Unit tests for QgsAuthManager
 */
class TestQgsAuthManager : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsAuthManager();

  public slots:

    void doSync();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testMasterPassword();
    void testAuthConfigs();
    void testAuthMethods();
    void testPasswordHelper();

  private:
    void cleanupTempDir();
    QList<QgsAuthMethodConfig> registerAuthConfigs();

    void reportRow( const QString &msg );
    void reportHeader( const QString &msg );

    QString mPkiData;
    QString mTempDir;
    const char *mPass = nullptr;
};


TestQgsAuthManager::TestQgsAuthManager()
  : QgsTest( u"QgsAuthManager Tests"_s )
  , mPkiData( QStringLiteral( TEST_DATA_DIR ) + "/auth_system/certs_keys" )
  , mTempDir( QDir::tempPath() + "/auth" )
  , mPass( "pass" )
{
}

void TestQgsAuthManager::initTestCase()
{
  cleanupTempDir();

  // make QGIS_AUTH_DB_DIR_PATH temp dir for qgis-auth.db and master password file
  const QDir tmpDir = QDir::temp();
  QVERIFY2( tmpDir.mkpath( mTempDir ), "Couldn't make temp directory" );
  qputenv( "QGIS_AUTH_DB_DIR_PATH", mTempDir.toLatin1() );

  // init app and auth manager
  QgsApplication::init();
  QgsApplication::initQgis();
  QVERIFY2( !QgsApplication::authManager()->isDisabled(), "Authentication system is DISABLED" );

  // verify QGIS_AUTH_DB_DIR_PATH (temp auth db path) worked
  Q_NOWARN_DEPRECATED_PUSH
  const QString db1( QFileInfo( QgsApplication::authManager()->authenticationDatabasePath() ).canonicalFilePath() );
  Q_NOWARN_DEPRECATED_POP
  const QString db2( QFileInfo( mTempDir + "/qgis-auth.db" ).canonicalFilePath() );
  QCOMPARE( db1, db2 );

  // verify master pass can be set manually
  // (this also creates a fresh password hash in the new temp database)
  QVERIFY2( QgsApplication::authManager()->setMasterPassword( mPass, true ), "Master password could not be set" );
  QVERIFY2( QgsApplication::authManager()->masterPasswordIsSet(), "Auth master password not set from passed string" );

  // create QGIS_AUTH_PASSWORD_FILE file
  const QString passfilepath = mTempDir + "/passfile";
  QFile passfile( passfilepath );
  if ( passfile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    QTextStream fout( &passfile );
    fout << QString( mPass ) << "\r\n";
    passfile.close();
    qputenv( "QGIS_AUTH_PASSWORD_FILE", passfilepath.toLatin1() );
  }
  // qDebug( "QGIS_AUTH_PASSWORD_FILE=%s", qgetenv( "QGIS_AUTH_PASSWORD_FILE" ).constData() );

  // re-init app and auth manager
  QgsApplication::quit();
  // QTest::qSleep( 3000 );
  QgsApplication::init();
  QgsApplication::initQgis();
  QVERIFY2( !QgsApplication::authManager()->isDisabled(), "Authentication system is DISABLED" );

  // verify QGIS_AUTH_PASSWORD_FILE worked, when compared against hash in db
  QVERIFY2( QgsApplication::authManager()->masterPasswordIsSet(), "Auth master password not set from QGIS_AUTH_PASSWORD_FILE" );

  // all tests should now have a valid qgis-auth.db and stored/set master password
}

void TestQgsAuthManager::cleanup()
{
  // Restore password_helper_insecure_fallback value
  QgsSettings settings;
  settings.setValue( u"password_helper_insecure_fallback"_s, false, QgsSettings::Section::Auth );
}

void TestQgsAuthManager::cleanupTempDir()
{
  QDir tmpDir = QDir( mTempDir );
  if ( tmpDir.exists() )
  {
    for ( const QString &tf : tmpDir.entryList( QDir::NoDotAndDotDot | QDir::Files ) )
    {
      QVERIFY2( tmpDir.remove( mTempDir + '/' + tf ), qPrintable( "Could not remove " + mTempDir + '/' + tf ) );
    }
    QVERIFY2( tmpDir.rmdir( mTempDir ), qPrintable( "Could not remove directory " + mTempDir ) );
  }
}

void TestQgsAuthManager::cleanupTestCase()
{
  QgsApplication::exitQgis();
  cleanupTempDir();
}

void TestQgsAuthManager::reportRow( const QString &msg )
{
  mReport += msg + "<br>\n";
}

void TestQgsAuthManager::reportHeader( const QString &msg )
{
  mReport += "<h3>" + msg + "</h3>\n";
}

void TestQgsAuthManager::testMasterPassword()
{
  // password is already stored/set in initTestCase()
  // NOTE: leave it in the same state when done with this test
  QgsAuthManager *authm = QgsApplication::authManager();

  QVERIFY( authm->masterPasswordIsSet() );
  QVERIFY( authm->masterPasswordHashInDatabase() );
  QVERIFY( authm->masterPasswordSame( mPass ) );
  QVERIFY( !authm->masterPasswordSame( "wrongpass" ) );
  QVERIFY( authm->setMasterPassword() );
  QVERIFY( authm->verifyMasterPassword() );
  QVERIFY( authm->verifyMasterPassword( mPass ) );
  QVERIFY( !authm->verifyMasterPassword( "wrongpass" ) );

  QList<QVariant> spyargs;
  QSignalSpy spy( authm, SIGNAL( masterPasswordVerified( bool ) ) );

  QVERIFY( authm->setMasterPassword( true ) );
  QCOMPARE( spy.count(), 1 );
  spyargs = spy.takeFirst();
  QVERIFY( spyargs.at( 0 ).userType() == QMetaType::Type::Bool );
  QVERIFY( spyargs.at( 0 ).toBool() );

  authm->clearMasterPassword();
  QVERIFY( !authm->masterPasswordIsSet() );
  QVERIFY( !authm->setMasterPassword( "wrongpass", true ) );
  QVERIFY( !authm->masterPasswordIsSet() );
  QCOMPARE( spy.count(), 1 );
  spyargs = spy.takeFirst();
  QVERIFY( spyargs.at( 0 ).userType() == QMetaType::Type::Bool );
  QVERIFY( !spyargs.at( 0 ).toBool() );

  authm->clearMasterPassword();
  QVERIFY( !authm->masterPasswordIsSet() );
  QVERIFY( authm->setMasterPassword( mPass, true ) );
  QVERIFY( authm->masterPasswordIsSet() );
  QCOMPARE( spy.count(), 1 );
  spyargs = spy.takeFirst();
  QVERIFY( spyargs.at( 0 ).userType() == QMetaType::Type::Bool );
  QVERIFY( spyargs.at( 0 ).toBool() );
}

void TestQgsAuthManager::testAuthConfigs()
{
  QList<QgsAuthMethodConfig> configs( registerAuthConfigs() );
  QVERIFY( !configs.isEmpty() );

  QgsAuthManager *authm = QgsApplication::authManager();

  // test storing/loading/updating
  for ( QgsAuthMethodConfig config : configs )
  {
    QVERIFY( config.isValid() );

    QVERIFY( authm->storeAuthenticationConfig( config ) );

    // config should now have a valid, unique ID
    const QString configid( config.id() );
    QVERIFY( !configid.isEmpty() );
    QVERIFY( !authm->configIdUnique( configid ) ); // uniqueness registered, so can't be overridden
    QVERIFY( authm->configIds().contains( configid ) );

    // config -> method map should have been updated and return the right method key
    QCOMPARE( authm->configAuthMethodKey( configid ), config.method() );

    // loading into new base config should return same as original stored
    QgsAuthMethodConfig config2;
    QVERIFY( authm->loadAuthenticationConfig( configid, config2, false ) );
    QVERIFY( config2.isValid() );
    QVERIFY( config2.configMap().isEmpty() ); // has no sensitive data

    QVERIFY( authm->loadAuthenticationConfig( configid, config2, true ) );
    QVERIFY( config2.isValid() );
    QVERIFY( !config2.configMap().isEmpty() ); // has sensitive data

    QVERIFY( config == config2 );

    // values haven't been changed, but db update should take place and values should roundtrip
    QVERIFY( authm->updateAuthenticationConfig( config2 ) );
    QVERIFY( authm->loadAuthenticationConfig( configid, config2, true ) );
    QVERIFY( config2.isValid() );
    QVERIFY( !config2.configMap().isEmpty() ); // has sensitive data
    QVERIFY( config == config2 );

    // changed config should update then correctly roundtrip
    const QgsStringMap configMap = config2.configMap();
    for ( auto it = configMap.constBegin(); it != configMap.constEnd(); it++ )
    {
      config2.setConfig( it.key(), it.value() + "changed" );
    }
    config2.setName( config2.name() + "changed" );
    config2.setUri( config2.uri() + "changed" );
    QVERIFY( authm->updateAuthenticationConfig( config2 ) );

    QgsAuthMethodConfig config3;
    QVERIFY( authm->loadAuthenticationConfig( configid, config3, true ) );
    QVERIFY( config3.isValid() );
    QVERIFY( !config3.configMap().isEmpty() );
    QVERIFY( config != config3 );
    QVERIFY( config2 == config3 );

    // config can be deleted
    QVERIFY( authm->removeAuthenticationConfig( configid ) );
    QVERIFY( !authm->configIds().contains( configid ) );
    QVERIFY( authm->configIds().isEmpty() );

    // config with custom id can be stored
    const QString customid( authm->uniqueConfigId() );
    config2.setId( customid );

    QVERIFY( authm->storeAuthenticationConfig( config2 ) );
    QCOMPARE( config2.id(), customid );
    QVERIFY( config2 != config3 ); // id should be different

    // custom configid can be deleted
    QVERIFY( authm->removeAuthenticationConfig( customid ) );
    QVERIFY( !authm->configIds().contains( customid ) );
    QVERIFY( authm->configIds().isEmpty() );
  }

  // verify cleanup of test configs
  QVERIFY( authm->configIds().isEmpty() );
  QVERIFY( authm->availableAuthMethodConfigs().isEmpty() );

  // test bulk operations
  configs.clear();
  configs = registerAuthConfigs();
  QVERIFY( !configs.isEmpty() );

  // test storing, then retrieving configid -> config map
  QgsAuthMethodConfigsMap idcfgmap;
  for ( QgsAuthMethodConfig config : configs )
  {
    QVERIFY( authm->storeAuthenticationConfig( config ) );
    idcfgmap.insert( config.id(), config );
  }

  QCOMPARE( authm->availableAuthMethodConfigs().size(), 3 );

  // Password-less export / import
  QVERIFY( authm->exportAuthenticationConfigsToXml( mTempDir + u"/configs.xml"_s, idcfgmap.keys() ) );
  QVERIFY( authm->removeAllAuthenticationConfigs() );
  QVERIFY( authm->importAuthenticationConfigsFromXml( mTempDir + u"/configs.xml"_s ) );

  QCOMPARE( authm->availableAuthMethodConfigs().size(), 3 );

  // Password-protected export / import
  QVERIFY( authm->exportAuthenticationConfigsToXml( mTempDir + u"/configs.xml"_s, idcfgmap.keys(), u"1234"_s ) );
  QVERIFY( authm->removeAllAuthenticationConfigs() );
  QVERIFY( authm->importAuthenticationConfigsFromXml( mTempDir + u"/configs.xml"_s, u"1234"_s ) );

  QgsAuthMethodConfigsMap authmap( authm->availableAuthMethodConfigs() );
  QCOMPARE( authmap.size(), 3 );

  QgsAuthMethodConfigsMap::iterator it = authmap.begin();
  for ( it = authmap.begin(); it != authmap.end(); ++it )
  {
    const QString cfgid = it.key();
    if ( !idcfgmap.contains( cfgid ) )
      continue;

    const QgsAuthMethodConfig cfg = it.value();
    const QgsAuthMethodConfig origcfg = idcfgmap.take( cfgid );
    QCOMPARE( origcfg.id(), cfg.id() );
    QCOMPARE( origcfg.name(), cfg.name() );
    QCOMPARE( origcfg.method(), cfg.method() );
    QCOMPARE( origcfg.uri(), cfg.uri() );
  }
  QCOMPARE( idcfgmap.size(), 0 );

  QVERIFY( authm->removeAllAuthenticationConfigs() );
}

void TestQgsAuthManager::testAuthMethods()
{
  const QList<QgsAuthMethodConfig> configs( registerAuthConfigs() );
  QVERIFY( !configs.isEmpty() );

  QgsAuthManager *authm = QgsApplication::authManager();

  for ( QgsAuthMethodConfig config : configs )
  {
    QVERIFY( config.isValid() );
    QVERIFY( authm->storeAuthenticationConfig( config ) );
    // config should now have a valid, unique ID
    // (see testAuthConfigs for further config testing)
    const QString configid( config.id() );

    // correct method, loaded from core auth method plugin registry, should be returned
    const QString key = authm->configAuthMethodKey( configid );
    const QgsAuthMethodMetadata *meta = authm->authMethodMetadata( key );
    QVERIFY( meta );
    QCOMPARE( meta->key(), config.method() );
  }
  QVERIFY( authm->removeAllAuthenticationConfigs() );
}

QList<QgsAuthMethodConfig> TestQgsAuthManager::registerAuthConfigs()
{
  QList<QgsAuthMethodConfig> configs;

  // Basic
  QgsAuthMethodConfig b_config;
  b_config.setName( u"Basic"_s );
  b_config.setMethod( u"Basic"_s );
  b_config.setUri( u"http://example.com"_s );
  b_config.setConfig( u"username"_s, u"username"_s );
  b_config.setConfig( u"password"_s, u"password"_s );
  b_config.setConfig( u"realm"_s, u"Realm"_s );
  if ( !b_config.isValid() )
  {
    return configs;
  }

  // PKI-Paths
  QgsAuthMethodConfig p_config;
  p_config.setName( u"PKI-Paths"_s );
  p_config.setMethod( u"PKI-Paths"_s );
  p_config.setUri( u"http://example.com"_s );
  p_config.setConfig( u"certpath"_s, mPkiData + "/gerardus_cert.pem" );
  p_config.setConfig( u"keypath"_s, mPkiData + "gerardus_key_w-pass.pem" );
  if ( !p_config.isValid() )
  {
    return configs;
  }

  // PKI-PKCS#12
  QgsAuthMethodConfig k_config;
  k_config.setName( u"PKI-PKCS#12"_s );
  k_config.setMethod( u"PKI-PKCS#12"_s );
  k_config.setUri( u"http://example.com"_s );
  k_config.setConfig( u"bundlepath"_s, mPkiData + "/gerardus.p12" );
  k_config.setConfig( u"bundlepass"_s, u"password"_s );
  if ( !k_config.isValid() )
  {
    return configs;
  }

  // do this last, so we are assured to have all core configs
  configs << b_config << p_config << k_config;
  return configs;
}


void TestQgsAuthManager::doSync()
{
  QgsAuthManager *authm = QgsApplication::authManager();
  QVERIFY( authm->passwordHelperSync() );
}

void TestQgsAuthManager::testPasswordHelper()
{
  if ( QgsTest::isCIRun() )
  {
    {
      QSKIP( "QtKeyChain not working on CI (requires dbus)" );
    }
  }
  QgsAuthManager *authm = QgsApplication::authManager();
  authm->clearMasterPassword();

  QgsSettings settings;
  settings.setValue( u"password_helper_insecure_fallback"_s, true, QgsSettings::Section::Auth );

  // Test enable/disable
  // It should be enabled by default
  QVERIFY( authm->passwordHelperEnabled() );
  authm->setPasswordHelperEnabled( false );
  QVERIFY( !authm->passwordHelperEnabled() );
  authm->setPasswordHelperEnabled( true );
  QVERIFY( authm->passwordHelperEnabled() );

  // Sync with wallet
  QVERIFY( authm->setMasterPassword( mPass, true ) );
  QVERIFY( authm->masterPasswordIsSet() );
  QObject::connect( authm, &QgsAuthManager::passwordHelperSuccess, QApplication::instance(), &QCoreApplication::quit );
  QObject::connect( authm, &QgsAuthManager::passwordHelperFailure, QApplication::instance(), &QCoreApplication::quit );
  QMetaObject::invokeMethod( this, "doSync", Qt::QueuedConnection );
  qApp->exec();
  authm->clearMasterPassword();
  QVERIFY( authm->setMasterPassword() );
  QVERIFY( authm->masterPasswordIsSet() );

  // Delete from wallet
  authm->clearMasterPassword();
  QVERIFY( authm->passwordHelperDelete() );
  QVERIFY( !authm->setMasterPassword() );
  QVERIFY( !authm->masterPasswordIsSet() );

  // Re-sync
  QVERIFY( authm->setMasterPassword( mPass, true ) );
  QMetaObject::invokeMethod( this, "doSync", Qt::QueuedConnection );
  qApp->exec();
  authm->clearMasterPassword();
  QVERIFY( authm->setMasterPassword() );
  QVERIFY( authm->masterPasswordIsSet() );
}

QGSTEST_MAIN( TestQgsAuthManager )
#include "testqgsauthmanager.moc"
