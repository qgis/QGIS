class QgsAuthSslConfigWidget : QWidget
{
%TypeHeaderCode
#include <qgsauthsslconfigwidget.h>
%End

  public:
    explicit QgsAuthSslConfigWidget( QWidget *parent /TransferThis/ = 0,
                                     const QSslCertificate &cert = QSslCertificate(),
                                     const QString &hostport = QString(),
                                     const QList<QSslCertificate>& connectionCAs = QList<QSslCertificate>() );
    ~QgsAuthSslConfigWidget();

    QGroupBox *certificateGroupBox();
    QGroupBox *sslConfigGroupBox();

    const QgsAuthConfigSslServer sslCustomConfig();

    const QSslCertificate sslCertificate();

    const QString sslHost();

    QSsl::SslProtocol sslProtocol();

    const QList<QSslError::SslError> sslIgnoreErrorEnums();

    QSslSocket::PeerVerifyMode sslPeerVerifyMode();

    int sslPeerVerifyDepth();

  public slots:
    void enableSslCustomOptions( bool enable );

    void setSslCertificate( const QSslCertificate& cert, const QString &hostport = QString() );

    void loadSslCustomConfig( const QgsAuthConfigSslServer& config = QgsAuthConfigSslServer() );

    void saveSslCertConfig();

    void resetSslCertConfig();

    void setSslProtocol( QSsl::SslProtocol protocol );

    void resetSslProtocol();

    void appendSslIgnoreErrors( const QList<QSslError>& errors );

    void setSslIgnoreErrorEnums( const QList<QSslError::SslError>& errorenums );

    void setSslIgnoreErrors( const QList<QSslError>& errors );

    void resetSslIgnoreErrors();

    void setSslPeerVerify( QSslSocket::PeerVerifyMode mode, int modedepth );

    void resetSslPeerVerify();

    void setSslHost( const QString& host );

    void setConfigCheckable( bool checkable );

    void validateHostPortText( const QString &txt );

    bool readyToSave();

  signals:
    void configEnabledChanged( bool enabled );
    void certFoundInAuthDatabase( bool found );
    void hostPortValidityChanged( bool valid );
    void readyToSaveChanged( bool cansave );
};

//////////////// Embed in dialog ///////////////////

class QgsAuthSslConfigDialog : QDialog
{
%TypeHeaderCode
#include <qgsauthsslconfigwidget.h>
%End

  public:
    explicit QgsAuthSslConfigDialog( QWidget *parent = 0,
                                     const QSslCertificate& cert = QSslCertificate(),
                                     const QString &hostport = QString() );
    ~QgsAuthSslConfigDialog();

    QgsAuthSslConfigWidget *sslCustomConfigWidget();

  public slots:
      void accept();
};
