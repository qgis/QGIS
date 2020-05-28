/***************************************************************************
    qgsauthcertutils.h
    ---------------------
    begin                : May 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSAUTHCERTUTILS_H
#define QGSAUTHCERTUTILS_H

#include <QFile>
#include "qgis_sip.h"
#include <QtCrypto>
#include <QSslCertificate>
#include <QSslError>

#include "qgsauthconfig.h"
#include "qgis_core.h"

class QgsAuthConfigSslServer;

#define SSL_ISSUER_INFO( var, prop ) var.issuerInfo( prop ).value(0)

#define SSL_SUBJECT_INFO( var, prop ) var.subjectInfo( prop ).value(0)

/**
 * \ingroup core
 * \brief Utilities for working with certificates and keys
 */
class CORE_EXPORT QgsAuthCertUtils
{
  public:
    //! Type of CA certificate source
    enum CaCertSource
    {
      SystemRoot = 0,
      FromFile = 1,
      InDatabase = 2,
      Connection = 3
    };

    //! Type of certificate trust policy
    enum CertTrustPolicy
    {
      DefaultTrust = 0,
      Trusted = 1,
      Untrusted = 2,
      NoPolicy = 3
    };

    //! Type of certificate usage
    enum CertUsageType
    {
      UndeterminedUsage = 0,
      AnyOrUnspecifiedUsage,
      CertAuthorityUsage,
      CertIssuerUsage,
      TlsServerUsage,
      TlsServerEvUsage,
      TlsClientUsage,
      CodeSigningUsage,
      EmailProtectionUsage,
      TimeStampingUsage,
      CRLSigningUsage
    };

    //! Type of certificate key group
    enum ConstraintGroup
    {
      KeyUsage = 0,
      ExtendedKeyUsage = 1
    };


    //! SSL Protocol name strings per enum
    static QString getSslProtocolName( QSsl::SslProtocol protocol );

    //! Map certificate sha1 to certificate as simple cache
    static QMap<QString, QSslCertificate> mapDigestToCerts( const QList<QSslCertificate> &certs );

    /**
     * Map certificates to their oraganization.
     * \note not available in Python bindings
     */
    static QMap< QString, QList<QSslCertificate> > certsGroupedByOrg( const QList<QSslCertificate> &certs ) SIP_SKIP;

    /**
     * Map SSL custom configs' certificate sha1 to custom config as simple cache
     */
    static QMap<QString, QgsAuthConfigSslServer> mapDigestToSslConfigs( const QList<QgsAuthConfigSslServer> &configs );

    /**
     * Map SSL custom configs' certificates to their oraganization.
     * \note not available in Python bindings
     */
    static QMap< QString, QList<QgsAuthConfigSslServer> > sslConfigsGroupedByOrg( const QList<QgsAuthConfigSslServer> &configs ) SIP_SKIP;

    /**
     * Returns data from a local file via a read-only operation
     * \param path Path to file to read
     * \returns All data contained in file or empty contents if file does not exist
     */
    static QByteArray fileData( const QString &path );

    //! Returns a list of concatenated certs from a PEM or DER formatted file
    static QList<QSslCertificate> certsFromFile( const QString &certspath );

    //! Returns a list of concatenated CAs from a PEM or DER formatted file
    static QList<QSslCertificate> casFromFile( const QString &certspath );

    //! Returns the first cert from a PEM or DER formatted file
    static QSslCertificate certFromFile( const QString &certpath );

    /**
     * \brief casMerge merges two certificate bundles in a single one removing duplicates, the certificates
     * from the \a bundle2 are appended to \a bundle1 if not already there
     * \param bundle1 first bundle
     * \param bundle2 second bundle
     * \return a list of unique certificates
     */
    static QList<QSslCertificate> casMerge( const QList<QSslCertificate> &bundle1,
                                            const QList<QSslCertificate> &bundle2 );

    /**
     * Returns non-encrypted key from a PEM or DER formatted file
     * \param keypath File path to private key
     * \param keypass Passphrase for private key
     * \param algtype QString to set with resolved algorithm type
     */
    static QSslKey keyFromFile( const QString &keypath,
                                const QString &keypass = QString(),
                                QString *algtype = nullptr );

    //! Returns a list of concatenated certs from a PEM Base64 text block
    static QList<QSslCertificate> certsFromString( const QString &pemtext );


    /**
     * \brief casRemoveSelfSigned remove self-signed CA certificates from \a caList
     * \param caList list of CA certificates
     * \return a list of non self-signed certificates
     */
    static QList<QSslCertificate> casRemoveSelfSigned( const QList<QSslCertificate> &caList );

