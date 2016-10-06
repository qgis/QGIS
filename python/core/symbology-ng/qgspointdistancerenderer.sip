/** \class QgsPointDistanceRenderer
 * \ingroup core
 * An abstract base class for distance based point renderers (eg clusterer and displacement renderers).
 * QgsPointDistanceRenderer handles calculation of point clusters using a distance based threshold.
 * Subclasses must implement drawGroup() to handle the rendering of individual point clusters
 * in the desired style.
 * \note added in QGIS 3.0
 */

class QgsPointDistanceRenderer : QgsFeatureRenderer
{
%TypeHeaderCode
#include <qgspointdistancerenderer.h>
%End

  public:

    //! Contains properties for a feature within a clustered group.
    struct GroupedFeature
    {
      /** Constructor for GroupedFeature.
       * @param feature feature
       * @param symbol base symbol for rendering feature
       * @param isSelected set to true if feature is selected and should be rendered in a selected state
       * @param label optional label text, or empty string for no label
       */
      GroupedFeature( const QgsFeature& feature, QgsMarkerSymbol* symbol, bool isSelected, const QString& label = QString() );

      //! Feature
      QgsFeature feature;

      //! Base symbol for rendering feature
      QgsMarkerSymbol* symbol;

      //! True if feature is selected and should be rendered in a selected state
      bool isSelected;

      //! Optional label text
      QString label;
    };

    //! A group of clustered points (ie features within the distance tolerance).
    typedef QList< QgsPointDistanceRenderer::GroupedFeature > ClusteredGroup;

    /** Constructor for QgsPointDistanceRenderer.
     * @param rendererName name of renderer for registry
     * @param labelAttributeName optional attribute for labeling points
     */
    QgsPointDistanceRenderer( const QString& rendererName, const QString& labelAttributeName = QString() );

    virtual void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props = QgsStringMap() ) const;
    bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );
    virtual QSet<QString> usedAttributes() const;
    virtual Capabilities capabilities();
    virtual QgsSymbolList symbols( QgsRenderContext& context );
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feat, QgsRenderContext& context );
    virtual QgsSymbolList symbolsForFeature( QgsFeature& feat, QgsRenderContext& context );
    virtual QgsSymbolList originalSymbolsForFeature( QgsFeature& feat, QgsRenderContext& context );
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext& context );
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );
    void stopRender( QgsRenderContext& context );
    QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );
    //QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = "" );
    void setEmbeddedRenderer( QgsFeatureRenderer* r /Transfer/ );
    const QgsFeatureRenderer* embeddedRenderer() const;
    void setLegendSymbolItem( const QString& key, QgsSymbol* symbol  /Transfer/ );
    bool legendSymbolItemsCheckable() const;
    bool legendSymbolItemChecked( const QString& key );
    void checkLegendSymbolItem( const QString& key, bool state );
    virtual QString filter( const QgsFields& fields = QgsFields() );

    /** Sets the attribute name for labeling points.
     * @param name attribute name, or empty string to avoid labeling features by the renderer
     * @see labelAttributeName()
     * @see setLabelFont()
     * @see setLabelColor()
     * @see setMaxLabelScaleDenominator()
     */
    void setLabelAttributeName( const QString& name );

    /** Returns the attribute name used for labeling points, or an empty string if no labeling
     * will be done by the renderer.
     * @see setLabelAttributeName()
     * @see labelFont()
     * @see maxLabelScaleDenominator()
     * @see labelColor()
     */
    QString labelAttributeName() const;

    /** Sets the font used for labeling points.
     * @param font label font
     * @see labelFont()
     * @see setLabelAttributeName()
     * @see setLabelColor()
     */
    void setLabelFont( const QFont& font );

    /** Returns the font used for labeling points.
     * @see setLabelFont()
     * @see labelAttributeName()
     * @see labelColor()
     */
    QFont labelFont() const;

    /** Sets the maximum scale at which points should be labeled by the renderer.
     * @param denominator maximum scale denominator
     * @see maxLabelScaleDenominator()
     * @see setLabelAttributeName()
     */
    void setMaxLabelScaleDenominator( double denominator );

    /** Returns the denominator for the maximum scale at which points should be labeled by the renderer.
     * @see setMaxLabelScaleDenominator()
     * @see labelAttributeName()
     */
    double maxLabelScaleDenominator() const;

    /** Sets the color to use for for labeling points.
     * @param color label color
     * @see labelColor()
     * @see setLabelAttributeName()
     * @see setLabelFont()
     */
    void setLabelColor( const QColor& color );

    /** Returns the color used for for labeling points.
     * @see setLabelColor()
     * @see labelAttributeName()
     * @see labelFont()
     */
    QColor labelColor() const;

    /** Sets the tolerance distance for grouping points. Units are specified using
     * setToleranceUnit().
     * @param distance tolerance distance
     * @see tolerance()
     * @see setToleranceUnit()
     */
    void setTolerance( double distance );

    /** Returns the tolerance distance for grouping points. Units are retrieved using
     * toleranceUnit().
     * @see setTolerance()
     * @see toleranceUnit()
     */
    double tolerance() const;

    /** Sets the units for the tolerance distance.
     * @param unit tolerance distance units
     * @see setTolerance()
     * @see toleranceUnit()
     * @note added in QGIS 2.12
     */
    void setToleranceUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the tolerance distance.
     * @see tolerance()
     * @see setToleranceUnit()
     * @note added in QGIS 2.12
     */
    QgsUnitTypes::RenderUnit toleranceUnit() const;

    /** Sets the map unit scale object for the distance tolerance. This is only used if the
     * toleranceUnit() is set to QgsUnitTypes::RenderMapUnits.
     * @param scale scale for distance tolerance
     * @see toleranceMapUnitScale()
     * @see setToleranceUnit()
     */
    void setToleranceMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the map unit scale object for the distance tolerance. This is only used if the
     * toleranceUnit() is set to QgsUnitTypes::RenderMapUnits.
     * @see setToleranceMapUnitScale()
     * @see toleranceUnit()
     */
    const QgsMapUnitScale& toleranceMapUnitScale() const;

  protected:

    /** Renders the labels for a group.
     * @param centerPoint center point of group
     * @param context destination render context
     * @param labelShifts displacement for individual label positions
     * @param group group of clustered features to label
     * @note may not be available in Python bindings on some platforms
     */
    void drawLabels( QPointF centerPoint, QgsSymbolRenderContext& context, const QList<QPointF>& labelShifts, const  QgsPointDistanceRenderer::ClusteredGroup& group );

  private:

    /** Draws a group of clustered points.
     * @param centerPoint central point (geographic centroid) of all points contained within the cluster
     * @param context destination render context
     * @param group contents of group
     */
    virtual void drawGroup( QPointF centerPoint, QgsRenderContext& context, const QgsPointDistanceRenderer::ClusteredGroup& group ) = 0;

};
