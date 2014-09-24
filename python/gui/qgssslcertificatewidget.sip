
class QgsSslCertificateWidget : QWidget
{
%TypeHeaderCode
#include "qgssslcertificatewidget.h"
%End

  public:
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    /** Widget for choosing/inspecting SSL certificate, using different store methods, and testing validity
     */
    explicit QgsSslCertificateWidget( QWidget *parent /TransferThis/ = 0, const QString& accessUrl = QString( "" ) );
    ~QgsSslCertificateWidget();

    void loadSettings( QgsSslPkiSettings::SslStoreType storeType = QgsSslPkiSettings::QGISStore,
                       const QString& certId = "",
                       const QString& keyId = "",
                       bool needsKeyPath = false,
                       bool needsKeyPhrase = false,
                       const QString& issuerId = "",
                       bool issuerSelf = false );

    void loadSslCertSettings( const QgsSslPkiSettings& pki );

    QgsSslPkiSettings toSslCertSettings();

    bool certIsValid();

    QgsSslPkiSettings::SslStoreType ssLStoreType();
    QString certId() const;
    QString keyId() const;
    bool needsKeyPath();
    bool needsKeyPassphrase();
    QString issuerId() const;
    bool issuerSelfSigned();
    QString accessUrl() const;

  public slots:
    void setSslStoreType( QgsSslPkiSettings::SslStoreType storeType );
    void setCertId( const QString& id );
    void setKeyId( const QString& id );
    void setNeedsKeyPath( bool needsPath );
    void setNeedsKeyPassphrase( bool needsPass );
    void setIssuerId( const QString& id );
    void setIssuerSelfSigned( bool selfSigned );
    void setAccessUrl( const QString& url );

    void validateCert();
};
