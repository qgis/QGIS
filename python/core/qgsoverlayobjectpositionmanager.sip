/**Interface for classes that arrange overlay objects of different layers on the map
* \note This class has been added in version 1.1
*/
class QgsOverlayObjectPositionManager
{
%TypeHeaderCode
#include "qgsoverlayobjectpositionmanager.h"
%End

  public:
    //virtual destructor needed for proper memory management
    virtual ~QgsOverlayObjectPositionManager();
    /**Adds a layer that may contain * overlays to the position manager. The overlay objects contained in the
    overlays will then be considered in label placement*/
    virtual void addLayer( QgsVectorLayer* vl, QList<QgsVectorOverlay*>& overlays ) = 0;
    /**Removes all the layers*/
    virtual void removeLayers() = 0;
    /**Calculate positions for the overlay objects
      @param context Context of rendering operation (Painter, scale factor)
      @param unitType meters, feet, degrees*/
    virtual void findObjectPositions( const QgsRenderContext& context, QGis::UnitType unitType ) = 0;
};
