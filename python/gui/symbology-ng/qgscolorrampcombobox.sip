class QgsColorRampComboBox : QComboBox
{
%TypeHeaderCode
#include <qgscolorrampcombobox.h>
%End
  public:
    QgsColorRampComboBox( QWidget *parent = 0 );

    ~QgsColorRampComboBox();

    //! initialize the combo box with color ramps from the style
    void populate( QgsStyleV2* style );

    //! add/select color ramp which was used previously by the renderer
    void setSourceColorRamp( QgsVectorColorRampV2* sourceRamp );

    //! return new instance of the current color ramp or NULL if there is no active color ramp
    QgsVectorColorRampV2* currentColorRamp();

    //! @note not available in python bindings
    // static QSize rampIconSize;

    //! @note added in 2.2
    void setShowGradientOnly( bool gradientOnly );
    //! @note added in 2.2
    bool showGradientOnly() const;

  public slots:
    void colorRampChanged( int index );
};
