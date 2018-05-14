/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrasterbandcombobox.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRasterBandComboBox : QComboBox
{
%Docstring
A combobox widget which displays the bands present in a raster layer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsrasterbandcombobox.h"
%End
  public:

    QgsRasterBandComboBox( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsRasterBandComboBox.
%End

    QgsRasterLayer *layer() const;
%Docstring
Returns the layer currently associated with the combobox.

.. seealso:: :py:func:`setLayer`
%End

    int currentBand() const;
%Docstring
Returns the current band number selected in the combobox, or -1
if no band is selected.

.. seealso:: :py:func:`setBand`
%End

    bool isShowingNotSetOption() const;
%Docstring
Returns true if the combo box is showing the "not set" option.

.. seealso:: :py:func:`setShowNotSetOption`
%End

    void setShowNotSetOption( bool show, const QString &string = QString() );
%Docstring
Sets whether the combo box should show the "not set" option.
Optionally the built in "not set" text can be overridden by specifying
a ``string``.

.. seealso:: :py:func:`setShowNotSetOption`
%End

  public slots:

    void setLayer( QgsMapLayer *layer );
%Docstring
Sets the raster ``layer`` for which the bands are listed in the combobox. If no layer is set
or a non-raster layer is set then the combobox will be empty.

.. seealso:: :py:func:`layer`
%End

    void setBand( int band );
%Docstring
Sets the current ``band`` number selected in the combobox.

.. seealso:: :py:func:`band`
%End

  signals:

    void bandChanged( int band );
%Docstring
This signal is emitted when the currently selected band changes.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrasterbandcombobox.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
