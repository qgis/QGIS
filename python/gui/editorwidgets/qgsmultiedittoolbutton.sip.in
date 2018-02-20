/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsmultiedittoolbutton.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsMultiEditToolButton : QToolButton
{
%Docstring
A tool button widget which is displayed next to editor widgets in attribute forms, and
allows for controlling how the widget behaves and interacts with the form while in multi
edit mode.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsmultiedittoolbutton.h"
%End
  public:

    enum State
    {
      Default,
      MixedValues,
      Changed,
    };

    explicit QgsMultiEditToolButton( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsMultiEditToolButton.

:param parent: parent object
%End

    State state() const;
%Docstring
Returns the current displayed state of the button.
%End

    void setField( const QgsField &field );
%Docstring
Sets the field associated with this button. This is used to customise the widget menu
and tooltips to match the field properties.

:param field: associated field
%End

  public slots:

    void setIsMixed( bool mixed );
%Docstring
Sets whether the associated field contains mixed values.

:param mixed: whether field values are mixed

.. seealso:: :py:func:`isMixed`

.. seealso:: :py:func:`setIsChanged`

.. seealso:: :py:func:`resetChanges`
%End

    void setIsChanged( bool changed );
%Docstring
Sets whether the associated field has changed.

:param changed: whether field has changed

.. seealso:: :py:func:`isChanged`

.. seealso:: :py:func:`setIsMixed`

.. seealso:: :py:func:`resetChanges`
%End

    void resetChanges();
%Docstring
Resets the changed state for the field.

.. seealso:: :py:func:`setIsMixed`

.. seealso:: :py:func:`setIsChanged`

.. seealso:: :py:func:`changesCommitted`
%End

    void changesCommitted();
%Docstring
Called when field values have been changed and field now contains all the same values.

.. seealso:: :py:func:`resetChanges`
%End

  signals:

    void setFieldValueTriggered();
%Docstring
Emitted when the "set field value for all features" option is selected.
%End

    void resetFieldValueTriggered();
%Docstring
Emitted when the "reset to original values" option is selected.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsmultiedittoolbutton.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
