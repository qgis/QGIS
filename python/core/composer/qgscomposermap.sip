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

    enum GridStyle
    {
      Solid, //solid lines
      Cross //only draw line crossings
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
      DegreeMinuteSecond
    };

    enum GridFrameStyle
    {
      NoGridFrame,
      Zebra // black/white pattern
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
     @param delta value from wheel event that describes magnitude and direction (positive /negative number)
    @param x x-coordinate of mouse position in item coordinates
    @param y y-coordinate of mouse position in item coordinates*/
    void zoomContent( int delta, double x, double y );

    /**Sets new scene rectangle bounds and recalculates hight and extent*/
    void setSceneRect( const QRectF& rectangle );

    /** \brief Scale */
    double scale() const;

    /**Sets new scale and changes only mExtent*/
    void setNewScale( double scaleDenominator, bool forceUpdate = true );

    /**Sets new Extent and changes width, height (and implicitely also scale)*/
    void setNewExtent( const QgsRectangle& extent );

    /**Sets new Extent for the current atlas preview and changes width, height (and implicitely also scale).
      Atlas preview extents are only temporary, and are regenerated whenever the atlas feature changes
    */
    void setNewAtlasFeatureExtent( const QgsRectangle& extent );

    /**Called when atlas preview is toggled, to force map item to update its extent and redraw*/
    void toggleAtlasPreview();

    /**Returns a pointer to the current map extent, which is either the original user specified
     * extent or the temporary atlas-driven feature extent depending on the current atlas state
     * of the composition. Both a const and non-const version are included.
     * @returns pointer to current map extent
     * @see visibleExtentPolygon
    */
    const QgsRectangle* currentMapExtent() const;

    PreviewMode previewMode() const;
    void setPreviewMode( PreviewMode m );

    /**Getter for flag that determines if the stored layer set should be used or the current layer set of the qgis mapcanvas
    @note this function was added in version 1.2*/
    bool keepLayerSet() const;
    /**Setter for flag that determines if the stored layer set should be used or the current layer set of the qgis mapcanvas
    @note this function was added in version 1.2*/
    void setKeepLayerSet( bool enabled );

    /**Getter for stored layer set that is used if mKeepLayerSet is true
    @note this function was added in version 1.2*/
    QStringList layerSet() const;
    /**Setter for stored layer set that is used if mKeepLayerSet is true
    @note this function was added in version 1.2*/
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
        @note this function was added in version 1.4*/
    void setGridEnabled( bool enabled );
    bool gridEnabled() const;

    /**Sets coordinate grid style to solid or cross
        @note this function was added in version 1.4*/
    void setGridStyle( GridStyle style );
    GridStyle gridStyle() const;

    /**Sets coordinate interval in x-direction for composergrid.
        @note this function was added in version 1.4*/
    void setGridIntervalX( double interval );
    double gridIntervalX() const;

    /**Sets coordinate interval in y-direction for composergrid.
    @note this function was added in version 1.4*/
    void setGridIntervalY( double interval );
    double gridIntervalY() const;

    /**Sets x-coordinate offset for composer grid
    @note this function was added in version 1.4*/
    void setGridOffsetX( double offset );
    double gridOffsetX() const;

    /**Sets y-coordinate offset for composer grid
    @note this function was added in version 1.4*/
    void setGridOffsetY( double offset );
    double gridOffsetY() const;

    /**Sets the pen to draw composer grid
    @note this function was added in version 1.4*/
    void setGridPen( const QPen& p );
    QPen gridPen() const;

    /**Sets with of grid pen
    @note this function was added in version 1.4*/
    void setGridPenWidth( double w );

    /**Sets the color of the grid pen
    @note this function was added in version 1.4*/
    void setGridPenColor( const QColor& c );

    /**Sets font for grid annotations
    @note this function was added in version 1.4*/
    void setGridAnnotationFont( const QFont& f );
    QFont gridAnnotationFont() const;

    /**Sets font color for grid annotations
        @note this function was added in version 2.0*/
    void setAnnotationFontColor( const QColor& c );
    /**Get font color for grid annotations
        @note: this function was added in version 2.0*/
    QColor annotationFontColor() const;

    /**Sets coordinate precision for grid annotations
    @note this function was added in version 1.4*/
    void setGridAnnotationPrecision( int p );
    int gridAnnotationPrecision() const;

    /**Sets flag if grid annotation should be shown
    @note this function was added in version 1.4*/
    void setShowGridAnnotation( bool show );
    bool showGridAnnotation() const;

    void setGridAnnotationPosition( GridAnnotationPosition p, QgsComposerMap::Border border );
    GridAnnotationPosition gridAnnotationPosition( QgsComposerMap::Border border ) const;

    /**Sets distance between map frame and annotations
    @note this function was added in version 1.4*/
    void setAnnotationFrameDistance( double d );
    double annotationFrameDistance() const;

    void setGridAnnotationDirection( GridAnnotationDirection d, QgsComposerMap::Border border );
    GridAnnotationDirection gridAnnotationDirection( QgsComposerMap::Border border ) const;

    void setGridAnnotationFormat( GridAnnotationFormat f );
    GridAnnotationFormat gridAnnotationFormat() const;

    /**Set grid frame style (NoGridFrame or Zebra)
        @note: this function was added in version 1.9*/
    void setGridFrameStyle( GridFrameStyle style );
    GridFrameStyle gridFrameStyle() const;

    /**Set grid frame width
        @note: this function was added in version 1.9*/
    void setGridFrameWidth( double w );
    double gridFrameWidth() const;

    /**Set grid frame pen thickness
        @note: this function was added in version 2.1*/
    void setGridFramePenSize( double w );
    double gridFramePenSize() const;

    /**Sets pen color for grid frame
        @note: this function was added in version 2.1*/
    void setGridFramePenColor( const QColor& c );
    /**Get pen color for grid frame
        @note: this function was added in version 2.1*/
    QColor gridFramePenColor() const;

    /**Sets first fill color for grid zebra frame
        @note: this function was added in version 2.1*/
    void setGridFrameFillColor1( const QColor& c );
    /**Get first fill color for grid zebra frame
        @note: this function was added in version 2.1*/
    QColor gridFrameFillColor1() const;

    /**Sets second fill color for grid zebra frame
        @note: this function was added in version 2.1*/
    void setGridFrameFillColor2( const QColor& c );
    /**Get second fill color for grid zebra frame
        @note: this function was added in version 2.1*/
    QColor gridFrameFillColor2() const;

    /**In case of annotations, the bounding rectangle can be larger than the map item rectangle
    @note this function was added in version 1.4*/
    QRectF boundingRect() const;
    /**Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle
    @note this function was added in version 1.4*/
    void updateBoundingRect();

    /* reimplement setFrameOutlineWidth, so that updateBoundingRect() is called after setting the frame width */
    virtual void setFrameOutlineWidth( const double outlineWidth );

    /**Sets length of the cros segments (if grid style is cross)
    @note this function was added in version 1.4*/
    void setCrossLength( double l );
    double crossLength();

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
    @note: this function was added in version 1.9*/
    void setOverviewFrameMap( int mapId );
    /**Returns id of overview frame (or -1 if no overfiew frame)
    @note: this function was added in version 1.9*/
    int overviewFrameMapId() const;

    void setOverviewFrameMapSymbol( QgsFillSymbolV2* symbol /Transfer/ );
    QgsFillSymbolV2* overviewFrameMapSymbol();

    /** Returns the overview's blending mode */
    QPainter::CompositionMode overviewBlendMode() const;
    /** Sets the overview's blending mode*/
    void setOverviewBlendMode( QPainter::CompositionMode blendMode );

    /** Returns true if the overview frame is inverted */
    bool overviewInverted() const;
    /** Sets the overview's inversion mode*/
    void setOverviewInverted( bool inverted );

    /** Returns true if the extent is forced to center on the overview */
    bool overviewCentered() const;
    /** Set the overview's centering mode */
    void setOverviewCentered( bool centered );

    void setGridLineSymbol( QgsLineSymbolV2* symbol /Transfer/ );
    QgsLineSymbolV2* gridLineSymbol();

    /** Returns the grid's blending mode */
    QPainter::CompositionMode gridBlendMode() const;
    /** Sets the grid's blending mode*/
    void setGridBlendMode( QPainter::CompositionMode blendMode );

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
     * @returns margin size in percentage to leave around the atlas feature's extent
     * @note this is only used if atlasScalingMode() is Auto.
     * @see atlasScalingMode
     * @see setAtlasMargin
    */
    double atlasMargin() const;

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

  signals:
    void extentChanged();

    /**Is emitted on rotation change to notify north arrow pictures*/
    void mapRotationChanged( double newRotation );

    /**Is emitted when the map has been prepared for atlas rendering, just before actual rendering*/
    void preparedForAtlas();

  public slots:

    /**Called if map canvas has changed*/
    void updateCachedImage( );
    /**Call updateCachedImage if item is in render mode*/
    void renderModeUpdateCachedImage();

    void overviewExtentChanged();

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties );
};
