
class QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptool.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsMapToolZoom*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapToolZoom;
  else if (dynamic_cast<QgsMapToolPan*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapToolPan;
  else
    sipClass = NULL;
%End

  public:
    
    //! virtual destructor
    virtual ~QgsMapTool();
    
    //! Mouse move event for overriding
    virtual void canvasMoveEvent(QMouseEvent * e);

    //! Mouse press event for overriding
    virtual void canvasPressEvent(QMouseEvent * e);

    //! Mouse release event for overriding
    virtual void canvasReleaseEvent(QMouseEvent * e);
    
    //! Called when rendering has finished
    virtual void renderComplete();
    
    /** Use this to associate a button, toolbutton, menu entry etc
     * that inherits qaction to this maptool. Then when the setMapTool
     * method of mapcanvas is called the action state will be set to on.
     * Usually this will cause e.g. a toolbutton to appear pressed in and
     * the previously used toolbutton to pop out. */
    void setAction(QAction* action);
    
    QAction* action();
    
    /** Check whether this MapTool performs a zoom or pan operation.
     * If it does, we will be able to perform the zoom  and then 
     * resume operations with the original / previously used tool.*/
    virtual bool isZoomTool();
    
    //! called when set as currently active map tool
    virtual void activate();
    
    //! called when map tool is being deactivated
    virtual void deactivate();
    
  protected:

    //! constructor takes map canvas as a parameter
    QgsMapTool(QgsMapCanvas* canvas);
        
    //! transformation from screen coordinates to map coordinates
    QgsPoint toMapCoords(const QPoint& point);
    
    //! transformation from screen coordinates to layer's coordinates
    QgsPoint toLayerCoords(QgsMapLayer* layer, const QPoint& point);
    
    //! trnasformation from map coordinates to layer's coordinates
    QgsPoint toLayerCoords(QgsMapLayer* layer, const QgsPoint& point);
    
    //! trnasformation of the rect from map coordinates to layer's coordinates
    QgsRect toLayerCoords(QgsMapLayer* layer, const QgsRect& rect);

    //! transformation from map coordinates to screen coordinates
    QPoint toCanvasCoords(const QgsPoint& point);
   
   
};

