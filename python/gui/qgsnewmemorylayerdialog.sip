class QgsNewMemoryLayerDialog : QDialog
{
%TypeHeaderCode
#include <qgsnewmemorylayerdialog.h>
%End

  public:

    /** Runs the dialoag and creates a new memory layer
     * @param parent parent widget
     * @returns new memory layer
     */
    static QgsVectorLayer* runAndCreateLayer( QWidget* parent = 0 );

    QgsNewMemoryLayerDialog( QWidget *parent /TransferThis/ = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    ~QgsNewMemoryLayerDialog();

    /** Returns the selected geometry type*/
    QGis::WkbType selectedType() const;

    /** Returns the selected crs*/
    QgsCoordinateReferenceSystem crs() const;

    /** Returns the layer name*/
    QString layerName() const;

};
