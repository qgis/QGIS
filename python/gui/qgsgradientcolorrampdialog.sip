/** \ingroup gui
 * \class QgsGradientColorRampDialog
 * A dialog which allows users to modify the properties of a QgsGradientColorRamp.
 * \note added in QGIS 3.0
 */

class QgsGradientColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgsgradientcolorrampdialog.h>
%End

  public:

    /** Constructor for QgsGradientColorRampDialog.
     * @param ramp initial ramp to show in dialog
     * @param parent parent widget
     */
    QgsGradientColorRampDialog( const QgsGradientColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );
    ~QgsGradientColorRampDialog();

    /** Returns a color ramp representing the current settings from the dialog.
     * @see setRamp()
     */
    QgsGradientColorRamp ramp() const;

    /** Sets the color ramp to show in the dialog.
     * @param ramp color ramp
     * @see ramp()
     */
    void setRamp( const QgsGradientColorRamp& ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();

  public slots:

    /** Sets the start color for the gradient ramp.
     * @see setColor2()
     */
    void setColor1( const QColor& color );

    /** Sets the end color for the gradient ramp.
     * @see setColor1()
     */
    void setColor2( const QColor& color );

};
