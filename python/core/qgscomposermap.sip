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
      Cache = 0,   // Use raster cache
      Render,      // Render the map
      Rectangle    // Display only rectangle
    };

    enum GridStyle
    {
      Solid = 0, //solid lines
      Cross //only draw line crossings
    };

    enum GridAnnotationPosition
    {
      InsideMapFrame = 0,
      OutsideMapFrame,
      Disabled
    };

    enum GridAnnotationDirection
    {
      Horizontal = 0,
      Vertical,
      HorizontalAndVertical,
      BoundaryDirection
    };

    /**Enum for different frame borders*/
    enum Border
    {
      Left,
      Right,
      Bottom,
      Top
    };

    /**@deprecated*/
    void draw( QPainter *painter, const QgsRectangle& extent, const QSize& size, int dpi );

    /** \brief Draw to paint device
    @param extent map extent
    @param size size in scene coordinates
    @param dpi scene dpi*/
    void draw( QPainter *painter, const QgsRectangle& extent, const QSizeF& size, double dpi );

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
    void setNewScale( double scaleDenominator );

    /**Sets new Extent and changes width, height (and implicitely also scale)*/
    void setNewExtent( const QgsRectangle& extent );

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

    const QgsMapRenderer* mapRenderer() const;

    /**Sets offset values to shift image (useful for live updates when moving item content)*/
    void setOffset( double xOffset, double yOffset );

    /**True if composer map renders a WMS layer*/
    bool containsWMSLayer() const;

    /** stores state in Dom node
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param temp write template file
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to 'ComposerMap' tag
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

     /**Sets coordinate precision for grid annotations
    @note this function was added in version 1.4*/
    void setGridAnnotationPrecision( int p );
    int gridAnnotationPrecision() const;

    /**Sets flag if grid annotation should be shown
    @note this function was added in version 1.4*/
    void setShowGridAnnotation( bool show );
    bool showGridAnnotation() const;

    /**Sets position of grid annotations. Possibilities are inside / outside of the map frame or disabled
    @note this function was added in version 1.9*/
    void setGridAnnotationPosition( GridAnnotationPosition p, QgsComposerMap::Border border );
    /**@note: this function was added in version 1.9*/
    GridAnnotationPosition gridAnnotationPosition( QgsComposerMap::Border border ) const;

    /**Sets distance between map frame and annotations
    @note this function was added in version 1.4*/
    void setAnnotationFrameDistance( double d );
    double annotationFrameDistance() const;

    /**Sets grid annotation direction. Can be horizontal or vertical
    @note this function was added in version 1.9*/
    void setGridAnnotationDirection( GridAnnotationDirection d, QgsComposerMap::Border border );
    /**@note: this function was added in version 1.9*/
    GridAnnotationDirection gridAnnotationDirection( QgsComposerMap::Border border ) const;

    /**In case of annotations, the bounding rectangle can be larger than the map item rectangle
    @note this function was added in version 1.4*/
    QRectF boundingRect() const;
    /**Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle
    @note this function was added in version 1.4*/
    void updateBoundingRect();

    /**Sets length of the cros segments (if grid style is cross)
    @note this function was added in version 1.4*/
    void setCrossLength(double l);
    double crossLength();

    void setMapRotation( double r );

    void updateItem();

  public slots:

    /**Called if map canvas has changed*/
    void updateCachedImage( );
    /**Call updateCachedImage if item is in render mode*/
    void renderModeUpdateCachedImage();

  signals:
    /**Is emitted when width/height is changed as a result of user interaction*/
    void extentChanged();
};