    /**
     * Returns list of certificate, private key and algorithm (as PEM text) from file path components
     * \param certpath File path to certificate
     * \param keypath File path to private key
     * \param keypass Passphrase for private key
     * \param reencrypt Whether to re-encrypt the private key with the passphrase
     * \returns certificate, private key, key's algorithm type
     */
    static QStringList certKeyBundleToPem( const QString &certpath,
                                           const QString &keypath,
                                           const QString &keypass = QString(),
                                           bool reencrypt = true );

    /**
     * Determine if the PEM-encoded text of a key is PKCS#8 format
     * \param keyPemTxt PEM-encoded text
     * \returns TRUE if PKCS#8, otherwise FALSE
     */
    static bool pemIsPkcs8( const QString &keyPemTxt );

#ifdef Q_OS_MAC

    /**
     * Extract the PrivateKey ASN.1 element of a DER-encoded PKCS#8 private key
     * \param pkcs8Der PKCS#8 DER-encoded private key data
     * \returns DER-encoded private key on success or an empty QByteArray upon failure
     * \note On some platforms, e.g. macOS, where the default SSL backend is not OpenSSL, a QSslKey
     * can not be created using PKCS#8-formatted data. However, PKCS#8 private key ASN.1 structures
     * contain the key data inside a wrapper describing the algorithm used, e.g. RSA, DSA, ECC etc.
     * Extracted PrivateKey ASN.1 data can be used to create a compatible QSslKey,
     * e.g. 'traditional' SSLeay RSA-specific PKCS#1.
     * By default OpenSSL 1.0.0+ returns private keys as PKCS#8, previously it was PKCS#1.
     * \note This function requires 'libtasn1' development files and library, which is used
     * to parse and extract the PrivateKey element from an ASN.1 PKCS#8 structure.
     */
    static QByteArray pkcs8PrivateKey( QByteArray &pkcs8Der ) SIP_SKIP;
#endif

    /**
     * Returns list of certificate, private key and algorithm (as PEM text) for a PKCS#12 bundle
     * \param bundlepath File path to the PKCS bundle
     * \param bundlepass Passphrase for bundle
     * \param reencrypt Whether to re-encrypt the private key with the passphrase
     * \returns certificate, private key, key's algorithm type
     */
    static QStringList pkcs12BundleToPem( const QString &bundlepath,
                                          const QString &bundlepass = QString(),
                                          bool reencrypt = true );

    /**
     * Returns list of CA certificates (as QSslCertificate) for a PKCS#12 bundle
     * \param bundlepath File path to the PKCS bundle
     * \param bundlepass Passphrase for bundle
     * \returns list of certificate
     */
    static QList<QSslCertificate> pkcs12BundleCas( const QString &bundlepath,
        const QString &bundlepass = QString() );


    /**
     * \brief certsToPemText dump a list of QSslCertificates to PEM text
     * \param certs list of certs
     * \return a byte array of concatenated certificates as PEM text
     */
    static QByteArray certsToPemText( const QList<QSslCertificate> &certs );

    /**
     * Write a temporary file for a PEM text of cert/key/CAs bundle component
     * \param pemtext Component content as PEM text
     * \param name Name of file
     * \returns File path to temporary file
     */
    static QString pemTextToTempFile( const QString &name, const QByteArray &pemtext );

    /**
     * Gets the general name for CA source enum type
     * \param source The enum source type for the CA
     * \param single Whether to return singular or plural description
     */
    static QString getCaSourceName( QgsAuthCertUtils::CaCertSource source, bool single = false );

    //! Gets the general name via RFC 5280 resolution
    static QString resolvedCertName( const QSslCertificate &cert, bool issuer = false );

    /**
     * Gets combined distinguished name for certificate
     * \param qcert Qt SSL cert object
     * \param acert QCA SSL cert object to add more info to the output
     * \param issuer Whether to return cert's subject or issuer combined name
     * \note not available in Python bindings
     */
    static QString getCertDistinguishedName( const QSslCertificate &qcert,
        const QCA::Certificate &acert = QCA::Certificate(),
        bool issuer = false ) SIP_SKIP;

    //! Gets the general name for certificate trust
    static QString getCertTrustName( QgsAuthCertUtils::CertTrustPolicy trust );

    //! Gets string with colon delimiters every 2 characters
    static QString getColonDelimited( const QString &txt );

    /**
     * Gets the sha1 hash for certificate
     * \param cert Qt SSL certificate to generate hash from
     * \param formatted Whether to colon-delimit the hash
     */
    static QString shaHexForCert( const QSslCertificate &cert, bool formatted = false );

    /**
     * Convert a QSslCertificate to a QCA::Certificate.
     * \note not available in Python bindings
     */
    static QCA::Certificate qtCertToQcaCert( const QSslCertificate &cert ) SIP_SKIP;

