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
      HorizontalAndVertical
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

    void setGridEnabled( bool enabled ) {mGridEnabled = enabled;}
    bool gridEnabled() const { return mGridEnabled; }

    void setGridStyle( GridStyle style ) {mGridStyle = style;}
    GridStyle gridStyle() const { return mGridStyle; }

    void setGridIntervalX( double interval ) { mGridIntervalX = interval;}
    double gridIntervalX() const { return mGridIntervalX; }

    void setGridIntervalY( double interval ) { mGridIntervalY = interval;}
    double gridIntervalY() const { return mGridIntervalY; }

    void setGridOffsetX( double offset ) { mGridOffsetX = offset; }
    double gridOffsetX() const { return mGridOffsetX; }

    void setGridOffsetY( double offset ) { mGridOffsetY = offset; }
    double gridOffsetY() const { return mGridOffsetY; }

    void setGridPen( const QPen& p ) { mGridPen = p; }
    QPen gridPen() const { return mGridPen; }
    void setGridPenWidth( double w );
    void setGridPenColor( const QColor& c );

    void setGridAnnotationFont( const QFont& f ) { mGridAnnotationFont = f; }
    QFont gridAnnotationFont() const { return mGridAnnotationFont; }

    void setShowGridAnnotation( bool show ) {mShowGridAnnotation = show;}
    bool showGridAnnotation() const {return mShowGridAnnotation;}

    void setGridAnnotationPosition( GridAnnotationPosition p ) {mGridAnnotationPosition = p;}
    GridAnnotationPosition gridAnnotationPosition() const {return mGridAnnotationPosition;}

    void setAnnotationFrameDistance( double d ) {mAnnotationFrameDistance = d;}
    double annotationFrameDistance() const {return mAnnotationFrameDistance;}

    void setGridAnnotationDirection( GridAnnotationDirection d ) {mGridAnnotationDirection = d;}
    GridAnnotationDirection gridAnnotationDirection() const {return mGridAnnotationDirection;}

    /**In case of annotations, the bounding rectangle can be larger than the map item rectangle*/
    QRectF boundingRect() const;
    /**Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle*/
    void updateBoundingRect();

  public slots:

    /**Called if map canvas has changed*/
    void updateCachedImage( );
    /**Call updateCachedImage if item is in render mode*/
    void renderModeUpdateCachedImage();

  signals:
    /**Is emitted when width/height is changed as a result of user interaction*/
    void extentChanged();

  private:

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

    /**Draws the map grid*/
    void drawGrid( QPainter* p );
    /**Annotations for composer grid*/
    void drawGridAnnotations( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines );
    /**Calculates the horizontal grid lines
        @lines list containing the map coordinates and the lines in item coordinates
        @return 0 in case of success*/
    int horizontalGridLines( QList< QPair< double, QLineF > >& lines ) const;
    /**Calculates the vertical grid lines
        @lines list containing the map coordinates and the lines in item coordinates
        @return 0 in case of success*/
    int verticalGridLines( QList< QPair< double, QLineF > >& lines ) const;
    /**Returns extent that considers mOffsetX / mOffsetY (during content move)*/
    QgsRectangle transformedExtent() const;
    double maxExtensionXDirection() const;
    double maxExtensionYDirection() const;
};

#endif
