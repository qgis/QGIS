#ifndef QGSCOMPOSERRULER_H
#define QGSCOMPOSERRULER_H

#include <QWidget>

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

  protected:
    void paintEvent( QPaintEvent* event );

  private:
    Direction mDirection;
    QTransform mTransform;
};

#endif // QGSCOMPOSERRULER_H
