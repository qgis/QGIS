


/** \class QgsMapCanvasLayer
  \brief class that stores additional layer's flags together with pointer to the layer
*/
class QgsMapCanvasLayer
{
%TypeHeaderCode
#include <qgsmapcanvas.h>
%End

public:
  QgsMapCanvasLayer(QgsMapLayer* layer, bool visible = TRUE, bool isInOverview = FALSE);

  void setVisible(bool visible);
  void setInOverview(bool isInOverview);

  bool isVisible() const;
  bool isInOverview() const;

  QgsMapLayer* layer();
  //const QgsMapLayer* layer() const;

};


/*! \class QgsMapCanvas
 * \brief Map canvas class for displaying all GIS data types.
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
    QgsMapCanvas(QWidget * parent /TransferThis/ = 0, const char *name = 0);

    //! Destructor
    ~QgsMapCanvas();

    void setLayerSet(QList<QgsMapCanvasLayer>& layers);

    void setCurrentLayer(QgsMapLayer* layer);

    void updateOverview();

    void enableOverviewMode(QgsMapOverviewCanvas* overview);

    QgsMapCanvasMap* map();

    QgsMapRenderer* mapRenderer() /Transfer/;

    //! Accessor for the canvas pixmap
    // @deprecated use canvasPaintDevice()
    QPixmap& canvasPixmap() /Deprecated/;

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
    void setExtent(const QgsRectangle & r);

    //! Zoom to the full extent of all layers
    void zoomToFullExtent();

    //! Zoom to the previous extent (view)
    void zoomToPreviousExtent();

    //! Zoom to the next extent (view)
    void zoomToNextExtent();

    // ! Clears the list of extents and sets current extent as first item
    void clearExtentHistory();

    /** Zoom to the extent of the selected features of current (vector) layer.
      Added in version 1.2: optionally specify different than current layer */
    void zoomToSelected(QgsVectorLayer* layer = NULL);

    /** Pan to the selected features of current (vector) layer keeping same extent.
      @note added in 2.0 */
    void panToSelected( QgsVectorLayer* layer = NULL );

    /** \brief Sets the map tool currently being used on the canvas */
    void setMapTool(QgsMapTool* mapTool);

    /** \brief Unset the current mapset tool or last non zoom tool if
     *         it the same as passed map tool pointer. The tool is not
     *         referenced/used any more, but the instance is not deleted
     *         by this method.
     */
    void unsetMapTool(QgsMapTool* mapTool);

    /**Returns the currently active tool*/
    QgsMapTool* mapTool();

    /** Write property of QColor bgColor. */
    virtual void setCanvasColor(const QColor & _newVal);

    /** Emits signal scalChanged to update scale in main window */
    void updateScale();

    /** Updates the full extent */
    void updateFullExtent();

    //! return the map layer at position index in the layer stack
    QgsMapLayer *layer(int index);

    //! return number of layers on the map
    int layerCount() const;

    //! return list of layers within map canvas. Added in v1.5
    QList<QgsMapLayer*> layers() const;

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
    void setMapUnits(QGis::UnitType mapUnits);
    //! Get the current canvas map units

    QGis::UnitType mapUnits() const;

    //! Get the current coordinate transform
    const QgsMapToPixel * getCoordinateTransform();

    //! true if canvas currently drawing
    bool isDrawing();

    //! returns current layer (set by legend widget)
    QgsMapLayer* currentLayer();

    //! set wheel action and zoom factor (should be greater than 1)
    void setWheelAction(WheelAction action, double factor = 2);

    //! Zoom in with fixed factor
    void zoomIn( );

    //! Zoom out with fixed factor
    void zoomOut( );

    //! Zoom to a specific scale
    // added in 1.5
    void zoomScale( double scale );

    //! Zoom with the factor supplied. Factor > 1 zooms in
    void zoomByFactor( double scaleFactor );

    //! Zooms in/out with a given center
    void zoomWithCenter(int x, int y, bool zoomIn);

    //! used to determine if anti-aliasing is enabled or not
    void enableAntiAliasing(bool theFlag);

    //! Select which Qt class to render with
    void useImageToRender(bool theFlag);

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

    //! Save the convtents of the map canvas to disk as an image
    void saveAsImage(QString theFileName,QPixmap * QPixmap=0, QString="PNG" );

    //! This slot is connected to the visibility change of one or more layers
    void layerStateChange();

    //! Whether to suppress rendering or not
    void setRenderFlag(bool theFlag);
    //! State of render suppression flag
    bool renderFlag();

    /** A simple helper method to find out if on the fly projections are enabled or not */
    bool hasCrsTransformEnabled();

    /** The map units may have changed, so cope with that */
    void mapUnitsChanged();

    /** updates pixmap on render progress */
    void updateMap();

    //! show whatever error is exposed by the QgsMapLayer.
    void showError(QgsMapLayer * mapLayer);

    //! called to read map canvas settings from project
    void readProject(const QDomDocument &);

    //! called to write map canvas settings to project
    void writeProject(QDomDocument &);

  signals:
    /** Let the owner know how far we are with render operations */
    void setProgress(int,int);

    /** emits current mouse position
      \note changed in 1.3 */
    void xyCoordinates(const QgsPoint &p);

    //! Emitted when the scale of the map changes
    void scaleChanged(QString);

    //! Emitted when the extents of the map change
    void extentsChanged();

    /** Emitted when the canvas has rendered.

     Passes a pointer to the painter on which the map was drawn. This is
     useful for plugins that wish to draw on the map after it has been
     rendered.  Passing the painter allows plugins to work when the map is
     being rendered onto a pixmap other than the mapCanvas own pixmap member.

    */
    void renderComplete(QPainter *);

    /** Emitted when the canvas is about to be rendered.
      \note Added in 1.5 */
    void renderStarting();

    //! Emitted when a new set of layers has been received
    void layersChanged();

    //! Emit key press event
    void keyPressed(QKeyEvent * e);

    //! Emit key release event
    void keyReleased(QKeyEvent * e);

    //! Emit map tool changed event
    void mapToolSet(QgsMapTool *tool);

    //! Emitted when zoom last status changed
    //! @note: this signal was added in version 1.4
    void zoomLastStatusChanged( bool );

    //! Emitted when zoom next status changed
    //! @note: this signal was added in version 1.4
    void zoomNextStatusChanged( bool );

  protected:

    //! Overridden key press event
    void keyPressEvent(QKeyEvent * e);

    //! Overridden key release event
    void keyReleaseEvent(QKeyEvent * e);

    //! Overridden mouse move event
    void mouseMoveEvent(QMouseEvent * e);

    //! Overridden mouse press event
    void mousePressEvent(QMouseEvent * e);

    //! Overridden mouse release event
    void mouseReleaseEvent(QMouseEvent * e);

    //! Overridden mouse wheel event
    void wheelEvent(QWheelEvent * e);

    //! Overridden resize event
    void resizeEvent(QResizeEvent * e);

    //! Overridden paint event
    void paintEvent(QPaintEvent *);

    //! called when panning is in action, reset indicates end of panning
    void moveCanvasContents(bool reset = FALSE);

    //! called on resize or changed extent to notify canvas items to change their rectangle
    void updateCanvasItemPositions();

}; // class QgsMapCanvas


