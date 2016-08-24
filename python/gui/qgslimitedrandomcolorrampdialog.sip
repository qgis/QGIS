class QgsLimitedRandomColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgslimitedrandomcolorrampdialog.h>
%End

  public:
    QgsLimitedRandomColorRampDialog( const QgsLimitedRandomColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );

    QgsLimitedRandomColorRamp ramp() const;

  public slots:
    void setCount( int val );
    void setHue1( int val );
    void setHue2( int val );
    void setSat1( int val );
    void setSat2( int val );
    void setVal1( int val );
    void setVal2( int val );

  protected:

    void updatePreview();
};
