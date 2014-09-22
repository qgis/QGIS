/***************************************************************************
    qgssslutils.h
    ---------------------
    begin                : 2014/09/12
    copyright            : (C) 2014 by Boundless Spatial, Inc.
    web                  : http://boundlessgeo.com
    author               : Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSSLUTILS_H
#define QGSSSLUTILS_H

#include <QCryptographicHash>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslCertificate>
#include <QSslKey>
#include <QUrl>


/**
 * @class QgsSslCertSettings
 * @ingroup core
 * @brief Settings for working with a SSL certificate, key and optional certificate issuer
 * @since since 2.6
 */

class CORE_EXPORT QgsSslCertSettings
{
  public:

    /**
     * @brief The SslStoreType enum defines the type of certificate store, e.g. local ~/.qgis2/cert_store,
     * which can contain client 'certs', client certificate 'private' keys, and any certificate 'issuers'
     * @since 2.6
     */
    enum SslStoreType
    {
      QGISStore = 0
    };

    QgsSslCertSettings();

    /**
     * @brief Get the client certificate for this PKI session
     */
    QSslCertificate clientCert() const;

    /**
     * @brief Get the client certificate key for this PKI session
     */
    QSslKey clientCertKey() const;

    /**
     * @brief Get the client certificate's issuer certificate (optional) for this PKI session.
     * The certificate can be installed in the main OpenSSL store, but defining it here allows it
     * to specifically skip the blocking error of being self-signed.
     */
    QSslCertificate issuerCert() const;

    // TODO: add protocol option, e.g. QSsl::TlsV1SslV3, etc.?

    bool certIsReady() const;
    /**
     * @brief Convenience function to set whether the cert is ready for SSL connection.
     * @note Only useful if you have just validated the cert outside of settings, otherwise may become stale.
     */
    void setCertReady( bool ready ) { mCertReady = ready; }

    /**
     * @brief The certificate store type, generally defauts to QGIS local store at ~/.qgis2/cert_store,
     * which is then added to the underlying OpenSSL store certificate chain.
     */
    QgsSslCertSettings::SslStoreType storeType() const { return mStoreType; }
    void setStoreType( QgsSslCertSettings::SslStoreType storetype ) { mStoreType = storetype; }

    /**
     * @brief String representation of the client certificate, e.g. file's basename.ext for QGIS local store.
     * For other stores, which are not file-based, it may be the PKI component's serial number, etc.
     */
    QString certId() const { return mCertId; }
    void setCertId( const QString& txtid ) { mCertId = txtid; }

    /**
     * @brief String representation of the client certificate's private key, e.g. file's basename.ext for QGIS local store.
     * For other stores, which are not file-based, it may be the PKI component's serial number, etc.
     */
    QString keyId() const { return mKeyId; }
    void setKeyId( const QString& txtid ) { mKeyId = txtid; }

    /**
     * @brief Whether the private key for the client certificate is protected by a passphrase.
     */
    bool hasKeyPassphrase() const { return mHasKeyPass; }
    void setHasKeyPassphrase( bool has ) { mHasKeyPass = has; }

    /**
     * @brief Temporary storage for the passphrase for private key of the client certificate,
     * when such a passphrase has been passed to the application in a obfuscated manner, e.g. commandline.
     * @note Do NOT write this out to disk. Upon valid connection, it is cached elsewhere and can be deleted.
     * Do NOT consider this secure storage for the password.
     */
    QString keyPassphrase() const { return mKeyPass; }
    void setKeyPassphrase( const QString& passphrase ) { mKeyPass = passphrase; }

    /**
     * @brief String representation of the client certificate's certificate issuer, e.g. file's basename.ext for QGIS local store.
     * For other stores, which are not file-based, it may be the PKI component's serial number, etc.
     */
    QString issuerId() const { return mIssuerCertId; }
    void setIssuerId( const QString& txtid ) { mIssuerCertId = txtid; }

    /**
     * @brief Whether the certificate issuer's trust chain is self-signed.
     * Only set this for issuer certificates whose origin you TRUST.
     */
    bool issuerSelfSigned() const { return mIssuerSelf; }
    void setIssuerSelfSigned( bool seflsigned ) { mIssuerSelf = seflsigned; }

    /**
     * @brief Some URL associated with the SSL access. It is not used for any validation of PKI components.
     * Shown in the private key's password input dialog, so user remembers what scheme://domain.tld:port the key was meant for.
     */
    QString accessUrl() const { return mAccessUrl; }
    void setAccessUrl( const QString& url ) { mAccessUrl = url; }

  private:
    bool mCertReady;

    QgsSslCertSettings::SslStoreType mStoreType;
    QString mCertId;
    QString mKeyId;
    bool mHasKeyPass;
    QString mKeyPass;
    QString mIssuerCertId;
    bool mIssuerSelf;
    QString mAccessUrl;
};

/**
 * @class QgsSslUtils
 * @ingroup core
 * @brief Utilites functions for working with SSL certificates, keys and connections
 * @since 2.6
 */

