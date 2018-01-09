/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmapcanvas.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/













class QgsMapCanvas : QGraphicsView
{
%Docstring
Map canvas is a class for displaying all GIS data types on a canvas.
%End

%TypeHeaderCode
#include "qgsmapcanvas.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsMapCanvas *>( sipCpp ) != NULL )
      sipType = sipType_QgsMapCanvas;
    else
      sipType = NULL;
%End
  public:

    QgsMapCanvas( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor
%End

    ~QgsMapCanvas();

    double magnificationFactor() const;
%Docstring
Returns the magnification factor

.. versionadded:: 2.16
%End

    void setLayers( const QList<QgsMapLayer *> &layers );
%Docstring
Sets the list of ``layers`` that should be shown in the canvas.

If the map canvas has been associated with a map theme via a call
to setTheme(), then any calls to setLayers() are ignored. It is necessary
to first clear the theme association by calling setTheme() with an
empty string before setLayers() calls can be made.

.. versionadded:: 3.0

.. seealso:: :py:func:`layers`
%End

    void setCurrentLayer( QgsMapLayer *layer );

    const QgsMapSettings &mapSettings() const /KeepReference/;
%Docstring
Get access to properties used for map rendering

.. versionadded:: 2.4
%End

    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
sets destination coordinate reference system

.. versionadded:: 2.4
%End

    void setMapSettingsFlags( QgsMapSettings::Flags flags );
%Docstring
Resets the ``flags`` for the canvas' map settings.

.. versionadded:: 3.0
%End

    const QgsLabelingResults *labelingResults() const;
%Docstring
Get access to the labeling results (may be null)

.. versionadded:: 2.4
%End

    void setCachingEnabled( bool enabled );
%Docstring
Set whether to cache images of rendered layers

.. versionadded:: 2.4
%End

    bool isCachingEnabled() const;
%Docstring
Check whether images of rendered layers are curerently being cached

.. versionadded:: 2.4
%End

    void clearCache();
%Docstring
Make sure to remove any rendered images from cache (does nothing if cache is not enabled)

.. versionadded:: 2.4
%End

    void refreshAllLayers();
%Docstring
Reload all layers, clear the cache and refresh the canvas

.. versionadded:: 2.9
%End

    void waitWhileRendering();
%Docstring
Blocks until the rendering job has finished.

In almost all cases you do NOT want to call this, as it will hang the UI
until the rendering job is complete. It's included in API solely for
unit testing and standalone Python scripts.

.. versionadded:: 3.0
%End

    void setParallelRenderingEnabled( bool enabled );
%Docstring
Set whether the layers are rendered in parallel or sequentially

.. versionadded:: 2.4
%End

    bool isParallelRenderingEnabled() const;
%Docstring
Check whether the layers are rendered in parallel or sequentially

.. versionadded:: 2.4
%End

    void setMapUpdateInterval( int timeMilliseconds );
%Docstring
Set how often map preview should be updated while it is being rendered (in milliseconds)

.. versionadded:: 2.4
%End

    int mapUpdateInterval() const;
%Docstring
Find out how often map preview should be updated while it is being rendered (in milliseconds)

.. versionadded:: 2.4
%End

    double scale() const;
%Docstring
Returns the last reported scale of the canvas.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
%End

    double mapUnitsPerPixel() const;
%Docstring
Returns the mapUnitsPerPixel (map units per pixel) for the canvas
%End

    QgsRectangle extent() const;
%Docstring
Returns the current zoom extent of the map canvas
%End
    QgsRectangle fullExtent() const;
%Docstring
Returns the combined extent for all layers on the map canvas
%End

    void setExtent( const QgsRectangle &r, bool magnified = false );
%Docstring
Set the extent of the map canvas
%End

    double rotation() const;
%Docstring
Get the current map canvas rotation in clockwise degrees

.. versionadded:: 2.8
%End

    void setRotation( double degrees );
%Docstring
Set the rotation of the map canvas in clockwise degrees

.. versionadded:: 2.8
%End

    void setCenter( const QgsPointXY &center );
%Docstring
Set the center of the map canvas, in geographical coordinates

.. versionadded:: 2.8
%End

    QgsPointXY center() const;
%Docstring
Get map center, in geographical coordinates

.. versionadded:: 2.8
%End

    void zoomToFullExtent();
%Docstring
Zoom to the full extent of all layers
%End

    void zoomToPreviousExtent();
%Docstring
Zoom to the previous extent (view)
%End

    void zoomToNextExtent();
%Docstring
Zoom to the next extent (view)
%End

    void clearExtentHistory();

    void zoomToSelected( QgsVectorLayer *layer = 0 );
%Docstring
Zoom to the extent of the selected features of current (vector) layer.

:param layer: optionally specify different than current layer
%End

    void zoomToFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids );
%Docstring
Set canvas extent to the bounding box of a set of features

:param layer: the vector layer
:param ids: the feature ids*
%End

