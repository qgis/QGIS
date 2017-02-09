/** \ingroup gui
 * \class QgsColorBrewerColorRampDialog
 * A widget which allows users to modify the properties of a QgsColorBrewerColorRamp.
 * \note added in QGIS 3.0
 */

class QgsColorBrewerColorRampWidget : QgsPanelWidget
{
%TypeHeaderCode
#include <qgscolorbrewercolorrampdialog.h>
%End

  public:

    /** Constructor for QgsColorBrewerColorRampDialog.
     * @param ramp initial ramp to show in dialog
     * @param parent parent widget
     */
    QgsColorBrewerColorRampWidget( const QgsColorBrewerColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );

    /** Returns a color ramp representing the current settings from the dialog.
     * @see setRamp()
     */
    QgsColorBrewerColorRamp ramp() const;

    /** Sets the color ramp to show in the dialog.
     * @param ramp color ramp
     * @see ramp()
     */
    void setRamp( const QgsColorBrewerColorRamp& ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();
};

/** \ingroup gui
 * \class QgsColorBrewerColorRampDialog
 * A dialog which allows users to modify the properties of a QgsColorBrewerColorRamp.
 * \note added in QGIS 3.0
 */

class QgsColorBrewerColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgscolorbrewercolorrampdialog.h>
%End

  public:

    /** Constructor for QgsColorBrewerColorRampDialog.
     * @param ramp initial ramp to show in dialog
     * @param parent parent widget
     */
    QgsColorBrewerColorRampDialog( const QgsColorBrewerColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );

    /** Returns a color ramp representing the current settings from the dialog.
     * @see setRamp()
     */
    QgsColorBrewerColorRamp ramp() const;

    /** Sets the color ramp to show in the dialog.
     * @param ramp color ramp
     * @see ramp()
     */
    void setRamp( const QgsColorBrewerColorRamp& ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();
};
