class QgsAuthImportIdentityDialog : QDialog
{
%TypeHeaderCode
#include <qgsauthimportidentitydialog.h>
%End

  public:
    /** Type of identity being imported */
    enum IdentityType
    {
      CertIdentity = 0,
    };

    /** Type of bundles supported */
    enum BundleTypes
    {
      PkiPaths = 0,
      PkiPkcs12 = 1,
    };

    /** Type of certificate/bundle validity output */
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    /**
     * Construct a dialog for importing identities
     * @param identitytype Type of the identity to import
     * @param parent Parent widget
     */
    explicit QgsAuthImportIdentityDialog( QgsAuthImportIdentityDialog::IdentityType identitytype,
                                          QWidget *parent /TransferThis/ = 0 );
    ~QgsAuthImportIdentityDialog();

    /** Get identity type */
    QgsAuthImportIdentityDialog::IdentityType identityType();

    /** Get certificate/key bundle to be imported.
     * @note not available in Python bindings
     */
    // const QPair<QSslCertificate, QSslKey> certBundleToImport();

    /** Get certificate/key bundle to be imported as a PKI bundle object */
    const QgsPkiBundle pkiBundleToImport();
};
