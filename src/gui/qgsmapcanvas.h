/***************************************************************************
                          qgsmapcanvas.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgsmapcanvas.h 5341 2006-04-22 12:11:36Z wonder $ */

#ifndef QGSMAPCANVAS_H
#define QGSMAPCANVAS_H

#include <list>
#include <memory>
#include <deque>

#include "qgsrectangle.h"
#include "qgspoint.h"
#include "qgis.h"

#include <QDomDocument>
#include <QGraphicsView>
#include <QtCore>

class QWheelEvent;
class QPixmap;
class QPaintEvent;
class QKeyEvent;
class ResizeEvent;

class QColor;
class QDomDocument;
class QPaintDevice;
class QMouseEvent;
class QRubberBand;
class QGraphicsScene;

class QgsMapToPixel;
class QgsMapLayer;
class QgsLegend;
class QgsLegendView;
class QgsHighlight;
class QgsVectorLayer;

class QgsMapRenderer;
class QgsMapCanvasMap;
class QgsMapOverviewCanvas;
class QgsMapTool;

/** \ingroup gui
  * A class that stores visibility and presence in overview flags together
  * with pointer to the layer.
  *
*/
class GUI_EXPORT QgsMapCanvasLayer
{
  public:
    QgsMapCanvasLayer( QgsMapLayer* layer, bool visible = true, bool isInOverview = false )
        : mLayer( layer ), mVisible( visible ), mInOverview( isInOverview ) {}

    void setVisible( bool visible ) { mVisible = visible; }
    void setInOverview( bool isInOverview ) { mInOverview = isInOverview; }

    bool isVisible() const { return mVisible; }
    bool isInOverview() const { return mInOverview; }

    QgsMapLayer* layer() { return mLayer; }
    const QgsMapLayer* layer() const { return mLayer; }

  private:
    QgsMapLayer* mLayer;

    /** Flag whether layer is visible */
    bool mVisible;

    /** Flag whether layer is shown in overview */
    bool mInOverview;
};


/** \ingroup gui
 * Map canvas is a class for displaying all GIS data types on a canvas.
 */

class GUI_EXPORT QgsMapCanvas : public QGraphicsView
{
    Q_OBJECT

  public:

    enum WheelAction { WheelZoom, WheelZoomAndRecenter, WheelZoomToMouseCursor, WheelNothing };

    //! Constructor
    QgsMapCanvas( QWidget * parent = 0, const char *name = 0 );

    //! Destructor
    ~QgsMapCanvas();

    void setLayerSet( QList<QgsMapCanvasLayer>& layers );

    void setCurrentLayer( QgsMapLayer* layer );

    void updateOverview();

    void enableOverviewMode( QgsMapOverviewCanvas* overview );

    QgsMapCanvasMap* map();

    QgsMapRenderer* mapRenderer();

    //! Accessor for the canvas pixmap
    //! @deprecated use canvasPaintDevice()
    QGISDEPRECATED QPixmap& canvasPixmap();

    //! Accessor for the canvas paint device
    QPaintDevice &canvasPaintDevice();

    //! Get the last reported scale of the canvas
    double scale();

    //! Clear the map canvas
    void clear();

    //! Returns the mapUnitsPerPixel (map units per pixel) for the canvas
    double mapUnitsPerPixel() const;

    //! Returns the current zoom exent of the map canvas
    QgsRectangle extent() const;
    //! Returns the combined exent for all layers on the map canvas
    QgsRectangle fullExtent() const;

    //! Set the extent of the map canvas
    void setExtent( QgsRectangle const & r );

    //! Zoom to the full extent of all layers
    void zoomToFullExtent();

    //! Zoom to the previous extent (view)
    void zoomToPreviousExtent();

    //! Zoom to the Next extent (view)
    void zoomToNextExtent();

    // ! Clears the list of extents and sets current extent as first item
    void clearExtentHistory();

    /** Zoom to the extent of the selected features of current (vector) layer.
      Added in version 1.2: optionally specify different than current layer */
    void zoomToSelected( QgsVectorLayer* layer = NULL );

    /** \brief Sets the map tool currently being used on the canvas */
    void setMapTool( QgsMapTool* mapTool );

