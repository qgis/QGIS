
typedef QList<QgsSymbolLayer*> QgsSymbolLayerList;

class QgsSymbol
{
%TypeHeaderCode
#include <qgssymbol.h>
%End

%ConvertToSubClassCode
  switch (sipCpp->type())
  {
    case QgsSymbol::Marker: sipType = sipType_QgsMarkerSymbol; break;
    case QgsSymbol::Line: sipType = sipType_QgsLineSymbol; break;
    case QgsSymbol::Fill: sipType = sipType_QgsFillSymbol; break;
    default: sipType = 0; break;
  }
%End

  public:

    /**
     * Type of the symbol
     */
    enum SymbolType
    {
      Marker, //!< Marker symbol
      Line,   //!< Line symbol
      Fill,   //!< Fill symbol
      Hybrid  //!< Hybrid symbol
    };

    /**
     * Scale method
     */
    enum ScaleMethod
    {
      ScaleArea,     //!< Calculate scale by the area
      ScaleDiameter  //!< Calculate scale by the diameter
    };

    //! Flags controlling behaviour of symbols during rendering
    enum RenderHint
    {
      DynamicRotation, //!< Rotation of symbol may be changed during rendering and symbol should not be cached
    };
    typedef QFlags<QgsSymbol::RenderHint> RenderHints;

    virtual ~QgsSymbol();

    //! return new default symbol for specified geometry type
    static QgsSymbol* defaultSymbol( QgsWkbTypes::GeometryType geomType ) /Factory/;

    SymbolType type() const;

    // symbol layers handling

    /** Returns list of symbol layers contained in the symbol.
     * @returns symbol layers list
     * @note added in QGIS 2.7
     * @see symbolLayer
     * @see symbolLayerCount
     */
    QgsSymbolLayerList symbolLayers();

    /** Returns a specific symbol layers contained in the symbol.
     * @param layer layer number
     * @returns corresponding symbol layer
     * @note added in QGIS 2.7
     * @see symbolLayers
     * @see symbolLayerCount
     */
    QgsSymbolLayer* symbolLayer( int layer );

    /** Returns total number of symbol layers contained in the symbol.
     * @returns count of symbol layers
     * @note added in QGIS 2.7
     * @see symbolLayers
     * @see symbolLayer
     */
    int symbolLayerCount();

    /**
     * Insert symbol layer to specified index
     * Ownership will be transferred.
     * @param index The index at which the layer should be added
     * @param layer The symbol layer to add
     * @return True if the layer is added, False if the index or the layer is bad
     */
    bool insertSymbolLayer( int index, QgsSymbolLayer* layer /Transfer/ );

    /**
     * Append symbol layer at the end of the list
     * Ownership will be transferred.
     * @param layer The layer to add
     * @return True if the layer is added, False if the layer is bad
     */
    bool appendSymbolLayer( QgsSymbolLayer* layer /Transfer/ );

    //! delete symbol layer at specified index
    bool deleteSymbolLayer( int index );

    /**
     * Remove symbol layer from the list and return pointer to it.
     * Ownership is handed to the caller.
     * @param index The index of the layer to remove
     * @return A pointer to the removed layer
     */
    QgsSymbolLayer* takeSymbolLayer( int index ) /TransferBack/;

    //! delete layer at specified index and set a new one
    bool changeSymbolLayer( int index, QgsSymbolLayer *layer /Transfer/ );

    /** Begins the rendering process for the symbol. This must be called before renderFeature(),
     * and should be followed by a call to stopRender().
     * @param context render context which symbol will be drawn using
     * @param fields fields for features to be rendered (usually the associated
     * vector layer's fields). Required for correct calculation of data defined
     * overrides.
     * @see stopRender()
     */
    void startRender( QgsRenderContext& context, const QgsFields& fields = QgsFields() );

