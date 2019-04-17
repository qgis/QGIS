/***************************************************************************
                             qgsspatialindexkdbush.h
                             -----------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALINDEXKDBUSH_H
#define QGSSPATIALINDEXKDBUSH_H

class QgsFeatureIterator;
class QgsFeedback;
class QgsFeatureSource;
class QgsSpatialIndexKDBushPrivate;
class QgsRectangle;

#include "qgis_core.h"
#include "qgsspatialindexkdbushdata.h"
#include "qgspointxy.h"
#include <memory>
#include <QList>
#include <functional>

/**
 * \class QgsSpatialIndexKDBush
 * \ingroup core
 *
 * A very fast static spatial index for 2D points based on a flat KD-tree.
 *
 * Compared to QgsSpatialIndex, this index:
 * - supports single point features only (no multipoints)
 * - is static (features cannot be added or removed from the index after construction)
 * - is much faster!
 * - allows direct retrieval of the original feature's points, without requiring additional feature requests
 * - supports true "distance based" searches, i.e. return all points within a radius
 * from a search point
 *
 * QgsSpatialIndexKDBush objects are implicitly shared and can be inexpensively copied.
 *
 * \see QgsSpatialIndex, which is an general, mutable index for geometry bounding boxes.
 * \since QGIS 3.4
*/
class CORE_EXPORT QgsSpatialIndexKDBush
{
  public:

    /**
     * Constructor - creates KDBush index and bulk loads it with features from the iterator.
     *
     * The optional \a feedback object can be used to allow cancellation of bulk feature loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     *
     * Any non-single point features encountered during iteration will be ignored and not included in the index.
     */
    explicit QgsSpatialIndexKDBush( QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr );

    /**
     * Constructor - creates KDBush index and bulk loads it with features from the source.
     *
     * The optional \a feedback object can be used to allow cancellation of bulk feature loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     *
     * Any non-single point features encountered during iteration will be ignored and not included in the index.
     */
    explicit QgsSpatialIndexKDBush( const QgsFeatureSource &source, QgsFeedback *feedback = nullptr );

    //! Copy constructor
    QgsSpatialIndexKDBush( const QgsSpatialIndexKDBush &other );

    //! Assignment operator
    QgsSpatialIndexKDBush &operator=( const QgsSpatialIndexKDBush &other );

    ~QgsSpatialIndexKDBush();

    /**
     * Returns the list of features which fall within the specified \a rectangle.
     */
    QList<QgsSpatialIndexKDBushData> intersects( const QgsRectangle &rectangle ) const;

    /**
     * Calls a \a visitor function for all features which fall within the specified \a rectangle.
     *
     * \note Not available in Python bindings
     */
    void intersects( const QgsRectangle &rectangle, const std::function<void( QgsSpatialIndexKDBushData )> &visitor ) const SIP_SKIP;

    /**
     * Returns the list of features which are within the given search \a radius
     * of \a point.
     */
    QList<QgsSpatialIndexKDBushData> within( const QgsPointXY &point, double radius ) const;

    /**
     * Calls a \a visitor function for all features which are within the given search \a radius
     * of \a point.
     *
     * \note Not available in Python bindings
     */
    void within( const QgsPointXY &point, double radius, const std::function<void( QgsSpatialIndexKDBushData )> &visitor ) SIP_SKIP;

    /**
     * Returns the size of the index, i.e. the number of points contained within the index.
     */
    qgssize size() const;

  private:

    //! Implicitly shared data pointer
    QgsSpatialIndexKDBushPrivate *d = nullptr;

    friend class TestQgsSpatialIndexKdBush;
};

#endif // QGSSPATIALINDEXKDBUSH_H
