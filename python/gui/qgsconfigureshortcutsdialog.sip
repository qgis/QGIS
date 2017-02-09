/** \ingroup gui
 * \class QgsConfigureShortcutsDialog
 * Reusable dialog for allowing users to configure shortcuts contained in a QgsShortcutsManager.
 * \note added in QGIS 2.16
 */

class QgsConfigureShortcutsDialog: QDialog
{
%TypeHeaderCode
#include <qgsconfigureshortcutsdialog.h>
%End
  public:

    /** Constructor for QgsConfigureShortcutsDialog.
     * @param parent parent widget
     * @param manager associated QgsShortcutsManager, or leave as null to use the default
     * singleton QgsShortcutsManager instance.
     */
    QgsConfigureShortcutsDialog( QWidget* parent /TransferThis/ = nullptr, QgsShortcutsManager* manager = nullptr );

    ~QgsConfigureShortcutsDialog();

  protected:
    void keyPressEvent( QKeyEvent * event );
    void keyReleaseEvent( QKeyEvent * event );

};
