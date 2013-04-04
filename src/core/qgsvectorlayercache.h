/***************************************************************************
  qgsvectorlayercache.h
  Cache features of a vector layer
  -------------------
         begin                : January 2013
         copyright            : (C) Matthias Kuhn
         email                : matthias dot kuhn at gmx dot ch

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

#include <QCache>

#include "qgsvectorlayer.h"

class QgsCachedFeatureIterator;
class QgsAbstractCacheIndex;

/**
 * This class caches features of a given QgsVectorLayer.
 *
 * @brief
 * The cached features can be indexed by @link QgsAbstractCacheIndex @endlink.
 *
 * Proper indexing for a given use-case may speed up performance substantially.
 */

class CORE_EXPORT QgsVectorLayerCache : public QObject
{
    Q_OBJECT

  private:
    /**
     * This is a wrapper class around a cached @link QgsFeature @endlink, which
     * will inform the cache, when it has been deleted, so indexes can be
     * updated that the wrapped feature needs to be fetched again if needed.
     */
    class QgsCachedFeature
    {
      public:
        /**
         * Will create a new cached feature.
         *
         * @param feat     The feature to cache. A copy will be made.
         * @param vlCache  The cache to inform when the feature has been removed from the cache.
         */
        QgsCachedFeature( const QgsFeature& feat, QgsVectorLayerCache* vlCache )
            : mCache( vlCache )
        {
          mFeature = new QgsFeature( feat );
        }

        ~QgsCachedFeature()
        {
          // That's the reason we need this wrapper:
          // Inform the cache that this feature has been removed
          mCache->featureRemoved( mFeature->id() );
          delete( mFeature );
        }

        inline const QgsFeature* feature() { return mFeature; }

      private:
        QgsFeature* mFeature;
        QgsVectorLayerCache* mCache;

        friend class QgsVectorLayerCache;
    };

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
     *
     * @param cacheIndex  The cache index to add.
     */
    void addCacheIndex( const QgsAbstractCacheIndex& cacheIndex );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest& featureRequest );

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
     * When filling the cache, this signal gets emited periodically to notify about the progress
     * and to be able to cancel an operation.
     *
     * @param i       The number of already fetched features
     * @param cancel  A reference to a boolean variable. Set to true and the operation will be canceled.
     */
    void progress( int i, bool& cancel );

    /**
     * @brief Is emitted when the cached layer is deleted. Is emitted when the cached layers layerDelete()
     * signal is being emitted, but before the local reference to it has been set to NULL. So call to
     * @link layer() @endlink will still return a valid pointer for cleanup purpose.
     */
    void cachedLayerDeleted();

  public slots:
    void attributeValueChanged( QgsFeatureId fid, int field, const QVariant& value );
    void featureDeleted( QgsFeatureId fid );
    void featureAdded( QgsFeatureId fid );
    void attributeAdded( int field );
    void attributeDeleted( int field );
    void geometryChanged( QgsFeatureId fid, QgsGeometry& geom );
    void layerDeleted();

  private:

    inline void cacheFeature( QgsFeature& feat )
    {
      QgsCachedFeature* cachedFeature = new QgsCachedFeature( feat, this );
      mCache.insert( feat.id(), cachedFeature );
    }

    QgsVectorLayer* mLayer;
    QCache< QgsFeatureId, QgsCachedFeature > mCache;

    bool mCacheGeometry;
    bool mFullCache;
    QList<QgsAbstractCacheIndex*> mCacheIndices;

    QgsAttributeList mCachedAttributes;

    friend class QgsCachedFeatureIterator;
    friend class QgsCachedFeatureWriterIterator;
    friend class QgsCachedFeature;
};
#endif // QgsVectorLayerCache_H
