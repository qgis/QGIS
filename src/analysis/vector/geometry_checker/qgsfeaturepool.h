/***************************************************************************
 *  qgsfeaturepool.h                                                       *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_FEATUREPOOL_H
#define QGS_FEATUREPOOL_H

#include <QCache>
#include <QMutex>
#include <QPointer>

#include "qgis_analysis.h"
#include "qgsfeature.h"
#include "qgsspatialindex.h"
#include "qgsfeaturesink.h"
#include "qgsvectorlayerfeatureiterator.h"

/**
 * \ingroup analysis
 * A feature pool is based on a vector layer and caches features.
 *
 * \note This class is a technology preview and unstable API.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsFeaturePool : public QgsFeatureSink SIP_ABSTRACT
{

  public:

    /**
     * Creates a new feature pool for \a layer.
     */
    QgsFeaturePool( QgsVectorLayer *layer );
    virtual ~QgsFeaturePool() = default;

    /**
     * Retrieves the feature with the specified \a id into \a feature.
     * It will be retrieved from the cache or from the underlying feature source if unavailable.
     * If the feature is neither available from the cache nor from the source it will return FALSE.
     */
    bool getFeature( QgsFeatureId id, QgsFeature &feature );

    /**
     * Gets features for the provided \a request. No features will be fetched
     * from the cache and the request is sent directly to the underlying feature source.
     * Results of the request are cached in the pool and the ids of all the features
     * are returned. This is used to warm the cache for a particular area of interest
     * (bounding box) or other set of features.
     * This will get a new feature source from the source vector layer.
     * This needs to be called from the main thread.
     * If \a feedback is specified, the call may return if the feedback is canceled.
     */
    QgsFeatureIds getFeatures( const QgsFeatureRequest &request, QgsFeedback *feedback = nullptr ) SIP_SKIP;

    /**
     * Updates a feature in this pool.
     * Implementations will update the feature on the layer or on the data provider.
     */
    virtual void updateFeature( QgsFeature &feature ) = 0;

    /**
     * Removes a feature from this pool.
     * Implementations will remove the feature from the layer or from the data provider.
     */
    virtual void deleteFeature( QgsFeatureId fid ) = 0;

    /**
     * Returns the complete set of feature ids in this pool.
     * Note that this concerns the features governed by this pool, which are not necessarily all cached.
     *
     * \note not available in Python bindings
     */
    QgsFeatureIds allFeatureIds() const SIP_SKIP;

    /**
     * Gets all feature ids in the bounding box \a rect. It will use a spatial index to
     * determine the ids.
     *
     * \note not available in Python bindings
     */
    QgsFeatureIds getIntersects( const QgsRectangle &rect ) const SIP_SKIP;

    /**
     * Gets a pointer to the underlying layer.
     * May return a ``NULLPTR`` if the layer has been deleted.
     * This must only be called from the main thread.
     */
    QgsVectorLayer *layer() const;

    /**
     * Gets a QPointer to the underlying layer.
     * Note that access to any methods of the object
     * will need to be done on the main thread and
     * the pointer will need to be checked for validity
     * before usage.
     *
     * \note not available in Python bindings
     */
    QPointer<QgsVectorLayer> layerPtr() const SIP_SKIP;

    /**
     * The layer id of the layer.
     */
    QString layerId() const;

    /**
     * The geometry type of this layer.
     */
    QgsWkbTypes::GeometryType geometryType() const;

    /**
     * The coordinate reference system of this layer.
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Returns the name of the layer.
     *
     * Should be preferred over layer().name() because it can directly be run on
     * the background thread.
     */
    QString layerName() const;

  protected:

    /**
     * Inserts a feature into the cache and the spatial index.
     * To be used by implementations of ``addFeature``.
     */
    void insertFeature( const QgsFeature &feature, bool skipLock = false );

    /**
     * Changes a feature in the cache and the spatial index.
     * To be used by implementations of ``updateFeature``.
     */
    void refreshCache( const QgsFeature &feature );

    /**
     * Removes a feature from the cache and the spatial index.
     * To be used by implementations of ``deleteFeature``.
     */
    void removeFeature( const QgsFeatureId featureId );

    /**
     * Sets all the feature ids governed by this feature pool.
     * Should be called by subclasses constructor and whenever
     * they insert a new feature.
     *
     * \note not available in Python bindings
     */
    void setFeatureIds( const QgsFeatureIds &ids ) SIP_SKIP;

    /**
     * Checks if the feature \a fid is cached.
     *
     * \note not available in Python bindings
     * \since QGIS 3.4
     */
    bool isFeatureCached( QgsFeatureId fid ) SIP_SKIP;

  private:
#ifdef SIP_RUN
    QgsFeaturePool( const QgsFeaturePool &other )
    {}
#endif

    static const int CACHE_SIZE = 1000;
    QCache<QgsFeatureId, QgsFeature> mFeatureCache;
    QPointer<QgsVectorLayer> mLayer;
    mutable QReadWriteLock mCacheLock;
    QgsFeatureIds mFeatureIds;
    QgsSpatialIndex mIndex;
    QgsWkbTypes::GeometryType mGeometryType;
    std::unique_ptr<QgsVectorLayerFeatureSource> mFeatureSource;
    QString mLayerName;
};

#endif // QGS_FEATUREPOOL_H
