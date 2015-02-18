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

#ifndef QGSMAPCANVAS_H
#define QGSMAPCANVAS_H

#include "qgsconfig.h"

#include <list>
#include <memory>
#include <deque>

#include "qgsrectangle.h"
#include "qgspoint.h"
#include "qgis.h"

#include <QDomDocument>
#include <QGraphicsView>
#include <QtCore>

#include "qgsmapsettings.h" // TEMPORARY
#include "qgsprevieweffect.h" //for QgsPreviewEffect::PreviewMode

#ifdef HAVE_TOUCH
#include <QGestureEvent>
#endif

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
class QgsHighlight;
class QgsVectorLayer;

class QgsLabelingResults;
class QgsMapRenderer;
class QgsMapRendererCache;
class QgsMapRendererQImageJob;
class QgsMapSettings;
class QgsMapCanvasMap;
class QgsMapOverviewCanvas;
class QgsMapTool;
class QgsSnappingUtils;

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

    // ### QGIS 3: make QgsMapCanvas independent from overview
    void updateOverview();

    // ### QGIS 3: make QgsMapCanvas independent from overview
    void enableOverviewMode( QgsMapOverviewCanvas* overview );

    //! Get access to properties used for map rendering
    //! @note added in 2.4
    const QgsMapSettings& mapSettings() const;

    //! sets whether to use projections for this layer set
    //! @note added in 2.4
    void setCrsTransformEnabled( bool enabled );

    //! sets destination coordinate reference system
    //! @note added in 2.4
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );

    //! Get access to the labeling results (may be null)
    //! @note added in 2.4
    const QgsLabelingResults* labelingResults() const;

    //! Set whether to cache images of rendered layers
    //! @note added in 2.4
    void setCachingEnabled( bool enabled );

    //! Check whether images of rendered layers are curerently being cached
    //! @note added in 2.4
    bool isCachingEnabled() const;

    //! Make sure to remove any rendered images from cache (does nothing if cache is not enabled)
    //! @note added in 2.4
    void clearCache();

    //! Set whether the layers are rendered in parallel or sequentially
    //! @note added in 2.4
    void setParallelRenderingEnabled( bool enabled );

    //! Check whether the layers are rendered in parallel or sequentially
    //! @note added in 2.4
    bool isParallelRenderingEnabled() const;

    //! Set how often map preview should be updated while it is being rendered (in milliseconds)
    //! @note added in 2.4
    void setMapUpdateInterval( int timeMiliseconds );

    //! Find out how often map preview should be updated while it is being rendered (in milliseconds)
    //! @note added in 2.4
    int mapUpdateInterval() const;

    //! @deprecated since 2.4 - there could be more than just one "map" items
    QgsMapCanvasMap* map();

    //! @deprecated since 2.4 - use mapSettings() for anything related to current renderer settings
    //// SIP: removed /Transfer/ because it crashes after few calls to iface.mapCanvas().mapRenderer().hasCrsTransformEnabled()
    //// and in fact there is no transfer of ownership from c++ to python!
    //// Actually the problem comes from the fact that "hasCrsTransformEnabled" is both a signal and a normal method
    //// /KeepReference/ is necessary because otherwise mapRenderer().hasCrsTransformEnabled() was crashing
    Q_DECL_DEPRECATED QgsMapRenderer* mapRenderer();

    //! Accessor for the canvas paint device
    //! @deprecated since 2.4
    Q_DECL_DEPRECATED QPaintDevice &canvasPaintDevice();

    //! Get the last reported scale of the canvas
    double scale();

    //! Clear the map canvas
    //! @deprecated since 2.4 - use refresh() - clear does the same thing
    Q_DECL_DEPRECATED void clear();

    //! Returns the mapUnitsPerPixel (map units per pixel) for the canvas
    double mapUnitsPerPixel() const;

    //! Returns the current zoom exent of the map canvas
    QgsRectangle extent() const;
    //! Returns the combined exent for all layers on the map canvas
    QgsRectangle fullExtent() const;

    //! Set the extent of the map canvas
    void setExtent( const QgsRectangle &r );

    //! Get the current map canvas rotation in clockwise degrees
    //! @note added in 2.8
    double rotation() const;

    //! Set the rotation of the map canvas in clockwise degrees
    //! @note added in 2.8
    void setRotation( double degrees );

    //! Set the center of the map canvas, in geographical coordinates
    //! @note added in 2.8
    void setCenter( const QgsPoint& center );

    //! Get map center, in geographical coordinates
    //! @note added in 2.8
    QgsPoint center() const;

    //! Zoom to the full extent of all layers
    void zoomToFullExtent();

    //! Zoom to the previous extent (view)
    void zoomToPreviousExtent();

    //! Zoom to the next extent (view)
    void zoomToNextExtent();

    // ! Clears the list of extents and sets current extent as first item
    void clearExtentHistory();

    /** Zoom to the extent of the selected features of current (vector) layer.
      @param layer optionally specify different than current layer */
    void zoomToSelected( QgsVectorLayer* layer = NULL );

    /** Pan to the selected features of current (vector) layer keeping same extent. */
    void panToSelected( QgsVectorLayer* layer = NULL );

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

    /** Set color of selected vector features */
    //! @note added in 2.4
    void setSelectionColor( const QColor& color );

    /** Emits signal scaleChanged to update scale in main window */
    void updateScale();

    /** Updates the full extent */
    //! @deprecated since v2.4 - does nothing
    Q_DECL_DEPRECATED void updateFullExtent() {}

    //! return the map layer at position index in the layer stack
    QgsMapLayer *layer( int index );

    //! return number of layers on the map
    int layerCount() const;

    //! return list of layers within map canvas.
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
    //! @deprecated since 2.4 - use refresh() to trigger a refresh (clients should not decide explicitly whether canvas is dirty or not)
    Q_DECL_DEPRECATED void setDirty( bool _dirty );

    //! Return the state of the canvas (dirty or not)
    //! @deprecated since 2.4 - dirty flag is not kept anymore - always returns false
    Q_DECL_DEPRECATED bool isDirty() const;

    //! Set map units (needed by project properties dialog)
    void setMapUnits( QGis::UnitType mapUnits );
    //! Get the current canvas map units

    QGis::UnitType mapUnits() const;

    //! Get the current coordinate transform
    const QgsMapToPixel* getCoordinateTransform();

    //! Find out whether rendering is in progress
    bool isDrawing();

    //! returns current layer (set by legend widget)
    QgsMapLayer* currentLayer();

    //! set wheel action and zoom factor (should be greater than 1)
    void setWheelAction( WheelAction action, double factor = 2 );

    //! Zoom in with fixed factor
    void zoomIn();

    //! Zoom out with fixed factor
    void zoomOut();

    //! Zoom to a specific scale
    void zoomScale( double scale );

    //! Zoom with the factor supplied. Factor > 1 zooms out, interval (0,1) zooms in
    //! If point is given, re-center on it
    void zoomByFactor( double scaleFactor, const QgsPoint *center = 0 );

    //! Zooms in/out with a given center
    void zoomWithCenter( int x, int y, bool zoomIn );

    //! used to determine if anti-aliasing is enabled or not
    void enableAntiAliasing( bool theFlag );

    //! true if antialising is enabled
    bool antiAliasingEnabled() const { return mSettings.testFlag( QgsMapSettings::Antialiasing ); }

    //! Select which Qt class to render with
    //! @deprecated since 2.4 - does nothing because now we always render to QImage
    Q_DECL_DEPRECATED void useImageToRender( bool theFlag );

    // following 2 methods should be moved elsewhere or changed to private
    // currently used by pan map tool
    //! Ends pan action and redraws the canvas.
    void panActionEnd( QPoint releasePoint );

    //! Called when mouse is moving and pan is activated
    void panAction( QMouseEvent * event );

    //! returns last position of mouse cursor
    QPoint mouseLastXY();

    /** Enables a preview mode for the map canvas
     * @param previewEnabled set to true to enable a preview mode
     * @see setPreviewMode
     * @note added in 2.3 */
    void setPreviewModeEnabled( bool previewEnabled );

    /** Returns whether a preview mode is enabled for the map canvas
     * @returns true if a preview mode is currently enabled
     * @see setPreviewModeEnabled
     * @see previewMode
     * @note added in 2.3 */
    bool previewModeEnabled() const;

    /** Sets a preview mode for the map canvas. This setting only has an effect if
     * previewModeEnabled is true.
     * @param mode preview mode for the canvas
     * @see previewMode
     * @see setPreviewModeEnabled
     * @see previewModeEnabled
     * @note added in 2.3 */
    void setPreviewMode( QgsPreviewEffect::PreviewMode mode );

    /** Returns the current preview mode for the map canvas. This setting only has an effect if
     * previewModeEnabled is true.
     * @returns preview mode for map canvas
     * @see setPreviewMode
     * @see previewModeEnabled
     * @note added in 2.3 */
    QgsPreviewEffect::PreviewMode previewMode() const;

    /** Return snapping utility class that is associated with map canvas.
     *  If no snapping utils instance has been associated previously, an internal will be created for convenience
     *  (so map tools do not need to test for existence of the instance).
     *
     * Main canvas in QGIS returns an instance which is always up-to-date with the project's snapping configuration.
     *  @note added in 2.8
     */
    QgsSnappingUtils* snappingUtils() const;
    /** Assign an instance of snapping utils to the map canvas.
     * The instance is not owned by the canvas, so it is possible to use one instance in multiple canvases.
     *
     * For main canvas in QGIS, do not associate a different instance from the existing one (it is updated from
     * the project's snapping configuration).
     * @note added in 2.8
     */
    void setSnappingUtils( QgsSnappingUtils* utils );

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

    //! @deprecated in 2.4 - does nothing - kept for API compatibility
    Q_DECL_DEPRECATED void updateMap();

    //! stop rendering (if there is any right now)
    //! @note added in 2.4
    void stopRendering();

    //! @deprecated since 2.4 - does nothing - errors are reported by different means
    Q_DECL_DEPRECATED void showError( QgsMapLayer * mapLayer );

    //! called to read map canvas settings from project
    void readProject( const QDomDocument & );

    //! called to write map canvas settings to project
    void writeProject( QDomDocument & );

    //! ask user about datum transformation
    void getDatumTransformInfo( const QgsMapLayer* ml, const QString& srcAuthId, const QString& destAuthId );

    //! return if canvas rotation is enabled
    //! @note added in 2.8
    static bool rotationEnabled();

    //! change canvas rotation support
    //! @note added in 2.8
    static void enableRotation( bool enabled );

  private slots:
    //! called when current maptool is destroyed
    void mapToolDestroyed();

    //! called when a renderer job has finished successfully or when it was cancelled
    void rendererJobFinished();

    void mapUpdateTimeout();

    void refreshMap();

  signals:
    /** Let the owner know how far we are with render operations */
    //! @deprecated since 2.4 - already unused in 2.0 anyway
    Q_DECL_DEPRECATED void setProgress( int, int );

    /** emits current mouse position
        \note changed in 1.3 */
    void xyCoordinates( const QgsPoint &p );

    //! Emitted when the scale of the map changes
    void scaleChanged( double );

    //! Emitted when the extents of the map change
    void extentsChanged();

    //! Emitted when the rotation of the map changes
    //! @note added in 2.8
    void rotationChanged( double );

    /** Emitted when the canvas has rendered.

     Passes a pointer to the painter on which the map was drawn. This is
     useful for plugins that wish to draw on the map after it has been
     rendered.  Passing the painter allows plugins to work when the map is
     being rendered onto a pixmap other than the mapCanvas own pixmap member.

    */
    //! TODO: deprecate when decorations are reimplemented as map canvas items
    //! - anything related to rendering progress is not visible outside of map canvas
    //! - additional drawing shall be done directly within the renderer job or independently as a map canvas item
    void renderComplete( QPainter * );

    // ### QGIS 3: renamte to mapRefreshFinished()
    /** Emitted when canvas finished a refresh request. */
    void mapCanvasRefreshed();

    // ### QGIS 3: rename to mapRefreshStarted()
    /** Emitted when the canvas is about to be rendered. */
    void renderStarting();

    //! Emitted when a new set of layers has been received
    void layersChanged();

    //! Emit key press event
    void keyPressed( QKeyEvent * e );

    //! Emit key release event
    void keyReleased( QKeyEvent * e );

    //! Emit map tool changed event
    void mapToolSet( QgsMapTool *tool );

    /** Emit map tool changed with the old tool
     * @note added in 2.3
     */
    void mapToolSet( QgsMapTool *newTool, QgsMapTool* oldTool );

    // ### QGIS 3: remove the signal
    //! Emitted when selection in any layer gets changed
    void selectionChanged( QgsMapLayer * layer );

    //! Emitted when zoom last status changed
    void zoomLastStatusChanged( bool );

    //! Emitted when zoom next status changed
    void zoomNextStatusChanged( bool );

    //! Emitted when on-the-fly projection has been turned on/off
    //! @note added in 2.4
    void hasCrsTransformEnabledChanged( bool flag );

    //! Emitted when map CRS has changed
    //! @note added in 2.4
    void destinationCrsChanged();

    //! Emmitted when map units are changed
    //! @note added in 2.4
    void mapUnitsChanged();

    //! Emitted when the current layer is changed
    //! @note added in 2.8
    void currentLayerChanged( QgsMapLayer* layer );

  protected:
