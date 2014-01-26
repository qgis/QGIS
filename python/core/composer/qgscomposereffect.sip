class QgsComposerEffect : QGraphicsEffect
{
%TypeHeaderCode
#include <qgscomposereffect.h>
%End
  public:
    QgsComposerEffect();
    ~QgsComposerEffect();

    void setCompositionMode( const QPainter::CompositionMode &compositionMode );

  protected:
    /** Called whenever source needs to be drawn */
    virtual void draw( QPainter *painter );
};
