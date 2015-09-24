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
}

void TestQgsAuthConfig::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAuthConfig::testMethodConfig()
{
  QgsAuthMethodConfig mconfig;
  Q_ASSERT( !mconfig.isValid() );

  mconfig.setName( "Some Name" );
  mconfig.setMethod( "MethodKey" );
  Q_ASSERT( mconfig.isValid() );

  mconfig.setId( "0000000" );
  Q_ASSERT( mconfig.isValid( true ) );

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
  Q_ASSERT( mconfig.configMap().isEmpty() );

  mconfig.setConfig( "key1", "value1" );
  mconfig.setConfig( "key2", "value2" );
  QStringList key3list;
  key3list << "value3a" << "value3b" << "value3c";
  mconfig.setConfigList( "key3", key3list );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  QCOMPARE( mconfig.config( "key1" ), QString( "value1" ) );
  QCOMPARE( mconfig.configList( "key3" ), key3list );

  Q_ASSERT( mconfig.hasConfig( "key2" ) );
  mconfig.removeConfig( "key2" );
  Q_ASSERT( !mconfig.hasConfig( "key2" ) );

  mconfig.loadConfigString( confstr );
  QCOMPARE( mconfig.configMap(), confmap );
  QCOMPARE( mconfig.configString(), confstr );

  QgsAuthMethodConfig mconfig2( mconfig );
  Q_ASSERT( mconfig2 == mconfig );

  mconfig.setMethod( "MethodKey2" );
  Q_ASSERT( mconfig2 != mconfig );
}

void TestQgsAuthConfig::testPkiBundle()
{
  QgsPkiBundle bundle;
  Q_ASSERT( bundle.isNull() );
  Q_ASSERT( !bundle.isValid() );

  QList<QSslCertificate> cacerts( QSslCertificate::fromPath( smPkiData + "/chain_subissuer-issuer-root.pem" ) );
  Q_ASSERT( !cacerts.isEmpty() );
  QCOMPARE( cacerts.size(), 3 );
  QgsPkiBundle bundle2( QgsPkiBundle::fromPemPaths( smPkiData + "/fra_cert.pem",
                        smPkiData + "/fra_key_w-pass.pem",
                        "password",
                        cacerts ) );
  Q_ASSERT( !bundle2.isNull() );
  Q_ASSERT( bundle2.isValid() );
  QCOMPARE( bundle2.certId(), QString( "c3633c428d441853973e5081ba9be39f667f5af6" ) );

  QSslCertificate clientcert( bundle2.clientCert() );
  Q_ASSERT( !clientcert.isNull() );
  QSslKey clientkey( bundle2.clientKey( true ) );
  Q_ASSERT( !clientkey.isNull() );
  QString keypass( bundle2.keyPassphrase() );
  Q_ASSERT( !keypass.isEmpty() );
  QList<QSslCertificate> cachain( bundle2.caChain() );
  Q_ASSERT( !cachain.isEmpty() );
  QCOMPARE( cachain.size(), 3 );

  QgsPkiBundle bundle3( clientcert, clientkey, keypass, cachain );
  Q_ASSERT( !bundle3.isNull() );
  Q_ASSERT( bundle3.isValid() );

  bundle.setClientCert( clientcert );
  bundle.setClientKey( clientkey );
  bundle.setKeyPassphrase( keypass );
  bundle.setCaChain( cachain );
  Q_ASSERT( !bundle.isNull() );
  Q_ASSERT( bundle.isValid() );

  QgsPkiBundle bundle4( QgsPkiBundle::fromPkcs12Paths( smPkiData + "/fra_w-chain.p12",
                        "password" ) );
  Q_ASSERT( !bundle4.isNull() );
  Q_ASSERT( bundle4.isValid() );
  QList<QSslCertificate> cachain4( bundle2.caChain() );
  Q_ASSERT( !cachain4.isEmpty() );
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
  Q_ASSERT( mconfig.isValid( true ) );

  QSslCertificate clientcert( QSslCertificate::fromPath( smPkiData + "/gerardus_cert.pem" ).first() );
  QByteArray keydata;
  QFile file( smPkiData + "/gerardus_key.pem" );
  if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    keydata = file.readAll();
  file.close();
  QSslKey clientkey( keydata, QSsl::Rsa );

  QgsPkiConfigBundle bundle( mconfig, clientcert, clientkey );
  Q_ASSERT( bundle.isValid() );
  QCOMPARE( bundle.config(), mconfig );

  QCOMPARE( bundle.clientCert(), clientcert );
  QCOMPARE( bundle.clientCertKey(), clientkey );
  bundle.setConfig( mconfig );
  bundle.setClientCert( clientcert );
  bundle.setClientCertKey( clientkey );
  Q_ASSERT( bundle.isValid() );
  QCOMPARE( bundle.config(), mconfig );
  QCOMPARE( bundle.clientCert(), clientcert );
  QCOMPARE( bundle.clientCertKey(), clientkey );
}

void TestQgsAuthConfig::testConfigSslServer()
{
  QString hostport( "localhost:443" );
  QString confstr( "2|||470|||2|||10~~19|||0~~2" );
  QSslCertificate sslcert( QSslCertificate::fromPath( smPkiData + "/localhost_ssl_cert.pem" ).first() );

  QgsAuthConfigSslServer sslconfig;
  Q_ASSERT( sslconfig.isNull() );
  QCOMPARE( sslconfig.qtVersion(), 480 );
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
  Q_ASSERT( !sslconfig.isNull() );

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
