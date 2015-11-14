
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

    bool forceVectorOutput() const;

    /** Returns true if advanced effects such as blend modes such be used */
    bool useAdvancedEffects() const;

    /** Used to enable or disable advanced effects such as blend modes */
    void setUseAdvancedEffects( bool enabled );

    bool drawEditingInformation() const;

    double rendererScale() const;

    QgsLabelingEngineInterface* labelingEngine();

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
    void setSelectionColor( const QColor& color );

    /** Sets whether vector selections should be shown in the rendered map
     * @param showSelection set to true if selections should be shown
     * @see showSelection
     * @see setSelectionColor
     * @note Added in QGIS v2.4
    */
    void setShowSelection( const bool showSelection );

    /** Returns true if the rendering optimization (geometry simplification) can be executed*/
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

    /** Returns pointer to the unsegmentized geometry
        @return the geometry*/
    const QgsAbstractGeometryV2* geometry() const;
    /** Sets pointer to original (unsegmentized) geometry
        @geometry the geometry*/
    void setGeometry( const QgsAbstractGeometryV2* geometry );
};
