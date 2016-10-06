class QgsAuthSslErrorsDialog : QDialog
{
%TypeHeaderCode
#include <qgsauthsslerrorsdialog.h>
%End
  public:
    QgsAuthSslErrorsDialog( QNetworkReply *reply,
                            const QList<QSslError>& sslErrors,
                            QWidget *parent /TransferThis/ = 0 ,
                            const QString &digest = QString(),
                            const QString &hostport = QString() );
    ~QgsAuthSslErrorsDialog();

};
