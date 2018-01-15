/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthtrustedcasdialog.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAuthTrustedCAsDialog : QDialog
{
%Docstring
Widget for listing trusted Certificate (Intermediate) Authorities used in secure connections
%End

%TypeHeaderCode
#include "qgsauthtrustedcasdialog.h"
%End
  public:

    explicit QgsAuthTrustedCAsDialog( QWidget *parent /TransferThis/ = 0,
                                      const QList<QSslCertificate> &trustedCAs = QList<QSslCertificate>() );
%Docstring
Construct a dialog that will list the trusted Certificate Authorities

:param parent: Parent widget
:param trustedCAs: List of trusted Certificate Authorities objects
%End

  protected:

    virtual void showEvent( QShowEvent *e );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthtrustedcasdialog.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
