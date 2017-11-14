/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewmemorylayerdialog.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsNewMemoryLayerDialog: QDialog
{

%TypeHeaderCode
#include "qgsnewmemorylayerdialog.h"
%End
  public:

    static QgsVectorLayer *runAndCreateLayer( QWidget *parent = 0, const QgsCoordinateReferenceSystem &defaultCrs = QgsCoordinateReferenceSystem() );
%Docstring
 Runs the dialog and creates a new memory layer
 \param parent parent widget
 \param defaultCrs default layer CRS to show in dialog
 :return: new memory layer
 :rtype: QgsVectorLayer
%End

    QgsNewMemoryLayerDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    ~QgsNewMemoryLayerDialog();

    QgsWkbTypes::Type selectedType() const;
%Docstring
Returns the selected geometry type
 :rtype: QgsWkbTypes.Type
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the ``crs`` value for the new layer in the dialog.
.. versionadded:: 3.0
.. seealso:: crs()
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
 Returns the selected CRS for the new layer.
.. seealso:: setCrs()
 :rtype: QgsCoordinateReferenceSystem
%End

    QString layerName() const;
%Docstring
Returns the layer name
 :rtype: str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewmemorylayerdialog.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
