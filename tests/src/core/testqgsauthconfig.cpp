/***************************************************************************
     testqgsauthconfig.cpp
     ----------------------
    Date                 : September 2015
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
#include <QObject>
#include <QString>
#include <QStringList>

#include "qgsapplication.h"
#include "qgsauthcrypto.h"
#include "qgsauthconfig.h"

/** \ingroup UnitTests
 * Unit tests for QgsAuthConfig
 */
class TestQgsAuthConfig: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init() {}
    void cleanup() {}

    void testMethodConfig();
    void testPkiBundle();
    void testPkiConfigBundle();
    void testConfigSslServer();

  private:
    static QString smPkiData;
};

QString TestQgsAuthConfig::smPkiData = QString( TEST_DATA_DIR ) + "/auth_system/certs_keys";


void TestQgsAuthConfig::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  if ( QgsAuthCrypto::isDisabled() )
    QSKIP( "QCA's qca-ossl plugin is missing, skipping test case", SkipAll );
}

void TestQgsAuthConfig::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAuthConfig::testMethodConfig()
{
  QgsAuthMethodConfig mconfig;
  QVERIFY( !mconfig.isValid() );

  mconfig.setName( "Some Name" );
  mconfig.setMethod( "MethodKey" );
  QVERIFY( mconfig.isValid() );

  mconfig.setId( "0000000" );
  QVERIFY( mconfig.isValid( true ) );

  mconfig.setVersion( 1 );
  mconfig.setUri( "http://example.com" );

  QCOMPARE( mconfig.name(), QString( "Some Name" ) );
  QCOMPARE( mconfig.method(), QString( "MethodKey" ) );
  QCOMPARE( mconfig.id(), QString( "0000000" ) );
  QCOMPARE( mconfig.version(), 1 );
  QCOMPARE( mconfig.uri(), QString( "http://example.com" ) );

  QString confstr( "key1:::value1|||key2:::value2|||key3:::value3a```value3b```value3c" );
  QgsStringMap confmap;
  confmap.insert( "key1", "value1" );
  confmap.insert( "key2", "value2" );
  confmap.insert( "key3", "value3a```value3b```value3c" );

  mconfig.setConfigMap( confmap );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  mconfig.clearConfigMap();
  QVERIFY( mconfig.configMap().isEmpty() );

  mconfig.setConfig( "key1", "value1" );
  mconfig.setConfig( "key2", "value2" );
  QStringList key3list;
  key3list << "value3a" << "value3b" << "value3c";
  mconfig.setConfigList( "key3", key3list );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  QCOMPARE( mconfig.config( "key1" ), QString( "value1" ) );
  QCOMPARE( mconfig.configList( "key3" ), key3list );

  QVERIFY( mconfig.hasConfig( "key2" ) );
  mconfig.removeConfig( "key2" );
  QVERIFY( !mconfig.hasConfig( "key2" ) );

  mconfig.loadConfigString( confstr );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  QgsAuthMethodConfig mconfig2( mconfig );
  QVERIFY( mconfig2 == mconfig );

  mconfig.setMethod( "MethodKey2" );
  QVERIFY( mconfig2 != mconfig );
}

void TestQgsAuthConfig::testPkiBundle()
{
  QgsPkiBundle bundle;
  QVERIFY( bundle.isNull() );
  QVERIFY( !bundle.isValid() );

  QList<QSslCertificate> cacerts( QSslCertificate::fromPath( smPkiData + "/chain_subissuer-issuer-root.pem" ) );
  QVERIFY( !cacerts.isEmpty() );
  QCOMPARE( cacerts.size(), 3 );
  QgsPkiBundle bundle2( QgsPkiBundle::fromPemPaths( smPkiData + "/fra_cert.pem",
                        smPkiData + "/fra_key_w-pass.pem",
                        "password",
                        cacerts ) );
  QVERIFY( !bundle2.isNull() );
  QVERIFY( bundle2.isValid() );
  QCOMPARE( bundle2.certId(), QString( "c3633c428d441853973e5081ba9be39f667f5af6" ) );

  QSslCertificate clientcert( bundle2.clientCert() );
  QVERIFY( !clientcert.isNull() );
  QSslKey clientkey( bundle2.clientKey() );
  QVERIFY( !clientkey.isNull() );
  QList<QSslCertificate> cachain( bundle2.caChain() );
  QVERIFY( !cachain.isEmpty() );
  QCOMPARE( cachain.size(), 3 );

  QgsPkiBundle bundle3( clientcert, clientkey, cachain );
  QVERIFY( !bundle3.isNull() );
  QVERIFY( bundle3.isValid() );

  bundle.setClientCert( clientcert );
  bundle.setClientKey( clientkey );
  bundle.setCaChain( cachain );
  QVERIFY( !bundle.isNull() );
  QVERIFY( bundle.isValid() );

  QgsPkiBundle bundle4( QgsPkiBundle::fromPkcs12Paths( smPkiData + "/fra_w-chain.p12",
                        "password" ) );
  QVERIFY( !bundle4.isNull() );
  QVERIFY( bundle4.isValid() );
  QList<QSslCertificate> cachain4( bundle2.caChain() );
  QVERIFY( !cachain4.isEmpty() );
  QCOMPARE( cachain4.size(), 3 );
}

