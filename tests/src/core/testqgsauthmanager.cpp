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
    void init() {}
    void cleanup() {}

    void testMasterPassword();

  private:
    QString mPkiData;
    QString mTempDir;
    const char* mPass;
};


TestQgsAuthManager::TestQgsAuthManager()
    : mPkiData( QString( TEST_DATA_DIR ) + "/auth_system/certs_keys" )
    , mTempDir( QDir::tempPath() + "/auth" )
    , mPass( "pass" )
{
}

void TestQgsAuthManager::initTestCase()
{
  // make QGIS_AUTH_DB_DIR_PATH temp dir for qgis-auth.db and master password file
  QDir tmpDir = QDir::temp();
  QVERIFY2( tmpDir.mkpath( mTempDir ), "Couldn't make temp directory" );
  qputenv( "QGIS_AUTH_DB_DIR_PATH", mTempDir.toAscii() );

  // init app and auth manager
  QgsApplication::init();
  QgsApplication::initQgis();
  QVERIFY2( !QgsAuthManager::instance()->isDisabled(),
            "Authentication system is DISABLED" );

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

void TestQgsAuthManager::cleanupTestCase()
{
  QgsApplication::exitQgis();
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

QTEST_MAIN( TestQgsAuthManager )
#include "testqgsauthmanager.moc"
