class QgsAuthImportIdentityDialog : QDialog
{
%TypeHeaderCode
#include <qgsauthimportidentitydialog.h>
%End

  public:
    enum IdentityType
    {
      CertIdentity = 0,
    };

    enum BundleTypes
    {
      PkiPaths = 0,
      PkiPkcs12 = 1,
    };

    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    explicit QgsAuthImportIdentityDialog( QgsAuthImportIdentityDialog::IdentityType identitytype,
                                          QWidget *parent /TransferThis/ = 0 );
    ~QgsAuthImportIdentityDialog();

    QgsAuthImportIdentityDialog::IdentityType identityType();

    // Should not be available via Python binding
    //const QPair<QSslCertificate, QSslKey> certBundleToImport();

    const QgsPkiBundle pkiBundleToImport();
};
