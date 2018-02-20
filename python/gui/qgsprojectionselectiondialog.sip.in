/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsprojectionselectiondialog.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsProjectionSelectionDialog : QDialog
{
%Docstring
A generic dialog to prompt the user for a Coordinate Reference System.

Typically you will use this when you want to prompt the user for
a coordinate system identifier e.g. from a plugin you might do this
to get an epsg code:

If you wish to embed the projection selector into an existing dialog
the you probably want to look at QgsProjectionSelectionWidget instead.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprojectionselectiondialog.h"
%End
  public:

    QgsProjectionSelectionDialog( QWidget *parent /TransferThis/ = 0,
                                  Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
Constructor for QgsProjectionSelectionDialog.
%End


    ~QgsProjectionSelectionDialog();

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns the CRS currently selected in the widget.

.. versionadded:: 3.0

.. seealso:: :py:func:`setCrs`
%End

    void setMessage( const QString &message );
%Docstring
Sets a ``message`` to show in the dialog. If an empty string is
passed, the message will be a generic
'define the CRS for this layer'.
%End

    void setShowNoProjection( bool show );
%Docstring
Sets whether a "no/invalid" projection option should be shown. If this
option is selected, calling crs() will return an invalid :py:class:`QgsCoordinateReferenceSystem`.

.. seealso:: :py:func:`showNoProjection`

.. versionadded:: 3.0
%End

    bool showNoProjection() const;
%Docstring
Returns whether the "no/invalid" projection option is shown. If this
option is selected, calling crs() will return an invalid :py:class:`QgsCoordinateReferenceSystem`.

.. versionadded:: 3.0

.. seealso:: :py:func:`setShowNoProjection`
%End

  public slots:

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the initial ``crs`` to show within the dialog.

.. versionadded:: 3.0

.. seealso:: :py:func:`crs`
%End

    void setOgcWmsCrsFilter( const QSet<QString> &crsFilter );
%Docstring
 filters this dialog by the given CRSs

Sets this dialog to filter the available projections to those listed
by the given Coordinate Reference Systems.

:param crsFilter: a list of OGC Coordinate Reference Systems to filter the
list of projections by.  This is useful in (e.g.) WMS situations
where you just want to offer what the WMS server can support.

.. warning::

   This function's behavior is undefined if it is called after the dialog is shown.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsprojectionselectiondialog.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
