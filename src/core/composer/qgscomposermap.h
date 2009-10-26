/***************************************************************************
                         qgscomposermap.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERMAP_H
#define QGSCOMPOSERMAP_H

//#include "ui_qgscomposermapbase.h"
#include "qgscomposeritem.h"
#include "qgsrectangle.h"
#include <QGraphicsRectItem>
#include <QObject>

class QgsComposition;
class QgsMapRenderer;
class QgsMapToPixel;
class QDomNode;
class QDomDocument;
class QPainter;

/** \ingroup MapComposer
 *  \class QgsComposerMap
 *  \brief Object representing map window.
 */
// NOTE: QgsComposerMapBase must be first, otherwise does not compile
class CORE_EXPORT QgsComposerMap : /*public QWidget, private Ui::QgsComposerMapBase,*/ public QObject, public QgsComposerItem
{
    Q_OBJECT

  public:
    /** Constructor. */
    QgsComposerMap( QgsComposition *composition, int x, int y, int width, int height );
    /** Constructor. Settings are read from project. */
    QgsComposerMap( QgsComposition *composition );
    ~QgsComposerMap();

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
      OutsideMapFrame
    };

    enum GridAnnotationDirection
    {
      Horizontal = 0,
      Vertical,
      HorizontalAndVertical,
      BoundaryDirection
    };

    /** \brief Draw to paint device
    @param extent map extent
    @param size size in scene coordinates
    @param dpi scene dpi*/
    void draw( QPainter *painter, const QgsRectangle& extent, const QSize& size, int dpi );

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** \brief Create cache image */
    void cache( void );

    /** \brief Get identification number*/
    int id() const {return mId;}

    /**True if a draw is already in progress*/
    bool isDrawing() const {return mDrawing;}

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
    double scale( void ) const;

    /**Sets new scale and changes only mExtent*/
    void setNewScale( double scaleDenominator );

    /**Sets new Extent and changes width, height (and implicitely also scale)*/
    void setNewExtent( const QgsRectangle& extent );

    PreviewMode previewMode() {return mPreviewMode;}
    void setPreviewMode( PreviewMode m ) {mPreviewMode = m;}

    /**Getter for flag that determines if the stored layer set should be used or the current layer set of the qgis mapcanvas
    @note this function was added in version 1.2*/
    bool keepLayerSet() const {return mKeepLayerSet;}
    /**Setter for flag that determines if the stored layer set should be used or the current layer set of the qgis mapcanvas
    @note this function was added in version 1.2*/
    void setKeepLayerSet( bool enabled ) {mKeepLayerSet = enabled;}

    /**Getter for stored layer set that is used if mKeepLayerSet is true
    @note this function was added in version 1.2*/
    QStringList layerSet() const {return mLayerSet;}
    /**Setter for stored layer set that is used if mKeepLayerSet is true
    @note this function was added in version 1.2*/
    void setLayerSet( const QStringList& layerSet ) {mLayerSet = layerSet;}
    /**Stores the current layer set of the qgis mapcanvas in mLayerSet*/
    void storeCurrentLayerSet();

    // Set cache outdated
    void setCacheUpdated( bool u = false );

    QgsRectangle extent() const {return mExtent;}

    const QgsMapRenderer* mapRenderer() const {return mMapRenderer;}

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
    void setGridEnabled( bool enabled ) {mGridEnabled = enabled;}
    bool gridEnabled() const { return mGridEnabled; }

    /**Sets coordinate grid style to solid or cross
        @note this function was added in version 1.4*/
    void setGridStyle( GridStyle style ) {mGridStyle = style;}
    GridStyle gridStyle() const { return mGridStyle; }

    /**Sets coordinate interval in x-direction for composergrid.
        @note this function was added in version 1.4*/
    void setGridIntervalX( double interval ) { mGridIntervalX = interval;}
    double gridIntervalX() const { return mGridIntervalX; }

    /**Sets coordinate interval in y-direction for composergrid.
    @note this function was added in version 1.4*/
    void setGridIntervalY( double interval ) { mGridIntervalY = interval;}
    double gridIntervalY() const { return mGridIntervalY; }

    /**Sets x-coordinate offset for composer grid
    @note this function was added in version 1.4*/
    void setGridOffsetX( double offset ) { mGridOffsetX = offset; }
    double gridOffsetX() const { return mGridOffsetX; }

    /**Sets y-coordinate offset for composer grid
    @note this function was added in version 1.4*/
    void setGridOffsetY( double offset ) { mGridOffsetY = offset; }
    double gridOffsetY() const { return mGridOffsetY; }

    /**Sets the pen to draw composer grid
    @note this function was added in version 1.4*/
    void setGridPen( const QPen& p ) { mGridPen = p; }
    QPen gridPen() const { return mGridPen; }
    /**Sets with of grid pen
    @note this function was added in version 1.4*/
    void setGridPenWidth( double w );
    /**Sets the color of the grid pen
    @note this function was added in version 1.4*/
    void setGridPenColor( const QColor& c );

    /**Sets font for grid annotations
    @note this function was added in version 1.4*/
    void setGridAnnotationFont( const QFont& f ) { mGridAnnotationFont = f; }
    QFont gridAnnotationFont() const { return mGridAnnotationFont; }

    /**Sets coordinate precision for grid annotations
    @note this function was added in version 1.4*/
    void setGridAnnotationPrecision( int p ) {mGridAnnotationPrecision = p;}
    int gridAnnotationPrecision() const {return mGridAnnotationPrecision;}

    /**Sets flag if grid annotation should be shown
    @note this function was added in version 1.4*/
    void setShowGridAnnotation( bool show ) {mShowGridAnnotation = show;}
    bool showGridAnnotation() const {return mShowGridAnnotation;}

    /**Sets position of grid annotations. Possibilities are inside or outside of the map frame
    @note this function was added in version 1.4*/
    void setGridAnnotationPosition( GridAnnotationPosition p ) {mGridAnnotationPosition = p;}
    GridAnnotationPosition gridAnnotationPosition() const {return mGridAnnotationPosition;}

    /**Sets distance between map frame and annotations
    @note this function was added in version 1.4*/
    void setAnnotationFrameDistance( double d ) {mAnnotationFrameDistance = d;}
    double annotationFrameDistance() const {return mAnnotationFrameDistance;}

    /**Sets grid annotation direction. Can be horizontal, vertical, direction of axis and horizontal and vertical
    @note this function was added in version 1.4*/
    void setGridAnnotationDirection( GridAnnotationDirection d ) {mGridAnnotationDirection = d;}
    GridAnnotationDirection gridAnnotationDirection() const {return mGridAnnotationDirection;}

    /**In case of annotations, the bounding rectangle can be larger than the map item rectangle
    @note this function was added in version 1.4*/
    QRectF boundingRect() const;
    /**Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle
    @note this function was added in version 1.4*/
    void updateBoundingRect();

    /**Sets the rotation of the map content
    @note this function was added in version 1.4*/
    void setRotation( double r ) { mRotation = r; }
    double rotation() const { return mRotation; }

    /**Sets length of the cros segments (if grid style is cross)
    @note this function was added in version 1.4*/
    void setCrossLength( double l ) {mCrossLength = l;}
    double crossLength() {return mCrossLength;}

  public slots:

    /**Called if map canvas has changed*/
    void updateCachedImage( );
    /**Call updateCachedImage if item is in render mode*/
    void renderModeUpdateCachedImage();

  signals:
    /**Is emitted when width/height is changed as a result of user interaction*/
    void extentChanged();

  private:

    /**Enum for different frame borders*/
    enum Border
    {
      Left,
      Right,
      Bottom,
      Top
    };

    // Pointer to map renderer of the QGIS main map. Note that QgsComposerMap uses a different map renderer,
    //it just copies some properties from the main map renderer.
    QgsMapRenderer *mMapRenderer;

    /**Unique identifier*/
    int mId;

    // Map region in map units realy used for rendering
    // It can be the same as mUserExtent, but it can be bigger in on dimension if mCalculate==Scale,
    // so that full rectangle in paper is used.
    QgsRectangle mExtent;

    // Cache used in composer preview
    QImage mCacheImage;

    // Is cache up to date
    bool mCacheUpdated;

    /** \brief Preview style  */
    PreviewMode mPreviewMode;

    /** \brief Number of layers when cache was created  */
    int mNumCachedLayers;

    /** \brief set to true if in state of drawing. Concurrent requests to draw method are returned if set to true */
    bool mDrawing;

    /**Offset in x direction for showing map cache image*/
    double mXOffset;
    /**Offset in y direction for showing map cache image*/
    double mYOffset;

    /**Flag if layers to be displayed should be read from qgis canvas (true) or from stored list in mLayerSet (false)*/
    bool mKeepLayerSet;

    /**Stored layer list (used if layer live-link mKeepLayerSet is disabled)*/
    QStringList mLayerSet;

    /**For the generation of new unique ids*/
    static int mCurrentComposerId;

    /**Establishes signal/slot connection for update in case of layer change*/
    void connectUpdateSlot();

    /**Removes layer ids from mLayerSet that are no longer present in the qgis main map*/
    void syncLayerSet();

    /**True if coordinate grid has to be displayed*/
    bool mGridEnabled;
    /**Solid or crosses*/
    GridStyle mGridStyle;
    /**Grid line interval in x-direction (map units)*/
    double mGridIntervalX;
    /**Grid line interval in y-direction (map units)*/
    double mGridIntervalY;
    /**Grid line offset in x-direction*/
    double mGridOffsetX;
    /**Grid line offset in y-direction*/
    double mGridOffsetY;
    /**Grid line pen*/
    QPen mGridPen;
    /**Font for grid line annotation*/
    QFont mGridAnnotationFont;
    /**Digits after the dot*/
    int mGridAnnotationPrecision;
    /**True if coordinate values should be drawn*/
    bool mShowGridAnnotation;
    /**Annotation position inside or outside of map frame*/
    GridAnnotationPosition mGridAnnotationPosition;
    /**Distance between map frame and annotation*/
    double mAnnotationFrameDistance;
    /**Annotation can be horizontal / vertical or different for axes*/
    GridAnnotationDirection mGridAnnotationDirection;
    /**Current bounding rectangle. This is used to check if notification to the graphics scene is necessary*/
    QRectF mCurrentRectangle;

    /**Rotation of the map. Clockwise in degrees, north direction is 0*/
    double mRotation;
    /**The length of the cross sides for mGridStyle Cross*/
    double mCrossLength;

    /**Draws the map grid*/
    void drawGrid( QPainter* p );
    /**Draw coordinates for mGridAnnotationType Coordinate
        @param lines the coordinate lines in item coordinates*/
    void drawCoordinateAnnotations( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines );
    void drawCoordinateAnnotation( QPainter* p, const QPointF& pos, QString annotationString );
    /**Draws a single annotation
        @param p drawing painter
        @param pos item coordinates where to draw
        @param rotation text rotation
        @param the text to draw*/
    void drawAnnotation( QPainter* p, const QPointF& pos, int rotation, const QString& annotationText );
    /**Returns the grid lines with associated coordinate value
        @return 0 in case of success*/
    int xGridLines( QList< QPair< double, QLineF > >& lines ) const;
    /**Returns the grid lines for the y-coordinates. Not vertical in case of rotation
        @return 0 in case of success*/
    int yGridLines( QList< QPair< double, QLineF > >& lines ) const;
    /**Returns extent that considers mOffsetX / mOffsetY (during content move)*/
    QgsRectangle transformedExtent() const;
    /**Returns extent that considers rotation and shift with mOffsetX / mOffsetY*/
    QPolygonF transformedMapPolygon() const;
    double maxExtension() const;
    /**Returns the polygon of the map extent. If rotation == 0, the result is the same as mExtent
    @param poly out: the result polygon with the four corner points. The points are clockwise, starting at the top-left point
    @return true in case of success*/
    void mapPolygon( QPolygonF& poly ) const;
    /**Calculates the extent to request and the yShift of the top-left point in case of rotation.*/
    void requestedExtent( QgsRectangle& extent ) const;
    /**Returns the conversion factor map units -> mm*/
    double mapUnitsToMM() const;
    /**Scales a composer map shift (in MM) and rotates it by mRotation
        @param xShift in: shift in x direction (in item units), out: xShift in map units
        @param yShift in: shift in y direction (in item units), out: yShift in map units*/
    void transformShift( double& xShift, double& yShift ) const;
    /**Transforms map coordinates to item coordinates (considering rotation and move offset)*/
    QPointF mapToItemCoords( const QPointF& mapCoords ) const;
    /**Returns the item border of a point (in item coordinates)*/
    Border borderForLineCoord( const QPointF& p ) const;
    /**Rotates a point / vector
        @param angle rotation angle in degrees, counterclockwise
        @param x in/out: x coordinate before / after the rotation
        @param y in/out: y cooreinate before / after the rotation*/
    void rotate( double angle, double& x, double& y ) const;
    /**Returns a point on the line from startPoint to directionPoint that is a certain distance away from the starting point*/
    QPointF pointOnLineWithDistance( const QPointF& startPoint, const QPointF& directionPoint, double distance ) const;
};

#endif
