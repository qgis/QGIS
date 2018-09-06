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

#define SIP_NO_FILE

#ifndef QGS_FEATUREPOOL_H
#define QGS_FEATUREPOOL_H

#include <QCache>
#include <QMutex>
#include "qgis_analysis.h"
#include "qgsfeature.h"
#include "qgsspatialindex.h"
#include "qgsvectorlayer.h"

class QgsVectorLayer;

/**
 * \ingroup analysis
 * A feature pool is based on a vector layer and caches features.
 */
class ANALYSIS_EXPORT QgsFeaturePool
{

  public:
    QgsFeaturePool( QgsVectorLayer *layer, double layerToMapUnits, const QgsCoordinateTransform &layerToMapTransform );
    virtual ~QgsFeaturePool() = default;

    /**
     * Retrieve the feature with the specified \a id into \a feature.
     * It will be retrieved from the cache or from the underlying layer if unavailable.
     * If the feature is neither available from the cache nor from the layer it will return false.
     */
    bool get( QgsFeatureId id, QgsFeature &feature );

    /**
     * Adds a feature to this pool.
     * Implementations will add the feature to the layer or to the data provider.
     */
    virtual void addFeature( QgsFeature &feature ) = 0;

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
     */
    QgsFeatureIds getFeatureIds() const;

    /**
     * Get all feature ids in the bounding box \a rect. It will use a spatial index to
     * determine the ids.
     */
    QgsFeatureIds getIntersects( const QgsRectangle &rect ) const;

    /**
     * The factor of layer units to map units.
     * TODO: should this be removed and determined on runtime by checks that need it?
     */
    double getLayerToMapUnits() const { return mLayerToMapUnits; }

    /**
     * A coordinate transform from layer to map CRS.
     * TODO: should this be removed and determined on runtime by checks that need it?
     */
    const QgsCoordinateTransform &getLayerToMapTransform() const { return mLayerToMapTransform; }

    /**
     * Get a pointer to the underlying layer.
     * May return a ``nullptr`` if the layer has been deleted.
     * This must only be called from the main thread.
     */
    QgsVectorLayer *layer() const;

    /**
     * The layer id of the layer.
     */
    QString layerId() const;

    /**
     * The geometry type of this layer.
     */
    QgsWkbTypes::GeometryType geometryType() const;

  protected:

    /**
     * Inserts a feature into the cache and the spatial index.
     * To be used by implementations of ``addFeature``.
     */
    void insertFeature( const QgsFeature &feature );

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
     * Set all the feature ids governed by this feature pool.
     * Should be called by subclasses constructor and whenever
     * they insert a new feature.
     */
    void setFeatureIds( const QgsFeatureIds &ids );

  private:
    static const int CACHE_SIZE = 1000;
    QCache<QgsFeatureId, QgsFeature> mFeatureCache;
    QPointer<QgsVectorLayer> mLayer;
    QReadWriteLock mCacheLock;
    QgsFeatureIds mFeatureIds;
    mutable QReadWriteLock mIndexLock;
    QgsSpatialIndex mIndex;
    double mLayerToMapUnits = 1.0;
    QgsCoordinateTransform mLayerToMapTransform;
    QString mLayerId;
    QgsWkbTypes::GeometryType mGeometryType;
};

#endif // QGS_FEATUREPOOL_H
