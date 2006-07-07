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
/* $Id$ */

#ifndef QGSMAPCANVAS_H
#define QGSMAPCANVAS_H

#include <list>
#include <memory>
#include <deque>

#include "qgsrect.h"
#include "qgspoint.h"
#include "qgis.h"

#include <QDomDocument>
#include <Q3CanvasView>

class QWheelEvent;
class QPixmap;
class QPaintEvent;
class QKeyEvent;
class QResizeEvent;

class QColor;
class QPaintDevice;
class QMouseEvent;
class QRubberBand;
class Q3Canvas;

class QgsMapToPixel;
class QgsMapLayer;
class QgsMapLayerInterface;
class QgsLegend;
class QgsLegendView;
class QgsMeasure;
class QgsRubberBand;

class QgsMapRender;
class QgsMapCanvasMap;
class QgsMapOverviewCanvas;
class QgsMapCanvasMapImage;
class QgsMapTool;

/*! \class QgsMapCanvas
 * \brief Map canvas class for displaying all GIS data types.
 */

class QgsMapCanvas : public Q3CanvasView
{
    Q_OBJECT;

 public:
    //! Constructor
    QgsMapCanvas(QWidget * parent = 0, const char *name = 0);

    //! Destructor
    ~QgsMapCanvas();

    void setLayerSet(std::deque<QString>& layerSet);
    
    void setCurrentLayer(QgsMapLayer* layer);
    
    void updateOverview();
    
    void setOverview(QgsMapOverviewCanvas* overview);
    
    QgsMapCanvasMap* map();
    
    QgsMapRender* mapRender();
    
    //! Accessor for the canvas pixmap
    QPixmap * canvasPixmap();

    //! Get the last reported scale of the canvas
    double getScale();

    //! Clear the map canvas
    void clear();

    //! Returns the mupp (map units per pixel) for the canvas
    double mupp() const;

    //! Returns the current zoom exent of the map canvas
    QgsRect extent() const;
    //! Returns the combined exent for all layers on the map canvas
    QgsRect fullExtent() const;

    //! Set the extent of the map canvas
    void setExtent(QgsRect const & r);

    //! Zoom to the full extent of all layers
    void zoomFullExtent();

    //! Zoom to the previous extent (view)
    void zoomPreviousExtent();

    /**Zooms to the extend of the selected features*/
    void zoomToSelected();

    /** \brief Sets the map tool currently being used on the canvas */
    void setMapTool(QgsMapTool* mapTool);

    /**Returns the currently active tool*/
    QgsMapTool* mapTool();

    /** Write property of QColor bgColor. */
    virtual void setCanvasColor(const QColor & _newVal);

    /** Emits signal scalChanged to update scale in main window */
    void updateScale();

    /** Updates the full extent */
    void updateFullExtent();

    //! return the map layer at postion index in the layer stack
    QgsMapLayer *getZpos(int index);
    
    //! return the layer by name
    QgsMapLayer *layerByName(QString n);

    //! return number of layers on the map
    int layerCount() const;

    /*! Freeze/thaw the map canvas. This is used to prevent the canvas from
     * responding to events while layers are being added/removed etc.
     * @param frz Boolean specifying if the canvas should be frozen (true) or
     * thawed (false). Default is true.
     */
    void freeze(bool frz = true);

    /*! Accessor for frozen status of canvas */
    bool isFrozen();

    //! Flag the canvas as dirty and needed a refresh
    void setDirty(bool _dirty);

    //! Return the state of the canvas (dirty or not)
    bool isDirty() const;

    //! Set map units (needed by project properties dialog)
    void setMapUnits(QGis::units mapUnits);
    //! Get the current canvas map units

    QGis::units mapUnits() const;

    //! Get the current coordinate transform
    QgsMapToPixel * getCoordinateTransform();

    /** stores state in DOM node
        layerNode is DOM node corresponding to ``qgis'' tag

        The DOM node corresponds to a DOM document project file XML element to be
        written by QgsProject.

        Invoked by QgsProject::write().

        returns true if successful
    */
    bool writeXML( QDomNode & layerNode, QDomDocument & doc );

    //! true if canvas currently drawing
    bool isDrawing();
    
    //! returns current layer (set by legend widget)
    QgsMapLayer* currentLayer();

    //! Zooms in/out with a given center (uses zoomByScale)
    void zoomWithCenter(int x, int y, bool zoomIn);