    /** Ends the rendering process. This should be called after rendering all desired features.
     * @param context render context, must match the context specified when startRender()
     * was called.
     * @see startRender()
     */
    void stopRender( QgsRenderContext& context );

    void setColor( const QColor& color );
    QColor color() const;

    //! Draw icon of the symbol that occupyies area given by size using the painter.
    //! Optionally custom context may be given in order to get rendering of symbols that use map units right.
    //! @note customContext parameter added in 2.6
    void drawPreviewIcon( QPainter* painter, QSize size, QgsRenderContext* customContext = 0 );

    //! export symbol as image format. PNG and SVG supported
    void exportImage( const QString& path, const QString& format, QSize size );

    //! Generate symbol as image
    QImage asImage( QSize size, QgsRenderContext* customContext = 0 );

    /** Returns a large (roughly 100x100 pixel) preview image for the symbol.
     * @param expressionContext optional expression context, for evaluation of
     * data defined symbol properties
     */
    QImage bigSymbolPreviewImage( QgsExpressionContext* expressionContext = 0 );

    QString dump() const;

    virtual QgsSymbol* clone() const = 0 /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    /** Returns the units to use for sizes and widths within the symbol. Individual
     * symbol layer definitions will interpret this in different ways, eg a marker symbol
     * may use it to specify the units for the marker size, while a line symbol
     * may use it to specify the units for the line width.
     * @returns output unit, or QgsUnitTypes::RenderUnknownUnit if the symbol contains mixed units
     * @see setOutputUnit()
     */
    QgsUnitTypes::RenderUnit outputUnit() const;

    /** Sets the units to use for sizes and widths within the symbol. Individual
     * symbol definitions will interpret this in different ways, eg a marker symbol
     * may use it to specify the units for the marker size, while a line symbol
     * may use it to specify the units for the line width.
     * @param unit output units
     * @see outputUnit()
     */
    void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale& scale );

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const;
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha );

    /** Sets rendering hint flags for the symbol.
     * @see renderHints()
     */
    void setRenderHints( RenderHints hints );

    /** Returns the rendering hint flags for the symbol.
     * @see setRenderHints()
     */
    RenderHints renderHints() const;

    /** Sets whether features drawn by the symbol should be clipped to the render context's
     * extent. If this option is enabled then features which are partially outside the extent
     * will be clipped. This speeds up rendering of the feature, but may have undesirable
     * side effects for certain symbol types.
     * @param clipFeaturesToExtent set to true to enable clipping (defaults to true)
     * @note added in QGIS 2.9
     * @see clipFeaturesToExtent
     */
    void setClipFeaturesToExtent( bool clipFeaturesToExtent );

    /** Returns whether features drawn by the symbol will be clipped to the render context's
     * extent. If this option is enabled then features which are partially outside the extent
     * will be clipped. This speeds up rendering of the feature, but may have undesirable
     * side effects for certain symbol types.
     * @returns true if features will be clipped
     * @note added in QGIS 2.9
     * @see setClipFeaturesToExtent
     */
    bool clipFeaturesToExtent() const;

    /**
     * Return a list of attributes required to render this feature.
     * This should include any attributes required by the symbology including
     * the ones required by expressions.
     */
    QSet<QString> usedAttributes() const;

    /** Returns whether the symbol utilises any data defined properties.
     * @note added in QGIS 2.12
     */
    bool hasDataDefinedProperties() const;

    //! @note the layer will be NULL after stopRender
    void setLayer( const QgsVectorLayer* layer );
    const QgsVectorLayer* layer() const;

    /**
     * Render a feature. Before calling this the startRender() method should be called to initialise
     * the rendering process. After rendering all features stopRender() must be called.
     */
    void renderFeature( const QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false, int currentVertexMarkerType = 0, int currentVertexMarkerSize = 0 );

    /**
     * Returns the symbol render context. Only valid between startRender and stopRender calls.
     *
     * @return The symbol render context
     */
    QgsSymbolRenderContext* symbolRenderContext();

  protected:
    QgsSymbol( SymbolType type, const QgsSymbolLayerList& layers /Transfer/ ); // can't be instantiated

    /**
     * Creates a point in screen coordinates from a QgsPointV2 in map coordinates
     */
    static QPointF _getPoint( QgsRenderContext& context, const QgsPointV2& point );

    /**
     * Creates a line string in screen coordinates from a QgsCurve in map
     * coordinates
     */
    static QPolygonF _getLineString( QgsRenderContext& context, const QgsCurve& curve, bool clipToExtent = true );

    /**
     * Creates a polygon ring in screen coordinates from a QgsCurve in map
     * coordinates
     */
    static QPolygonF _getPolygonRing( QgsRenderContext& context, const QgsCurve& curve, bool clipToExtent );

    /**
     * Creates a polygon in screen coordinates from a QgsPolygonV2 in map
     * coordinates
     */
    static void _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, const QgsPolygonV2& polygon, bool clipToExtent = true );

    /**
     * Retrieve a cloned list of all layers that make up this symbol.
     * Ownership is transferred to the caller.
     */
    QgsSymbolLayerList cloneLayers() const /Factory/;

    /**
     * Renders a context using a particular symbol layer without passing in a
     * geometry. This is used as fallback, if the symbol being rendered is not
     * compatible with the specified layer. In such a case, this method can be
     * called and will call the layer's rendering method anyway but the
     * geometry passed to the layer will be empty.
     * This is required for layers that generate their own geometry from other
     * information in the rendering context.
     */
    void renderUsingLayer( QgsSymbolLayer* layer, QgsSymbolRenderContext& context );

    //! Render editing vertex marker at specified point
    //! @note added in QGIS 2.16
    void renderVertexMarker( QPointF pt, QgsRenderContext& context, int currentVertexMarkerType, int currentVertexMarkerSize );

  private:
    QgsSymbol( const QgsSymbol& );
};

