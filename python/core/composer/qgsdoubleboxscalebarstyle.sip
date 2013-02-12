class QgsDoubleBoxScaleBarStyle: QgsScaleBarStyle
{
%TypeHeaderCode
#include <qgsdoubleboxscalebarstyle.h>
%End
  public:
    QgsDoubleBoxScaleBarStyle( const QgsComposerScaleBar* bar );
    ~QgsDoubleBoxScaleBarStyle();

    QString name() const;

    void draw( QPainter* p, double xOffset = 0 ) const;
};
