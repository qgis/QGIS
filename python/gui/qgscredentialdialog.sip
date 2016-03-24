/** \ingroup gui
 * A generic dialog for requesting credentials
 */
class QgsCredentialDialog : QDialog, QgsCredentials
{
%TypeHeaderCode
#include <qgscredentialdialog.h>
%End

  public:
    QgsCredentialDialog( QWidget *parent /TransferThis/ = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    ~QgsCredentialDialog();

  protected:
    virtual bool request( const QString& realm, QString &username /In,Out/, QString &password /In,Out/, const QString& message = QString::null );

    virtual bool requestMasterPassword( QString &password /In,Out/, bool stored = false );
};
