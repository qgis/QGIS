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
.. seealso:: setUnit()
 :rtype: QgsUnitTypes.LayoutUnit
%End

    void setUnit( QgsUnitTypes::LayoutUnit unit );
%Docstring
 Sets the ``unit`` currently selected in the combo box.
.. seealso:: unit()
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
