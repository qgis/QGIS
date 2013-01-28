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

  protected:
    void paintEvent( QPaintEvent* event );

  private:
    Direction mDirection;
};

#endif // QGSCOMPOSERRULER_H