    /** \brief Unset the current map tool or last non zoom tool
     *
     * This is called from destructor of map tools to make sure
     * that this map tool won't be used any more.
     * You don't have to call it manualy, QgsMapTool takes care of it.
     */
    void unsetMapTool( QgsMapTool* mapTool );

    /**Returns the currently active tool*/
    QgsMapTool* mapTool();

    /** Write property of QColor bgColor. */
    virtual void setCanvasColor( const QColor & _newVal );
    /** Read property of QColor bgColor. */
    virtual QColor canvasColor() const;

    /** Emits signal scalChanged to update scale in main window */
    void updateScale();

    /** Updates the full extent */
    void updateFullExtent();

    //! return the map layer at position index in the layer stack
    QgsMapLayer *layer( int index );

    //! return number of layers on the map
    int layerCount() const;

    //! return list of layers within map canvas. Added in v1.5
    QList<QgsMapLayer*> layers() const;

    /*! Freeze/thaw the map canvas. This is used to prevent the canvas from
     * responding to events while layers are being added/removed etc.
     * @param frz Boolean specifying if the canvas should be frozen (true) or
     * thawed (false). Default is true.
     */
    void freeze( bool frz = true );

    /*! Accessor for frozen status of canvas */
    bool isFrozen();

    //! Flag the canvas as dirty and needed a refresh
    void setDirty( bool _dirty );

    //! Return the state of the canvas (dirty or not)
    bool isDirty() const;

    //! Set map units (needed by project properties dialog)
    void setMapUnits( QGis::UnitType mapUnits );
    //! Get the current canvas map units

    QGis::UnitType mapUnits() const;

    //! Get the current coordinate transform
    const QgsMapToPixel* getCoordinateTransform();

    //! true if canvas currently drawing
    bool isDrawing();

    //! returns current layer (set by legend widget)
    QgsMapLayer* currentLayer();

    //! set wheel action and zoom factor (should be greater than 1)
    void setWheelAction( WheelAction action, double factor = 2 );

    //! Zoom in with fixed factor
    void zoomIn( );

    //! Zoom out with fixed factor
    void zoomOut( );

    //! Zoom to a specific scale
    // added in 1.5
    void zoomScale( double scale );

    //! Zoom with the factor supplied. Factor > 1 zooms out, interval (0,1) zooms in
    void zoomByFactor( double scaleFactor );

    //! Zooms in/out with a given center
    void zoomWithCenter( int x, int y, bool zoomIn );

    //! used to determine if anti-aliasing is enabled or not
    void enableAntiAliasing( bool theFlag );

    //! Select which Qt class to render with
    void useImageToRender( bool theFlag );

    // following 2 methods should be moved elsewhere or changed to private
    // currently used by pan map tool
    //! Ends pan action and redraws the canvas.
    void panActionEnd( QPoint releasePoint );

    //! Called when mouse is moving and pan is activated
    void panAction( QMouseEvent * event );

    //! returns last position of mouse cursor
    QPoint mouseLastXY();

  public slots:

    /**Repaints the canvas map*/
    void refresh();

    //! Receives signal about selection change, and pass it on with layer info
    void selectionChangedSlot();

    //! Save the convtents of the map canvas to disk as an image
    void saveAsImage( QString theFileName, QPixmap * QPixmap = 0, QString = "PNG" );

    //! This slot is connected to the visibility change of one or more layers
    void layerStateChange();

    //! Whether to suppress rendering or not
    void setRenderFlag( bool theFlag );
    //! State of render suppression flag
    bool renderFlag() {return mRenderFlag;};

    /** A simple helper method to find out if on the fly projections are enabled or not */
    bool hasCrsTransformEnabled();

    /** The map units may have changed, so cope with that */
    void mapUnitsChanged();

    /** updates pixmap on render progress */
    void updateMap();

    //! show whatever error is exposed by the QgsMapLayer.
    void showError( QgsMapLayer * mapLayer );

    //! called to read map canvas settings from project
    void readProject( const QDomDocument & );

    //! called to write map canvas settings to project
    void writeProject( QDomDocument & );

  signals:
    /** Let the owner know how far we are with render operations */
    void setProgress( int, int );

    /** emits current mouse position
        \note changed in 1.3 */
    void xyCoordinates( const QgsPoint & p );

    //! Emitted when the scale of the map changes
    void scaleChanged( double );

    //! Emitted when the extents of the map change
    void extentsChanged();

