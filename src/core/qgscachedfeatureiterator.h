/***************************************************************************
    qgscachedfeatureiterator.h
     --------------------------------------
    Date                 : 12.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCACHEDFEATUREITERATOR_H
#define QGSCACHEDFEATUREITERATOR_H

#include "qgis_core.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgscoordinatetransform.h"

class QgsVectorLayerCache;

/**
 * \ingroup core
 * \brief
 * \brief Delivers features from the cache
 *
 */
class CORE_EXPORT QgsCachedFeatureIterator : public QgsAbstractFeatureIterator
{
  public:

    /**
     * This constructor creates a feature iterator, that delivers all cached features. No request is made to the backend.
     *
     * \param vlCache          The vector layer cache to use
     * \param featureRequest   The feature request to answer
     */
    QgsCachedFeatureIterator( QgsVectorLayerCache *vlCache, const QgsFeatureRequest &featureRequest );

    ~QgsCachedFeatureIterator() override;

    /**
     * Rewind to the beginning of the iterator
     *
     * \returns bool TRUE if the operation was OK
     */
    bool rewind() override;

    /**
     * Close this iterator. No further features will be available.
     *
     * \returns TRUE if successful
     */
    bool close() override;

    // QgsAbstractFeatureIterator interface
  protected:

    /**
     * Implementation for fetching a feature.
     *
     * \param f      Will write to this feature
     * \returns bool  TRUE if the operation was OK
     *
     * \see bool getFeature( QgsFeature& f )
     */
    bool fetchFeature( QgsFeature &f ) override;

    /**
     * We have a local special iterator for FilterFids, no need to run the generic.
     *
     * \param f      Will write to this feature
     * \returns bool  TRUE if the operation was OK
     */
    bool nextFeatureFilterFids( QgsFeature &f ) override { return fetchFeature( f ); }

  private:
#ifdef SIP_RUN
    QgsCachedFeatureIterator( const QgsCachedFeatureIterator &other );
#endif

    QList< QgsFeatureId > mFeatureIds;
    QgsVectorLayerCache *mVectorLayerCache = nullptr;
    QList< QgsFeatureId >::ConstIterator mFeatureIdIterator;
    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;

    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;
    double mDistanceWithin = 0;
};

/**
 * \ingroup core
 * \brief
 * \brief Uses another iterator as backend and writes features to the cache
 *
 */
class CORE_EXPORT QgsCachedFeatureWriterIterator : public QgsAbstractFeatureIterator
{
  public:

    /**
     * This constructor creates a feature iterator, which queries the backend and caches retrieved features.
     *
     * \param vlCache          The vector layer cache to use
     * \param featureRequest   The feature request to answer
     */
    QgsCachedFeatureWriterIterator( QgsVectorLayerCache *vlCache, const QgsFeatureRequest &featureRequest );

    /**
     * Rewind to the beginning of the iterator
     *
     * \returns bool TRUE if the operation was OK
     */
    bool rewind() override;

    /**
     * Close this iterator. No further features will be available.
     *
     * \returns TRUE if successful
     */
    bool close() override;

  protected:

    /**
     * Implementation for fetching a feature.
     *
     * \param f      Will write to this feature
     * \returns bool  TRUE if the operation was OK
     *
     * \see bool getFeature( QgsFeature& f )
     */
    bool fetchFeature( QgsFeature &f ) override;

  private:
    QgsFeatureIterator mFeatIt;
    QgsVectorLayerCache *mVectorLayerCache = nullptr;
    QgsFeatureIds mFids;
    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;

};
#endif // QGSCACHEDFEATUREITERATOR_H
