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

#include "util.h"

namespace pal
{
  /**
   * \ingroup core
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
        return ( x2 - x1 ) *( y3 - y1 ) - ( x3 - x1 ) *( y2 - y1 );
      }

      static inline double dist_euc2d( double x1, double y1, double x2, double y2 )
      {
        return sqrt(( x2 - x1 ) *( x2 - x1 ) + ( y2 - y1 ) *( y2 - y1 ) );
      }

      static inline double dist_euc2d_sq( double x1, double y1, double x2, double y2 )
      {
        return ( x2 - x1 ) *( x2 - x1 ) + ( y2 - y1 ) *( y2 - y1 );
      }

      static void findLineCircleIntersection( double cx, double cy, double radius,
                                              double x1, double y1, double x2, double y2,
                                              double& xRes, double& yRes );

      /**
       * \brief Compute the convex hull in O(n·log(n))
       * \param id set of point (i.e. point no 0 is (x,y) = x[id[0]],y[id[0]])
       * \param x x coordinates
       * \param y y coordinates
       * \param n Size of subset (vector id)
       * \param cHull returns the point id (id of id's vector...) whom are parts of the convex hull
       * \return convexHull's size
       */
      static int convexHullId( int *id, const double* const x, const double* const y, int n, int *&cHull );

      /**
       * Returns true if the two segments intersect.
       */
      static bool isSegIntersects( double x1, double y1, double x2, double y2,  // 1st segment
                                   double x3, double y3, double x4, double y4 ); // 2nd segment

      /**
       * Compute the point where two lines intersect.
       * \returns true if the lines intersect, or false if the lines are parallel
       */
      static bool computeLineIntersection( double x1, double y1, double x2, double y2,  // 1st line (segment)
                                           double x3, double y3, double x4, double y4,  // 2nd line segment
                                           double *x, double *y );

      //! Reorder points to have cross prod ((x,y)[i], (x,y)[i+1), point) > 0 when point is outside
      static int reorderPolygon( int nbPoints, double *x, double *y );

  };
} //namespace

#endif
