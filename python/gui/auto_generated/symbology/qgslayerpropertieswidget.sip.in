/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgslayerpropertieswidget.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsLayerPropertiesWidget : QgsPanelWidget, QgsExpressionContextGenerator
{

%TypeHeaderCode
#include "qgslayerpropertieswidget.h"
%End
  public:

    QgsLayerPropertiesWidget( QgsSymbolLayer *layer, const QgsSymbol *symbol, QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsLayerPropertiesWidget.

:param layer: the symbol layer
:param symbol: the symbol
:param vl: associated vector layer
:param parent: parent widget
%End

    void setContext( const QgsSymbolWidgetContext &context );
%Docstring
Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.

:param context: symbol widget context

.. seealso:: :py:func:`context`

.. versionadded:: 3.0
%End

    QgsSymbolWidgetContext context() const;
%Docstring
Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.

.. seealso:: :py:func:`setContext`

.. versionadded:: 3.0
%End

    virtual void setDockMode( bool dockMode );

%Docstring
Set the widget in dock mode which tells the widget to emit panel
widgets and not open dialogs

:param dockMode: True to enable dock mode.
%End

  public slots:
    void layerTypeChanged();
    void emitSignalChanged();

  signals:
    void changed();
    void changeLayer( QgsSymbolLayer * );

  protected:
    void populateLayerTypes();
    void updateSymbolLayerWidget( QgsSymbolLayer *layer );

    virtual QgsExpressionContext createExpressionContext() const;


    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key );
%Docstring
Registers a data defined override button. Handles setting up connections
for the button and initializing the button to show the correct descriptions
and help text for the associated property.

.. versionadded:: 3.0
%End

  protected: // data


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgslayerpropertieswidget.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
