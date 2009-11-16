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

#include "geomfunction.h"

//#include <pal/Layer.h>

#include "feature.h"
#include "util.h"

namespace pal
{

  void heapsort( int *sid, int *id, const double* const x, int N )
  {
    unsigned int n = N, i = n / 2, parent, child;
    int tx;
    for ( ;; )
    {
      if ( i > 0 )
      {
        i--;
        tx = sid[i];
      }
      else
      {
        n--;
        if ( n == 0 ) return;
        tx = sid[n];
        sid[n] = sid[0];
      }
      parent = i;
      child = i * 2 + 1;
      while ( child < n )
      {
        if ( child + 1 < n  &&  x[id[sid[child + 1]]] > x[id[sid[child]]] )
        {
          child++;
        }
        if ( x[id[sid[child]]] > x[id[tx]] )
        {
          sid[parent] = sid[child];
          parent = child;
          child = parent * 2 + 1;
        }
        else
        {
          break;
        }
      }
      sid[parent] = tx;
    }
  }


  void heapsort2( int *x, double* heap, int N )
  {
    unsigned int n = N, i = n / 2, parent, child;
    double t;
    int tx;
    for ( ;; )
    {
      if ( i > 0 )
      {
        i--;
        t = heap[i];
        tx = x[i];
      }
      else
      {
        n--;
        if ( n == 0 ) return;
        t = heap[n];
        tx = x[n];
        heap[n] = heap[0];
        x[n] = x[0];
      }
      parent = i;
      child = i * 2 + 1;
      while ( child < n )
      {
        if ( child + 1 < n  &&  heap[child + 1] > heap[child] )
        {
          child++;
        }
        if ( heap[child] > t )
        {
          heap[parent] = heap[child];
          x[parent] = x[child];
          parent = child;
          child = parent * 2 + 1;
        }
        else
        {
          break;
        }
      }
      heap[parent] = t;
      x[parent] = tx;
    }
  }




  /*
   * \brief return true if the two seg intersect
   */

  bool isSegIntersects( double x1, double y1, double x2, double y2,  // 1st segment
                        double x3, double y3, double x4, double y4 )  // 2nd segment
  {
    /*
       std::cout << "SegInrersect ? " << std::endl;
       std::cout << "   cp1 : " << cross_product (x1, y1, x2, y2, x3, y3) << std::endl;
       std::cout << "   cp2 : " << cross_product (x1, y1, x2, y2, x4, y4) << std::endl;
       std::cout << "   cp3 : " << cross_product (x3, y3, x4, y4, x1, y1) << std::endl;
       std::cout << "   cp4 : " << cross_product (x3, y3, x4, y4, x2, y2) << std::endl;
    */
    return ( cross_product( x1, y1, x2, y2, x3, y3 ) * cross_product( x1, y1, x2, y2, x4, y4 ) < 0
             && cross_product( x3, y3, x4, y4, x1, y1 ) * cross_product( x3, y3, x4, y4, x2, y2 ) < 0 );
  }




  /*
   */

