/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsunitselectionwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMapUnitScaleWidget : QgsPanelWidget
{
%Docstring
A widget which allows the user to choose the minimum and maximum scale of an object in map units
and millimeters. This widget is designed to allow users to edit the properties of a
QgsMapUnitScale object.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsMapUnitScaleDialog`

.. seealso:: :py:class:`QgsUnitSelectionWidget`
%End

%TypeHeaderCode
#include "qgsunitselectionwidget.h"
%End
  public:

    QgsMapUnitScaleWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsMapUnitScaleWidget.

:param parent: parent widget
%End

    QgsMapUnitScale mapUnitScale() const;
%Docstring
Returns a QgsMapUnitScale representing the settings shown in the
widget.

.. seealso:: :py:func:`setMapUnitScale`

.. seealso:: :py:func:`mapUnitScaleChanged`
%End

    void setMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Updates the widget to reflect the settings from the specified
QgsMapUnitScale object.

:param scale: map unit scale to show in widget

.. seealso:: :py:func:`mapUnitScale`

.. seealso:: :py:func:`mapUnitScaleChanged`
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets the map canvas associated with the widget. This allows the
widget to retrieve the current map scale from the canvas.

:param canvas: map canvas
%End

  signals:

    void mapUnitScaleChanged( const QgsMapUnitScale &scale );
%Docstring
Emitted when the settings in the widget are modified.

:param scale: QgsMapUnitScale reflecting new settings from the widget
%End

};

class QgsMapUnitScaleDialog : QDialog
{
%Docstring
A dialog which allows the user to choose the minimum and maximum scale of an object in map units
and millimeters. This dialog is designed to allow users to edit the properties of a
QgsMapUnitScale object.

.. seealso:: :py:class:`QgsMapUnitScaleWidget`

.. seealso:: :py:class:`QgsUnitSelectionWidget`
%End

%TypeHeaderCode
#include "qgsunitselectionwidget.h"
%End
  public:

    QgsMapUnitScaleDialog( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsMapUnitScaleDialog.

:param parent: parent widget
%End

    QgsMapUnitScale getMapUnitScale() const;
%Docstring
Returns a QgsMapUnitScale representing the settings shown in the
dialog.

.. seealso:: :py:func:`setMapUnitScale`
%End

    void setMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Updates the dialog to reflect the settings from the specified
QgsMapUnitScale object.

:param scale: map unit scale to show in dialog

.. seealso:: :py:func:`mapUnitScale`
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets the map canvas associated with the dialog. This allows the dialog to retrieve the current
map scale from the canvas.

:param canvas: map canvas

.. versionadded:: 2.12
%End

};

class QgsUnitSelectionWidget : QWidget
{
%Docstring
A widget displaying a combobox allowing the user to choose between various display units,
such as millimeters or map unit. If the user chooses map units, a button appears allowing
adjustment of minimum and maximum scaling.

.. seealso:: :py:class:`QgsMapUnitScaleWidget`

.. seealso:: :py:class:`QgsMapUnitScaleDialog`
%End

%TypeHeaderCode
#include "qgsunitselectionwidget.h"
%End
  public:

    QgsUnitSelectionWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsUnitSelectionWidget.

:param parent: parent widget
%End

    void setUnits( const QStringList &units, int mapUnitIdx );
%Docstring
Sets the units which the user can choose from in the combobox.

:param units: list of strings for custom units to display in the widget
:param mapUnitIdx: specifies which entry corresponds to the map units, or -1 if none
%End

    void setUnits( const QgsUnitTypes::RenderUnitList &units );
%Docstring
Sets the units which the user can choose from in the combobox. Clears any existing units.

:param units: list of valid units

.. versionadded:: 2.9
%End

    int getUnit() const;
%Docstring
Get the selected unit index
%End

    QgsUnitTypes::RenderUnit unit() const;
%Docstring
Returns the current predefined selected unit (if applicable).

:return: selected output unit, or QgsUnitTypes.RenderUnknownUnit if the widget was populated with custom unit types

.. versionadded:: 2.9
%End

    void setUnit( int unitIndex ) /PyName=setUnitIndex/;
%Docstring
Sets the selected unit index

:param unitIndex: index of unit to set as current

.. note::

   available in Python bindings as setUnitIndex
%End

    void setUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the selected unit

:param unit: predefined unit to set as current
%End

    QgsMapUnitScale getMapUnitScale() const;
%Docstring
Returns the map unit scale
%End

    void setMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets the map canvas associated with the widget. This allows the widget to retrieve the current
map scale from the canvas.

:param canvas: map canvas

.. versionadded:: 2.12
%End

  signals:
    void changed();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsunitselectionwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
