class QgsMapToolIdentifyFeature : QgsMapToolIdentify
{
%TypeHeaderCode
#include <qgsmaptoolidentifyfeature.h>
%End

  public:
    /**
     * @brief QgsMapToolIdentifyFeature is a map tool to identify a feature on a chosen layer
     * @param canvas the map canvas
     * @param vl the vector layer. The map tool can be initialized without any layer and can be set afterward.
     */
    QgsMapToolIdentifyFeature( QgsMapCanvas* canvas, QgsVectorLayer* vl = 0 );

    ~QgsMapToolIdentifyFeature();

    //! change the layer used by the map tool to identify
    void setLayer( QgsVectorLayer* vl );

    void canvasReleaseEvent( QgsMapMouseEvent* e );

  signals:
    void featureIdentified( QgsFeature );
};
