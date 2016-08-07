class QgsVectorColorBrewerColorRampDialog : QDialog
{
%TypeHeaderCode
#include <qgsvectorcolorbrewercolorrampdialog.h>
%End

  public:
    QgsVectorColorBrewerColorRampDialog( QgsVectorColorBrewerColorRamp* ramp, QWidget* parent /TransferThis/ = NULL );

  public slots:
    void setSchemeName();
    void setColors();

    void populateVariants();

  protected:

    void updatePreview();
};
