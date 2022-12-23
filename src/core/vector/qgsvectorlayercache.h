/***************************************************************************
  qgsvectorlayercache.h
  Cache features of a vector layer
  -------------------
         begin                : January 2013
         copyright            : (C) Matthias Kuhn
         email                : matthias at opengis dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QgsVectorLayerCache_H
#define QgsVectorLayerCache_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfield.h"
#include "qgsfeaturerequest.h"
#include "qgsfeatureiterator.h"
#include <unordered_set>
#include <deque>
#include <QCache>

class QgsVectorLayer;
class QgsFeature;
class QgsCachedFeatureIterator;
class QgsAbstractCacheIndex;

/**
 * \ingroup core
 * \brief This class caches features of a given QgsVectorLayer.
 *
 * \brief
 * The cached features can be indexed by QgsAbstractCacheIndex.
 *
 * Proper indexing for a given use-case may speed up performance substantially.
 */

class CORE_EXPORT QgsVectorLayerCache : public QObject
{
    Q_OBJECT

  private:

    /**
     * This is a wrapper class around a cached QgsFeature, which
     * will inform the cache, when it has been deleted, so indexes can be
     * updated that the wrapped feature needs to be fetched again if needed.
     */
    class QgsCachedFeature
    {
      public:

        /**
         * Will create a new cached feature.
         *
         * \param feat     The feature to cache. A copy will be made.
         * \param vlCache  The cache to inform when the feature has been removed from the cache.
         */
        QgsCachedFeature( const QgsFeature &feat, QgsVectorLayerCache *vlCache )
          : mCache( vlCache )
        {
          mFeature = new QgsFeature( feat );
        }

        ~QgsCachedFeature()
        {
          // That's the reason we need this wrapper:
          // Inform the cache that this feature has been removed
          mCache->featureRemoved( mFeature->id() );
          delete mFeature;
        }

        inline const QgsFeature *feature() { return mFeature; }

      private:
        QgsFeature *mFeature = nullptr;
        QgsVectorLayerCache *mCache = nullptr;

        friend class QgsVectorLayerCache;
        Q_DISABLE_COPY( QgsCachedFeature )
    };

  public:
    QgsVectorLayerCache( QgsVectorLayer *layer, int cacheSize, QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsVectorLayerCache() override;

    /**
     * Sets the maximum number of features to keep in the cache. Some features will be removed from
     * the cache if the number is smaller than the previous size of the cache.
     *
     * \param cacheSize indicates the maximum number of features to keep in the cache
     */
    void setCacheSize( int cacheSize );

    /**
     * \brief
     * Returns the maximum number of features this cache will hold.
     * In case full caching is enabled, this number can change, as new features get added.
     *
     * \returns int
     */
    int cacheSize();

    /**
     * Enable or disable the caching of geometries
     *
     * \param cacheGeometry    Enable or disable the caching of geometries
     * \see cacheGeometry()
     */
    void setCacheGeometry( bool cacheGeometry );

    /**
     * Returns TRUE if the cache will fetch and cache feature geometries.
     * \see setCacheGeometry()
     * \since QGIS 3.0
     */
    bool cacheGeometry() const { return mCacheGeometry; }

    /**
     * Set the subset of attributes to be cached
     *
     * \param attributes   The attributes to be cached
     */
    void setCacheSubsetOfAttributes( const QgsAttributeList &attributes );

    /**
     * If this is enabled, the subset of cached attributes will automatically be extended
     * to also include newly added attributes.
     *
     * \param cacheAddedAttributes   Automatically cache new attributes
     */
    void setCacheAddedAttributes( bool cacheAddedAttributes );

    /**
     * \brief
     * This enables or disables full caching.
     * If enabled, all features will be held in the cache. The cache size will incrementally
     * be increased to offer space for all features.
     * When enabled, all features will be read into cache. As this feature will most likely
     * be used for slow data sources, be aware, that the call to this method might take a long time.
     *
     * \param fullCache   TRUE: enable full caching, FALSE: disable full caching
     * \note when a cache is invalidated() (e.g. by adding an attribute to a layer) this setting
     * is reset. A full cache rebuild must be performed by calling setFullCache( TRUE ) again.
     * \see hasFullCache()
     */
    void setFullCache( bool fullCache );

    /**
     * Returns TRUE if the cache is complete, ie it contains all features. This may happen as
     * a result of a call to setFullCache() or by through a feature request which resulted in
     * all available features being cached.
     * \see setFullCache()
     * \since QGIS 3.0
     */
    bool hasFullCache() const { return mFullCache; }

    /**
     * \brief
     * Adds a QgsAbstractCacheIndex to this cache. Cache indices know about features present
     * in this cache and decide, if enough information is present in the cache to respond to a QgsFeatureRequest.
     * The layer cache will take ownership of the index.
     *
     * \param cacheIndex  The cache index to add.
     */
    void addCacheIndex( QgsAbstractCacheIndex *cacheIndex SIP_TRANSFER );

    /**
     * Query this VectorLayerCache for features.
     * If the VectorLayerCache (and moreover any of its indices) is able to satisfy
     * the request, the returned QgsFeatureIterator will iterate over cached features.
     * If it's not possible to fully satisfy the request from the cache, part or all of the features
     * will be requested from the data provider.
     * \param featureRequest  The request specifying filter and required data.
     * \returns An iterator over the requested data.
     */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &featureRequest = QgsFeatureRequest() );