class CORE_EXPORT QgsSslUtils
{

  public:
    /**
     * @brief Path to user-local QGIS directory for certs, keys and issuers
     */
    static const QString qgisCertStoreDirPath();

    /**
     * @brief Create user-local QGIS directory for certs, keys and issuers.
     * Done automatically upon startup of QGIS Desktop, other uses will need to call this manually.
     */
    static bool createQgisCertStoreDir();

    /**
     * @brief Path specific to QGIS local store: ~/.qgis2/certs
     */
    static const QString qgisCertsDirPath();
    /**
     * @brief Path specific to QGIS local store: ~/.qgis2/private
     */
    static const QString qgisKeysDirPath();
    /**
     * @brief Path specific to QGIS local store: ~/.qgis2/issuers
     */
    static const QString qgisIssuersDirPath();

    /**
     * @brief Absolute path of file in QGIS local store: ~/.qgis2/certs/file
     * @return NULL QString if file does not exist
     */
    static const QString qgisCertPath( const QString& file );
    /**
     * @brief Absolute path of file in QGIS local store: ~/.qgis2/private/file
     * @return NULL QString if file does not exist
     */
    static const QString qgisKeyPath( const QString& file );
    /**
     * @brief Absolute path of file in QGIS local store: ~/.qgis2/issuers/file
     * @return NULL QString if file does not exist
     */
    static const QString qgisIssuerPath( const QString& file );

    // TODO: return different collection type to accommodate all stores (list of QVariants?)
    /**
     * @brief Filtered list of all available PEM PKI client certificates.
     * @param store The store type, e.g. QGIS local store
     */
    static const QStringList storeCerts( QgsSslCertSettings::SslStoreType store );
    /**
     * @brief Filtered list of all available PEM PKI client certificate keys.
     * @param store The store type, e.g. QGIS local store
     */
    static const QStringList storeKeys( QgsSslCertSettings::SslStoreType store );
    /**
     * @brief Filtered list of all available PEM PKI client certificate issuers.
     * @param store The store type, e.g. QGIS local store
     */
    static const QStringList storeIssuers( QgsSslCertSettings::SslStoreType store );

    /**
     * @brief Get a certificate from an absolute file path.
     */
    static QSslCertificate certFromPath( const QString& path, QSsl::EncodingFormat format = QSsl::Pem );

    /**
     * @brief Get the deduced encryption algorithm used for the client certificate's key.
     */
    static QSsl::KeyAlgorithm keyAlgorithm( const QByteArray& keydata );

    /**
     * @brief Get a private/public key from provided data
     * @param hasKeyPhrase Whether the key is passphrase-protected
     * @param passphrase Passphrase of key (if set, you do not need hasKeyPhrase)
     * @param accessurl Some URL associated with the SSL access. It is not used for any validation of PKI components.
     * Shown in the private key's password input dialog, so user remembers what scheme://domain.tld:port the key was meant for.
     */
    static QSslKey keyFromData( const QByteArray& keydata,
                                QSsl::EncodingFormat format = QSsl::Pem,
                                QSsl::KeyType type = QSsl::PrivateKey,
                                bool hasKeyPhrase = false,
                                const QString& passphrase = QString(),
                                const QString& accessurl = QString() );
    /**
     * @brief Get a private/public key from provided absolute file path
     * @param hasKeyPhrase Whether the key is passphrase-protected
     * @param passphrase Passphrase of key (if set, you do not need hasKeyPhrase)
     * @param accessurl Some URL associated with the SSL access. It is not used for any validation of PKI components.
     * Shown in the private key's password input dialog, so user remembers what scheme://domain.tld:port the key was meant for.
     */
    static QSslKey keyFromPath( const QString& path,
                                QSsl::EncodingFormat format = QSsl::Pem,
                                QSsl::KeyType type = QSsl::PrivateKey,
                                bool hasKeyPhrase = false,
                                const QString& passphrase = QString(),
                                const QString& accessurl = QString() );

    /**
     * @brief Get hash string from data to be used as key for passphrase storage
     * @return Hex-based checksum string
     */
    static const QString keyHashFromData( const QByteArray & data );
    /**
     * @brief Get hash string from absolute file path to be used as key for passphrase storage
     * @return Hex-based checksum string
     */
    static const QString keyHashFromPath( const QString& path );

    /**
     * @brief Update existing network request SslConfiguration with PKI connection settings
     * @note This should set up the connection with everything it needs for an (successful) SSL handshake
     */
    static void updateRequestSslConfiguration( QNetworkRequest &request, const QgsSslCertSettings& pki );
    /**
     * @brief Update existing network reply with expected SSL errors derived from PKI connection settings
     * @note Generally this is used for ignoring self-signed issuer certificates, individually.
     */
    static void updateReplyExpectedSslErrors( QNetworkReply *reply, const QgsSslCertSettings& pki );
};

#endif // QGSSSLUTILS_H
