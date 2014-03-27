
/** \ingroup gui
 * \class QgsBusyIndicatorDialog
 * A simple dialog to show an indeterminate busy progress indicator.
 */
class QgsBusyIndicatorDialog : QDialog
{
%TypeHeaderCode
#include <qgsbusyindicatordialog.h>
%End
  public:
    /** Constructor
     * Modal busy indicator dialog with no buttons.
     * @param message Text to show above busy progress indicator.
     * @param parent parent object (owner)
     * @param fl widget flags
     * @note added in 1.9
    */
    QgsBusyIndicatorDialog( const QString& message = "", QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsBusyIndicatorDialog();

    QString message() const;
    void setMessage( const QString& message );
};
