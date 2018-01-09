/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformeditorwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAttributeFormEditorWidget : QgsAttributeFormWidget
{
%Docstring
A widget consisting of both an editor widget and additional widgets for controlling the behavior
of the editor widget depending on a number of possible modes. For instance, if the parent attribute
form is in the multi edit mode, this widget will show both the editor widget and a tool button for
controlling the multi edit results.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsattributeformeditorwidget.h"
%End
  public:

    explicit QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper *editorWidget, const QString &widgetType,
                                           QgsAttributeForm *form  /TransferThis/ );
%Docstring
Constructor for QgsAttributeFormEditorWidget.

:param editorWidget: associated editor widget wrapper (for default/edit modes)
:param widgetType: the type identifier of the widget passed in the
wrapper
:param form: parent attribute form
%End

    ~QgsAttributeFormEditorWidget();

    virtual void createSearchWidgetWrappers();


    void initialize( const QVariant &initialValue, bool mixedValues = false );
%Docstring
Resets the widget to an initial value.

:param initialValue: initial value to show in widget
:param mixedValues: set to true to initially show the mixed values state
%End

    bool hasChanged() const;
%Docstring
Returns true if the widget's value has been changed since it was initialized.

.. seealso:: :py:func:`initialize`
%End

    QVariant currentValue() const;
%Docstring
Returns the current value of the attached editor widget.
%End

    void setConstraintStatus( const QString &constraint, const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result );
%Docstring
Set the constraint status for this widget.
%End

    void setConstraintResultVisible( bool editable );
%Docstring
Set the constraint result lable visible or invisible according to the layer editable status
%End

  public slots:

    void setIsMixed( bool mixed );
%Docstring
Sets whether the widget should be displayed in a "mixed values" mode.

:param mixed: set to true to show in a mixed values state
%End

    void changesCommitted();
%Docstring
Called when field values have been committed;
%End

  signals:

    void valueChanged( const QVariant &value );
%Docstring
Emitted when the widget's value changes

:param value: new widget value
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformeditorwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
