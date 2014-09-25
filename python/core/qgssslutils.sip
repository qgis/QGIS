/**
 * @class QgsSslPkiSettings
 * @ingroup core
 * @brief Settings for working with a SSL certificate, key and optional certificate issuer
 * @since since 2.5
 */

class QgsSslPkiSettings
{
  public:

    enum SslStoreType
    {
      QGISStore = 0
    };

    QgsSslPkiSettings();

    QByteArray clientCertData() const;

    QByteArray clientKeyData() const;

    QByteArray issuerCertData() const;

    // TODO: add protocol option, e.g. QSsl::TlsV1SslV3, etc.?

    bool certIsReady() const;
    void setCertReady( bool ready );

    QgsSslPkiSettings::SslStoreType storeType() const;
    void setStoreType( QgsSslPkiSettings::SslStoreType storetype );

    QString certId() const;
    void setCertId( const QString& txtid );

    QString keyId() const;
    void setKeyId( const QString& txtid );

    bool needsKeyPath() const;
    void setNeedsKeyPath( bool needs );

    bool needsKeyPassphrase() const;
    void setNeedsKeyPassphrase( bool has );

    QString keyPassphrase() const;
    void setKeyPassphrase( const QString& passphrase );

    QString issuerId() const;
    void setIssuerId( const QString& txtid );

    bool issuerSelfSigned() const;
    void setIssuerSelfSigned( bool seflsigned );

    QString accessUrl() const;
    void setAccessUrl( const QString& url );
};

/**
 * @class QgsSslPkiGroup
 * @ingroup core
 * @brief Storage set for constructed SSL certificate, key and optional certificate issuer
 * @since 2.6
 */

class QgsSslPkiGroup
{
%TypeHeaderCode
#include <qgssslutils.h>
%End
  public:
    QgsSslPkiGroup( const QSslCertificate& cert = QSslCertificate(), const QSslKey& certkey = QSslKey(),
                    const QSslCertificate& issuer = QSslCertificate(), bool issuerselfsigned = false );
    ~QgsSslPkiGroup();

    bool isNull();

    QSslCertificate clientCert();
    void setClientCert( const QSslCertificate& cert );

    QSslKey clientCertKey();
    void setClientCertKey( const QSslKey& certkey );

    QSslCertificate issuerCert();
    void setIssuerCert( const QSslCertificate& issuer );

    bool issuerSelfSigned();
    void setIssuerSelfSigned( bool selfsigned );
};

/**
  \class QgsSslPkiUtility
  \ingroup core
  \brief Utilites functions for working with SSL certificates, keys and connections
  \since 2.5
*/

class QgsSslPkiUtility
{
%TypeHeaderCode
#include <qgssslutils.h>
%End
  public:

    static QgsSslPkiUtility *instance();

    static bool urlToResource( const QString& accessurl, QString *resource );

    QgsSslPkiGroup * getSslPkiGroup( const QgsSslPkiSettings& pki );
    void putSslPkiGroup( const QgsSslPkiSettings& pki, QgsSslPkiGroup * pkigroup );
    void removeSslPkiGroup( const QgsSslPkiSettings& pki );

    static const QString qgisCertStoreDirPath();

    static bool createQgisCertStoreDir();

    static const QString qgisCertsDirPath();
    static const QString qgisKeysDirPath();
    static const QString qgisIssuersDirPath();

    /**
     * @return NULL QString if file does not exist
     */
    static const QString qgisCertPath( const QString& file );
    static const QString qgisKeyPath( const QString& file );
    static const QString qgisIssuerPath( const QString& file );

    static const QStringList storeCerts( QgsSslPkiSettings::SslStoreType store );
    static const QStringList storeKeys( QgsSslPkiSettings::SslStoreType store );
    static const QStringList storeIssuers( QgsSslPkiSettings::SslStoreType store );

    static QSslCertificate certFromPath( const QString& path, QSsl::EncodingFormat format = QSsl::Pem );

    static QSsl::KeyAlgorithm keyAlgorithm( const QByteArray& keydata );

    void updateRequestSslConfiguration( QNetworkRequest &request /In,Out/, const QgsSslPkiSettings& pki );
    void updateReplyExpectedSslErrors( QNetworkReply *reply, const QgsSslPkiSettings& pki );
};
