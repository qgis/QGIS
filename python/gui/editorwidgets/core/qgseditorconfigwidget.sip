/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/core/qgseditorconfigwidget.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsEditorConfigWidget : QWidget, QgsExpressionContextGenerator
{
%Docstring
This class should be subclassed for every configurable editor widget type.

It implements the GUI configuration widget and transforms this to/from a configuration.

It will only be instantiated by {:py:class:`QgsEditorWidgetFactory`}
%End

%TypeHeaderCode
#include "qgseditorconfigwidget.h"
%End
  public:

    explicit QgsEditorConfigWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent /TransferThis/ );
%Docstring
Create a new configuration widget

:param vl:       The layer for which the configuration dialog will be created
:param fieldIdx: The index of the field on the layer for which this dialog will be created
:param parent:   A parent widget
%End

    virtual QVariantMap config() = 0;
%Docstring
 Create a configuration from the current GUI state

:return: A widget configuration
%End

    virtual void setConfig( const QVariantMap &config ) = 0;
%Docstring
 Update the configuration widget to represent the given configuration.

:param config: The configuration which should be represented by this widget
%End

    int field();
%Docstring
Returns the field for which this configuration widget applies

:return: The field index
%End

    QgsVectorLayer *layer();
%Docstring
Returns the layer for which this configuration widget applies

:return: The layer
%End

    virtual QgsExpressionContext createExpressionContext() const;


  signals:

    void changed();
%Docstring
Emitted when the configuration of the widget is changed.

.. versionadded:: 3.0
%End

  protected:

    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsWidgetWrapper::Property key );
%Docstring
Registers a property override button, setting up its initial value, connections and description.

:param button: button to register
:param key: corresponding data defined property key
%End

    void updateDataDefinedButtons();
%Docstring
Updates all property override buttons to reflect the widgets's current properties.
%End

    void updateDataDefinedButton( QgsPropertyOverrideButton *button );
%Docstring
Updates a specific property override ``button`` to reflect the widgets's current properties.
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/core/qgseditorconfigwidget.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
