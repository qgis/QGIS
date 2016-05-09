/** Dialog to set up parameters to create a new GeoPackage layer, and on accept() to create it and add it to the layers */
class QgsNewGeoPackageLayerDialog : QDialog
{
%TypeHeaderCode
#include <qgsnewgeopackagelayerdialog.h>
%End

  public:
    /** Constructor */
    QgsNewGeoPackageLayerDialog( QWidget *parent /TransferThis/ = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );

    ~QgsNewGeoPackageLayerDialog();

};
