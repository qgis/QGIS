/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewvectorlayerdialog.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsNewVectorLayerDialog: QDialog
{

%TypeHeaderCode
#include "qgsnewvectorlayerdialog.h"
%End
  public:

    static QString runAndCreateLayer( QWidget *parent = 0, QString *enc = 0, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );
%Docstring
Runs the dialog and creates a layer matching the dialog parameters.

:return: fileName on success, empty string use aborted, QString() if creation failed
%End

    QgsNewVectorLayerDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    ~QgsNewVectorLayerDialog();
    QgsWkbTypes::Type selectedType() const;
%Docstring
Returns the selected geometry type
%End
    void attributes( QList< QPair<QString, QString> > &at ) const;
%Docstring
Appends the chosen attribute names and types to at
%End
    QString selectedFileFormat() const;
%Docstring
Returns the file format for storage
%End
    QString selectedFileEncoding() const;
%Docstring
Returns the file format for storage
%End
    QString filename() const;
%Docstring
Returns the name for the new layer
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns the selected CRS for the new layer.

.. seealso:: :py:func:`setCrs`
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the ``crs`` value for the new layer in the dialog.

.. versionadded:: 3.0

.. seealso:: :py:func:`crs`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewvectorlayerdialog.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
