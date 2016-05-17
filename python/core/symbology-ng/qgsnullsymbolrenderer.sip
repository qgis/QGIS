/** \ingroup core
 * \class QgsNullSymbolRenderer
 * \brief Null symbol renderer. Renderer which draws no symbols for features by default, but allows for labeling
 * and diagrams for the layer. Selected features will also be drawn with a default symbol.
 * \note Added in version 2.16
 */

class QgsNullSymbolRenderer : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsnullsymbolrenderer.h>
%End
  public:

    QgsNullSymbolRenderer();

    virtual ~QgsNullSymbolRenderer();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );
    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );
    virtual void stopRender( QgsRenderContext& context );
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext& context );

    virtual QList<QString> usedAttributes();
    virtual QString dump() const;
    virtual QgsFeatureRendererV2* clone() const /Factory/;
    virtual QgsSymbolV2List symbols( QgsRenderContext& context ) /PyName=symbols2/;

    /** Creates a null renderer from XML element.
     * @param element DOM element
     * @returns new null symbol renderer
     */
    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    virtual QDomElement save( QDomDocument& doc );

    /** Creates a QgsNullSymbolRenderer from an existing renderer.
     * @param renderer renderer to convert from
     * @returns a new renderer if the conversion was possible, otherwise nullptr.
     */
    static QgsNullSymbolRenderer* convertFromRenderer( const QgsFeatureRendererV2 *renderer ) /Factory/;

};
