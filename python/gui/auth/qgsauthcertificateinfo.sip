class QgsAuthCertInfo : QWidget
{
%TypeHeaderCode
#include <qgsauthcertificateinfo.h>
%End
  public:
    explicit QgsAuthCertInfo( const QSslCertificate& cert,
                              bool manageCertTrust = false,
                              QWidget *parent /TransferThis/ = 0,
                              const QList<QSslCertificate>& connectionCAs = QList<QSslCertificate>() );
    ~QgsAuthCertInfo();

    bool trustCacheRebuilt();
};

//////////////// Embed in dialog ///////////////////

class QgsAuthCertInfoDialog : QDialog
{
%TypeHeaderCode
#include <qgsauthcertificateinfo.h>
%End

  public:
    explicit QgsAuthCertInfoDialog( const QSslCertificate& cert,
                                    bool manageCertTrust,
                                    QWidget *parent /TransferThis/ = 0,
                                    const QList<QSslCertificate>& connectionCAs = QList<QSslCertificate>() );
    ~QgsAuthCertInfoDialog();

    QgsAuthCertInfo *certInfoWidget();

    bool trustCacheRebuilt();
};