    void panToFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids );
%Docstring
Centers canvas extent to feature ids

:param layer: the vector layer
:param ids: the feature ids*
%End

    void panToSelected( QgsVectorLayer *layer = 0 );
%Docstring
Pan to the selected features of current (vector) layer keeping same extent.
%End

    void flashFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids,
                          const QColor &startColor = QColor( 255, 0, 0, 255 ), const QColor &endColor = QColor( 255, 0, 0, 0 ),
                          int flashes = 3, int duration = 500 );
%Docstring
Causes a set of features with matching ``ids`` from a vector ``layer`` to flash
within the canvas.

The ``startColor`` and ``endColor`` can be specified, along with the number of
``flashes`` and ``duration`` of each flash (in milliseconds).

.. note::

   If the features or geometries are already available, flashGeometries() is much more efficient.

.. versionadded:: 3.0

.. seealso:: :py:func:`flashGeometries`
%End

    void flashGeometries( const QList< QgsGeometry > &geometries, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(),
                          const QColor &startColor = QColor( 255, 0, 0, 255 ), const QColor &endColor = QColor( 255, 0, 0, 0 ),
                          int flashes = 3, int duration = 500 );
%Docstring
Causes a set of ``geometries`` to flash within the canvas.

If ``crs`` is a valid coordinate reference system, the geometries will be automatically
transformed from this CRS to the canvas CRS.

The ``startColor`` and ``endColor`` can be specified, along with the number of
``flashes`` and ``duration`` of each flash (in milliseconds).

.. versionadded:: 3.0

.. seealso:: :py:func:`flashFeatureIds`
%End

    void setMapTool( QgsMapTool *mapTool, bool clean = false );
%Docstring
 Sets the map tool currently being used on the canvas
%End

    void unsetMapTool( QgsMapTool *mapTool );
%Docstring
 Unset the current map tool or last non zoom tool

This is called from destructor of map tools to make sure
that this map tool won't be used any more.
You don't have to call it manually, QgsMapTool takes care of it.
%End

    QgsMapTool *mapTool();
%Docstring
Returns the currently active tool
%End

    void setCanvasColor( const QColor &_newVal );
%Docstring
Write property of QColor bgColor.
%End
    QColor canvasColor() const;
%Docstring
Read property of QColor bgColor.
%End

    void setSelectionColor( const QColor &color );
%Docstring
Set color of selected vector features

.. versionadded:: 2.4
%End

    QColor selectionColor() const;
%Docstring
Returns color for selected features

.. versionadded:: 3.0
%End

    void updateScale();
%Docstring
Emits signal scaleChanged to update scale in main window
%End

    QgsMapLayer *layer( int index );
%Docstring
return the map layer at position index in the layer stack
%End

    int layerCount() const;
%Docstring
return number of layers on the map
%End

    QList<QgsMapLayer *> layers() const;
%Docstring
Return the list of layers shown within the map canvas.