    /**
     * Query the layer for features matching a given expression.
     */
    inline QgsFeatureIterator getFeatures( const QString &expression )
    {
      return getFeatures( QgsFeatureRequest( expression ) );
    }

    /**
     * Query the layer for the feature with the given id.
     * If there is no such feature, the returned feature will be invalid.
     */
    inline QgsFeature getFeature( QgsFeatureId fid )
    {
      QgsFeature feature;
      getFeatures( QgsFeatureRequest( fid ) ).nextFeature( feature );
      return feature;
    }

    /**
     * Query the layer for the features with the given ids.
     */
    inline QgsFeatureIterator getFeatures( const QgsFeatureIds &fids )
    {
      return getFeatures( QgsFeatureRequest( fids ) );
    }

    /**
     * Query the layer for the features which intersect the specified rectangle.
     */
    inline QgsFeatureIterator getFeatures( const QgsRectangle &rectangle )
    {
      return getFeatures( QgsFeatureRequest( rectangle ) );
    }

    /**
     * Check if a certain feature id is cached.
     * \param  fid The feature id to look for
     * \returns TRUE if this id is in the cache
     * \see cachedFeatureIds()
     */
    bool isFidCached( QgsFeatureId fid ) const;

    /**
     * Returns the set of feature IDs for features which are cached.
     * \see isFidCached()
     * \since QGIS 3.0
     */
    QgsFeatureIds cachedFeatureIds() const;

    /**
     * Gets the feature at the given feature id. Considers the changed, added, deleted and permanent features
     * \param featureId The id of the feature to query
     * \param feature   The result of the operation will be written to this feature
     * \param skipCache Will query the layer regardless if the feature is in the cache already
     * \returns TRUE in case of success
     */
    bool featureAtId( QgsFeatureId featureId, QgsFeature &feature, bool skipCache = false );

    /**
     * Removes the feature identified by fid from the cache if present.
     * \param fid The id of the feature to delete
     * \returns TRUE if the feature was removed, FALSE if the feature id was not found in the cache
     */
    bool removeCachedFeature( QgsFeatureId fid );

    /**
     * Returns the layer to which this cache belongs
     */
    QgsVectorLayer *layer();

    /**
     * Returns the coordinate reference system for features in the cache.
     */
    QgsCoordinateReferenceSystem sourceCrs() const;

    /**
     * Returns the fields associated with features in the cache.
     */
    QgsFields fields() const;

    /**
     * Returns the geometry type for features in the cache.
     */
    QgsWkbTypes::Type wkbType() const;

#ifdef SIP_RUN

    /**
     * Returns the number of features contained in the source, or -1
     * if the feature count is unknown.
     */
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->featureCount();
    % End

    //! Ensures that bool(obj) returns TRUE (otherwise __len__() would be used)
    int __bool__() const;
    % MethodCode
    sipRes = true;
    % End
#endif

    /**
     * Returns the number of features contained in the source, or -1
     * if the feature count is unknown.
     */
    long long featureCount() const;

  protected:

    /**
     * \brief
     * Gets called, whenever the full list of feature ids for a certain request is known.
     * Broadcasts this information to indices, so they can update their tables.
     *
     * \param featureRequest  The feature request that was answered
     * \param fids            The feature ids that have been returned
     */
    void requestCompleted( const QgsFeatureRequest &featureRequest, const QgsFeatureIds &fids );

    /**
     * \brief
     * Gets called, whenever a feature has been removed.
     * Broadcasts this information to indices, so they can invalidate their cache if required.
     *
     * \param fid             The feature id of the removed feature.
     */
    void featureRemoved( QgsFeatureId fid );

