class QgsAuthImportCertDialog : QDialog
{
%TypeHeaderCode
#include <qgsauthimportcertdialog.h>
%End

  public:
    enum CertFilter
    {
      NoFilter = 1,
      CaFilter = 2,
    };

    enum CertInput
    {
      AllInputs = 1,
      FileInput = 2,
      TextInput = 3,
    };

    explicit QgsAuthImportCertDialog( QWidget *parent /TransferThis/ = 0,
                                      QgsAuthImportCertDialog::CertFilter filter = NoFilter,
                                      QgsAuthImportCertDialog::CertInput input = AllInputs );
    ~QgsAuthImportCertDialog();

    const QList<QSslCertificate> certificatesToImport();

    const QString certFileToImport();

    const QString certTextToImport();

    bool allowInvalidCerts();

    QgsAuthCertUtils::CertTrustPolicy certTrustPolicy();
};
