/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutunitscombobox.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutUnitsComboBox : QComboBox
{
%Docstring
A custom combo box for selecting units for layout settings.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutunitscombobox.h"
%End
  public:

    QgsLayoutUnitsComboBox( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsLayoutUnitsComboBox.
%End

    QgsUnitTypes::LayoutUnit unit() const;
%Docstring
Returns the unit currently selected in the combo box.

.. seealso:: :py:func:`setUnit`
%End

    void setUnit( QgsUnitTypes::LayoutUnit unit );
%Docstring
Sets the ``unit`` currently selected in the combo box.

.. seealso:: :py:func:`unit`
%End

    void linkToWidget( QDoubleSpinBox *widget );
%Docstring
Registers a spin box ``widget`` as linked with the combo box.

Registered spin boxes will automatically be upodated whenever the unit is changed. I.e. a
spin box with a value of 100 will be set to 1 when the unit is changed from centimeters to meters.

A measurement converter() must be set in order for the automatic unit conversion to occur.

.. seealso:: :py:func:`setConverter`
%End

    QgsLayoutMeasurementConverter *converter() const;
%Docstring
Returns the converter used when automatically converting units for linked widgets.

.. seealso:: :py:func:`setConverter`
%End

    void setConverter( QgsLayoutMeasurementConverter *converter );
%Docstring
Sets a ``converter`` to use when automatically converting units for linked widgets.
The ownership of ``converter`` is not transferred, and converter must exist for the
life of the combo box.

.. seealso:: :py:func:`converter`
%End

  signals:

    void changed( QgsUnitTypes::LayoutUnit unit );
%Docstring
Emitted when the ``unit`` is changed.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutunitscombobox.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