    /** Emitted when the canvas has rendered.

     Passes a pointer to the painter on which the map was drawn. This is
     useful for plugins that wish to draw on the map after it has been
     rendered.  Passing the painter allows plugins to work when the map is
     being rendered onto a pixmap other than the mapCanvas own pixmap member.

    */
    void renderComplete( QPainter * );

    /** Emitted when the canvas is about to be rendered.
      \note Added in 1.5 */
    void renderStarting();

    //! Emitted when a new set of layers has been received
    void layersChanged();

    //! Emit key press event
    void keyPressed( QKeyEvent * e );

    //! Emit key release event
    void keyReleased( QKeyEvent * e );

    //! Emit map tool changed event
    void mapToolSet( QgsMapTool * tool );

    //! Emitted when selection in any layer gets changed
    void selectionChanged( QgsMapLayer * layer );

    //! Emitted when zoom last status changed
    //! @note: this signal was added in version 1.4
    void zoomLastStatusChanged( bool );

    //! Emitted when zoom next status changed
    //! @note: this signal was added in version 1.4
    void zoomNextStatusChanged( bool );

  protected:
    //! Overridden key press event
    void keyPressEvent( QKeyEvent * e );

    //! Overridden key release event
    void keyReleaseEvent( QKeyEvent * e );

    //! Overridden mouse double click event
    void mouseDoubleClickEvent( QMouseEvent * e );

    //! Overridden mouse move event
    void mouseMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    void mousePressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    void mouseReleaseEvent( QMouseEvent * e );

    //! Overridden mouse wheel event
    void wheelEvent( QWheelEvent * e );

    //! Overridden resize event
    void resizeEvent( QResizeEvent * e );

    //! Overridden paint event
    void paintEvent( QPaintEvent * e );

    //! called when panning is in action, reset indicates end of panning
    void moveCanvasContents( bool reset = false );

    //! called on resize or changed extent to notify canvas items to change their rectangle
    void updateCanvasItemPositions();

    /// implementation struct
    class CanvasProperties;

    /// Handle pattern for implementation object
    std::auto_ptr<CanvasProperties> mCanvasProperties;

  private:
    /// this class is non-copyable
    /**
       @note

       Otherwise std::auto_ptr would pass the object responsiblity on to the
       copy like a hot potato leaving the copyer in a weird state.
     */
    QgsMapCanvas( QgsMapCanvas const & );

    //! all map rendering is done in this class
    QgsMapRenderer* mMapRenderer;

    //! owns pixmap with rendered map and controls rendering
    QgsMapCanvasMap* mMap;

    //! map overview widget - it's controlled by QgsMapCanvas
    QgsMapOverviewCanvas* mMapOverview;

    //! Flag indicating a map refresh is in progress
    bool mDrawing;

    //! Flag indicating if the map canvas is frozen.
    bool mFrozen;

    /*! \brief Flag to track the state of the Map canvas.
     *
     * The canvas is
     * flagged as dirty by any operation that changes the state of
     * the layers or the view extent. If the canvas is not dirty, paint
     * events are handled by bit-blitting the stored canvas bitmap to
     * the canvas. This improves performance by not reading the data source
     * when no real change has occurred
     */
    bool mDirty;

    //! determines whether user has requested to suppress rendering
    bool mRenderFlag;

    /**Resize events that have been ignored because the canvas is busy with
       rendering may put their sizes into this list. The canvas then picks up
       the last entry in case a lot of resize events arrive in short time*/
    QList< QPair<int, int> > mResizeQueue;

    /**debugging member
       invoked when a connect() is made to this object
    */
    void connectNotify( const char * signal );

    //! current layer in legend
    QgsMapLayer* mCurrentLayer;

    //! graphics scene manages canvas items
    QGraphicsScene* mScene;

    //! pointer to current map tool
    QgsMapTool* mMapTool;

    //! previous tool if current is for zooming/panning
    QgsMapTool* mLastNonZoomMapTool;

    //! recently used extent
    QList <QgsRectangle> mLastExtent;
    int mLastExtentIndex;

    //! Scale factor multiple for default zoom in/out
    double mWheelZoomFactor;

    //! Mouse wheel action
    WheelAction mWheelAction;

    //! resize canvas size
    QSize mNewSize;

    //! currently in paint event
    bool mPainting;

}; // class QgsMapCanvas


#endif
