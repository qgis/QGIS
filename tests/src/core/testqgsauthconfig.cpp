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
#include "qgsapplication.h"
#include "qgsauthconfig.h"
#include "qgsauthcrypto.h"
#include "qgsauthmanager.h"
#include "qgstest.h"

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * \ingroup UnitTests
 * Unit tests for QgsAuthConfig
 */
class TestQgsAuthConfig : public QObject
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
    static QString sPkiData;
};

QString TestQgsAuthConfig::sPkiData = QStringLiteral( TEST_DATA_DIR ) + "/auth_system/certs_keys";


void TestQgsAuthConfig::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::authManager()->ensureInitialized();
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

  mconfig.setName( u"Some Name"_s );
  mconfig.setMethod( u"MethodKey"_s );
  QVERIFY( mconfig.isValid() );

  mconfig.setId( u"0000000"_s );
  QVERIFY( mconfig.isValid( true ) );

  mconfig.setVersion( 1 );
  mconfig.setUri( u"http://example.com"_s );

  QCOMPARE( mconfig.name(), QString( "Some Name" ) );
  QCOMPARE( mconfig.method(), QString( "MethodKey" ) );
  QCOMPARE( mconfig.id(), QString( "0000000" ) );
  QCOMPARE( mconfig.version(), 1 );
  QCOMPARE( mconfig.uri(), QString( "http://example.com" ) );

  const QString confstr( u"key1:::value1|||key2:::value2|||key3:::value3a```value3b```value3c"_s );
  QgsStringMap confmap;
  confmap.insert( u"key1"_s, u"value1"_s );
  confmap.insert( u"key2"_s, u"value2"_s );
  confmap.insert( u"key3"_s, u"value3a```value3b```value3c"_s );

  mconfig.setConfigMap( confmap );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  mconfig.clearConfigMap();
  QVERIFY( mconfig.configMap().isEmpty() );

  mconfig.setConfig( u"key1"_s, u"value1"_s );
  mconfig.setConfig( u"key2"_s, u"value2"_s );
  QStringList key3list;
  key3list << u"value3a"_s << u"value3b"_s << u"value3c"_s;
  mconfig.setConfigList( u"key3"_s, key3list );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  QCOMPARE( mconfig.config( "key1" ), QString( "value1" ) );
  QCOMPARE( mconfig.configList( "key3" ), key3list );

  QVERIFY( mconfig.hasConfig( "key2" ) );
  mconfig.removeConfig( u"key2"_s );
  QVERIFY( !mconfig.hasConfig( "key2" ) );

  mconfig.loadConfigString( confstr );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  const QgsAuthMethodConfig mconfig2( mconfig );
  QVERIFY( mconfig2 == mconfig );

  mconfig.setMethod( u"MethodKey2"_s );
  QVERIFY( mconfig2 != mconfig );
}

void TestQgsAuthConfig::testPkiBundle()
{
  QgsPkiBundle bundle;
  QVERIFY( bundle.isNull() );
  QVERIFY( !bundle.isValid() );

  const QList<QSslCertificate> cacerts( QSslCertificate::fromPath( sPkiData + "/chain_subissuer-issuer-root.pem" ) );
  QVERIFY( !cacerts.isEmpty() );
  QCOMPARE( cacerts.size(), 3 );
  const QgsPkiBundle bundle2( QgsPkiBundle::fromPemPaths( sPkiData + "/fra_cert.pem", sPkiData + "/fra_key_w-pass.pem", u"password"_s, cacerts ) );
  QVERIFY( !bundle2.isNull() );
  QVERIFY( bundle2.isValid() );
  QCOMPARE( bundle2.certId(), QString( "2dbb930cf358e56b3492fde459e3b68c2bf1739d" ) );

  const QSslCertificate clientcert( bundle2.clientCert() );
  QVERIFY( !clientcert.isNull() );
  const QSslKey clientkey( bundle2.clientKey() );
  QVERIFY( !clientkey.isNull() );
  const QList<QSslCertificate> cachain( bundle2.caChain() );
  QVERIFY( !cachain.isEmpty() );
  QCOMPARE( cachain.size(), 3 );

  const QgsPkiBundle bundle3( clientcert, clientkey, cachain );
  QVERIFY( !bundle3.isNull() );
  QVERIFY( bundle3.isValid() );

  bundle.setClientCert( clientcert );
  bundle.setClientKey( clientkey );
  bundle.setCaChain( cachain );
  QVERIFY( !bundle.isNull() );
  QVERIFY( bundle.isValid() );

  const QgsPkiBundle bundle4( QgsPkiBundle::fromPkcs12Paths( sPkiData + "/fra_w-chain.p12", u"password"_s ) );
  QVERIFY( !bundle4.isNull() );
  QVERIFY( bundle4.isValid() );
  const QList<QSslCertificate> cachain4( bundle2.caChain() );
  QVERIFY( !cachain4.isEmpty() );
  QCOMPARE( cachain4.size(), 3 );
}

