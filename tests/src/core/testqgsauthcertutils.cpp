/***************************************************************************
     TestQgsAuthCertUtils.cpp
     ----------------------
    Date                 : October 2017
    Copyright            : (C) 2017 by Boundless Spatial, Inc. USA
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
#include <QSslKey>
#include <QString>
#include <QStringList>

#include "qgsapplication.h"
#include "qgsauthcrypto.h"
#include "qgsauthcertutils.h"
#include "qgslogger.h"

/**
 * \ingroup UnitTests
 * Unit tests for QgsAuthCertUtils static functions
 */
class TestQgsAuthCertUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init() {}
    void cleanup() {}

    void testValidationUtils();
    void testPkcsUtils();

  private:
    static QString sPkiData;
};

QString TestQgsAuthCertUtils::sPkiData = QStringLiteral( TEST_DATA_DIR ) + "/auth_system/certs_keys";

void TestQgsAuthCertUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  if ( QgsAuthCrypto::isDisabled() )
    QSKIP( "QCA's qca-ossl plugin is missing, skipping test case", SkipAll );
}

void TestQgsAuthCertUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAuthCertUtils::testValidationUtils()
{
  // null cert
  QSslCertificate cert;
  QVERIFY( !QgsAuthCertUtils::certIsCurrent( cert ) );
  QList<QSslError> res = QgsAuthCertUtils::certViabilityErrors( cert );
  QVERIFY( res.count() == 0 );
  QVERIFY( !QgsAuthCertUtils::certIsViable( cert ) );

  cert.clear();
  res.clear();
  // valid cert
  cert = QgsAuthCertUtils::certFromFile( sPkiData + "/gerardus_cert.pem" );
  QVERIFY( QgsAuthCertUtils::certIsCurrent( cert ) );
  res = QgsAuthCertUtils::certViabilityErrors( cert );
  QVERIFY( res.count() == 0 );
  QVERIFY( QgsAuthCertUtils::certIsViable( cert ) );


  cert.clear();
  res.clear();
  // expired cert
  cert = QgsAuthCertUtils::certFromFile( sPkiData + "/marinus_cert-EXPIRED.pem" );
  QVERIFY( !QgsAuthCertUtils::certIsCurrent( cert ) );
  res = QgsAuthCertUtils::certViabilityErrors( cert );
  QVERIFY( res.count() > 0 );
  QVERIFY( res.contains( QSslError( QSslError::SslError::CertificateExpired, cert ) ) );
  QVERIFY( !QgsAuthCertUtils::certIsViable( cert ) );
}

void TestQgsAuthCertUtils::testPkcsUtils()
{
  QByteArray pkcs;

  pkcs = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key.pem" );
  QVERIFY( !pkcs.isEmpty() );
  QVERIFY( !QgsAuthCertUtils::pemIsPkcs8( QString( pkcs ) ) );

  pkcs.clear();
  pkcs = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key-pkcs8-rsa.pem" );
  QVERIFY( !pkcs.isEmpty() );
  QVERIFY( QgsAuthCertUtils::pemIsPkcs8( QString( pkcs ) ) );


#ifdef Q_OS_MAC
  QByteArray pkcs1;
  pkcs.clear();

  // Nothing should return nothing
  pkcs1 = QgsAuthCertUtils::pkcs8PrivateKey( pkcs );
  QVERIFY( pkcs1.isEmpty() );

  pkcs.clear();
  pkcs1.clear();
  // Is actually a PKCS#1 key, not #8
  pkcs = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key.der" );
  QVERIFY( !pkcs.isEmpty() );
  pkcs1 = QgsAuthCertUtils::pkcs8PrivateKey( pkcs );
  QVERIFY( pkcs1.isEmpty() );

  pkcs.clear();
  pkcs1.clear();
  // Is PKCS#1 PEM text, not DER
  pkcs = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key.pem" );
  QVERIFY( !pkcs.isEmpty() );
  pkcs1 = QgsAuthCertUtils::pkcs8PrivateKey( pkcs );
  QVERIFY( pkcs1.isEmpty() );

  pkcs.clear();
  pkcs1.clear();
  // Is PKCS#8 PEM text, not DER
  pkcs = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key-pkcs8-rsa.pem" );
  QVERIFY( !pkcs.isEmpty() );
  pkcs1 = QgsAuthCertUtils::pkcs8PrivateKey( pkcs );
  QVERIFY( pkcs1.isEmpty() );

  pkcs.clear();
  pkcs1.clear();
  // Correct PKCS#8 DER input
  pkcs = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key-pkcs8-rsa.der" );
  QVERIFY( !pkcs.isEmpty() );
  pkcs1 = QgsAuthCertUtils::pkcs8PrivateKey( pkcs );
  QVERIFY( !pkcs1.isEmpty() );

  // PKCS#8 DER format should fail, and the reason for QgsAuthCertUtils::pkcs8PrivateKey
  // (as of Qt5.9.0, and where macOS Qt5 SSL backend is not OpenSSL, and
  //  where PKCS#8 is *still* unsupported for macOS)
  QSslKey pkcs8Key( pkcs, QSsl::Rsa, QSsl::Der, QSsl::PrivateKey );
  QVERIFY( pkcs8Key.isNull() );

  // PKCS#1 DER format should work
  QSslKey pkcs1Key( pkcs1, QSsl::Rsa, QSsl::Der, QSsl::PrivateKey );
  QVERIFY( !pkcs1Key.isNull() );

  // Converted PKCS#8 DER should match PKCS#1 PEM
  QByteArray pkcs1PemRef = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key.pem" );
  QVERIFY( !pkcs1PemRef.isEmpty() );
  QCOMPARE( pkcs1Key.toPem(), pkcs1PemRef );
#endif
}

QGSTEST_MAIN( TestQgsAuthCertUtils )
#include "testqgsauthcertutils.moc"
