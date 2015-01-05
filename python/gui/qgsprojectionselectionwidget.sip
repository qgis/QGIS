
class QgsProjectionSelectionWidget : QWidget
{
%TypeHeaderCode
#include <qgsprojectionselectionwidget.h>
%End

  public:
    explicit QgsProjectionSelectionWidget( QWidget *parent /TransferThis/ = 0 );

  signals:
    void crsChanged( QgsCoordinateReferenceSystem );

  public slots:
    void setCrs( QgsCoordinateReferenceSystem crs );
    void selectCrs();
};
