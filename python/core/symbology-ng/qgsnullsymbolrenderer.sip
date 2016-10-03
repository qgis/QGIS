/** \ingroup core
 * \class QgsNullSymbolRenderer
 * \brief Null symbol renderer. Renderer which draws no symbols for features by default, but allows for labeling
 * and diagrams for the layer. Selected features will also be drawn with a default symbol.
 * \note Added in version 2.16
 */

class QgsNullSymbolRenderer : QgsFeatureRenderer
{
%TypeHeaderCode
#include <qgsnullsymbolrenderer.h>
%End
  public:

    QgsNullSymbolRenderer();

    virtual ~QgsNullSymbolRenderer();

    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );
    virtual void stopRender( QgsRenderContext& context );
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext& context );

    virtual QSet<QString> usedAttributes() const;
    virtual QString dump() const;
    virtual QgsFeatureRenderer* clone() const /Factory/;
    virtual QgsSymbolList symbols( QgsRenderContext& context );

    /** Creates a null renderer from XML element.
     * @param element DOM element
     * @returns new null symbol renderer
     */
    static QgsFeatureRenderer* create( QDomElement& element ) /Factory/;

    virtual QDomElement save( QDomDocument& doc );

    /** Creates a QgsNullSymbolRenderer from an existing renderer.
     * @param renderer renderer to convert from
     * @returns a new renderer if the conversion was possible, otherwise nullptr.
     */
    static QgsNullSymbolRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;

};
