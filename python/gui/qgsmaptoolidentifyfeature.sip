class QgsMapToolIdentifyFeature : QgsMapToolIdentify
{
%TypeHeaderCode
#include <qgsmaptoolidentifyfeature.h>
%End

  public:
    QgsMapToolIdentifyFeature( QgsVectorLayer* vl, QgsMapCanvas* canvas );

    void canvasReleaseEvent( QMouseEvent* e );

  signals:
    void featureIdentified( QgsFeature );
};
