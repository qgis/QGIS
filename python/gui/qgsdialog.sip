/** \ingroup gui
 * A generic dialog with layout and button box
 */
class QgsDialog : QDialog
{
%TypeHeaderCode
#include <qgsdialog.h>
%End

  public:
    QgsDialog( QWidget *parent /TransferThis/ = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags,
               const QDialogButtonBox::StandardButtons& buttons = QDialogButtonBox::Close,
               Qt::Orientation orientation = Qt::Horizontal );
    ~QgsDialog();

    //! Returns the central layout. Widgets added to it must have this dialog as parent.
    QVBoxLayout *layout();
    //! Returns the button box.
    QDialogButtonBox *buttonBox();
};
