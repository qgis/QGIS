
/** \ingroup gui
 * \class QgsColorDialog
 * A dialog for selecting a color
 */

class QgsColorDialog : QObject
{
%TypeHeaderCode
#include <qgscolordialog.h>
%End
  public:
    QgsColorDialog();
    ~QgsColorDialog();

    /** Return a color selection from a QColorDialog, with live updating of interim selections.
     * @param initialColor The initial color of the selection dialog.
     * @param updateObject The receiver object of the live updating.
     * @param updateSlot The receiver object's slot for live updating (e.g. SLOT( setValidColor( const QColor& ) ) ).
     * @param parent Parent widget. Usually 0 is best for native system color dialogs.
     * @param title The title of the QColorDialog.
     * @param options ColorDialogOptions passed to QColorDialog.
     * @return Selected color on accepted() or initialColor on rejected().
     */
    static QColor getLiveColor( const QColor& initialColor, QObject* updateObject, const char* updateSlot,
                                QWidget* parent = 0,
                                const QString& title = "",
                                QColorDialog::ColorDialogOptions options = 0 );
};