    /**
     * Convert a QList of QSslCertificate to a QCA::CertificateCollection.
     * \note not available in Python bindings
     */
    static QCA::CertificateCollection qtCertsToQcaCollection( const QList<QSslCertificate> &certs ) SIP_SKIP;

    /**
     * PKI key/cert bundle from file path, e.g. from .p12 or pfx files.
     * \note not available in Python bindings
     */
    static QCA::KeyBundle qcaKeyBundle( const QString &path, const QString &pass ) SIP_SKIP;

    /**
     * Certificate validity check messages per enum.
     * \note not available in Python bindings
     */
    static QString qcaValidityMessage( QCA::Validity validity ) SIP_SKIP;

    /**
     * Certificate signature algorithm strings per enum.
     * \note not available in Python bindings
     */
    static QString qcaSignatureAlgorithm( QCA::SignatureAlgorithm algorithm ) SIP_SKIP;

    /**
     * Certificate well-known constraint strings per enum.
     * \note not available in Python bindings
     */
    static QString qcaKnownConstraint( QCA::ConstraintTypeKnown constraint ) SIP_SKIP;

    /**
     * Certificate usage type strings per enum
     * \note not available in Python bindings
     */
    static QString certificateUsageTypeString( QgsAuthCertUtils::CertUsageType usagetype ) SIP_SKIP;

    //! Try to determine the certificates usage types
    static QList<QgsAuthCertUtils::CertUsageType> certificateUsageTypes( const QSslCertificate &cert );

    //! Gets whether a certificate is an Authority
    static bool certificateIsAuthority( const QSslCertificate &cert );

    //! Gets whether a certificate can sign other certificates
    static bool certificateIsIssuer( const QSslCertificate &cert );

    //! Gets whether a certificate is an Authority or can at least sign other certificates
    static bool certificateIsAuthorityOrIssuer( const QSslCertificate &cert );

    //! Gets whether a certificate is probably used for a SSL server
    static bool certificateIsSslServer( const QSslCertificate &cert );

    //! Gets whether a certificate is probably used for a client identity
    static bool certificateIsSslClient( const QSslCertificate &cert );

    //! Gets short strings describing an SSL error
    static QString sslErrorEnumString( QSslError::SslError errenum );

    /**
     * Gets short strings describing SSL errors.
     * \note not available in Python bindings
     */
    static QList<QPair<QSslError::SslError, QString> > sslErrorEnumStrings() SIP_SKIP;

    /**
     * \brief certIsCurrent checks if \a cert is viable for its not before and not after dates
     * \param cert certificate to be checked
     */
    static bool certIsCurrent( const QSslCertificate &cert );

    /**
     * \brief certViabilityErrors checks basic characteristics (validity dates, blacklisting, etc.) of given \a cert
     * \param cert certificate to be checked
     * \return list of QSslError (will return NO ERRORS if a null QSslCertificate is passed)
     */
    static QList<QSslError> certViabilityErrors( const QSslCertificate &cert );

    /**
     * \brief certIsViable checks for viability errors of \a cert and whether it is NULL
     * \param cert certificate to be checked
     * \return FALSE if cert is NULL or has viability errors
     */
    static bool certIsViable( const QSslCertificate &cert );

    /**
     * \brief validateCertChain validates the given \a certificateChain
     * \param certificateChain list of certificates to be checked, with leaf first and with optional root CA last
     * \param hostName (optional) name of the host to be verified
     * \param trustRootCa if TRUE the CA will be added to the trusted CAs for this validation check
     * \return list of QSslError, if the list is empty then the cert chain is valid
     */
    static QList<QSslError> validateCertChain( const QList<QSslCertificate> &certificateChain,
        const QString &hostName = QString(),
        bool trustRootCa = false ) ;

    /**
     * \brief validatePKIBundle validate the PKI bundle by checking the certificate chain, the
     * expiration and effective dates, optionally trusts the root CA
     * \param bundle
     * \param useIntermediates if TRUE the intermediate certs are also checked
     * \param trustRootCa if TRUE the CA will be added to the trusted CAs for this validation check (if useIntermediates is FALSE)
     * this option is ignored and set to FALSE
     * \return a list of error strings, if the list is empty then the PKI bundle is valid
     */
    static QStringList validatePKIBundle( QgsPkiBundle &bundle, bool useIntermediates = true, bool trustRootCa = false );

  private:
    static void appendDirSegment_( QStringList &dirname, const QString &segment, QString value );

    static QSsl::EncodingFormat sniffEncoding( const QByteArray &payload );
};

#endif // QGSAUTHCERTUTILS_H
