%If (HAVE_TOUCH)
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
    virtual void canvasMoveEvent( QgsMapMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent * e );

    //! Overridden Mouse double click event.
    virtual void canvasDoubleClickEvent( QgsMapMouseEvent * e );

    virtual bool isTransient();

    bool gestureEvent( QGestureEvent *event );
};
%End
