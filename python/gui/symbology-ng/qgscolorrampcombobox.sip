class QgsColorRampComboBox : QComboBox
{
%TypeHeaderCode
#include <qgscolorrampcombobox.h>
%End
  public:
    QgsColorRampComboBox( QWidget *parent /TransferThis/ = 0 );

    ~QgsColorRampComboBox();

    //! initialize the combo box with color ramps from the style
    void populate( QgsStyle* style );

    /** Adds or selects the current color ramp to show in the combo box. The ramp appears
     * in the combo box as the "source" ramp.
     * @param sourceRamp color ramp, ownership is transferred.
     * @see currentColorRamp()
     */
    void setSourceColorRamp( QgsColorRamp* sourceRamp /Transfer/ );

    /** Returns a new instance of the current color ramp or NULL if there is no active color ramp.
     * The caller takes responsibility for deleting the returned value.
     * @see setSourceColorRamp()
     */
    QgsColorRamp* currentColorRamp() const /Factory/;

    /** Returns true if the current selection in the combo box is the option for creating
     * a new color ramp
     * @note added in QGIS 2.7
     */
    bool createNewColorRampSelected() const;

    //! @note not available in python bindings
    // static QSize rampIconSize;

    //! @note added in 2.2
    void setShowGradientOnly( bool gradientOnly );
    //! @note added in 2.2
    bool showGradientOnly() const;

  public slots:
    void colorRampChanged( int index );

    /** Triggers a dialog which allows users to edit the current source
     * ramp for the combo box.
     * @see sourceRampEdited
     * @note added in QGIS 2.12
     */
    void editSourceRamp();

  signals:

    /** Emitted when the user has edited the current source ramp.
     * @see editSourceRamp
     * @note added in QGIS 2.12
     */
    void sourceRampEdited();
};