void TestQgsAuthConfig::testPkiConfigBundle()
{
  QgsAuthMethodConfig mconfig;
  mconfig.setName( u"Some Name"_s );
  mconfig.setMethod( u"MethodKey"_s );
  mconfig.setId( u"0000000"_s );
  mconfig.setVersion( 1 );
  mconfig.setUri( u"http://example.com"_s );
  QVERIFY( mconfig.isValid( true ) );

  const QSslCertificate clientcert( QSslCertificate::fromPath( sPkiData + "/gerardus_cert.pem" ).at( 0 ) );
  QByteArray keydata;
  QFile file( sPkiData + "/gerardus_key.pem" );
  if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    keydata = file.readAll();
  file.close();
  const QSslKey clientkey( keydata, QSsl::Rsa );

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
  const QString hostport( u"localhost:443"_s );
  const QSslCertificate sslcert( QSslCertificate::fromPath( sPkiData + "/localhost_ssl_cert.pem" ).at( 0 ) );

  QgsAuthConfigSslServer sslconfig;
  QVERIFY( sslconfig.isNull() );
  QCOMPARE( sslconfig.qtVersion(), 480 );
  QCOMPARE( sslconfig.version(), 1 );
  QCOMPARE( sslconfig.sslPeerVerifyMode(), QSslSocket::VerifyPeer );

  sslconfig.setSslCertificate( sslcert );
  sslconfig.setSslHostPort( hostport );
  sslconfig.setSslProtocol( QSsl::TlsV1_0 );
  sslconfig.setVersion( 2 );
  sslconfig.setQtVersion( 470 );
  sslconfig.setSslPeerVerifyMode( QSslSocket::VerifyNone );
  sslconfig.setSslPeerVerifyDepth( 2 );
  QList<QSslError::SslError> sslerrenums;
  sslerrenums << QSslError::SelfSignedCertificateInChain << QSslError::SubjectIssuerMismatch;
  sslconfig.setSslIgnoredErrorEnums( sslerrenums );
  QVERIFY( !sslconfig.isNull() );

  QCOMPARE( sslconfig.configString(), u"2|||470|||TlsV1_0|||10~~19|||0~~2"_s );
  QCOMPARE( sslconfig.sslHostPort(), hostport );
  QCOMPARE( sslconfig.sslCertificate(), sslcert );
  QCOMPARE( sslconfig.sslProtocol(), QSsl::TlsV1_0 );
  QCOMPARE( sslconfig.version(), 2 );
  QCOMPARE( sslconfig.qtVersion(), 470 );
  QCOMPARE( sslconfig.sslPeerVerifyMode(), QSslSocket::VerifyNone );
  QCOMPARE( sslconfig.sslPeerVerifyDepth(), 2 );
  QCOMPARE( sslconfig.sslIgnoredErrorEnums(), sslerrenums );

  QgsAuthConfigSslServer sslconfig2;
  // try loading the older format strings, used in QGIS < 3.40
  sslconfig2.loadConfigString( u"2|||470|||2|||10~~19|||0~~2"_s );
  QCOMPARE( sslconfig2.sslProtocol(), QSsl::TlsV1_0 );
  QCOMPARE( sslconfig2.version(), 2 );
  QCOMPARE( sslconfig2.qtVersion(), 470 );
  QCOMPARE( sslconfig2.sslPeerVerifyMode(), QSslSocket::VerifyNone );
  QCOMPARE( sslconfig2.sslPeerVerifyDepth(), 2 );
  QCOMPARE( sslconfig2.sslIgnoredErrorEnums(), sslerrenums );
  QCOMPARE( sslconfig2.configString(), u"2|||470|||TlsV1_0|||10~~19|||0~~2"_s );

  sslconfig2.loadConfigString( u"2|||470|||3|||10~~19|||0~~2"_s );
  QCOMPARE( sslconfig2.sslProtocol(), QSsl::TlsV1_1 );
  QCOMPARE( sslconfig2.version(), 2 );
  QCOMPARE( sslconfig2.qtVersion(), 470 );
  QCOMPARE( sslconfig2.sslPeerVerifyMode(), QSslSocket::VerifyNone );
  QCOMPARE( sslconfig2.sslPeerVerifyDepth(), 2 );
  QCOMPARE( sslconfig2.sslIgnoredErrorEnums(), sslerrenums );
  QCOMPARE( sslconfig2.configString(), u"2|||470|||TlsV1_1|||10~~19|||0~~2"_s );

  QgsAuthConfigSslServer sslconfig3;
  // try loading the newer format string, used in QGIS >= 3.40
  sslconfig2.loadConfigString( u"2|||470|||TlsV1_3|||10~~19|||0~~2"_s );
  QCOMPARE( sslconfig2.sslProtocol(), QSsl::TlsV1_3 );
  QCOMPARE( sslconfig2.version(), 2 );
  QCOMPARE( sslconfig2.qtVersion(), 470 );
  QCOMPARE( sslconfig2.sslPeerVerifyMode(), QSslSocket::VerifyNone );
  QCOMPARE( sslconfig2.sslPeerVerifyDepth(), 2 );
  QCOMPARE( sslconfig2.sslIgnoredErrorEnums(), sslerrenums );
  QCOMPARE( sslconfig2.configString(), u"2|||470|||TlsV1_3|||10~~19|||0~~2"_s );
}

QGSTEST_MAIN( TestQgsAuthConfig )
#include "testqgsauthconfig.moc"