  bool computeSegIntersectionExt( double x1, double y1, double x2, double y2, double xs1, double ys1,  // 1st (segment)
                                  double x3, double y3, double x4, double y4, double xs2, double ys2, // 2nd segment
                                  double *x, double *y )
  {
    double cp1, cp2, cp3, cp4;
    cp1 = cross_product( x1, y1, x2, y2, x3, y3 );
    cp2 = cross_product( x1, y1, x2, y2, x4, y4 );
    cp3 = cross_product( x3, y3, x4, y4, x1, y1 );
    cp4 = cross_product( x3, y3, x4, y4, x2, y2 );


    if ( cp1 == 0 && cp2 == 0 && cp3 == 0 && cp4 == 0 )
    {
#ifdef _DEBUG_FULL_
      std::cout << "coolineaire..." << std::endl;
#endif
      return false;
    }

    // 1 ter
    if ( cp1 == 0 && cp3 == 0 )
    {
#ifdef _DEBUG_FULL_
      std::cout << "cp1 = cp3 = 0 => ignoring..." << std::endl;
#endif
      return false;
    }

    // 1 bis
    if ( cp1 == 0 && cp4 == 0 )
    {
#ifdef _DEBUG_FULL_
      std::cout << "cp1 = cp4 = 0 => ignoring..." << std::endl;
#endif
      return false;
    }

    // 1 bis
    if ( cp2 == 0 && cp3 == 0 )
    {
#ifdef _DEBUG_FULL_
      std::cout << "cp2 = cp3 = 0 => ignoring..." << std::endl;
#endif
      return false;
    }

    // 2bis and 3bis
    if ( cp1 == 0 || cp3 == 0 )
    {
#ifdef _DEBUG_FULL_
      std::cout << "skip..." << std::endl;
#endif
      return false;
    }

    // case 3
    if ( cp4 == 0 && cp1 * cp1 < 0 )
    {
      if ( cross_product( x3, y3, x4, y4, xs1, ys1 ) * cp3 < 0 )
      {
        *x = x2;
        *y = y2;
        return true;
      }
      else
        return false;
    }

    // case 2
    if ( cp2 == 0 && cp3 * cp4 < 0 )
    {
      if ( cross_product( x1, y1, x2, y2, xs2, ys2 ) * cp1 < 0 )
      {
        *x = x4;
        *y = y4;
        return true;
      }
      else
        return false;
    }

    // case 1
    if ( cp2 == 0 && cp4 == 0 )
    {
      double distance[4];
      double cx, cy;
      double dx, dy;
      double nx[4], ny[4];
      double toDist;
      double ratio;
      int i;

      cx = x2;
      cy = y2;

      nx[0] = x1;
      ny[0] = y1;

      nx[1] = xs1;
      ny[1] = ys1;

      nx[2] = x3;
      ny[2] = y3;

      nx[3] = xs2;
      ny[3] = ys2;

      distance[0] = dist_euc2d( cx, cy, x1, y1 ); // i
      toDist = distance[0];

      distance[1] = dist_euc2d( cx, cy, xs1, ys1 );// j2
      toDist = max( toDist, distance[1] );

      distance[2] = dist_euc2d( cx, cy, x3, y3 );// k
      toDist = max( toDist, distance[2] );

      distance[3] = dist_euc2d( cx, cy, xs2, ys2 ); // l2
      toDist = max( toDist, distance[3] );

      for ( i = 0;i < 4;i++ )
      {
        dx = nx[i] - cx;
        dy = ny[i] - cy;

        ratio = toDist / distance[i];

        nx[i] = cx + dx * ratio;
        ny[i] = cy + dy * ratio;
      }

      bool return_val =  computeSegIntersection( nx[0], ny[0], nx[1], ny[1], nx[2], ny[2], nx[3], ny[3], x, y );

      return return_val;
    }

    if ( cp1 * cp2 <= 0
         && cp3 *cp4 <= 0 )
    {
      return computeLineIntersection( x1, y1, x2, y2, x3, y3, x4, y4, x, y );
    }

    return false;
  }




  /*
   * \brief Intersection bw a line and a segment
   * \return true if the point exist false otherwise
   */
  bool computeLineSegIntersection( double x1, double y1, double x2, double y2,  // 1st line
                                   double x3, double y3, double x4, double y4,  // 2nd segment
                                   double *x, double *y )
  {
    double cp1, cp2;
    cp1 = cross_product( x1, y1, x2, y2, x3, y3 );
    cp2 = cross_product( x1, y1, x2, y2, x4, y4 );

    if ( cp1 * cp2 <= 0 )
      return computeLineIntersection( x1, y1, x2, y2, x3, y3, x4, y4, x, y );

    return false;
  }



  /*
   * \brief compute the point wherre two segment intersects
   * \return true if the point exist false otherwise
   */

  bool computeSegIntersection( double x1, double y1, double x2, double y2,  // 1st (segment)
                               double x3, double y3, double x4, double y4,  // 2nd segment
                               double *x, double *y )
  {
    double cp1, cp2, cp3, cp4;
    cp1 = cross_product( x1, y1, x2, y2, x3, y3 );
    cp2 = cross_product( x1, y1, x2, y2, x4, y4 );
    cp3 = cross_product( x3, y3, x4, y4, x1, y1 );
    cp4 = cross_product( x3, y3, x4, y4, x2, y2 );

    if ( cp1 * cp2 <= 0
         && cp3 *cp4 <= 0 )
      return computeLineIntersection( x1, y1, x2, y2, x3, y3, x4, y4, x, y );

    return false;
  }

  /*
   * \brief compute the point wherre two lines intersects
   * \return true if the ok false if line are parallel
   */
  bool computeLineIntersection( double x1, double y1, double x2, double y2,  // 1st line (segment)
                                double x3, double y3, double x4, double y4,  // 2nd line segment
                                double *x, double *y )
  {

    double a1, a2, b1, b2, c1, c2;
    double denom;

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = x2 * y1 - x1 * y2;

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;

    if (( denom = a1 * b2 - a2 * b1 ) == 0 )
    {
      return false;
    }
    else
    {
      *x = ( b1 * c2 - b2 * c1 ) / denom;
      *y = ( a2 * c1 - a1 * c2 ) / denom;
    }

    return true;

  }



