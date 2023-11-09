/***************************************************************************
  parlrtree.h
  ------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "RTree.h"
#include "qgsrectangle.h"
#include <array>

#ifndef QGSPALRTREE_H
#define QGSPALRTREE_H

#define SIP_NO_FILE

/**
 * \ingroup core
 * \class PalRtree
 *
 * \brief A rtree spatial index for use in the pal labeling engine.
 *
 * \note Not available in Python bindings.
 * \since QGIS 3.12
 */
template <typename T>
class PalRtree : public RTree<T *, float, 2, float>
{
  public:

    /**
     * Constructor for PalRtree. The \a maxBounds argument specifies the maximum bounding box
     * for all coordinates which will be stored in the index.
     */
    PalRtree( const QgsRectangle &maxBounds )
      : mXMin( maxBounds.xMinimum() )
      , mYMin( maxBounds.yMinimum() )
      , mXRes( ( std::numeric_limits< float >::max() - 1 ) / ( maxBounds.xMaximum() - maxBounds.xMinimum() ) )
      , mYRes( ( std::numeric_limits< float >::max() - 1 ) / ( maxBounds.yMaximum() - maxBounds.yMinimum() ) )
      , mMaxBounds( maxBounds )
    {

    }

    /**
     * Inserts new \a data into the spatial index, with the specified \a bounds.
     *
     * Ownership of \a data is not transferred, and it is the caller's responsibility to ensure that
     * it exists for the lifetime of the spatial index.
     */
    void insert( T *data, const QgsRectangle &bounds )
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Insert(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      data );
    }

    /**
     * Removes existing \a data from the spatial index, with the specified \a bounds.
     *
     * \a data is not deleted, and it is the caller's responsibility to ensure that
     * it is appropriately cleaned up.
     */
    void remove( T *data, const QgsRectangle &bounds )
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Remove(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      data );
    }

    /**
     * Performs an intersection check against the index, for data intersecting the specified \a bounds.
     *
     * The \a callback function will be called once for each matching data object encountered.
     */
    bool intersects( const QgsRectangle &bounds, const std::function< bool( T *data )> &callback ) const
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Search(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      callback );
      return true;
    }

  private:

    // Coordinates are scaled inside the index so that they cover the maximum range for float values
    double mXMin = 0;
    double mYMin = 0;
    double mXRes = 1;
    double mYRes = 1;
    const QgsRectangle mMaxBounds;
    std::array<float, 4> scaleBounds( const QgsRectangle &bounds ) const
    {
      return
      {
        static_cast< float >( ( std::max( bounds.xMinimum(), mMaxBounds.xMinimum() ) - mXMin ) / mXRes ),
        static_cast< float >( ( std::max( bounds.yMinimum(), mMaxBounds.yMinimum() ) - mYMin ) / mYRes ),
        static_cast< float >( ( std::min( bounds.xMaximum(), mMaxBounds.xMaximum() ) - mXMin ) / mXRes ),
        static_cast< float >( ( std::min( bounds.yMaximum(), mMaxBounds.yMaximum() ) - mYMin ) / mYRes )
      };
    }
};

#endif

