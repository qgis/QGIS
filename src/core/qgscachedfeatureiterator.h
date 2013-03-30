/***************************************************************************
    qgscachedfeatureiterator.h
     --------------------------------------
    Date                 : 12.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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

#include "qgsfeature.h"
#include "qgsfeatureiterator.h"

class QgsVectorLayerCache;

/**
 * @brief
 * Delivers features from the cache
 *
 */
class CORE_EXPORT QgsCachedFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    /**
     * @brief
     * This constructor creates a feature iterator, that delivers only cached information, based on the
     * @link QgsFeatureIds @endlink. No request is made to the backend.
     *
     * @param vlCache          The vector layer cache to use
     * @param featureRequest   The feature request to answer
     * @param featureIds       The feature ids to return
     */
    QgsCachedFeatureIterator( QgsVectorLayerCache* vlCache, QgsFeatureRequest featureRequest, QgsFeatureIds featureIds );

    /**
     * @brief
     *
     * @param f
     * @return bool
     */
    virtual bool nextFeature( QgsFeature& f );

    /**
     * @brief
     *
     * @return bool
     */
    virtual bool rewind();

    /**
     * @brief
     *
     * @return bool
     */
    virtual bool close();

  private:
    QgsFeatureIds mFeatureIds;
    QgsVectorLayerCache* mVectorLayerCache;
    QgsFeatureIds::Iterator mFeatureIdIterator;
};

/**
 * @brief
 * Uses another iterator as backend and writes features to the cache
 *
 */
class CORE_EXPORT QgsCachedFeatureWriterIterator : public QgsAbstractFeatureIterator
{
  public:
    /**
     * @brief
     * This constructor creates a feature iterator, which queries the backend and caches retrieved features.
     *
     * @param vlCache          The vector layer cache to use
     * @param featureRequest   The feature request to answer
     */
    QgsCachedFeatureWriterIterator( QgsVectorLayerCache* vlCache, QgsFeatureRequest featureRequest );

    /**
     * @brief
     *
     * @param f
     * @return bool
     */
    virtual bool nextFeature( QgsFeature& f );

    /**
     * @brief
     *
     * @return bool
     */
    virtual bool rewind();

    /**
     * @brief
     *
     * @return bool
     */
    virtual bool close();

  private:
    QgsFeatureIterator mFeatIt;
    QgsVectorLayerCache* mVectorLayerCache;
    QgsFeatureIds mFids;
};
#endif // QGSCACHEDFEATUREITERATOR_H
