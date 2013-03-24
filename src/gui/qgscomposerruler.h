#ifndef QGSCOMPOSERRULER_H
#define QGSCOMPOSERRULER_H

#include "qgscomposeritem.h"
#include <QWidget>
class QgsComposition;
class QGraphicsLineItem;

/**A class to show paper scale and the current cursor position*/
class GUI_EXPORT QgsComposerRuler: public QWidget
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
    //items snapped to the current snap line
    QList< QPair< QgsComposerItem*, QgsComposerItem::ItemPositionMode > > mSnappedItems;

    void setSnapLinePosition( const QPointF& pos );
};

#endif // QGSCOMPOSERRULER_H
