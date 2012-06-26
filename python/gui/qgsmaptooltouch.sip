class QgsMapToolTouch : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptooltouch.h>
%End

  public:
    //! constructor
    QgsMapToolTouch( QgsMapCanvas* canvas );

    ~QgsMapToolTouch();

    void activate();
    void deactivate();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    //! Overridden Mouse double click event.
    virtual void canvasDoubleClickEvent( QMouseEvent * e );

    virtual bool isTransient() { return true; }
}
