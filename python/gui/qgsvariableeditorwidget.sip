/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsvariableeditorwidget.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsVariableEditorWidget : QWidget
{
%Docstring
A tree based widget for editing expression context scope variables. The widget allows editing
variables from a QgsExpressionContextScope, and can optionally also show inherited
variables from a QgsExpressionContext.

.. versionadded:: 2.12
%End

%TypeHeaderCode
#include "qgsvariableeditorwidget.h"
%End
  public:

    QgsVariableEditorWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsVariableEditorWidget.

:param parent: parent widget
%End

    ~QgsVariableEditorWidget();

    void setContext( QgsExpressionContext *context );
%Docstring
Overwrites the QgsExpressionContext for the widget. Setting a context
allows the widget to show all inherited variables for the context,
and highlight any overridden variables within scopes.

:param context: expression context

.. seealso:: :py:func:`context`
%End

    QgsExpressionContext *context() const;
%Docstring
Returns the current expression context for the widget. QgsVariableEditorWidget widgets
are created with an empty context by default.

.. seealso:: :py:func:`setContext`
%End

    void setEditableScopeIndex( int scopeIndex );
%Docstring
Sets the editable scope for the widget. Only variables from the editable scope can
be modified by users.

:param scopeIndex: index of current editable scope. Set to -1 to disable
editing and make the widget read-only.

.. seealso:: :py:func:`editableScope`
%End

    QgsExpressionContextScope *editableScope() const;
%Docstring
Returns the current editable scope for the widget.

:return: editable scope, or 0 if no editable scope is set

.. seealso:: :py:func:`setEditableScopeIndex`
%End

    void setSettingGroup( const QString &group );
%Docstring
Sets the setting group for the widget. QgsVariableEditorWidget widgets with
the same setting group will synchronise their settings, e.g., the size
of columns in the tree widget.

:param group: setting group

.. seealso:: :py:func:`settingGroup`
%End

    QString settingGroup() const;
%Docstring
Returns the setting group for the widget. QgsVariableEditorWidget widgets with
the same setting group will synchronise their settings, e.g., the size
of columns in the tree widget.

:return: setting group name

.. seealso:: :py:func:`setSettingGroup`
%End

    QVariantMap variablesInActiveScope() const;
%Docstring
Returns a map variables set within the editable scope. Read only variables are not
returned. This method can be used to retrieve the variables edited an added by
users via the widget.
%End

  public slots:

    void reloadContext();
%Docstring
Reloads all scopes from the editor's current context. This method should be called
after adding or removing scopes from the attached context.

.. seealso:: :py:func:`context`
%End

  signals:

    void scopeChanged();
%Docstring
Emitted when the user has modified a scope using the widget.
%End

  protected:

    virtual void showEvent( QShowEvent *event );


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsvariableeditorwidget.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
