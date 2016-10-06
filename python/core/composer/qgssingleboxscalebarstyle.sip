class QgsSingleBoxScaleBarStyle : QgsScaleBarStyle
{
%TypeHeaderCode
#include <qgssingleboxscalebarstyle.h>
%End
  public:
    QgsSingleBoxScaleBarStyle( const QgsComposerScaleBar* bar );
    ~QgsSingleBoxScaleBarStyle();

    QString name() const;

    /** Draw method
     @param p painter object
     @param xOffset x offset
     */
    void draw( QPainter* p, double xOffset = 0 ) const;
};
