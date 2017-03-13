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
    sipType = sipType_QgsMapCanvas;
  else
    sipType = NULL;
%End

  public:

    //! Constructor
    QgsMapCanvas( QWidget *parent /TransferThis/ = 0 );
    ~QgsMapCanvas();
    double magnificationFactor() const;
    void setLayers( const QList<QgsMapLayer *> &layers );
    void setCurrentLayer( QgsMapLayer *layer );
    const QgsMapSettings &mapSettings() const /KeepReference/;
    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
    const QgsLabelingResults *labelingResults() const;
    void setCachingEnabled( bool enabled );
    bool isCachingEnabled() const;
    void clearCache();
    void refreshAllLayers();
    void waitWhileRendering();
    void setParallelRenderingEnabled( bool enabled );
    bool isParallelRenderingEnabled() const;
    void setMapUpdateInterval( int timeMilliseconds );
    int mapUpdateInterval() const;
    double scale();
    double mapUnitsPerPixel() const;
    QgsRectangle extent() const;
    QgsRectangle fullExtent() const;
    void setExtent( const QgsRectangle &r, bool magnified = false );
    double rotation() const;
    void setRotation( double degrees );
    void setCenter( const QgsPoint &center );
    QgsPoint center() const;
    void zoomToFullExtent();
    void zoomToPreviousExtent();
    void zoomToNextExtent();
    void clearExtentHistory();
    void zoomToSelected( QgsVectorLayer *layer = 0 );
    void zoomToFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids );
    void panToFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids );
    void panToSelected( QgsVectorLayer *layer = 0 );
    void setMapTool( QgsMapTool *mapTool );
    void unsetMapTool( QgsMapTool *mapTool );
    QgsMapTool *mapTool();
    void setCanvasColor( const QColor &_newVal );
    QColor canvasColor() const;
    void setSelectionColor( const QColor &color );
    void updateScale();
    QgsMapLayer *layer( int index );
    int layerCount() const;
    QList<QgsMapLayer *> layers() const;
    void freeze( bool frozen = true );
    bool isFrozen() const;
    bool renderFlag() const;
    QgsUnitTypes::DistanceUnit mapUnits() const;
    QMap<QString, QString> layerStyleOverrides() const;
    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );
    void setTheme( const QString &theme );
    QString theme() const;
    const QgsMapToPixel *getCoordinateTransform();
    bool isDrawing();
    QgsMapLayer *currentLayer();
    void setWheelFactor( double factor );
    void zoomScale( double scale );
    void zoomByFactor( double scaleFactor, const QgsPoint *center = 0 );
    void zoomWithCenter( int x, int y, bool zoomIn );
    void zoomToFeatureExtent( QgsRectangle &rect );
    bool scaleLocked() const;
    void enableAntiAliasing( bool flag );
    bool antiAliasingEnabled() const;
    void enableMapTileRendering( bool flag );
    void panActionEnd( QPoint releasePoint );
    void panAction( QMouseEvent *event );
    QPoint mouseLastXY();
    void setPreviewModeEnabled( bool previewEnabled );
    bool previewModeEnabled() const;
    void setPreviewMode( QgsPreviewEffect::PreviewMode mode );
    QgsPreviewEffect::PreviewMode previewMode() const;
    QgsSnappingUtils *snappingUtils() const;
    void setSnappingUtils( QgsSnappingUtils *utils );
    void setExpressionContextScope( const QgsExpressionContextScope &scope );
    QgsExpressionContextScope &expressionContextScope();
    // const QgsExpressionContextScope &expressionContextScope() const;
    void setSegmentationTolerance( double tolerance );
    void setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType type );

    QList< QgsMapCanvasAnnotationItem *> annotationItems() const;
    bool annotationsVisible() const;
    void setAnnotationsVisible( bool visible );

  public slots:

    //! Repaints the canvas map
    void refresh();

    //! Receives signal about selection change, and pass it on with layer info
    void selectionChangedSlot();

    //! Save the convtents of the map canvas to disk as an image
    void saveAsImage( const QString& fileName, QPixmap * QPixmap = 0, const QString& = "PNG" );

    //! This slot is connected to the visibility change of one or more layers
    void layerStateChange();

    //! This slot is connected to the layer's CRS change
    void layerCrsChange();
    void setRenderFlag( bool flag );

    //! stop rendering (if there is any right now)
    //! @note added in 2.4
    void stopRendering();

    //! called to read map canvas settings from project
    void readProject( const QDomDocument & );

    //! called to write map canvas settings to project
    void writeProject( QDomDocument & );

    //! ask user about datum transformation
    void getDatumTransformInfo( const QgsMapLayer* ml, const QString& srcAuthId, const QString& destAuthId );

    //! Sets the factor of magnification to apply to the map canvas. Indeed, we
    //! increase/decrease the DPI of the map settings according to this factor
    //! in order to render marker point, labels, ... bigger.
    //! @note added in 2.16
    void setMagnificationFactor( double factor );

    //! Lock the scale, so zooming can be performed using magnication
    //! @note added in 2.16
    //! @see scaleLocked()
    void setScaleLocked( bool isLocked );

    //! Zoom in with fixed factor
    void zoomIn();

    //! Zoom out with fixed factor
    void zoomOut();

  signals:

    /** Emits current mouse position
        \note changed in 1.3 */
    void xyCoordinates( const QgsPoint &p );

    //! Emitted when the scale of the map changes
    void scaleChanged( double );

    //! Emitted when the extents of the map change
    void extentsChanged();

    //! Emitted when the rotation of the map changes
    //! @note added in 2.8
    void rotationChanged( double );

    //! Emitted when the scale of the map changes
    //! @note added in 2.16
    void magnificationChanged( double );

    /** Emitted when the canvas has rendered.
     * Passes a pointer to the painter on which the map was drawn. This is
     * useful for plugins that wish to draw on the map after it has been
     * rendered.  Passing the painter allows plugins to work when the map is
     * being rendered onto a pixmap other than the mapCanvas own pixmap member.
     *
     */
    //! TODO: deprecate when decorations are reimplemented as map canvas items
    //! - anything related to rendering progress is not visible outside of map canvas
    //! - additional drawing shall be done directly within the renderer job or independently as a map canvas item
    void renderComplete( QPainter * );

    // ### QGIS 3: renamte to mapRefreshFinished()
    //! Emitted when canvas finished a refresh request.
    void mapCanvasRefreshed();

    // ### QGIS 3: rename to mapRefreshStarted()
    //! Emitted when the canvas is about to be rendered.
    void renderStarting();

    //! Emitted when a new set of layers has been received
    void layersChanged();

    //! Emit key press event
    void keyPressed( QKeyEvent * e );

    //! Emit key release event
    void keyReleased( QKeyEvent * e );

    void mapToolSet( QgsMapTool *newTool, QgsMapTool* oldTool );

    // ### QGIS 3: remove the signal
    //! Emitted when selection in any layer gets changed
    void selectionChanged( QgsMapLayer * layer );

    //! Emitted when zoom last status changed
    void zoomLastStatusChanged( bool );

    //! Emitted when zoom next status changed
    void zoomNextStatusChanged( bool );

    //! Emitted when map CRS has changed
    //! @note added in 2.4
    void destinationCrsChanged();

    //! Emitted when the current layer is changed
    //! @note added in 2.8
    void currentLayerChanged( QgsMapLayer* layer );

    void layerStyleOverridesChanged();
    void themeChanged( const QString &theme );
    void messageEmitted( const QString &title, const QString &message, QgsMessageBar::MessageLevel = QgsMessageBar::INFO );

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

    //! Make sure the datum transform store is properly populated
    void updateDatumTransformEntries();

  private:
    void connectNotify( const char * signal );
}; // class QgsMapCanvas
