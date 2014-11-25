/** \ingroup MapComposer
 *  \class QgsComposerMap
 *  \brief Object representing map window.
 */
// NOTE: QgsComposerMapBase must be first, otherwise does not compile
class QgsComposerMap : QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposermap.h>
%End

  public:
    /** Constructor. */
    QgsComposerMap( QgsComposition *composition /TransferThis/, int x, int y, int width, int height );
    /** Constructor. Settings are read from project. */
    QgsComposerMap( QgsComposition *composition /TransferThis/ );
    ~QgsComposerMap();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Preview style  */
    enum PreviewMode
    {
      Cache,       // Use raster cache
      Render,      // Render the map
      Rectangle    // Display only rectangle
    };

    //grid enums are moved to QgsComposerMapGrid
    //TODO - remove for QGIS 3.0
    enum GridStyle
    {
      Solid, //solid lines
      Cross, //only draw line crossings
      Markers,
      FrameAnnotationsOnly
    };

    enum GridAnnotationPosition
    {
      InsideMapFrame,
      OutsideMapFrame,
      Disabled
    };

    enum GridAnnotationDirection
    {
      Horizontal,
      Vertical,
      HorizontalAndVertical,
      BoundaryDirection
    };

    enum GridAnnotationFormat
    {
      Decimal,
      DegreeMinute,
      DegreeMinuteSecond,
      DecimalWithSuffix,
      DegreeMinuteNoSuffix,
      DegreeMinutePadded,
      DegreeMinuteSecondNoSuffix,
      DegreeMinuteSecondPadded
    };

    enum GridFrameStyle
    {
      NoGridFrame,
      Zebra, // black/white pattern
      InteriorTicks,
      ExteriorTicks,
      InteriorExteriorTicks,
      LineBorder
    };

    /**Enum for different frame borders*/
    enum Border
    {
      Left,
      Right,
      Bottom,
      Top
    };

    /** Scaling modes used for the serial rendering (atlas)
     */
    enum AtlasScalingMode
    {
      Fixed,      /*< The current scale of the map is used for each feature of the atlas */
      Predefined, /*< A scale is chosen from the predefined scales. The smallest scale from
                    the list of scales where the atlas feature is fully visible is chosen.
                    @see QgsAtlasComposition::setPredefinedScales.
                    @note This mode is only valid for polygon or line atlas coverage layers
                */
      Auto        /*< The extent is adjusted so that each feature is fully visible.
                    A margin is applied around the center @see setAtlasMargin
                    @note This mode is only valid for polygon or line atlas coverage layers*/
    };

    /** \brief Draw to paint device
        @param painter painter
        @param extent map extent
        @param size size in scene coordinates
        @param dpi scene dpi
        @param forceWidthScale force wysiwyg line widths / marker sizes
    */
    void draw( QPainter *painter, const QgsRectangle& extent, const QSizeF& size, double dpi, double* forceWidthScale = 0 );

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** \brief Create cache image */
    void cache();

    /** Return map settings that would be used for drawing of the map
     *  @note added in 2.6 */
    QgsMapSettings mapSettings( const QgsRectangle& extent, const QSizeF& size, int dpi ) const;

    /** \brief Get identification number*/
    int id() const;

    /**True if a draw is already in progress*/
    bool isDrawing() const;

    /** resizes an item in x- and y direction (canvas coordinates)*/
    void resize( double dx, double dy );

    /**Move content of map
       @param dx move in x-direction (item and canvas coordinates)
       @param dy move in y-direction (item and canvas coordinates)*/
    void moveContent( double dx, double dy );

    /**Zoom content of map
     * @param delta value from wheel event that describes direction (positive /negative number)
     * @param x x-position of mouse cursor (in item coordinates)
     * @param y y-position of mouse cursor (in item coordinates)
     * @deprecated use zoomContent( double, QPointF, ZoomMode ) instead
    */
    void zoomContent( int delta, double x, double y ) /Deprecated/;

    /**Zoom content of item. Does nothing per default (but implemented in composer map)
     * @param factor zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
     * @param point item point for zoom center
     * @param mode zoom mode
     * @note added in QGIS 2.5
    */
    virtual void zoomContent( const double factor, const QPointF point, const ZoomMode mode = QgsComposerItem::Zoom );

    /**Sets new scene rectangle bounds and recalculates hight and extent*/
    void setSceneRect( const QRectF& rectangle );

    /** \brief Scale */
    double scale() const;

    /**Sets new scale and changes only mExtent*/
    void setNewScale( double scaleDenominator, bool forceUpdate = true );

    /**Sets new extent for the map. This method may change the width or height of the map
     * item to ensure that the extent exactly matches the specified extent, with no
     * overlap or margin. This method implicitly alters the map scale.
     * @param extent new extent for the map
     * @see zoomToExtent
    */
    void setNewExtent( const QgsRectangle& extent );

    /**Zooms the map so that the specified extent is fully visible within the map item.
     * This method will not change the width or height of the map, and may result in
     * an overlap or margin from the specified extent. This method implicitly alters the
     * map scale.
     * @param extent new extent for the map
     * @see setNewExtent
     * @note added in QGIS 2.5
    */
    void zoomToExtent( const QgsRectangle& extent );

    /**Sets new Extent for the current atlas preview and changes width, height (and implicitely also scale).
      Atlas preview extents are only temporary, and are regenerated whenever the atlas feature changes
    */
    void setNewAtlasFeatureExtent( const QgsRectangle& extent );

    /**Called when atlas preview is toggled, to force map item to update its extent and redraw
     * @deprecated no longer required
    */
    void toggleAtlasPreview();

    /**Returns a pointer to the current map extent, which is either the original user specified
     * extent or the temporary atlas-driven feature extent depending on the current atlas state
     * of the composition. Both a const and non-const version are included.
     * @returns pointer to current map extent
     * @see visibleExtentPolygon
    */
    // QgsRectangle* currentMapExtent();
    const QgsRectangle* currentMapExtent() const;

    PreviewMode previewMode() const;
    void setPreviewMode( PreviewMode m );

    /**Getter for flag that determines if the stored layer set should be used or the current layer set of the qgis mapcanvas */
    bool keepLayerSet() const;
    /**Setter for flag that determines if the stored layer set should be used or the current layer set of the qgis mapcanvas */
    void setKeepLayerSet( bool enabled );

    /**Getter for stored layer set that is used if mKeepLayerSet is true */
    QStringList layerSet() const;
    /**Setter for stored layer set that is used if mKeepLayerSet is true */
    void setLayerSet( const QStringList& layerSet );
    /**Stores the current layer set of the qgis mapcanvas in mLayerSet*/
    void storeCurrentLayerSet();

    // Set cache outdated
    void setCacheUpdated( bool u = false );

    QgsRectangle extent() const;

    //! @deprecated since 2.4 - use mapSettings() - may return 0 if not initialized with QgsMapRenderer
    const QgsMapRenderer* mapRenderer() const /Deprecated/;

    /**Sets offset values to shift image (useful for live updates when moving item content)*/
    void setOffset( double xOffset, double yOffset );

    /**True if composer map renders a WMS layer*/
    bool containsWMSLayer() const;

    /**True if composer map contains layers with blend modes or flattened layers for vectors */
    bool containsAdvancedEffects() const;

    /** stores state in Dom node
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc Dom document
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to 'ComposerMap' tag
     * @param doc is Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Enables a coordinate grid that is shown on top of this composermap.
     * @deprecated use grid()->setEnabled() or grids() instead
     */
    void setGridEnabled( bool enabled ) /Deprecated/;

    /**
     * @deprecated use grid()->enabled() or grids() instead
     */
    bool gridEnabled() const /Deprecated/;

    /**Sets coordinate grid style to solid or cross
     * @deprecated use grid()->setStyle() or grids() instead
    */
    void setGridStyle( GridStyle style ) /Deprecated/;

    /**
     * @deprecated use grid()->style() or grids() instead
     */
    GridStyle gridStyle() const /Deprecated/;

    /**Sets coordinate interval in x-direction for composergrid.
     * @deprecated use grid()->setIntervalX() or grids() instead
    */
    void setGridIntervalX( double interval ) /Deprecated/;

    /**
     * @deprecated use grid()->intervalX() or grids() instead
     */
    double gridIntervalX() const /Deprecated/;

    /**Sets coordinate interval in y-direction for composergrid.
     * @deprecated use grid()->setIntervalY() or grids() instead
    */
    void setGridIntervalY( double interval ) /Deprecated/;

    /**
     * @deprecated use grid()->intervalY() or grids() instead
     */
    double gridIntervalY() const /Deprecated/;

    /**Sets x-coordinate offset for composer grid
     * @deprecated use grid()->setOffsetX() or grids() instead
    */
    void setGridOffsetX( double offset ) /Deprecated/;

    /**
     * @deprecated use grid()->offsetX() or grids() instead
     */
    double gridOffsetX() const /Deprecated/;

    /**Sets y-coordinate offset for composer grid
     * @deprecated use grid()->setOffsetY() or grids() instead
    */
    void setGridOffsetY( double offset ) /Deprecated/;

    /**
     * @deprecated use grid()->offsetY() or grids() instead
     */
    double gridOffsetY() const /Deprecated/;

    /**Sets the pen to draw composer grid
     * @deprecated use grid()->setPenWidth(), grid()->setPenColor() or grids() instead
    */
    void setGridPen( const QPen& p ) /Deprecated/;

    /**
     * @deprecated use grid()->pen() or grids() instead
     */
    QPen gridPen() const /Deprecated/;

    /**Sets width of grid pen
     * @deprecated use grid()->setPenWidth() or grids() instead
    */
    void setGridPenWidth( double w ) /Deprecated/;

    /**Sets the color of the grid pen
     * @deprecated use grid()->setPenColor() or grids() instead
    */
    void setGridPenColor( const QColor& c ) /Deprecated/;

    /**Sets font for grid annotations
     * @deprecated use grid()->setAnnotationFont() or grids() instead
    */
    void setGridAnnotationFont( const QFont& f ) /Deprecated/;

    /**
     * @deprecated use grid()->annotationFont() or grids() instead
     */
    QFont gridAnnotationFont() const /Deprecated/;

    /**Sets font color for grid annotations
     * @deprecated use grid()->setAnnotationFontColor() or grids() instead
    */
    void setAnnotationFontColor( const QColor& c ) /Deprecated/;

    /**Get font color for grid annotations
     * @deprecated use grid()->annotationFontColor() or grids() instead
    */
    QColor annotationFontColor() const /Deprecated/;

    /**Sets coordinate precision for grid annotations
     * @deprecated use grid()->setAnnotationPrecision or grids() instead
    */
    void setGridAnnotationPrecision( int p ) /Deprecated/;

    /**
     * @deprecated use grid()->annotationPrecision() or grids() instead
     */
    int gridAnnotationPrecision() const /Deprecated/;

    /**Sets flag if grid annotation should be shown
     * @deprecated use grid()->setAnnotationEnabled() or grids() instead
    */
    void setShowGridAnnotation( bool show ) /Deprecated/;

    /**
     * @deprecated use grid()->annotationEnabled() or grids() instead
     */
    bool showGridAnnotation() const /Deprecated/;

    /**
     * @deprecated use grid()->setAnnotationPosition() or grids() instead
     */
    void setGridAnnotationPosition( GridAnnotationPosition p, QgsComposerMap::Border border ) /Deprecated/;

    /**
     * @deprecated use grid()->annotationPosition() or grids() instead
     */
    GridAnnotationPosition gridAnnotationPosition( QgsComposerMap::Border border ) const /Deprecated/;

    /**Sets distance between map frame and annotations
     * @deprecated use grid()->setAnnotationFrameDistance() or grids() instead
    */
    void setAnnotationFrameDistance( double d ) /Deprecated/;

    /**
     * @deprecated use grid()->annotationFrameDistance() or grids() instead
     */
    double annotationFrameDistance() const /Deprecated/;

    /**
     * @deprecated use grid()->setAnnotationDirection() or grids() instead
     */
    void setGridAnnotationDirection( GridAnnotationDirection d, QgsComposerMap::Border border ) /Deprecated/;

    /**
     * @deprecated use grid()->annotationDirection() or grids() instead
     */
    GridAnnotationDirection gridAnnotationDirection( QgsComposerMap::Border border ) const /Deprecated/;

    /**
     * @deprecated use grid()->setAnnotationFormat() or grids() instead
     */
    void setGridAnnotationFormat( GridAnnotationFormat f ) /Deprecated/;

    /**
     * @deprecated use grid()->annotationFormat() or grids() instead
     */
    GridAnnotationFormat gridAnnotationFormat() const /Deprecated/;

    /**Set grid frame style (NoGridFrame or Zebra)
     * @deprecated use grid()->setFrameStyle() or grids() instead
    */
    void setGridFrameStyle( GridFrameStyle style ) /Deprecated/;

    /**
     * @deprecated use grid()->frameStyle() or grids() instead
     */
    GridFrameStyle gridFrameStyle() const /Deprecated/;

    /**Set grid frame width
     * @deprecated use grid()->setFrameWidth() or grids() instead
    */
    void setGridFrameWidth( double w ) /Deprecated/;

    /**
     * @deprecated use grid()->frameWidth() or grids() instead
     */
    double gridFrameWidth() const /Deprecated/;

    /**Set grid frame pen thickness
     * @note: this function was added in version 2.1
     * @deprecated use grid()->setFramePenSize() or grids() instead
    */
    void setGridFramePenSize( double w ) /Deprecated/;

    /**
     * @deprecated use grid()->framePenSize() or grids() instead
     */
    double gridFramePenSize() const /Deprecated/;

    /**Sets pen color for grid frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->setFramePenColor() or grids() instead
    */
    void setGridFramePenColor( const QColor& c ) /Deprecated/;

    /**Get pen color for grid frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->framePenColor() or grids() instead
    */
    QColor gridFramePenColor() const /Deprecated/;

    /**Sets first fill color for grid zebra frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->setFrameFillColor1() or grids() instead
    */
    void setGridFrameFillColor1( const QColor& c ) /Deprecated/;

    /**Get first fill color for grid zebra frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->frameFillColor1() or grids() instead
    */
    QColor gridFrameFillColor1() const /Deprecated/;

    /**Sets second fill color for grid zebra frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->setFrameFillColor2() or grids() instead
    */
    void setGridFrameFillColor2( const QColor& c ) /Deprecated/;

    /**Get second fill color for grid zebra frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->frameFillColor2() or grids() instead
    */
    QColor gridFrameFillColor2() const /Deprecated/;

    /**Sets length of the cross segments (if grid style is cross)
     * @deprecated use grid()->setCrossLength() or grids() instead
    */
    void setCrossLength( double l ) /Deprecated/;

    /**
     * @deprecated use grid()->crossLength() or grids() instead
     */
    double crossLength() /Deprecated/;

    /**
     * @deprecated use grid()->setLineSymbol() or grids() instead
     */
    void setGridLineSymbol( QgsLineSymbolV2* symbol /Transfer/) /Deprecated/;

    /**
     * @deprecated use grid()->lineSymbol() or grids() instead
     */
    QgsLineSymbolV2* gridLineSymbol() /Deprecated/;

    /** Returns the grid's blending mode
     * @deprecated use grid()->blendMode() or grids() instead
     */
    QPainter::CompositionMode gridBlendMode() const /Deprecated/;

    /** Sets the grid's blending mode
     * @deprecated use grid()->setBlendMode() or grids() instead
     */
    void setGridBlendMode( QPainter::CompositionMode blendMode ) /Deprecated/;

    /**Returns the map item's grid stack, which is used to control how grids
     * are drawn over the map's contents.
     * @returns pointer to grid stack
     * @see grid()
     * @note introduced in QGIS 2.5
     */
    QgsComposerMapGridStack* grids();

    /**Returns the map item's first grid. This is a convenience function.
     * @returns pointer to first grid for map item
     * @see grids()
     * @note introduced in QGIS 2.5
     */
    QgsComposerMapGrid* grid();

    /**Returns the map item's overview stack, which is used to control how over
     * are drawn over the map's contents.
     * @returns pointer to overview stack
     * @see overview()
     * @note introduced in QGIS 2.5
     */
    QgsComposerMapOverviewStack* overviews();

    /**Returns the map item's first overview. This is a convenience function.
     * @returns pointer to first overview for map item
     * @see overviews()
     * @note introduced in QGIS 2.5
     */
    QgsComposerMapOverview* overview();

    /**In case of annotations, the bounding rectangle can be larger than the map item rectangle */
    QRectF boundingRect() const;

    /* reimplement setFrameOutlineWidth, so that updateBoundingRect() is called after setting the frame width */
    virtual void setFrameOutlineWidth( const double outlineWidth );

    /**Sets rotation for the map - this does not affect the composer item shape, only the
      way the map is drawn within the item
     * @deprecated Use setMapRotation( double rotation ) instead
     */
    void setRotation( double r ) /Deprecated/;

    /**Returns the rotation used for drawing the map within the composer item
     * @deprecated Use mapRotation() instead
     */
    double rotation() const /Deprecated/;

    /**Sets rotation for the map - this does not affect the composer item shape, only the
      way the map is drawn within the item
      @note this function was added in version 2.1*/
    void setMapRotation( double r );

    /**Returns the rotation used for drawing the map within the composer item
     * @returns rotation for map
     * @param valueType controls whether the returned value is the user specified rotation,
     * or the current evaluated rotation (which may be affected by data driven rotation
     * settings).
    */
    double mapRotation( QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue ) const;

    void updateItem();

    /**Sets canvas pointer (necessary to query and draw map canvas items)*/
    void setMapCanvas( QGraphicsView* canvas /Transfer/ );

    void setDrawCanvasItems( bool b );
    bool drawCanvasItems() const;

    /**Returns the conversion factor map units -> mm*/
    double mapUnitsToMM() const;

    /**Sets overview frame map. -1 disables the overview frame
     * @deprecated use overview()->setFrameMap() or overviews() instead
    */
    void setOverviewFrameMap( int mapId ) /Deprecated/;

    /**Returns id of overview frame (or -1 if no overfiew frame)
     * @deprecated use overview()->frameMapId() or overviews() instead
    */
    int overviewFrameMapId() const /Deprecated/;

    /**
     * @deprecated use overview()->setFrameSymbol() or overviews() instead
    */
    void setOverviewFrameMapSymbol( QgsFillSymbolV2* symbol /Transfer/ ) /Deprecated/;

    /**
     * @deprecated use overview()->frameSymbol() or overviews() instead
    */
    QgsFillSymbolV2* overviewFrameMapSymbol() /Deprecated/;

    /** Returns the overview's blending mode
     * @deprecated use overview()->blendMode() or overviews() instead
    */
    QPainter::CompositionMode overviewBlendMode() const /Deprecated/;

    /** Sets the overview's blending mode
     * @deprecated use overview()->setBlendMode() or overviews() instead
     */
    void setOverviewBlendMode( QPainter::CompositionMode blendMode ) /Deprecated/;

    /** Returns true if the overview frame is inverted
     * @deprecated use overview()->inverted() or overviews() instead
    */
    bool overviewInverted() const /Deprecated/;

    /** Sets the overview's inversion mode
     * @deprecated use overview()->setInverted() or overviews() instead
    */
    void setOverviewInverted( bool inverted ) /Deprecated/;

    /** Returns true if the extent is forced to center on the overview
     * @deprecated use overview()->centered() or overviews() instead
    */
    bool overviewCentered() const;

    /** Set the overview's centering mode
     * @deprecated use overview()->setCentered() or overviews() instead
    */
    void setOverviewCentered( bool centered ) /Deprecated/;

    /**Sets mId to a number not yet used in the composition. mId is kept if it is not in use.
        Usually, this function is called before adding the composer map to the composition*/
    void assignFreeId();

    /**Calculates width and hight of the picture (in mm) such that it fits into the item frame with the given rotation
     * @deprecated Use bool QgsComposerItem::imageSizeConsideringRotation( double& width, double& height, double rotation )
     * instead
     */
    bool imageSizeConsideringRotation( double& width, double& height ) const /Deprecated/;
    /**Calculates corner point after rotation and scaling
     * @deprecated Use QgsComposerItem::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height, double rotation )
     * instead
     */
    bool cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const /Deprecated/;
    /**Calculates width / height of the bounding box of a rotated rectangle
    * @deprecated Use QgsComposerItem::sizeChangedByRotation( double& width, double& height, double rotation )
    * instead
    */
    void sizeChangedByRotation( double& width, double& height ) /Deprecated/;

    /**Returns whether the map extent is set to follow the current atlas feature.
     * @returns true if map will follow the current atlas feature.
     * @see setAtlasDriven
     * @see atlasScalingMode
    */
    bool atlasDriven() const;

    /**Sets whether the map extent will follow the current atlas feature.
     * @param enabled set to true if the map extents should be set by the current atlas feature.
     * @see atlasDriven
     * @see setAtlasScalingMode
    */
    void setAtlasDriven( bool enabled );

    /**Returns true if the map uses a fixed scale when in atlas mode
     * @deprecated since 2.4 Use atlasScalingMode() instead
    */
    bool atlasFixedScale() const /Deprecated/;

    /**Set to true if the map should use a fixed scale when in atlas mode
     * @deprecated since 2.4 Use setAtlasScalingMode() instead
    */
    void setAtlasFixedScale( bool fixed ) /Deprecated/;

    /**Returns the current atlas scaling mode. This controls how the map's extents
     * are calculated for the current atlas feature when an atlas composition
     * is enabled.
     * @returns the current scaling mode
     * @note this parameter is only used if atlasDriven() is true
     * @see setAtlasScalingMode
     * @see atlasDriven
    */
    AtlasScalingMode atlasScalingMode();

    /**Sets the current atlas scaling mode. This controls how the map's extents
     * are calculated for the current atlas feature when an atlas composition
     * is enabled.
     * @param mode atlas scaling mode to set
     * @note this parameter is only used if atlasDriven() is true
     * @see atlasScalingMode
     * @see atlasDriven
    */
    void setAtlasScalingMode( AtlasScalingMode mode );

    /**Returns the margin size (percentage) used when the map is in atlas mode.
     * @param valueType controls whether the returned value is the user specified atlas margin,
     * or the current evaluated atlas margin (which may be affected by data driven atlas margin
     * settings).
     * @returns margin size in percentage to leave around the atlas feature's extent
     * @note this is only used if atlasScalingMode() is Auto.
     * @see atlasScalingMode
     * @see setAtlasMargin
    */
    double atlasMargin( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue );

    /**Sets the margin size (percentage) used when the map is in atlas mode.
     * @param margin size in percentage to leave around the atlas feature's extent
     * @note this is only used if atlasScalingMode() is Auto.
     * @see atlasScalingMode
     * @see atlasMargin
    */
    void setAtlasMargin( double margin );

    /** Sets whether updates to the composer map are enabled. */
    void setUpdatesEnabled( bool enabled );

    /** Returns whether updates to the composer map are enabled. */
    bool updatesEnabled() const;

    /**Get the number of layers that this item requires for exporting as layers
     * @returns 0 if this item is to be placed on the same layer as the previous item,
     * 1 if it should be placed on its own layer, and >1 if it requires multiple export layers
     * @note this method was added in version 2.4
    */
    int numberExportLayers() const;

    /**Returns a polygon representing the current visible map extent, considering map extents and rotation.
     * If the map rotation is 0, the result is the same as currentMapExtent
     * @returns polygon with the four corner points representing the visible map extent. The points are
     * clockwise, starting at the top-left point
     * @see currentMapExtent
    */
    QPolygonF visibleExtentPolygon() const;

    //overriden to show "Map 1" type names
    virtual QString displayName() const;

    /**Returns extent that considers rotation and shift with mOffsetX / mOffsetY*/
    QPolygonF transformedMapPolygon() const;

    /**Transforms map coordinates to item coordinates (considering rotation and move offset)*/
    QPointF mapToItemCoords( const QPointF& mapCoords ) const;

    void connectMapOverviewSignals() /Deprecated/;

    /**Calculates the extent to request and the yShift of the top-left point in case of rotation.
     * @note added in 2.6 */
    void requestedExtent( QgsRectangle& extent ) const;

  signals:
    void extentChanged();

    /**Is emitted on rotation change to notify north arrow pictures*/
    void mapRotationChanged( double newRotation );

    /**Is emitted when the map has been prepared for atlas rendering, just before actual rendering*/
    void preparedForAtlas();

  public slots:

    /**Called if map canvas has changed*/
    void updateCachedImage();
    /**Call updateCachedImage if item is in render mode*/
    void renderModeUpdateCachedImage();

    /**Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle */
    void updateBoundingRect();

    /**@deprecated use QgsComposerMapOverview::overviewExtentChanged instead*/
    void overviewExtentChanged();

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties );
};
