/** \ingroup gui
 * \class QgsLimitedRandomColorRampWidget
 * A widget which allows users to modify the properties of a QgsLimitedRandomColorRamp.
 * \note added in QGIS 3.0
 */

class QgsLimitedRandomColorRampWidget : QgsPanelWidget
{
%TypeHeaderCode
#include <qgslimitedrandomcolorrampdialog.h>
%End

  public:

    /** Constructor for QgsLimitedRandomColorRampWidget.
     * @param ramp initial ramp to show in dialog
     * @param parent parent widget
     */
    QgsLimitedRandomColorRampWidget( const QgsLimitedRandomColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );

    /** Returns a color ramp representing the current settings from the dialog.
     * @see setRamp()
     */
    QgsLimitedRandomColorRamp ramp() const;

    /** Sets the color ramp to show in the dialog.
     * @param ramp color ramp
     * @see ramp()
     */
    void setRamp( const QgsLimitedRandomColorRamp& ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();

  public slots:

    //! Sets the number of colors to create in the ramp
    void setCount( int val );
    //! Sets the minimum hue for colors in the ramp
    void setHue1( int val );
    //! Sets the maximum hue for colors in the ramp
    void setHue2( int val );
    //! Sets the minimum saturation for colors in the ramp
    void setSat1( int val );
    //! Sets the maximum saturation for colors in the ramp
    void setSat2( int val );
    //! Sets the minimum value for colors in the ramp
    void setVal1( int val );
    //! Sets the maximum value for colors in the ramp
    void setVal2( int val );
};

/** \ingroup gui
 * \class QgsLimitedRandomColorRampDialog
 * A dialog which allows users to modify the properties of a QgsLimitedRandomColorRamp.
 * \note added in QGIS 3.0
 */

class QgsLimitedRandomColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgslimitedrandomcolorrampdialog.h>
%End

  public:

    /** Constructor for QgsLimitedRandomColorRampDialog.
     * @param ramp initial ramp to show in dialog
     * @param parent parent widget
     */
    QgsLimitedRandomColorRampDialog( const QgsLimitedRandomColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );

    /** Returns a color ramp representing the current settings from the dialog.
     * @see setRamp()
     */
    QgsLimitedRandomColorRamp ramp() const;

    /** Sets the color ramp to show in the dialog.
     * @param ramp color ramp
     * @see ramp()
     */
    void setRamp( const QgsLimitedRandomColorRamp& ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();

};
