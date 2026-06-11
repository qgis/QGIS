/***************************************************************************
  testqgsaisecretstore.cpp
  ------------------------
  begin                : June 2026
***************************************************************************/

#include "ai/qgsaisecretstore.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QScopeGuard>

using namespace Qt::StringLiterals;

namespace
{
  //! True when the auth vault can be made usable in this test environment.
  bool unlockTestVault()
  {
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( !authManager || authManager->isDisabled() )
      return false;
    if ( authManager->masterPasswordIsSet() )
      return true;
    return authManager->setMasterPassword( u"qgsai_test_master"_s, true );
  }

  void wipeSecret( const QString &key )
  {
    QgsAiSecretStore::removeSecret( key );
  }
} //namespace

class TestQgsAiSecretStore : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    // Locked-vault tests must run before the vault tests below set a master password.
    void cleartextFallbackWhenVaultLocked();
    void envFallback();
    void plaintextPassthroughWithoutVault();
    void migrationDeferredWhenVaultLocked();
    void vaultRoundTrip();
    void migrationMovesCleartextSecrets();
    void dataKeyBootstrapAndRoundTrip();
};

void TestQgsAiSecretStore::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiSecretStore::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiSecretStore::cleartextFallbackWhenVaultLocked()
{
  // Interactive unlock is never enabled in tests; without a master password
  // the vault is unusable and writes must land in cleartext settings.
  const QString key = u"ai/test/secretstore/cleartext"_s;
  const auto cleanup = qScopeGuard( [key]() { wipeSecret( key ); } );

  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( authManager && authManager->masterPasswordIsSet() )
    QSKIP( "Master password already set in this environment: cleartext fallback not reachable." );

  QVERIFY( !QgsAiSecretStore::vaultUsable() );
  QVERIFY( QgsAiSecretStore::writeSecret( key, u"plain-secret"_s ) );

  QgsSettings settings;
  QCOMPARE( settings.value( key ).toString(), u"plain-secret"_s );
  QVERIFY( !settings.value( QgsAiSecretStore::flagKey( key ), false ).toBool() );
  QCOMPARE( QgsAiSecretStore::readSecret( key ), u"plain-secret"_s );
  QVERIFY( QgsAiSecretStore::hasSecret( key ) );

  QgsAiSecretStore::removeSecret( key );
  QVERIFY( !QgsAiSecretStore::hasSecret( key ) );
}

void TestQgsAiSecretStore::envFallback()
{
  const QString key = u"ai/test/secretstore/env"_s;
  const auto cleanup = qScopeGuard( [key]() {
    wipeSecret( key );
    qunsetenv( "QGSAI_TEST_SECRET" );
  } );

  wipeSecret( key );
  qputenv( "QGSAI_TEST_SECRET", "from-env" );
  QCOMPARE( QgsAiSecretStore::readSecret( key, { u"QGSAI_TEST_SECRET"_s } ), u"from-env"_s );

  // A stored value always wins over the environment.
  QgsAiSecretStore::writeSecret( key, u"stored"_s );
  QCOMPARE( QgsAiSecretStore::readSecret( key, { u"QGSAI_TEST_SECRET"_s } ), u"stored"_s );
}

void TestQgsAiSecretStore::vaultRoundTrip()
{
  if ( !unlockTestVault() )
    QSKIP( "Authentication vault unavailable in this environment." );

  const QString key = u"ai/test/secretstore/vault"_s;
  const auto cleanup = qScopeGuard( [key]() { wipeSecret( key ); } );

  QVERIFY( QgsAiSecretStore::vaultUsable() );
  QVERIFY( QgsAiSecretStore::writeSecret( key, u"vault-secret"_s ) );

  // The secret lives in the vault: no cleartext copy, presence flag set.
  QgsSettings settings;
  QVERIFY( settings.value( key ).toString().isEmpty() );
  QVERIFY( settings.value( QgsAiSecretStore::flagKey( key ), false ).toBool() );
  QVERIFY( QgsAiSecretStore::hasSecret( key ) );
  QCOMPARE( QgsAiSecretStore::readSecret( key ), u"vault-secret"_s );

  QgsAiSecretStore::removeSecret( key );
  QVERIFY( !QgsAiSecretStore::hasSecret( key ) );
  QVERIFY( QgsAiSecretStore::readSecret( key ).isEmpty() );
}

void TestQgsAiSecretStore::migrationMovesCleartextSecrets()
{
  if ( !unlockTestVault() )
    QSKIP( "Authentication vault unavailable in this environment." );

  const QStringList legacyKeys = {
    u"ai/provider/openai/apiKey"_s,
    u"ai/provider/claude/apiKey"_s,
    u"ai/provider/openrouter/apiKey"_s,
    u"ai/provider/codex/oauth/refreshToken"_s,
  };

  QgsSettings settings;
  const auto cleanup = qScopeGuard( [legacyKeys, &settings]() {
    for ( const QString &key : legacyKeys )
      wipeSecret( key );
    settings.remove( u"ai/security/secretsMigrated_v1"_s );
  } );

  for ( const QString &key : legacyKeys )
  {
    wipeSecret( key );
    settings.setValue( key, QString( u"legacy-%1"_s ).arg( key.section( '/', -1 ) ) );
  }
  settings.remove( u"ai/security/secretsMigrated_v1"_s );

  QgsAiSecretStore::migrateLegacySecrets();

  QVERIFY( settings.value( u"ai/security/secretsMigrated_v1"_s, false ).toBool() );
  for ( const QString &key : legacyKeys )
  {
    QVERIFY2( settings.value( key ).toString().isEmpty(), qPrintable( key ) );
    QVERIFY2( settings.value( QgsAiSecretStore::flagKey( key ), false ).toBool(), qPrintable( key ) );
    QCOMPARE( QgsAiSecretStore::readSecret( key ), QString( u"legacy-%1"_s ).arg( key.section( '/', -1 ) ) );
  }
}

