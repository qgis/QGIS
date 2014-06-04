/**
 * Listens to the updates in map layer registry and does changes in layer tree.
 *
 * When connected to a layer tree, any layers added to the map layer registry
 * will be also added to the layer tree. Similarly, map layers that are removed
 * from registry will be removed from the layer tree.
 *
 * If a layer is completely removed from the layer tree, it will be also removed
 * from the map layer registry.
 *
 * @note added in 2.4
 */
class QgsLayerTreeRegistryBridge : QObject
{
%TypeHeaderCode
#include <qgslayertreeregistrybridge.h>
%End

  public:
    explicit QgsLayerTreeRegistryBridge( QgsLayerTreeGroup* root, QObject *parent /TransferThis/ = 0 );

    void setEnabled( bool enabled );
    bool isEnabled() const;

    void setNewLayersVisible( bool enabled );
    bool newLayersVisible() const;

    //! Set where the new layers should be inserted - can be used to follow current selection.
    //! By default it is root group with zero index.
    void setLayerInsertionPoint( QgsLayerTreeGroup* parentGroup, int index );

};
