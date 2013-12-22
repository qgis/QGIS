#ifndef QGSCOMPOSERRULER_H
#define QGSCOMPOSERRULER_H

#include "qgscomposeritem.h"
#include <QWidget>
class QgsComposition;
class QGraphicsLineItem;

/**A class to show paper scale and the current cursor position*/
class GUI_EXPORT QgsComposerRuler: public QWidget
{
    Q_OBJECT

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

    int rulerSize() { return mRulerMinSize; }

  protected:
    void paintEvent( QPaintEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );
    void mousePressEvent( QMouseEvent* event );

  private:
    static const int validScaleMultiples[];
    static const int validScaleMagnitudes[];

    Direction mDirection;
    QTransform mTransform;
    QPointF mMarkerPos;
    QgsComposition* mComposition; //reference to composition for paper size, nPages
    QGraphicsLineItem* mLineSnapItem;
    //items snapped to the current snap line
    QList< QPair< QgsComposerItem*, QgsComposerItem::ItemPositionMode > > mSnappedItems;

    QFont * mRulerFont;
    QFontMetrics * mRulerFontMetrics;
    double mScaleMinPixelsWidth;
    int mRulerMinSize;
    int mMinPixelsPerDivision;
    int mPixelsBetweenLineAndText;
    int mTextBaseline;
    int mMinSpacingVerticalLabels;

    void setSnapLinePosition( const QPointF& pos );

    //calculate optimum labeled units for ruler so that labels are a good distance apart
    int optimumScale( double minPixelDiff, int &magnitude, int &multiple );
    //calculate number of small divisions for each ruler unit, ensuring that they
    //are sufficiently spaced
    int optimumNumberDivisions( double rulerScale, int scaleMultiple );

    //draws vertical text on a painter
    void drawRotatedText( QPainter *painter, QPointF pos, const QString &text );

    /* Draws small ruler divisions
     * Starting at startPos in mm, for numDivisions divisions, with major division spacing of rulerScale (in mm)
     * Stop drawing if position exceeds maxPos
     */
    void drawSmallDivisions( QPainter *painter, double startPos, int numDivisions, double rulerScale, double maxPos = 0 );

    //draw current marker pos on ruler
    void drawMarkerPos( QPainter *painter );

  signals:
    /**Is emitted when mouse cursor coordinates change*/
    void cursorPosChanged( QPointF );

};

#endif // QGSCOMPOSERRULER_H
