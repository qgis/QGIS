
class QgsMapToolPan : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptoolpan.h>
%End

  public:
    //! constructor
    QgsMapToolPan( QgsMapCanvas* canvas );

    //! Mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e );

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e );

    virtual bool isTransient();
};

