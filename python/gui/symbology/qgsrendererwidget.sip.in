/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsrendererwidget.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsRendererWidget : QgsPanelWidget
{
%Docstring
Base class for renderer settings widgets

WORKFLOW:
- open renderer dialog with some RENDERER  (never null!)
- find out which widget to use
- instantiate it and set in stacked widget
- on any change of renderer type, create some default (dummy?) version and change the stacked widget
- when clicked OK/Apply, get the renderer from active widget and clone it for the layer
%End

%TypeHeaderCode
#include "qgsrendererwidget.h"
%End
  public:
    QgsRendererWidget( QgsVectorLayer *layer, QgsStyle *style );

    virtual QgsFeatureRenderer *renderer() = 0;
%Docstring
return pointer to the renderer (no transfer of ownership)
%End

    void showSymbolLevelsDialog( QgsFeatureRenderer *r );
%Docstring
show a dialog with renderer's symbol level settings
%End

    virtual void setContext( const QgsSymbolWidgetContext &context );
%Docstring
Sets the context in which the renderer widget is shown, e.g., the associated map canvas and expression contexts.

:param context: symbol widget context

.. seealso:: :py:func:`context`

.. versionadded:: 3.0
%End

    QgsSymbolWidgetContext context() const;
%Docstring
Returns the context in which the renderer widget is shown, e.g., the associated map canvas and expression contexts.

.. seealso:: :py:func:`setContext`

.. versionadded:: 3.0
%End

    const QgsVectorLayer *vectorLayer() const;
%Docstring
Returns the vector layer associated with the widget.

.. versionadded:: 2.12
%End

    void applyChanges();
%Docstring
This method should be called whenever the renderer is actually set on the layer.
%End

  signals:

    void layerVariablesChanged();
%Docstring
Emitted when expression context variables on the associated
vector layers have been changed. Will request the parent dialog
to re-synchronize with the variables.
%End

  protected:


    virtual QList<QgsSymbol *> selectedSymbols();
%Docstring
Subclasses may provide the capability of changing multiple symbols at once by implementing the following two methods
and by connecting the slot contextMenuViewCategories(const QPoint&)*
%End
    virtual void refreshSymbolView();

    QgsDataDefinedSizeLegendWidget *createDataDefinedSizeLegendWidget( const QgsMarkerSymbol *symbol, const QgsDataDefinedSizeLegend *ddsLegend ) /Factory/;
%Docstring
Creates widget to setup data-defined size legend.
Returns newly created panel - may be null if it could not be opened. Ownership is transferred to the caller.

.. versionadded:: 3.0
%End

  protected slots:
    void  contextMenuViewCategories( QPoint p );
    void changeSymbolColor();
%Docstring
Change color of selected symbols
%End
    void changeSymbolOpacity();
%Docstring
Change opacity of selected symbols
%End
    void changeSymbolUnit();
%Docstring
Change units mm/map units of selected symbols
%End
    void changeSymbolWidth();
%Docstring
Change line widths of selected symbols
%End
    void changeSymbolSize();
%Docstring
Change marker sizes of selected symbols
%End
    void changeSymbolAngle();
%Docstring
Change marker angles of selected symbols
%End

    virtual void copy();
    virtual void paste();

  private:
    virtual void apply() ;
%Docstring
This will be called whenever the renderer is set on a layer.
This can be overwritten in subclasses.
%End
};







class QgsDataDefinedValueDialog : QDialog
{
%Docstring
Utility classes for "en masse" size definition
%End

%TypeHeaderCode
#include "qgsrendererwidget.h"
%End
  public:

    QgsDataDefinedValueDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer, const QString &label );
%Docstring
Constructor

:param symbolList: must not be empty
:param layer: must not be null
:param label: value label
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

    const QgsVectorLayer *vectorLayer() const;
%Docstring
Returns the vector layer associated with the widget.

.. versionadded:: 2.12
%End

  public slots:
    void dataDefinedChanged();

  protected:

    void init( int propertyKey ); // needed in children ctor to call virtual

  private:
    QgsProperty symbolDataDefined() const ;
    virtual QgsProperty symbolDataDefined( const QgsSymbol * ) const = 0 ;
    virtual double value( const QgsSymbol * ) const = 0 ;
    virtual void setDataDefined( QgsSymbol *symbol, const QgsProperty &dd ) = 0 ;
};

class QgsDataDefinedSizeDialog : QgsDataDefinedValueDialog
{

%TypeHeaderCode
#include "qgsrendererwidget.h"
%End
  public:
    QgsDataDefinedSizeDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer );

  protected:
    virtual QgsProperty symbolDataDefined( const QgsSymbol *symbol ) const;


    virtual double value( const QgsSymbol *symbol ) const;

    virtual void setDataDefined( QgsSymbol *symbol, const QgsProperty &dd );


};

class QgsDataDefinedRotationDialog : QgsDataDefinedValueDialog
{

%TypeHeaderCode
#include "qgsrendererwidget.h"
%End
  public:
    QgsDataDefinedRotationDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer );

  protected:
    virtual QgsProperty symbolDataDefined( const QgsSymbol *symbol ) const;


    virtual double value( const QgsSymbol *symbol ) const;

    virtual void setDataDefined( QgsSymbol *symbol, const QgsProperty &dd );

};

class QgsDataDefinedWidthDialog : QgsDataDefinedValueDialog
{

%TypeHeaderCode
#include "qgsrendererwidget.h"
%End
  public:
    QgsDataDefinedWidthDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer );

  protected:
    virtual QgsProperty symbolDataDefined( const QgsSymbol *symbol ) const;


    virtual double value( const QgsSymbol *symbol ) const;

    virtual void setDataDefined( QgsSymbol *symbol, const QgsProperty &dd );

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsrendererwidget.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
