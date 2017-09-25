/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewgeopackagelayerdialog.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsNewGeoPackageLayerDialog: QDialog
{
%Docstring
 Dialog to set up parameters to create a new GeoPackage layer, and on accept() to create it and add it to the layers *
%End

%TypeHeaderCode
#include "qgsnewgeopackagelayerdialog.h"
%End
  public:
    QgsNewGeoPackageLayerDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
Constructor
%End
    ~QgsNewGeoPackageLayerDialog();

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the ``crs`` value for the new layer in the dialog.
.. versionadded:: 3.0
%End

    QString databasePath() const;
%Docstring
 Returns the database path
.. versionadded:: 3.0
 :rtype: str
%End

    void setDatabasePath( const QString &path );
%Docstring
 Sets the the database ``path``
.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewgeopackagelayerdialog.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
