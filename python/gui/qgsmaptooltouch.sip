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
    virtual void canvasMoveEvent( QgsMapMouseEvent *e );
    virtual void canvasReleaseEvent( QgsMapMouseEvent *e );
    virtual void canvasDoubleClickEvent( QgsMapMouseEvent *e );
    virtual Flags flags() const;
    bool gestureEvent( QGestureEvent *e );
};
%End
