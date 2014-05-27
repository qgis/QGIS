/**
 * This class caches features of a given QgsVectorLayer.
 *
 * @brief
 * The cached features can be indexed by @link QgsAbstractCacheIndex @endlink.
 *
 * Proper indexing for a given use-case may speed up performance substantially.
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

    /**
     * @brief
     * Returns the maximum number of features this cache will hold.
     * In case full caching is enabled, this number can change, as new features get added.
     *
     * @return int
     */
    int cacheSize();

    /**
     * Enable or disable the caching of geometries
     *
     * @param cacheGeometry    Enable or disable the caching of geometries
     */
    void setCacheGeometry( bool cacheGeometry );


    /**
     * Set the subset of attributes to be cached
     *
     * @param attributes   The attributes to be cached
     */
    void setCacheSubsetOfAttributes( const QgsAttributeList& attributes );

    /**
     * If this is enabled, the subset of cached attributes will automatically be extended
     * to also include newly added attributes.
     *
     * @param cacheAddedAttributes   Automatically cache new attributes
     */
    void setCacheAddedAttributes( bool cacheAddedAttributes );

    /**
     * @brief
     * This enables or disables full caching.
     * If enabled, all features will be held in the cache. The cache size will incrementally
     * be increased to offer space for all features.
     * When enabled, all features will be read into cache. As this feature will most likely
     * be used for slow data sources, be aware, that the call to this method might take a long time.
     *
     * @param fullCache   True: enable full caching, False: disable full caching
     */
    void setFullCache( bool fullCache );

    /**
     * @brief
     * Adds a {@link QgsAbstractCacheIndex} to this cache. Cache indices know about features present
     * in this cache and decide, if enough information is present in the cache to respond to a {@link QgsFeatureRequest}.
     * The layer cache will take ownership of the index.
     *
     * @param cacheIndex  The cache index to add.
     */
    void addCacheIndex( QgsAbstractCacheIndex *cacheIndex );

    /**
     * Query this VectorLayerCache for features.
     * If the VectorLayerCache (and moreover any of its indices) is able to satisfy
     * the request, the returned {@link QgsFeatureIterator} will iterate over cached features.
     * If it's not possible to fully satisfy the request from the cache, part or all of the features
     * will be requested from the data provider.
     * @param featureRequest  The request specifying filter and required data.
     * @return An iterator over the requested data.
     */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest& featureRequest = QgsFeatureRequest() );

    /**
     * Check if a certain feature id is cached.
     * @param  fid The feature id to look for
     * @return True if this id is in the cache
     */
    bool isFidCached( const QgsFeatureId fid );

    /**
     * Gets the feature at the given feature id. Considers the changed, added, deleted and permanent features
     * @param featureId The id of the feature to query
     * @param feature   The result of the operation will be written to this feature
     * @param skipCache Will query the layer regardless if the feature is in the cache already
     * @return true in case of success
     */
    bool featureAtId( QgsFeatureId featureId, QgsFeature &feature, bool skipCache = false );

    /**
     * Removes the feature identified by fid from the cache if present.
     * @param fid The id of the feature to delete
     * @return true if the feature was removed, false if the feature id was not found in the cache
     */
    bool removeCachedFeature( QgsFeatureId fid );

    /**
     * Returns the layer to which this cache belongs
     */
    QgsVectorLayer* layer();

  protected:
    /**
     * @brief
     * Gets called, whenever the full list of feature ids for a certain request is known.
     * Broadcasts this information to indices, so they can update their tables.
     *
     * @param featureRequest  The feature request that was answered
     * @param fids            The feature ids that have been returned
     */
    void requestCompleted( QgsFeatureRequest featureRequest, QgsFeatureIds fids );

    /**
     * @brief
     * Gets called, whenever a feature has been removed.
     * Broadcasts this information to indices, so they can invalidate their cache if required.
     *
     * @param fid             The feature id of the removed feature.
     */
    void featureRemoved( QgsFeatureId fid );

    /**
     * @brief
     * Checks if the information required to complete the request is cached.
     * i.e. If all attributes required and the geometry is held in the cache.
     * Please note, that this does not check, if the requested features are cached.
     *
     *
     * @param featureRequest  The {@link QgsFeatureRequest} to be answered
     * @return                True if the information is being cached, false if not
     */
    bool checkInformationCovered( const QgsFeatureRequest& featureRequest );


  signals:

    /**
     * When filling the cache, this signal gets emitted periodically to notify about the progress
     * and to be able to cancel an operation.
     *
     * @param i       The number of already fetched features
     * @param cancel  A reference to a boolean variable. Set to true and the operation will be canceled.
     *
     * @note not available in python bindings
     */
    // void progress( int i, bool& cancel );

    /**
     * When filling the cache, this signal gets emitted once the cache is fully initialized.
     */
    void finished();

    /**
     * @brief Is emitted when the cached layer is deleted. Is emitted when the cached layers layerDelete()
     * signal is being emitted, but before the local reference to it has been set to NULL. So call to
     * @link layer() @endlink will still return a valid pointer for cleanup purpose.
     */
    void cachedLayerDeleted();

    /**
     * @brief Is emitted when an attribute is changed. Is re-emitted after the layer itself emits this signal.
     *        You should connect to this signal, to be sure, to not get a cached value if querying the cache.
     */
    void attributeValueChanged( const QgsFeatureId& fid, const int& field, const QVariant &value );

    /**
     * Is emitted, when a new feature has been added to the layer and this cache.
     * You should connect to this signal instead of the layers', if you want to be sure
     * that this cache has updated information for the new feature
     *
     * @param fid The featureid of the changed feature
     */
    void featureAdded( QgsFeatureId fid );

};

