class QgsMapCanvasAnnotationItem : public QObject, public QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsmapcanvasannotationitem.h>
%End

  public:

    enum MouseMoveAction
    {
      NoAction,
      MoveMapPosition,
      MoveFramePosition,
      ResizeFrameUp,
      ResizeFrameDown,
      ResizeFrameLeft,
      ResizeFrameRight,
      ResizeFrameLeftUp,
      ResizeFrameRightUp,
      ResizeFrameLeftDown,
      ResizeFrameRightDown,
    };

    QgsMapCanvasAnnotationItem( QgsAnnotation* annotation /Transfer/, QgsMapCanvas* mapCanvas /TransferThis/ );
    ~QgsMapCanvasAnnotationItem();

    //const QgsAnnotation* annotation() const { return mAnnotation; }
    QgsAnnotation* annotation();

    void updatePosition();

    QRectF boundingRect() const;

    void paint( QPainter * painter );

    /**
     * Returns the mouse move behavior for a given position in scene coordinates
     */
    MouseMoveAction moveActionForPosition( QPointF pos ) const;

    /**
     * Returns matching cursor shape for a mouse move action.
     */
    Qt::CursorShape cursorShapeForAction( MouseMoveAction moveAction ) const;

};