.. seealso:: :py:func:`setLayers`
%End

    void freeze( bool frozen = true );
%Docstring
Freeze/thaw the map canvas. This is used to prevent the canvas from
responding to events while layers are being added/removed etc.

:param frozen: Boolean specifying if the canvas should be frozen (true) or
thawed (false). Default is true.

.. seealso:: :py:func:`isFrozen`

.. seealso:: :py:func:`setRenderFlag`
while setRenderFlag() should only be used when users disable rendering via GUI.
%End

    bool isFrozen() const;
%Docstring
Returns true if canvas is frozen.

.. seealso:: :py:func:`renderFlag`
have been halted programmatically, while renderFlag() should be used to
determine whether a user has disabled rendering via GUI.

.. seealso:: :py:func:`freeze`
%End

    bool renderFlag() const;
%Docstring
Returns true if canvas render is disabled as a result of user disabling
renders via the GUI.

.. seealso:: :py:func:`setRenderFlag`

.. seealso:: :py:func:`isFrozen`
have been halted programmatically, while renderFlag() should be used to
determine whether a user has disabled rendering via GUI.
%End

    QgsUnitTypes::DistanceUnit mapUnits() const;
%Docstring
Convience function for returning the current canvas map units. The map units
are dictated by the canvas' destinationCrs() map units.
%End

    QMap<QString, QString> layerStyleOverrides() const;
%Docstring
Returns the stored overrides of styles for layers.

.. versionadded:: 2.12

.. seealso:: :py:func:`setLayerStyleOverrides`
%End

    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );
%Docstring
Sets the stored overrides of styles for rendering layers.

If the map canvas has been associated with a map theme via a call
to setTheme(), then any calls to setLayerStyleOverrides() are ignored. It is necessary
to first clear the theme association by calling setTheme() with an
empty string before setLayerStyleOverrides() calls can be made.

.. versionadded:: 2.12

.. seealso:: :py:func:`layerStyleOverrides`
%End

    void setTheme( const QString &theme );
%Docstring
Sets a map ``theme`` to show in the canvas. The theme name must match
a theme present in the associated project's :py:class:`QgsMapThemeCollection`.

When the canvas is associated to a map theme, it will automatically follow
the layer selection and layer styles from that theme. Calls to setLayers()
or setLayerStyleOverrides() will have no effect, and canvases associated
with a QgsLayerTreeMapCanvasBridge will no longer synchronize their
state with the layer tree. In these cases it is necessary to call
setTheme() with an empty string to clear the theme association and
allow map updates with setLayers(), setLayerStyleOverrides(), or via
:py:class:`QgsLayerTreeMapCanvasBridge`.

If an empty string is passed then the current theme association will be
cleared.

.. versionadded:: 3.0

.. seealso:: :py:func:`theme`
%End

    QString theme() const;
%Docstring
Returns the map's theme shown in the canvas, if set.

.. versionadded:: 3.0

.. seealso:: :py:func:`setTheme`
%End

    const QgsMapToPixel *getCoordinateTransform();
%Docstring
Get the current coordinate transform
%End

    bool isDrawing();
%Docstring
Find out whether rendering is in progress
%End

    QgsMapLayer *currentLayer();
%Docstring
returns current layer (set by legend widget)
%End

    void setWheelFactor( double factor );
%Docstring
set wheel zoom factor (should be greater than 1)
%End

    void zoomScale( double scale );
%Docstring
Zooms the canvas to a specific ``scale``.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
%End

    void zoomByFactor( double scaleFactor, const QgsPointXY *center = 0 );
%Docstring
Zoom with the factor supplied. Factor > 1 zooms out, interval (0,1) zooms in
If point is given, re-center on it
%End

    void zoomWithCenter( int x, int y, bool zoomIn );
%Docstring
Zooms in/out with a given center
%End

    void zoomToFeatureExtent( QgsRectangle &rect );
%Docstring
Zooms to feature extent. Adds a small margin around the extent
and does a pan if rect is empty (point extent)
%End

    bool scaleLocked() const;
