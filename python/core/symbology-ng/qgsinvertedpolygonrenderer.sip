class QgsInvertedPolygonRenderer : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsinvertedpolygonrenderer.h>
%End
  public:

    /** Constructor
     * @param embeddedRenderer optional embeddedRenderer. If null, a default one will be assigned
     */
    QgsInvertedPolygonRenderer( const QgsFeatureRendererV2* embeddedRenderer /Transfer/ = 0 );
    virtual ~QgsInvertedPolygonRenderer();

    /** Used to clone this feature renderer.*/
    virtual QgsFeatureRendererV2* clone() const /Factory/;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    /** Renders a given feature.
     * This will here collect features. The actual rendering will be postponed to stopRender()
     * @param feature the feature to render
     * @param context the rendering context
     * @param layer the symbol layer to render, if that makes sense
     * @param selected whether this feature has been selected (this will add decorations)
     * @param drawVertexMarker whether this feature has vertex markers (in edit mode usually)
     * @returns true if the rendering was ok
     */
    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    /**
     * The actual rendering will take place here.
     * Features collected during renderFeature() are rendered using the embedded feature renderer
     */
    virtual void stopRender( QgsRenderContext& context );

    /** @returns a textual representation of the renderer */
    virtual QString dump() const;

    /** Proxy that will call this method on the embedded renderer. */
    virtual QList<QString> usedAttributes();
    /** Proxy that will call this method on the embedded renderer. */
    virtual int capabilities();
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2List symbols();
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feat );
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat );
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2List originalSymbolsForFeature( QgsFeature& feat );
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );
    /** Proxy that will call this method on the embedded renderer.
      @note not available in python bindings
     */
    // virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = "" );
    /** Proxy that will call this method on the embedded renderer. */
    virtual bool willRenderFeature( QgsFeature& feat );

    /** Creates a renderer out of an XML, for loading*/
    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    /** Creates an XML representation of the renderer. Used for saving purpose
     * @param doc the XML document where to create the XML subtree
     * @returns the created XML subtree
     */
    virtual QDomElement save( QDomDocument& doc );

    /** Sets the embedded renderer
     * @param subRenderer the embedded renderer (will be cloned)
     */
    void setEmbeddedRenderer( const QgsFeatureRendererV2* subRenderer );
    /** @returns the current embedded renderer
     */
    const QgsFeatureRendererV2* embeddedRenderer() const;

    /** @returns true if the geometries are to be preprocessed (merged with an union) before rendering.*/
    bool preprocessingEnabled() const;
    /**
        @param enabled enables or disables the preprocessing.
        When enabled, geometries will be merged with an union before being rendered.
        It allows fixing some rendering artefacts (when rendering overlapping polygons for instance).
        This will involve some CPU-demanding computations and is thus disabled by default.
    */
    void setPreprocessingEnabled( bool enabled );

    /** Creates a QgsInvertedPolygonRenderer by a conversion from an existing renderer.
        @note added in 2.5
        @returns a new renderer if the conversion was possible, otherwise 0.
        */
    static QgsInvertedPolygonRenderer* convertFromRenderer( const QgsFeatureRendererV2* renderer ) /Factory/;
};
