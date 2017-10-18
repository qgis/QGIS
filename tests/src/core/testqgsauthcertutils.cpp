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

void TestQgsAuthCertUtils::testPkcsUtils()
{
  QByteArray pkcs;

  pkcs = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key.pem", false );
  QVERIFY( !pkcs.isEmpty() );
  QVERIFY( !QgsAuthCertUtils::pemIsPkcs8( QString( pkcs ) ) );

  pkcs.clear();
  pkcs = QgsAuthCertUtils::fileData( sPkiData + "/gerardus_key-pkcs8-rsa.pem", false );
  QVERIFY( !pkcs.isEmpty() );
  QVERIFY( QgsAuthCertUtils::pemIsPkcs8( QString( pkcs ) ) );
}

QGSTEST_MAIN( TestQgsAuthCertUtils )
#include "testqgsauthcertutils.moc"
