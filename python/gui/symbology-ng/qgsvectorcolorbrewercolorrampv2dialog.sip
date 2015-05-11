class QgsVectorColorBrewerColorRampV2Dialog : QDialog
{
%TypeHeaderCode
#include <qgsvectorcolorbrewercolorrampv2dialog.h>
%End

  public:
    QgsVectorColorBrewerColorRampV2Dialog( QgsVectorColorBrewerColorRampV2* ramp, QWidget* parent /TransferThis/ = NULL );

  public slots:
    void setSchemeName();
    void setColors();

    void populateVariants();

  protected:

    void updatePreview();
};
