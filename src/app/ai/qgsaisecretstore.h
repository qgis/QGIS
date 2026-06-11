/***************************************************************************
    qgsaisecretstore.h
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

#ifndef QGSAISECRETSTORE_H
#define QGSAISECRETSTORE_H

#include "qgis_app.h"

#include <QString>
#include <QStringList>

/**
 * Central store for AI provider secrets (API keys, OAuth refresh tokens).
 *
 * Secrets live in the encrypted QGIS authentication vault (qgis-auth.db) when
 * it is usable; a cleartext QgsSettings fallback keeps configuration working
 * (with a once-per-session warning) when the vault is unavailable — e.g. QCA
 * missing, no master password, or headless test runs.
 *
 * A cleartext boolean presence flag (`<secretKey>_inVault`) marks secrets that
 * live in the vault, so existence checks never need to unlock it.
 *
 * Interactive vault unlock (master password prompt / keychain read) is only
 * allowed after QgisApp calls setInteractiveUnlockAllowed(true) — test
 * binaries never enable it, keeping them deterministic and keychain-free.
 *
 * Key-loss note: if qgis-auth.db or the master password is lost, vault-stored
 * secrets are unrecoverable (same blast radius as any other QGIS credential);
 * the user simply re-enters the API keys.
 */
class APP_EXPORT QgsAiSecretStore
{
  public:
    //! Allows the vault to trigger interactive unlock (GUI only; never call in tests).
    static void setInteractiveUnlockAllowed( bool allowed );

    //! True when the auth vault can be used for reads/writes under the current unlock policy.
    static bool vaultUsable();

    //! Cleartext presence-flag key for \a secretKey.
    static QString flagKey( const QString &secretKey );

    /**
     * Returns the secret for \a key: vault (when flagged and usable) →
     * legacy cleartext QgsSettings (opportunistically migrated to the vault
     * when silently possible) → first non-empty \a envFallbacks variable.
     */
    static QString readSecret( const QString &key, const QStringList &envFallbacks = QStringList() );

    /**
     * Stores \a value for \a key in the vault when usable (removing any
     * cleartext copy), otherwise falls back to cleartext QgsSettings with a
     * once-per-session warning. Never fails the configuration flow.
     */
    static bool writeSecret( const QString &key, const QString &value );

    //! Removes the secret from the vault, the presence flag and any cleartext copy.
    static void removeSecret( const QString &key );

    //! True when a secret exists (vault flag or cleartext). Never unlocks the vault.
    static bool hasSecret( const QString &key );

    /**
     * One-time migration of legacy cleartext secrets (provider API keys and the
     * Codex OAuth refresh token) into the vault. Strictly non-interactive: when
     * the vault is locked the migration is deferred (single-shot retry on
     * master-password verification, otherwise next start).
     */
    static void migrateLegacySecrets();

    // --- Encryption-at-rest helpers for the AI data stores (RAG index, chat history) ---

    /**
     * Returns the AES-256 data key used to encrypt the AI stores, generating and
     * persisting it in the vault on first use. The key is 24 random bytes encoded
     * as base64 (exactly 32 characters, since QgsAuthCrypto uses the UTF-8 bytes
     * of the passphrase directly as the AES-256 key). Empty when the vault or QCA
     * is unavailable — callers then fall back to plaintext storage.
     */
    static QString dataEncryptionKey();

    //! True when encryptValue/decryptValue can actually encrypt (QCA + vault + data key).
    static bool storageEncryptionAvailable();

    /**
     * Encrypts \a plain with the data key and a per-value random IV. Format:
     * `enc1:<ivHex>:<cipherHex>`. Returns \a plain unchanged when encryption is
     * unavailable, and an empty string for empty input.
     */
    static QString encryptValue( const QString &plain );

    /**
     * Decrypts a value produced by encryptValue(). Values without the `enc1:`
     * prefix are returned as-is (legacy plaintext rows, mixed-mode DBs). Returns
     * an empty string when decryption fails (missing/rotated key), with one
     * warning per session.
     */
    static QString decryptValue( const QString &stored );

    //! Blob variants: base64-encode, then encryptValue. Pass-through for non-encrypted blobs.
    static QByteArray encryptBlob( const QByteArray &blob );
    static QByteArray decryptBlob( const QByteArray &stored );

    //! Logs the "AI data stored unencrypted" warning once per session.
    static void warnPlaintextStorageOnce();

    //! Clears the cached data key (unit tests only).
    static void resetDataKeyCacheForTesting();

  private:
    static void warnCleartextOnce();

    static bool sInteractiveUnlockAllowed;
    static bool sCleartextWarned;
    static bool sMigrationRetryRegistered;
};

#endif // QGSAISECRETSTORE_H
