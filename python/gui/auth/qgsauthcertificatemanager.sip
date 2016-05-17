class QgsAuthCertEditors : QWidget
{
%TypeHeaderCode
#include <qgsauthcertificatemanager.h>
%End

  public:
    explicit QgsAuthCertEditors( QWidget *parent /TransferThis/ = 0 );

    ~QgsAuthCertEditors();
};

class QgsAuthCertManager : QDialog
{
%TypeHeaderCode
#include <qgsauthcertificatemanager.h>
%End

  public:
    explicit QgsAuthCertManager( QWidget *parent /TransferThis/ = 0 );

    ~QgsAuthCertManager();

    QgsAuthCertEditors *certEditorsWidget();
};
