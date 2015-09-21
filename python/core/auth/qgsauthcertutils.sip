class QgsAuthCertUtils
{
%TypeHeaderCode
#include <qgsauthcertutils.h>
%End
  public:
    enum CaCertSource
    {
      SystemRoot = 0,
      FromFile = 1,
      InDatabase = 2,
      Connection = 3
    };

    enum CertTrustPolicy
    {
      DefaultTrust = 0,
      Trusted = 1,
      Untrusted = 2,
      NoPolicy = 3
    };

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

    enum ConstraintGroup
    {
      KeyUsage = 0,
      ExtendedKeyUsage = 1
    };



    static const QString getSslProtocolName( QSsl::SslProtocol protocol );


    static const QMap< QString, QSslCertificate> mapDigestToCerts( QList<QSslCertificate> certs );


    // static const QMap< QString, QList<QSslCertificate> > certsGroupedByOrg( QList<QSslCertificate> certs );


    static const QMap< QString, QgsAuthConfigSslServer> mapDigestToSslConfigs( QList<QgsAuthConfigSslServer> configs );


    // static const QMap< QString, QList<QgsAuthConfigSslServer> > sslConfigsGroupedByOrg( QList<QgsAuthConfigSslServer> configs );


    static const QList<QSslCertificate> certsFromFile( const QString &certspath );


    static const QSslCertificate certFromFile( const QString &certpath );


    static const QSslKey keyFromFile( const QString &keypath,
                                      const QString &keypass = QString(),
                                      QString *algtype = 0 );


    static const QList<QSslCertificate> certsFromString( const QString &pemtext );


    static const QStringList certKeyBundleToPem( const QString &certpath,
                                                 const QString &keypath,
                                                 const QString &keypass = QString(),
                                                 bool reencrypt = true );


    static const QStringList pkcs12BundleToPem( const QString &bundlepath,
                                                const QString &bundlepass = QString(),
                                                bool reencrypt = true );


    static const QString getCaSourceName( QgsAuthCertUtils::CaCertSource source , bool single = false );


    static const QString resolvedCertName( const QSslCertificate& cert, bool issuer = false );


    // static const QString getCertDistinguishedName( const QSslCertificate& qcert,
    //                                                const QCA::Certificate& acert = QCA::Certificate(),
    //                                                bool issuer = false );


    static const QString getCertTrustName( QgsAuthCertUtils::CertTrustPolicy trust );


    static const QString getColonDelimited( const QString& txt );


    static const QString shaHexForCert( const QSslCertificate &cert , bool formatted = false );


    // static const QCA::Certificate qtCertToQcaCert( const QSslCertificate& cert );


    // static const QCA::CertificateCollection qtCertsToQcaCollection( const QList<QSslCertificate>& certs );


    // static const QCA::KeyBundle qcaKeyBundle( const QString &path, const QString &pass );


    // static const QString qcaValidityMessage( QCA::Validity validity );


    // static const QString qcaSignatureAlgorithm( QCA::SignatureAlgorithm algorithm );


    // static const QString qcaKnownConstraint( QCA::ConstraintTypeKnown constraint );


    static const QString certificateUsageTypeString( QgsAuthCertUtils::CertUsageType usagetype );


    static QList<QgsAuthCertUtils::CertUsageType> certificateUsageTypes( const QSslCertificate& cert );


    static bool certificateIsAuthority( const QSslCertificate& cert );


    static bool certificateIsIssuer( const QSslCertificate& cert );


    static bool certificateIsAuthorityOrIssuer( const QSslCertificate& cert );


    static bool certificateIsSslServer( const QSslCertificate& cert );


    static bool certificateIsSslClient( const QSslCertificate& cert );

    static const QString sslErrorEnumString( QSslError::SslError errenum );

    // static const QList<QPair<QSslError::SslError, QString> > sslErrorEnumStrings();
};