  /*
   * \brief Compute the convex hull in O(n·log(n))
   * \param id set of point (i.e. point no 0 is (x,y) = x[id[0]],y[id[0]])
   * \param x x coordinates
   * \param y y coordinates
   * \param n Size of subset (vector id)
   * \param cHull returns the point id (id of id's vector...) whom are parts of the convex hull
   * \return convexHull's size
   */
  int convexHullId( int *id, const double* const x, const double* const y, int n, int *&cHull )
  {
    int i;

    cHull = new int[n];
    for ( i = 0;i < n;i++ )
    {
      cHull[i] = i;
    }


    if ( n <= 3 ) return n;

    int* stack = new int[n];
    double* tan = new double [n];
    int ref;

    int second, top;
    double result;

    // find the lowest y value
    heapsort( cHull, id, y, n );

    // find the lowest x value from the lowest y
    ref = 1;
    while ( ref < n && vabs( y[id[cHull[ref]]] -  y[id[cHull[0]]] ) < EPSILON ) ref++;

    heapsort( cHull, id, x, ref );

    // the first point is now for sure in the hull as well as the ref one
    for ( i = ref; i < n; i++ )
    {
      if ( vabs( y[id[cHull[i]]] - y[id[cHull[0]]] ) < EPSILON )
        tan[i] = FLT_MAX;
      else
        tan[i] = ( x[id[cHull[0]]] - x[id[cHull[i]]] ) / ( y[id[cHull[i]]] - y[id[cHull[0]]] );
    }

    if ( ref < n )
      heapsort2( cHull + ref, tan + ref, n - ref );

    // the second point is in too
    stack[0] = cHull[0];
    if ( ref == 1 )
    {
      stack[1] = cHull[1];
      ref++;
    }
    else
      stack[1] = cHull[ref-1];


    top = 1;
    second = 0;

    for ( i = ref; i < n; i++ )
    {
      result = cross_product( x[id[stack[second]]], y[id[stack[second]]],
                              x[id[stack[top]]], y[id[stack[top]]], x[id[cHull[i]]], y[id[cHull[i]]] );
      // Coolineaire !! garder le plus éloigné
      if ( vabs( result ) < EPSILON )
      {
        if ( dist_euc2d_sq( x[id[stack[second]]], y[id[stack[second]]], x[id[cHull[i]]], y[id[cHull[i]]] )
             >  dist_euc2d_sq( x[id[stack[second]]], y[id[stack[second]]], x[id[stack[top]]], y[id[stack[top]]] ) )
        {
          stack[top] = cHull[i];
        }
      }
      else if ( result > 0 ) //convexe
      {
        second++;
        top++;
        stack[top] = cHull[i];
      }
      else
      {
        while ( result < 0 && top > 1 )
        {
          second--;
          top--;
          result = cross_product( x[id[stack[second]]],
                                  y[id[stack[second]]], x[id[stack[top]]],
                                  y[id[stack[top]]], x[id[cHull[i]]], y[id[cHull[i]]] );
        }
        second++;
        top++;
        stack[top] = cHull[i];
      }
    }

    for ( i = 0;i <= top;i++ )
    {
      cHull[i] = stack[i];
    }

    delete[] stack;
    delete[] tan;

    return top + 1;
  }

// reorder points to have cross prod ((x,y)[i], (x,y)[i+1), point) > 0 when point is outside
  int reorderPolygon( int nbPoints, double *x, double *y )
  {
    int inc = 0;
    int *cHull;
    int cHullSize;
    int i;

    int *pts = new int[nbPoints];
    for ( i = 0;i < nbPoints;i++ )
      pts[i] = i;


    cHullSize = convexHullId( pts, x, y, nbPoints, cHull );

    if ( pts[cHull[0]] < pts[cHull[1]] && pts[cHull[1]] < pts[cHull[2]] )
      inc = 1;
    else if ( pts[cHull[0]] > pts[cHull[1]] && pts[cHull[1]] > pts[cHull[2]] )
      inc = -1;
    else if ( pts[cHull[0]] > pts[cHull[1]] && pts[cHull[1]] < pts[cHull[2]] && pts[cHull[2]] < pts[cHull[0]] )
      inc = 1;
    else if ( pts[cHull[0]] > pts[cHull[1]] && pts[cHull[1]] < pts[cHull[2]] && pts[cHull[2]] > pts[cHull[0]] )
      inc = -1;
    else if ( pts[cHull[0]] < pts[cHull[1]] && pts[cHull[1]] > pts[cHull[2]] && pts[cHull[2]] > pts[cHull[0]] )
      inc = -1;
    else if ( pts[cHull[0]] < pts[cHull[1]] && pts[cHull[1]] > pts[cHull[2]] && pts[cHull[2]] < pts[cHull[0]] )
      inc = 1;
    else
    {
      std::cout << "Warning wrong cHull -> geometry: " << nbPoints << std::endl;
      for ( i = 0;i < nbPoints;i++ )
      {
        std::cout << x[i] << ";" << y[i] << std::endl;
      }
      std::cout << "hull : " << cHullSize << std::endl;
      for ( i = 0;i < cHullSize;i++ )
      {
        std::cout << pts[cHull[i]] << " ";
      }
      std::cout << std::endl;
      delete[] cHull;
      delete[] pts;
      return -1;
    }

    if ( inc == -1 ) // re-order points
    {
      double tmp;
      int j;
      for ( i = 0, j = nbPoints - 1;i <= j;i++, j-- )
      {
        tmp = x[i];
        x[i] = x[j];
        x[j] = tmp;

        tmp = y[i];
        y[i] = y[j];
        y[j] = tmp;
      }
    }


    delete[] cHull;
    delete[] pts;

    return 0;

  }


