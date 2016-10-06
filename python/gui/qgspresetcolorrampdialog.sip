/** \ingroup gui
 * \class QgsPresetColorRampWidget
 * A widget which allows users to modify the properties of a QgsPresetSchemeColorRamp.
 * \note added in QGIS 3.0
 */

class QgsPresetColorRampWidget : QgsPanelWidget
{
%TypeHeaderCode
#include <qgspresetcolorrampdialog.h>
%End

  public:

/** Constructor for QgsPresetColorRampWidget.
     * @param ramp initial ramp to show in dialog
     * @param parent parent widget
     */
    QgsPresetColorRampWidget( const QgsPresetSchemeColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );

    ~QgsPresetColorRampWidget();

    /** Returns a color ramp representing the current settings from the dialog.
     * @see setRamp()
     */
    QgsPresetSchemeColorRamp ramp() const;

    /** Sets the color ramp to show in the dialog.
     * @param ramp color ramp
     * @see ramp()
     */
    void setRamp( const QgsPresetSchemeColorRamp& ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();
};

/** \ingroup gui
 * \class QgsPresetColorRampDialog
 * A dialog which allows users to modify the properties of a QgsPresetSchemeColorRamp.
 * \note added in QGIS 3.0
 */

class QgsPresetColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgspresetcolorrampdialog.h>
%End

  public:

    /** Constructor for QgsPresetColorRampDialog.
     * @param ramp initial ramp to show in dialog
     * @param parent parent widget
     */
    QgsPresetColorRampDialog( const QgsPresetSchemeColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );

    /** Returns a color ramp representing the current settings from the dialog.
     * @see setRamp()
     */
    QgsPresetSchemeColorRamp ramp() const;

    /** Sets the color ramp to show in the dialog.
     * @param ramp color ramp
     * @see ramp()
     */
    void setRamp( const QgsPresetSchemeColorRamp& ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();
};
