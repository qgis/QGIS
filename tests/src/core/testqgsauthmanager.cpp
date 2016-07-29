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

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QObject>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsauthconfig.h"

/** \ingroup UnitTests
 * Unit tests for QgsAuthManager
 */
class TestQgsAuthManager: public QObject
{
    Q_OBJECT

  public:
    TestQgsAuthManager();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup() {}

    void testMasterPassword();
    void testAuthConfigs();
    void testAuthMethods();

  private:
    void cleanupTempDir();
    QList<QgsAuthMethodConfig> registerAuthConfigs();

    void reportRow( const QString& msg );
    void reportHeader( const QString& msg );

    QString mPkiData;
    QString mTempDir;
    const char* mPass;
    QString mReport;
};


TestQgsAuthManager::TestQgsAuthManager()
    : mPkiData( QString( TEST_DATA_DIR ) + "/auth_system/certs_keys" )
    , mTempDir( QDir::tempPath() + "/auth" )
    , mPass( "pass" )
{
}

void TestQgsAuthManager::initTestCase()
{
  cleanupTempDir();

  mReport += "<h1>QgsAuthManager Tests</h1>\n";

  // make QGIS_AUTH_DB_DIR_PATH temp dir for qgis-auth.db and master password file
  QDir tmpDir = QDir::temp();
  QVERIFY2( tmpDir.mkpath( mTempDir ), "Couldn't make temp directory" );
  qputenv( "QGIS_AUTH_DB_DIR_PATH", mTempDir.toAscii() );

  // init app and auth manager
  QgsApplication::init();
  QgsApplication::initQgis();
  QVERIFY2( !QgsAuthManager::instance()->isDisabled(),
            "Authentication system is DISABLED" );

  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( '\n', "<br />\n" );
  mReport += "<p>" + mySettings + "</p>\n";

  // verify QGIS_AUTH_DB_DIR_PATH (temp auth db path) worked
  QString db1( QFileInfo( QgsAuthManager::instance()->authenticationDbPath() ).canonicalFilePath() );
  QString db2( QFileInfo( mTempDir + "/qgis-auth.db" ).canonicalFilePath() );
  QVERIFY2( db1 == db2, "Auth db temp path does not match db path of manager" );

  // verify master pass can be set manually
  // (this also creates a fresh password hash in the new temp database)
  QVERIFY2( QgsAuthManager::instance()->setMasterPassword( mPass, true ),
            "Master password could not be set" );
  QVERIFY2( QgsAuthManager::instance()->masterPasswordIsSet(),
            "Auth master password not set from passed string" );

  // create QGIS_AUTH_PASSWORD_FILE file
  QString passfilepath = mTempDir + "/passfile";
  QFile passfile( passfilepath );
  if ( passfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream fout( &passfile );
    fout << QString( mPass ) << "\r\n";
    passfile.close();
    qputenv( "QGIS_AUTH_PASSWORD_FILE", passfilepath.toAscii() );
  }
  // qDebug( "QGIS_AUTH_PASSWORD_FILE=%s", qgetenv( "QGIS_AUTH_PASSWORD_FILE" ).constData() );

  // re-init app and auth manager
  QgsApplication::quit();
  // QTest::qSleep( 3000 );
  QgsApplication::init();
  QgsApplication::initQgis();
  QVERIFY2( !QgsAuthManager::instance()->isDisabled(),
            "Authentication system is DISABLED" );

  // verify QGIS_AUTH_PASSWORD_FILE worked, when compared against hash in db
  QVERIFY2( QgsAuthManager::instance()->masterPasswordIsSet(),
            "Auth master password not set from QGIS_AUTH_PASSWORD_FILE" );

  // all tests should now have a valid qgis-auth.db and stored/set master password
}

void TestQgsAuthManager::cleanupTempDir()
{
  QDir tmpDir = QDir( mTempDir );
  if ( tmpDir.exists() )
  {
    Q_FOREACH ( const QString &tf, tmpDir.entryList( QDir::NoDotAndDotDot | QDir::Files ) )
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

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    // QDesktopServices::openUrl( "file:///" + myReportFile );
  }
}

void TestQgsAuthManager::init()
{
  mReport += "<h2>" + QString( QTest::currentTestFunction() ) + "</h2>\n";
}

void TestQgsAuthManager::reportRow( const QString& msg )
{
  mReport += msg + "<br>\n";
}

void TestQgsAuthManager::reportHeader( const QString& msg )
{
  mReport += "<h3>" + msg + "</h3>\n";
}