void TestQgsAiSecretStore::migrationDeferredWhenVaultLocked()
{
  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( authManager && authManager->masterPasswordIsSet() )
    QSKIP( "Master password already set in this environment: deferred path not reachable." );

  const QString key = u"ai/provider/openai/apiKey"_s;
  QgsSettings settings;
  const auto cleanup = qScopeGuard( [key, &settings]() {
    wipeSecret( key );
    settings.remove( u"ai/security/secretsMigrated_v1"_s );
  } );

  wipeSecret( key );
  settings.setValue( key, u"still-cleartext"_s );
  settings.remove( u"ai/security/secretsMigrated_v1"_s );

  QgsAiSecretStore::migrateLegacySecrets();

  // Vault locked: nothing moved, flag NOT set (so migration retries next start).
  QCOMPARE( settings.value( key ).toString(), u"still-cleartext"_s );
  QVERIFY( !settings.value( u"ai/security/secretsMigrated_v1"_s, false ).toBool() );
}

void TestQgsAiSecretStore::plaintextPassthroughWithoutVault()
{
  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( authManager && authManager->masterPasswordIsSet() )
    QSKIP( "Master password already set in this environment: plaintext mode not reachable." );

  QgsAiSecretStore::resetDataKeyCacheForTesting();
  QVERIFY( !QgsAiSecretStore::storageEncryptionAvailable() );

  // Without a data key, encryptValue is a pass-through and decryptValue keeps
  // legacy plaintext readable.
  QCOMPARE( QgsAiSecretStore::encryptValue( u"plain text"_s ), u"plain text"_s );
  QCOMPARE( QgsAiSecretStore::decryptValue( u"plain text"_s ), u"plain text"_s );
  const QByteArray blob( "\x01\x02\x03", 3 );
  QCOMPARE( QgsAiSecretStore::encryptBlob( blob ), blob );
  QCOMPARE( QgsAiSecretStore::decryptBlob( blob ), blob );
}

void TestQgsAiSecretStore::dataKeyBootstrapAndRoundTrip()
{
  if ( !unlockTestVault() )
    QSKIP( "Authentication vault unavailable in this environment." );

  QgsSettings settings;
  const auto cleanup = qScopeGuard( [&settings]() {
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( authManager )
      authManager->removeAuthSetting( u"ai/storage/dataKey"_s );
    settings.remove( u"ai/storage/hasDataKey"_s );
    QgsAiSecretStore::resetDataKeyCacheForTesting();
  } );

  QgsAiSecretStore::resetDataKeyCacheForTesting();

  // First call generates a 32-char base64 key and persists it; later calls are stable.
  const QString key = QgsAiSecretStore::dataEncryptionKey();
  QCOMPARE( key.size(), 32 );
  QVERIFY( settings.value( u"ai/storage/hasDataKey"_s, false ).toBool() );
  QCOMPARE( QgsAiSecretStore::dataEncryptionKey(), key );
  QgsAiSecretStore::resetDataKeyCacheForTesting();
  QCOMPARE( QgsAiSecretStore::dataEncryptionKey(), key );
  QVERIFY( QgsAiSecretStore::storageEncryptionAvailable() );

  // Value round trip with per-call random IVs.
  const QString secretText = u"sensitive layer attribute"_s;
  const QString encryptedA = QgsAiSecretStore::encryptValue( secretText );
  const QString encryptedB = QgsAiSecretStore::encryptValue( secretText );
  QVERIFY( encryptedA.startsWith( u"enc1:"_s ) );
  QVERIFY( encryptedA != encryptedB ); // different IVs
  QVERIFY( !encryptedA.contains( u"sensitive"_s ) );
  QCOMPARE( QgsAiSecretStore::decryptValue( encryptedA ), secretText );
  QCOMPARE( QgsAiSecretStore::decryptValue( encryptedB ), secretText );

  // Blob round trip (embeddings, WKT).
  QByteArray blob;
  for ( int i = 0; i < 64; ++i )
    blob.append( static_cast<char>( i ) );
  const QByteArray encryptedBlob = QgsAiSecretStore::encryptBlob( blob );
  QVERIFY( encryptedBlob.startsWith( "enc1:" ) );
  QCOMPARE( QgsAiSecretStore::decryptBlob( encryptedBlob ), blob );

  // Legacy plaintext values still pass through.
  QCOMPARE( QgsAiSecretStore::decryptValue( u"legacy plaintext"_s ), u"legacy plaintext"_s );
}

QGSTEST_MAIN( TestQgsAiSecretStore )
#include "testqgsaisecretstore.moc"
