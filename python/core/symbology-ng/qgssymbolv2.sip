
typedef QList<QgsSymbolLayerV2*> QgsSymbolLayerV2List;

class QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

%ConvertToSubClassCode
  switch (sipCpp->type())
  {
    case QgsSymbolV2::Marker: sipType = sipType_QgsMarkerSymbolV2; break;
    case QgsSymbolV2::Line: sipType = sipType_QgsLineSymbolV2; break;
    case QgsSymbolV2::Fill: sipType = sipType_QgsFillSymbolV2; break;
    default: sipType = 0; break;
  }
%End

  public:

    /**
     * The unit of the output
     */
    //TODO QGIS 3.0 - move to QgsUnitTypes and rename to SymbolUnit
    enum OutputUnit
    {
      MM = 0,     //!< The output shall be in millimeters
      MapUnit,    //!< The output shall be in map unitx
      Mixed,      //!< Mixed units in symbol layers
      Pixel,      //!< The output shall be in pixels
      Percentage,  //!< The ouput shall be a percentage of another measurement (eg canvas size, feature size)
    };

    typedef QList<QgsSymbolV2::OutputUnit> OutputUnitList;

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

    enum RenderHint
    {
      DataDefinedSizeScale,
      DataDefinedRotation
    };

    virtual ~QgsSymbolV2();

    //! return new default symbol for specified geometry type
    static QgsSymbolV2* defaultSymbol( QGis::GeometryType geomType ) /Factory/;

    SymbolType type() const;

    // symbol layers handling

    /** Returns list of symbol layers contained in the symbol.
     * @returns symbol layers list
     * @note added in QGIS 2.7
     * @see symbolLayer
     * @see symbolLayerCount
     */
    QgsSymbolLayerV2List symbolLayers();

    /** Returns a specific symbol layers contained in the symbol.
     * @param layer layer number
     * @returns corresponding symbol layer
     * @note added in QGIS 2.7
     * @see symbolLayers
     * @see symbolLayerCount
     */
    QgsSymbolLayerV2* symbolLayer( int layer );

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
    bool insertSymbolLayer( int index, QgsSymbolLayerV2* layer /Transfer/ );

    /**
     * Append symbol layer at the end of the list
     * Ownership will be transferred.
     * @param layer The layer to add
     * @return True if the layer is added, False if the layer is bad
     */
    bool appendSymbolLayer( QgsSymbolLayerV2* layer /Transfer/ );

    //! delete symbol layer at specified index
    bool deleteSymbolLayer( int index );

    /**
     * Remove symbol layer from the list and return pointer to it.
     * Ownership is handed to the caller.
     * @param index The index of the layer to remove
     * @return A pointer to the removed layer
     */
    QgsSymbolLayerV2* takeSymbolLayer( int index ) /TransferBack/;

    //! delete layer at specified index and set a new one
    bool changeSymbolLayer( int index, QgsSymbolLayerV2 *layer /Transfer/ );

    void startRender( QgsRenderContext& context, const QgsFields* fields = 0 );
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

    virtual QgsSymbolV2* clone() const = 0 /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    OutputUnit outputUnit() const;
    void setOutputUnit( OutputUnit u );

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale& scale );

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const;
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha );

    void setRenderHints( int hints );
    int renderHints() const;

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
     * Render a feature.
     */
    void renderFeature( const QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false, int currentVertexMarkerType = 0, int currentVertexMarkerSize = 0 );

    /**
     * Returns the symbol render context. Only valid between startRender and stopRender calls.
     *
     * @return The symbol render context
     */
    QgsSymbolV2RenderContext* symbolRenderContext();

  protected:
    QgsSymbolV2( SymbolType type, const QgsSymbolLayerV2List& layers /Transfer/ ); // can't be instantiated

    /**
     * Creates a point in screen coordinates from a QgsPointV2 in map coordinates
     */
    static void _getPoint( QPointF& pt /Out/, QgsRenderContext& context, const QgsPointV2* point );

    /**
     * Creates a point in screen coordinates from a wkb string in map
     * coordinates
     */
    static QgsConstWkbPtr _getPoint( QPointF& pt, QgsRenderContext& context, QgsConstWkbPtr& wkb );

    /**
     * Creates a line string in screen coordinates from a wkb string in map
     * coordinates
     */
    static QgsConstWkbPtr _getLineString( QPolygonF& pts, QgsRenderContext& context, QgsConstWkbPtr& wkb, bool clipToExtent = true );

    /**
     * Creates a polygon in screen coordinates from a wkb string in map
     * coordinates
     */
    static QgsConstWkbPtr _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, QgsConstWkbPtr& wkb, bool clipToExtent = true );

    /**
     * Retrieve a cloned list of all layers that make up this symbol.
     * Ownership is transferred to the caller.
     */
    QgsSymbolLayerV2List cloneLayers() const /Factory/;

    /**
     * Renders a context using a particular symbol layer without passing in a
     * geometry. This is used as fallback, if the symbol being rendered is not
     * compatible with the specified layer. In such a case, this method can be
     * called and will call the layer's rendering method anyway but the
     * geometry passed to the layer will be empty.
     * This is required for layers that generate their own geometry from other
     * information in the rendering context.
     */
    void renderUsingLayer( QgsSymbolLayerV2* layer, QgsSymbolV2RenderContext& context );

    //! check whether a symbol layer type can be used within the symbol
    //! (marker-marker, line-line, fill-fill/line)
    //! @deprecated since 2.14, use QgsSymbolLayerV2::isCompatibleWithSymbol instead
    bool isSymbolLayerCompatible( SymbolType layerType );

    //! Render editing vertex marker at specified point
    //! @note added in QGIS 2.16
    void renderVertexMarker( QPointF pt, QgsRenderContext& context, int currentVertexMarkerType, int currentVertexMarkerSize );

  private:
    QgsSymbolV2( const QgsSymbolV2& );
};

