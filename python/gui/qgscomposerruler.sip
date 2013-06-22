class QgsComposerRuler : QWidget
{
%TypeHeaderCode
#include <qgscomposerruler.h>
%End
  public:
    enum Direction
    {
      Horizontal,
      Vertical
    };

    QgsComposerRuler( Direction d );
    ~QgsComposerRuler();

    QSize minimumSizeHint() const;

    void setSceneTransform( const QTransform& transform );
    void updateMarker( const QPointF& pos );

    void setComposition( QgsComposition* c );
    QgsComposition* composition();

  protected:
    void paintEvent( QPaintEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );
    void mousePressEvent( QMouseEvent* event );
};
