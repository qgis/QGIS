class QgsGradientColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgsgradientcolorrampdialog.h>
%End

  public:
    QgsGradientColorRampDialog( const QgsGradientColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );
    ~QgsGradientColorRampDialog();

    QgsGradientColorRamp ramp() const;

  public slots:
    void setColor1( const QColor& color );
    void setColor2( const QColor& color );

  protected slots:
    void on_cboType_currentIndexChanged( int index );
    void on_btnInformation_pressed();

  protected:
};
