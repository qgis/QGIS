class QgsColorRampComboBox : QComboBox
{
%TypeHeaderCode
#include <qgscolorrampcombobox.h>
%End
  public:
    QgsColorRampComboBox( QWidget *parent /TransferThis/ = 0 );

    ~QgsColorRampComboBox();

    //! initialize the combo box with color ramps from the style
    void populate( QgsStyleV2* style );

    //! add/select color ramp which was used previously by the renderer
    void setSourceColorRamp( QgsVectorColorRampV2* sourceRamp );

    //! return new instance of the current color ramp or NULL if there is no active color ramp
    QgsVectorColorRampV2* currentColorRamp();

    /**Returns true if the current selection in the combo box is the option for creating
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
};
