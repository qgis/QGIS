/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/auth/qgsauthcertutils.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsAuthCertUtils
{
%Docstring
 Utilities for working with certificates and keys
%End

%TypeHeaderCode
#include "qgsauthcertutils.h"
%End
  public:
    enum CaCertSource
    {
      SystemRoot,
      FromFile,
      InDatabase,
      Connection
    };

    enum CertTrustPolicy
    {
      DefaultTrust,
      Trusted,
      Untrusted,
      NoPolicy
    };

    enum CertUsageType
    {
      UndeterminedUsage,
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

    enum ConstraintGroup
    {
      KeyUsage,
      ExtendedKeyUsage
    };


    static QString getSslProtocolName( QSsl::SslProtocol protocol );
%Docstring
SSL Protocol name strings per enum
 :rtype: str
%End

    static QMap<QString, QSslCertificate> mapDigestToCerts( const QList<QSslCertificate> &certs );
%Docstring
Map certificate sha1 to certificate as simple cache
 :rtype: QMap<str, QSslCertificate>
%End


    static QMap<QString, QgsAuthConfigSslServer> mapDigestToSslConfigs( const QList<QgsAuthConfigSslServer> &configs );
%Docstring
 Map SSL custom configs' certificate sha1 to custom config as simple cache
 :rtype: QMap<str, QgsAuthConfigSslServer>
%End


    static QByteArray fileData( const QString &path );
%Docstring
 Return data from a local file via a read-only operation
 \param path Path to file to read
 :return: All data contained in file or empty contents if file does not exist
 :rtype: QByteArray
%End

    static QList<QSslCertificate> certsFromFile( const QString &certspath );
%Docstring
Return list of concatenated certs from a PEM or DER formatted file
 :rtype: list of QSslCertificate
%End

    static QList<QSslCertificate> casFromFile( const QString &certspath );
%Docstring
Return list of concatenated CAs from a PEM or DER formatted file
 :rtype: list of QSslCertificate
%End

    static QSslCertificate certFromFile( const QString &certpath );
%Docstring
Return first cert from a PEM or DER formatted file
 :rtype: QSslCertificate
%End

    static QList<QSslCertificate> casMerge( const QList<QSslCertificate> &bundle1,
                                            const QList<QSslCertificate> &bundle2 );
%Docstring
 casMerge merges two certificate bundles in a single one removing duplicates, the certificates
 from the ``bundle2`` are appended to ``bundle1`` if not already there
 \param bundle1 first bundle
 \param bundle2 second bundle
 :return: a list of unique certificates
 :rtype: list of QSslCertificate
%End

    static QSslKey keyFromFile( const QString &keypath,
                                const QString &keypass = QString(),
                                QString *algtype = 0 );
%Docstring
 Return non-encrypted key from a PEM or DER formatted file
 \param keypath File path to private key
 \param keypass Passphrase for private key
 \param algtype QString to set with resolved algorithm type
 :rtype: QSslKey
%End

    static QList<QSslCertificate> certsFromString( const QString &pemtext );
%Docstring
Return list of concatenated certs from a PEM Base64 text block
 :rtype: list of QSslCertificate
%End


    static QList<QSslCertificate> casRemoveSelfSigned( const QList<QSslCertificate> &caList );
%Docstring
 casRemoveSelfSigned remove self-signed CA certificates from ``caList``
 \param caList list of CA certificates
 :return: a list of non self-signed certificates
 :rtype: list of QSslCertificate
%End

    static QStringList certKeyBundleToPem( const QString &certpath,
                                           const QString &keypath,
                                           const QString &keypass = QString(),
                                           bool reencrypt = true );
%Docstring
 Return list of certificate, private key and algorithm (as PEM text) from file path components
 \param certpath File path to certificate
 \param keypath File path to private key
 \param keypass Passphrase for private key
 \param reencrypt Whether to re-encrypt the private key with the passphrase
 :return: certificate, private key, key's algorithm type
 :rtype: list of str
%End

    static bool pemIsPkcs8( const QString &keyPemTxt );
%Docstring
 Determine if the PEM-encoded text of a key is PKCS#8 format
 \param keyPemTxt PEM-encoded text
 :return: True if PKCS#8, otherwise false
 :rtype: bool
%End



    static QStringList pkcs12BundleToPem( const QString &bundlepath,
                                          const QString &bundlepass = QString(),
                                          bool reencrypt = true );
%Docstring
 Return list of certificate, private key and algorithm (as PEM text) for a PKCS#12 bundle
 \param bundlepath File path to the PKCS bundle
 \param bundlepass Passphrase for bundle
 \param reencrypt Whether to re-encrypt the private key with the passphrase
 :return: certificate, private key, key's algorithm type
 :rtype: list of str
%End

    static QList<QSslCertificate> pkcs12BundleCas( const QString &bundlepath,
        const QString &bundlepass = QString() );
%Docstring
 Return list of CA certificates (as QSslCertificate) for a PKCS#12 bundle
 \param bundlepath File path to the PKCS bundle
 \param bundlepass Passphrase for bundle
 :return: list of certificate
 :rtype: list of QSslCertificate
%End


    static QByteArray certsToPemText( const QList<QSslCertificate> &certs );
%Docstring
 certsToPemText dump a list of QSslCertificates to PEM text
 \param certs list of certs
 :return: a byte array of concatenated certificates as PEM text
 :rtype: QByteArray
%End

    static QString pemTextToTempFile( const QString &name, const QByteArray &pemtext );
%Docstring
 Write a temporary file for a PEM text of cert/key/CAs bundle component
 \param pemtext Component content as PEM text
 \param name Name of file
 :return: File path to temporary file
 :rtype: str
%End

    static QString getCaSourceName( QgsAuthCertUtils::CaCertSource source, bool single = false );
%Docstring
 Get the general name for CA source enum type
 \param source The enum source type for the CA
 \param single Whether to return singular or plural description
 :rtype: str
%End

    static QString resolvedCertName( const QSslCertificate &cert, bool issuer = false );
%Docstring
Get the general name via RFC 5280 resolution
 :rtype: str
%End


    static QString getCertTrustName( QgsAuthCertUtils::CertTrustPolicy trust );
%Docstring
Get the general name for certificate trust
 :rtype: str
%End

    static QString getColonDelimited( const QString &txt );
%Docstring
Get string with colon delimiters every 2 characters
 :rtype: str
%End

    static QString shaHexForCert( const QSslCertificate &cert, bool formatted = false );
%Docstring
 Get the sha1 hash for certificate
 \param cert Qt SSL certificate to generate hash from
 \param formatted Whether to colon-delimit the hash
 :rtype: str
%End








    static QList<QgsAuthCertUtils::CertUsageType> certificateUsageTypes( const QSslCertificate &cert );
%Docstring
Try to determine the certificates usage types
 :rtype: list of QgsAuthCertUtils.CertUsageType
%End

    static bool certificateIsAuthority( const QSslCertificate &cert );
%Docstring
Get whether a certificate is an Authority
 :rtype: bool
%End

    static bool certificateIsIssuer( const QSslCertificate &cert );
%Docstring
Get whether a certificate can sign other certificates
 :rtype: bool
%End

    static bool certificateIsAuthorityOrIssuer( const QSslCertificate &cert );
%Docstring
Get whether a certificate is an Authority or can at least sign other certificates
 :rtype: bool
%End

    static bool certificateIsSslServer( const QSslCertificate &cert );
%Docstring
Get whether a certificate is probably used for a SSL server
 :rtype: bool
%End

    static bool certificateIsSslClient( const QSslCertificate &cert );
%Docstring
Get whether a certificate is probably used for a client identity
 :rtype: bool
%End

    static QString sslErrorEnumString( QSslError::SslError errenum );
%Docstring
Get short strings describing an SSL error
 :rtype: str
%End


    static bool certIsCurrent( const QSslCertificate &cert );
%Docstring
 certIsCurrent checks if ``cert`` is viable for its not before and not after dates
 \param cert certificate to be checked
 :rtype: bool
%End

    static QList<QSslError> certViabilityErrors( const QSslCertificate &cert );
%Docstring
 certViabilityErrors checks basic characteristics (validity dates, blacklisting, etc.) of given ``cert``
 \param cert certificate to be checked
 :return: list of QSslError (will return NO ERRORS if a null QSslCertificate is passed)
 :rtype: list of QSslError
%End

    static bool certIsViable( const QSslCertificate &cert );
%Docstring
 certIsViable checks for viability errors of ``cert`` and whether it is NULL
 \param cert certificate to be checked
 :return: false if cert is NULL or has viability errors
 :rtype: bool
%End

    static QList<QSslError> validateCertChain( const QList<QSslCertificate> &certificateChain,
        const QString &hostName = QString(),
        bool trustRootCa = false ) ;
%Docstring
 validateCertChain validates the given ``certificateChain``
 \param certificateChain list of certificates to be checked, with leaf first and with optional root CA last
 \param hostName (optional) name of the host to be verified
 \param trustRootCa if true the CA will be added to the trusted CAs for this validation check
 :return: list of QSslError, if the list is empty then the cert chain is valid
 :rtype: list of QSslError
%End

    static QStringList validatePKIBundle( QgsPkiBundle &bundle, bool useIntermediates = true, bool trustRootCa = false );
%Docstring
 validatePKIBundle validate the PKI bundle by checking the certificate chain, the
 expiration and effective dates, optionally trusts the root CA
 \param bundle
 \param useIntermediates if true the intermediate certs are also checked
 \param trustRootCa if true the CA will be added to the trusted CAs for this validation check (if useIntermediates is false)
 this option is ignored and set to false
 :return: a list of error strings, if the list is empty then the PKI bundle is valid
 :rtype: list of str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/auth/qgsauthcertutils.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
