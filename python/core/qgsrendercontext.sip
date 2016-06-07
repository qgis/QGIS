class QgsRenderContext
{

%TypeHeaderCode
#include <qgsrendercontext.h>
%End

  public:
    QgsRenderContext();
    QgsRenderContext( const QgsRenderContext& rh );
    ~QgsRenderContext();

    /** Enumeration of flags that affect rendering operations.
     * @note added in QGIS 2.14
     */
    enum Flag
    {
      DrawEditingInfo,          //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput,        //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects,       //!< Enable layer transparency and blending effects
      UseRenderingOptimization, //!< Enable vector simplification and other rendering optimizations
      DrawSelection,            //!< Whether vector selections should be shown in the rendered map
      DrawSymbolBounds,         //!< Draw bounds of symbols (for debugging/testing)
      RenderMapTile,            //!< Draw map such that there are no problems between adjacent tiles
      Antialiasing,             //!< Use antialiasing while drawing
    };
    typedef QFlags<QgsRenderContext::Flag> Flags;

    /** Set combination of flags that will be used for rendering.
     * @note added in QGIS 2.14
     */
    void setFlags( const QgsRenderContext::Flags& flags );

    /** Enable or disable a particular flag (other flags are not affected)
     * @note added in QGIS 2.14
     */
    void setFlag( Flag flag, bool on = true );

    /** Return combination of flags used for rendering.
     * @note added in QGIS 2.14
     */
    Flags flags() const;

    /** Check whether a particular flag is enabled.
     * @note added in QGIS 2.14
     */
    bool testFlag( Flag flag ) const;

    //! create initialized QgsRenderContext instance from given QgsMapSettings
    //! @note added in 2.4
    static QgsRenderContext fromMapSettings( const QgsMapSettings& mapSettings );

    //getters

    QPainter* painter();
    const QPainter* constPainter() const;

    const QgsCoordinateTransform* coordinateTransform() const;

    const QgsRectangle& extent() const;

    const QgsMapToPixel& mapToPixel() const;

    double scaleFactor() const;

    double rasterScaleFactor() const;

    bool renderingStopped() const;

    bool forceVectorOutput() const;

    /** Returns true if advanced effects such as blend modes such be used
     */
    bool useAdvancedEffects() const;

    /** Used to enable or disable advanced effects such as blend modes
     */
    void setUseAdvancedEffects( bool enabled );

    bool drawEditingInformation() const;

    double rendererScale() const;

    QgsLabelingEngineInterface* labelingEngine();

    //! Get access to new labeling engine (may be nullptr)
    //! @note not available in Python bindings
    // QgsLabelingEngineV2* labelingEngineV2() const { return mLabelingEngine2; }

    QColor selectionColor() const;

    /** Returns true if vector selections should be shown in the rendered map
     * @returns true if selections should be shown
     * @see setShowSelection
     * @see selectionColor
     * @note Added in QGIS v2.4
     */
    bool showSelection() const;

    //setters

    /** Sets coordinate transformation. QgsRenderContext does not take ownership*/
    void setCoordinateTransform( const QgsCoordinateTransform* t );
    void setMapToPixel( const QgsMapToPixel& mtp );
    void setExtent( const QgsRectangle& extent );

    void setDrawEditingInformation( bool b );

    void setRenderingStopped( bool stopped );
    void setScaleFactor( double factor );
    void setRasterScaleFactor( double factor );
    void setRendererScale( double scale );
    void setPainter( QPainter* p );

    void setForceVectorOutput( bool force );

    void setLabelingEngine( QgsLabelingEngineInterface* iface );
    //! Assign new labeling engine
    //! @note not available in Python bindings
    // void setLabelingEngineV2( QgsLabelingEngineV2* engine2 ) { mLabelingEngine2 = engine2; }
    void setSelectionColor( const QColor& color );

    /** Sets whether vector selections should be shown in the rendered map
     * @param showSelection set to true if selections should be shown
     * @see showSelection
     * @see setSelectionColor
     * @note Added in QGIS v2.4
     */
    void setShowSelection( const bool showSelection );

    /** Returns true if the rendering optimization (geometry simplification) can be executed
     */
    bool useRenderingOptimization() const;

    void setUseRenderingOptimization( bool enabled );

    //! Added in QGIS v2.4
    const QgsVectorSimplifyMethod& vectorSimplifyMethod() const;
    void setVectorSimplifyMethod( const QgsVectorSimplifyMethod& simplifyMethod );

    /** Sets the expression context. This context is used for all expression evaluation
     * associated with this render context.
     * @see expressionContext()
     * @note added in QGIS 2.12
     */
    void setExpressionContext( const QgsExpressionContext& context );

    /** Gets the expression context. This context should be used for all expression evaluation
     * associated with this render context.
     * @see setExpressionContext()
     * @note added in QGIS 2.12
     */
    QgsExpressionContext& expressionContext();

    /** Gets the expression context (const version). This context should be used for all expression evaluation
     * associated with this render context.
     * @see setExpressionContext()
     * @note added in QGIS 2.12
     * @note not available in Python bindings
     */
    //const QgsExpressionContext& expressionContext() const;

    /** Returns pointer to the unsegmentized geometry*/
    const QgsAbstractGeometryV2* geometry() const;
    /** Sets pointer to original (unsegmentized) geometry*/
    void setGeometry( const QgsAbstractGeometryV2* geometry );

    /** Set a filter feature provider used for additional filtering of rendered features.
     * @param ffp the filter feature provider
     * @note added in QGIS 2.14
     * @see featureFilterProvider()
     */
    void setFeatureFilterProvider( const QgsFeatureFilterProvider* ffp );

    /** Get the filter feature provider used for additional filtering of rendered features.
     * @return the filter feature provider
     * @note added in QGIS 2.14
     * @see setFeatureFilterProvider()
     */
    const QgsFeatureFilterProvider* featureFilterProvider() const;

    /** Sets the segmentation tolerance applied when rendering curved geometries
    @param tolerance the segmentation tolerance*/
    void setSegmentationTolerance( double tolerance );
    /** Gets the segmentation tolerance applied when rendering curved geometries*/
    double segmentationTolerance() const;

    /** Sets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
    @param type the segmentation tolerance typename*/
    void setSegmentationToleranceType( QgsAbstractGeometryV2::SegmentationToleranceType type );
    /** Gets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)*/
    QgsAbstractGeometryV2::SegmentationToleranceType segmentationToleranceType() const;
};
