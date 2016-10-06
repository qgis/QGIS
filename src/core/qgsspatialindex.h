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

// forward declaration
namespace SpatialIndex
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

class QgsFeature;
class QgsRectangle;
class QgsPoint;

#include <QList>
#include <QSharedDataPointer>

#include "qgsfeature.h"

class QgsSpatialIndexData;
class QgsFeatureIterator;

/** \ingroup core
 * \class QgsSpatialIndex
 */
class CORE_EXPORT QgsSpatialIndex
{

  public:

    /* creation of spatial index */

    /** Constructor - creates R-tree */
    QgsSpatialIndex();

    /** Constructor - creates R-tree and bulk loads it with features from the iterator.
     * This is much faster approach than creating an empty index and then inserting features one by one.
     *
     * @note added in 2.8
     */
    explicit QgsSpatialIndex( const QgsFeatureIterator& fi );

    /** Copy constructor */
    QgsSpatialIndex( const QgsSpatialIndex& other );

    /** Destructor finalizes work with spatial index */
    ~QgsSpatialIndex();

    /** Implement assignment operator */
    QgsSpatialIndex& operator=( const QgsSpatialIndex& other );

    /* operations */

    /** Add feature to index */
    bool insertFeature( const QgsFeature& f );

    /** Remove feature from index */
    bool deleteFeature( const QgsFeature& f );


    /* queries */

    /** Returns features that intersect the specified rectangle */
    QList<QgsFeatureId> intersects( const QgsRectangle& rect ) const;

    /** Returns nearest neighbors (their count is specified by second parameter) */
    QList<QgsFeatureId> nearestNeighbor( const QgsPoint& point, int neighbors ) const;

    /* debugging */

    //! get reference count - just for debugging!
    QAtomicInt refs() const;

  protected:
    //! @note not available in python bindings
    static SpatialIndex::Region rectToRegion( const QgsRectangle& rect );
    //! @note not available in python bindings
    static bool featureInfo( const QgsFeature& f, SpatialIndex::Region& r, QgsFeatureId &id );

    friend class QgsFeatureIteratorDataStream; // for access to featureInfo()

  private:

    QSharedDataPointer<QgsSpatialIndexData> d;

};

#endif

