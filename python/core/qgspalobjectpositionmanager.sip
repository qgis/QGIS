class QgsPALObjectPositionManager : QgsOverlayObjectPositionManager
{
%TypeHeaderCode
#include <qgspalobjectpositionmanager.h>
%End
  public:
    QgsPALObjectPositionManager();
    ~QgsPALObjectPositionManager();
    void addLayer( QgsVectorLayer* vl, QList<QgsVectorOverlay*>& overlays );
    void findObjectPositions( const QgsRenderContext& renderContext, QGis::UnitType unitType );
    void removeLayers();

    /**Sets algorithm used for object placements. Possibilities are:
      Chain, Popmusic tabu chain, Popmusic tabu, Popmusic chain*/
    void setPlacementAlgorithm( const QString& algorithmName );
};
