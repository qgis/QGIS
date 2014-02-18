/**
 * This class contains context information for attribute editor widgets.
 * It will be passed to embedded widgets whenever this occurs (e.g. when
 * showing an embedded form due to relations)
 */

class QgsAttributeEditorContext
{
%TypeHeaderCode
#include "qgsattributeeditorcontext.h"
%End
  public:
    QgsAttributeEditorContext();

    QWidget* proxyWidget( QgsVectorLayer* vl, int fieldIdx );
    //! @note not available in python bindings
    // void addProxyWidgets( QgsVectorLayer* vl, QMap<int, QWidget*> proxyWidgets );
    void addProxyWidget( QgsVectorLayer* vl, int idx, QWidget* widget );

    void setDistanceArea( const QgsDistanceArea& distanceArea );
    const QgsDistanceArea& distanceArea();

    void setVectorLayerTools( QgsVectorLayerTools* vlTools );
    QgsVectorLayerTools* vectorLayerTools();

    /**
     * When copying the context for another layer,  call this.
     * Will adjast the distance area for this layer
     *
     * @param layer The layer to adjust for.
     */
    void adjustForLayer( QgsVectorLayer* layer );
};