#ifdef HAVE_TOUCH
    //! Overridden standard event to be gestures aware
    bool event( QEvent * e ) override;
#endif

    //! Overridden key press event
    void keyPressEvent( QKeyEvent * e ) override;

    //! Overridden key release event
    void keyReleaseEvent( QKeyEvent * e ) override;

    //! Overridden mouse double click event
    void mouseDoubleClickEvent( QMouseEvent * e ) override;

    //! Overridden mouse move event
    void mouseMoveEvent( QMouseEvent * e ) override;

    //! Overridden mouse press event
    void mousePressEvent( QMouseEvent * e ) override;

    //! Overridden mouse release event
    void mouseReleaseEvent( QMouseEvent * e ) override;

    //! Overridden mouse wheel event
    void wheelEvent( QWheelEvent * e ) override;

    //! Overridden resize event
    void resizeEvent( QResizeEvent * e ) override;

    //! Overridden paint event
    void paintEvent( QPaintEvent * e ) override;

    //! Overridden drag enter event
    void dragEnterEvent( QDragEnterEvent * e ) override;

    //! called when panning is in action, reset indicates end of panning
    void moveCanvasContents( bool reset = false );

    //! called on resize or changed extent to notify canvas items to change their rectangle
    void updateCanvasItemPositions();

    /// implementation struct
    class CanvasProperties;

    /// Handle pattern for implementation object
    QScopedPointer<CanvasProperties> mCanvasProperties;

