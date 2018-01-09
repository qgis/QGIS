/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorbrewercolorrampdialog.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsColorBrewerColorRampWidget : QgsPanelWidget
{
%Docstring
A widget which allows users to modify the properties of a QgsColorBrewerColorRamp.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorbrewercolorrampdialog.h"
%End
  public:

    QgsColorBrewerColorRampWidget( const QgsColorBrewerColorRamp &ramp, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsColorBrewerColorRampWidget.

:param ramp: initial ramp to show in dialog
:param parent: parent widget
%End

    QgsColorBrewerColorRamp ramp() const;
%Docstring
Returns a color ramp representing the current settings from the dialog.

.. seealso:: :py:func:`setRamp`
%End

    void setRamp( const QgsColorBrewerColorRamp &ramp );
%Docstring
Sets the color ramp to show in the dialog.

:param ramp: color ramp

.. seealso:: :py:func:`ramp`
%End

  signals:

    void changed();
%Docstring
Emitted when the dialog settings change
%End

};

class QgsColorBrewerColorRampDialog : QDialog
{
%Docstring
A dialog which allows users to modify the properties of a :py:class:`QgsColorBrewerColorRamp`.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscolorbrewercolorrampdialog.h"
%End
  public:

    QgsColorBrewerColorRampDialog( const QgsColorBrewerColorRamp &ramp, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsColorBrewerColorRampDialog.

:param ramp: initial ramp to show in dialog
:param parent: parent widget
%End

    QgsColorBrewerColorRamp ramp() const;
%Docstring
Returns a color ramp representing the current settings from the dialog.

.. seealso:: :py:func:`setRamp`
%End

    void setRamp( const QgsColorBrewerColorRamp &ramp );
%Docstring
Sets the color ramp to show in the dialog.

:param ramp: color ramp

.. seealso:: :py:func:`ramp`
%End

  signals:

    void changed();
%Docstring
Emitted when the dialog settings change
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorbrewercolorrampdialog.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
