/** \ingroup gui
 * A generic dialog for requesting credentials
 */
class QgsCredentialDialog : QDialog, QgsCredentials
{
%TypeHeaderCode
#include <qgscredentialdialog.h>
%End

  public:
    QgsCredentialDialog( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsCredentialDialog();

  protected:
    virtual bool request( QString realm, QString &username, QString &password, QString message = QString::null );
};
