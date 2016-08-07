class QgsVectorGradientColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgsvectorgradientcolorrampdialog.h>
%End

  public:
    QgsVectorGradientColorRampDialog( QgsVectorGradientColorRamp* ramp, QWidget* parent /TransferThis/ = NULL );

  public slots:
    void setColor1( const QColor& color );
    void setColor2( const QColor& color );

  protected slots:
    void on_cboType_currentIndexChanged( int index );
    void on_btnInformation_pressed();

  protected:
};