    //! used to determine if anti-aliasing is enabled or not
    void enableAntiAliasing(bool theFlag);
    
    //! Select which Qt class to render with
    void useQImageToRender(bool theFlag);

    // following 2 methods should be moved elsewhere or changed to private
    // currently used by pan map tool
    //! Ends pan action and redraws the canvas.
    void panActionEnd(QPoint releasePoint);
    //! Called when mouse is moving and pan is activated
    void panAction(QMouseEvent * event);
    
    //! returns last position of mouse cursor
    QPoint mouseLastXY();
  
  public slots:

    /**Sets dirty=true and calls render()*/
    void refresh();

    virtual void render();

    //! Save the convtents of the map canvas to disk as an image
    void saveAsImage(QString theFileName,QPixmap * QPixmap=0, QString="PNG" );

    //! This slot is connected to the visibility change of one or more layers
    void layerStateChange();

    //! Whether to suppress rendering or not
    void setRenderFlag(bool theFlag);
    //! State of render suppression flag
    bool renderFlag() {return mRenderFlag;};

    /** A simple helper method to find out if on the fly projections are enabled or not */
    bool projectionsEnabled();

    /** The map units may have changed, so cope with that */
    void mapUnitsChanged();
    
    /** updates pixmap on render progress */
    void updateMap();
    
    //! show whatever error is exposed by the QgsMapLayer.
    void showError(QgsMapLayer * mapLayer);
    
    
signals:
    /** Let the owner know how far we are with render operations */
    void setProgress(int,int);
    /** emits current mouse position */
    void xyCoordinates(QgsPoint & p);

    //! Emitted when the scale of the map changes
    void scaleChanged(QString);

    //! Emitted when the extents of the map change
    void extentsChanged(QgsRect);

    /** Emitted when the canvas has rendered.

     Passes a pointer to the painter on which the map was drawn. This is
     useful for plugins that wish to draw on the map after it has been
     rendered.  Passing the painter allows plugins to work when the map is
     being rendered onto a pixmap other than the mapCanvas own pixmap member.

    */
    void renderComplete(QPainter *);
    
    //! Emitted when a new set of layers has been received
    void layersChanged();

    //! Emit key press event
    void keyPressed(QKeyEvent * e);

    //! Emit key release event
    void keyReleased(QKeyEvent * e);
    
protected:
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

    /**
       private to force use of ctor with arguments
     */
    QgsMapCanvas();

    //! all map rendering is done in this class
    QgsMapRender* mMapRender;
    
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
    
    //! Overridden key press event
    void keyPressEvent(QKeyEvent * e);

    //! Overridden key release event
    void keyReleaseEvent(QKeyEvent * e);

    //! Overridden mouse move event
    void contentsMouseMoveEvent(QMouseEvent * e);

    //! Overridden mouse press event
    void contentsMousePressEvent(QMouseEvent * e);

    //! Overridden mouse release event
    void contentsMouseReleaseEvent(QMouseEvent * e);

    //! Overridden mouse wheel event
    void wheelEvent(QWheelEvent * e);

    //! Overridden resize event
    void resizeEvent(QResizeEvent * e);

    //! Overridden draw contents from canvas view
    void drawContents(QPainter * p, int cx, int cy, int cw, int ch);
        
    //! Zooms to a given center and scale 
    void zoomByScale(int x, int y, double scaleFactor);
    
    //! called when panning is in action, reset indicates end of panning
    void moveCanvasContents(bool reset = FALSE);
    
    //! called on resize or changed extent to notify canvas items to change their rectangle
    void updateCanvasItemsPositions();

    //! determines whether user has requested to suppress rendering
    bool mRenderFlag;
    
  /** debugging member
      invoked when a connect() is made to this object
  */
  void connectNotify( const char * signal );

    //! current layer in legend
    QgsMapLayer* mCurrentLayer;

    Q3Canvas* mCanvas;
    
    //! pointer to current map tool
    QgsMapTool* mMapTool;
    
    //! previous tool if current is for zooming/panning
    QgsMapTool* mLastNonZoomMapTool;

    //! recently used extent
    QgsRect mLastExtent;
    
    //! Scale factor multiple for default zoom in/out
    // TODO Make this customisable by the user
    static const double scaleDefaultMultiple;

}; // class QgsMapCanvas


#endif
