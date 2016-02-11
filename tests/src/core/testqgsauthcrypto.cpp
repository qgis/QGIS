/***************************************************************************
     testqgsauthcrypto.cpp
     ----------------------
    Date                 : December 2014
    Copyright            : (C) 2014 by Boundless Spatial, Inc. USA
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

/** \ingroup UnitTests
 * Unit tests for QgsAuthCrypto
 */
class TestQgsAuthCrypto: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testEncrypt();
    void testDecrypt();
    void testPasswordHashDerived();
    void testPasswordHashKnown();

  private:
    static const QString smPass;
    static const QString smSalt;
    static const QString smHash;
    static const QString smCiv;
    static const QString smText;
    static const QString smCrypt;
};

const QString TestQgsAuthCrypto::smPass = "password";
const QString TestQgsAuthCrypto::smSalt = "f48b706946df69d4d2b45bd0603c95af";
const QString TestQgsAuthCrypto::smHash = "0be18c3f1bf872194d6042f5f4a0c116";
const QString TestQgsAuthCrypto::smCiv = QString(
      "1c18c442b6723ee465bcbb60568412179fcc3313eb0187b4546ca96d869fbdc1"
    );
const QString TestQgsAuthCrypto::smText = QString(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
      "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
      "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
      "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
      "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
      "mollit anim id est laborum."
    );
const QString TestQgsAuthCrypto::smCrypt = QString(
      "ec53707ca8769489a6084e88c0e1fb18d3423406ac23c34ad5ac25600f59486375927bc9ed79"
      "d363f8774810ef74a92412ae236b949126c6b799dd8c20b5dbc68e7aa8501e414831e91c6533"
      "83aa6f86eb85eb5206a7e8e894b5c15b6e64de9555c580e1434248d3c0b80ee346583b998ee2"
      "72997788679f37675c0b03dd5661d90c4a4bae4c238b508e6a405e1fe432e03208d5acae6b29"
      "a91fad073a07caa4bda1991b8c2eae79b0179a9fe4e7548089e5a4779e4b92359a332191a60f"
      "2389218f46341f3ced9d30a268101afcfd9645bbf6c6684bcf620ab433554de05f95bcc6c50d"
      "4527ed8dd809eacf60c3e988f5314b41da8fe7d8b773c1d54208f5eca54c678ef0acfc3134a6"
      "b0b18bcadd5f3d00e3188e07aaf9ff88be4ee093514cdb9fa851e5f86d165021c327f0a789fa"
      "3ce2f8efe72f671ef9758ef4c442e9ff736131de5382f7169f8fbf426ba043eafe1c853ae7bb"
      "76861a176af8ffdaa498f459ea2d92d5cffd4244216090ce32a1bfb1dda5a91f9308556770cf"
      "bb13cb91e5909f142f29c7e788a81732bf37c3571955f1a6e4f23fb9c9dbe753b22b4ef47b44"
      "548091393e0ddd6225958423a354ba160e9e8415d5fb9ce55670a6e23ac0"
    );

void TestQgsAuthCrypto::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  if ( QgsAuthCrypto::isDisabled() )
    QSKIP( "QCA's qca-ossl plugin is missing, skipping test case", SkipAll );
}

void TestQgsAuthCrypto::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAuthCrypto::testEncrypt()
{
  QString res = QgsAuthCrypto::encrypt( smPass, smCiv, smText );
  QCOMPARE( res, smCrypt );
}

void TestQgsAuthCrypto::testDecrypt()
{
  QString res = QgsAuthCrypto::decrypt( smPass, smCiv, smCrypt );
  QCOMPARE( res, smText );
}

void TestQgsAuthCrypto::testPasswordHashDerived()
{
  QString salt, hash, civ, derived;
  QgsAuthCrypto::passwordKeyHash( smPass, &salt, &hash, &civ );
  QVERIFY( !civ.isNull() );
  QVERIFY( QgsAuthCrypto::verifyPasswordKeyHash( smPass, salt, hash, &derived ) );
  QVERIFY( !derived.isNull() );
  QCOMPARE( hash, derived ); // double-check assigned output
}

void TestQgsAuthCrypto::testPasswordHashKnown()
{
  QVERIFY( QgsAuthCrypto::verifyPasswordKeyHash( smPass, smSalt, smHash ) );
}

QTEST_MAIN( TestQgsAuthCrypto )
#include "testqgsauthcrypto.moc"
