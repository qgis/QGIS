/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbolselectordialog.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/












class QgsSymbolSelectorWidget: QgsPanelWidget
{
%Docstring
Symbol selector widget that can be used to select and build a symbol
%End

%TypeHeaderCode
#include "qgssymbolselectordialog.h"
%End
  public:

    QgsSymbolSelectorWidget( QgsSymbol *symbol, QgsStyle *style, QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0 );
%Docstring
Symbol selector widget that can be used to select and build a symbol

:param symbol: The symbol to load into the widget as a start point.
:param style: The style used by the widget.
:param vl: The vector layer for the symbol.
:param parent:
%End

    QMenu *advancedMenu();
%Docstring
return menu for "advanced" button - create it if doesn't exist and show the advanced button
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

    QgsSymbol *symbol();
%Docstring
 Return the symbol that is currently active in the widget. Can be null.

:return: The active symbol.
%End

  protected:

    void loadSymbol();
%Docstring
Reload the current symbol in the view.
%End


    void updateUi();
%Docstring
Update the state of the UI based on the currently set symbol layer.
%End

    void updateLockButton();
%Docstring
Update the lock button states based on the current symbol layer.
%End


    QgsSymbolLayer *currentLayer();
%Docstring
The current symbol layer that is active in the interface.

:return: The active symbol layer.
%End

    void moveLayerByOffset( int offset );
%Docstring
Move the current active layer by a set offset in the list.

:param offset: The offset to move the layer by
%End

    void setWidget( QWidget *widget );
%Docstring
Set the properties widget for the active symbol layer.

:param widget: The widget to set to configure the active symbol layer.
%End

  signals:

    void symbolModified();
%Docstring
Emiited when a symbol is modified in the widget.
%End

  public slots:

    void moveLayerDown();
%Docstring
Move the active symbol layer down.
%End

    void moveLayerUp();
%Docstring
Move the active symbol layer up.
%End

    void addLayer();
%Docstring
Add a symbol layer to the bottom of the stack.
%End

    void removeLayer();
%Docstring
Remove the current active symbol layer.
%End

    void lockLayer();
%Docstring
Lock the current active symbol layer.
%End

    void duplicateLayer();
%Docstring
Duplicates the current symbol layer and places the duplicated layer above the current symbol layer

.. versionadded:: 2.14
%End

    void layerChanged();
%Docstring
Called when the layer changes in the widget. Updates the active properties for
active symbol layer.
%End

    void updateLayerPreview();
%Docstring
Update the single symbol layer preview in the widget.
%End

    void updatePreview();
%Docstring
Update the preview of the whole symbol in the interface.
%End

    void symbolChanged();
%Docstring
Slot to update tree when a new symbol from style
%End

    void changeLayer( QgsSymbolLayer *layer );
%Docstring
alters tree and sets proper widget when Layer Type is changed
\note: The layer is received from the LayerPropertiesWidget
%End


  protected: // data


};

class QgsSymbolSelectorDialog : QDialog
{

%TypeHeaderCode
#include "qgssymbolselectordialog.h"
%End
  public:

    QgsSymbolSelectorDialog( QgsSymbol *symbol, QgsStyle *style, QgsVectorLayer *vl, QWidget *parent /TransferThis/ = 0, bool embedded = false );
%Docstring
Constructor for QgsSymbolSelectorDialog.

:param symbol: The symbol
:param style: The style
:param vl: Associated vector layer
:param parent: Parent widget
:param embedded: True to embed in renderer properties dialog, false otherwise
%End
    ~QgsSymbolSelectorDialog();

    QMenu *advancedMenu();
%Docstring
return menu for "advanced" button - create it if doesn't exist and show the advanced button
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

    QgsSymbol *symbol();
%Docstring
 Return the symbol that is currently active in the widget. Can be null.

:return: The active symbol.
%End

  protected:
    virtual void keyPressEvent( QKeyEvent *e );


    void loadSymbol();


    void updateUi();

    void updateLockButton();

    QgsSymbolLayer *currentLayer();

    void moveLayerByOffset( int offset );

    void setWidget( QWidget *widget );

  signals:
    void symbolModified();

  public slots:
    void moveLayerDown();
    void moveLayerUp();

    void addLayer();
    void removeLayer();

    void lockLayer();

    void duplicateLayer();
%Docstring
Duplicates the current symbol layer and places the duplicated layer above the current symbol layer

.. versionadded:: 2.14
%End

    void layerChanged();

    void updateLayerPreview();
    void updatePreview();

    void symbolChanged();
%Docstring
Slot to update tree when a new symbol from style
%End

    void changeLayer( QgsSymbolLayer *layer );
%Docstring
alters tree and sets proper widget when Layer Type is changed
\note: The layer is received from the LayerPropertiesWidget
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbolselectordialog.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
