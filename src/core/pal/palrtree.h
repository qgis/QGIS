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

#ifndef QGSPALRTREE_H
#define QGSPALRTREE_H

#define SIP_NO_FILE

/**
 * \ingroup core
 * \class PalRtree
 *
 * A rtree spatial index for use in the pal labeling engine.
 *
 * \note Not available in Python bindings.
 * \since QGIS 3.12
 */
template <typename T>
class PalRtree : public RTree<T *, float, 2, float>
{
  public:

    /**
     * Inserts new \a data into the spatial index, with the specified \a bounds.
     *
     * Ownership of \a data is not transferred, and it is the caller's responsibility to ensure that
     * it exists for the lifetime of the spatial index.
     */
    void insert( T *data, const QgsRectangle &bounds )
    {
      this->Insert(
      {
        static_cast< float >( bounds.xMinimum() ),
        static_cast< float >( bounds.yMinimum() )
      },
      {
        static_cast< float>( bounds.xMaximum() ),
        static_cast< float >( bounds.yMaximum() )
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
      this->Remove(
      {
        static_cast< float >( bounds.xMinimum() ),
        static_cast< float >( bounds.yMinimum() )
      },
      {
        static_cast< float>( bounds.xMaximum() ),
        static_cast< float >( bounds.yMaximum() )
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
      this->Search(
      {
        static_cast< float >( bounds.xMinimum() ),
        static_cast< float >( bounds.yMinimum() )
      },
      {
        static_cast< float>( bounds.xMaximum() ),
        static_cast< float >( bounds.yMaximum() )
      },
      callback );
      return true;
    }
};

#endif

