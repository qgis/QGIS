class QgsHeatmapRenderer : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsheatmaprenderer.h>
%End
  public:

    QgsHeatmapRenderer();
    virtual ~QgsHeatmapRenderer();

    //reimplemented methods
    virtual QgsHeatmapRenderer* clone() const /Factory/;
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );
    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );
    virtual void stopRender( QgsRenderContext& context );
    //! @note symbolForFeature2 in python bindings
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) /PyName=symbolForFeature2/;
    //! @note symbol2 in python bindings
    virtual QgsSymbolV2List symbols( QgsRenderContext& context ) /PyName=symbols2/;
    virtual QString dump() const;
    virtual QList<QString> usedAttributes();
    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;
    virtual QDomElement save( QDomDocument& doc );
    static QgsHeatmapRenderer* convertFromRenderer( const QgsFeatureRendererV2* renderer ) /Factory/;

    //reimplemented to extent the request so that points up to heatmap's radius distance outside
    //visible area are included
    virtual void modifyRequestExtent( QgsRectangle& extent, QgsRenderContext& context );

    //heatmap specific methods

    /** Returns the color ramp used for shading the heatmap.
     * @returns color ramp for heatmap
     * @see setColorRamp
     */
    QgsVectorColorRampV2* colorRamp() const;
    /** Sets the color ramp to use for shading the heatmap.
     * @param ramp color ramp for heatmap. Ownership of ramp is transferred to the renderer.
     * @see colorRamp
     */
    void setColorRamp( QgsVectorColorRampV2* ramp /Transfer/ );

    /** Returns whether the ramp is inverted
     * @returns true if color ramp is inverted
     * @see setInvertRamp
     * @see colorRamp
     */
    bool invertRamp() const;

    /** Sets whether the ramp is inverted
     * @param invert set to true to invert color ramp
     * @see invertRamp
     * @see setColorRamp
     */
    void setInvertRamp( const bool invert );

    /** Returns the radius for the heatmap
     * @returns heatmap radius
     * @see setRadius
     * @see radiusUnit
     * @see radiusMapUnitScale
     */
    double radius() const;
    /** Sets the radius for the heatmap
     * @param radius heatmap radius
     * @see radius
     * @see setRadiusUnit
     * @see setRadiusMapUnitScale
     */
    void setRadius( const double radius );

    /** Returns the units used for the heatmap's radius
     * @returns units for heatmap radius
     * @see radius
     * @see setRadiusUnit
     * @see radiusMapUnitScale
     */
    QgsSymbolV2::OutputUnit radiusUnit() const;
    /** Sets the units used for the heatmap's radius
     * @param unit units for heatmap radius
     * @see radiusUnit
     * @see setRadius
     * @see radiusMapUnitScale
     */
    void setRadiusUnit( const QgsSymbolV2::OutputUnit unit );

    /** Returns the map unit scale used for the heatmap's radius
     * @returns map unit scale for heatmap's radius
     * @see radius
     * @see radiusUnit
     * @see setRadiusMapUnitScale
     */
    const QgsMapUnitScale& radiusMapUnitScale() const;
    /** Sets the map unit scale used for the heatmap's radius
     * @param scale map unit scale for heatmap's radius
     * @see setRadius
     * @see setRadiusUnit
     * @see radiusMapUnitScale
     */
    void setRadiusMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the maximum value used for shading the heatmap.
     * @returns maximum value for heatmap shading. If 0, then maximum value will be automatically
     * calculated.
     * @see setMaximumValue
     */
    double maximumValue() const;
    /** Sets the maximum value used for shading the heatmap.
     * @param value maximum value for heatmap shading. Set to 0 for automatic calculation of
     * maximum value.
     * @see maximumValue
     */
    void setMaximumValue( const double value );

    /** Returns the render quality used for drawing the heatmap.
     * @returns render quality. A value of 1 indicates maximum quality, and increasing the
     * value will result in faster drawing but lower quality rendering.
     * @see setRenderQuality
     */
    double renderQuality() const;
    /** Sets the render quality used for drawing the heatmap.
     * @param quality render quality. A value of 1 indicates maximum quality, and increasing the
     * value will result in faster drawing but lower quality rendering.
     * @see renderQuality
     */
    void setRenderQuality( const int quality );

    /** Returns the expression used for weighting points when generating the heatmap.
     * @returns point weight expression. If empty, all points are equally weighted.
     * @see setWeightExpression
     */
    QString weightExpression() const;

    /** Sets the expression used for weighting points when generating the heatmap.
     * @param expression point weight expression. If set to empty, all points are equally weighted.
     * @see weightExpression
     */
    void setWeightExpression( const QString& expression );

};
