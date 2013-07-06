class QgsCentralPointPositionManager : QgsOverlayObjectPositionManager
{
%TypeHeaderCode
#include <qgscentralpointpositionmanager.h>
%End
  public:
    QgsCentralPointPositionManager();
    ~QgsCentralPointPositionManager();
    void addLayer( QgsVectorLayer* vl, QList<QgsVectorOverlay*>& overlays );
    /**Removes all the overlays*/
    void removeLayers();
    void findObjectPositions( const QgsRenderContext& context, QGis::UnitType unitType );
};
