/**
 * This class caches features of a given QgsVectorLayer.
 * The features are automatically updated and/or invalidated, whenever a change happens to the feature.
 *
 * It is the callers responsibility to make sure, that he specifies all required fields in every select operation.
 * There is no guarantee, that a cached feature will contain a field not specified in a previous select operation.
 *
 * Performance:
 * Depending on the usage scenario, it can be a good idea to prepopulate the cache by calling select, nextFeature and
 * and removeCachedFeature. This is especially interesting, whenever requesting a single feature from the dataProvider
 * takes much longer than querying in bunches (e.g. network latency, slow views in the database...)
 */

class QgsVectorLayerCache : QObject
{
%TypeHeaderCode
#include "qgsvectorlayercache.h"
%End

  public:
    QgsVectorLayerCache( QgsVectorLayer* layer, int cacheSize, QObject* parent = NULL );

    /**
     * Sets the maximum number of features to keep in the cache. Some features will be removed from
     * the cache if the number is smaller than the previous size of the cache.
     *
     * @param cacheSize indicates the maximum number of features to keep in the cache
     */
    void setCacheSize( int cacheSize );
};

