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

#ifndef _GEOM_FUNCTION_
#define _GEOM_FUNCTION_

#include "util.h"

namespace pal
{

  /*
   *           o(x2,y2)
   *          /
   * cp > 0  /
   *        /    cp < 0
   *       /
   *      /
   *     o (x1, y1)
   */
  inline double cross_product( double x1, double y1, double x2, double y2, double x3, double y3 )
  {
    return ( x2 - x1 ) *( y3 - y1 ) - ( x3 - x1 ) *( y2 - y1 );
  }

  inline double dist_euc2d( double x1, double y1, double x2, double y2 )
  {
    return sqrt(( x2 - x1 ) *( x2 - x1 ) + ( y2 - y1 ) *( y2 - y1 ) );
  }

  inline double dist_euc2d_sq( double x1, double y1, double x2, double y2 )
  {
    return ( x2 - x1 ) *( x2 - x1 ) + ( y2 - y1 ) *( y2 - y1 );
  }

  void findLineCircleIntersection( double cx, double cy, double radius,
                                   double x1, double y1, double x2, double y2,
                                   double& xRes, double& yRes );


  int convexHullId( int *id, const double* const x, const double* const y, int n, int *&cHull );

  bool isSegIntersects( double x1, double y1, double x2, double y2,  // 1st segment
                        double x3, double y3, double x4, double y4 ); // 2nd segment

  /*
   * \brief compute the point wherre two lines intersects
   * \return true if the ok false if line are parallel
   */
  bool computeLineIntersection( double x1, double y1, double x2, double y2,  // 1st line (segment)
                                double x3, double y3, double x4, double y4,  // 2nd line segment
                                double *x, double *y );

  int reorderPolygon( int nbPoints, double *x, double *y );

} // end namespace

#endif
