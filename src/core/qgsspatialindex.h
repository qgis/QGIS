/***************************************************************************
    qgsspatialindex.h  - wrapper class for spatial index library
    ----------------------
    begin                : December 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALINDEX_H
#define QGSSPATIALINDEX_H


#include "qgis_sip.h"

// forward declaration
namespace SpatialIndex SIP_SKIP
{
  class IStorageManager;
  class ISpatialIndex;
  class Region;
  class Point;

  namespace StorageManager
  {
    class IBuffer;
  }
}

class QgsFeedback;
class QgsFeature;
class QgsRectangle;
class QgsPointXY;

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QList>
#include <QSharedDataPointer>

#include "qgsfeature.h"

class QgsSpatialIndexData;
class QgsFeatureIterator;
class QgsFeatureSource;

/**
 * \ingroup core
 * \class QgsSpatialIndex
 */
class CORE_EXPORT QgsSpatialIndex
{

  public:

    /* creation of spatial index */

    //! Constructor - creates R-tree
    QgsSpatialIndex();

    /**
     * Constructor - creates R-tree and bulk loads it with features from the iterator.
     * This is much faster approach than creating an empty index and then inserting features one by one.
     *
     * The optional \a feedback object can be used to allow cancelation of bulk feature loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     *
     * \since QGIS 2.8
     */
    explicit QgsSpatialIndex( const QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr );

    /**
     * Constructor - creates R-tree and bulk loads it with features from the source.
     * This is much faster approach than creating an empty index and then inserting features one by one.
     *
     * The optional \a feedback object can be used to allow cancelation of bulk feature loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.

     *
     * \since QGIS 3.0
     */
    explicit QgsSpatialIndex( const QgsFeatureSource &source, QgsFeedback *feedback = nullptr );

    //! Copy constructor
    QgsSpatialIndex( const QgsSpatialIndex &other );

    //! Destructor finalizes work with spatial index
    ~QgsSpatialIndex();

    //! Implement assignment operator
    QgsSpatialIndex &operator=( const QgsSpatialIndex &other );

    /* operations */

    //! Add feature to index
    bool insertFeature( const QgsFeature &f );

    /**
     * Add a feature \a id to the index with a specified bounding box.
     * \returns true if feature was successfully added to index.
     * \since QGIS 3.0
    */
    bool insertFeature( QgsFeatureId id, const QgsRectangle &bounds );

    //! Remove feature from index
    bool deleteFeature( const QgsFeature &f );


    /* queries */

    //! Returns features that intersect the specified rectangle
    QList<QgsFeatureId> intersects( const QgsRectangle &rect ) const;

    //! Returns nearest neighbors (their count is specified by second parameter)
    QList<QgsFeatureId> nearestNeighbor( const QgsPointXY &point, int neighbors ) const;

    /* debugging */

    //! get reference count - just for debugging!
    QAtomicInt SIP_PYALTERNATIVETYPE( int ) refs() const;

  private:

    static SpatialIndex::Region rectToRegion( const QgsRectangle &rect );

    /**
     * Calculates feature info to insert into index.
    * \param f input feature
    * \param r will be set to spatial index region
    * \param id will be set to feature's ID
    * \returns true if feature info was successfully retrieved and the feature can be added to
    * the index
    */
    static bool featureInfo( const QgsFeature &f, SpatialIndex::Region &r, QgsFeatureId &id ) SIP_SKIP;

    /**
     * Calculates feature info to insert into index.
     * \param f input feature
     * \param rect will be set to feature's geometry bounding box
     * \param id will be set to feature's ID
     * \returns true if feature info was successfully retrieved and the feature can be added to
     * the index
     * \since QGIS 3.0
     */
    static bool featureInfo( const QgsFeature &f, QgsRectangle &rect, QgsFeatureId &id );

    friend class QgsFeatureIteratorDataStream; // for access to featureInfo()

  private:

    QSharedDataPointer<QgsSpatialIndexData> d;

};

#endif