void TestQgsAuthManager::testMasterPassword()
{
  // password is already stored/set in initTestCase()
  // NOTE: leave it in the same state when done with this test
  QgsAuthManager *authm = QgsAuthManager::instance();

  QVERIFY( authm->masterPasswordIsSet() );
  QVERIFY( authm->masterPasswordHashInDb() );
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
  QVERIFY( spyargs.at( 0 ).type() == QVariant::Bool );
  QVERIFY( spyargs.at( 0 ).toBool() == true );

  authm->clearMasterPassword();
  QVERIFY( !authm->masterPasswordIsSet() );
  QVERIFY( !authm->setMasterPassword( "wrongpass" , true ) );
  QVERIFY( !authm->masterPasswordIsSet() );
  QCOMPARE( spy.count(), 1 );
  spyargs = spy.takeFirst();
  QVERIFY( spyargs.at( 0 ).type() == QVariant::Bool );
  QVERIFY( spyargs.at( 0 ).toBool() == false );

  authm->clearMasterPassword();
  QVERIFY( !authm->masterPasswordIsSet() );
  QVERIFY( authm->setMasterPassword( mPass, true ) );
  QVERIFY( authm->masterPasswordIsSet() );
  QCOMPARE( spy.count(), 1 );
  spyargs = spy.takeFirst();
  QVERIFY( spyargs.at( 0 ).type() == QVariant::Bool );
  QVERIFY( spyargs.at( 0 ).toBool() == true );
}

void TestQgsAuthManager::testAuthConfigs()
{
  QList<QgsAuthMethodConfig> configs( registerAuthConfigs() );
  QVERIFY( !configs.isEmpty() );

  QgsAuthManager *authm = QgsAuthManager::instance();

  // test storing/loading/updating
  Q_FOREACH ( QgsAuthMethodConfig config, configs )
  {
    QVERIFY( config.isValid() );

    QVERIFY( authm->storeAuthenticationConfig( config ) );

    // config should now have a valid, unique ID
    QString configid( config.id() );
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
    Q_FOREACH ( const QString &key, config2.configMap().keys() )
    {
      config2.setConfig( key, config2.configMap().value( key ) + "changed" );
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
    QString customid( authm->uniqueConfigId() );
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
  Q_FOREACH ( QgsAuthMethodConfig config, configs )
  {
    QVERIFY( authm->storeAuthenticationConfig( config ) );
    idcfgmap.insert( config.id(), config );
  }
  QgsAuthMethodConfigsMap authmap( authm->availableAuthMethodConfigs() );
  QCOMPARE( authmap.size(), 3 );

  QgsAuthMethodConfigsMap::iterator it = authmap.begin();
  for ( it = authmap.begin(); it != authmap.end(); ++it )
  {
    QString cfgid = it.key();
    if ( !idcfgmap.contains( cfgid ) )
      continue;

    QgsAuthMethodConfig cfg = it.value();
    QgsAuthMethodConfig origcfg = idcfgmap.take( cfgid );
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
  QList<QgsAuthMethodConfig> configs( registerAuthConfigs() );
  QVERIFY( !configs.isEmpty() );

  QgsAuthManager *authm = QgsAuthManager::instance();

  Q_FOREACH ( QgsAuthMethodConfig config, configs )
  {
    QVERIFY( config.isValid() );
    QVERIFY( authm->storeAuthenticationConfig( config ) );
    // config should now have a valid, unique ID
    // (see testAuthConfigs for further config testing)
    QString configid( config.id() );

    // correct method, loaded from core auth method plugin registry, should be returned
    QgsAuthMethod *authmethod = authm->configAuthMethod( configid );
    QVERIFY( authmethod );
    QCOMPARE( authmethod->key(), config.method() );


  }
  QVERIFY( authm->removeAllAuthenticationConfigs() );
}

QList<QgsAuthMethodConfig> TestQgsAuthManager::registerAuthConfigs()
{
  QList<QgsAuthMethodConfig> configs;

  // Basic
  QgsAuthMethodConfig b_config;
  b_config.setName( "Basic" );
  b_config.setMethod( "Basic" );
  b_config.setUri( "http://example.com" );
  b_config.setConfig( "username", "username" );
  b_config.setConfig( "password", "password" );
  b_config.setConfig( "realm", "Realm" );
  if ( !b_config.isValid() )
  {
    return configs;
  }

  // PKI-Paths
  QgsAuthMethodConfig p_config;
  p_config.setName( "PKI-Paths" );
  p_config.setMethod( "PKI-Paths" );
  p_config.setUri( "http://example.com" );
  p_config.setConfig( "certpath", mPkiData + "/gerardus_cert.pem" );
  p_config.setConfig( "keypath", mPkiData + "gerardus_key_w-pass.pem" );
  if ( !p_config.isValid() )
  {
    return configs;
  }

  // PKI-PKCS#12
  QgsAuthMethodConfig k_config;
  k_config.setName( "PKI-PKCS#12" );
  k_config.setMethod( "PKI-PKCS#12" );
  k_config.setUri( "http://example.com" );
  k_config.setConfig( "bundlepath", mPkiData + "/gerardus.p12" );
  k_config.setConfig( "bundlepass", "password" );
  if ( !k_config.isValid() )
  {
    return configs;
  }

  // do this last, so we are assured to have all core configs
  configs << b_config << p_config << k_config;
  return configs;
}

QTEST_MAIN( TestQgsAuthManager )
#include "testqgsauthmanager.moc"
