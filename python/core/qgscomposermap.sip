/** \ingroup MapComposer
 *  \class QgsComposerMap
 *  \brief Object representing map window.
 */
// NOTE: QgsComposerMapBase must be first, otherwise does not compile
class QgsComposerMap : QObject, QgsComposerItem
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

    /** \brief Preview style  */
    enum PreviewMode
    {
      Cache = 0,   // Use raster cache
      Render,      // Render the map
      Rectangle    // Display only rectangle
    };

    /** \brief Draw to paint device
    @param extent map extent
    @param size size in scene coordinates
    @param dpi scene dpi*/
    void draw( QPainter *painter, const QgsRectangle& extent, const QSize& size, int dpi );

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

    PreviewMode previewMode();
    void setPreviewMode( PreviewMode m );

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

  public slots:

    /**Called if map canvas has changed*/
    void updateCachedImage( );
    /**Call updateCachedImage if item is in render mode*/
    void renderModeUpdateCachedImage();

  signals:
    /**Is emitted when width/height is changed as a result of user interaction*/
    void extentChanged();

  private:

    /**Establishes signal/slot connection for update in case of layer change*/
    void connectUpdateSlot();

    /**Returns the zoom factor of the graphics view. If no
     graphics view exists, the default 1 is returned*/
    double horizontalViewScaleFactor() const;
};
