class QgsColorBrewerColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgscolorbrewercolorrampdialog.h>
%End

  public:
    QgsColorBrewerColorRampDialog( QgsColorBrewerColorRamp* ramp, QWidget* parent /TransferThis/ = NULL );

  public slots:
    void setSchemeName();
    void setColors();

    void populateVariants();

  protected:

    void updatePreview();
};
