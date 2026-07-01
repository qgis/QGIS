/***************************************************************************
    qgsaisecretstore.cpp
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaisecretstore.h"

#include "qgsapplication.h"
#include "qgsauthcrypto.h"
#include "qgsauthmanager.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QRandomGenerator>
#include <QString>

using namespace Qt::StringLiterals;

namespace
{
  constexpr const char *MIGRATION_FLAG_KEY = "ai/security/secretsMigrated_v1";
  constexpr const char *DATA_KEY_VAULT_KEY = "ai/storage/dataKey";
  constexpr const char *DATA_KEY_FLAG_KEY = "ai/storage/hasDataKey";
  constexpr const char *ENCRYPTED_VALUE_PREFIX = "enc1:";
  // QgsAuthCrypto convention: 32 random bytes, hex-encoded (OpenSSL uses the first 16).
  constexpr int CIPHER_IV_BYTES = 32;

  QString sCachedDataKey;
  bool sDataKeyResolved = false;
  QMutex sDataKeyMutex;
  bool sPlaintextStorageWarned = false;
  bool sDecryptFailureWarned = false;

  const QStringList &legacySecretKeys()
  {
    static const QStringList keys = {
      u"ai/provider/openai/apiKey"_s,
      u"ai/provider/claude/apiKey"_s,
      u"ai/provider/openrouter/apiKey"_s,
      u"ai/provider/codex/oauth/refreshToken"_s,
    };
    return keys;
  }

  QString randomHex( int bytes )
  {
    QByteArray raw( bytes, Qt::Uninitialized );
    QRandomGenerator::system()->fillRange( reinterpret_cast<quint32 *>( raw.data() ), bytes / static_cast<int>( sizeof( quint32 ) ) );
    return QString::fromLatin1( raw.toHex() );
  }
} //namespace

bool QgsAiSecretStore::sCleartextWarned = false;
bool QgsAiSecretStore::sMigrationRetryRegistered = false;

bool QgsAiSecretStore::vaultUsable()
{
  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( !authManager || authManager->isDisabled() )
    return false;

  // Best-effort, never-prompt policy: the vault is used ONLY when it is
  // already unlocked for this session. Calling storeAuthSetting/authSetting on
  // a locked vault would trigger a keychain read (macOS ACL prompt on every
  // dev rebuild) or a master password dialog — the AI store must never be the
  // one to initiate that, from any thread. masterPasswordIsSet() is a cached
  // const check, so this is also safe from worker threads (indexing task,
  // remote embedding batches).
  return authManager->masterPasswordIsSet();
}

QString QgsAiSecretStore::flagKey( const QString &secretKey )
{
  return secretKey + u"_inVault"_s;
}

void QgsAiSecretStore::warnCleartextOnce()
{
  if ( sCleartextWarned )
    return;
  sCleartextWarned = true;
  QgsMessageLog::
    logMessage( u"AI credentials are stored unencrypted. Unlock or set a QGIS master password (Settings ▸ Options ▸ Authentication) to store them encrypted in the authentication vault."_s, u"AI/Security"_s, Qgis::MessageLevel::Warning, false );
}

QString QgsAiSecretStore::readSecret( const QString &key, const QStringList &envFallbacks )
{
  QgsSettings settings;

  // Vault first, when the secret is flagged as stored there.
  if ( settings.value( flagKey( key ), false ).toBool() && vaultUsable() )
  {
    QgsAuthManager *authManager = QgsApplication::authManager();
    const QString value = authManager->authSetting( key, QVariant(), true ).toString().trimmed();
    if ( !value.isEmpty() )
      return value;
  }
  else if ( vaultUsable() )
  {
    // Legacy vault entries written by older builds without the presence flag
    // (e.g. Plan session token, Claude OAuth refresh token). Only read when
    // the vault is ALREADY unlocked; existsAuthSetting() never decrypts, so
    // this branch can never trigger the master password prompt.
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( authManager->existsAuthSetting( key ) )
    {
      const QString value = authManager->authSetting( key, QVariant(), true ).toString().trimmed();
      if ( !value.isEmpty() )
      {
        settings.setValue( flagKey( key ), true );
        return value;
      }
    }
  }

  // Legacy cleartext value (also the storage fallback when the vault is unavailable).
  const QString legacy = settings.value( key ).toString().trimmed();
  if ( !legacy.isEmpty() )
  {
    // Opportunistic migration: only when the vault is already unlocked, so a
    // read can never trigger an interactive prompt.
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( authManager && !authManager->isDisabled() && authManager->masterPasswordIsSet() && authManager->storeAuthSetting( key, legacy, true ) )
    {
      settings.setValue( flagKey( key ), true );
      settings.remove( key );
      QgsMessageLog::logMessage( u"Migrated AI credential '%1' into the encrypted authentication vault."_s.arg( key ), u"AI/Security"_s, Qgis::MessageLevel::Info, false );
    }
    return legacy;
  }

  for ( const QString &envName : envFallbacks )
  {
    const QString envValue = qEnvironmentVariable( envName.toUtf8().constData() ).trimmed();
    if ( !envValue.isEmpty() )
      return envValue;
  }

  return QString();
}

bool QgsAiSecretStore::writeSecret( const QString &key, const QString &value )
{
  QgsSettings settings;

  if ( vaultUsable() )
  {
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( authManager->storeAuthSetting( key, value, true ) )
    {
      settings.setValue( flagKey( key ), true );
      // Kill any cleartext copy left behind by older versions.
      settings.remove( key );
      return true;
    }
  }

  // Fallback: cleartext settings — never break the configuration flow.
  warnCleartextOnce();
  settings.setValue( key, value );
  settings.remove( flagKey( key ) );
  return true;
}

void QgsAiSecretStore::removeSecret( const QString &key )
{
  QgsSettings settings;
  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( authManager && !authManager->isDisabled() && settings.value( flagKey( key ), false ).toBool() )
    authManager->removeAuthSetting( key );
  settings.remove( flagKey( key ) );
  settings.remove( key );
}

bool QgsAiSecretStore::hasSecret( const QString &key )
{
  QgsSettings settings;
  if ( settings.value( flagKey( key ), false ).toBool() )
    return true;
  return !settings.value( key ).toString().trimmed().isEmpty();
}

void QgsAiSecretStore::migrateLegacySecrets()
{
  QgsSettings settings;
  if ( settings.value( QString::fromUtf8( MIGRATION_FLAG_KEY ), false ).toBool() )
    return;

  bool anyPending = false;
  for ( const QString &key : legacySecretKeys() )
  {
    if ( !settings.value( key ).toString().trimmed().isEmpty() )
    {
      anyPending = true;
      break;
    }
  }
  if ( !anyPending )
  {
    settings.setValue( QString::fromUtf8( MIGRATION_FLAG_KEY ), true );
    return;
  }

  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( !authManager || authManager->isDisabled() )
  {
    QgsMessageLog::logMessage( u"AI credentials are stored unencrypted and the authentication vault is unavailable: migration skipped."_s, u"AI/Security"_s, Qgis::MessageLevel::Warning, false );
    return;
  }

  // Strictly non-interactive at startup: defer until the vault gets unlocked
  // (keychain auto-password or first interactive use), retry then or next start.
  if ( !authManager->masterPasswordIsSet() )
  {
    if ( !sMigrationRetryRegistered )
    {
      sMigrationRetryRegistered = true;
      QObject::connect(
        authManager,
        &QgsAuthManager::masterPasswordVerified,
        authManager,
        []( bool verified ) {
          if ( verified )
            QgsAiSecretStore::migrateLegacySecrets();
        },
        Qt::SingleShotConnection
      );
    }
    QgsMessageLog::logMessage( u"AI credential migration into the authentication vault deferred until the vault is unlocked."_s, u"AI/Security"_s, Qgis::MessageLevel::Info, false );
    return;
  }

  bool allMigrated = true;
  for ( const QString &key : legacySecretKeys() )
  {
    const QString value = settings.value( key ).toString().trimmed();
    if ( value.isEmpty() )
      continue;

    if ( authManager->storeAuthSetting( key, value, true ) )
    {
      settings.setValue( flagKey( key ), true );
      settings.remove( key );
      QgsMessageLog::logMessage( u"Migrated AI credential '%1' into the encrypted authentication vault."_s.arg( key ), u"AI/Security"_s, Qgis::MessageLevel::Info, false );
    }
    else
    {
      allMigrated = false;
      QgsMessageLog::logMessage( u"Failed to migrate AI credential '%1' into the authentication vault; will retry next start."_s.arg( key ), u"AI/Security"_s, Qgis::MessageLevel::Warning, false );
    }
  }

  if ( allMigrated )
    settings.setValue( QString::fromUtf8( MIGRATION_FLAG_KEY ), true );
}

QString QgsAiSecretStore::dataEncryptionKey()
{
  QMutexLocker locker( &sDataKeyMutex );
  if ( sDataKeyResolved )
    return sCachedDataKey;

  if ( QgsAuthCrypto::isDisabled() || !vaultUsable() )
  {
    // Not resolved permanently: the vault may become usable later in the session.
    return QString();
  }

  QgsAuthManager *authManager = QgsApplication::authManager();
  QgsSettings settings;

  if ( settings.value( QString::fromUtf8( DATA_KEY_FLAG_KEY ), false ).toBool() )
  {
    const QString stored = authManager->authSetting( QString::fromUtf8( DATA_KEY_VAULT_KEY ), QVariant(), true ).toString().trimmed();
    if ( !stored.isEmpty() )
    {
      sCachedDataKey = stored;
      sDataKeyResolved = true;
      return sCachedDataKey;
    }
  }

  // First use: generate 24 random bytes → base64 is exactly 32 chars, so the
  // UTF-8 passphrase bytes form a full-strength AES-256 key for QgsAuthCrypto.
  QByteArray raw( 24, Qt::Uninitialized );
  QRandomGenerator::system()->fillRange( reinterpret_cast<quint32 *>( raw.data() ), 6 );
  const QString key = QString::fromLatin1( raw.toBase64() );

  if ( !authManager->storeAuthSetting( QString::fromUtf8( DATA_KEY_VAULT_KEY ), key, true ) )
    return QString();

  settings.setValue( QString::fromUtf8( DATA_KEY_FLAG_KEY ), true );
  sCachedDataKey = key;
  sDataKeyResolved = true;
  QgsMessageLog::logMessage( u"Generated the AI storage encryption key (stored in the authentication vault)."_s, u"AI/Security"_s, Qgis::MessageLevel::Info, false );
  return sCachedDataKey;
}

bool QgsAiSecretStore::storageEncryptionAvailable()
{
  return !dataEncryptionKey().isEmpty();
}

void QgsAiSecretStore::warnPlaintextStorageOnce()
{
  if ( sPlaintextStorageWarned )
    return;
  sPlaintextStorageWarned = true;
  QgsMessageLog::
    logMessage( u"AI data (RAG index / chat history) is stored unencrypted. Unlock or set a QGIS master password (Settings ▸ Options ▸ Authentication) to enable encryption at rest."_s, u"AI/Security"_s, Qgis::MessageLevel::Warning, false );
}

QString QgsAiSecretStore::encryptValue( const QString &plain )
{
  const EncryptionResult result = tryEncryptValue( plain );
  return result.encrypted ? result.value : plain;
}

QgsAiSecretStore::EncryptionResult QgsAiSecretStore::tryEncryptValue( const QString &plain )
{
  if ( plain.isEmpty() )
    return { QString(), true, false, QString() };

  const QString key = dataEncryptionKey();
  if ( key.isEmpty() )
    return { plain, false, false, u"AI storage encryption key is unavailable."_s };

  const QString ivHex = randomHex( CIPHER_IV_BYTES );
  const QString cipherHex = QgsAuthCrypto::encrypt( key, ivHex, plain );
  if ( cipherHex.isEmpty() )
    return { plain, false, false, u"AI storage encryption failed."_s };

  return { QString::fromUtf8( ENCRYPTED_VALUE_PREFIX ) + ivHex + ':' + cipherHex, true, true, QString() };
}

QString QgsAiSecretStore::decryptValue( const QString &stored )
{
  // Legacy plaintext rows (or plaintext-mode DBs) pass through untouched.
  if ( !stored.startsWith( QString::fromUtf8( ENCRYPTED_VALUE_PREFIX ) ) )
    return stored;

  const QString payload = stored.mid( static_cast<int>( qstrlen( ENCRYPTED_VALUE_PREFIX ) ) );
  const int separator = payload.indexOf( ':' );
  const QString key = dataEncryptionKey();
  if ( separator <= 0 || key.isEmpty() )
  {
    if ( !sDecryptFailureWarned )
    {
      sDecryptFailureWarned = true;
      QgsMessageLog::logMessage( u"Cannot decrypt AI data: the storage encryption key is unavailable (vault locked or reset). Affected values are skipped."_s, u"AI/Security"_s, Qgis::MessageLevel::Warning, false );
    }
    return QString();
  }

  const QString ivHex = payload.left( separator );
  const QString cipherHex = payload.mid( separator + 1 );
  const QString plain = QgsAuthCrypto::decrypt( key, ivHex, cipherHex );
  if ( plain.isEmpty() && !cipherHex.isEmpty() && !sDecryptFailureWarned )
  {
    sDecryptFailureWarned = true;
    QgsMessageLog::logMessage( u"Failed to decrypt AI data: the storage encryption key does not match (vault reset?). Affected values are skipped."_s, u"AI/Security"_s, Qgis::MessageLevel::Warning, false );
  }
  return plain;
}

QByteArray QgsAiSecretStore::encryptBlob( const QByteArray &blob )
{
  const BlobEncryptionResult result = tryEncryptBlob( blob );
  return result.encrypted ? result.value : blob;
}

QgsAiSecretStore::BlobEncryptionResult QgsAiSecretStore::tryEncryptBlob( const QByteArray &blob )
{
  if ( blob.isEmpty() )
    return { QByteArray(), true, false, QString() };
  const EncryptionResult encrypted = tryEncryptValue( QString::fromLatin1( blob.toBase64() ) );
  if ( !encrypted.ok || !encrypted.encrypted )
    return { blob, encrypted.ok, false, encrypted.errorMessage };
  return { encrypted.value.toLatin1(), true, true, QString() };
}

QByteArray QgsAiSecretStore::decryptBlob( const QByteArray &stored )
{
  if ( !stored.startsWith( ENCRYPTED_VALUE_PREFIX ) )
    return stored; // legacy plaintext blob
  const QString base64 = decryptValue( QString::fromLatin1( stored ) );
  if ( base64.isEmpty() )
    return QByteArray();
  return QByteArray::fromBase64( base64.toLatin1() );
}

void QgsAiSecretStore::resetDataKeyCacheForTesting()
{
  QMutexLocker locker( &sDataKeyMutex );
  sCachedDataKey.clear();
  sDataKeyResolved = false;
}