QFlags<QgsSymbol::RenderHint> operator|(QgsSymbol::RenderHint f1, QFlags<QgsSymbol::RenderHint> f2);


///////////////////////

class QgsSymbolRenderContext
{
%TypeHeaderCode
#include <qgssymbol.h>
%End

  public:

    /** Constructor for QgsSymbolRenderContext
     * @param c
     * @param u
     * @param alpha
     * @param selected set to true if symbol should be drawn in a "selected" state
     * @param renderHints flags controlling rendering behaviour
     * @param f
     * @param fields
     * @param mapUnitScale
     */
    QgsSymbolRenderContext( QgsRenderContext& c, QgsUnitTypes::RenderUnit u, qreal alpha = 1.0, bool selected = false, QgsSymbol::RenderHints renderHints = 0, const QgsFeature* f = 0, const QgsFields& fields = QgsFields(), const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() );
    ~QgsSymbolRenderContext();

    QgsRenderContext& renderContext();
    // const QgsRenderContext& renderContext() const;

    /** Sets the original value variable value for data defined symbology
     * @param value value for original value variable. This usually represents the symbol property value
     * before any data defined overrides have been applied.
     * @note added in QGIS 2.12
     */
    void setOriginalValueVariable( const QVariant& value );

    //! Returns the output unit for the context
    QgsUnitTypes::RenderUnit outputUnit() const;

