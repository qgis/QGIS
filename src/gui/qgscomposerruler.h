#ifndef QGSCOMPOSERRULER_H
#define QGSCOMPOSERRULER_H

#include <QWidget>
class QgsComposition;
class QGraphicsLineItem;

/**A class to show paper scale and the current cursor position*/
class QgsComposerRuler: public QWidget
{
  public:
    enum Direction
    {
      Horizontal = 0,
      Vertical
    };

    QgsComposerRuler( QgsComposerRuler::Direction d );
    ~QgsComposerRuler();

    QSize minimumSizeHint() const;

    void setSceneTransform( const QTransform& transform );
    void updateMarker( const QPointF& pos ) { mMarkerPos = pos; repaint(); }

    void setComposition( QgsComposition* c ) { mComposition = c; }
    QgsComposition* composition() { return mComposition; }

  protected:
    void paintEvent( QPaintEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );
    void mousePressEvent( QMouseEvent* event );

  private:
    Direction mDirection;
    QTransform mTransform;
    QPointF mMarkerPos;
    QgsComposition* mComposition; //reference to composition for paper size, nPages
    QGraphicsLineItem* mLineSnapItem;

    void setSnapLinePosition( const QPointF& pos );
    static QGraphicsLineItem* createLineSnapItem();
};

#endif // QGSCOMPOSERRULER_H
