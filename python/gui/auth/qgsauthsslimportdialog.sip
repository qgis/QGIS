class QgsAuthSslImportDialog : QDialog
{
%TypeHeaderCode
#include <qgsauthsslimportdialog.h>
%End
  public:
    QgsAuthSslImportDialog( QWidget *parent /TransferThis/ = 0 );
    ~QgsAuthSslImportDialog();

  public slots:
    void accept();
};