///////////////////////

class QgsSymbolV2RenderContext
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

  public:
    QgsSymbolV2RenderContext( QgsRenderContext& c, QgsSymbolV2::OutputUnit u, qreal alpha = 1.0, bool selected = false, int renderHints = 0, const QgsFeature* f = 0, const QgsFields* fields = 0, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() );
    ~QgsSymbolV2RenderContext();

    QgsRenderContext& renderContext();
    // const QgsRenderContext& renderContext() const;

    /** Sets the original value variable value for data defined symbology
     * @param value value for original value variable. This usually represents the symbol property value
     * before any data defined overrides have been applied.
     * @note added in QGIS 2.12
     */
    void setOriginalValueVariable( const QVariant& value );

    QgsSymbolV2::OutputUnit outputUnit() const;
    void setOutputUnit( QgsSymbolV2::OutputUnit u );

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale& scale );

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const;
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha );

    bool selected() const;
    void setSelected( bool selected );

    int renderHints() const;
    void setRenderHints( int hints );

    void setFeature( const QgsFeature* f );
    //! Current feature being rendered - may be null
    const QgsFeature* feature() const;

    //! Fields of the layer. Currently only available in startRender() calls
    //! to allow symbols with data-defined properties prepare the expressions
    //! (other times fields() returns null)
    //! @note added in 2.4
    const QgsFields* fields() const;

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
    // QgsSymbolV2RenderContext& operator=( const QgsSymbolV2RenderContext& );

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

    QgsSymbolV2RenderContext( const QgsSymbolV2RenderContext& rh );
};



//////////////////////



class QgsMarkerSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

  public:
    /** Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
      This is a convenience method for easier creation of marker symbols.
     */
    static QgsMarkerSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsMarkerSymbolV2( const QgsSymbolLayerV2List& layers /Transfer/ = QgsSymbolLayerV2List() );

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
    void setSizeUnit( OutputUnit unit );

    /** Returns the size units for the whole symbol (including all symbol layers).
     * @returns size units, or mixed units if symbol layers have different units
     * @note added in QGIS 2.16
     * @see setSizeUnit()
     * @see sizeMapUnitScale()
     * @see size()
     */
    OutputUnit sizeUnit() const;

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

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
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

    virtual QgsMarkerSymbolV2* clone() const /Factory/;
};



class QgsLineSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

  public:
    /** Create a line symbol with one symbol layer: SimpleLine with specified properties.
     * This is a convenience method for easier creation of line symbols.
     */
    static QgsLineSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsLineSymbolV2( const QgsSymbolLayerV2List& layers /Transfer/ = QgsSymbolLayerV2List() );

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

    virtual QgsLineSymbolV2* clone() const /Factory/;
};



class QgsFillSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

  public:
    /** Create a fill symbol with one symbol layer: SimpleFill with specified properties.
     * This is a convenience method for easier creation of fill symbols.
     */
    static QgsFillSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsFillSymbolV2( const QgsSymbolLayerV2List& layers /Transfer/ = QgsSymbolLayerV2List() );
    void setAngle( double angle );
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsFillSymbolV2* clone() const /Factory/;
};
