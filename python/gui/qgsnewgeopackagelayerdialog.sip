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

    enum OverwriteBehavior
    {
      Prompt,
      Overwrite,
      AddNewLayer,
    };

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
 Sets the initial database ``path``
.. versionadded:: 3.0
%End

    void lockDatabasePath();
%Docstring
 Sets the database path widgets to a locked and read-only mode.
.. versionadded:: 3.0
%End

    void setOverwriteBehavior( OverwriteBehavior behavior );
%Docstring
 Sets the ``behavior`` to use when a path to an existing geopackage file is used.

 The default behavior is to prompt the user for an action to take.

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