#if 0
    /**debugging member
       invoked when a connect() is made to this object
    */
    void connectNotify( const char * signal ) override;
#endif
    //! Make sure the datum transform store is properly populated
    void updateDatumTransformEntries();

  private:
    /// this class is non-copyable
    /**
       @note

       Otherwise QScopedPointer would pass the object responsiblity on to the
       copy like a hot potato leaving the copyer in a weird state.
     */
    QgsMapCanvas( QgsMapCanvas const & );

    //! encompases all map settings necessary for map rendering
    QgsMapSettings mSettings;

    //! all map rendering is done in this class
    QgsMapRenderer* mMapRenderer;

    //! owns pixmap with rendered map and controls rendering
    QgsMapCanvasMap* mMap;

    //! map overview widget - it's controlled by QgsMapCanvas
    QgsMapOverviewCanvas* mMapOverview;

    //! Flag indicating if the map canvas is frozen.
    bool mFrozen;

    //! Flag that allows squashing multiple refresh() calls into just one delayed rendering job
    bool mRefreshScheduled;

    //! determines whether user has requested to suppress rendering
    bool mRenderFlag;

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

    //! Timer that periodically fires while map rendering is in progress to update the visible map
    QTimer mMapUpdateTimer;

    //! Job that takes care of map rendering in background
    QgsMapRendererQImageJob* mJob;

    //! Flag determining whether the active job has been cancelled
    bool mJobCancelled;

    //! Labeling results from the recently rendered map
    QgsLabelingResults* mLabelingResults;

    //! Whether layers are rendered sequentially or in parallel
    bool mUseParallelRendering;

    //! Whether to add rendering stats to the rendered image
    bool mDrawRenderingStats;

    //! Optionally use cache with rendered map layers for the current map settings
    QgsMapRendererCache* mCache;

    QTimer *mResizeTimer;

    QgsPreviewEffect* mPreviewEffect;

    QgsRectangle imageRect( const QImage& img, const QgsMapSettings& mapSettings );

    QgsSnappingUtils* mSnappingUtils;

}; // class QgsMapCanvas




/** Class that does synchronization between QgsMapCanvas and its associated QgsMapRenderer:
 *   - changes done in map canvas settings are pushed to map renderer
 *   - changes done in map renderer are pushed to map canvas settings
 *
 * This class can be removed within API cleanup when QgsMapRenderer will not be accessible from canvas API anymore.
 * Added in 2.4. This class is not a part of public API!
 */
class QgsMapCanvasRendererSync : public QObject
{
    Q_OBJECT
  public:
    QgsMapCanvasRendererSync( QgsMapCanvas* canvas, QgsMapRenderer* renderer );

  protected slots:
    void onExtentC2R();
    void onExtentR2C();

    void onMapUnitsC2R();
    void onMapUnitsR2C();

    //! @note added in 2.8
    void onMapRotationC2R();
    //! @note added in 2.8
    void onMapRotationR2C();

    void onCrsTransformC2R();
    void onCrsTransformR2C();

    void onDestCrsC2R();
    void onDestCrsR2C();

    void onLayersC2R();

  protected:
    QgsMapCanvas* mCanvas;
    QgsMapRenderer* mRenderer;

    bool mSyncingExtent;
};


#endif
