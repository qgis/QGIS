class QgsVectorGradientColorRampV2Dialog : QDialog
{
%TypeHeaderCode
#include <qgsvectorgradientcolorrampv2dialog.h>
%End

  public:
    QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent /TransferThis/ = NULL );

  public slots:
    void setColor1( const QColor& color );
    void setColor2( const QColor& color );

  protected slots:
    void on_cboType_currentIndexChanged( int index );
    void on_btnInformation_pressed();

  protected:
};
