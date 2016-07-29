
class QgsMapToolZoom : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptoolzoom.h>
%End

  public:
    //! constructor
    QgsMapToolZoom( QgsMapCanvas* canvas, bool zoomOut );

    ~QgsMapToolZoom();

    virtual Flags flags() const;

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e );

    //! Flag to indicate a map canvas drag operation is taking place
    virtual void deactivate();
};