  bool isPointInPolygon( int npol, double *xp, double *yp, double x, double y )
  {
    // code from Randolph Franklin (found at http://local.wasp.uwa.edu.au/~pbourke/geometry/insidepoly/)
    int i, j;
    bool c = false;

    for ( i = 0, j = npol - 1; i < npol; j = i++ )
    {
      if (((( yp[i] <= y ) && ( y < yp[j] ) ) ||
           (( yp[j] <= y ) && ( y < yp[i] ) ) )
          && ( x < ( xp[j] - xp[i] ) * ( y - yp[i] ) / ( yp[j] - yp[i] ) + xp[i] ) )
      {
        c = !c;
      }
    }
    return c;
  }

#ifdef _EXPORT_MAP_
  void toSVGPath( int nbPoints, int geomType, double *x, double *y,
                  int dpi, double scale, int xmin, int ymax,
                  char *layername, char *objectID,
                  std::ostream &out )
  {
    int i;

    if ( nbPoints > 1 )
    {
      out << "  <path style=\"fill:none;fill-opacity:1;fill-rule:evenodd;stroke:#000000;stroke-width:1;stroke-linecap:round;stroke-linejoin:round;stroke-opacity:1\" d=\"M " << convert2pt( x[0], scale, dpi ) - xmin << "," << ymax - convert2pt( y[0], scale, dpi ) << " ";
      for ( i = 1;i < nbPoints;i++ )
      {
        out << "L " << convert2pt( x[i], scale, dpi ) - xmin  << ", " << ymax - convert2pt( y[i], scale, dpi ) << " ";
      }

      if ( geomType == GEOS_POLYGON )
      {
        out << " z ";
      }

      out << "\" ";
      out << "id=\"" << layername << "-" << objectID << "\" ";
      out << "inkscape:label=\"#path-" << layername << "-" << objectID << "\"/>\n";
    }
    else
    {
      int cx = convert2pt( x[0], scale, dpi ) - xmin;
      int cy = ymax - convert2pt( y[0], scale, dpi );
      out << "   <path ";
      out << "      sodipodi:type=\"arc\" ";
      out << "      style=\"opacity:1;fill:#bcbcbc;fill-opacity:l;stroke:#000000;stroke-opacity:1;stroke-width:0.5;stroke-linejoin:miter;stroke-dasharray:none;display:inline\"";
      out << "      id=\"" << layername << "-" << objectID << "\" ";
      out << "      sodipodi:cx=\"" << cx << "\" ";
      out << "      sodipodi:cy=\"" << cy << "\" ";
      out << "      sodipodi:rx=\"1\" ";
      out << "      sodipodi:ry=\"1\" ";
      out << "      d=\"M " << cx + 1 << " " << cy << " A 1 1 0 1 1 "
      << cx - 1 << "," << cy << " A 1 1 0 1 1 "
      << cx + 1 << " " << cy << " z\" ";
      out << "      inkscape:label=\"#path-" << layername << "-" << objectID << "\" />\n";
    }
  }
#endif


    void findLineCircleIntersection(double cx, double cy, double radius,
                                  double x1, double y1, double x2, double y2,
                                  double& xRes, double& yRes)
    {
      double dx = x2 - x1;
      double dy = y2 - y1;

      double A = dx * dx + dy * dy;
      double B = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
      double C = (x1 - cx) * (x1 - cx) + (y1 - cy) * (y1 - cy) - radius * radius;

      double det = B * B - 4 * A * C;
      if (A <= 0.0000001 || det < 0)
          // Should never happen, No real solutions.
          return;

      if (det == 0)
      {
          // Could potentially happen.... One solution.
          double t = -B / (2 * A);
          xRes = x1 + t * dx;
          yRes = y1 + t * dy;
      }
      else
      {
          // Two solutions.
          // Always use the 1st one
          // We only really have one solution here, as we know the line segment will start in the circle and end outside
          double t = (-B + sqrt(det)) / (2 * A);
          xRes = x1 + t * dx;
          yRes = y1 + t * dy;
      }
    }


} // end namespace
