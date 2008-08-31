
class QgsMapToolPan : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptoolpan.h>
%End

  public:
    //! constructor
    QgsMapToolPan(QgsMapCanvas* canvas);
    
    //! Overridden mouse move event
    virtual void canvasMoveEvent(QMouseEvent * e);
  
    //! Overridden mouse press event
    virtual void canvasPressEvent(QMouseEvent * e);
  
    //! Overridden mouse release event
    virtual void canvasReleaseEvent(QMouseEvent * e);
    
    virtual bool isTransient();
    
};