%Docstring
Returns whether the scale is locked, so zooming can be performed using magnication.

.. versionadded:: 2.16

.. seealso:: :py:func:`setScaleLocked`
%End

    void enableAntiAliasing( bool flag );
%Docstring
used to determine if anti-aliasing is enabled or not
%End

    bool antiAliasingEnabled() const;
%Docstring
true if antialising is enabled
%End

    void enableMapTileRendering( bool flag );
%Docstring
sets map tile rendering flag
%End

    void panActionEnd( QPoint releasePoint );
%Docstring
Ends pan action and redraws the canvas.
%End

    void panAction( QMouseEvent *event );
%Docstring
Called when mouse is moving and pan is activated
%End

    QPoint mouseLastXY();
%Docstring
returns last position of mouse cursor
%End

    void setPreviewModeEnabled( bool previewEnabled );
%Docstring
Enables a preview mode for the map canvas

:param previewEnabled: set to true to enable a preview mode

.. seealso:: :py:func:`setPreviewMode`

.. versionadded:: 2.3
%End

    bool previewModeEnabled() const;
%Docstring
Returns whether a preview mode is enabled for the map canvas

:return: true if a preview mode is currently enabled

.. seealso:: :py:func:`setPreviewModeEnabled`

.. seealso:: :py:func:`previewMode`

.. versionadded:: 2.3
%End

    void setPreviewMode( QgsPreviewEffect::PreviewMode mode );
%Docstring
Sets a preview mode for the map canvas. This setting only has an effect if
previewModeEnabled is true.

:param mode: preview mode for the canvas

.. seealso:: :py:func:`previewMode`

.. seealso:: :py:func:`setPreviewModeEnabled`

.. seealso:: :py:func:`previewModeEnabled`

.. versionadded:: 2.3
%End

    QgsPreviewEffect::PreviewMode previewMode() const;
%Docstring
Returns the current preview mode for the map canvas. This setting only has an effect if
previewModeEnabled is true.

:return: preview mode for map canvas

.. seealso:: :py:func:`setPreviewMode`

.. seealso:: :py:func:`previewModeEnabled`

.. versionadded:: 2.3
%End

    QgsSnappingUtils *snappingUtils() const;
%Docstring
Return snapping utility class that is associated with map canvas.
If no snapping utils instance has been associated previously, an internal will be created for convenience
(so map tools do not need to test for existence of the instance).

Main canvas in QGIS returns an instance which is always up-to-date with the project's snapping configuration.

.. versionadded:: 2.8
%End

    void setSnappingUtils( QgsSnappingUtils *utils );
%Docstring
Assign an instance of snapping utils to the map canvas.
The instance is not owned by the canvas, so it is possible to use one instance in multiple canvases.

For main canvas in QGIS, do not associate a different instance from the existing one (it is updated from
the project's snapping configuration).

.. versionadded:: 2.8
%End

    void setExpressionContextScope( const QgsExpressionContextScope &scope );
%Docstring
Sets an expression context scope for the map canvas. This scope is injected into the expression
context used for rendering the map, and can be used to apply specific variable overrides for
expression evaluation for the map canvas render. This method will overwrite the existing expression
context scope for the canvas.

:param scope: new expression context scope

.. versionadded:: 2.12

.. seealso:: :py:func:`expressionContextScope`
%End

    QgsExpressionContextScope &expressionContextScope();
%Docstring
Returns a reference to the expression context scope for the map canvas. This scope is injected
into the expression context used for rendering the map, and can be used to apply specific variable
overrides for expression evaluation for the map canvas render.

.. versionadded:: 2.12

.. seealso:: :py:func:`setExpressionContextScope`
%End


    void setSegmentationTolerance( double tolerance );
%Docstring
Sets the segmentation tolerance applied when rendering curved geometries

:param tolerance: the segmentation tolerance*
%End

    void setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType type );
