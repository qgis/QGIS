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



    static QString getSslProtocolName( QSsl::SslProtocol protocol );


    static QMap< QString, QSslCertificate> mapDigestToCerts( QList<QSslCertificate> certs );


    // static QMap< QString, QList<QSslCertificate> > certsGroupedByOrg( QList<QSslCertificate> certs );


    static QMap< QString, QgsAuthConfigSslServer> mapDigestToSslConfigs( QList<QgsAuthConfigSslServer> configs );


    // static QMap< QString, QList<QgsAuthConfigSslServer> > sslConfigsGroupedByOrg( QList<QgsAuthConfigSslServer> configs );


    static QList<QSslCertificate> certsFromFile( const QString &certspath );


    static QSslCertificate certFromFile( const QString &certpath );


    static QSslKey keyFromFile( const QString &keypath,
                                const QString &keypass = QString(),
                                QString *algtype = 0 );


    static QList<QSslCertificate> certsFromString( const QString &pemtext );


    static QStringList certKeyBundleToPem( const QString &certpath,
                                           const QString &keypath,
                                           const QString &keypass = QString(),
                                           bool reencrypt = true );


    static QStringList pkcs12BundleToPem( const QString &bundlepath,
                                          const QString &bundlepass = QString(),
                                          bool reencrypt = true );


    static QString getCaSourceName( QgsAuthCertUtils::CaCertSource source , bool single = false );


    static QString resolvedCertName( const QSslCertificate& cert, bool issuer = false );


    // static QString getCertDistinguishedName( const QSslCertificate& qcert,
    //                                                const QCA::Certificate& acert = QCA::Certificate(),
    //                                                bool issuer = false );


    static QString getCertTrustName( QgsAuthCertUtils::CertTrustPolicy trust );


    static QString getColonDelimited( const QString& txt );


    static QString shaHexForCert( const QSslCertificate &cert , bool formatted = false );


    // static QCA::Certificate qtCertToQcaCert( const QSslCertificate& cert );


    // static QCA::CertificateCollection qtCertsToQcaCollection( const QList<QSslCertificate>& certs );


    // static QCA::KeyBundle qcaKeyBundle( const QString &path, const QString &pass );


    // static QString qcaValidityMessage( QCA::Validity validity );


    // static QString qcaSignatureAlgorithm( QCA::SignatureAlgorithm algorithm );


    // static QString qcaKnownConstraint( QCA::ConstraintTypeKnown constraint );


    static QString certificateUsageTypeString( QgsAuthCertUtils::CertUsageType usagetype );


    static QList<QgsAuthCertUtils::CertUsageType> certificateUsageTypes( const QSslCertificate& cert );


    static bool certificateIsAuthority( const QSslCertificate& cert );


    static bool certificateIsIssuer( const QSslCertificate& cert );


    static bool certificateIsAuthorityOrIssuer( const QSslCertificate& cert );


    static bool certificateIsSslServer( const QSslCertificate& cert );


    static bool certificateIsSslClient( const QSslCertificate& cert );

    static QString sslErrorEnumString( QSslError::SslError errenum );

    // static QList<QPair<QSslError::SslError, QString> > sslErrorEnumStrings();
};
