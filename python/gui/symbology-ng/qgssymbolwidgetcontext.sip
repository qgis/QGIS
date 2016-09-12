
/** \ingroup gui
 * \class QgsSymbolWidgetContext
 * Contains settings which reflect the context in which a symbol (or renderer) widget is shown, eg the
 * map canvas and relevant expression contexts.
 *
 * \note added in QGIS 3.0
 */
 class QgsSymbolWidgetContext
{
%TypeHeaderCode
#include <qgssymbolwidgetcontext.h>
%End
  public:

    QgsSymbolWidgetContext();

    QgsSymbolWidgetContext( const QgsSymbolWidgetContext& other );

    //QgsSymbolWidgetContext& operator=( const QgsSymbolWidgetContext& other );

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas()
     */
    QgsMapCanvas* mapCanvas() const;

    /** Sets the optional expression context used for the widget. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the layer widget.
     * @param context expression context pointer. Ownership is not transferred.
     * @see expressionContext()
     * @see setAdditionalExpressionContextScopes()
     */
    void setExpressionContext( QgsExpressionContext* context );

    /** Returns the expression context used for the widget, if set. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the layer widget.
     * @see setExpressionContext()
     */
    QgsExpressionContext* expressionContext() const;

    /** Sets a list of additional expression context scopes to show as available within the layer.
     * @param scopes list of additional scopes which will be added in order to the end of the default expression context
     * @see setExpressionContext()
     */
    void setAdditionalExpressionContextScopes( const QList< QgsExpressionContextScope >& scopes );

    /** Returns the list of additional expression context scopes to show as available within the layer.
     * @see setAdditionalExpressionContextScopes()
     */
    QList< QgsExpressionContextScope > additionalExpressionContextScopes() const;
};

