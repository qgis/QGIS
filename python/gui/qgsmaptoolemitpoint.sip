
class QgsMapToolEmitPoint : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptoolemitpoint.h>
%End

  public:
    //! constructor
    QgsMapToolEmitPoint( QgsMapCanvas* canvas );

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event - emits the signal
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

  signals:
    //! signal emitted on canvas click
    void canvasClicked( const QgsPoint& point, Qt::MouseButton button );
};
