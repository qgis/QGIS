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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
    return ( x2 - x1 ) * ( y3 - y1 ) - ( x3 - x1 ) * ( y2 - y1 );
  }

  inline double dist_euc2d( double x1, double y1, double x2, double y2 )
  {
    return sqrt(( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 ) );
  }

  inline double dist_euc2d_sq( double x1, double y1, double x2, double y2 )
  {
    return ( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 );
  }

  bool isPointInPolygon( int npol, double *xp, double *yp, double x, double y );
  /*
     // code from Randolph Franklin (found at http://local.wasp.uwa.edu.au/~pbourke/geometry/insidepoly/)
     int i, j;
     bool c = false;

     for (i = 0, j = npol-1; i < npol; j = i++){
        if ((( (yp[i] <= y) && (y < yp[j])) ||
              ((yp[j] <= y) && (y < yp[i])))
            && (x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i])){
           c = !c;
        }
     }
     return c;
  }*/


  inline int nbLabelPointInPolygon( int npol, double *xp, double *yp, double x[4], double y[4] )
  {
    int a, k, count = 0;
    double px, py;

    // cheack each corner
    for ( k = 0;k < 4;k++ )
    {
      px = x[k];
      py = y[k];

      for ( a = 0;a < 2;a++ ) // and each middle of segment
      {
        if ( isPointInPolygon( npol, xp, yp, px, py ) )
          count++;
        px = ( x[k] + x[( k+1 ) %4] ) / 2.0;
        py = ( y[k] + y[( k+1 ) %4] ) / 2.0;
      }
    }

    px = ( x[0] + x[2] ) / 2.0;
    py = ( y[0] + y[2] ) / 2.0;

    // and the label center
    if ( isPointInPolygon( npol, xp, yp, px, py ) )
      count += 4; // virtually 4 points

    return count;
  }



  int convexHull( int *id, const double* const x, const double* const y, int n );


  int convexHullId( int *id, const double* const x, const double* const y, int n, int *&cHull );

  bool isSegIntersects( double x1, double y1, double x2, double y2,  // 1st segment
                        double x3, double y3, double x4, double y4 ); // 2nd segment

  bool computeSegIntersectionExt( double x1, double y1, double x2, double y2, double xs1, double ys1,  // 1st (segment)
                                  double x3, double y3, double x4, double y4, double xs2, double ys2, // 2nd segment
                                  double *x, double *y );


  /*
   * \brief Intersection bw a line and a segment
   * \return true if the point exist false otherwise
   */
  bool computeLineSegIntersection( double x1, double y1, double x2, double y2,  // 1st line
                                   double x3, double y3, double x4, double y4,  // 2nd segment
                                   double *x, double *y );



  /*
   * \brief compute the point wherre two segmentss intersects
   * \return true if the point exists
   */
  bool computeSegIntersection( double x1, double y1, double x2, double y2,  // 1st line (segment)
                               double x3, double y3, double x4, double y4,  // 2nd line segment
                               double *x, double *y );


  /*
   * \brief compute the point wherre two lines intersects
   * \return true if the ok false if line are parallel
   */
  bool computeLineIntersection( double x1, double y1, double x2, double y2,  // 1st line (segment)
                                double x3, double y3, double x4, double y4,  // 2nd line segment
                                double *x, double *y );

#ifdef _EXPORT_MAP_
  /**
   * \brief generate SVG code for a geometry
   *
   * @param nbPoints # points in x and y vector
   * @param typeID from geos
   * @param x x coordinates
   * @param y y coordinates
   * @param dpi map resolution
   * @param scale map scale is 1:scale
   * @param xmin minimum x value in mapExtent
   * @param ymax maximum y value in mapExtent
   * @param layername SVG layer name
   * @param objectID SVG ID
   * @param out stream to write
   */
  void toSVGPath( int nbPoints, int geomType,
                  double *x, double *y,
                  int dpi, double scale,
                  int xmin, int ymax,
                  char *layername,
                  char *objectID,
                  std::ostream &out );
#endif

  int reorderPolygon( int nbPoints, double *x, double *y );

} // end namespace

#endif
