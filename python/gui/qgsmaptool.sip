
%ModuleHeaderCode
// fix to allow compilation with sip 4.7 that for some reason
// doesn't add these includes to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsmaptoolzoom.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolemitpoint.h>
#include <qgsmaptoolidentify.h>
%End

class QgsMapTool : QObject
{
%TypeHeaderCode
#include <qgsmaptool.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsMapToolZoom*>(sipCpp) != NULL)
    sipType = sipType_QgsMapToolZoom;
  else if (dynamic_cast<QgsMapToolPan*>(sipCpp) != NULL)
    sipType = sipType_QgsMapToolPan;
  else if (dynamic_cast<QgsMapToolEmitPoint*>(sipCpp) != NULL)
    sipType = sipType_QgsMapToolEmitPoint;
  else if (dynamic_cast<QgsMapToolIdentify*>(sipCpp) != NULL)
    sipType = sipType_QgsMapToolIdentify;
  else
    sipType = NULL;
%End

  public:

    //! Enumeration of flags that adjust the way the map tool operates
    //! @note added in QGIS 2.16
    enum Flag
    {
      Transient, /*!< Indicates that this map tool performs a transient (one-off) operation.
                               If it does, the tool can be operated once and then a previous map
                               tool automatically restored. */
      EditTool, /*!< Map tool is an edit tool, which can only be used when layer is editable*/
      AllowZoomRect, /*!< Allow zooming by rectangle (by holding shift and dragging) while the tool is active*/
    };
    typedef QFlags<QgsMapTool::Flag> Flags;

    /** Returns the flags for the map tool.
     * @note added in QGIS 2.16
     */
    virtual Flags flags() const;

    //! virtual destructor
    virtual ~QgsMapTool();

    //! Mouse move event for overriding. Default implementation does nothing.
    virtual void canvasMoveEvent( QgsMapMouseEvent* e );

    //! Mouse double click event for overriding. Default implementation does nothing.
    virtual void canvasDoubleClickEvent( QgsMapMouseEvent* e );

    //! Mouse press event for overriding. Default implementation does nothing.
    virtual void canvasPressEvent( QgsMapMouseEvent* e );

    //! Mouse release event for overriding. Default implementation does nothing.
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e );

    //! Mouse wheel event for overriding. Default implementation does nothing.
    virtual void wheelEvent( QWheelEvent* e );

    //! Key event for overriding. Default implementation does nothing.
    virtual void keyPressEvent( QKeyEvent* e );

    //! Key event for overriding. Default implementation does nothing.
    virtual void keyReleaseEvent( QKeyEvent* e );

%If (HAVE_TOUCH)
    //! gesture event for overriding. Default implementation does nothing.
    virtual bool gestureEvent( QGestureEvent* e );
%End

    //! Called when rendering has finished. Default implementation does nothing.
    //! @deprecated since 2.4 - not called anymore - map tools must not directly depend on rendering progress
    virtual void renderComplete() /Deprecated/;


    /** Use this to associate a QAction to this maptool. Then when the setMapTool
     * method of mapcanvas is called the action state will be set to on.
     * Usually this will cause e.g. a toolbutton to appear pressed in and
     * the previously used toolbutton to pop out. */
    void setAction( QAction* action );

    /** Return associated action with map tool or NULL if no action is associated */
    QAction* action();

    /** Use this to associate a button to this maptool. It has the same meaning
     * as setAction() function except it works with a button instead of an QAction. */
    void setButton( QAbstractButton* button );

    /** Return associated button with map tool or NULL if no button is associated */
    QAbstractButton* button();

    /** Set a user defined cursor */
    virtual void setCursor( const QCursor& cursor );

    /** Check whether this MapTool performs a zoom or pan operation.
     * If it does, we will be able to perform the zoom  and then
     * resume operations with the original / previously used tool.
     * @deprecated use flags() instead
    */
    virtual bool isTransient() const /Deprecated/;

    /** Check whether this MapTool performs an edit operation.
     * If it does, we will deactivate it when editing is turned off.
     * @deprecated use flags() instead
     */
    virtual bool isEditTool() const /Deprecated/;

    //! called when set as currently active map tool
    virtual void activate();

    //! called when map tool is being deactivated
    virtual void deactivate();

    //! returns pointer to the tool's map canvas
    QgsMapCanvas* canvas();

    //! Emit map tool changed with the old tool
    //! @note added in 2.3
    QString toolName();

    /** Get search radius in mm. Used by identify, tip etc.
     *  The values is currently set in identify tool options (move somewhere else?)
     *  and defaults to QGis::DEFAULT_SEARCH_RADIUS_MM.
     *  @note added in 2.3 */
    static double searchRadiusMM();

    /** Get search radius in map units for given context. Used by identify, tip etc.
     *  The values is calculated from searchRadiusMM().
     *  @note added in 2.3 */
    static double searchRadiusMU( const QgsRenderContext& context );

    /** Get search radius in map units for given canvas. Used by identify, tip etc.
     *  The values is calculated from searchRadiusMM().
     *  @note added in 2.3 */
    static double searchRadiusMU( QgsMapCanvas * canvas );

  signals:
    //! emit a message
    void messageEmitted( const QString& message, QgsMessageBar::MessageLevel = QgsMessageBar::INFO );

    //! emit signal to clear previous message
    void messageDiscarded();

    //! signal emitted once the map tool is activated
    void activated();

    //! signal emitted once the map tool is deactivated
    void deactivated();

  protected:

    //! constructor takes map canvas as a parameter
    QgsMapTool( QgsMapCanvas* canvas /TransferThis/ );

    //! transformation from screen coordinates to map coordinates
    QgsPoint toMapCoordinates( QPoint point );

    //! transformation from screen coordinates to layer's coordinates
    QgsPoint toLayerCoordinates( QgsMapLayer* layer, QPoint point );

    //! transformation from map coordinates to layer's coordinates
    QgsPoint toLayerCoordinates( QgsMapLayer* layer, const QgsPoint& point );

    //!transformation from layer's coordinates to map coordinates (which is different in case reprojection is used)
    QgsPoint toMapCoordinates( QgsMapLayer* layer, const QgsPoint& point );

    //!transformation from layer's coordinates to map coordinates (which is different in case reprojection is used)
    //! @note available in python bindings as toMapCoordinatesV2
    QgsPointV2 toMapCoordinates( QgsMapLayer* layer, const QgsPointV2 &point ) /PyName=toMapCoordinatesV2/;

    //! trnasformation of the rect from map coordinates to layer's coordinates
    QgsRectangle toLayerCoordinates( QgsMapLayer* layer, const QgsRectangle& rect );

    //! transformation from map coordinates to screen coordinates
    QPoint toCanvasCoordinates( const QgsPoint& point );


};


QFlags<QgsMapTool::Flag> operator|(QgsMapTool::Flag f1, QFlags<QgsMapTool::Flag> f2);