    /**
     * \brief
     * Checks if the information required to complete the request is cached.
     * i.e. If all attributes required and the geometry is held in the cache.
     * Please note, that this does not check, if the requested features are cached.
     *
     *
     * \param featureRequest  The QgsFeatureRequest to be answered
     * \returns                TRUE if the information is being cached, FALSE if not
     */
    bool checkInformationCovered( const QgsFeatureRequest &featureRequest );


  signals:

    /**
     * When filling the cache, this signal gets emitted periodically to notify about the progress
     * and to be able to cancel an operation.
     *
     * \param i       The number of already fetched features
     * \param cancel  A reference to a boolean variable. Set to TRUE and the operation will be canceled.
     *
     * \note not available in Python bindings
     */
    void progress( int i, bool &cancel ) SIP_SKIP;

    /**
     * When filling the cache, this signal gets emitted once the cache is fully initialized.
     */
    void finished();

    /**
     * \brief Is emitted when the cached layer is deleted. Is emitted when the cached layers layerDelete()
     * signal is being emitted, but before the local reference to it has been set to NULLPTR. So call to
     * layer() will still return a valid pointer for cleanup purpose.
     */
    void cachedLayerDeleted();

    /**
     * Emitted when an attribute is changed. Is re-emitted after the layer itself emits this signal.
     * You should connect to this signal, to be sure, to not get a cached value if querying the cache.
     */
    void attributeValueChanged( QgsFeatureId fid, int field, const QVariant &value );

    /**
     * Emitted when a new feature has been added to the layer and this cache.
     * You should connect to this signal instead of the layers', if you want to be sure
     * that this cache has updated information for the new feature
     *
     * \param fid The featureid of the changed feature
     */
    void featureAdded( QgsFeatureId fid );

    /**
     * The cache has been invalidated and cleared. Note that when a cache is invalidated
     * the fullCache() setting will be cleared, and a full cache rebuild via setFullCache( TRUE )
     * will need to be performed.
     */
    void invalidated();

  private slots:
    void onAttributeValueChanged( QgsFeatureId fid, int field, const QVariant &value );
    void onJoinAttributeValueChanged( QgsFeatureId fid, int field, const QVariant &value );
    void featureDeleted( QgsFeatureId fid );
    void onFeatureAdded( QgsFeatureId fid );
    void attributeAdded( int field );
    void attributeDeleted( int field );
    void geometryChanged( QgsFeatureId fid, const QgsGeometry &geom );
    void layerDeleted();
    void invalidate();

  private:

    void connectJoinedLayers() const;

    inline void cacheFeature( QgsFeature &feat )
    {
      QgsCachedFeature *cachedFeature = new QgsCachedFeature( feat, this );
      mCache.insert( feat.id(), cachedFeature );
      if ( mCacheUnorderedKeys.find( feat.id() ) == mCacheUnorderedKeys.end() )
      {
        mCacheUnorderedKeys.insert( feat.id() );
        mCacheOrderedKeys.emplace_back( feat.id() );
      }
    }

    QgsVectorLayer *mLayer = nullptr;
    QCache< QgsFeatureId, QgsCachedFeature > mCache;

    // we need two containers here. One is used for efficient tracking of the IDs which have been added to the cache, the other
    // is used to store the order of the incoming feature ids, so that we can correctly iterate through features in the original order.
    // the ordered list alone is far too slow to handle this -- searching for existing items in a list is magnitudes slower than the unordered_set
    std::unordered_set< QgsFeatureId > mCacheUnorderedKeys;
    std::deque< QgsFeatureId > mCacheOrderedKeys;

    bool mCacheGeometry = true;
    bool mFullCache = false;
    QList<QgsAbstractCacheIndex *> mCacheIndices;

    QgsAttributeList mCachedAttributes;

    friend class QgsCachedFeatureIterator;
    friend class QgsCachedFeatureWriterIterator;
    friend class QgsCachedFeature;

    /**
     * Returns TRUE if the cache contains all the features required for a specified request.
     * \param featureRequest feature request
     * \param it will be set to iterator for matching features
     * \returns TRUE if cache can satisfy request
     * \note this method only checks for available features, not whether the cache
     * contains required attributes or geometry. For that, use checkInformationCovered()
     */
    bool canUseCacheForRequest( const QgsFeatureRequest &featureRequest, QgsFeatureIterator &it );

    friend class TestVectorLayerCache;
};
#endif // QgsVectorLayerCache_H
