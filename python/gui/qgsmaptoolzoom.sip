
class QgsMapToolZoom : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptoolzoom.h>
%End

  public:
    //! constructor
    QgsMapToolZoom(QgsMapCanvas* canvas, bool zoomOut);
          
    //! Overridden mouse move event
    virtual void canvasMoveEvent(QMouseEvent * e);
  
    //! Overridden mouse press event
    virtual void canvasPressEvent(QMouseEvent * e);
  
    //! Overridden mouse release event
    virtual void canvasReleaseEvent(QMouseEvent * e);

    virtual bool isTransient();
    
};

