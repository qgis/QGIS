
/**
* \class QgsMapLayerRegistry
* \brief This class tracks map layers that are currently loaded an provides
* a means to fetch a pointer to a map layer and delete it
*/
class QgsMapLayerRegistry : QObject
{
%TypeHeaderCode
#include <qgsmaplayerregistry.h>
%End
  
public:

 //! Returns the instance pointer, creating the object on the first call
 static QgsMapLayerRegistry * instance();

 //! Return the number of registered layers.
 int count();

 ~QgsMapLayerRegistry();
 
 //! Retrieve a pointer to a loaded layer by id
 QgsMapLayer *mapLayer(QString theLayerId);

 //! Retrieve the mapLayers collection (mainly intended for use by projection)
 QMap<QString,QgsMapLayer*> & mapLayers();

 /** Add a layer to the map of loaded layers 
    @returns NULL if unable to add layer, otherwise pointer to newly added layer
    @note

    As a side-effect QgsProject is made dirty.
 
    Emits signal that layer has been added only if theEmitSignal is true (by default).
    Not emitting signal is useful when you want to use registry also for layers
    which won't be used in main map canvas but will be used in a special one
 */
 QgsMapLayer *  addMapLayer(QgsMapLayer * theMapLayer /Transfer/, bool theEmitSignal = TRUE);

 /** Remove a layer from qgis

    @note

    As a side-effect QgsProject is made dirty.

    Any canvases using that layer will need to remove it
 
    theEmitSignal - see addMapLayer()
 */
 void removeMapLayer(QString theLayerId, bool theEmitSignal = TRUE);

 /** Remove all registered layers 

    @note raises removedAll()

    As a side-effect QgsProject is made dirty.

 */
 void removeAllMapLayers();

 /* Clears all layer caches, resetting them to zero and 
 * freeing up any memory they may have been using. Layer
 * caches are used to speed up rendering in certain situations
 * see ticket #1974 for more details.
 * @note this method was added in QGIS 1.4
 */
 void clearAllLayerCaches();

 /**Reload all provider data caches (currently used for WFS and WMS providers)
 @note: this method was added in QGIS 1.6*/
 void reloadAllLayers();

signals:

    /** emitted when a layer is removed from the registry

       connected to main map canvas and overview map canvas remove()
    */
 void layerWillBeRemoved(QString theLayerId);

    /** emitted when a layer is added to the registry

       connected to main map canvas and overview map canvas addLayer()
    */
 void layerWasAdded(QgsMapLayer * theMapLayer);

 /** emitted when ALL layers are removed at once

    This could have been implemented by iteratively signalling
    layerWillBeRemoved() for each layer as it is removed.  However, this
    generally causes a cascade of effects that are unnecessary if we're
    ultimately removing all layers.  E.g., removing the legend item
    corresponding to the layer.  Why bother doing that when you're just going
    to clear everything anyway?

  */
 void removedAll();

protected:

 //! protected constructor
 QgsMapLayerRegistry( QObject * parent = 0 );


}; // class QgsMapLayerRegistry

