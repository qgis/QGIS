class QgsVectorGradientColorRampV2Dialog : QDialog
{
%TypeHeaderCode
#include <qgsvectorgradientcolorrampv2dialog.h>
%End

  public:
    QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent = NULL );

  public slots:
    void setColor1();
    void setColor2();

    void toggledStops( bool on );
    void addStop();
    void removeStop();

    void stopDoubleClicked( QTreeWidgetItem* item, int column );

  protected:

    void updatePreview();
    void setStopColor( QTreeWidgetItem* item, QColor color );
};