%Docstring
Sets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)

:param type: the segmentation tolerance typename*
%End

    QList< QgsMapCanvasAnnotationItem *> annotationItems() const;
%Docstring
Returns a list of all annotation items in the canvas.

.. versionadded:: 3.0
%End

    bool annotationsVisible() const;
%Docstring
Returns true if annotations are visible within the map canvas.

.. versionadded:: 3.0

.. seealso:: :py:func:`setAnnotationsVisible`
%End

    void setAnnotationsVisible( bool visible );
%Docstring
Sets whether annotations are ``visible`` in the canvas.

.. versionadded:: 3.0

.. seealso:: :py:func:`annotationsVisible`
%End

    void setLabelingEngineSettings( const QgsLabelingEngineSettings &settings );
%Docstring
Sets global labeling engine settings in the internal map settings

.. versionadded:: 3.0
%End

    const QgsLabelingEngineSettings &labelingEngineSettings() const;
%Docstring
Returns global labeling engine settings from the internal map settings

.. versionadded:: 3.0
%End

    bool previewJobsEnabled() const;
%Docstring
Returns true if canvas map preview jobs (low priority render jobs which render portions
of the view just outside of the canvas extent, to allow preview of these
out-of-canvas areas when panning or zooming out the map) are enabled
for the canvas.

.. seealso:: :py:func:`setPreviewJobsEnabled`

.. versionadded:: 3.0
%End

    void setPreviewJobsEnabled( bool enabled );
%Docstring
Sets whether canvas map preview jobs (low priority render jobs which render portions
of the view just outside of the canvas extent, to allow preview of these
out-of-canvas areas when panning or zooming out the map) are ``enabled``
for the canvas.

.. seealso:: :py:func:`previewJobsEnabled`

.. versionadded:: 3.0
%End

  public slots:

    void refresh();
%Docstring
Repaints the canvas map
%End

    void selectionChangedSlot();
%Docstring
Receives signal about selection change, and pass it on with layer info
%End

    void saveAsImage( const QString &fileName, QPixmap *QPixmap = 0, const QString & = "PNG" );
%Docstring
Save the convtents of the map canvas to disk as an image
%End

    void layerStateChange();
%Docstring
This slot is connected to the visibility change of one or more layers
%End

    void setRenderFlag( bool flag );
%Docstring
Sets whether a user has disabled canvas renders via the GUI.

:param flag: set to false to indicate that user has disabled renders

.. seealso:: :py:func:`renderFlag`

.. seealso:: :py:func:`freeze`
while setRenderFlag() should only be used when users disable rendering via GUI.
%End

    void stopRendering();
%Docstring
stop rendering (if there is any right now)

.. versionadded:: 2.4
%End

    void readProject( const QDomDocument & );
%Docstring
called to read map canvas settings from project
%End

    void writeProject( QDomDocument & );
%Docstring
called to write map canvas settings to project
%End


    void setMagnificationFactor( double factor );
%Docstring
Sets the factor of magnification to apply to the map canvas. Indeed, we
increase/decrease the DPI of the map settings according to this factor
in order to render marker point, labels, ... bigger.

.. versionadded:: 2.16
%End

    void setScaleLocked( bool isLocked );
%Docstring
Lock the scale, so zooming can be performed using magnication

.. versionadded:: 2.16

.. seealso:: :py:func:`scaleLocked`
%End

    void zoomIn();
%Docstring
Zoom in with fixed factor
%End

    void zoomOut();
%Docstring
Zoom out with fixed factor
%End

  signals:

    void xyCoordinates( const QgsPointXY &p );
%Docstring
Emits current mouse position

.. note::

   changed in 1.3 *
%End

    void scaleChanged( double );
%Docstring
Emitted when the scale of the map changes
%End

    void extentsChanged();
%Docstring
Emitted when the extents of the map change
%End

    void rotationChanged( double );
%Docstring
Emitted when the rotation of the map changes