void TestQgsAuthConfig::testPkiConfigBundle()
{
  QgsAuthMethodConfig mconfig;
  mconfig.setName( "Some Name" );
  mconfig.setMethod( "MethodKey" );
  mconfig.setId( "0000000" );
  mconfig.setVersion( 1 );
  mconfig.setUri( "http://example.com" );
  QVERIFY( mconfig.isValid( true ) );

  QSslCertificate clientcert( QSslCertificate::fromPath( smPkiData + "/gerardus_cert.pem" ).at( 0 ) );
  QByteArray keydata;
  QFile file( smPkiData + "/gerardus_key.pem" );
  if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    keydata = file.readAll();
  file.close();
  QSslKey clientkey( keydata, QSsl::Rsa );

  QgsPkiConfigBundle bundle( mconfig, clientcert, clientkey );
  QVERIFY( bundle.isValid() );
  QCOMPARE( bundle.config(), mconfig );

  QCOMPARE( bundle.clientCert(), clientcert );
  QCOMPARE( bundle.clientCertKey(), clientkey );
  bundle.setConfig( mconfig );
  bundle.setClientCert( clientcert );
  bundle.setClientCertKey( clientkey );
  QVERIFY( bundle.isValid() );
  QCOMPARE( bundle.config(), mconfig );
  QCOMPARE( bundle.clientCert(), clientcert );
  QCOMPARE( bundle.clientCertKey(), clientkey );
}

void TestQgsAuthConfig::testConfigSslServer()
{
  QString hostport( "localhost:443" );
  QString confstr( "2|||470|||2|||10~~19|||0~~2" );
  QSslCertificate sslcert( QSslCertificate::fromPath( smPkiData + "/localhost_ssl_cert.pem" ).at( 0 ) );

  QgsAuthConfigSslServer sslconfig;
  QVERIFY( sslconfig.isNull() );
#if QT_VERSION >= 0x040800
  QCOMPARE( sslconfig.qtVersion(), 480 );
#else
  QCOMPARE( sslconfig.qtVersion(), 470 );
#endif
  QCOMPARE( sslconfig.version(), 1 );
  QCOMPARE( sslconfig.sslPeerVerifyMode(), QSslSocket::VerifyPeer );

  sslconfig.setSslCertificate( sslcert );
  sslconfig.setSslHostPort( hostport );
  sslconfig.setSslProtocol( QSsl::TlsV1 );
  sslconfig.setVersion( 2 );
  sslconfig.setQtVersion( 470 );
  sslconfig.setSslPeerVerifyMode( QSslSocket::VerifyNone );
  sslconfig.setSslPeerVerifyDepth( 2 );
  QList<QSslError::SslError> sslerrenums;
  sslerrenums << QSslError::SelfSignedCertificateInChain << QSslError::SubjectIssuerMismatch;
  sslconfig.setSslIgnoredErrorEnums( sslerrenums );
  QVERIFY( !sslconfig.isNull() );

  QCOMPARE( sslconfig.configString(), confstr );
  QCOMPARE( sslconfig.sslHostPort(), hostport );
  QCOMPARE( sslconfig.sslCertificate(), sslcert );
  QCOMPARE( sslconfig.sslProtocol(), QSsl::TlsV1 );
  QCOMPARE( sslconfig.version(), 2 );
  QCOMPARE( sslconfig.qtVersion(), 470 );
  QCOMPARE( sslconfig.sslPeerVerifyMode(), QSslSocket::VerifyNone );
  QCOMPARE( sslconfig.sslPeerVerifyDepth(), 2 );
  QCOMPARE( sslconfig.sslIgnoredErrorEnums(), sslerrenums );

  QgsAuthConfigSslServer sslconfig2;
  sslconfig2.loadConfigString( confstr );
  QCOMPARE( sslconfig2.sslProtocol(), QSsl::TlsV1 );
  QCOMPARE( sslconfig2.version(), 2 );
  QCOMPARE( sslconfig2.qtVersion(), 470 );
  QCOMPARE( sslconfig2.sslPeerVerifyMode(), QSslSocket::VerifyNone );
  QCOMPARE( sslconfig2.sslPeerVerifyDepth(), 2 );
  QCOMPARE( sslconfig2.sslIgnoredErrorEnums(), sslerrenums );
  QCOMPARE( sslconfig2.configString(), confstr );
}

QTEST_MAIN( TestQgsAuthConfig )
#include "testqgsauthconfig.moc"
