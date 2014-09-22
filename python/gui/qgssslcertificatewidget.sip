
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

    void loadSettings( QgsSslCertSettings::SslStoreType storeType = QgsSslCertSettings::QGISStore,
                       const QString& certId = "",
                       const QString& keyId = "",
                       bool keyPassphrase = false,
                       const QString& issuerId = "",
                       bool issuerSelf = false );

    void loadSslCertSettings( const QgsSslCertSettings& pki );

    QgsSslCertSettings toSslCertSettings();

    bool certIsValid();

    QgsSslCertSettings::SslStoreType ssLStoreType();
    QString certId() const;
    QString keyId() const;
    bool keyHasPassPhrase();
    QString issuerId() const;
    bool issuerSelfSigned();
    QString accessUrl() const;

  public slots:
    void setSslStoreType( QgsSslCertSettings::SslStoreType storeType );
    void setCertId( const QString& id );
    void setKeyId( const QString& id );
    void setKeyHasPassPhrase( bool hasPass );
    void setIssuerId( const QString& id );
    void setIssuerSelfSigned( bool selfSigned );
    void setAccessUrl( const QString& url );

    void validateCert();
};