.. versionadded:: 2.8
%End

    void magnificationChanged( double );
%Docstring
Emitted when the scale of the map changes

.. versionadded:: 2.16
%End

    void canvasColorChanged();
%Docstring
Emitted when canvas background color changes

.. versionadded:: 3.0
%End


    void renderComplete( QPainter * );
%Docstring
TODO: deprecate when decorations are reimplemented as map canvas items
- anything related to rendering progress is not visible outside of map canvas
- additional drawing shall be done directly within the renderer job or independently as a map canvas item
%End

    void mapCanvasRefreshed();
%Docstring
Emitted when canvas finished a refresh request.
%End

    void renderStarting();
%Docstring
Emitted when the canvas is about to be rendered.
%End

    void layersChanged();
%Docstring
Emitted when a new set of layers has been received
%End

    void keyPressed( QKeyEvent *e );
%Docstring
Emit key press event
%End

    void keyReleased( QKeyEvent *e );
%Docstring
Emit key release event
%End

    void mapToolSet( QgsMapTool *newTool, QgsMapTool *oldTool );
%Docstring
Emit map tool changed with the old tool

.. versionadded:: 2.3
%End

    void selectionChanged( QgsMapLayer *layer );
%Docstring
Emitted when selection in any layer gets changed
%End

    void zoomLastStatusChanged( bool );
%Docstring
Emitted when zoom last status changed
%End

    void zoomNextStatusChanged( bool );
%Docstring
Emitted when zoom next status changed
%End

    void destinationCrsChanged();
%Docstring
Emitted when map CRS has changed

.. versionadded:: 2.4
%End

    void transformContextChanged();
%Docstring
Emitted when the canvas transform context is changed.

.. versionadded:: 3.0
%End

    void currentLayerChanged( QgsMapLayer *layer );
%Docstring
Emitted when the current layer is changed

.. versionadded:: 2.8
%End

    void layerStyleOverridesChanged();
%Docstring
Emitted when the configuration of overridden layer styles changes

.. versionadded:: 2.12
%End

    void themeChanged( const QString &theme );
%Docstring
Emitted when the canvas has been assigned a different map theme.

.. seealso:: :py:func:`setTheme`

.. versionadded:: 3.0
%End

    void messageEmitted( const QString &title, const QString &message, QgsMessageBar::MessageLevel = QgsMessageBar::INFO );
%Docstring
emit a message (usually to be displayed in a message bar)
%End

  protected:

    virtual bool event( QEvent *e );

%Docstring
Overridden standard event to be gestures aware
%End

    virtual void keyPressEvent( QKeyEvent *e );

%Docstring
Overridden key press event
%End

    virtual void keyReleaseEvent( QKeyEvent *e );

%Docstring
Overridden key release event
%End

    virtual void mouseDoubleClickEvent( QMouseEvent *e );

%Docstring
Overridden mouse double-click event
%End

    virtual void mouseMoveEvent( QMouseEvent *e );

%Docstring
Overridden mouse move event
%End

    virtual void mousePressEvent( QMouseEvent *e );

%Docstring
Overridden mouse press event
%End

    virtual void mouseReleaseEvent( QMouseEvent *e );

%Docstring
Overridden mouse release event
%End

    virtual void wheelEvent( QWheelEvent *e );

%Docstring
Overridden mouse wheel event
%End

    virtual void resizeEvent( QResizeEvent *e );

%Docstring
Overridden resize event
%End

    virtual void paintEvent( QPaintEvent *e );

%Docstring
Overridden paint event
%End

    virtual void dragEnterEvent( QDragEnterEvent *e );

%Docstring
Overridden drag enter event
%End

    void moveCanvasContents( bool reset = false );
%Docstring
called when panning is in action, reset indicates end of panning
%End




  protected slots:
    void updateCanvasItemPositions();
%Docstring
called on resize or changed extent to notify canvas items to change their rectangle
%End

}; // class QgsMapCanvas


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmapcanvas.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
