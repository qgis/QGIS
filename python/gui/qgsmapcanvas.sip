/** \class QgsMapCanvasLayer
  \brief class that stores additional layer's flags together with pointer to the layer
*/
class QgsMapCanvasLayer
{
%TypeHeaderCode
#include <qgsmapcanvas.h>
%End
  public:
    QgsMapCanvasLayer( QgsMapLayer* layer, bool visible = true, bool isInOverview = false );

    void setVisible( bool visible );
    void setInOverview( bool isInOverview );

    bool isVisible() const;
    bool isInOverview() const;

    const QgsMapLayer* layer() const;
};


/** \ingroup gui
 * Map canvas is a class for displaying all GIS data types on a canvas.
 */

class QgsMapCanvas : QGraphicsView
{
%TypeHeaderCode
#include <qgsmapcanvas.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsMapCanvas*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapCanvas;
  else
    sipClass = NULL;
%End

  public:

    enum WheelAction { WheelZoom, WheelZoomAndRecenter, WheelZoomToMouseCursor, WheelNothing };

    //! Constructor
    QgsMapCanvas( QWidget * parent /TransferThis/ = 0, const char *name = 0 );

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
    const QgsMapSettings& mapSettings() const /KeepReference/;

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

    //! Reload all layers, clear the cache and refresh the canvas
    //! @note added in 2.9
    void refreshAllLayers();

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
    QgsMapRenderer* mapRenderer() /Deprecated, KeepReference/;

    //! Accessor for the canvas paint device
    //! @deprecated since 2.4
    QPaintDevice &canvasPaintDevice() /Deprecated/;

    //! Get the last reported scale of the canvas
    double scale();

    //! Clear the map canvas
    //! @deprecated since 2.4 - use refresh() - clear does the same thing
    void clear() /Deprecated/;

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
    void updateFullExtent() /Deprecated/;

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
    void setDirty( bool _dirty ) /Deprecated/;

    //! Return the state of the canvas (dirty or not)
    //! @deprecated since 2.4 - dirty flag is not kept anymore - always returns false
    bool isDirty() const /Deprecated/;

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
    bool antiAliasingEnabled() const;

    //! Select which Qt class to render with
    //! @deprecated since 2.4 - does nothing because now we always render to QImage
    void useImageToRender( bool theFlag ) /Deprecated/;

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

    //! This slot is connected to the layer's CRS change
    void layerCrsChange();

    //! Whether to suppress rendering or not
    void setRenderFlag( bool theFlag );
    //! State of render suppression flag
    bool renderFlag();

    /** A simple helper method to find out if on the fly projections are enabled or not */
    bool hasCrsTransformEnabled();

    //! @deprecated in 2.4 - does nothing - kept for API compatibility
    void updateMap() /Deprecated/;

    //! stop rendering (if there is any right now)
    //! @note added in 2.4
    void stopRendering();

    //! @deprecated since 2.4 - does nothing - errors are reported by different means
    void showError( QgsMapLayer * mapLayer ) /Deprecated/;

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

  signals:
    /** Let the owner know how far we are with render operations */
    //! @deprecated since 2.4 - already unused in 2.0 anyway
    void setProgress( int, int ) /Deprecated/;

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
    //! Overridden standard event to be gestures aware
    bool event( QEvent * e );

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

    //! Overridden drag enter event
    void dragEnterEvent( QDragEnterEvent * e );

    //! called when panning is in action, reset indicates end of panning
    void moveCanvasContents( bool reset = false );

    //! called on resize or changed extent to notify canvas items to change their rectangle
    void updateCanvasItemPositions();

private:
    void connectNotify( const char * signal );
}; // class QgsMapCanvas
