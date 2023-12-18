/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PAL_GEOM_FUNCTION
#define PAL_GEOM_FUNCTION

#define SIP_NO_FILE


#include "qgis_core.h"
#include <cmath>
#include "qgsgeos.h"

namespace pal
{

  /**
   * \ingroup core
   * \brief Pal labeling engine geometry functions.
   * \class pal::GeomFunction
   * \note not available in Python bindings
   */
  class CORE_EXPORT GeomFunction
  {
    public:

      /*
       *           o(x2,y2)
       *          /
       * cp > 0  /
       *        /    cp < 0
       *       /
       *      /
       *     o (x1, y1)
       */
      static inline double cross_product( double x1, double y1, double x2, double y2, double x3, double y3 )
      {
        return ( x2 - x1 ) * ( y3 - y1 ) - ( x3 - x1 ) * ( y2 - y1 );
      }

      static void findLineCircleIntersection( double cx, double cy, double radius,
                                              double x1, double y1, double x2, double y2,
                                              double &xRes, double &yRes );

      /**
       * \brief Compute the convex hull in O(n·log(n))
       * \param id set of point (i.e. point no 0 is (x,y) = x[id[0]],y[id[0]])
       * \param x x coordinates
       * \param y y coordinates
       * \returns convexHull vertex ids
       */
      static std::vector< int > convexHullId( std::vector<int> &id, const std::vector< double > &x, const std::vector< double > &y );

      /**
       * Returns TRUE if the two segments intersect.
       */
      static bool isSegIntersects( double x1, double y1, double x2, double y2,  // 1st segment
                                   double x3, double y3, double x4, double y4 ); // 2nd segment

      /**
       * Compute the point where two lines intersect.
       * \returns TRUE if the lines intersect, or FALSE if the lines are parallel
       */
      static bool computeLineIntersection( double x1, double y1, double x2, double y2,  // 1st line (segment)
                                           double x3, double y3, double x4, double y4,  // 2nd line segment
                                           double *x, double *y );

      //! Reorder points to have cross prod ((x,y)[i], (x,y)[i+1), point) > 0 when point is outside
      static bool reorderPolygon( std::vector< double > &x, std::vector< double> &y );

      /**
       * Returns TRUE if a GEOS prepared geometry totally contains a label candidate.
       * \param geom GEOS prepared geometry
       * \param x candidate x
       * \param y candidate y
       * \param width candidate width
       * \param height candidate height
       * \param alpha candidate angle
       * \returns TRUE if candidate is totally contained
       */
      static bool containsCandidate( const GEOSPreparedGeometry *geom, double x, double y, double width, double height, double alpha );

  };
} //namespace

#endif
