
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
    //! Added in version 2.0
    virtual void wheelEvent( QWheelEvent * e );

    //! Key event for overriding. Default implementation does nothing.
    virtual void keyPressEvent( QKeyEvent* e );

    //! Key event for overriding. Default implementation does nothing.
    //! Added in version 1.1
    virtual void keyReleaseEvent( QKeyEvent* e );

%If (HAVE_TOUCH)
    //! gesture event for overriding. Default implementation does nothing.
    virtual bool gestureEvent( QGestureEvent* e );
%End

    //! Called when rendering has finished. Default implementation does nothing.
    virtual void renderComplete();


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
