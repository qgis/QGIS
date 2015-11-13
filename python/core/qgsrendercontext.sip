
class QgsRenderContext
{

%TypeHeaderCode
#include <qgsrendercontext.h>
%End

  public:
    QgsRenderContext();
    ~QgsRenderContext();

    //! Enumeration of flags that affect rendering operations
    enum Flag
    {
      DrawEditingInfo,  //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput,  //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects,  //!< Enable layer transparency and blending effects
      UseRenderingOptimization, //!< Enable vector simplification and other rendering optimizations
      DrawSelection,  //!< Whether vector selections should be shown in the rendered map
    };
    typedef QFlags<QgsRenderContext::Flag> Flags;

    //! Set combination of flags that will be used for rendering
    void setFlags( const QgsRenderContext::Flags& flags );
    //! Enable or disable a particular flag (other flags are not affected)
    void setFlag( Flag flag, bool on = true );
    //! Return combination of flags used for rendering
    Flags flags() const;
    //! Check whether a particular flag is enabled
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

    //! @deprecated use testFlag( QgsRenderContext::ForceVectorOutput ) instead
    bool forceVectorOutput() const /Deprecated/;

    /** Returns true if advanced effects such as blend modes such be used
     * @deprecated use testFlag( QgsRenderContext::UseAdvancedEffects ) instead
    */
    bool useAdvancedEffects() const /Deprecated/;

    /** Used to enable or disable advanced effects such as blend modes
     * @deprecated use setFlag( QgsRenderContext::UseAdvancedEffects, enabled ) instead
    */
    void setUseAdvancedEffects( bool enabled ) /Deprecated/;

    //! @deprecated use testFlag( QgsRenderContext::DrawEditingInfo ) instead
    bool drawEditingInformation() const /Deprecated/;

    double rendererScale() const;

    QgsLabelingEngineInterface* labelingEngine();

    QColor selectionColor() const;

    /** Returns true if vector selections should be shown in the rendered map
     * @returns true if selections should be shown
     * @see setShowSelection
     * @see selectionColor
     * @note Added in QGIS v2.4
     * @deprecated use testFlag( QgsRenderContext::DrawSelection ) instead
    */
    bool showSelection() const /Deprecated/;

    //setters

    /** Sets coordinate transformation. QgsRenderContext does not take ownership*/
    void setCoordinateTransform( const QgsCoordinateTransform* t );
    void setMapToPixel( const QgsMapToPixel& mtp );
    void setExtent( const QgsRectangle& extent );

    //! @deprecated use setFlag( QgsRenderContext::DrawEditingInfo, b ) instead
    void setDrawEditingInformation( bool b ) /Deprecated/;

    void setRenderingStopped( bool stopped );
    void setScaleFactor( double factor );
    void setRasterScaleFactor( double factor );
    void setRendererScale( double scale );
    void setPainter( QPainter* p );

    //! @deprecated use setFlag( QgsRenderContext::ForceVectorOutput, b ) instead
    void setForceVectorOutput( bool force ) /Deprecated/;

    void setLabelingEngine( QgsLabelingEngineInterface* iface );
    void setSelectionColor( const QColor& color );

    /** Sets whether vector selections should be shown in the rendered map
     * @param showSelection set to true if selections should be shown
     * @see showSelection
     * @see setSelectionColor
     * @note Added in QGIS v2.4
     * @deprecated use setFlag( QgsRenderContext::DrawSelection, showSelection ) instead
    */
    void setShowSelection( const bool showSelection ) /Deprecated/;

    /** Returns true if the rendering optimization (geometry simplification) can be executed
     * @deprecated use testFlag( QgsRenderContext::UseRenderingOptimization ) instead
    */
    bool useRenderingOptimization() const /Deprecated/;

    //! @deprecated use setFlag( QgsRenderContext::UseRenderingOptimization, b ) instead
    void setUseRenderingOptimization( bool enabled ) /Deprecated/;

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

    /** Returns pointer to the unsegmentized geometry
        @return the geometry*/
    const QgsAbstractGeometryV2* geometry() const;
    /** Sets pointer to original (unsegmentized) geometry
        @geometry the geometry*/
    void setGeometry( const QgsAbstractGeometryV2* geometry );
};
