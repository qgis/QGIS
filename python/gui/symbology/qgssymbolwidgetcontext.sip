/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbolwidgetcontext.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsSymbolWidgetContext
{
%Docstring
Contains settings which reflect the context in which a symbol (or renderer) widget is shown, e.g., the
map canvas and relevant expression contexts.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssymbolwidgetcontext.h"
%End
  public:

    QgsSymbolWidgetContext();
%Docstring
Constructor for QgsSymbolWidgetContext.
%End

    QgsSymbolWidgetContext( const QgsSymbolWidgetContext &other );
%Docstring
Copy constructor.

:param other: source QgsSymbolWidgetContext
%End


    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets the map canvas associated with the widget. This allows the widget to retrieve the current
map scale and other properties from the canvas.

:param canvas: map canvas

.. seealso:: :py:func:`mapCanvas`
%End

    QgsMapCanvas *mapCanvas() const;
%Docstring
Returns the map canvas associated with the widget.

.. seealso:: :py:func:`setMapCanvas`
%End

    void setExpressionContext( QgsExpressionContext *context );
%Docstring
Sets the optional expression context used for the widget. This expression context is used for
evaluating data defined symbol properties and for populating based expression widgets in
the layer widget.

:param context: expression context pointer. Ownership is not transferred.

.. seealso:: :py:func:`expressionContext`

.. seealso:: :py:func:`setAdditionalExpressionContextScopes`
%End

    QgsExpressionContext *expressionContext() const;
%Docstring
Returns the expression context used for the widget, if set. This expression context is used for
evaluating data defined symbol properties and for populating based expression widgets in
the layer widget.

.. seealso:: :py:func:`setExpressionContext`
%End

    void setAdditionalExpressionContextScopes( const QList< QgsExpressionContextScope > &scopes );
%Docstring
Sets a list of additional expression context scopes to show as available within the layer.

:param scopes: list of additional scopes which will be added in order to the end of the default expression context

.. seealso:: :py:func:`setExpressionContext`
%End

    QList< QgsExpressionContextScope > additionalExpressionContextScopes() const;
%Docstring
Returns the list of additional expression context scopes to show as available within the layer.

.. seealso:: :py:func:`setAdditionalExpressionContextScopes`
%End

    QList<QgsExpressionContextScope *> globalProjectAtlasMapLayerScopes( const QgsMapLayer *layer ) const /Factory/;
%Docstring
Returns list of scopes: global, project, atlas, map, layer.
Ownership is transferred to the caller.

.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbolwidgetcontext.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
