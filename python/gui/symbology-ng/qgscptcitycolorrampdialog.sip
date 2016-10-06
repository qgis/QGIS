/** \ingroup gui
 * \class QgsCptCityColorRampDialog
 * A dialog which allows users to modify the properties of a QgsCptCityColorRamp.
 * \note added in QGIS 3.0
 */
class QgsCptCityColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgscptcitycolorrampdialog.h>
%End

  public:

    /** Constructor for QgsCptCityColorRampDialog.
     * @param ramp initial ramp to show in dialog
     * @param parent parent widget
     */
    QgsCptCityColorRampDialog( const QgsCptCityColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );
    ~QgsCptCityColorRampDialog();

    /** Returns a color ramp representing the current settings from the dialog.
     * @see setRamp()
     */
    QgsCptCityColorRamp ramp() const;

    /** Sets the color ramp to show in the dialog.
     * @param ramp color ramp
     * @see ramp()
     */
    void setRamp( const QgsCptCityColorRamp& ramp );

    /** Returns the name of the ramp currently selected in the dialog.
     */
    QString selectedName() const;

    /** Returns true if the ramp should be converted to a QgsGradientColorRamp.
     */
    bool saveAsGradientRamp() const;

    bool eventFilter( QObject *obj, QEvent *event );

  signals:

    //! Emitted when the dialog settings change
    void changed();
};
