/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthimportcertdialog.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAuthImportCertDialog : QDialog
{
%Docstring
Widget for importing a certificate into the authentication database
%End

%TypeHeaderCode
#include "qgsauthimportcertdialog.h"
%End
  public:
    enum CertFilter
    {
      NoFilter,
      CaFilter,
    };

    enum CertInput
    {
      AllInputs,
      FileInput,
      TextInput,
    };

    explicit QgsAuthImportCertDialog( QWidget *parent /TransferThis/ = 0,
                                      QgsAuthImportCertDialog::CertFilter filter = NoFilter,
                                      QgsAuthImportCertDialog::CertInput input = AllInputs );
%Docstring
Construct a dialog for importing certificates
:param parent: Parent widget
:param filter: Certificate type filter to apply to dialog
:param input: Type of input(s) for certificates
%End

    const QList<QSslCertificate> certificatesToImport();
%Docstring
Get list of certificate objects to import

:rtype: list of QSslCertificate
%End

    const QString certFileToImport();
%Docstring
Get the file path to a certificate to import

:rtype: str
%End

    const QString certTextToImport();
%Docstring
Get certificate text to import

:rtype: str
%End

    bool allowInvalidCerts();
%Docstring
Whether to allow importation of invalid certificates (so trust policy can be overridden)

:rtype: bool
%End

    QgsAuthCertUtils::CertTrustPolicy certTrustPolicy();
%Docstring
Defined trust policy for imported certificates

:rtype: QgsAuthCertUtils.CertTrustPolicy
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthimportcertdialog.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
