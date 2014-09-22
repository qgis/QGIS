/**
 * @class QgsSslCertSettings
 * @ingroup core
 * @brief Settings for working with a SSL certificate, key and optional certificate issuer
 * @since since 2.5
 */

class QgsSslCertSettings
{
  public:

    enum SslStoreType
    {
      QGISStore = 0
    };

    QgsSslCertSettings();

    QSslCertificate clientCert() const;

    QSslKey clientCertKey() const;

    QSslCertificate issuerCert() const;

    // TODO: add protocol option, e.g. QSsl::TlsV1SslV3, etc.?

    bool certIsReady() const;
    void setCertReady( bool ready );

    QgsSslCertSettings::SslStoreType storeType() const;
    void setStoreType( QgsSslCertSettings::SslStoreType storetype );

    QString certId() const;
    void setCertId( const QString& txtid );

    QString keyId() const;
    void setKeyId( const QString& txtid );

    bool hasKeyPassphrase() const;
    void setHasKeyPassphrase( bool has );

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
  \class QgsSslUtils
  \ingroup core
  \brief Utilites functions for working with SSL certificates, keys and connections
  \since 2.5
*/

class QgsSslUtils
{
%TypeHeaderCode
#include <qgssslutils.h>
%End
  public:

    /** Path to user-local QGIS directory for certs, keys and issuers
     */
    static const QString qgisCertStoreDirPath();

    /** Create user-local QGIS directory for certs, keys and issuers
     */
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

    static const QStringList storeCerts( QgsSslCertSettings::SslStoreType store );
    static const QStringList storeKeys( QgsSslCertSettings::SslStoreType store );
    static const QStringList storeIssuers( QgsSslCertSettings::SslStoreType store );

    static QSslCertificate certFromPath( const QString& path, QSsl::EncodingFormat format = QSsl::Pem );

    static QSsl::KeyAlgorithm keyAlgorithm( const QByteArray& keydata );

    static QSslKey keyFromData( const QByteArray& keydata,
                                QSsl::EncodingFormat format = QSsl::Pem,
                                QSsl::KeyType type = QSsl::PrivateKey,
                                bool hasKeyPhrase = false,
                                const QString& passphrase = QString(),
                                const QString& accessurl = QString() );
    static QSslKey keyFromPath( const QString& path,
                                QSsl::EncodingFormat format = QSsl::Pem,
                                QSsl::KeyType type = QSsl::PrivateKey,
                                bool hasKeyPhrase = false,
                                const QString& passphrase = QString(),
                                const QString& accessurl = QString() );

    static const QString keyHashFromData( const QByteArray & data );
    static const QString keyHashFromPath( const QString& path );

    static void updateRequestSslConfiguration( QNetworkRequest &request /In,Out/, const QgsSslCertSettings& pki );
    static void updateReplyExpectedSslErrors( QNetworkReply *reply, const QgsSslCertSettings& pki );
};
