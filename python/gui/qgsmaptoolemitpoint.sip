
class QgsMapToolEmitPoint : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptoolemitpoint.h>
%End

  public:
    //! constructor
    QgsMapToolEmitPoint( QgsMapCanvas* canvas );

    virtual Flags flags() const;

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent * e );

    //! Overridden mouse press event - emits the signal
    virtual void canvasPressEvent( QgsMapMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent * e );

  signals:
    //! signal emitted on canvas click
    void canvasClicked( const QgsPoint& point, Qt::MouseButton button );
};
