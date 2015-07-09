
%ModuleHeaderCode
// fix to allow compilation with sip 4.7 that for some reason
// doesn't add these includes to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsmaptoolzoom.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolemitpoint.h>
#include <qgsmaptoolidentify.h>
%End

%Feature HAVE_TOUCH

class QgsMapTool : QObject
{
%TypeHeaderCode
#include <qgsmaptool.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsMapToolZoom*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapToolZoom;
  else if (dynamic_cast<QgsMapToolPan*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapToolPan;
  else if (dynamic_cast<QgsMapToolEmitPoint*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapToolEmitPoint;
  else if (dynamic_cast<QgsMapToolIdentify*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapToolIdentify;
  else
    sipClass = NULL;
%End

  public:

    //! virtual destructor
    virtual ~QgsMapTool();

    //! Mouse move event for overriding. Default implementation does nothing.
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Mouse double click event for overriding. Default implementation does nothing.
    virtual void canvasDoubleClickEvent( QMouseEvent * e );

    //! Mouse press event for overriding. Default implementation does nothing.
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Mouse release event for overriding. Default implementation does nothing.
    virtual void canvasReleaseEvent( QMouseEvent * e );

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
    virtual void setCursor( QCursor cursor );

    /** Check whether this MapTool performs a zoom or pan operation.
     * If it does, we will be able to perform the zoom  and then
     * resume operations with the original / previously used tool.*/
    virtual bool isTransient();

    /** Check whether this MapTool performs an edit operation.
     * If it does, we will deactivate it when editing is turned off
     */
    virtual bool isEditTool();

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
    void messageEmitted( QString message, QgsMessageBar::MessageLevel = QgsMessageBar::INFO );

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
    QgsPoint toMapCoordinates( const QPoint& point );

    //! transformation from screen coordinates to layer's coordinates
    QgsPoint toLayerCoordinates( QgsMapLayer* layer, const QPoint& point );

    //! transformation from map coordinates to layer's coordinates
    QgsPoint toLayerCoordinates( QgsMapLayer* layer, const QgsPoint& point );

    //!transformation from layer's coordinates to map coordinates (which is different in case reprojection is used)
    QgsPoint toMapCoordinates( QgsMapLayer* layer, const QgsPoint& point );

    //! trnasformation of the rect from map coordinates to layer's coordinates
    QgsRectangle toLayerCoordinates( QgsMapLayer* layer, const QgsRectangle& rect );

    //! transformation from map coordinates to screen coordinates
    QPoint toCanvasCoordinates( const QgsPoint& point );


};
