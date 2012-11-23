class QgsNumericScaleBarStyle : QgsScaleBarStyle
{
%TypeHeaderCode
#include <qgsnumericscalebarstyle.h>
%End
  public:
    QgsNumericScaleBarStyle( QgsComposerScaleBar* bar );
    ~QgsNumericScaleBarStyle();

    QString name() const;

    void draw( QPainter* p, double xOffset = 0 ) const;

    //calculation of box size is different compared to segment based scale bars
    QRectF calculateBoxSize() const;
};