    //! Sets the output unit for the context
    void setOutputUnit( QgsUnitTypes::RenderUnit u );

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale& scale );

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const;
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha );

    bool selected() const;
    void setSelected( bool selected );

    /** Returns the rendering hint flags for the symbol.
     * @see setRenderHints()
     */
    QgsSymbol::RenderHints renderHints() const;

    /** Sets rendering hint flags for the symbol.
     * @see renderHints()
     */
    void setRenderHints( QgsSymbol::RenderHints hints );

    void setFeature( const QgsFeature* f );
    //! Current feature being rendered - may be null
    const QgsFeature* feature() const;

    //! Fields of the layer. Currently only available in startRender() calls
    //! to allow symbols with data-defined properties prepare the expressions
    //! (other times fields() returns null)
    //! @note added in 2.4
    QgsFields fields() const;

    /** Part count of current geometry
     * @note added in QGIS 2.16
     */
    int geometryPartCount() const;
    /** Sets the part count of current geometry
     * @note added in QGIS 2.16
     */
    void setGeometryPartCount( int count );

    /** Part number of current geometry
     * @note added in QGIS 2.16
     */
    int geometryPartNum() const;
    /** Sets the part number of current geometry
     * @note added in QGIS 2.16
     */
    void setGeometryPartNum( int num );

    double outputLineWidth( double width ) const;
    double outputPixelSize( double size ) const;

    // workaround for sip 4.7. Don't use assignment - will fail with assertion error
    // QgsSymbolRenderContext& operator=( const QgsSymbolRenderContext& );

    /**
     * This scope is always available when a symbol of this type is being rendered.
     *
     * @return An expression scope for details about this symbol
     */
    QgsExpressionContextScope* expressionContextScope();
    /**
     * Set an expression scope for this symbol.
     *
     * Will take ownership.
     *
     * @param contextScope An expression scope for details about this symbol
     */
    void setExpressionContextScope( QgsExpressionContextScope* contextScope /Transfer/);

  private:

    QgsSymbolRenderContext( const QgsSymbolRenderContext& rh );
};



//////////////////////



class QgsMarkerSymbol : QgsSymbol
{
%TypeHeaderCode
#include <qgssymbol.h>
%End

  public:
    /** Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
      This is a convenience method for easier creation of marker symbols.
     */
    static QgsMarkerSymbol* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsMarkerSymbol( const QgsSymbolLayerList& layers /Transfer/ = QgsSymbolLayerList() );

    /** Sets the angle for the whole symbol. Individual symbol layer sizes
     * will be rotated to maintain their current relative angle to the whole symbol angle.
     * @param angle new symbol angle
     * @see angle()
     */
    void setAngle( double angle );

    /** Returns the marker angle for the whole symbol. Note that for symbols with
     * multiple symbol layers, this will correspond just to the angle of
     * the first symbol layer.
     * @note added in QGIS 2.16
     * @see setAngle()
     */
    double angle() const;

    /** Set data defined angle for whole symbol (including all symbol layers).
     * @param dd data defined angle
     * @note added in QGIS 2.9
     * @see dataDefinedAngle
     */
    void setDataDefinedAngle( const QgsDataDefined& dd );

    /** Returns data defined angle for whole symbol (including all symbol layers).
     * @returns data defined angle, or empty data defined if angle is not set
     * at the marker level
     * @note added in QGIS 2.9
     * @see setDataDefinedAngle
     */
    QgsDataDefined dataDefinedAngle() const;

    /** Sets the line angle modification for the symbol's angle. This angle is added to
     * the marker's rotation and data defined rotation before rendering the symbol, and
     * is usually used for orienting symbols to match a line's angle.
     * @param lineAngle Angle in degrees, valid values are between 0 and 360
     * @note added in QGIS 2.9
     */
    void setLineAngle( double lineAngle );

    /** Sets the size for the whole symbol. Individual symbol layer sizes
     * will be scaled to maintain their current relative size to the whole symbol size.
     * @param size new symbol size
     * @see size()
     * @see setSizeUnit()
     * @see setSizeMapUnitScale()
     */
    void setSize( double size );

    /** Returns the size for the whole symbol, which is the maximum size of
     * all marker symbol layers in the symbol.
     * @see setSize()
     * @see sizeUnit()
     * @see sizeMapUnitScale()
     */
    double size() const;

