class QgsColorBrewerColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgscolorbrewercolorrampdialog.h>
%End

  public:
    QgsColorBrewerColorRampDialog( const QgsColorBrewerColorRamp& ramp, QWidget* parent /TransferThis/ = nullptr );

    QgsColorBrewerColorRamp ramp() const;

  public slots:
    void setSchemeName();
    void setColors();

    void populateVariants();

  protected:

    void updatePreview();
};
