
class QgsMapToolZoom : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptoolzoom.h>
%End

  public:
    //! constructor
    QgsMapToolZoom( QgsMapCanvas* canvas, bool zoomOut );

    ~QgsMapToolZoom();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    //! indicates whether we're zooming in or out
    virtual bool isTransient();

    //! Flag to indicate a map canvas drag operation is taking place
    virtual void deactivate();

};