    /** Sets the size units for the whole symbol (including all symbol layers).
     * @param unit size units
     * @note added in QGIS 2.16
     * @see sizeUnit()
     * @see setSizeMapUnitScale()
     * @see setSize()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the size units for the whole symbol (including all symbol layers).
     * @returns size units, or mixed units if symbol layers have different units
     * @note added in QGIS 2.16
     * @see setSizeUnit()
     * @see sizeMapUnitScale()
     * @see size()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /** Sets the size map unit scale for the whole symbol (including all symbol layers).
     * @param scale map unit scale
     * @note added in QGIS 2.16
     * @see sizeMapUnitScale()
     * @see setSizeUnit()
     * @see setSize()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the size map unit scale for the whole symbol. Note that for symbols with
     * multiple symbol layers, this will correspond just to the map unit scale
     * for the first symbol layer.
     * @note added in QGIS 2.16
     * @see setSizeMapUnitScale()
     * @see sizeUnit()
     * @see size()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /** Set data defined size for whole symbol (including all symbol layers).
     * @param dd data defined size
     * @note added in QGIS 2.9
     * @see dataDefinedSize
     */
    void setDataDefinedSize( const QgsDataDefined& dd );

    /** Returns data defined size for whole symbol (including all symbol layers).
     * @returns data defined size, or empty data defined if size is not set
     * at the marker level
     * @note added in QGIS 2.9
     * @see setDataDefinedSize
     */
    QgsDataDefined dataDefinedSize() const;

    void setScaleMethod( QgsSymbol::ScaleMethod scaleMethod );
    ScaleMethod scaleMethod();

    void renderPoint( QPointF point, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    /** Returns the approximate bounding box of the marker symbol, which includes the bounding box
     * of all symbol layers for the symbol. It is recommended to use this method only between startRender()
     * and stopRender() calls, or data defined rotation and offset will not be correctly calculated.
     * @param point location of rendered point in painter units
     * @param context render context
     * @param feature feature being rendered at point (optional). If not specified, the bounds calculation will not
     * include data defined parameters such as offset and rotation
     * @returns approximate symbol bounds, in painter units
     * @note added in QGIS 2.14
    */
    QRectF bounds(QPointF point, QgsRenderContext& context, const QgsFeature &feature = QgsFeature() ) const;

    virtual QgsMarkerSymbol* clone() const /Factory/;
};



class QgsLineSymbol : QgsSymbol
{
%TypeHeaderCode
#include <qgssymbol.h>
%End

  public:
    /** Create a line symbol with one symbol layer: SimpleLine with specified properties.
     * This is a convenience method for easier creation of line symbols.
     */
    static QgsLineSymbol* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsLineSymbol( const QgsSymbolLayerList& layers /Transfer/ = QgsSymbolLayerList() );

    void setWidth( double width );
    double width() const;

    /** Set data defined width for whole symbol (including all symbol layers).
     * @param dd data defined width
     * @note added in QGIS 2.9
     * @see dataDefinedWidth
     */
    void setDataDefinedWidth( const QgsDataDefined& dd );

    /** Returns data defined size for whole symbol (including all symbol layers).
     * @returns data defined size, or empty data defined if size is not set
     * at the line level
     * @note added in QGIS 2.9
     * @see setDataDefinedWidth
     */
    QgsDataDefined dataDefinedWidth() const;

    void renderPolyline( const QPolygonF& points, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsLineSymbol* clone() const /Factory/;
};



class QgsFillSymbol : QgsSymbol
{
%TypeHeaderCode
#include <qgssymbol.h>
%End

  public:
    /** Create a fill symbol with one symbol layer: SimpleFill with specified properties.
     * This is a convenience method for easier creation of fill symbols.
     */
    static QgsFillSymbol* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsFillSymbol( const QgsSymbolLayerList& layers /Transfer/ = QgsSymbolLayerList() );
    void setAngle( double angle );
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsFillSymbol* clone() const /Factory/;
};
