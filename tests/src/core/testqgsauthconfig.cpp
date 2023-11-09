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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>

#include "qgsapplication.h"
#include "qgsauthcrypto.h"
#include "qgsauthconfig.h"

/**
 * \ingroup UnitTests
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
    static QString sPkiData;
};

QString TestQgsAuthConfig::sPkiData = QStringLiteral( TEST_DATA_DIR ) + "/auth_system/certs_keys";


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

  mconfig.setName( QStringLiteral( "Some Name" ) );
  mconfig.setMethod( QStringLiteral( "MethodKey" ) );
  QVERIFY( mconfig.isValid() );

  mconfig.setId( QStringLiteral( "0000000" ) );
  QVERIFY( mconfig.isValid( true ) );

  mconfig.setVersion( 1 );
  mconfig.setUri( QStringLiteral( "http://example.com" ) );

  QCOMPARE( mconfig.name(), QString( "Some Name" ) );
  QCOMPARE( mconfig.method(), QString( "MethodKey" ) );
  QCOMPARE( mconfig.id(), QString( "0000000" ) );
  QCOMPARE( mconfig.version(), 1 );
  QCOMPARE( mconfig.uri(), QString( "http://example.com" ) );

  const QString confstr( QStringLiteral( "key1:::value1|||key2:::value2|||key3:::value3a```value3b```value3c" ) );
  QgsStringMap confmap;
  confmap.insert( QStringLiteral( "key1" ), QStringLiteral( "value1" ) );
  confmap.insert( QStringLiteral( "key2" ), QStringLiteral( "value2" ) );
  confmap.insert( QStringLiteral( "key3" ), QStringLiteral( "value3a```value3b```value3c" ) );

  mconfig.setConfigMap( confmap );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  mconfig.clearConfigMap();
  QVERIFY( mconfig.configMap().isEmpty() );

  mconfig.setConfig( QStringLiteral( "key1" ), QStringLiteral( "value1" ) );
  mconfig.setConfig( QStringLiteral( "key2" ), QStringLiteral( "value2" ) );
  QStringList key3list;
  key3list << QStringLiteral( "value3a" ) << QStringLiteral( "value3b" ) << QStringLiteral( "value3c" );
  mconfig.setConfigList( QStringLiteral( "key3" ), key3list );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  QCOMPARE( mconfig.config( "key1" ), QString( "value1" ) );
  QCOMPARE( mconfig.configList( "key3" ), key3list );

  QVERIFY( mconfig.hasConfig( "key2" ) );
  mconfig.removeConfig( QStringLiteral( "key2" ) );
  QVERIFY( !mconfig.hasConfig( "key2" ) );

  mconfig.loadConfigString( confstr );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  const QgsAuthMethodConfig mconfig2( mconfig );
  QVERIFY( mconfig2 == mconfig );

  mconfig.setMethod( QStringLiteral( "MethodKey2" ) );
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
  const QgsPkiBundle bundle2( QgsPkiBundle::fromPemPaths( sPkiData + "/fra_cert.pem",
                              sPkiData + "/fra_key_w-pass.pem",
                              QStringLiteral( "password" ),
                              cacerts ) );
  QVERIFY( !bundle2.isNull() );
  QVERIFY( bundle2.isValid() );
  QCOMPARE( bundle2.certId(), QString( "c3633c428d441853973e5081ba9be39f667f5af6" ) );

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

  const QgsPkiBundle bundle4( QgsPkiBundle::fromPkcs12Paths( sPkiData + "/fra_w-chain.p12",
                              QStringLiteral( "password" ) ) );
  QVERIFY( !bundle4.isNull() );
  QVERIFY( bundle4.isValid() );
  const QList<QSslCertificate> cachain4( bundle2.caChain() );
  QVERIFY( !cachain4.isEmpty() );
  QCOMPARE( cachain4.size(), 3 );
}

void TestQgsAuthConfig::testPkiConfigBundle()
{
  QgsAuthMethodConfig mconfig;
  mconfig.setName( QStringLiteral( "Some Name" ) );
  mconfig.setMethod( QStringLiteral( "MethodKey" ) );
  mconfig.setId( QStringLiteral( "0000000" ) );
  mconfig.setVersion( 1 );
  mconfig.setUri( QStringLiteral( "http://example.com" ) );
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
  const QString hostport( QStringLiteral( "localhost:443" ) );
  const QString confstr( QStringLiteral( "2|||470|||2|||10~~19|||0~~2" ) );
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

  QCOMPARE( sslconfig.configString(), confstr );
  QCOMPARE( sslconfig.sslHostPort(), hostport );
  QCOMPARE( sslconfig.sslCertificate(), sslcert );
  QCOMPARE( sslconfig.sslProtocol(), QSsl::TlsV1_0 );
  QCOMPARE( sslconfig.version(), 2 );
  QCOMPARE( sslconfig.qtVersion(), 470 );
  QCOMPARE( sslconfig.sslPeerVerifyMode(), QSslSocket::VerifyNone );
  QCOMPARE( sslconfig.sslPeerVerifyDepth(), 2 );
  QCOMPARE( sslconfig.sslIgnoredErrorEnums(), sslerrenums );

  QgsAuthConfigSslServer sslconfig2;
  sslconfig2.loadConfigString( confstr );
  QCOMPARE( sslconfig2.sslProtocol(), QSsl::TlsV1_0 );
  QCOMPARE( sslconfig2.version(), 2 );
  QCOMPARE( sslconfig2.qtVersion(), 470 );
  QCOMPARE( sslconfig2.sslPeerVerifyMode(), QSslSocket::VerifyNone );
  QCOMPARE( sslconfig2.sslPeerVerifyDepth(), 2 );
  QCOMPARE( sslconfig2.sslIgnoredErrorEnums(), sslerrenums );
  QCOMPARE( sslconfig2.configString(), confstr );
}

QGSTEST_MAIN( TestQgsAuthConfig )
#include "testqgsauthconfig.moc"
